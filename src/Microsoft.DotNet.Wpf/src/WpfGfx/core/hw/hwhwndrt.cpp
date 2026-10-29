// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//

//
//  $TAG ENGR

//      $Module:    win_mil_graphics_targets
//      $Keywords:
//
//  $Description:
//      CHwHWNDRenderTarget implementation
//
//      This object creates the d3d device for the current hwnd and manages a
//      flipping chain.  Note that instead of using the primary flipping chain,
//      a secondary chain is created and used since secondary chains can be
//      resized without recreating the d3d device.
//
//      This object is also repsonsible for responding to resize and disabling
//      rendering when the hwnd is minimized.
//
//  $ENDTAG
//
//------------------------------------------------------------------------------

#include "precomp.hpp"

MtDefine(CHwHWNDRenderTarget, MILRender, "CHwHWNDRenderTarget");


//+-----------------------------------------------------------------------------
//
//  Member:
//      CHwHWNDRenderTarget::CHwHWNDRenderTarget
//
//  Synopsis:
//      ctor
//
//------------------------------------------------------------------------------

CHwHWNDRenderTarget::CHwHWNDRenderTarget(
    __inout_ecount(1) CD3DDeviceLevel1 *pD3DDevice,
    __in_ecount(1) MilSwapChainParams const &D3DPresentParams,
    UINT AdapterOrdinalInGroup,
    DisplayId associatedDisplay,
    MilWindowLayerType::Enum eWindowLayerType
    ) :
    CHwDisplayRenderTarget(pD3DDevice, D3DPresentParams, AdapterOrdinalInGroup, associatedDisplay),
    m_eWindowLayerType(eWindowLayerType)
{
}


