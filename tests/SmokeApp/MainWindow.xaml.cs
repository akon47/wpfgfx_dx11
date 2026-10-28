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
///   SmokeApp.exe [--screenshot out.png] [--exit] [--delay ms]
/// The report is printed to stdout and written to smoke-report.txt next to the exe.
/// </summary>
public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();
        Loaded += OnLoaded;
    }

    private async void OnLoaded(object sender, RoutedEventArgs e)
    {
        string? shot = null;
        bool exit = false;
        int delayMs = 2000;
        string[] args = Environment.GetCommandLineArgs();
        for (int i = 1; i < args.Length; i++)
        {
            if (args[i] == "--screenshot" && i + 1 < args.Length) shot = args[++i];
            else if (args[i] == "--exit") exit = true;
            else if (args[i] == "--delay" && i + 1 < args.Length) delayMs = int.Parse(args[++i], System.Globalization.CultureInfo.InvariantCulture);
        }
        await Task.Delay(delayMs);

        string report = BuildReport();
        Title = "SmokeApp | " + report.Split('\n')[0].Trim();
        File.WriteAllText(Path.Combine(AppContext.BaseDirectory, "smoke-report.txt"), report);
        Console.Write(report);

        if (shot != null)
        {
            string full = Path.GetFullPath(shot);
            Capture(full);
            Console.WriteLine("screenshot=" + full);
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
        sb.AppendLine($"NativeSearch={AppContext.GetData("NATIVE_DLL_SEARCH_DIRECTORIES")}");
        string[] prefixes = { "wpfgfx", "d3d", "dxgi", "presentationnative", "dwrite", "dcomp" };
        foreach (ProcessModule m in Process.GetCurrentProcess().Modules)
        {
            string n = m.ModuleName.ToLowerInvariant();
            if (Array.Exists(prefixes, p => n.StartsWith(p, StringComparison.Ordinal)))
                sb.AppendLine($"  {m.ModuleName,-28} {m.FileVersionInfo.FileVersion,-24} {m.FileName}");
        }
        return sb.ToString();
    }

    private void Capture(string path)
    {
        IntPtr hwnd = new WindowInteropHelper(this).Handle;
        GetWindowRect(hwnd, out RECT r);
        int w = r.Right - r.Left, h = r.Bottom - r.Top;
        using var bmp = new System.Drawing.Bitmap(w, h);
        using (var g = System.Drawing.Graphics.FromImage(bmp))
        {
            // PrintWindow renders only this window (via DWM), independent of z-order,
            // so overlapping windows never end up in the capture.
            IntPtr hdc = g.GetHdc();
            try { PrintWindow(hwnd, hdc, PW_RENDERFULLCONTENT); }
            finally { g.ReleaseHdc(hdc); }
        }
        bmp.Save(path, System.Drawing.Imaging.ImageFormat.Png);
    }

    private const uint PW_RENDERFULLCONTENT = 0x00000002;

    [StructLayout(LayoutKind.Sequential)]
    private struct RECT { public int Left, Top, Right, Bottom; }

    [DllImport("user32.dll")]
    private static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);

    [DllImport("user32.dll")]
    private static extern bool PrintWindow(IntPtr hWnd, IntPtr hdcBlt, uint nFlags);
}
