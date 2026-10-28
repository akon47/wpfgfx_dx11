using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Media;
using System.Windows.Threading;
using WpfGfx.Dx11;
using WpfGfx.Dx11.Interop;

namespace SmokeApp;

/// <summary>
/// Frame queue test (D19). A worker thread renders into N queues at 60 Hz with the app's own
/// D3D11 device (raw vtable calls, no packages): each frame clears a buffer with a cycling color
/// and draws a moving bar, then TryAcquire/keyed-mutex/Present. Meanwhile the UI thread can be
/// blocked periodically. Reports producer cost, queue counters, and how much of the window
/// changed between two captures 250 ms apart (continuous updates).
/// </summary>
internal sealed class FrameQueueWindow : Window
{
    private readonly List<ProducerQueue> _queues = new();
    private readonly List<D3D11Image> _images = new();
    private readonly IntPtr _device, _context, _context1;
    private Thread? _worker;
    private volatile bool _stop;
    private long _calls, _ticks, _maxTicks, _frames;

    public FrameQueueWindow(int queueCount)
    {
        Title = "SmokeApp frame queues";
        Width = 960;
        Height = 600;
        ResizeMode = ResizeMode.NoResize;
        ShowInTaskbar = false;
        Background = new SolidColorBrush(Color.FromRgb(0x20, 0x24, 0x28));

        // D3D_DRIVER_TYPE_HARDWARE = 1, BGRA_SUPPORT = 0x20, SDK version 7.
        Native.Check(Native.D3D11CreateDevice(IntPtr.Zero, 1, IntPtr.Zero, 0x20, IntPtr.Zero, 0, 7, out _device, out _, out _context), "D3D11CreateDevice");
        Guid iidContext1 = new("bb2c6faa-b5fb-4082-8e6b-388b8cfa90e1");
        Native.Check(Marshal.QueryInterface(_context, ref iidContext1, out _context1), "ID3D11DeviceContext1");

        var grid = new UniformGrid { Margin = new Thickness(4) };
        Content = grid;
        for (int i = 0; i < queueCount; i++)
        {
            (int w, int h) = (i % 4) switch { 0 => (1280, 720), 1 => (640, 360), _ => (320, 180) };
            var q = ProducerQueue.Create(_device, w, h, 3);
            _queues.Add(q);
            var image = new D3D11Image();
            Bind(image, q.Queue);
            image.IsFrontBufferAvailableChanged += (_, _) => { if (image.IsFrontBufferAvailable) Bind(image, q.Queue); };
            _images.Add(image);
            grid.Children.Add(new Border
            {
                Margin = new Thickness(2), BorderBrush = new SolidColorBrush(Color.FromRgb(0x50, 0x58, 0x60)), BorderThickness = new Thickness(1),
                Child = new Image { Source = image, Stretch = Stretch.Uniform },
            });
        }
    }

    private static void Bind(D3D11Image image, Dx11FrameQueue queue)
    {
        image.Lock();
        image.SetFrameQueue(queue);
        image.AddDirtyRect(new Int32Rect(0, 0, queue.Width, queue.Height));
        image.Unlock();
    }

    public void StartProducer()
    {
        _worker = new Thread(ProducerLoop) { IsBackground = true, Priority = ThreadPriority.Highest, Name = "fq producer" };
        _worker.Start();
    }

    public void StopProducer()
    {
        _stop = true;
        _worker?.Join(2000);
    }

    private void ProducerLoop()
    {
        var clock = Stopwatch.StartNew();
        double period = 1000.0 / 60.0;
        long frame = 0;
        while (!_stop)
        {
            double due = (frame + 1) * period;
            foreach (var q in _queues)
            {
                long t0 = Stopwatch.GetTimestamp();
                q.RenderFrame(_context, _context1, frame);
                long dt = Stopwatch.GetTimestamp() - t0;
                _ticks += dt;
                _calls++;
                if (dt > _maxTicks) _maxTicks = dt;
            }
            frame++;
            _frames = frame;
            double wait = due - clock.Elapsed.TotalMilliseconds;
            if (wait > 1.5) Thread.Sleep((int)(wait - 1));
            while (clock.Elapsed.TotalMilliseconds < due && !_stop) Thread.SpinWait(50);
        }
    }

