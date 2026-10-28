// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//
//  $TAG ENGR
//
//      $Module:    win_mil_graphics
//      $Keywords:
//
//  $Description:
//      Provides utililites. Visibile outside hw directory.
//
//  $ENDTAG
//
//------------------------------------------------------------------------------

#include "precomp.hpp"


//+----------------------------------------------------------------------------
//
//  Member:
//      CacheHwTextureOnBitmap
//
//  Synopsis:
//      Caches a hardware texture on the specified bitmap.
//
//  Notes:
//      The texture is assumed to have fully valid contents, and to be the same
//      size as the bitmap.
//

HRESULT
CacheHwTextureOnBitmap(
    __inout_ecount(1) ID3D11Texture2D *pTexture,
    __inout_ecount(1) IWGXBitmap *pBitmap,
    __inout_ecount(1) CD3DDeviceLevel1 *pDevice
    )
{
    HRESULT hr = S_OK;

    CD3DVidMemOnlyTexture *pVidMemTexture  = NULL;
    CHwBitmapCache *pBitmapCache = NULL;
    CHwDeviceBitmapColorSource *pDeviceBitmapColorSource = NULL;
    UINT textureWidth;
    UINT textureHeight;

    //
    // Create a video memory texture wrapper for the texture.
    //

    IFC(CD3DVidMemOnlyTexture::Create(
        pTexture,
        false,              // fIsEvictable
        pDevice,
        &pVidMemTexture
        ));

    pVidMemTexture->GetTextureSize(
        &textureWidth,
        &textureHeight
        );


#if DBG
    {
        UINT dbgBitmapWidth;
        UINT dbgBitmapHeight;
        IGNORE_HR(pBitmap->GetSize(&dbgBitmapWidth, &dbgBitmapHeight));

        Assert(dbgBitmapWidth == textureWidth);
        Assert(dbgBitmapHeight == textureHeight);
    }
#endif

    //
    // Check for a bitmap cache.  Create and store one if it doesn't exist.
    //
    // Normally caching is optional, but in this case it is required, since
    // failure caching means we won't have access to the device bitmap
    // surface later when it is used as source.
    //

    IFC(CHwBitmapCache::GetCache(
        pDevice,
        pBitmap,
        NULL,
        true, 			// fSetResourceRequired
        &pBitmapCache
        ));

    {
        CMilRectU rcSurfBounds(0, 0, textureWidth, textureHeight, LTRB_Parameters);

        // Create the color source and put it in the cache
        IFC(pBitmapCache->CreateColorSourceForTexture(
            D3DFormatToPixelFormat(
                pVidMemTexture->D3DSurface0Desc().Format,
                FALSE),
            rcSurfBounds,           // rcBoundsRequired
            pVidMemTexture,
            OUT &pDeviceBitmapColorSource
            ));

        // Let the bitmap color source know that it contains fully valid bits.
        // A precondition for this function is that the texture contains valid
        // bits.
        pDeviceBitmapColorSource->UpdateValidBounds(
            rcSurfBounds
        );
    }

Cleanup:
    ReleaseInterfaceNoNULL(pVidMemTexture);
    ReleaseInterfaceNoNULL(pBitmapCache);
    ReleaseInterfaceNoNULL(pDeviceBitmapColorSource);

    RRETURN(hr);
}


//+----------------------------------------------------------------------------
//
//  Function:  ReadRenderTargetIntoSysMemBuffer
//
//  Synopsis:  Reads the texture's contents in the specified source rectangle
//             and writes it into the provided buffer.
//
//             Copied from CD3DSurface::ReadIntoSysMemBuffer because
//             pSourceTexture is not from one of our devices. It uses the
//             texture's own device and immediate context.
//
//             Differences:
//              1) No clip rects
//              2) pbBufferOut is the upper left corner, so we need to offset
//                 it by rcCopy
//              3) Only works if pSourceTexture is D3D11_USAGE_DEFAULT
//              4) Only works if system memory and video memory are same fmt
//
//+----------------------------------------------------------------------------

