using System.Runtime.InteropServices;

namespace WpfGfx.Dx11.Interop;

/// <summary>Who owns the ring of textures behind a <see cref="Dx11FrameQueue"/>.</summary>
public enum Dx11FrameQueueTextures
{
    /// <summary>The producer created the ring on its own device (MISC_SHARED_NTHANDLE | MISC_SHARED_KEYEDMUTEX) and passes NT handles.</summary>
    User = 1,
    /// <summary>The engine creates the ring on its device; the producer opens <see cref="Dx11FrameQueue.SharedHandles"/> with OpenSharedResource1.</summary>
    Engine = 2,
}

/// <summary>Per-queue counters (<c>WpfGfxDx11_FrameQueue_GetStats</c>).</summary>
public readonly record struct Dx11FrameQueueStats(
    ulong Presented,
    ulong DroppedByProducer,
    ulong AcquireTimeouts,
    ulong Displayed,
    ulong AcquireFailures);

/// <summary>
/// A ring of 2 to 4 shared textures that an app render thread fills and the D3D11 WPF engine
/// composites without the UI thread (D19). Protocol per frame on the producer side:
/// <list type="number">
/// <item><see cref="TryAcquire"/> gives a free buffer index or false (skip the frame).</item>
/// <item>AcquireSync(0) on that buffer's keyed mutex, render, ReleaseSync(1). No Flush needed.</item>
/// <item><see cref="Present"/> publishes it; the engine copies it during its next pass with AcquireSync(1)/ReleaseSync(0).</item>
/// </list>
/// Only the latest presented frame is shown; earlier ones still waiting are dropped. Neither call
/// blocks or allocates. Bind the queue to an image with <c>D3D11Image.SetFrameQueue</c> (WpfGfx.Dx11).
/// </summary>
public sealed class Dx11FrameQueue : IDisposable
{
    private const string EngineModule = "wpfgfx_cor3.dll";
    private const uint MaxBuffers = 4;
    private const uint FlagUserTextures = 0x1, FlagEngineTextures = 0x2, FlagOpaque = 0x4;

    [StructLayout(LayoutKind.Sequential)]
    private unsafe struct NativeDesc
    {
        public uint cbSize, uWidth, uHeight, uFormat, cBuffers, uFlags;
        public fixed long rgSharedHandles[4];
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct NativeStats
    {
        public uint cbSize, uReserved;
        public ulong uPresented, uDroppedByProducer, uAcquireTimeouts, uDisplayed, uAcquireFailures;
    }

