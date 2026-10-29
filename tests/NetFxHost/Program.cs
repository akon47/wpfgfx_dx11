using System;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Effects;
using System.Windows.Shapes;

namespace NetFxHost;

/// <summary>
/// Runs a small WPF scene on .NET Framework 4.8.x, optionally on the D3D11 engine (D37).
/// Usage: NetFxHost.exe &lt;engine folder | -&gt; &lt;png&gt;. The engine folder holds the D3D11 engine renamed to
/// wpfgfx_v0400.dll plus D3DCompiler_47_cor3.dll; "-" runs the stock engine.
/// </summary>
/// <remarks>
/// .NET Framework loads "&lt;WPF install path&gt;\wpfgfx_v0400.dll" with LoadLibraryW and a full path before any
/// P/Invoke, so neither the app folder nor a preloaded copy takes its place, and there is no import resolver.
/// This host redirects that one load inside its own process by patching kernelbase!LoadLibraryW before the
/// first WPF type is touched. P/Invokes then bind to the redirected module by its base name. Test use only.
/// </remarks>
internal static class Program
{
    [DllImport("kernel32", SetLastError = true, CharSet = CharSet.Unicode)]
    private static extern IntPtr LoadLibraryEx(string path, IntPtr file, uint flags);
    [DllImport("kernel32", CharSet = CharSet.Ansi)]
    private static extern IntPtr GetProcAddress(IntPtr module, string name);
    [DllImport("kernel32", CharSet = CharSet.Unicode)]
    private static extern IntPtr GetModuleHandle(string name);
    [DllImport("kernel32")]
    private static extern bool VirtualProtect(IntPtr address, UIntPtr size, uint newProtect, out uint oldProtect);
    [DllImport("kernel32")]
    private static extern bool FlushInstructionCache(IntPtr process, IntPtr address, UIntPtr size);
    [DllImport("kernel32")]
    private static extern IntPtr GetCurrentProcess();

    [UnmanagedFunctionPointer(CallingConvention.Winapi, CharSet = CharSet.Unicode)]
    private delegate IntPtr LoadLibraryWFn(string path);

    private static LoadLibraryWFn s_hook;   // keeps the thunk alive
    private static string s_engine;

    [STAThread]
    private static int Main(string[] args)
    {
        if (args.Length < 2)
        {
            Console.WriteLine("usage: NetFxHost <engine folder | -> <png>");
            return 2;
        }
        if (args[0] != "-")
        {
            InstallHook(System.IO.Path.Combine(System.IO.Path.GetFullPath(args[0]), "wpfgfx_v0400.dll"));
        }
        return RunWpf(args[1]);
    }

    private static IntPtr HookLoadLibraryW(string path)
    {
        if (path != null && path.EndsWith("\\WPF\\wpfgfx_v0400.dll", StringComparison.OrdinalIgnoreCase))
        {
            Console.WriteLine("redirect: " + path);
            return LoadLibraryEx(s_engine, IntPtr.Zero, 0x8 /* LOAD_WITH_ALTERED_SEARCH_PATH */);
        }
        return LoadLibraryEx(path, IntPtr.Zero, 0);
    }

    private static void InstallHook(string engine)
    {
        s_engine = engine;
        s_hook = HookLoadLibraryW;
        IntPtr target = GetProcAddress(GetModuleHandle("kernelbase.dll"), "LoadLibraryW");
        IntPtr thunk = Marshal.GetFunctionPointerForDelegate(s_hook);
        byte[] patch;
        if (IntPtr.Size == 8)
        {
            patch = new byte[12];
            patch[0] = 0x48; patch[1] = 0xB8;                    // mov rax, imm64
            BitConverter.GetBytes(thunk.ToInt64()).CopyTo(patch, 2);
            patch[10] = 0xFF; patch[11] = 0xE0;                  // jmp rax
        }
        else
        {
            patch = new byte[7];
            patch[0] = 0xB8;                                     // mov eax, imm32
            BitConverter.GetBytes(thunk.ToInt32()).CopyTo(patch, 1);
            patch[5] = 0xFF; patch[6] = 0xE0;                    // jmp eax
        }
        VirtualProtect(target, (UIntPtr)patch.Length, 0x40 /* PAGE_EXECUTE_READWRITE */, out uint old);
        Marshal.Copy(patch, 0, target, patch.Length);
        VirtualProtect(target, (UIntPtr)patch.Length, old, out _);
        FlushInstructionCache(GetCurrentProcess(), target, (UIntPtr)patch.Length);
    }