HRESULT
ReadRenderTargetIntoSysMemBuffer(
    __in ID3D11Texture2D *pSourceTexture,
    __in const CMilRectU &rcCopy,
    MilPixelFormat::Enum fmtOut,
    UINT uStrideOut,
    DBG_ANALYSIS_PARAM_COMMA(UINT cbBufferOut)
    __out_bcount_full(cbBufferOut) BYTE *pbBufferOut
    )
{
    HRESULT hr = S_OK;

    const UINT uBitsPerPixel = GetPixelFormatSize(fmtOut);
    UINT uStrideCopy;
    const UINT uCopyWidth = rcCopy.Width();
    const UINT uCopyHeight = rcCopy.Height();

    ID3D11Device *pIDevice = NULL;
    ID3D11DeviceContext *pIContext = NULL;

    ID3D11Texture2D *pD3DStagingTexture = NULL;
    ID3D11Texture2D *pD3DResolvedTexture = NULL;
    bool fMapped = false;

    if (   rcCopy.left > SURFACE_RECT_MAX
        || rcCopy.right > SURFACE_RECT_MAX
        || rcCopy.top > SURFACE_RECT_MAX
        || rcCopy.bottom > SURFACE_RECT_MAX
        || !rcCopy.IsWellOrdered())
    {
        IFC(E_INVALIDARG);
    }

    if (uBitsPerPixel % BITS_PER_BYTE)
    {
        IFC(E_INVALIDARG);
    }

    D3D11_TEXTURE2D_DESC desc;
    pSourceTexture->GetDesc(&desc);

    if (desc.Usage != D3D11_USAGE_DEFAULT)
    {
        IFC(E_INVALIDARG);
    }

    {
        const DXGI_FORMAT fmtRequested = PixelFormatToD3DFormat(fmtOut);

        bool fCompatible =
               (fmtRequested == desc.Format)
            || (   (fmtRequested == DXGI_FORMAT_B8G8R8A8_UNORM || fmtRequested == DXGI_FORMAT_B8G8R8X8_UNORM)
                && (desc.Format == DXGI_FORMAT_B8G8R8A8_UNORM || desc.Format == DXGI_FORMAT_B8G8R8X8_UNORM));

        if (!fCompatible)
        {
            IFC(E_INVALIDARG);
        }
    }

    IFC(HrCalcByteAlignedScanlineStride(uCopyWidth, uBitsPerPixel, OUT uStrideCopy));

#if DBG_ANALYSIS
    Assert(uStrideCopy <= uStrideOut);
    Assert(SUCCEEDED(HrCheckBufferSize(fmtOut, uStrideOut, uCopyWidth, uCopyHeight, cbBufferOut)));
#endif

    pSourceTexture->GetDevice(&pIDevice);
    pIDevice->GetImmediateContext(&pIContext);

    //
    // Create a staging texture to copy into.
    //
    {
        D3D11_TEXTURE2D_DESC descStaging;

        ZeroMemory(&descStaging, sizeof(descStaging));
        descStaging.Width = uCopyWidth;
        descStaging.Height = uCopyHeight;
        descStaging.MipLevels = 1;
        descStaging.ArraySize = 1;
        descStaging.Format = desc.Format;
        descStaging.SampleDesc.Count = 1;
        descStaging.SampleDesc.Quality = 0;
        descStaging.Usage = D3D11_USAGE_STAGING;
        descStaging.BindFlags = 0;
        descStaging.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        descStaging.MiscFlags = 0;

        IFC(MilTranslateD3D11Error(pIDevice->CreateTexture2D(&descStaging, NULL, &pD3DStagingTexture)));
    }

    //
    // Multisampled sources are resolved first.
    //
    {
        ID3D11Texture2D *pCopySource = pSourceTexture;

        if (desc.SampleDesc.Count > 1)
        {
            D3D11_TEXTURE2D_DESC descResolved = desc;

            descResolved.MipLevels = 1;
            descResolved.ArraySize = 1;
            descResolved.SampleDesc.Count = 1;
            descResolved.SampleDesc.Quality = 0;
            descResolved.Usage = D3D11_USAGE_DEFAULT;
            descResolved.BindFlags = D3D11_BIND_RENDER_TARGET;
            descResolved.CPUAccessFlags = 0;
            descResolved.MiscFlags = 0;

            IFC(MilTranslateD3D11Error(pIDevice->CreateTexture2D(&descResolved, NULL, &pD3DResolvedTexture)));

            pIContext->ResolveSubresource(pD3DResolvedTexture, 0, pSourceTexture, 0, desc.Format);

            pCopySource = pD3DResolvedTexture;
        }

        D3D11_BOX box;

        box.left = rcCopy.left;
        box.top = rcCopy.top;
        box.front = 0;
        box.right = rcCopy.right;
        box.bottom = rcCopy.bottom;
        box.back = 1;

        pIContext->CopySubresourceRegion(pD3DStagingTexture, 0, 0, 0, 0, pCopySource, 0, &box);
    }

    {
        D3D11_MAPPED_SUBRESOURCE mapped;

        IFC(MilTranslateD3D11Error(pIContext->Map(pD3DStagingTexture, 0, D3D11_MAP_READ, 0, &mapped)));
        fMapped = true;

        const UINT uBytesPerPixel = uBitsPerPixel / BITS_PER_BYTE;
        const BYTE *pbBufferIn = static_cast<const BYTE *>(mapped.pData);
#if DBG_ANALYSIS
        const BYTE *pbAnalysisBufferOutOrig = pbBufferOut;
#endif

        // pbBufferOut is the upper left pixel so adjust for rcCopy
        UINT cbLeftInset, cbBufferOutInset;
        IFC(UIntMult(uBytesPerPixel, rcCopy.left, &cbLeftInset));
        IFC(UIntMult(uStrideOut, rcCopy.top, &cbBufferOutInset));
        IFC(UIntAdd(cbLeftInset, cbBufferOutInset, &cbBufferOutInset));
        pbBufferOut += cbBufferOutInset;

        for (UINT i = rcCopy.top; i < rcCopy.bottom; ++i)
        {
            Assert(pbBufferOut + uStrideCopy <= pbAnalysisBufferOutOrig + cbBufferOut);

            memcpy(pbBufferOut, pbBufferIn, uStrideCopy);
            pbBufferOut += uStrideOut;
            pbBufferIn += mapped.RowPitch;
        }
    }

Cleanup:
    if (fMapped)
    {
        pIContext->Unmap(pD3DStagingTexture, 0);
    }

    ReleaseInterface(pD3DResolvedTexture);
    ReleaseInterface(pD3DStagingTexture);
    ReleaseInterface(pIContext);
    ReleaseInterface(pIDevice);

    RRETURN(hr);
}

