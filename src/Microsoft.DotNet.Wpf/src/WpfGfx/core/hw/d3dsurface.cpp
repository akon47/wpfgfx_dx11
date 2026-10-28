// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//-----------------------------------------------------------------------------
//
//  Description:
//      Contains CD3DSurface implementation
//
//      Provides basic abstraction of a D3D11 texture subresource and tracks
//      it as a D3D resource.
//

#include "precomp.hpp"

MtDefine(CD3DSurface, MILRender, "CD3DSurface");
MtDefine(D3DResource_Surface, MILHwMetrics, "Approximate surface sizes");

//+------------------------------------------------------------------------
//
//  Member:    CD3DSurface::Create
//
//  Synopsis:  Create a CD3DSurface object to wrap a D3D texture subresource
//
//-------------------------------------------------------------------------
HRESULT
CD3DSurface::Create(
    __inout_ecount(1) CD3DResourceManager *pResourceManager,
    __inout_ecount(1) ID3D11Texture2D *pD3DTexture,
    UINT uSubresource,
    __deref_out_ecount(1) CD3DSurface **ppSurface
    )
{
    HRESULT hr = S_OK;

    *ppSurface = NULL;

    //
    // Create the D3D surface wrapper
    //

    *ppSurface = new CD3DSurface(pD3DTexture, uSubresource);
    IFCOOM(*ppSurface);
    (*ppSurface)->AddRef(); // CD3DSurface::ctor sets ref count == 0

    //
    // Call init
    //

    IFC((*ppSurface)->Init(
        pResourceManager
        ));

Cleanup:
    if (FAILED(hr))
    {
        ReleaseInterface(*ppSurface);
    }
    RRETURN(hr);
}

//+------------------------------------------------------------------------
//
//  Member:    CD3DSurface::CD3DSurface
//
//  Synopsis:  ctor
//
//-------------------------------------------------------------------------
CD3DSurface::CD3DSurface(
    __inout_ecount(1) ID3D11Texture2D * const pD3DTexture,
    UINT uSubresource
    )
    : m_pD3DTexture(pD3DTexture),
      m_uSubresource(uSubresource),
      m_pRTV(NULL),
      m_pDSV(NULL),
      m_pSRV(NULL),
      m_fLocked(false)
{
    m_pD3DTexture->AddRef();
    ZeroMemory(&m_desc, sizeof(m_desc));
}

//+------------------------------------------------------------------------
//
//  Member:    CD3DSurface::~CD3DSurface
//
//  Synopsis:  dtor
//
//-------------------------------------------------------------------------
CD3DSurface::~CD3DSurface()
{
    ReleaseViews();
    ReleaseInterfaceNoNULL(m_pD3DTexture);
}

//+------------------------------------------------------------------------
//
//  Member:    CD3DSurface::ReadDesc
//
//  Synopsis:  Read the texture description and adjust it to this level
//
//-------------------------------------------------------------------------
void
CD3DSurface::ReadDesc()
{
    m_pD3DTexture->GetDesc(&m_desc);

    UINT uMipLevel = (m_desc.MipLevels > 0) ? (m_uSubresource % m_desc.MipLevels) : 0;

    for (UINT i = 0; i < uMipLevel; i++)
    {
        m_desc.Width = max(1u, m_desc.Width / 2);
        m_desc.Height = max(1u, m_desc.Height / 2);
    }
}

//+------------------------------------------------------------------------
//
//  Member:    CD3DSurface::Init
//
//  Synopsis:  Inits the surface wrapper
//
//-------------------------------------------------------------------------
HRESULT
CD3DSurface::Init(
    __inout_ecount(1) CD3DResourceManager *pResourceManager
    )
{
    HRESULT hr = S_OK;
    UINT uResourceSize;

    //
    // Compute the size of the resource
    //

    ReadDesc();

    if (IsStaging())
    {
        uResourceSize = 0;
    }
    else
    {
        UINT uSamplesPerPixel = max(1u, m_desc.SampleDesc.Count);

        uResourceSize = D3DFormatSize(m_desc.Format) * m_desc.Width * m_desc.Height * uSamplesPerPixel;
        Assert(uResourceSize > 0);
    }

    //
    // Init the base class
    //

    CD3DResource::Init(pResourceManager, uResourceSize);

    RRETURN(hr);
}

