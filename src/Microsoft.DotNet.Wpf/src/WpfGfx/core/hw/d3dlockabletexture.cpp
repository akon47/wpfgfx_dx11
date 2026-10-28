// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//
//  $TAG ENGR
//
//      $Module:    win_mil_graphics_text
//      $Keywords:
//
//  $Description:
//      Contains CD3DLockableTexture implementation
//
//      Abstract a lockable D3D texture and track it
//       as a D3D resource.
//
//      Also contains CD3DLockableTexturePair implementation
//
//  $ENDTAG
//
//------------------------------------------------------------------------------

#include "precomp.hpp"

MtDefine(CD3DLockableTexture, MILRender, "CD3DLockableTexture");
MtDefine(D3DResource_LockableTexture, MILHwMetrics, "Approximate lockable texture size");

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DLockableTexture::Create
//
//  Synopsis:
//      Create the CD3DLockableTexture
//
//------------------------------------------------------------------------------
HRESULT
CD3DLockableTexture::Create(
    __inout_ecount(1) CD3DResourceManager *pResourceManager,
    __in_ecount(1) ID3D11Texture2D *pD3DGpuTexture,
    __in_ecount(1) ID3D11Texture2D *pD3DStagingTexture,
    __deref_out_ecount(1) CD3DLockableTexture **ppLockableTexture)
{
    HRESULT hr = S_OK;

    *ppLockableTexture = NULL;

    //
    // Create the CD3DLockableTexture
    //

    *ppLockableTexture = new CD3DLockableTexture();
    IFCOOM(*ppLockableTexture);
    (*ppLockableTexture)->AddRef(); // CD3DLockableTexture::ctor sets ref count == 0

    //
    // Call Init
    //

    IFC((*ppLockableTexture)->Init(pResourceManager, pD3DGpuTexture, pD3DStagingTexture));

Cleanup:
    if (FAILED(hr))
    {
        ReleaseInterface(*ppLockableTexture);
    }
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DLockableTexture::CD3DLockableTexture
//
//  Synopsis:
//      ctor
//
//------------------------------------------------------------------------------
CD3DLockableTexture::CD3DLockableTexture()
    : m_pD3DStagingTexture(NULL),
      m_fLocked(false),
      m_cDirtyRects(0)
{
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DLockableTexture::~CD3DLockableTexture
//
//  Synopsis:
//      dtor
//
//------------------------------------------------------------------------------
CD3DLockableTexture::~CD3DLockableTexture()
{
    ReleaseInterfaceNoNULL(m_pD3DStagingTexture);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DLockableTexture::Init
//
//  Synopsis:
//      Inits the texture pair
//
//------------------------------------------------------------------------------
HRESULT
CD3DLockableTexture::Init(
    __inout_ecount(1) CD3DResourceManager *pResourceManager,
    __in_ecount(1) ID3D11Texture2D *pD3DGpuTexture,
    __in_ecount(1) ID3D11Texture2D *pD3DStagingTexture
    )
{
    HRESULT hr = S_OK;

    //
    // Let CD3DTexture handle the init of the GPU texture
    //

    IFC(CD3DTexture::Init(pResourceManager, pD3DGpuTexture));

    m_pD3DStagingTexture = pD3DStagingTexture;
    m_pD3DStagingTexture->AddRef();

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DLockableTexture::ReleaseD3DResources
//
//  Synopsis:
//      Release both textures.
//
//------------------------------------------------------------------------------
void
CD3DLockableTexture::ReleaseD3DResources()
{
    if (m_pD3DStagingTexture != NULL)
    {
        // D26: staging halves are pooled too (reused one present later).
        Device().ReturnTextureToPool(m_pD3DStagingTexture);
        m_pD3DStagingTexture = NULL;
    }

    CD3DTexture::ReleaseD3DResources();
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DLockableTexture::LockRect
//
//  Synopsis:
//      Map the staging texture. The whole level is mapped; the returned
//      pointer is offset to pRect.
//
//------------------------------------------------------------------------------
HRESULT
CD3DLockableTexture::LockRect(
    __out_ecount(1) MilLockedRect* pLockedRect,
    __in_ecount(1) CONST RECT* pRect,
    DWORD dwFlags
    )
{
    HRESULT hr = S_OK;

    D3D11_MAPPED_SUBRESOURCE mapped;

    Assert(!m_fLocked);
    Assert(IsValid());

    IFC(MilTranslateD3D11Error(Device().GetD3DContextNoRef()->Map(
        m_pD3DStagingTexture,
        0,
        (dwFlags & D3DLOCK_READONLY) ? D3D11_MAP_READ : D3D11_MAP_READ_WRITE,
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
            + static_cast<SIZE_T>(pRect->left) * D3DFormatSize(m_sdLevel0.Format);

        if (!(dwFlags & (D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_READONLY)))
        {
            IFC(AddDirtyRect(*pRect));
        }
    }
    else if (!(dwFlags & (D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_READONLY)))
    {
        RECT rcAll = { 0, 0, static_cast<LONG>(m_sdLevel0.Width), static_cast<LONG>(m_sdLevel0.Height) };
        IFC(AddDirtyRect(rcAll));
    }

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DLockableTexture::UnlockRect
//
//  Synopsis:
//      Unmap the staging texture.
//
//------------------------------------------------------------------------------
HRESULT
CD3DLockableTexture::UnlockRect()
{
    Assert(m_fLocked);

    Device().GetD3DContextNoRef()->Unmap(m_pD3DStagingTexture, 0);
    m_fLocked = false;

    return S_OK;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DLockableTexture::AddDirtyRect
//
//  Synopsis:
//      Remember a region that must be copied to the GPU texture.
//
//------------------------------------------------------------------------------
HRESULT
CD3DLockableTexture::AddDirtyRect(
    __in_ecount(1) const RECT &rc
    )
{
    if (m_cDirtyRects < c_cMaxDirtyRects)
    {
        m_rgDirtyRects[m_cDirtyRects++] = rc;
    }
    else
    {
        // Too many rectangles: collapse everything into one bounding box.
        RECT rcUnion = m_rgDirtyRects[0];

        for (UINT i = 1; i < m_cDirtyRects; i++)
        {
            UnionRect(&rcUnion, &rcUnion, &m_rgDirtyRects[i]);
        }

        UnionRect(&rcUnion, &rcUnion, &rc);

        m_rgDirtyRects[0] = rcUnion;
        m_cDirtyRects = 1;
    }

    return S_OK;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DLockableTexture::EnsureGpuContents
//
//  Synopsis:
//      Copy dirty regions from the staging texture to the GPU texture.
//
//------------------------------------------------------------------------------
HRESULT
CD3DLockableTexture::EnsureGpuContents()
{
    Assert(!m_fLocked);

    if (m_cDirtyRects > 0 && IsValid())
    {
        ID3D11DeviceContext *pContext = Device().GetD3DContextNoRef();

        for (UINT i = 0; i < m_cDirtyRects; i++)
        {
            RECT const &rc = m_rgDirtyRects[i];

            if (rc.right <= rc.left || rc.bottom <= rc.top)
            {
                continue;
            }

            D3D11_BOX box;

            box.left = rc.left;
            box.top = rc.top;
            box.front = 0;
            box.right = rc.right;
            box.bottom = rc.bottom;
            box.back = 1;

            pContext->CopySubresourceRegion(
                m_pD3DTexture,
                0,
                rc.left,
                rc.top,
                0,
                m_pD3DStagingTexture,
                0,
                &box
                );
        }

        m_cDirtyRects = 0;
    }

    return S_OK;
}




//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DLockableTexturePair::CD3DLockableTexturePair
//
//  Synopsis:
//      ctor
//
//------------------------------------------------------------------------------
CD3DLockableTexturePair::CD3DLockableTexturePair()
{
    m_pTextureMain = NULL;
    m_pTextureAux = NULL;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DLockableTexturePair::~CD3DLockableTexturePair
//
//  Synopsis:
//      dtor
//
//------------------------------------------------------------------------------
CD3DLockableTexturePair::~CD3DLockableTexturePair()
{
    ReleaseInterface(m_pTextureMain);
    ReleaseInterface(m_pTextureAux);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DLockableTexturePair::InitMain
//
//  Synopsis:
//      Initialize main lockable texture.
//
//------------------------------------------------------------------------------
VOID
CD3DLockableTexturePair::InitMain(
    __in_ecount(1) CD3DLockableTexture *pTexture
    )
{
    Assert(m_pTextureMain == NULL);
    m_pTextureMain = pTexture;
    m_pTextureMain->AddRef();
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DLockableTexturePair::InitAux
//
//  Synopsis:
//      Initialize auxiliary lockable texture.
//
//------------------------------------------------------------------------------
VOID
CD3DLockableTexturePair::InitAux(
    __in_ecount(1) CD3DLockableTexture *pTexture
    )
{
    Assert(m_pTextureAux == NULL);
    m_pTextureAux = pTexture;
    m_pTextureAux->AddRef();
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DLockableTexturePair::Draw
//
//  Synopsis:
//      Render the texture pair using given device. Depending on fUseAux
//      argument, vector alpha values kept in m_pTextureAux will be involved.
//
//------------------------------------------------------------------------------
HRESULT
CD3DLockableTexturePair::Draw(
    __in_ecount(1) CD3DDeviceLevel1 *pDevice,
    __in_ecount(1) const MilPointAndSizeL &rc,
    bool fUseAux
    )
{
    HRESULT hr = S_OK;
    Assert(m_pTextureMain);

    if (fUseAux)
    {
        // vector alpha required, draw in two passes
        Assert(m_pTextureAux);
        IFC(pDevice->RenderTexture(
            m_pTextureAux,
            rc,
            TBM_APPLY_VECTOR_ALPHA
            ));

        IFC(pDevice->RenderTexture(
            m_pTextureMain,
            rc,
            TBM_ADD_COLORS
            ));
    }
    else
    {
        // regular case, just draw main texture
        IFC(pDevice->RenderTexture(
            m_pTextureMain,
            rc,
            TBM_DEFAULT
            ));
    }

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DLockableTexturePairLock::Lock
//
//  Synopsis:
//      Prepare one texture for filling with data: lock and clear the rectangle
//      on given texture, that may be either main or auxiliary. Return locked
//      buffer data (pointers and pitch) via LockData structure
//
//------------------------------------------------------------------------------
HRESULT CD3DLockableTexturePairLock::Lock(
    UINT uWidth,
    UINT uHeight,
    __out_ecount(1) LockData &lockData,
    bool fUseAux
    )
{
    Assert(!m_fMainLocked && !m_fAuxLocked);

    HRESULT hr = S_OK;

#if DBG_ANALYSIS
    {
        // Inspect texture sizes and let caller know locked area parameters.
        UINT uWidthMain = 0;
        UINT uHeightMain = 0;
        m_texturePair.m_pTextureMain->GetTextureSize(&uWidthMain, &uHeightMain);

        if (fUseAux)
        {
            UINT uWidthAux = 0;
            UINT uHeightAux = 0;
            m_texturePair.m_pTextureAux->GetTextureSize(&uWidthAux, &uHeightAux);
            Assert(uWidthMain == uWidthAux);
            Assert(uHeightMain == uHeightAux);
        }

        // We never attempt to lock outside of texture.
        Assert(uWidth <= uWidthMain);
        Assert(uHeight <= uHeightMain);

        lockData.m_uDbgAnalysisLockedWidth  = uWidth;
        lockData.m_uDbgAnalysisLockedHeight = uHeight;
    }
#endif

    MilLockedRect lockedRect;

    // Prepare main texture
    {
        IFC( LockOne(m_texturePair.m_pTextureMain, uWidth, uHeight, &lockedRect) );
        m_fMainLocked = true;
        lockData.pMainBits = static_cast<BYTE*>(lockedRect.pBits);
        lockData.uPitch = static_cast<UINT>(lockedRect.Pitch);
    }

    // Prepare auxiliary texture if necessary
    if (fUseAux)
    {
        IFC( LockOne(m_texturePair.m_pTextureAux, uWidth, uHeight, &lockedRect) );
        m_fAuxLocked = true;
        Assert(lockData.uPitch == lockedRect.Pitch);
        if (lockedRect.Pitch <= 0)
        {
            IFC(WGXERR_INTERNALERROR);
        }
        lockData.pAuxBits = static_cast<BYTE*>(lockedRect.pBits);
    }
    else
    {
        lockData.pAuxBits = NULL;
    }

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DLockableTexturePairLock::LockOne
//
//  Synopsis:
//      Prepare one texture for filling with data: lock and clear the rectangle
//      on given texture, that may be either main or auxiliary.
//
//------------------------------------------------------------------------------
HRESULT
CD3DLockableTexturePairLock::LockOne(
    __in_ecount(1) CD3DLockableTexture* pTexture,
    UINT uWidth,
    UINT uHeight,
    __out_ecount(1) MilLockedRect* pLockedRect
    )
{
    HRESULT hr = S_OK;

    //
    // Lock the rect and declare it dirty
    //

    CMilRectL rcLock(0, 0, uWidth, uHeight, XYWH_Parameters);
    IFC(pTexture->LockRect(pLockedRect, &rcLock, D3DLOCK_NO_DIRTY_UPDATE));

    IFC(pTexture->AddDirtyRect(rcLock));

    //
    // Clear the rect
    //

    BYTE *pScanline = (BYTE *)pLockedRect->pBits;
    for (UINT i = 0; i < uHeight; i++)
    {
        ZeroMemory(pScanline, uWidth*sizeof(ARGB));
        pScanline += pLockedRect->Pitch;
    }

Cleanup:
    RRETURN(hr);
}
