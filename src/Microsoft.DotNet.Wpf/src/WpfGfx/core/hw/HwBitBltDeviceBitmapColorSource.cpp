// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//

//
//
//  Class:
//      CHwBitBltDeviceBitmapColorSource
//
//  Synopsis:
//      Provides a color source that can be BitBlt to. It operates differently
//      based upon fIsDependent at creation.
//      
//      If fIsDependent = true, then it is dependent upon another color source 
//      for content, referred to as the primary DBCS. Realize will grab the bitmap 
//      dirty rects and copy from the bitmap's primary DBCS. D3DImage uses a dependent 
//      BBDBCS when drawing on a different adapter in BitBlt mode.
//
//      If fIsDependent = false, Realize will no-op, just like
//      CHwDeviceBitmapColorSource does, because it's always up to date. The
//      device bitmap pushes updates through UpdateSurface. D3DImage uses an
//      independent BBDBCS as its front buffer in BitBlt mode.
//
//------------------------------------------------------------------------------

#include "precomp.hpp"

MtDefine(CHwBitBltDeviceBitmapColorSource, MILRender, "CHwBitBltDeviceBitmapColorSource");

//+-----------------------------------------------------------------------------
//
//  Method:
//      ctor
//
//------------------------------------------------------------------------------

CHwBitBltDeviceBitmapColorSource::CHwBitBltDeviceBitmapColorSource(
    __in_ecount(1) CD3DDeviceLevel1 *pDevice,
    __in_opt IWGXBitmap *pBitmap,
    MilPixelFormat::Enum fmt,
    __in_ecount(1) const D3D11_TEXTURE2D_DESC &d3dsd,
    UINT uLevels
    )
    : 
    CHwDeviceBitmapColorSource(pDevice, pBitmap, fmt, d3dsd, uLevels), m_pTransferSurface(NULL)
{
}
    
//+-----------------------------------------------------------------------------
//
//  Method:
//      dtor
//
//------------------------------------------------------------------------------

CHwBitBltDeviceBitmapColorSource::~CHwBitBltDeviceBitmapColorSource()
{
    ReleaseInterface(m_pTransferSurface);
}

//+-----------------------------------------------------------------------------
//
//  Method:
//      CHwBitBltDeviceBitmapColorSource::Realize
//
//  Synopsis:
//      If necessary, copies dirty rects from the bitmap's primary DBCS
//      to this. See more comments at top of the file. 
//
//      When !IsRealizationValid(), then the color sources are on two different
//      device objects.
//
//------------------------------------------------------------------------------

