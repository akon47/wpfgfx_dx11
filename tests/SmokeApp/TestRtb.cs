using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Effects;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using WpfGfx.Dx11;

namespace SmokeApp;

/// <summary>
/// RenderTargetBitmap test: renders a small visual tree (gradient, text, a ps_2_0 effect, a ps_3_0
/// effect, a drop shadow) into a RenderTargetBitmap and saves it as PNG. With --rtb-hw the
/// D3D11 engine's opt-in GPU path is enabled first (Dx11Engine.HardwareRenderTargetBitmap); the
/// stock engine renders it with the software rasterizer, where the ps_3_0 effect is replaced by a
/// pass-through (dotnet/wpf#7856).
/// </summary>
internal static class RtbTest
{
    public static string Render(string path, bool hardware)
    {
        if (hardware)
        {
            Dx11Engine.HardwareRenderTargetBitmap = true;   // throws NotSupportedException on the stock engine
        }

        var root = new Grid { Width = 320, Height = 240, Background = new SolidColorBrush(Color.FromRgb(0x20, 0x24, 0x28)) };
        root.Children.Add(new Rectangle
        {
            Width = 140, Height = 100, Margin = new Thickness(10), HorizontalAlignment = HorizontalAlignment.Left, VerticalAlignment = VerticalAlignment.Top,
            RadiusX = 12, RadiusY = 12,
            Fill = new LinearGradientBrush(Color.FromRgb(0xE0, 0x40, 0x30), Color.FromRgb(0x30, 0x80, 0xE0), 45),
            Effect = new DropShadowEffect { BlurRadius = 10, ShadowDepth = 4 },
        });
        root.Children.Add(new TextBlock
        {
            Text = "RenderTargetBitmap", Foreground = Brushes.White, FontSize = 20, FontWeight = FontWeights.Bold,
            Margin = new Thickness(160, 20, 0, 0), HorizontalAlignment = HorizontalAlignment.Left, VerticalAlignment = VerticalAlignment.Top,
        });
        root.Children.Add(new Ellipse
        {
            Width = 120, Height = 100, Margin = new Thickness(10, 125, 0, 0), HorizontalAlignment = HorizontalAlignment.Left, VerticalAlignment = VerticalAlignment.Top,
            Fill = new RadialGradientBrush(Colors.Orange, Colors.DarkRed),
            Effect = new TintEffect(),           // ps_2_0 bytecode
        });
        root.Children.Add(new Rectangle
        {
            Width = 150, Height = 100, Margin = new Thickness(160, 125, 0, 0), HorizontalAlignment = HorizontalAlignment.Left, VerticalAlignment = VerticalAlignment.Top,
            Fill = new LinearGradientBrush(Colors.LightGreen, Colors.DarkGreen, 90),
            Effect = new SplitBlurEffect(),      // ps_3_0 bytecode (loop/branch)
        });

        root.Measure(new Size(320, 240));
        root.Arrange(new Rect(0, 0, 320, 240));
        root.UpdateLayout();

        var rtb = new RenderTargetBitmap(320, 240, 96, 96, PixelFormats.Pbgra32);
        rtb.Render(root);

        var encoder = new PngBitmapEncoder();
        encoder.Frames.Add(BitmapFrame.Create(rtb));
        using (var fs = File.Create(path))
        {
            encoder.Save(fs);
        }

        return $"{rtb.PixelWidth}x{rtb.PixelHeight} hardware={hardware} file={path}";
    }
}
