// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//-----------------------------------------------------------------------------
//
//  Description:
//      Contains CD3DSwapChainWithSwDC implementation
//
//      This class overrides the GetDC method of CD3DSwapChain to implement
//      GetDC by reading the back buffer into a system memory DIB.
//

#include "precomp.hpp"
#include "mildx11log.h"

//+----------------------------------------------------------------------------
//
//  Member:
//      CD3DSwapChainWithSwDC::CD3DSwapChainWithSwDC
//
//  Synopsis:
//      ctor
//

#pragma warning( push )
#pragma warning( disable : 4355 )

CD3DSwapChainWithSwDC::CD3DSwapChainWithSwDC(
    __in HDC hdcPresentVia,
    __in_range(>, 0) /*__out_range(==, this->m_cBackBuffers)*/ UINT cBackBuffers,
    __inout_ecount(1) IDXGISwapChain1 *pDXGISwapChain
    ) : CD3DSwapChain(
            pDXGISwapChain,
            cBackBuffers,
            reinterpret_cast<CD3DSurface **>(reinterpret_cast<BYTE *>(this) + sizeof(*this))
            ),
      m_hdcCopiedBackBuffer(hdcPresentVia),
      m_hbmpCopiedBackBuffer(NULL),
      m_pBuffer(NULL),
      m_pStagingTexture(NULL)
{
}

#pragma warning( pop )

//+----------------------------------------------------------------------------
//
//  Member:
//      CD3DSwapChainWithSwDC::~CD3DSwapChainWithSwDC
//
//  Synopsis:
//      dtor
//

CD3DSwapChainWithSwDC::~CD3DSwapChainWithSwDC()
{
    if (m_hdcCopiedBackBuffer)
    {
        DeleteDC(m_hdcCopiedBackBuffer);
    }

    if (m_hbmpCopiedBackBuffer)
    {
        DeleteObject(m_hbmpCopiedBackBuffer);
    }

    ReleaseInterface(m_pStagingTexture);
}

void
CD3DSwapChainWithSwDC::ReleaseD3DResources()
{
    ReleaseInterface(m_pStagingTexture);
    CD3DSwapChain::ReleaseD3DResources();
}


//+----------------------------------------------------------------------------
//
//  Member:
//      CD3DSwapChainWithSwDC::Init
//
//  Synopsis:
//      Inits the swap chain and creates the sysmem present surface
//
//-----------------------------------------------------------------------------
HRESULT
CD3DSwapChainWithSwDC::Init(
    __inout_ecount(1) CD3DResourceManager *pResourceManager
    )
{
    HRESULT hr = S_OK;

    // The base class must be initialized first
    IFC(CD3DSwapChain::Init(pResourceManager));

    Assert(m_cBackBuffers >= 1);
    D3D11_TEXTURE2D_DESC const &surfDesc = m_rgBackBuffers[0]->Desc();

    // We don't handle anything else yet
    Assert(   surfDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM
           || surfDesc.Format == DXGI_FORMAT_B8G8R8X8_UNORM
          );

    BITMAPINFO bmi;

    bmi.bmiHeader.biSize = sizeof(bmi);
    bmi.bmiHeader.biWidth = surfDesc.Width;
    bmi.bmiHeader.biHeight = -INT(surfDesc.Height);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 0;
    bmi.bmiHeader.biXPelsPerMeter = 10000;
    bmi.bmiHeader.biYPelsPerMeter = 10000;
    bmi.bmiHeader.biClrUsed = 0;
    bmi.bmiHeader.biClrImportant = 0;

    IFCW32_CHECKOOH(GR_GDIOBJECTS, m_hbmpCopiedBackBuffer = CreateDIBSection(
        m_hdcCopiedBackBuffer,
        &bmi,
        DIB_RGB_COLORS,
        &m_pBuffer,
        NULL,
        0
        ));

    MilPixelFormat::Enum milFormat = D3DFormatToPixelFormat(surfDesc.Format, TRUE);

    IFC(HrCalcDWordAlignedScanlineStride(surfDesc.Width, milFormat, OUT m_stride));

    IFC(HrGetRequiredBufferSize(
        milFormat,
        m_stride,
        surfDesc.Width,
        surfDesc.Height,
        &m_cbBuffer));

    IFCW32(SelectObject(
        m_hdcCopiedBackBuffer,
        m_hbmpCopiedBackBuffer
        ));

    {
        D3D11_TEXTURE2D_DESC descStaging;
        ZeroMemory(&descStaging, sizeof(descStaging));
        descStaging.Width = surfDesc.Width;
        descStaging.Height = surfDesc.Height;
        descStaging.MipLevels = 1;
        descStaging.ArraySize = 1;
        descStaging.Format = surfDesc.Format;
        descStaging.SampleDesc.Count = 1;
        descStaging.SampleDesc.Quality = 0;
        descStaging.Usage = D3D11_USAGE_STAGING;
        descStaging.BindFlags = 0;
        descStaging.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        descStaging.MiscFlags = 0;

        IFC(Device().CreateTexture(&descStaging, NULL, &m_pStagingTexture));
    }

Cleanup:
    RRETURN(hr);
}