    // Kept out of Main so that no WPF assembly loads before the hook is in place.
    [MethodImpl(MethodImplOptions.NoInlining)]
    private static int RunWpf(string png)
    {
        var app = new Application { ShutdownMode = ShutdownMode.OnExplicitShutdown };
        int exit = 0;
        app.DispatcherUnhandledException += (s, e) => { Console.WriteLine("UNHANDLED " + e.Exception); exit = 1; };
        app.Startup += async (s, e) =>
        {
            try
            {
                var window = BuildWindow();
                window.Show();
                await Task.Delay(1200);
                Report();
                Capture(window, png);
                window.Close();
            }
            catch (Exception ex)
            {
                Console.WriteLine("ERROR " + ex);
                exit = 1;
            }
            app.Shutdown(exit);
        };
        app.Run();
        return exit;
    }

    private static Window BuildWindow()
    {
        var canvas = new Canvas { Width = 480, Height = 320, Background = new LinearGradientBrush(Colors.MidnightBlue, Colors.Teal, 30) };
        void Add(UIElement e, double x, double y) { Canvas.SetLeft(e, x); Canvas.SetTop(e, y); canvas.Children.Add(e); }

        Add(new TextBlock { Text = ".NET Framework WPF scene", FontSize = 22, Foreground = Brushes.White }, 16, 12);
        Add(new Ellipse { Width = 120, Height = 120, Fill = new RadialGradientBrush(Colors.Gold, Colors.OrangeRed), Stroke = Brushes.White, StrokeThickness = 3 }, 24, 60);
        Add(new Rectangle { Width = 140, Height = 90, RadiusX = 14, RadiusY = 14, Fill = Brushes.SteelBlue, Effect = new DropShadowEffect { BlurRadius = 16, ShadowDepth = 6 } }, 180, 70);
        Add(new Border { Width = 120, Height = 60, Background = Brushes.IndianRed, CornerRadius = new CornerRadius(8), Effect = new BlurEffect { Radius = 6 } }, 340, 80);
        Add(new Button { Content = "Button", Width = 110, Height = 32, Focusable = false }, 24, 210);
        Add(new CheckBox { Content = "CheckBox", IsChecked = true, Foreground = Brushes.White, Focusable = false }, 160, 216);
        Add(new Slider { Width = 160, Value = 4, Focusable = false }, 280, 216);
        var chart = new Polyline { Stroke = Brushes.Lime, StrokeThickness = 1.5 };
        for (int i = 0; i <= 60; i++) chart.Points.Add(new Point(i * 7.5, 25 + 18 * Math.Sin(i * 0.4)));
        Add(chart, 16, 260);

        return new Window
        {
            Title = "NetFxHost", Content = canvas, SizeToContent = SizeToContent.WidthAndHeight, ResizeMode = ResizeMode.NoResize,
            WindowStartupLocation = WindowStartupLocation.CenterScreen, Topmost = true, ShowActivated = false,
        };
    }

    private static void Report()
    {
        Console.WriteLine("runtime: " + RuntimeInformation.FrameworkDescription + " " + (IntPtr.Size * 8) + "-bit");
        Console.WriteLine("Tier=" + (RenderCapability.Tier >> 16));
        foreach (ProcessModule m in Process.GetCurrentProcess().Modules)
        {
            if (m.ModuleName.StartsWith("wpfgfx", StringComparison.OrdinalIgnoreCase))
            {
                Console.WriteLine("module: " + m.FileName + " " + m.FileVersionInfo.FileVersion);
            }
        }
        IntPtr gfx = GetModuleHandle("wpfgfx_v0400.dll");
        bool dx11 = gfx != IntPtr.Zero && GetProcAddress(gfx, "WpfGfxDx11_GetInfo") != IntPtr.Zero;
        Console.WriteLine("engine: " + (dx11 ? "D3D11" : "stock"));
    }

    private static void Capture(Window window, string png)
    {
        var content = (FrameworkElement)window.Content;
        Matrix toDevice = PresentationSource.FromVisual(window).CompositionTarget.TransformToDevice;
        Point origin = content.PointToScreen(new Point(0, 0));
        int width = (int)Math.Round(content.ActualWidth * toDevice.M11);
        int height = (int)Math.Round(content.ActualHeight * toDevice.M22);
        using (var bitmap = new System.Drawing.Bitmap(width, height))
        {
            using (var g = System.Drawing.Graphics.FromImage(bitmap))
            {
                g.CopyFromScreen((int)origin.X, (int)origin.Y, 0, 0, bitmap.Size);
            }
            bitmap.Save(png, System.Drawing.Imaging.ImageFormat.Png);
        }
        Console.WriteLine("screenshot: " + png);
    }
}
