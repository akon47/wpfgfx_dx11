using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;

namespace SmokeApp;

/// <summary>Captures a window to a PNG, independent of z-order where the OS allows it.</summary>
internal static class ScreenCapture
{
    /// <summary>
    /// Captures the client area through DWM (PrintWindow with PW_CLIENTONLY | PW_RENDERFULLCONTENT),
    /// so overlapping windows never end up in the capture and the title bar (whose colors depend
    /// on focus) is left out. If DWM hands back nothing, the client rectangle is copied from the
    /// screen instead. Returns the method used.
    /// </summary>
    public static string Capture(Window window, string path)
    {
        IntPtr hwnd = new WindowInteropHelper(window).Handle;
        GetClientRect(hwnd, out RECT c);
        int w = c.Right - c.Left, h = c.Bottom - c.Top;
        string method = "printwindow";
        var bmp = new System.Drawing.Bitmap(w, h, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
        using (var g = System.Drawing.Graphics.FromImage(bmp))
        {
            IntPtr hdc = g.GetHdc();
            try { PrintWindow(hwnd, hdc, PW_CLIENTONLY | PW_RENDERFULLCONTENT); }
            finally { g.ReleaseHdc(hdc); }
        }

        if (IsBlank(bmp))
        {
            method = "screen";
            var origin = new POINT { X = 0, Y = 0 };
            ClientToScreen(hwnd, ref origin);
            using var g = System.Drawing.Graphics.FromImage(bmp);
            g.CopyFromScreen(origin.X, origin.Y, 0, 0, new System.Drawing.Size(w, h), System.Drawing.CopyPixelOperation.SourceCopy);
        }

        bmp.Save(path, System.Drawing.Imaging.ImageFormat.Png);
        bmp.Dispose();
        return method;
    }

    /// <summary>
    /// Copies the window's rectangle from the screen: the composed result of the window over
    /// whatever is behind it. Used for the layered window, which sits over the main window, so
    /// its per-pixel alpha shows up as blending against the main window's content.
    /// </summary>
    public static string CaptureFromScreen(Window window, string path)
    {
        IntPtr hwnd = new WindowInteropHelper(window).Handle;
        GetWindowRect(hwnd, out RECT r);
        int w = r.Right - r.Left, h = r.Bottom - r.Top;
        using var bmp = new System.Drawing.Bitmap(w, h, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
        using (var g = System.Drawing.Graphics.FromImage(bmp))
        {
            g.CopyFromScreen(r.Left, r.Top, 0, 0, new System.Drawing.Size(w, h), System.Drawing.CopyPixelOperation.SourceCopy);
        }
        bmp.Save(path, System.Drawing.Imaging.ImageFormat.Png);
        return "screen";
    }

    /// <summary>True when every sampled point of the window's rectangle belongs to that window.</summary>
    public static bool IsUnobscured(Window window)
    {
        IntPtr hwnd = new WindowInteropHelper(window).Handle;
        GetWindowRect(hwnd, out RECT r);
        for (int iy = 0; iy < 3; iy++)
        {
            for (int ix = 0; ix < 3; ix++)
            {
                var pt = new POINT { X = r.Left + (r.Right - r.Left) * (ix + 1) / 4, Y = r.Top + (r.Bottom - r.Top) * (iy + 1) / 4 };
                if (GetAncestor(WindowFromPoint(pt), GA_ROOT) != hwnd) return false;
            }
        }
        return true;
    }

    private static bool IsBlank(System.Drawing.Bitmap bmp)
    {
        var rect = new System.Drawing.Rectangle(0, 0, bmp.Width, bmp.Height);
        var data = bmp.LockBits(rect, System.Drawing.Imaging.ImageLockMode.ReadOnly, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
        try
        {
            var row = new byte[data.Stride];
            for (int y = 0; y < data.Height; y++)
            {
                Marshal.Copy(data.Scan0 + y * data.Stride, row, 0, data.Stride);
                for (int x = 0; x < data.Width * 4; x++)
                {
                    if (row[x] != 0) return false;
                }
            }
            return true;
        }
        finally
        {
            bmp.UnlockBits(data);
        }
    }

    private const uint PW_CLIENTONLY = 0x00000001;
    private const uint PW_RENDERFULLCONTENT = 0x00000002;

    [StructLayout(LayoutKind.Sequential)]
    private struct RECT { public int Left, Top, Right, Bottom; }

    [StructLayout(LayoutKind.Sequential)]
    private struct POINT { public int X, Y; }

    private const uint GA_ROOT = 2;

    [DllImport("user32.dll")]
    private static extern IntPtr WindowFromPoint(POINT pt);

    [DllImport("user32.dll")]
    private static extern IntPtr GetAncestor(IntPtr hwnd, uint flags);

    [DllImport("user32.dll")]
    private static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);

    [DllImport("user32.dll")]
    private static extern bool GetClientRect(IntPtr hWnd, out RECT lpRect);

    [DllImport("user32.dll")]
    private static extern bool ClientToScreen(IntPtr hWnd, ref POINT lpPoint);

    [DllImport("user32.dll")]
    private static extern bool PrintWindow(IntPtr hWnd, IntPtr hdcBlt, uint nFlags);
}
