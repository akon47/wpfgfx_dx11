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
        bool exit = false;
        int delayMs = 2000;
        string[] args = Environment.GetCommandLineArgs();
        for (int i = 1; i < args.Length; i++)
        {
            if (args[i] == "--screenshot" && i + 1 < args.Length) shot = args[++i];
            else if (args[i] == "--transparent" && i + 1 < args.Length) transparentShot = args[++i];
            else if (args[i] == "--media" && i + 1 < args.Length) mediaFile = args[++i];
            else if (args[i] == "--media-shot" && i + 1 < args.Length) mediaShot = args[++i];
            else if (args[i] == "--bench" && i + 1 < args.Length) benchSeconds = ParseInt(args[++i]);
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
        if (benchSeconds > 0)
        {
            var bench = new BenchWindow(benchSeconds) { Owner = this, Left = Left + 20, Top = Top + 60 };
            bench.Show();
            string result = await bench.Completion;
            bench.Close();
            Console.WriteLine("bench: " + result);
            File.AppendAllText(Path.Combine(AppContext.BaseDirectory, "smoke-report.txt"), "bench: " + result + Environment.NewLine);
        }
        if (exit) Close();
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