//+------------------------------------------------------------------------
//
//  Member:    CD3DSurface::ReleaseViews
//
//  Synopsis:  Release lazily created views
//
//-------------------------------------------------------------------------
void
CD3DSurface::ReleaseViews()
{
    ReleaseInterface(m_pRTV);
    ReleaseInterface(m_pDSV);
    ReleaseInterface(m_pSRV);
}

//+------------------------------------------------------------------------
//
//  Member:    CD3DSurface::ReleaseD3DResources
//
//  Synopsis:  Release the surface.
//
//             This method may only be called by CD3DResourceManager because
//             there are various restrictions around when a call to
//             ReleaseD3DResources is okay.
//
//-------------------------------------------------------------------------
void
CD3DSurface::ReleaseD3DResources()
{
    // This resource should have been marked invalid already or at least be out
    // of use.
    Assert(!m_fResourceValid || (m_cRef == 0));
    Assert(IsValid() == m_fResourceValid);

    if (IsRenderTarget())
    {
        Device().ReleaseUseOfRenderTarget(this);
    }

    if (IsDepthStencil())
    {
        Device().ReleaseUseOfDepthStencilSurface(this);
    }

    ReleaseViews();

    // This context is protected so it is safe to release the D3D resource
    ReleaseInterface((*const_cast<ID3D11Texture2D **>(&m_pD3DTexture)));

    return;
}

//+------------------------------------------------------------------------
//
//  Member:    CD3DSurface::GetSurfaceSize
//
//  Synopsis:  Return the dimensions of the surface.
//
//-------------------------------------------------------------------------
void
CD3DSurface::GetSurfaceSize(
    __out_ecount(1) UINT *puWidth,
    __out_ecount(1) UINT *puHeight
    ) const
{
    Assert(IsValid());

    *puWidth = m_desc.Width;
    *puHeight = m_desc.Height;
}

//+------------------------------------------------------------------------
//
//  Member:    CD3DSurface::GetRenderTargetViewNoRef
//
//  Synopsis:  Get (and lazily create) the render target view for this level
//
//-------------------------------------------------------------------------
HRESULT
CD3DSurface::GetRenderTargetViewNoRef(
    __deref_out_ecount(1) ID3D11RenderTargetView **ppRTV
    )
{
    HRESULT hr = S_OK;

    Assert(IsValid());
    Assert(IsRenderTarget());

    if (m_pRTV == NULL)
    {
        D3D11_RENDER_TARGET_VIEW_DESC desc;

        desc.Format = m_desc.Format;

        if (IsMultisampled())
        {
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = (m_desc.MipLevels > 0) ? (m_uSubresource % m_desc.MipLevels) : 0;
        }

        IFC(MilTranslateD3D11Error(Device().GetD3DDeviceNoRef()->CreateRenderTargetView(
            m_pD3DTexture,
            &desc,
            &m_pRTV
            )));
    }

    *ppRTV = m_pRTV;

Cleanup:
    RRETURN(hr);
}

//+------------------------------------------------------------------------
//
//  Member:    CD3DSurface::GetDepthStencilViewNoRef
//
//  Synopsis:  Get (and lazily create) the depth stencil view
//
//-------------------------------------------------------------------------
HRESULT
CD3DSurface::GetDepthStencilViewNoRef(
    __deref_out_ecount(1) ID3D11DepthStencilView **ppDSV
    )
{
    HRESULT hr = S_OK;

    Assert(IsValid());
    Assert(IsDepthStencil());

    if (m_pDSV == NULL)
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC desc;

        ZeroMemory(&desc, sizeof(desc));
        desc.Format = m_desc.Format;
        desc.ViewDimension = IsMultisampled() ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = 0;

        IFC(MilTranslateD3D11Error(Device().GetD3DDeviceNoRef()->CreateDepthStencilView(
            m_pD3DTexture,
            &desc,
            &m_pDSV
            )));
    }

    *ppDSV = m_pDSV;

