using System.Windows.Interop;
using WpfGfx.Dx11.Interop;

namespace WpfGfx.Dx11;

/// <summary>
/// A <see cref="D3DImage"/> whose back buffer is an <c>ID3D11Texture2D</c>. The usage protocol is
/// the ordinary D3DImage one (Lock, render, AddDirtyRect, Unlock); only the back buffer type
/// differs. Requirements for the texture: 2D, single array slice, DXGI_FORMAT_B8G8R8A8_UNORM or
/// B8G8R8X8_UNORM, D3D11_USAGE_DEFAULT. A texture that does not belong to the engine's device
/// (<see cref="Dx11Engine.GetDevice"/>) must be created with D3D11_RESOURCE_MISC_SHARED.
/// </summary>
public class D3D11Image : D3DImage
{
    public D3D11Image()
    {
    }

    public D3D11Image(double dpiX, double dpiY) : base(dpiX, dpiY)
    {
    }

    /// <summary>
    /// Sets an <c>ID3D11Texture2D</c> as the back buffer. Must be called between Lock and Unlock
    /// like <see cref="D3DImage.SetBackBuffer(D3DResourceType, IntPtr)"/>. Pass
    /// <see cref="IntPtr.Zero"/> to detach.
    /// </summary>
    /// <exception cref="NotSupportedException">The D3D11 engine is not loaded, so the pointer would
    /// be handed to an engine that expects IDirect3DSurface9.</exception>
    /// <summary>
    /// Shows a frame queue (D19): the engine presents frames the producer publishes with
    /// <see cref="Dx11FrameQueue.Present"/> without any Lock/Unlock per frame. Call between Lock and
    /// Unlock once, add a full dirty rect, and unlock; after that the UI thread is not involved.
    /// Pass null to detach. Re-attach the same queue after <see cref="D3DImage.IsFrontBufferAvailable"/>
    /// comes back (device loss) or when the image is recreated.
    /// </summary>
    public void SetFrameQueue(Dx11FrameQueue? queue)
    {
        if (queue != null)
        {
            Dx11Engine.EnsureAvailable();
        }

        SetBackBuffer(D3DResourceType.IDirect3DSurface9, queue?.NativePointer ?? IntPtr.Zero, false);
    }

    public void SetBackBuffer(IntPtr texture2D, bool enableSoftwareFallback = false)
    {
        if (texture2D != IntPtr.Zero)
        {
            Dx11Engine.EnsureAvailable();
        }

        SetBackBuffer(D3DResourceType.IDirect3DSurface9, texture2D, enableSoftwareFallback);
    }
}