HRESULT
CHwBitBltDeviceBitmapColorSource::Realize()
{
    HRESULT hr = S_OK;

    if (!IsRealizationValid())
    {
        const CMilRectU rcFull(0, 0, m_uBitmapWidth, m_uBitmapHeight, XYWH_Parameters);
        const CMilRectU *rgDirtyRects = NULL;
        
        const MilRectU *rgDirtyRectsNoClass = rgDirtyRects;

        UINT cDirtyRects = 0;
        UINT uNewUniqueness = 0;

        if (!m_pBitmap->GetDirtyRects(&rgDirtyRectsNoClass, &cDirtyRects, &uNewUniqueness))
        {
            rgDirtyRects = &rcFull;
            cDirtyRects = 1;
        }
        // else we're good because rgDirtyRectsNoClass points at rgDirtyRects
        
        CDeviceBitmap *pDeviceBitmap = DYNCAST(CDeviceBitmap, m_pBitmap);
        Assert(pDeviceBitmap);
        CHwDeviceBitmapColorSource *pPrimaryCS = pDeviceBitmap->GetDeviceColorSourceNoRef();
        if (pPrimaryCS)
        { 
            CD3DSurface *pPrimaryTransferSurface = pPrimaryCS->GetValidTransferSurfaceNoRef();
            if (pPrimaryTransferSurface)
            {
                //
                // Cross device BitBlt transfer is reworked in M9 together with
                // the shared handle path; this color source is not created
                // until then.
                //
                UNREFERENCED_PARAMETER(cDirtyRects);
                UNREFERENCED_PARAMETER(rgDirtyRects);
                IFC(E_NOTIMPL);
            }
        }
        // Else skip update because the device bitmap's front buffer doesn't exist yet.
        // Do not update cached uniqueness. Stay dirty.
    }

Cleanup:
    
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Method:
//      CHwBitBltDeviceBitmapColorSource::Create
//
//  Synopsis:
//      Creates a BitBlt color source on pDevice
//
//------------------------------------------------------------------------------

/* static */ HRESULT
CHwBitBltDeviceBitmapColorSource::Create(
    __in_ecount(1) CD3DDeviceLevel1 *pDevice,
    __in_ecount(1) IWGXBitmap *pBitmap,
    MilPixelFormat::Enum fmt,
    __in_ecount(1) CMilRectU const &rcBoundsRequired,
    bool fIsDependent,
    __deref_out_ecount(1) CHwDeviceBitmapColorSource **ppHwBitBltDBCS
    )
{
    HRESULT hr = S_OK;
    CHwBitBltDeviceBitmapColorSource *pbcs = NULL;
    CacheParameters oRealizationDesc;
    D3D11_TEXTURE2D_DESC d3dsd;
    UINT uLevels;

    IFC(CHwDeviceBitmapColorSource::CreateCommon(
        pDevice,
        pBitmap,
        fmt,
        rcBoundsRequired,
        NULL,         // pVidMemTexture
        OUT oRealizationDesc,
        OUT d3dsd,
        OUT uLevels
        ));

    // This will be the target of StretchRect so make sure RT was specified
    Assert((d3dsd.BindFlags & D3D11_BIND_RENDER_TARGET) == D3D11_BIND_RENDER_TARGET);

    pbcs = new CHwBitBltDeviceBitmapColorSource(
        pDevice,
        // This is key: a NULL bitmap means IsRealizationValid() always returns true and Realize()
        // won't do anything. Not NULL means Realize() will update this color source based on the
        // dirty rects.
        fIsDependent ? pBitmap : NULL,
        fmt, 
        d3dsd, 
        uLevels
        );
    IFCOOM(pbcs);
    pbcs->AddRef();

    IFC(pbcs->Init(
        pBitmap,
        oRealizationDesc,
        NULL,       // pVidMemTexture
        NULL        // pSharedHandle
        ));

    IFC(pDevice->CheckRenderTargetFormat(d3dsd.Format, /* pphrTestGetDC = */ NULL));
    
    {
        DXGI_SAMPLE_DESC sampleDesc = { 1, 0 };

        IFC(pDevice->CreateRenderTarget(
            d3dsd.Width,
            d3dsd.Height,
            d3dsd.Format,
            sampleDesc,
            true,       // fLockable
            &pbcs->m_pTransferSurface
            ));
    }

    *ppHwBitBltDBCS = pbcs;
    pbcs = NULL;

Cleanup:
    ReleaseInterfaceNoNULL(pbcs);

    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Method:
//      CHwBitBltDeviceBitmapColorSource::UpdateSurface
//
//  Synopsis:
//      Copies dirty rects from pISrcSurface to our texture. The source and
//      dest are on the same adapter, but different device objects.
//
//         Workaround for NVIDIA dynamic texture swizzle
//          Pre-G80 NVIDIA cards do swizzling on the CPU. Despite the fact that
//          the BitBlt was being done in hardware, just calling GetDC on the
//          dynamic texture caused very CPU expensive swizzling. Since you can't
//          create a dynamic render target texture, we introduced a new 
//          intermediate render target surface to be the destination of the 
//          BitBlt. This greatly improved NVIDIA performance. At first I thought
//          an offscreen plain surface would be more efficient, but it turns out
//          the G80 will pull that down to software on GetDC. RTS is the best.
//
//------------------------------------------------------------------------------

HRESULT
CHwBitBltDeviceBitmapColorSource::UpdateSurface(
    __in UINT cDirtyRects,
    __in_ecount(cDirtyRects) const CMilRectU *prgDirtyRects,
    __in_ecount(1) IDirect3DSurface9 *pISrcSurface
    )
{
    UNREFERENCED_PARAMETER(cDirtyRects);
    UNREFERENCED_PARAMETER(prgDirtyRects);
    UNREFERENCED_PARAMETER(pISrcSurface);

    // GDI BitBlt transfer from the user's D3D9 surface: reworked in M9.
    RRETURN(E_NOTIMPL);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CHwBitBltDeviceBitmapColorSource::GetValidTransferSurfaceNoRef
//
//  Synopsis:
//      Returns the transfer surface if the front buffer is valid
//
//------------------------------------------------------------------------------

__out_opt CD3DSurface *
CHwBitBltDeviceBitmapColorSource::GetValidTransferSurfaceNoRef()
{
    return IsValid() ? m_pTransferSurface : NULL;
}


