using System;
using System.Collections.Generic;
using System.Globalization;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Shapes;

namespace SmokeApp;

/// <summary>
/// Small vector content where anti-aliasing quality shows (D36): path icons at 16-48 px, 1 px tooltip borders,
/// thin lines, shapes that touch (seams), and an aliased icon. --icons &lt;png&gt; captures it and prints one
/// "icons-region: name x y w h" line per region (client pixels) so the gate can compare region by region.
/// </summary>
internal static class TestIcons
{
    // A gear: outer teeth polygon plus a hole (even-odd), in a 24x24 design box.
    private static readonly Geometry s_gear = Geometry.Parse(
        "F0 M12,1 L13.6,1 L14.1,3.6 L15.9,4.3 L18.1,2.8 L19.2,3.9 L17.7,6.1 L18.4,7.9 L21,8.4 L21,10 L21,11.6 " +
        "L18.4,12.1 L17.7,13.9 L19.2,16.1 L18.1,17.2 L15.9,15.7 L14.1,16.4 L13.6,19 L12,19 L10.4,19 L9.9,16.4 " +
        "L8.1,15.7 L5.9,17.2 L4.8,16.1 L6.3,13.9 L5.6,12.1 L3,11.6 L3,10 L3,8.4 L5.6,7.9 L6.3,6.1 L4.8,3.9 " +
        "L5.9,2.8 L8.1,4.3 L9.9,3.6 L10.4,1 Z M12,6.5 A3.5,3.5 0 1 0 12.001,6.5 Z");

    public static async Task<string> RunAsync(Window owner, string path)
    {
        var canvas = new Canvas { Width = 640, Height = 300, Background = new SolidColorBrush(Color.FromRgb(0x20, 0x24, 0x28)) };
        var regions = new List<(string Name, Rect R)>();
        void Add(UIElement e, double x, double y) { Canvas.SetLeft(e, x); Canvas.SetTop(e, y); canvas.Children.Add(e); }

        // Icons (fill), fractional positions like real layouts.
        double ix = 12.5;
        foreach (int size in new[] { 16, 24, 32, 48 })
        {
            Add(new Path { Data = s_gear, Fill = Brushes.Gainsboro, Stretch = Stretch.Uniform, Width = size, Height = size }, ix, 10.5);
            regions.Add(("icon" + size, new Rect(Math.Floor(ix) - 1, 9, size + 3, size + 3)));
            ix += size + 18;
        }
        // Aliased icon: EdgeMode.Aliased must stay crisp.
        var aliased = new Path { Data = s_gear, Fill = Brushes.Gainsboro, Stretch = Stretch.Uniform, Width = 24, Height = 24 };
        RenderOptions.SetEdgeMode(aliased, EdgeMode.Aliased);
        Add(aliased, 230.5, 10.5);
        regions.Add(("aliased24", new Rect(229, 9, 27, 27)));

        // Tooltip borders: 1 px, rounded, uniform (pen path) at two sizes.
        Add(new Border { Width = 120, Height = 28, BorderThickness = new Thickness(1), CornerRadius = new CornerRadius(3),
                         BorderBrush = Brushes.LightGray, Background = new SolidColorBrush(Color.FromRgb(0x30, 0x34, 0x3a)) }, 12.5, 80.5);
        regions.Add(("tooltip120", new Rect(11, 79, 124, 32)));
        Add(new Border { Width = 260, Height = 40, BorderThickness = new Thickness(1), CornerRadius = new CornerRadius(4),
                         BorderBrush = Brushes.LightGray, Background = new SolidColorBrush(Color.FromRgb(0x30, 0x34, 0x3a)) }, 150.5, 80.5);
        regions.Add(("tooltip260", new Rect(149, 79, 264, 44)));

        // Thin lines: a long diagonal hairline and a polyline chart.
        Add(new Line { X1 = 0, Y1 = 0, X2 = 300, Y2 = 60, Stroke = Brushes.White, StrokeThickness = 1 }, 12.5, 135.5);
        regions.Add(("line1px", new Rect(11, 134, 304, 64)));
        var chart = new Polyline { Stroke = Brushes.Orange, StrokeThickness = 1.5 };
        for (int i = 0; i <= 40; i++) chart.Points.Add(new Point(i * 7.5, 30 + 25 * Math.Sin(i * 0.55)));
        Add(chart, 330.5, 130.5);
        regions.Add(("chart", new Rect(329, 129, 304, 64)));

        // Seams: a small square touching a large square at a fractional edge, and two large squares touching.
        Add(new Rectangle { Width = 40.3, Height = 40, Fill = Brushes.SteelBlue }, 12.4, 215.5);
        Add(new Rectangle { Width = 150, Height = 70, Fill = Brushes.SteelBlue }, 52.7, 200.5);
        regions.Add(("seamSmallLarge", new Rect(10, 199, 196, 74)));
        Add(new Rectangle { Width = 120.3, Height = 70, Fill = Brushes.IndianRed }, 230.4, 200.5);
        Add(new Rectangle { Width = 120, Height = 70, Fill = Brushes.IndianRed }, 350.7, 200.5);
        regions.Add(("seamLargeLarge", new Rect(228, 199, 246, 74)));

        var window = new Window
        {
            Title = "SmokeApp icons", Content = canvas, SizeToContent = SizeToContent.WidthAndHeight, ResizeMode = ResizeMode.NoResize,
            Left = owner.Left + 60, Top = owner.Top + 80, ShowInTaskbar = false,
        };
        window.Show();
        await Task.Delay(700);
        string shot = ScreenCapture.Capture(window, path);
        foreach (var (name, r) in regions)
        {
            Console.WriteLine(string.Format(CultureInfo.InvariantCulture, "icons-region: {0} {1} {2} {3} {4}", name, (int)r.X, (int)r.Y, (int)r.Width, (int)r.Height));
        }
        window.Close();
        return path + " via " + shot;
    }
}
