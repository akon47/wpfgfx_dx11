using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Threading;

namespace SmokeApp;

/// <summary>
/// Repro scenarios for known dotnet/wpf rendering-engine bugs. Each one reports whether the render thread
/// survived ("ok") or the app saw UCEERR_RENDERTHREADFAILURE. The stock engine is expected to fail some of
/// them; the D3D11 engine should not.
///   --bug-nan                 dotnet/wpf#7421: a clip geometry built from a NaN rectangle
///   --bug-aliased [points]    dotnet/wpf#3501: a dense aliased polyline that moves sideways
///   --bug-bitmapcache [n]     dotnet/wpf#8031: n elements with CacheMode = BitmapCache (default 6000)
///   --bug-maximize [n]        D35: a WindowChrome dialog maximized/restored n times (default 12); maximized windows
///                             are larger than the monitor (8 px border outside the screen). A watchdog reports a
///                             hung UI thread instead of hanging.
/// </summary>
internal static class TestBugs
{
    private static string? s_failure;

    public static async Task<string> RunAsync(Window owner, string bug, string? arg)
    {
        s_failure = null;
        DispatcherUnhandledExceptionEventHandler handler = (_, e) =>
        {
            s_failure ??= e.Exception.GetType().Name + ": " + e.Exception.Message.Split('\n')[0].Trim();
            e.Handled = true;
        };
        Application.Current.DispatcherUnhandledException += handler;

        var window = new Window
        {
            Title = "SmokeApp bug " + bug, Width = 900, Height = 520, Left = owner.Left + 40, Top = owner.Top + 60,
            ShowInTaskbar = false, Background = Brushes.White,
        };
        string detail;
        try
        {
            window.Show();
            detail = bug switch
            {
                "nan" => await RunNaNAsync(window),
                "aliased" => await RunAliasedAsync(window, arg),
                "bitmapcache" => await RunBitmapCacheAsync(window, arg),
                "maximize" => await RunMaximizeAsync(window, arg),
                "spinblur" => await RunSpinBlurAsync(window, arg),
                _ => "unknown bug scenario",
            };
            // One more frame so a failure in the last pass is seen.
            await Flush(window, 300);
        }
        catch (Exception ex)
        {
            s_failure ??= ex.GetType().Name + ": " + ex.Message.Split('\n')[0].Trim();
            detail = "threw";
        }
        finally
        {
            try { window.Close(); } catch (Exception ex) { s_failure ??= ex.GetType().Name + ": " + ex.Message.Split('\n')[0].Trim(); }
            Application.Current.DispatcherUnhandledException -= handler;
        }
        return string.Format(CultureInfo.InvariantCulture, "{0} survived={1} {2}{3}",
            bug, s_failure == null ? "true" : "false", detail, s_failure == null ? "" : " failure=" + s_failure);
    }

    private static async Task Flush(Window window, int ms)
    {
        await Task.Delay(ms);
        await window.Dispatcher.InvokeAsync(() => window.UpdateLayout(), DispatcherPriority.Render);
    }

    // ---------------------------------------------------------------- #7421
    private sealed class NaNElement : FrameworkElement
    {
        protected override void OnRender(DrawingContext dc)
        {
            dc.DrawRectangle(Brushes.SteelBlue, null, new Rect(20, 20, 200, 120));   // something valid to see
            var rect = new Rect(0, double.NaN, 36, 144);
            var outer = RoundedRect(rect, 18);
            var inner = RoundedRect(new Rect(rect.Left + 1, rect.Top + 1, Math.Max(0, rect.Width - 2), Math.Max(0, rect.Height - 2)), 17);
            dc.PushClip(new CombinedGeometry(GeometryCombineMode.Exclude, outer, inner));
            dc.DrawRectangle(Brushes.Red, null, new Rect(0, 0, 300, 300));
            dc.Pop();
            dc.DrawGeometry(Brushes.Orange, new Pen(Brushes.Black, 1), outer);
        }

        private static PathGeometry RoundedRect(Rect r, double cr)
        {
            var f = new PathFigure { StartPoint = new Point(r.Left + cr, r.Top), IsClosed = true };
            void Arc(double x, double y) => f.Segments.Add(new ArcSegment(new Point(x, y), new Size(cr, cr), 0, false, SweepDirection.Clockwise, true));
            void Line(double x, double y) => f.Segments.Add(new LineSegment(new Point(x, y), true));
            Line(r.Right - cr, r.Top); Arc(r.Right, r.Top + cr);
            Line(r.Right, r.Bottom - cr); Arc(r.Right - cr, r.Bottom);
            Line(r.Left + cr, r.Bottom); Arc(r.Left, r.Bottom - cr);
            Line(r.Left, r.Top + cr); Arc(r.Left + cr, r.Top);
            var g = new PathGeometry(); g.Figures.Add(f); g.Freeze();
            return g;
        }
    }

