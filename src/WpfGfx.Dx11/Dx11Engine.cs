using System.Runtime.InteropServices;

namespace WpfGfx.Dx11;

/// <summary>
/// Information reported by the D3D11 engine (<c>WpfGfxDx11_GetInfo</c>).
/// </summary>
/// <param name="AbiVersion">Version of the extra export ABI.</param>
/// <param name="FeatureLevel">D3D_FEATURE_LEVEL of the primary display's device (0xb100 = 11.1).</param>
/// <param name="DisplayCount">Displays backed by a DXGI adapter.</param>
/// <param name="AdapterLuid">LUID of the primary display's adapter.</param>
public readonly record struct Dx11EngineInfo(uint AbiVersion, uint FeatureLevel, uint DisplayCount, long AdapterLuid);

/// <summary>
/// Entry points of the D3D11 WpfGfx engine that the stock WPF API cannot reach. Everything here
/// binds to the <c>wpfgfx_cor3.dll</c> instance WPF has already loaded; call it after a window
/// has been shown so the engine exists.
/// </summary>
public static class Dx11Engine
{
    private const string EngineModule = "wpfgfx_cor3.dll";

    [StructLayout(LayoutKind.Sequential)]
    private struct NativeInfo
    {
        public uint cbSize;
        public uint uAbiVersion;
        public uint uFeatureLevel;
        public uint uDisplayCount;
        public long luidAdapter;
    }

    [DllImport(EngineModule, ExactSpelling = true)]
    private static extern int WpfGfxDx11_GetInfo(ref NativeInfo info);

    [DllImport(EngineModule, ExactSpelling = true)]
    private static extern int WpfGfxDx11_GetDevice(uint displayIndex, out IntPtr device);

    [DllImport(EngineModule, ExactSpelling = true)]
    private static extern int WpfGfxDx11_SetOption(uint option, uint value);

    private const uint OptionHardwareRenderTargetBitmap = 1;
    private const uint OptionGpuAntialiasing = 2;
    private static bool s_hardwareRenderTargetBitmap;

    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, ExactSpelling = true)]
    private static extern IntPtr GetModuleHandleW(string moduleName);

    static Dx11Engine()
    {
        // Resolve our P/Invokes against the engine WPF loaded (never a second copy).
        NativeLibrary.SetDllImportResolver(typeof(Dx11Engine).Assembly, (name, _, _) =>
            name.Equals(EngineModule, StringComparison.OrdinalIgnoreCase) ? GetModuleHandleW(EngineModule) : IntPtr.Zero);
    }

    /// <summary>
    /// True when the loaded rendering engine is the D3D11 engine (it exports the extra entry
    /// points). False on the stock DX9 engine, or before WPF has loaded any engine.
    /// </summary>
    public static bool IsAvailable
    {
        get
        {
            IntPtr module = GetModuleHandleW(EngineModule);
            return module != IntPtr.Zero && NativeLibrary.TryGetExport(module, "WpfGfxDx11_GetInfo", out _);
        }
    }

    /// <summary>Reports ABI version, feature level and adapter of the engine's device.</summary>
    /// <exception cref="NotSupportedException">The D3D11 engine is not loaded.</exception>
    public static Dx11EngineInfo GetInfo()
    {
        EnsureAvailable();
        var info = new NativeInfo { cbSize = (uint)Marshal.SizeOf<NativeInfo>() };
        Marshal.ThrowExceptionForHR(WpfGfxDx11_GetInfo(ref info));
        return new Dx11EngineInfo(info.uAbiVersion, info.uFeatureLevel, info.uDisplayCount, info.luidAdapter);
    }

    /// <summary>
    /// Returns the engine's <c>ID3D11Device</c> for a display, AddRef'd. Release it with
    /// <see cref="Marshal.Release"/>. Create resources on it (thread-safe); never use its
    /// immediate context, which belongs to the engine's render thread.
    /// </summary>
    /// <exception cref="NotSupportedException">The D3D11 engine is not loaded.</exception>
    public static IntPtr GetDevice(uint displayIndex = 0)
    {
        EnsureAvailable();
        Marshal.ThrowExceptionForHR(WpfGfxDx11_GetDevice(displayIndex, out IntPtr device));
        return device;
    }

    /// <summary>
    /// When true, <see cref="System.Windows.Media.Imaging.RenderTargetBitmap"/> renders on the GPU
    /// (effects included, ps_3_0 as well) instead of the software rasterizer. Opt-in because the
    /// pixel output differs slightly from the software path (anti-aliasing, text). Takes effect for
    /// bitmaps created after the change. Needs ABI 2 of the engine.
    /// </summary>
    /// <exception cref="NotSupportedException">The D3D11 engine is not loaded.</exception>
    public static bool HardwareRenderTargetBitmap
    {
        get => s_hardwareRenderTargetBitmap;
        set
        {
            EnsureAvailable();
            Marshal.ThrowExceptionForHR(WpfGfxDx11_SetOption(OptionHardwareRenderTargetBitmap, value ? 1u : 0u));
            s_hardwareRenderTargetBitmap = value;
        }
    }

    /// <summary>
    /// MSAA sample count for GPU anti-aliasing of windows (0 = off, the default; 4 or 8). When on, shape
    /// edges are anti-aliased by the GPU instead of WPF's CPU coverage rasterizer, which takes a large share
    /// of render-thread time in shape-heavy, animated scenes. Edges then differ slightly from the stock
    /// output, and content drawn with <c>EdgeMode.Aliased</c> also gets smooth edges. Applies to windows
    /// created or resized after the change; set it before the first window is shown. Same as the
    /// environment variable <c>WPFGFX_DX11_MSAA=&lt;samples&gt;</c>.
    /// </summary>
    /// <exception cref="NotSupportedException">The D3D11 engine is not loaded.</exception>
    public static int GpuAntialiasingSamples
    {
        get => s_gpuAntialiasingSamples;
        set
        {
            EnsureAvailable();
            Marshal.ThrowExceptionForHR(WpfGfxDx11_SetOption(OptionGpuAntialiasing, (uint)Math.Max(0, value)));
            s_gpuAntialiasingSamples = value;
        }
    }

    private static int s_gpuAntialiasingSamples;

    internal static void EnsureAvailable()
    {
        if (!IsAvailable)
        {
            throw new NotSupportedException("The D3D11 WpfGfx engine is not loaded; this API is unavailable on the stock DX9 engine.");
        }
    }
}