    public string Report()
    {
        double usPerCall = _calls == 0 ? 0 : _ticks * 1e6 / Stopwatch.Frequency / _calls;
        double maxUs = _maxTicks * 1e6 / Stopwatch.Frequency;
        ulong presented = 0, dropped = 0, timeouts = 0, displayed = 0, failures = 0;
        int mutexFailures = 0, firstHr = 0;
        foreach (var q in _queues)
        {
            var s = q.Queue.Stats;
            presented += s.Presented; dropped += s.DroppedByProducer; timeouts += s.AcquireTimeouts; displayed += s.Displayed; failures += s.AcquireFailures;
            mutexFailures += q.AcquireFailures; if (firstHr == 0) firstHr = q.FirstAcquireHr;
        }
        return string.Format(System.Globalization.CultureInfo.InvariantCulture,
            "queues={0} frames={1} presented={2} displayed={3} droppedByProducer={4} acquireFailures={5} acquireTimeouts={6} producerMutexFailures={9} (hr=0x{10:x8}) producerUsPerQueueFrame={7:F1} maxUs={8:F0}",
            _queues.Count, _frames, presented, displayed, dropped, failures, timeouts, usPerCall, maxUs, mutexFailures, firstHr);
    }

    public void DisposeQueues()
    {
        foreach (var image in _images) { image.Lock(); image.SetFrameQueue(null); image.Unlock(); }
        foreach (var q in _queues) q.Dispose();
        _queues.Clear();
        if (_context1 != IntPtr.Zero) Marshal.Release(_context1);
        if (_context != IntPtr.Zero) Marshal.Release(_context);
        if (_device != IntPtr.Zero) Marshal.Release(_device);
    }

    /// <summary>Runs the whole scenario and returns the report line(s).</summary>
    public static async Task<string> RunAsync(Window owner, int queueCount, int seconds, bool blockUi, string? shotPath)
    {
        if (!Dx11FrameQueue.IsAvailable)
        {
            return "not available (stock engine): consumer falls back";
        }
        if (!Dx11FrameQueue.IsHardwareCapable(0))
        {
            return "not hardware capable on display 0: consumer falls back";
        }

        FrameQueueWindow? window = null;
        DispatcherTimer? blocker = null;
        try
        {
            window = new FrameQueueWindow(queueCount) { Owner = owner, Left = owner.Left + 40, Top = owner.Top + 80 };
            window.Show();
            window.StartProducer();

            if (blockUi)
            {
                blocker = new DispatcherTimer { Interval = TimeSpan.FromMilliseconds(500) };
                blocker.Tick += (_, _) => Thread.Sleep(100);   // UI thread stalls, producer must not care
                blocker.Start();
            }

            await Task.Delay(Math.Max(1000, seconds * 1000 - 400));

            string changed = "";
            if (shotPath != null)
            {
                string first = Path.Combine(Path.GetDirectoryName(shotPath)!, Path.GetFileNameWithoutExtension(shotPath) + "-a.png");
                ScreenCapture.Capture(window, first);
                await Task.Delay(250);
                ScreenCapture.Capture(window, shotPath);
                changed = string.Format(System.Globalization.CultureInfo.InvariantCulture, " changedBetweenCaptures={0:P1}", ChangedFraction(first, shotPath));
            }

            blocker?.Stop();
            window.StopProducer();
            return window.Report() + changed;
        }
        finally
        {
            // Whatever failed, leave no producer thread or window behind.
            blocker?.Stop();
            if (window != null)
            {
                window.StopProducer();
                try { window.DisposeQueues(); } catch { }
                window.Close();
            }
        }
    }

