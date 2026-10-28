using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Interop;
using System.Windows.Media;
using WpfGfx.Dx11;

namespace SmokeApp;

/// <summary>
/// D3DImage test cells. Both talk to Direct3D through raw vtable calls so the test app needs no
/// extra packages. Cell 15 uses the public D3DImage contract (IDirect3DSurface9 from a D3D9Ex
/// device), which the DX11 engine must keep working. Cell 16 hands an ID3D11Texture2D to
/// D3DImage through the companion assembly; on the stock engine it shows a notice instead.
/// </summary>
internal static class InteropCells
{
    private const uint Width = 160, Height = 120;

    public static void Populate(Image d3d9Cell, Image d3d11Cell, TextBlock d3d11Notice, IntPtr hwnd)
    {
        try
        {
            d3d9Cell.Source = CreateD3D9ExImage(hwnd);
        }
        catch (Exception e)
        {
            Console.WriteLine("D3D9Ex cell failed: " + e.Message);
        }

        if (Dx11Engine.IsAvailable)
        {
            try
            {
                var info = Dx11Engine.GetInfo();
                Console.WriteLine($"Dx11Engine: abi={info.AbiVersion} featureLevel=0x{info.FeatureLevel:x} displays={info.DisplayCount}");
                d3d11Cell.Source = CreateD3D11Image();
                d3d11Notice.Visibility = Visibility.Collapsed;
            }
            catch (Exception e)
            {
                Console.WriteLine("D3D11 cell failed: " + e.Message);
            }
        }
        else
        {
            d3d11Notice.Text = "D3D11 engine not loaded";
        }
    }

    /// <summary>
    /// The D3DImage contract after a device loss: the front buffer goes away, and when it comes
    /// back the app must set its back buffer again. The simulated loss (WPFGFX_DX11_TEST_DEVICE_LOSS)
    /// leaves the app's own device intact, so the same surface is set again.
    /// </summary>
    private static void OnFrontBufferAvailableChanged(D3DImage image, string name, Action setBackBuffer)
    {
        Console.WriteLine($"{name} D3DImage front buffer available={image.IsFrontBufferAvailable}");
        if (image.IsFrontBufferAvailable)
        {
            image.Lock();
            setBackBuffer();
            image.AddDirtyRect(new Int32Rect(0, 0, (int)Width, (int)Height));
            image.Unlock();
        }
    }

    // ------------------------------------------------------------------ pattern

    /// <summary>Four quadrants plus a diagonal, as premultiplied BGRA.</summary>
    private static byte[] MakePattern(uint w, uint h)
    {
        var px = new byte[w * h * 4];
        for (uint y = 0; y < h; y++)
        {
            for (uint x = 0; x < w; x++)
            {
                uint i = (y * w + x) * 4;
                bool right = x >= w / 2, bottom = y >= h / 2;
                byte b = 0, g = 0, r = 0;
                if (!right && !bottom) { r = 220; g = 60; b = 40; }      // red
                else if (right && !bottom) { r = 40; g = 180; b = 60; }  // green
                else if (!right && bottom) { r = 50; g = 90; b = 230; }  // blue
                else { r = 230; g = 200; b = 40; }                       // yellow
                if (Math.Abs((int)x * (int)h - (int)y * (int)w) < 3 * (int)w) { r = g = b = 255; }
                px[i] = b; px[i + 1] = g; px[i + 2] = r; px[i + 3] = 255;
            }
        }
        return px;
    }

    // ------------------------------------------------------------------ D3D9Ex (public contract)

    [DllImport("d3d9.dll", ExactSpelling = true)]
    private static extern int Direct3DCreate9Ex(uint sdkVersion, out IntPtr d3d9ex);

    [StructLayout(LayoutKind.Sequential)]
    private struct D3DPRESENT_PARAMETERS
    {
        public uint BackBufferWidth, BackBufferHeight, BackBufferFormat, BackBufferCount;
        public uint MultiSampleType, MultiSampleQuality, SwapEffect;
        public IntPtr hDeviceWindow;
        public int Windowed, EnableAutoDepthStencil;
        public uint AutoDepthStencilFormat, Flags, FullScreen_RefreshRateInHz, PresentationInterval;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct RECT { public int Left, Top, Right, Bottom; }

    private delegate int CreateDeviceExFn(IntPtr self, uint adapter, uint deviceType, IntPtr hFocusWindow, uint behaviorFlags,
        ref D3DPRESENT_PARAMETERS pp, IntPtr fullscreenMode, out IntPtr device);
    private delegate int CreateRenderTargetFn(IntPtr self, uint width, uint height, uint format, uint multiSample, uint quality,
        int lockable, out IntPtr surface, IntPtr sharedHandle);
    private delegate int ColorFillFn(IntPtr self, IntPtr surface, ref RECT rect, uint color);

    private static T Method<T>(IntPtr obj, int slot) where T : Delegate =>
        Marshal.GetDelegateForFunctionPointer<T>(Marshal.ReadIntPtr(Marshal.ReadIntPtr(obj), slot * IntPtr.Size));

    private static void Check(int hr, string what)
    {
        if (hr < 0) throw new InvalidOperationException($"{what} failed 0x{hr:x8}");
    }