    private static async Task<string> RunNaNAsync(Window window)
    {
        window.Content = new NaNElement();
        await Flush(window, 600);
        // Move the window so the target is resized/updated again (the original report fails there).
        window.Width += 40;
        await Flush(window, 400);
        return "frames rendered";
    }

    // ---------------------------------------------------------------- #3501
    private sealed class PolylineElement : FrameworkElement
    {
        private readonly List<Point> _points;
        private readonly Pen _pen = new(Brushes.Red, 1.0);
        public PolylineElement(List<Point> points) { _points = points; _pen.Freeze(); RenderOptions.SetEdgeMode(this, EdgeMode.Aliased); }
        protected override void OnRender(DrawingContext dc)
        {
            var g = new StreamGeometry { FillRule = FillRule.Nonzero };
            using (var c = g.Open())
            {
                c.BeginFigure(_points[0], true, false);
                for (int i = 1; i < _points.Count; i++) c.LineTo(_points[i], true, false);
            }
            g.Freeze();
            dc.DrawGeometry(null, _pen, g);
        }
    }

    private static List<Point> AliasedData(string? file)
    {
        if (file != null && File.Exists(file))
        {
            return File.ReadAllLines(file).Where(l => l.Trim().Length > 0)
                .Select(l => l.Split(' ', StringSplitOptions.RemoveEmptyEntries))
                .Select(p => new Point(double.Parse(p[0], CultureInfo.InvariantCulture), double.Parse(p[1], CultureInfo.InvariantCulture)))
                .ToList();
        }
        // Synthetic data with the same character as the report: integer pixel coordinates, x advancing
        // slowly, y jumping up and down with repeated points, so segments stack on top of each other.
        var rnd = new Random(3501);
        var pts = new List<Point>();
        double x = 31, y = 30;
        for (int i = 0; i < 900; i++)
        {
            if (rnd.Next(3) == 0) x += 1;
            int r = rnd.Next(10);
            y = r < 2 ? y : r < 6 ? Math.Clamp(y + rnd.Next(-3, 4), 0, 50) : rnd.Next(0, 51);
            pts.Add(new Point(x, y));
        }
        return pts;
    }

    private static async Task<string> RunAliasedAsync(Window window, string? file)
    {
        var points = AliasedData(file);
        var element = new PolylineElement(points) { HorizontalAlignment = HorizontalAlignment.Left };
        var host = new Grid();
        host.Children.Add(element);
        window.Content = host;
        await Flush(window, 300);
        // Drag-the-splitter equivalent: shift the element sideways in whole and half pixels.
        int steps = 0;
        for (double dx = 0; dx <= 60 && s_failure == null; dx += 0.5)
        {
            element.Margin = new Thickness(dx, 0, 0, 0);
            element.Width = 800 - dx;
            await Flush(window, 16);
            steps++;
        }
        return string.Format(CultureInfo.InvariantCulture, "points={0} steps={1}", points.Count, steps);
    }

    // ---------------------------------------------------------------- #8031
    private static async Task<string> RunBitmapCacheAsync(Window window, string? arg)
    {
        int n = arg != null && int.TryParse(arg, out int v) ? v : 6000;
        var panel = new WrapPanel();
        for (int i = 0; i < n; i++)
        {
            panel.Children.Add(new Border
            {
                Width = 4, Height = 4, Margin = new Thickness(1),
                Background = new SolidColorBrush(Color.FromRgb((byte)(i * 7), (byte)(i * 13), (byte)(i * 29))),
                CacheMode = new BitmapCache(),
            });
        }
        window.Content = panel;   // all on screen: every cache gets a render target
        await Flush(window, 1500);
        window.Width += 30;   // the report fails on the next window update
        await Flush(window, 800);
        return string.Format(CultureInfo.InvariantCulture, "elements={0} gdiObjects={1}", n, GetGuiResources(System.Diagnostics.Process.GetCurrentProcess().Handle, 0));
    }

