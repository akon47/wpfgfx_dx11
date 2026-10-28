using System.Windows;
using System.Windows.Controls;

namespace SmokeApp;

public partial class MediaWindow : Window
{
    public string Status { get; private set; } = "not opened";

    public MediaWindow(string file)
    {
        InitializeComponent();
        Player.Source = new Uri(System.IO.Path.GetFullPath(file));
        Player.Play();
    }

    private void OnMediaOpened(object sender, RoutedEventArgs e)
    {
        Status = $"opened {Player.NaturalVideoWidth}x{Player.NaturalVideoHeight} hasVideo={Player.HasVideo}";
        Console.WriteLine("media: " + Status);
    }

    private void OnMediaFailed(object? sender, ExceptionRoutedEventArgs e)
    {
        Status = "failed: " + e.ErrorException.Message;
        Console.WriteLine("media: " + Status);
    }
}