    private static D3DImage CreateD3D9ExImage(IntPtr hwnd)
    {
        Check(Direct3DCreate9Ex(32 /* D3D_SDK_VERSION */, out IntPtr d3d), "Direct3DCreate9Ex");

        var pp = new D3DPRESENT_PARAMETERS
        {
            BackBufferWidth = 1, BackBufferHeight = 1, BackBufferCount = 1,
            SwapEffect = 1 /* DISCARD */, hDeviceWindow = hwnd, Windowed = 1,
        };
        // IDirect3D9Ex::CreateDeviceEx is vtable slot 20. HAL = 1. HARDWARE_VERTEXPROCESSING | MULTITHREADED | FPU_PRESERVE.
        Check(Method<CreateDeviceExFn>(d3d, 20)(d3d, 0, 1, hwnd, 0x40 | 0x4 | 0x2, ref pp, IntPtr.Zero, out IntPtr device), "CreateDeviceEx");

        // IDirect3DDevice9::CreateRenderTarget is slot 28: A8R8G8B8 (21), no multisampling, not lockable.
        Check(Method<CreateRenderTargetFn>(device, 28)(device, Width, Height, 21, 0, 0, 0, out IntPtr surface, IntPtr.Zero), "CreateRenderTarget");

        // IDirect3DDevice9::ColorFill is slot 35: paint the quadrants.
        var fill = Method<ColorFillFn>(device, 35);
        uint Argb(byte r, byte g, byte b) => 0xFF000000u | ((uint)r << 16) | ((uint)g << 8) | b;
        int hw = (int)Width / 2, hh = (int)Height / 2;
        var r1 = new RECT { Left = 0, Top = 0, Right = hw, Bottom = hh };
        var r2 = new RECT { Left = hw, Top = 0, Right = (int)Width, Bottom = hh };
        var r3 = new RECT { Left = 0, Top = hh, Right = hw, Bottom = (int)Height };
        var r4 = new RECT { Left = hw, Top = hh, Right = (int)Width, Bottom = (int)Height };
        Check(fill(device, surface, ref r1, Argb(220, 60, 40)), "ColorFill");
        Check(fill(device, surface, ref r2, Argb(40, 180, 60)), "ColorFill");
        Check(fill(device, surface, ref r3, Argb(50, 90, 230)), "ColorFill");
        Check(fill(device, surface, ref r4, Argb(230, 200, 40)), "ColorFill");

        var image = new D3DImage();
        image.Lock();
        image.SetBackBuffer(D3DResourceType.IDirect3DSurface9, surface);
        image.AddDirtyRect(new Int32Rect(0, 0, (int)Width, (int)Height));
        image.Unlock();
        image.IsFrontBufferAvailableChanged += (_, _) =>
            OnFrontBufferAvailableChanged(image, "D3D9Ex", () => image.SetBackBuffer(D3DResourceType.IDirect3DSurface9, surface));
        // The device and surface are intentionally kept alive for the life of the process.
        return image;
    }

    // ------------------------------------------------------------------ D3D11 (companion assembly)

    [DllImport("d3d11.dll", ExactSpelling = true)]
    private static extern int D3D11CreateDevice(IntPtr adapter, uint driverType, IntPtr software, uint flags,
        IntPtr featureLevels, uint featureLevelCount, uint sdkVersion, out IntPtr device, out uint featureLevel, out IntPtr context);

    [StructLayout(LayoutKind.Sequential)]
    private struct D3D11_TEXTURE2D_DESC
    {
        public uint Width, Height, MipLevels, ArraySize, Format;
        public uint SampleCount, SampleQuality;
        public uint Usage, BindFlags, CPUAccessFlags, MiscFlags;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct D3D11_SUBRESOURCE_DATA { public IntPtr pSysMem; public uint SysMemPitch, SysMemSlicePitch; }

    private delegate int CreateTexture2DFn(IntPtr self, ref D3D11_TEXTURE2D_DESC desc, ref D3D11_SUBRESOURCE_DATA data, out IntPtr texture);

    private static D3D11Image CreateD3D11Image()
    {
        // A device of our own: the texture is created shared so the engine can open it (the
        // cross-device path). D3D_DRIVER_TYPE_HARDWARE = 1, D3D11_CREATE_DEVICE_BGRA_SUPPORT = 0x20, SDK version 7.
        Check(D3D11CreateDevice(IntPtr.Zero, 1, IntPtr.Zero, 0x20, IntPtr.Zero, 0, 7, out IntPtr device, out _, out _), "D3D11CreateDevice");

        byte[] pixels = MakePattern(Width, Height);
        GCHandle pinned = GCHandle.Alloc(pixels, GCHandleType.Pinned);
        try
        {
            var desc = new D3D11_TEXTURE2D_DESC
            {
                Width = Width, Height = Height, MipLevels = 1, ArraySize = 1,
                Format = 87 /* DXGI_FORMAT_B8G8R8A8_UNORM */, SampleCount = 1, SampleQuality = 0,
                Usage = 0 /* DEFAULT */, BindFlags = 0x8 | 0x20 /* SHADER_RESOURCE | RENDER_TARGET */,
                CPUAccessFlags = 0, MiscFlags = 0x2 /* D3D11_RESOURCE_MISC_SHARED */,
            };
            var data = new D3D11_SUBRESOURCE_DATA { pSysMem = pinned.AddrOfPinnedObject(), SysMemPitch = Width * 4 };
            // ID3D11Device::CreateTexture2D is vtable slot 5.
            Check(Method<CreateTexture2DFn>(device, 5)(device, ref desc, ref data, out IntPtr texture), "CreateTexture2D");

            var image = new D3D11Image();
            image.Lock();
            image.SetBackBuffer(texture);
            image.AddDirtyRect(new Int32Rect(0, 0, (int)Width, (int)Height));
            image.Unlock();
            image.IsFrontBufferAvailableChanged += (_, _) =>
                OnFrontBufferAvailableChanged(image, "D3D11", () => image.SetBackBuffer(texture));
            return image;
        }
        finally
        {
            pinned.Free();
        }
    }
}