    // ---------------------------------------------------------------- D35 maximize / restore
    private static async Task<string> RunMaximizeAsync(Window window, string? arg)
    {
        int n = arg != null && int.TryParse(arg, out int v) ? v : 12;
        window.Width = 980; window.Height = 500;
        System.Windows.Shell.WindowChrome.SetWindowChrome(window, new System.Windows.Shell.WindowChrome
        {
            CaptionHeight = 32, ResizeBorderThickness = new Thickness(6), GlassFrameThickness = new Thickness(0),
        });
        var grid = new Grid { Background = new SolidColorBrush(Color.FromRgb(0x20, 0x24, 0x28)) };
        for (int i = 0; i < 24; i++)
        {
            grid.Children.Add(new Border
            {
                Width = 120 + i * 37 % 300, Height = 40 + i * 13 % 90, CornerRadius = new CornerRadius(6),
                HorizontalAlignment = HorizontalAlignment.Left, VerticalAlignment = VerticalAlignment.Top,
                Margin = new Thickness(i * 53 % 900, i * 29 % 420, 0, 0),
                Background = new SolidColorBrush(Color.FromRgb((byte)(40 + i * 9), (byte)(90 + i * 5), 200)),
                Effect = i % 3 == 0 ? new System.Windows.Media.Effects.DropShadowEffect { BlurRadius = 12, ShadowDepth = 3 } : null,
            });
        }
        grid.Children.Add(new TextBlock { Text = "maximize / restore", Foreground = Brushes.White, FontSize = 20, Margin = new Thickness(12, 6, 0, 0) });
        window.Content = grid;
        await Flush(window, 400);

        // Watchdog: if the UI thread stops pumping (render thread died while the UI waits on it), report and exit.
        long lastTick = Environment.TickCount64;
        var watchdog = new System.Threading.Thread(() =>
        {
            while (true)
            {
                System.Threading.Thread.Sleep(500);
                if (Environment.TickCount64 - System.Threading.Interlocked.Read(ref lastTick) > 8000)
                {
                    Console.WriteLine("bug: maximize survived=false hung UI thread for 8 s");
                    Console.Out.Flush();
                    Environment.Exit(3);
                }
            }
        }) { IsBackground = true };
        watchdog.Start();

        double maxW = 0, maxH = 0;
        for (int i = 0; i < n && s_failure == null; i++)
        {
            window.WindowState = WindowState.Maximized;
            await Flush(window, 250);
            System.Threading.Interlocked.Exchange(ref lastTick, Environment.TickCount64);
            var src = (System.Windows.Interop.HwndSource?)PresentationSource.FromVisual(window);
            if (src != null && GetWindowRect(src.Handle, out RECTW r)) { maxW = r.Right - r.Left; maxH = r.Bottom - r.Top; }
            window.WindowState = WindowState.Normal;
            await Flush(window, 250);
            System.Threading.Interlocked.Exchange(ref lastTick, Environment.TickCount64);
        }
        System.Threading.Interlocked.Exchange(ref lastTick, long.MaxValue / 2);
        return string.Format(CultureInfo.InvariantCulture, "cycles={0} maximized={1}x{2}", n, maxW, maxH);
    }

    // ---------------------------------------------------------------- blurred rotating element jitter
    // A rectangle with BlurEffect rotates in place. Its blurred body's centroid must stay put; frame-to-frame
    // wobble shows up as deviation (the bench's "shaking" panels).
    private static async Task<string> RunSpinBlurAsync(Window window, string? arg)
    {
        int frames = arg != null && int.TryParse(arg, out int v) ? v : 40;
        window.Width = 420; window.Height = 420;
        var rotate = new RotateTransform(0);
        var rect = new Border
        {
            Width = 140, Height = 70, CornerRadius = new CornerRadius(8),
            Background = new SolidColorBrush(Color.FromRgb(0x30, 0x90, 0xF0)),
            Effect = new System.Windows.Media.Effects.BlurEffect { Radius = 10 },
            RenderTransformOrigin = new Point(0.5, 0.5), RenderTransform = rotate,
            HorizontalAlignment = HorizontalAlignment.Center, VerticalAlignment = VerticalAlignment.Center,
        };
        window.Content = new Grid { Background = Brushes.White, Children = { rect } };
        await Flush(window, 300);
        var pts = new List<(double x, double y)>();
        for (int f = 0; f < frames; f++)
        {
            rotate.Angle = f * 7.3;
            await Flush(window, 40);
            using var bmp = ScreenCapture.CaptureBitmap(window);
            double sx = 0, sy = 0, sw = 0;
            for (int y = 0; y < bmp.Height; y += 1)
            {
                for (int x = 0; x < bmp.Width; x += 1)
                {
                    var c = bmp.GetPixel(x, y);
                    double w = Math.Max(0, 255 - c.R) / 255.0;   // blue body darkens red; weight by coverage
                    if (w > 0.02) { sx += x * w; sy += y * w; sw += w; }
                }
            }
            if (sw > 0) pts.Add((sx / sw, sy / sw));
        }
        double mx = pts.Average(p => p.x), my = pts.Average(p => p.y);
        double dev = pts.Max(p => Math.Sqrt((p.x - mx) * (p.x - mx) + (p.y - my) * (p.y - my)));
        return string.Format(CultureInfo.InvariantCulture, "frames={0} centroidMaxDev={1:F2}px", pts.Count, dev);
    }

    private struct RECTW { public int Left, Top, Right, Bottom; }

    [System.Runtime.InteropServices.DllImport("user32.dll")]
    private static extern bool GetWindowRect(IntPtr hWnd, out RECTW lpRect);

    [System.Runtime.InteropServices.DllImport("user32.dll")]
    private static extern uint GetGuiResources(IntPtr hProcess, uint uiFlags);
}