//+----------------------------------------------------------------------------
//
//  Member:
//      CD3DSwapChainWithSwDC::GetDC
//
//  Synopsis:
//      Gets a DC that refers to a system memory bitmap.
//
//      The system memory bitmap is updated during this call. The dirty rect is
//      used to determine how much of it needs updating.
//

HRESULT
CD3DSwapChainWithSwDC::GetDC(
    /*__in_range(<, this->m_cBackBuffers)*/ UINT iBackBuffer,
    __in_ecount(1) const CMilRectU& rcDirty,
    __deref_out HDC *phdcBackBuffer
    ) const
{
    HRESULT hr = S_OK;

    ENTER_USE_CONTEXT_FOR_SCOPE(Device());

    Assert(iBackBuffer < m_cBackBuffers);

    D3D11_TEXTURE2D_DESC const &surfDesc = m_rgBackBuffers[iBackBuffer]->Desc();

    UINT cbBufferInset =
          m_stride * rcDirty.top
        + D3DFormatSize(surfDesc.Format) * rcDirty.left;

    BYTE *pbBuffer = reinterpret_cast<BYTE*>(m_pBuffer) + cbBufferInset;

    //
    // D31: copy only the dirty rectangle into the persistent staging texture
    // and from there into the DIB. No per-present allocation.
    //

    {
        ID3D11DeviceContext *pContext = Device().GetD3DContextNoRef();
        D3D11_MAPPED_SUBRESOURCE mapped;
        double const msStart = MilDx11QpcMs();
        UINT const cbPixel = D3DFormatSize(surfDesc.Format);

        D3D11_BOX box;
        box.left = rcDirty.left;
        box.top = rcDirty.top;
        box.front = 0;
        box.right = rcDirty.right;
        box.bottom = rcDirty.bottom;
        box.back = 1;

        pContext->CopySubresourceRegion(
            m_pStagingTexture,
            0,
            rcDirty.left,
            rcDirty.top,
            0,
            m_rgBackBuffers[iBackBuffer]->ID3DTexture(),
            m_rgBackBuffers[iBackBuffer]->Subresource(),
            &box
            );

        IFC(MilTranslateD3D11Error(pContext->Map(m_pStagingTexture, 0, D3D11_MAP_READ, 0, &mapped)));

        BYTE const *pbSource =
              static_cast<BYTE const *>(mapped.pData)
            + static_cast<SIZE_T>(mapped.RowPitch) * rcDirty.top
            + static_cast<SIZE_T>(cbPixel) * rcDirty.left;
        SIZE_T const cbRow = static_cast<SIZE_T>(cbPixel) * rcDirty.Width();

        for (UINT y = rcDirty.top; y < rcDirty.bottom; y++)
        {
            memcpy(pbBuffer, pbSource, cbRow);
            pbBuffer += m_stride;
            pbSource += mapped.RowPitch;
        }

        pContext->Unmap(m_pStagingTexture, 0);
        MilDx11TimingAdd(MilDx11Timing_GdiReadback, MilDx11QpcMs() - msStart);
    }

    *phdcBackBuffer = m_hdcCopiedBackBuffer;

Cleanup:
    RRETURN(hr);
}

//+----------------------------------------------------------------------------
//
//  Member:
//      CD3DSwapChainWithSwDC::ReleaseDC
//
//  Synopsis:
//      Releases the DC returned by GetDC if necessary
//

HRESULT
CD3DSwapChainWithSwDC::ReleaseDC(
    /*__in_range(<, this->m_cBackBuffers)*/ UINT iBackBuffer,
    __in HDC hdcBackBuffer
    ) const
{
    //
    // Do nothing- GetDC just hands out the DC so release is not necessary.
    //
    UNREFERENCED_PARAMETER(iBackBuffer);
    UNREFERENCED_PARAMETER(hdcBackBuffer);

    return S_OK;
}