//+-----------------------------------------------------------------------------
//
//  Member:
//      CHwHWNDRenderTarget::Init
//
//  Synopsis:
//      1. Create the CD3DDeviceLevel1
//      2. Initialize the CHwDisplayRenderTarget
//
//------------------------------------------------------------------------------
HRESULT 
CHwHWNDRenderTarget::Init(
    __in_ecount_opt(1) HWND hwnd,
    __in_ecount(1) CDisplay const *pDisplay,
    MilDeviceKind type,
    MilRTInitialization::Flags dwFlags
    )
{
    HRESULT hr = S_OK;

    IFC(CHwDisplayRenderTarget::Init(
        hwnd,
        pDisplay,
        type,
        dwFlags));
    
    // Finish initialization with 0x0 flipping chain.  A call to Resize
    // is required before use.
    //
    
    IFC(UpdateFlippingChain(0, 0));

    // Call base init only after size has been updated by UpdateFlippingChain
    IFC(CBaseRenderTarget::Init());

    //
    // Check to see if we need to present linear content to a non-linear
    // display format
    //

    if (m_D3DPresentParams.BackBufferFormat == DXGI_FORMAT_R10G10B10A2_UNORM)
    {
        MilDisplayMode mode;

        IFC(pDisplay->GetMode(&mode));

        if (mode.Format != DXGI_FORMAT_R10G10B10A2_UNORM)
        {
            // DXGI has no D3DPRESENT_LINEAR_CONTENT; 10 bit back buffers are
            // only presented to 10 bit display modes.
            IFC(WGXERR_DISPLAYFORMATNOTSUPPORTED);
        }
        else
        {
            // The only way known to enable A2R10G10B10 is with a fullscreen
            // D3D RT (not enabled in WPF) so output a little warning if we see 
            // something else.  It could mean that the system is in transition 
            // or that the DWM has enabled A2R10G10B10, but this is just an application.
            TraceTag((tagMILWarning,
                      "Display mode is A2R10G10B10, but RT is not fullscreen."));
        }
    }

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CHwHWNDRenderTarget::SetPosition
//
//  Synopsis:
//      Remember Present position for when UpdateLayeredWindowEx is called.
//
//------------------------------------------------------------------------------

void
CHwHWNDRenderTarget::SetPosition(POINT ptOrigin)
{
    m_MILDC.SetPosition(ptOrigin);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CHwHWNDRenderTarget::UpdatePresentProperties
//
//  Synopsis:
//      Remember Present transparency properties for when UpdateLayeredWindowEx
//      is called.
//
//------------------------------------------------------------------------------

void
CHwHWNDRenderTarget::UpdatePresentProperties(
    MilTransparency::Flags transparencyFlags,
    BYTE constantAlpha,
    COLORREF colorKey
    )
{
    m_MILDC.SetLayerProperties(transparencyFlags, constantAlpha, colorKey, NULL);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CHwHWNDRenderTarget::Present
//
//  Synopsis:
//      1. Present the flipping chain
//      2. Update the render target
//
//------------------------------------------------------------------------------
STDMETHODIMP
CHwHWNDRenderTarget::Present(
    __in_ecount(1) const RECT *pRect
    )
{
    HRESULT hr = S_OK;

    IFC(CHwDisplayRenderTarget::Present(pRect));
    
Cleanup:
    RRETURN(hr);
}

STDMETHODIMP
CHwHWNDRenderTarget::ScrollBlt (
    __in_ecount(1) const RECT *prcSource,
    __in_ecount(1) const RECT *prcDest
    )
{
    RRETURN(E_NOTIMPL);
}


//+-----------------------------------------------------------------------------
//
//  Member:
//      CHwHWNDRenderTarget::Resize
//
//  Synopsis:
//      Updated the flipping chain size
//
//------------------------------------------------------------------------------
STDMETHODIMP 
CHwHWNDRenderTarget::Resize(
    UINT uWidth,
    UINT uHeight
    )
{
    HRESULT hr = S_OK;

    ENTER_DEVICE_FOR_SCOPE(*m_pD3DDevice);

    // All calls to Resize make the contents invalid
    DbgSetInvalidContents();

    //
    // Update the flipping chain size
    //

    IFC( UpdateFlippingChain(uWidth, uHeight) );

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CHwHWNDRenderTarget::UpdateFlippingChain
//
//  Synopsis:
//      If a flipping chain currently exists, replace it.   Otherwise create a
//      new one.
//
//      Note that we use a secondary flipping chain instead of the primary so
//      that we can resize it without recreating a device.
//
//------------------------------------------------------------------------------
HRESULT 
CHwHWNDRenderTarget::UpdateFlippingChain(
    UINT uWidth,
    UINT uHeight
    )
{
    HRESULT hr = S_OK;

    Assert(m_pD3DDevice);

    //
    // Release old resources
    //

    if (m_pD3DTargetSurface)
    {
        // A bound back buffer would keep DXGI from resizing the chain (D29).
        m_pD3DDevice->ReleaseUseOfRenderTarget(m_pD3DTargetSurface);
    }

    ReleaseInterface(m_pD3DTargetSurface);
    ReleaseInterface(m_pD3DBackBufferSurface);

    if (m_pD3DIntermediateMultisampleTargetSurface)
    {
        //
        // If new size of less than a quarter of current intermediate use
        // release the intermediate.
        //

        ULONGLONG ullSizeCur =
            m_pD3DIntermediateMultisampleTargetSurface->Desc().Width *
            m_pD3DIntermediateMultisampleTargetSurface->Desc().Height;

        ULONGLONG ullSizeNew = uWidth * uHeight;

        if (ullSizeNew < ullSizeCur / 4)
        {
            ReleaseInterface(m_pD3DIntermediateMultisampleTargetSurface);
        }
    }

    //
    // Don't render when minimized or empty
    //

    if (uWidth == 0 || uHeight == 0)
    {
        ReleaseInterface(m_pD3DSwapChain);
        m_fEnableRendering = FALSE;
        goto Cleanup;
    }

    //
    // D29: a flip model chain is resized in place. Recreating it would show
    // the window's never painted redirection surface in between.
    //

    if (   m_pD3DSwapChain
        && m_pD3DSwapChain->IsValid()
        && m_pD3DSwapChain->IsFlipModel())
    {
        if (FAILED(m_pD3DSwapChain->Resize(uWidth, uHeight)))
        {
            ReleaseInterface(m_pD3DSwapChain);
        }
    }
    else
    {
        ReleaseInterface(m_pD3DSwapChain);
    }

    //
    // Update Present Parameters
    //

    m_D3DPresentParams.BackBufferWidth = uWidth;
    m_D3DPresentParams.BackBufferHeight = uHeight;

    //
    // Create flipping chain
    //

    if (!m_pD3DSwapChain)
    {
        IFCSUB1(m_pD3DDevice->CreateAdditionalSwapChain(
            &m_MILDC,
            &m_D3DPresentParams,
            &m_pD3DSwapChain
            ));
    }

    //
    // Get the current back buffer and update CHwDisplayRenderTarget
    //

    m_rcBounds.left = 0;
    m_rcBounds.top  = 0;
    m_rcBounds.right  = static_cast<LONG>(uWidth);
    m_rcBounds.bottom = static_cast<LONG>(uHeight);

    m_uWidth = uWidth;
    m_uHeight = uHeight;

    IFCSUB1(m_pD3DSwapChain->GetBackBuffer(0, &m_pD3DTargetSurface));

    //
    // D32: GPU anti-aliasing. Draw into a multisampled surface; fills skip
    // the CPU coverage rasterizer (CHwRasterizer) and the edges come from
    // MSAA. Present resolves the surface into the back buffer.
    //
    {
        UINT cSamples = MilDx11GpuAntialiasSamples();
        ID3D11Device *pDevice = m_pD3DDevice->GetD3DDeviceNoRef();
        DXGI_FORMAT fmt = m_pD3DTargetSurface->Desc().Format;
        UINT uQuality = 0;

        while (   cSamples > 1
               && (   FAILED(pDevice->CheckMultisampleQualityLevels(fmt, cSamples, &uQuality))
                   || uQuality == 0))
        {
            cSamples /= 2;
        }

        if (cSamples > 1)
        {
            DXGI_SAMPLE_DESC sampleDesc = { cSamples, 0 };
            CD3DSurface *pMultisampleSurface = NULL;

            if (SUCCEEDED(m_pD3DDevice->CreateRenderTarget(
                    uWidth, uHeight, fmt, sampleDesc, false, &pMultisampleSurface)))
            {
                m_pD3DBackBufferSurface = m_pD3DTargetSurface;
                m_pD3DTargetSurface = pMultisampleSurface;
                MIL_DX11_LOG("gpu antialiasing: %u samples %ux%u", cSamples, uWidth, uHeight);
            }
        }
    }

    //
    // Reset dirty list for new frame (expected next) in case there was a
    // failure after some prior Invalidate calls, but before a call to
    // Present to clear the dirty list.
    //

    IFC(ClearInvalidatedRects());

    MIL_DX11_LOG("UpdateFlippingChain ok %ux%u", uWidth, uHeight);

SubCleanup1:

    if (FAILED(hr))
    {
        MIL_DX11_LOG("UpdateFlippingChain failed hr=0x%08x %ux%u", hr, uWidth, uHeight);
        //
        // Remember if the display is invalid, because we want to be consistent
        // about returning WGXERR_DISPLAYSTATEINVALID during Present.
        //

        if (hr == WGXERR_DISPLAYSTATEINVALID)
        {
            m_hrDisplayInvalid = hr;
        }

        m_fEnableRendering = FALSE;
    }
    else
    {
        m_fEnableRendering = TRUE;
    }

Cleanup:
    RRETURN(hr);
}