    private static double ChangedFraction(string a, string b)
    {
        using var ia = new System.Drawing.Bitmap(a);
        using var ib = new System.Drawing.Bitmap(b);
        int w = Math.Min(ia.Width, ib.Width), h = Math.Min(ia.Height, ib.Height), n = 0;
        for (int y = 0; y < h; y += 2)
            for (int x = 0; x < w; x += 2)
                if (ia.GetPixel(x, y) != ib.GetPixel(x, y)) n++;
        return n / (double)((w / 2) * (h / 2));
    }
}

/// <summary>One ring of shared textures on the app device plus its engine queue.</summary>
internal sealed unsafe class ProducerQueue : IDisposable
{
    public Dx11FrameQueue Queue = null!;
    private int _w, _h;
    private IntPtr[] _textures = null!, _views = null!, _mutexes = null!;
    private Native.AcquireSyncFn[] _acquire = null!;
    private Native.ReleaseSyncFn[] _release = null!;
    private Native.ClearRenderTargetViewFn _clear = null!;
    private Native.ClearViewFn _clearView = null!;
    public int AcquireFailures;
    public int FirstAcquireHr;

    public static ProducerQueue Create(IntPtr device, int w, int h, int buffers)
    {
        var q = new ProducerQueue { _w = w, _h = h, _textures = new IntPtr[buffers], _views = new IntPtr[buffers], _mutexes = new IntPtr[buffers], _acquire = new Native.AcquireSyncFn[buffers], _release = new Native.ReleaseSyncFn[buffers] };
        var handles = new IntPtr[buffers];
        var createTexture = Native.Method<Native.CreateTexture2DFn>(device, 5);
        var createRtv = Native.Method<Native.CreateRenderTargetViewFn>(device, 9);
        Guid iidResource1 = new("30961379-4609-4a41-998e-54fe567ee0c1");
        Guid iidKeyedMutex = new("9d8e1289-d7b3-465f-8126-250e349af85d");
        for (int i = 0; i < buffers; i++)
        {
            var desc = new Native.TEXTURE2D_DESC
            {
                Width = (uint)w, Height = (uint)h, MipLevels = 1, ArraySize = 1, Format = 87, SampleCount = 1, SampleQuality = 0,
                Usage = 0, BindFlags = 0x8 | 0x20, CPUAccessFlags = 0,
                MiscFlags = 0x800 | 0x100,  // D3D11_RESOURCE_MISC_SHARED_NTHANDLE | D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX
            };
            Native.Check(createTexture(device, ref desc, IntPtr.Zero, out q._textures[i]), "CreateTexture2D");
            Native.Check(createRtv(device, q._textures[i], IntPtr.Zero, out q._views[i]), "CreateRenderTargetView");
            Native.Check(Marshal.QueryInterface(q._textures[i], ref iidResource1, out IntPtr resource1), "IDXGIResource1");
            // IDXGIResource1::CreateSharedHandle is slot 13: DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE.
            Native.Check(Native.Method<Native.CreateSharedHandleFn>(resource1, 13)(resource1, IntPtr.Zero, 0x80000000u | 1u, IntPtr.Zero, out handles[i]), "CreateSharedHandle");
            Marshal.Release(resource1);
            Native.Check(Marshal.QueryInterface(q._textures[i], ref iidKeyedMutex, out q._mutexes[i]), "IDXGIKeyedMutex");
            q._acquire[i] = Native.Method<Native.AcquireSyncFn>(q._mutexes[i], 8);
            q._release[i] = Native.Method<Native.ReleaseSyncFn>(q._mutexes[i], 9);
        }
        q.Queue = Dx11FrameQueue.Create(w, h, handles);
        foreach (IntPtr hnd in handles) Native.CloseHandle(hnd);   // the engine holds duplicates
        return q;
    }

