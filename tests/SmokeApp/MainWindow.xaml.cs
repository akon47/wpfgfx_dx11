using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows;
using System.Windows.Interop;
using System.Windows.Media;

namespace SmokeApp;

/// <summary>
/// Renders a fixed set of primitives (gradients, text, effects, 3D, blending) and reports
/// which graphics DLLs are loaded. Usage:
///   SmokeApp.exe [--screenshot out.png] [--exit] [--delay ms] [--left x] [--top y]
///                [--transparent out.png] [--media file.mp4 --media-shot out.png] [--bench seconds]
///                [--rtb out.png [--rtb-hw]] [--fq out.png [--fq-queues N] [--fq-seconds S] [--fq-block]]
/// --left/--top place the main window (e.g. straddling two monitors). --transparent also opens
/// a layered window (AllowsTransparency) over the main window and captures it from the screen.
/// The report is printed to stdout and written to smoke-report.txt next to the exe.
/// </summary>
public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();
        string[] args = Environment.GetCommandLineArgs();
        for (int i = 1; i < args.Length; i++)
        {
            if (args[i] == "--left" && i + 1 < args.Length) Left = ParseInt(args[++i]);
            else if (args[i] == "--top" && i + 1 < args.Length) Top = ParseInt(args[++i]);
        }
        Loaded += OnLoaded;
    }

    private static int ParseInt(string s) => int.Parse(s, System.Globalization.CultureInfo.InvariantCulture);

    private async void OnLoaded(object sender, RoutedEventArgs e)
    {
        InteropCells.Populate(D3D9Cell, D3D11Cell, D3D11Notice, new WindowInteropHelper(this).Handle);

        string? shot = null;
        string? transparentShot = null;
        string? mediaFile = null;
        string? mediaShot = null;
        int benchSeconds = 0;
        bool benchTransparent = false;
        string? rtbShot = null;
        bool rtbHardware = false;
        string? fqShot = null;
        int jitterFrames = 0;
        int fqQueues = 0, fqSeconds = 4;
        bool fqBlock = false;
        bool exit = false;
        int delayMs = 2000;
        int resizeSteps = 0;
        string[] args = Environment.GetCommandLineArgs();
        for (int i = 1; i < args.Length; i++)
        {
            if (args[i] == "--screenshot" && i + 1 < args.Length) shot = args[++i];
            else if (args[i] == "--transparent" && i + 1 < args.Length) transparentShot = args[++i];
            else if (args[i] == "--media" && i + 1 < args.Length) mediaFile = args[++i];
            else if (args[i] == "--media-shot" && i + 1 < args.Length) mediaShot = args[++i];
            else if (args[i] == "--bench" && i + 1 < args.Length) benchSeconds = ParseInt(args[++i]);
            else if (args[i] == "--bench-transparent") benchTransparent = true;
            else if (args[i] == "--rtb" && i + 1 < args.Length) rtbShot = args[++i];
            else if (args[i] == "--rtb-hw") rtbHardware = true;
            else if (args[i] == "--fq" && i + 1 < args.Length) { fqShot = args[++i]; if (fqQueues == 0) fqQueues = 8; }
            else if (args[i] == "--fq-queues" && i + 1 < args.Length) fqQueues = ParseInt(args[++i]);
            else if (args[i] == "--fq-seconds" && i + 1 < args.Length) fqSeconds = ParseInt(args[++i]);
            else if (args[i] == "--fq-block") fqBlock = true;
            else if (args[i] == "--spin-jitter" && i + 1 < args.Length) jitterFrames = ParseInt(args[++i]);
            else if (args[i] == "--resize" && i + 1 < args.Length) resizeSteps = ParseInt(args[++i]);
            else if (args[i] == "--exit") exit = true;
            else if (args[i] == "--delay" && i + 1 < args.Length) delayMs = ParseInt(args[++i]);
        }

        TransparentWindow? transparent = null;
        if (transparentShot != null)
        {
            transparent = new TransparentWindow { Owner = this, Left = Left + 120, Top = Top + 260 };
            transparent.Show();
        }

        MediaWindow? media = null;
        if (mediaFile != null)
        {
            media = new MediaWindow(mediaFile) { Owner = this, Left = Left + 500, Top = Top + 260 };
            media.Show();
        }

        await Task.Delay(delayMs);

        string report = BuildReport();
        Title = "SmokeApp | " + report.Split('\n')[0].Trim();
        File.WriteAllText(Path.Combine(AppContext.BaseDirectory, "smoke-report.txt"), report);
        Console.Write(report);

        if (shot != null)
        {
            string full = Path.GetFullPath(shot);
            string method = ScreenCapture.Capture(this, full);
            Console.WriteLine($"screenshot={full} via {method}");
        }
        if (transparent != null && transparentShot != null)
        {
            // PrintWindow gives the premultiplied colors regardless of z-order. The composed
            // result over the main window (which shows the alpha channel at work) can only be read
            // from the screen, so it is taken when nothing else covers the layered window.
            string full = Path.GetFullPath(transparentShot);
            string method = ScreenCapture.Capture(transparent, full);
            Console.WriteLine($"transparent={full} via {method}");
            string screenPath = Path.Combine(Path.GetDirectoryName(full)!, Path.GetFileNameWithoutExtension(full) + "-screen.png");
            if (ScreenCapture.IsUnobscured(transparent))
            {
                ScreenCapture.CaptureFromScreen(transparent, screenPath);
                Console.WriteLine($"transparent-screen={screenPath}");
            }
            else
            {
                Console.WriteLine("transparent-screen=skipped (another window covers the layered window)");
            }
        }
        if (media != null)
        {
            Console.WriteLine("media status: " + media.Status);
            if (mediaShot != null)
            {
                string full = Path.GetFullPath(mediaShot);
                string method = ScreenCapture.Capture(media, full);
                Console.WriteLine($"media-shot={full} via {method}");
            }
        }
        if (rtbShot != null)
        {
            try
            {
                Console.WriteLine("rtb: " + RtbTest.Render(Path.GetFullPath(rtbShot), rtbHardware));
            }
            catch (Exception ex)
            {
                Console.WriteLine("rtb: failed " + ex.Message);
            }
        }
        if (jitterFrames > 0)
        {
            Console.WriteLine("spin-jitter: " + await MeasureSpinJitterAsync(jitterFrames));
        }
        if (resizeSteps > 0)
        {
            Console.WriteLine("resize: " + await MeasureResizeAsync(resizeSteps));
        }
        if (fqShot != null)
        {
            try
            {
                Console.WriteLine("fq: " + await FrameQueueWindow.RunAsync(this, fqQueues, fqSeconds, fqBlock, Path.GetFullPath(fqShot)));
            }
            catch (Exception ex)
            {
                Console.WriteLine("fq: failed " + ex);
            }
        }
        if (benchSeconds > 0)
        {
            var bench = new BenchWindow(benchSeconds, benchTransparent) { Owner = this, Left = Left + 20, Top = Top + 60 };
            bench.Show();
            string result = await bench.Completion;
            bench.Close();
            Console.WriteLine("bench: " + result);
            File.AppendAllText(Path.Combine(AppContext.BaseDirectory, "smoke-report.txt"), "bench: " + result + Environment.NewLine);
        }
        if (exit) Close();
    }

    /// <summary>
    /// Captures consecutive frames of the spinning cell (top right) and reports how much the
    /// centroids of its shadow pixels and of its blue body move. The element rotates about its
    /// center with a fixed shadow offset, so both centroids should stay put; frame-to-frame
    /// wobble shows up as deviation in pixels.
    /// </summary>
    private async Task<string> MeasureSpinJitterAsync(int frames)
    {
        var shadow = new List<(double x, double y)>();
        var body = new List<(double x, double y)>();
        for (int f = 0; f < frames; f++)
        {
            using (var bmp = ScreenCapture.CaptureBitmap(this))
            {
                int x0 = bmp.Width * 3 / 4, x1 = bmp.Width, y0 = 0, y1 = bmp.Height / 4;
                double sx = 0, sy = 0, bx = 0, by = 0; int sn = 0, bn = 0;
                for (int y = y0; y < y1; y++)
                {
                    for (int x = x0; x < x1; x++)
                    {
                        var c = bmp.GetPixel(x, y);
                        int sum = c.R + c.G + c.B;
                        if (sum < 80) { sx += x; sy += y; sn++; }                    // shadow (darker than the 32,36,40 background)
                        else if (c.B > 150 && c.R < 110) { bx += x; by += y; bn++; } // blue body
                    }
                }
                if (sn > 0) shadow.Add((sx / sn, sy / sn));
                if (bn > 0) body.Add((bx / bn, by / bn));
            }
            await Task.Delay(25);
        }
        static double Deviation(List<(double x, double y)> pts)
        {
            if (pts.Count == 0) return -1;
            double mx = pts.Average(p => p.x), my = pts.Average(p => p.y);
            return pts.Max(p => Math.Sqrt((p.x - mx) * (p.x - mx) + (p.y - my) * (p.y - my)));
        }
        return string.Format(System.Globalization.CultureInfo.InvariantCulture,
            "frames={0} shadowCentroidMaxDev={1:F2}px bodyCentroidMaxDev={2:F2}px", frames, Deviation(shadow), Deviation(body));
    }

    /// <summary>
    /// Resizes the window step by step (alternating grow and shrink) and captures the client area
    /// after each step. Pure black pixels mean the window showed something other than a rendered
    /// frame; a flip model swap chain that is recreated instead of resized in place would expose
    /// the never painted redirection surface (D29).
    /// </summary>
    private async Task<string> MeasureResizeAsync(int steps)
    {
        double w0 = Width, h0 = Height;
        double maxBlack = 0, sumBlack = 0;
        string last = "";
        for (int i = 1; i <= steps; i++)
        {
            double t = (i % 2 == 0) ? i / (double)steps : 1 - i / (double)steps;
            Width = Math.Round(w0 * (0.7 + 0.5 * t));
            Height = Math.Round(h0 * (0.7 + 0.5 * t));
            await Task.Delay(60);
            using (var bmp = ScreenCapture.CaptureBitmap(this))
            {
                var rect = new System.Drawing.Rectangle(0, 0, bmp.Width, bmp.Height);
                var data = bmp.LockBits(rect, System.Drawing.Imaging.ImageLockMode.ReadOnly, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
                try
                {
                    byte[] px = new byte[data.Stride * bmp.Height];
                    System.Runtime.InteropServices.Marshal.Copy(data.Scan0, px, 0, px.Length);
                    long black = 0, n = (long)bmp.Width * bmp.Height;
                    for (int y = 0; y < bmp.Height; y++)
                    {
                        int row = y * data.Stride;
                        for (int x = 0; x < bmp.Width; x++)
                        {
                            int o = row + x * 4;
                            if (px[o] + px[o + 1] + px[o + 2] < 15) black++;
                        }
                    }
                    double frac = n > 0 ? black / (double)n : 1;
                    maxBlack = Math.Max(maxBlack, frac);
                    sumBlack += frac;
                    last = $"{bmp.Width}x{bmp.Height}";
                }
                finally { bmp.UnlockBits(data); }
            }
        }
        Width = w0; Height = h0;
        await Task.Delay(60);
        return string.Format(System.Globalization.CultureInfo.InvariantCulture,
            "steps={0} maxBlack={1:F2}% meanBlack={2:F2}% last={3}", steps, maxBlack * 100, sumBlack / steps * 100, last);
    }

    private string BuildReport()
    {
        var sb = new StringBuilder();
        int tier = RenderCapability.Tier >> 16;
        IntPtr hwnd = new WindowInteropHelper(this).Handle;
        HwndSource? src = HwndSource.FromHwnd(hwnd);
        sb.AppendLine($"Tier={tier} RenderMode={src?.CompositionTarget.RenderMode} ps2={RenderCapability.IsPixelShaderVersionSupported(2, 0)} ps3={RenderCapability.IsPixelShaderVersionSupported(3, 0)} maxTex={RenderCapability.MaxHardwareTextureSize}");
        sb.AppendLine($"Runtime={RuntimeInformation.FrameworkDescription} WPF={typeof(Window).Assembly.GetName().Version} ({typeof(Window).Assembly.Location})");
        sb.AppendLine($"Window=({Left},{Top}) {ActualWidth}x{ActualHeight}");
        sb.AppendLine($"NativeSearch={AppContext.GetData("NATIVE_DLL_SEARCH_DIRECTORIES")}");
        string[] prefixes = { "wpfgfx", "d3d", "dxgi", "presentationnative", "dwrite", "dcomp" };
        foreach (ProcessModule m in Process.GetCurrentProcess().Modules)
        {
            string n = m.ModuleName.ToLowerInvariant();
            if (Array.Exists(prefixes, p => n.StartsWith(p, StringComparison.Ordinal)))
                sb.AppendLine($"  {m.ModuleName,-28} {m.FileVersionInfo.FileVersion,-24} {m.FileName}");
        }
        sb.AppendLine("d3d9 watch: " + App.D3D9WatchStatus);
        lock (App.D3D9LoadStacks)
        {
            foreach (string stack in App.D3D9LoadStacks)
                sb.AppendLine("d3d9.dll loaded from: " + stack);
        }
        return sb.ToString();
    }
}