    [DllImport(EngineModule, ExactSpelling = true)] private static extern int WpfGfxDx11_FrameQueue_Create(ref NativeDesc desc, out IntPtr queue);
    [DllImport(EngineModule, ExactSpelling = true)] private static extern int WpfGfxDx11_FrameQueue_GetSharedHandles(IntPtr queue, [Out] IntPtr[] handles, uint count);
    [DllImport(EngineModule, ExactSpelling = true)] private static extern int WpfGfxDx11_FrameQueue_TryAcquire(IntPtr queue, out uint index);
    [DllImport(EngineModule, ExactSpelling = true)] private static extern int WpfGfxDx11_FrameQueue_Present(IntPtr queue, uint index);
    [DllImport(EngineModule, ExactSpelling = true)] private static extern int WpfGfxDx11_FrameQueue_Discard(IntPtr queue, uint index);
    [DllImport(EngineModule, ExactSpelling = true)] private static extern int WpfGfxDx11_FrameQueue_GetStats(IntPtr queue, ref NativeStats stats);
    [DllImport(EngineModule, ExactSpelling = true)] private static extern int WpfGfxDx11_FrameQueue_QueryCapability(uint displayIndex, out int hardware);
    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, ExactSpelling = true)] private static extern IntPtr GetModuleHandleW(string moduleName);
    [DllImport("kernel32.dll", ExactSpelling = true)] private static extern bool CloseHandle(IntPtr handle);

    static Dx11FrameQueue()
    {
        NativeLibrary.SetDllImportResolver(typeof(Dx11FrameQueue).Assembly, (name, _, _) =>
            name.Equals(EngineModule, StringComparison.OrdinalIgnoreCase) ? GetModuleHandleW(EngineModule) : IntPtr.Zero);
    }

    private IntPtr _queue;
    private readonly object _lifetime = new();
    private IntPtr[]? _sharedHandles;

    private Dx11FrameQueue(IntPtr queue, int width, int height, int bufferCount, Dx11FrameQueueTextures textures)
    {
        _queue = queue;
        Width = width;
        Height = height;
        BufferCount = bufferCount;
        Textures = textures;
    }

    /// <summary>True when the loaded engine is the D3D11 engine with frame queue exports.</summary>
    public static bool IsAvailable
    {
        get
        {
            IntPtr module = GetModuleHandleW(EngineModule);
            return module != IntPtr.Zero && NativeLibrary.TryGetExport(module, "WpfGfxDx11_FrameQueue_Create", out _);
        }
    }

    /// <summary>
    /// Whether the engine can composite queues on the GPU for the given display (a hardware
    /// device that opens NT shared handles). False on software (WARP) composition: fall back.
    /// </summary>
    public static bool IsHardwareCapable(uint displayIndex = 0)
    {
        if (!IsAvailable) return false;
        return WpfGfxDx11_FrameQueue_QueryCapability(displayIndex, out int hw) >= 0 && hw != 0;
    }

    /// <summary>
    /// Creates a queue over the producer's own textures. <paramref name="ntSharedHandles"/> are the
    /// NT handles (IDXGIResource1::CreateSharedHandle) of 2 to 4 B8G8R8A8_UNORM textures created
    /// with MISC_SHARED_NTHANDLE | MISC_SHARED_KEYEDMUTEX; the engine duplicates them, so the caller
    /// may close its copies afterwards.
    /// </summary>
    public static unsafe Dx11FrameQueue Create(int width, int height, IntPtr[] ntSharedHandles, bool opaque = false)
    {
        if (ntSharedHandles.Length < 2 || ntSharedHandles.Length > MaxBuffers) throw new ArgumentOutOfRangeException(nameof(ntSharedHandles));
        EnsureAvailable();
        var desc = new NativeDesc
        {
            cbSize = (uint)Marshal.SizeOf<NativeDesc>(), uWidth = (uint)width, uHeight = (uint)height,
            uFormat = 87 /* DXGI_FORMAT_B8G8R8A8_UNORM */, cBuffers = (uint)ntSharedHandles.Length,
            uFlags = FlagUserTextures | (opaque ? FlagOpaque : 0),
        };
        for (int i = 0; i < ntSharedHandles.Length; i++) desc.rgSharedHandles[i] = (long)ntSharedHandles[i];
        Marshal.ThrowExceptionForHR(WpfGfxDx11_FrameQueue_Create(ref desc, out IntPtr queue));
        return new Dx11FrameQueue(queue, width, height, ntSharedHandles.Length, Dx11FrameQueueTextures.User);
    }

    /// <summary>
    /// Creates a queue whose textures the engine allocates on its own device. Open
    /// <see cref="SharedHandles"/> with ID3D11Device1::OpenSharedResource1 on the producer device.
    /// </summary>
    public static unsafe Dx11FrameQueue CreateWithEngineTextures(int width, int height, int bufferCount = 3, bool opaque = false)
    {
        if (bufferCount < 2 || bufferCount > MaxBuffers) throw new ArgumentOutOfRangeException(nameof(bufferCount));
        EnsureAvailable();
        var desc = new NativeDesc
        {
            cbSize = (uint)Marshal.SizeOf<NativeDesc>(), uWidth = (uint)width, uHeight = (uint)height,
            uFormat = 87, cBuffers = (uint)bufferCount, uFlags = FlagEngineTextures | (opaque ? FlagOpaque : 0),
        };
        Marshal.ThrowExceptionForHR(WpfGfxDx11_FrameQueue_Create(ref desc, out IntPtr queue));
        var q = new Dx11FrameQueue(queue, width, height, bufferCount, Dx11FrameQueueTextures.Engine);
        var handles = new IntPtr[bufferCount];
        Marshal.ThrowExceptionForHR(WpfGfxDx11_FrameQueue_GetSharedHandles(queue, handles, (uint)bufferCount));
        q._sharedHandles = handles;
        return q;
    }

    public int Width { get; }
    public int Height { get; }
    public int BufferCount { get; }
    public Dx11FrameQueueTextures Textures { get; }

    /// <summary>Engine-textures mode: NT handles owned by this queue (closed on Dispose). Null for user textures.</summary>
    public IReadOnlyList<IntPtr>? SharedHandles => _sharedHandles;

    /// <summary>The native queue object (IUnknown). Pass it to <c>D3D11Image.SetFrameQueue</c>.</summary>
    public IntPtr NativePointer => _queue;

    /// <summary>Takes a free buffer. False when none is free (skip this frame). Never blocks.</summary>
    public bool TryAcquire(out int index)
    {
        index = -1;
        IntPtr q = _queue;
        if (q == IntPtr.Zero) return false;
        int hr = WpfGfxDx11_FrameQueue_TryAcquire(q, out uint i);
        if (hr != 0) return false;   // S_FALSE (none free) or failure
        index = (int)i;
        return true;
    }

    /// <summary>
    /// Publishes a buffer acquired with <see cref="TryAcquire"/> (after ReleaseSync(1) on its keyed
    /// mutex). True when a bound image will show it, false when the queue is not bound yet (the frame
    /// is kept for the next binding) or disposed.
    /// </summary>
    public bool Present(int index)
    {
        IntPtr q = _queue;
        if (q == IntPtr.Zero) return false;
        return WpfGfxDx11_FrameQueue_Present(q, (uint)index) == 0;
    }

    /// <summary>Gives an acquired buffer back without presenting it (the frame could not be rendered).</summary>
    public bool Discard(int index)
    {
        IntPtr q = _queue;
        if (q == IntPtr.Zero) return false;
        return WpfGfxDx11_FrameQueue_Discard(q, (uint)index) == 0;
    }

    public Dx11FrameQueueStats Stats
    {
        get
        {
            IntPtr q = _queue;
            if (q == IntPtr.Zero) return default;
            var s = new NativeStats { cbSize = (uint)Marshal.SizeOf<NativeStats>() };
            Marshal.ThrowExceptionForHR(WpfGfxDx11_FrameQueue_GetStats(q, ref s));
            return new Dx11FrameQueueStats(s.uPresented, s.uDroppedByProducer, s.uAcquireTimeouts, s.uDisplayed, s.uAcquireFailures);
        }
    }

    /// <summary>
    /// Releases the native queue. Safe while a producer is between TryAcquire and Present: later calls
    /// return false. Detach the queue from its image source first (or let the image be collected).
    /// </summary>
    public void Dispose()
    {
        IntPtr q;
        lock (_lifetime)
        {
            q = _queue;
            _queue = IntPtr.Zero;
        }
        if (q != IntPtr.Zero) Marshal.Release(q);
        if (_sharedHandles != null)
        {
            foreach (IntPtr h in _sharedHandles) if (h != IntPtr.Zero) CloseHandle(h);
            _sharedHandles = null;
        }
        GC.SuppressFinalize(this);
    }

    ~Dx11FrameQueue() => Dispose();

    private static void EnsureAvailable()
    {
        if (!IsAvailable) throw new NotSupportedException("The D3D11 WpfGfx engine with frame queue support is not loaded.");
    }
}