Cleanup:
    RRETURN(hr);
}

//+------------------------------------------------------------------------
//
//  Member:    CD3DSurface::GetShaderResourceViewNoRef
//
//  Synopsis:  Get (and lazily create) a shader resource view over the whole
//             texture. Only meaningful for subresource 0.
//
//-------------------------------------------------------------------------
HRESULT
CD3DSurface::GetShaderResourceViewNoRef(
    __deref_out_ecount(1) ID3D11ShaderResourceView **ppSRV
    )
{
    HRESULT hr = S_OK;

    Assert(IsValid());
    Assert(m_desc.BindFlags & D3D11_BIND_SHADER_RESOURCE);

    if (m_pSRV == NULL)
    {
        IFC(MilTranslateD3D11Error(Device().GetD3DDeviceNoRef()->CreateShaderResourceView(
            m_pD3DTexture,
            NULL,
            &m_pSRV
            )));
    }

    *ppSRV = m_pSRV;

Cleanup:
    RRETURN(hr);
}

//+------------------------------------------------------------------------
//
//  Member:    CD3DSurface::LockRect
//
//  Synopsis:  Map the subresource for CPU access.
//
//-------------------------------------------------------------------------
HRESULT
CD3DSurface::LockRect(
    __out_ecount(1) MilLockedRect *pLockedRect,
    __in_ecount_opt(1) CONST RECT *pRect,
    DWORD Flags
    )
{
    HRESULT hr = S_OK;

    D3D11_MAPPED_SUBRESOURCE mapped;
    D3D11_MAP mapType;

    Assert(IsValid());
    Assert(!m_fLocked);

    if (m_desc.Usage == D3D11_USAGE_DYNAMIC)
    {
        mapType = D3D11_MAP_WRITE_DISCARD;
    }
    else if (m_desc.Usage == D3D11_USAGE_STAGING)
    {
        if (Flags & D3DLOCK_READONLY)
        {
            mapType = D3D11_MAP_READ;
        }
        else if (m_desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ)
        {
            mapType = D3D11_MAP_READ_WRITE;
        }
        else
        {
            mapType = D3D11_MAP_WRITE;
        }
    }
    else
    {
        IFC(WGXERR_INVALIDCALL);
    }

    IFC(MilTranslateD3D11Error(Device().GetD3DContextNoRef()->Map(
        m_pD3DTexture,
        m_uSubresource,
        mapType,
        0,
        &mapped
        )));

    m_fLocked = true;

    pLockedRect->Pitch = static_cast<INT>(mapped.RowPitch);
    pLockedRect->pBits = mapped.pData;

    if (pRect)
    {
        pLockedRect->pBits =
              static_cast<BYTE *>(mapped.pData)
            + static_cast<SIZE_T>(pRect->top) * mapped.RowPitch
            + static_cast<SIZE_T>(pRect->left) * D3DFormatSize(m_desc.Format);
    }

Cleanup:
    RRETURN(hr);
}

//+------------------------------------------------------------------------
//
//  Member:    CD3DSurface::UnlockRect
//
//  Synopsis:  Unmap the subresource.
//
//-------------------------------------------------------------------------
HRESULT
CD3DSurface::UnlockRect(
    )
{
    Assert(m_fLocked);

    Device().GetD3DContextNoRef()->Unmap(m_pD3DTexture, m_uSubresource);
    m_fLocked = false;

    return S_OK;
}

