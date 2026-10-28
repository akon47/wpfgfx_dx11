using System.Diagnostics;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Effects;
using System.Windows.Shapes;

namespace SmokeApp;

/// <summary>
/// Throughput benchmark: a scene heavy enough that the render thread, not the UI thread, bounds
/// the frame rate (many gradient shapes, text, and blur effects, all animated every frame).
/// WPF schedules the next UI frame from the compositor's presented notification, so the measured
/// frame rate follows the slower of the two threads; CPU time per frame shows the engine's CPU
/// overhead. Same scene on the stock and the D3D11 engine gives a like-for-like comparison.
/// </summary>
public class BenchWindow : Window
{
    private readonly Canvas _canvas = new();
    private readonly List<(FrameworkElement element, RotateTransform rotate, double speed, double dx)> _items = new();
    private readonly TaskCompletionSource<string> _done = new();
    private readonly int _seconds;
    private readonly Stopwatch _clock = new();
    private TimeSpan _cpuStart;
    private int _frames;
    private double _lastRenderTime = -1;

    public BenchWindow(int seconds, bool transparent = false)
    {
        _seconds = seconds;
        Title = "SmokeApp bench";
        Width = 900;
        Height = 700;
        ResizeMode = ResizeMode.NoResize;
        ShowInTaskbar = false;
        Background = new SolidColorBrush(Color.FromRgb(0x20, 0x24, 0x28));
        if (transparent)
        {
            // Layered window: the engine reads the frame back and calls UpdateLayeredWindow.
            AllowsTransparency = true;
            WindowStyle = WindowStyle.None;
            Background = new SolidColorBrush(Color.FromArgb(0xC0, 0x20, 0x24, 0x28));
        }
        Content = _canvas;
        var rnd = new Random(12345);

        // 1200 gradient rectangles, 300 text labels, 120 radial ellipses, 60 blurred panels.
        for (int i = 0; i < 1200; i++) Add(MakeRect(rnd), rnd);
        for (int i = 0; i < 300; i++) Add(MakeText(rnd, i), rnd);
        for (int i = 0; i < 120; i++) Add(MakeEllipse(rnd), rnd);
        for (int i = 0; i < 60; i++) Add(MakeBlurred(rnd), rnd);

        Loaded += (_, _) =>
        {
            _cpuStart = Process.GetCurrentProcess().TotalProcessorTime;
            _clock.Start();
            CompositionTarget.Rendering += OnRendering;
        };
    }

    public Task<string> Completion => _done.Task;

    private void Add(FrameworkElement e, Random rnd)
    {
        var rotate = new RotateTransform(rnd.Next(360));
        e.RenderTransformOrigin = new Point(0.5, 0.5);
        e.RenderTransform = rotate;
        Canvas.SetLeft(e, rnd.NextDouble() * 860);
        Canvas.SetTop(e, rnd.NextDouble() * 640);
        _canvas.Children.Add(e);
        _items.Add((e, rotate, rnd.NextDouble() * 4 - 2, rnd.NextDouble() * 2 - 1));
    }

    private static Brush Gradient(Random rnd) => new LinearGradientBrush(
        Color.FromArgb(0xC0, (byte)rnd.Next(256), (byte)rnd.Next(256), (byte)rnd.Next(256)),
        Color.FromArgb(0xC0, (byte)rnd.Next(256), (byte)rnd.Next(256), (byte)rnd.Next(256)), rnd.Next(360));

    private static Rectangle MakeRect(Random rnd) => new()
    {
        Width = 20 + rnd.Next(40), Height = 20 + rnd.Next(40), RadiusX = 4, RadiusY = 4,
        Fill = Gradient(rnd), Stroke = Brushes.White, StrokeThickness = 1,
    };

    private static TextBlock MakeText(Random rnd, int i) => new()
    {
        Text = "WPF DX11 " + i, FontSize = 10 + rnd.Next(14), Foreground = Brushes.White,
    };

    private static Ellipse MakeEllipse(Random rnd) => new()
    {
        Width = 30 + rnd.Next(60), Height = 30 + rnd.Next(60),
        Fill = new RadialGradientBrush(Color.FromArgb(0xFF, (byte)rnd.Next(256), (byte)rnd.Next(256), (byte)rnd.Next(256)), Colors.Transparent),
    };

    private static Border MakeBlurred(Random rnd) => new()
    {
        Width = 60 + rnd.Next(60), Height = 40 + rnd.Next(40), CornerRadius = new CornerRadius(8),
        Background = Gradient(rnd), Effect = new BlurEffect { Radius = 6 + rnd.Next(10) },
    };

    private void OnRendering(object? sender, EventArgs e)
    {
        // Advance only once per compositor frame (Rendering can fire twice with the same time).
        double t = ((RenderingEventArgs)e).RenderingTime.TotalSeconds;
        if (t == _lastRenderTime) return;
        _lastRenderTime = t;
        _frames++;

        foreach (var (element, rotate, speed, dx) in _items)
        {
            rotate.Angle += speed;
            double left = Canvas.GetLeft(element) + dx;
            if (left < -80) left = 900; else if (left > 900) left = -80;
            Canvas.SetLeft(element, left);
        }

        if (_clock.Elapsed.TotalSeconds >= _seconds)
        {
            CompositionTarget.Rendering -= OnRendering;
            _clock.Stop();
            var proc = Process.GetCurrentProcess();
            double cpuMs = (proc.TotalProcessorTime - _cpuStart).TotalMilliseconds;
            double fps = _frames / _clock.Elapsed.TotalSeconds;
            _done.TrySetResult(string.Format(System.Globalization.CultureInfo.InvariantCulture,
                "frames={0} seconds={1:F1} fps={2:F1} cpuMsPerFrame={3:F2} privateMB={4:F0}",
                _frames, _clock.Elapsed.TotalSeconds, fps, cpuMs / Math.Max(1, _frames), proc.PrivateMemorySize64 / 1048576.0));
        }
    }
}