    public void RenderFrame(IntPtr context, IntPtr context1, long frame)
    {
        if (!Queue.TryAcquire(out int i)) return;
        // Producer protocol: key 0 to render, key 1 when done.
        int hrAcquire = _acquire[i](_mutexes[i], 0, 1);
        if (hrAcquire != 0)
        {
            if (AcquireFailures++ == 0) FirstAcquireHr = hrAcquire;
            Queue.Discard(i);
            return;
        }
        _clear ??= Native.Method<Native.ClearRenderTargetViewFn>(context, 50);
        _clearView ??= Native.Method<Native.ClearViewFn>(context1, 132);
        float hue = (frame % 180) / 180f;
        float* color = stackalloc float[4];
        HsvToRgb(hue, color);
        _clear(context, _views[i], color);
        float* white = stackalloc float[4] { 1, 1, 1, 1 };
        int x = (int)((frame * 7) % Math.Max(1, _w - 24));
        var rect = new Native.RECT { Left = x, Top = 0, Right = x + 24, Bottom = _h };
        _clearView(context1, _views[i], white, &rect, 1);
        _release[i](_mutexes[i], 1);
        Queue.Present(i);
    }

    private static void HsvToRgb(float h, float* rgba)
    {
        float r = Math.Clamp(Math.Abs(h * 6 - 3) - 1, 0, 1), g = Math.Clamp(2 - Math.Abs(h * 6 - 2), 0, 1), b = Math.Clamp(2 - Math.Abs(h * 6 - 4), 0, 1);
        rgba[0] = r * 0.8f; rgba[1] = g * 0.8f; rgba[2] = b * 0.8f; rgba[3] = 1;
    }

    public void Dispose()
    {
        Queue?.Dispose();
        for (int i = 0; i < (_textures?.Length ?? 0); i++)
        {
            if (_mutexes[i] != IntPtr.Zero) Marshal.Release(_mutexes[i]);
            if (_views[i] != IntPtr.Zero) Marshal.Release(_views[i]);
            if (_textures[i] != IntPtr.Zero) Marshal.Release(_textures[i]);
        }
    }
}

internal static unsafe class Native
{
    [DllImport("d3d11.dll", ExactSpelling = true)]
    public static extern int D3D11CreateDevice(IntPtr adapter, uint driverType, IntPtr software, uint flags, IntPtr featureLevels, uint featureLevelCount, uint sdkVersion, out IntPtr device, out uint featureLevel, out IntPtr context);

    [DllImport("kernel32.dll", ExactSpelling = true)]
    public static extern bool CloseHandle(IntPtr handle);

    [StructLayout(LayoutKind.Sequential)]
    public struct TEXTURE2D_DESC { public uint Width, Height, MipLevels, ArraySize, Format, SampleCount, SampleQuality, Usage, BindFlags, CPUAccessFlags, MiscFlags; }

    [StructLayout(LayoutKind.Sequential)]
    public struct RECT { public int Left, Top, Right, Bottom; }

    public delegate int CreateTexture2DFn(IntPtr self, ref TEXTURE2D_DESC desc, IntPtr initialData, out IntPtr texture);
    public delegate int CreateRenderTargetViewFn(IntPtr self, IntPtr resource, IntPtr desc, out IntPtr view);
    public delegate void ClearRenderTargetViewFn(IntPtr self, IntPtr view, float* color);
    public delegate void ClearViewFn(IntPtr self, IntPtr view, float* color, RECT* rects, uint count);
    public delegate int CreateSharedHandleFn(IntPtr self, IntPtr attributes, uint access, IntPtr name, out IntPtr handle);
    public delegate int AcquireSyncFn(IntPtr self, ulong key, uint milliseconds);
    public delegate int ReleaseSyncFn(IntPtr self, ulong key);

    public static T Method<T>(IntPtr obj, int slot) where T : Delegate =>
        Marshal.GetDelegateForFunctionPointer<T>(Marshal.ReadIntPtr(Marshal.ReadIntPtr(obj), slot * IntPtr.Size));

    public static void Check(int hr, string what)
    {
        if (hr < 0) throw new InvalidOperationException($"{what} failed 0x{hr:x8}");
    }
}