//+------------------------------------------------------------------------
//
//  Member:    CD3DSurface::GetDC
//
//  Synopsis:  Retrieve a DC wrapping this surface. Requires a GDI
//             compatible texture.
//
//-------------------------------------------------------------------------
HRESULT
CD3DSurface::GetDC(
    __out_ecount(1) HDC *phdc
    )
{
    HRESULT hr = S_OK;

    IDXGISurface1 *pDXGISurface = NULL;
    HDC hBackBuffer = NULL;

    *phdc = NULL;

    IFC(m_pD3DTexture->QueryInterface(__uuidof(IDXGISurface1), reinterpret_cast<void **>(&pDXGISurface)));

    MIL_THR(MilTranslateD3D11Error(pDXGISurface->GetDC(FALSE, &hBackBuffer)));

    //
    // DXGI isn't supposed to return E_FAIL to us.  We want to see if we can
    // reinterpret it.
    //
    if (hr == E_FAIL)
    {
        //
        // Check to see if we're close to being out of available GDI handles.
        // If we are we'll evaluate that as E_OUTOFMEMORY, otherwise we'll
        // return D3DERR_DRIVERINTERNALERROR.
        //
        IFC(CheckGUIHandleQuota(
            GR_GDIOBJECTS,
            E_OUTOFMEMORY,
            D3DERR_DRIVERINTERNALERROR
            ));
    }

    //
    // If we've succeeded assign the Handle to return.
    //
    if (SUCCEEDED(hr))
    {
        *phdc = hBackBuffer;
        hBackBuffer = NULL;
    }

Cleanup:
    //
    // If we have a backbuffer handle at this point it means
    // something failed.
    //
    if (hBackBuffer)
    {
        IGNORE_HR(pDXGISurface->ReleaseDC(NULL));
    }

    ReleaseInterface(pDXGISurface);

    RRETURN(hr);
}

//+------------------------------------------------------------------------
//
//  Member:    CD3DSurface::ReleaseDC
//
//  Synopsis:  Release a DC obtained with GetDC.
//
//-------------------------------------------------------------------------
HRESULT
CD3DSurface::ReleaseDC(
    HDC hdc
    )
{
    HRESULT hr = S_OK;

    IDXGISurface1 *pDXGISurface = NULL;

    UNREFERENCED_PARAMETER(hdc);

    IFC(m_pD3DTexture->QueryInterface(__uuidof(IDXGISurface1), reinterpret_cast<void **>(&pDXGISurface)));

    IFC(MilTranslateD3D11Error(pDXGISurface->ReleaseDC(NULL)));

Cleanup:
    ReleaseInterface(pDXGISurface);

    RRETURN(hr);
}