//+----------------------------------------------------------------------------
//
//  Function:  IsD3DFailure
//
//  Synopsis:  Returns true if "hr" is a bad D3D HRESULT
//
//+----------------------------------------------------------------------------

bool IsD3DFailure(HRESULT hr)
{
    static const HRESULT hrD3DMask = MAKE_D3DHRESULT(0);
    return static_cast<HRESULT>(hr & hrD3DMask) == hrD3DMask;
}

//+-----------------------------------------------------------------------------
//
//  Function:  ReadD3D11TextureIntoSysMemBuffer
//
//  Synopsis:  Copies a rectangle of a user's D3D11 texture into a system
//             memory buffer using the texture's own device (staging copy).
//             Only the 32bpp BGRA family is supported.
//
//------------------------------------------------------------------------------

HRESULT
ReadD3D11TextureIntoSysMemBuffer(
    __in ID3D11Texture2D *pSourceTexture,
    __in const CMilRectU &rcCopy,
    MilPixelFormat::Enum fmtOut,
    UINT uStrideOut,
    DBG_ANALYSIS_PARAM_COMMA(UINT cbBufferOut)
    __out_bcount_full(cbBufferOut) BYTE *pbBufferOut
    )
{
    HRESULT hr = S_OK;
    ID3D11Device *pDevice = NULL;
    ID3D11DeviceContext *pContext = NULL;
    ID3D11Texture2D *pResolved = NULL;
    ID3D11Texture2D *pStaging = NULL;
    ID3D11Texture2D *pSource = pSourceTexture;
    bool fMapped = false;
    D3D11_TEXTURE2D_DESC desc;

    const UINT uBitsPerPixel = GetPixelFormatSize(fmtOut);

    if (   rcCopy.right > SURFACE_RECT_MAX
        || rcCopy.bottom > SURFACE_RECT_MAX
        || !rcCopy.IsWellOrdered()
        || uBitsPerPixel != 32)
    {
        IFC(E_INVALIDARG);
    }

    pSourceTexture->GetDesc(&desc);
    pSourceTexture->GetDevice(&pDevice);
    pDevice->GetImmediateContext(&pContext);

    if (rcCopy.right > desc.Width || rcCopy.bottom > desc.Height)
    {
        IFC(E_INVALIDARG);
    }

    if (desc.SampleDesc.Count > 1)
    {
        D3D11_TEXTURE2D_DESC descResolved = desc;
        descResolved.MipLevels = 1;
        descResolved.ArraySize = 1;
        descResolved.SampleDesc.Count = 1;
        descResolved.SampleDesc.Quality = 0;
        descResolved.BindFlags = D3D11_BIND_RENDER_TARGET;
        descResolved.CPUAccessFlags = 0;
        descResolved.MiscFlags = 0;

        IFC(MilTranslateD3D11Error(pDevice->CreateTexture2D(&descResolved, NULL, &pResolved)));
        pContext->ResolveSubresource(pResolved, 0, pSourceTexture, 0, desc.Format);
        pSource = pResolved;
    }

    {
        D3D11_TEXTURE2D_DESC descStaging = desc;
        descStaging.Width = rcCopy.Width();
        descStaging.Height = rcCopy.Height();
        descStaging.MipLevels = 1;
        descStaging.ArraySize = 1;
        descStaging.SampleDesc.Count = 1;
        descStaging.SampleDesc.Quality = 0;
        descStaging.Usage = D3D11_USAGE_STAGING;
        descStaging.BindFlags = 0;
        descStaging.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        descStaging.MiscFlags = 0;

        IFC(MilTranslateD3D11Error(pDevice->CreateTexture2D(&descStaging, NULL, &pStaging)));

        D3D11_BOX box;
        box.left = rcCopy.left;
        box.top = rcCopy.top;
        box.front = 0;
        box.right = rcCopy.right;
        box.bottom = rcCopy.bottom;
        box.back = 1;

        pContext->CopySubresourceRegion(pStaging, 0, 0, 0, 0, pSource, 0, &box);

        D3D11_MAPPED_SUBRESOURCE mapped;
        IFC(MilTranslateD3D11Error(pContext->Map(pStaging, 0, D3D11_MAP_READ, 0, &mapped)));
        fMapped = true;

        const UINT uBytesPerPixel = uBitsPerPixel / BITS_PER_BYTE;
        const UINT cbRow = rcCopy.Width() * uBytesPerPixel;
        BYTE *pbDest = pbBufferOut + static_cast<SIZE_T>(rcCopy.top) * uStrideOut + static_cast<SIZE_T>(rcCopy.left) * uBytesPerPixel;
        const BYTE *pbSrc = static_cast<const BYTE *>(mapped.pData);

#if DBG_ANALYSIS
        Assert(static_cast<SIZE_T>(rcCopy.bottom - 1) * uStrideOut + static_cast<SIZE_T>(rcCopy.right) * uBytesPerPixel <= cbBufferOut);
#endif

        for (UINT y = 0; y < rcCopy.Height(); y++)
        {
            CopyMemory(pbDest, pbSrc, cbRow);
            pbDest += uStrideOut;
            pbSrc += mapped.RowPitch;
        }
    }

Cleanup:
    if (fMapped)
    {
        pContext->Unmap(pStaging, 0);
    }

    ReleaseInterface(pStaging);
    ReleaseInterface(pResolved);
    ReleaseInterface(pContext);
    ReleaseInterface(pDevice);
    RRETURN(hr);
}