//+----------------------------------------------------------------------------
//
//  Member:    CD3DSurface::ReadIntoSysMemBuffer
//
//  Synopsis:  Reads the surface's contents in the specified source rectangle
//             and writes it into the provided buffer.
//
//             If cClipRects is non-zero, then writes are restricted to the
//             intersection of rcSource and rectangles in rgClipRects.  For
//             best performance clipping rectangles should not overlap one
//             another.
//
//             Note: the output buffer might be sparsely filled if nStrideOut
//                   is larger than the stride of the source in the given
//                   rectangle.
//
//             IMPORTANT: HwUtil.cpp's ReadRenderTargetIntoSysMemBuffer borrows
//             heavily from this so it should be kept in sync with this
//             implementation
//
//-----------------------------------------------------------------------------
HRESULT
CD3DSurface::ReadIntoSysMemBuffer(
    __in_ecount(1) const CMilRectU &rcSource,
    UINT cClipRects,
    __in_ecount_opt(cClipRects) const CMilRectU *rgClipRects,
    MilPixelFormat::Enum fmtOut,
    UINT nStrideOut,
    DBG_ANALYSIS_PARAM_COMMA(UINT cbBufferOut)
    __out_bcount_full(cbBufferOut) BYTE *pbBufferOut
    )
{
    HRESULT hr = S_OK;

    ID3D11Device *pDevice = Device().GetD3DDeviceNoRef();
    ID3D11DeviceContext *pContext = Device().GetD3DContextNoRef();

    ID3D11Texture2D *pStagingTexture = NULL;
    ID3D11Texture2D *pResolvedTexture = NULL;
    ID3D11Texture2D *pMappedTexture = NULL;

    UINT uMappedSubresource = 0;
    bool fMapped = false;
    UINT nStrideCopy;

    // Offset of rcSource inside the mapped texture
    UINT uMappedLeft = 0;
    UINT uMappedTop = 0;

    UINT const uSourceWidth = rcSource.Width();
    UINT const uSourceHeight = rcSource.Height();

    BYTE const BitsPerPixel = GetPixelFormatSize(fmtOut);

    if (BitsPerPixel % BITS_PER_BYTE)
    {
        TraceTag((tagMILWarning,
                  "Call to CD3DSurface::ReadIntoSysMemBuffer requested fraction byte copy"));
        IFC(WGXERR_INVALIDPARAMETER);
    }

    IFC(HrCalcByteAlignedScanlineStride(uSourceWidth, BitsPerPixel, nStrideCopy));

#if DBG_ANALYSIS
    Assert(nStrideCopy <= nStrideOut);
    Assert(SUCCEEDED(HrCheckBufferSize(fmtOut, nStrideOut, uSourceWidth, uSourceHeight, cbBufferOut)));
#endif

    Assert(Device().IsInAUseContext());

    //
    // 출력 포맷은 서피스 포맷과 같아야 한다. BGRA와 BGRX는 비트 호환이므로 허용.
    //

    {
        DXGI_FORMAT fmtRequested = PixelFormatToD3DFormat(fmtOut);

        bool fCompatible =
               (fmtRequested == m_desc.Format)
            || (   (fmtRequested == DXGI_FORMAT_B8G8R8A8_UNORM || fmtRequested == DXGI_FORMAT_B8G8R8X8_UNORM)
                && (m_desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM || m_desc.Format == DXGI_FORMAT_B8G8R8X8_UNORM));

        if (!fCompatible)
        {
            IFC(WGXERR_INVALIDCALL);
        }
    }

    if (IsStaging() && (m_desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ))
    {
        pMappedTexture = m_pD3DTexture;
        pMappedTexture->AddRef();
        uMappedSubresource = m_uSubresource;
        uMappedLeft = rcSource.left;
        uMappedTop = rcSource.top;
    }
    else
    {
        //
        // Copy the requested rectangle into a staging texture of the same
        // format and read it back from there. Multisampled sources are
        // resolved into a plain texture first.
        //

        D3D11_TEXTURE2D_DESC descStaging;

        ZeroMemory(&descStaging, sizeof(descStaging));
        descStaging.Width = uSourceWidth;
        descStaging.Height = uSourceHeight;
        descStaging.MipLevels = 1;
        descStaging.ArraySize = 1;
        descStaging.Format = m_desc.Format;
        descStaging.SampleDesc.Count = 1;
        descStaging.SampleDesc.Quality = 0;
        descStaging.Usage = D3D11_USAGE_STAGING;
        descStaging.BindFlags = 0;
        descStaging.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        descStaging.MiscFlags = 0;

        IFC(MilTranslateD3D11Error(pDevice->CreateTexture2D(&descStaging, NULL, &pStagingTexture)));

        ID3D11Texture2D *pCopySource = m_pD3DTexture;
        UINT uCopySourceSubresource = m_uSubresource;
        D3D11_BOX box;

        box.left = rcSource.left;
        box.top = rcSource.top;
        box.front = 0;
        box.right = rcSource.right;
        box.bottom = rcSource.bottom;
        box.back = 1;

        if (IsMultisampled())
        {
            D3D11_TEXTURE2D_DESC descResolved = m_desc;

            descResolved.MipLevels = 1;
            descResolved.ArraySize = 1;
            descResolved.SampleDesc.Count = 1;
            descResolved.SampleDesc.Quality = 0;
            descResolved.Usage = D3D11_USAGE_DEFAULT;
            descResolved.BindFlags = D3D11_BIND_RENDER_TARGET;
            descResolved.CPUAccessFlags = 0;
            descResolved.MiscFlags = 0;

            IFC(MilTranslateD3D11Error(pDevice->CreateTexture2D(&descResolved, NULL, &pResolvedTexture)));

            pContext->ResolveSubresource(pResolvedTexture, 0, m_pD3DTexture, m_uSubresource, m_desc.Format);

            pCopySource = pResolvedTexture;
            uCopySourceSubresource = 0;
        }

        pContext->CopySubresourceRegion(
            pStagingTexture,
            0,
            0,
            0,
            0,
            pCopySource,
            uCopySourceSubresource,
            &box
            );

        pMappedTexture = pStagingTexture;
        pMappedTexture->AddRef();
        uMappedSubresource = 0;
        uMappedLeft = 0;
        uMappedTop = 0;
    }

    //
    // Map and copy the bits to the destination.
    //
    {
        D3D11_MAPPED_SUBRESOURCE mapped;

        IFC(MilTranslateD3D11Error(pContext->Map(pMappedTexture, uMappedSubresource, D3D11_MAP_READ, 0, &mapped)));
        fMapped = true;

        UINT const BytesPerPixel = BitsPerPixel / BITS_PER_BYTE;

    #if DBG_ANALYSIS
        BYTE *pbAnalysisBufferOutOrig = pbBufferOut;
    #endif

        if (   cClipRects == 0
            || rgClipRects == NULL)
        {
            cClipRects = 1;
            rgClipRects = &rcSource;
        }

        do
        {
            CMilRectU rcCopy = *rgClipRects;
            if (rcCopy.Intersect(rcSource))
            {
                // Adjust pbBufferOut and mapped buffer pointer by amount
                // inset by intersection with source rect.  None of this math
                // should overflow since rcCopy is restricted to rcSource which
                // has already been validated as a good bounding rectangle.
                UINT cbBufferInset, cbLeftInset;

                IFC(MultiplyUINT(BytesPerPixel, rcCopy.left - rcSource.left, OUT cbLeftInset));

                IFC(MultiplyUINT(nStrideOut, rcCopy.top - rcSource.top, OUT cbBufferInset));
                IFC(AddUINT(cbBufferInset, cbLeftInset, OUT cbBufferInset));
                BYTE *pbOutBuffer = pbBufferOut + cbBufferInset;

                UINT cbMappedInset;
                IFC(MultiplyUINT(mapped.RowPitch, rcCopy.top - rcSource.top + uMappedTop, OUT cbMappedInset));
                IFC(MultiplyUINT(BytesPerPixel, uMappedLeft, OUT cbLeftInset));
                IFC(AddUINT(cbMappedInset, cbLeftInset, OUT cbMappedInset));
                IFC(MultiplyUINT(BytesPerPixel, rcCopy.left - rcSource.left, OUT cbLeftInset));
                IFC(AddUINT(cbMappedInset, cbLeftInset, OUT cbMappedInset));
                BYTE *pbInBuffer = static_cast<BYTE *>(mapped.pData) + cbMappedInset;

                IFC(MultiplyUINT(BytesPerPixel, rcCopy.Width(), OUT nStrideCopy));

                for (UINT i = rcCopy.top; i < rcCopy.bottom; i++)
                {
                    Assert(pbOutBuffer + nStrideCopy <= pbAnalysisBufferOutOrig + cbBufferOut);

                    GpMemcpy(pbOutBuffer, pbInBuffer, nStrideCopy);
                    pbOutBuffer += nStrideOut;
                    pbInBuffer += mapped.RowPitch;
                }
            }
        } while (rgClipRects++, --cClipRects > 0);
    }

Cleanup:
    if (fMapped)
    {
        pContext->Unmap(pMappedTexture, uMappedSubresource);
    }

    ReleaseInterface(pMappedTexture);
    ReleaseInterface(pResolvedTexture);
    ReleaseInterface(pStagingTexture);

    RRETURN(hr);
}
