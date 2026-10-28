// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//-----------------------------------------------------------------------------
//
//  Description:
//      Contains CD3DSwapChain implementation
//
//      Abstracts the DXGI swapchain.  The main reason to wrap this object is
//      so that we can respond to mode changes, i.e., respond to device
//      removal on the Present.
//

#include "precomp.hpp"
#include "mildx11log.h"

MtDefine(CD3DSwapChain, MILRender, "CD3DSwapChain");
MtDefine(D3DResource_SwapChain, MILHwMetrics, "Approximate swap chain size");

MtExtern(CD3DSwapChainWithSwDC);

//+----------------------------------------------------------------------------
//
//  Member:
//      CD3DSwapChain::Create
//
//  Synopsis:
//      Create the CD3DSwapChain wrapper from an IDXGISwapChain1
//
//  Notes:
//      This class behaves very differently depending on whether a present
//      context is passed in. If a present context is supplied, the swap chain
//      will implement GetDC by copying the backbuffer to a software GDI DIB
//      section. Even if GetDC is never called, this DIB section would be
//      created.
//

HRESULT
CD3DSwapChain::Create(
    __inout_ecount(1) CD3DResourceManager *pResourceManager,
    __inout_ecount(1) IDXGISwapChain1 *pDXGISwapChain,
    UINT BackBufferCount,
    __in_ecount_opt(1) CMILDeviceContext const *pPresentContext,
    __deref_out_ecount(1) CD3DSwapChain **ppSwapChain
    )
{
    HRESULT hr = S_OK;

    *ppSwapChain = NULL;

    //
    // Look up back buffer count if not given
    //

    if (BackBufferCount == 0)
    {
        DXGI_SWAP_CHAIN_DESC1 desc;

        IFC(pDXGISwapChain->GetDesc1(&desc));

        BackBufferCount = desc.BufferCount;
    }

    //
    // Create the swap chain wrapper
    //

    if (pPresentContext)
    {
        Assert(!pPresentContext->PresentWithHAL());

        HDC hdcPresentVia = NULL;

        IFC(pPresentContext->CreateCompatibleDC(&hdcPresentVia));

        *ppSwapChain = new(BackBufferCount) CD3DSwapChainWithSwDC(
            hdcPresentVia,
            BackBufferCount,
            pDXGISwapChain
            );

        //
        // If the swapchain was not successfully allocated, then it has not
        // taken ownership of the DC and the DC must now be deleted.  Otherwise
        // let the swapchain own it.
        //
        if (!*ppSwapChain)
        {
            DeleteDC(hdcPresentVia);
        }
    }
    else
    {
        *ppSwapChain = new(BackBufferCount) CD3DSwapChain(
            pDXGISwapChain,
            BackBufferCount,
            NULL // prgBackBuffers
            );
    }
    IFCOOM(*ppSwapChain);
    (*ppSwapChain)->AddRef(); // CD3DSwapChain::ctor sets ref count == 0

    //
    // Call init
    //

    IFC((*ppSwapChain)->Init(pResourceManager));

Cleanup:
    if (FAILED(hr))
    {
        ReleaseInterface(*ppSwapChain);
    }
    RRETURN(hr);
}

//+----------------------------------------------------------------------------
//
//  Member:    CD3DSwapChain::operator new
//
//  Synopsis:  Allocate memory for a CD3DSwapChain with cBackBuffers number of
//             back buffer surface.  Extra space is allocated for back buffer
//             pointer array.
//
//-----------------------------------------------------------------------------

__allocator __bcount_opt(cb + cBackBuffers*sizeof(PVOID) /* Prefast can't deal with type-modifiers, so using PVOID */)
void * __cdecl CD3DSwapChain::operator new(
    size_t cb,
    size_t cBackBuffers
    )
{
    void *pNew = NULL;

    // Calculate extra space for CD3DSurface array (m_rgBackBuffers). The order
    // of memory layout is this followed by back buffer array.
    size_t cbExtraData = cBackBuffers * sizeof(CD3DSurface *);

    // Check for overflow
    if (cbExtraData > cBackBuffers)
    {
        cb += cbExtraData;

        // Check for overflow
        if (cb >= cbExtraData)
        {
            pNew = WPFAlloc(ProcessHeap,
                                 Mt(CD3DSwapChain),
                                 cb);
        }
    }

    return pNew;
}

//+----------------------------------------------------------------------------
//
//  Member:    CD3DSwapChain::CD3DSwapChain
//
//  Synopsis:  ctor
//
//-----------------------------------------------------------------------------

#pragma warning( push )
// Allow use of this in constructor
#pragma warning( disable : 4355 )

CD3DSwapChain::CD3DSwapChain(
    __inout_ecount(1) IDXGISwapChain1 *pDXGISwapChain,
    __in_range(>, 0) __out_range(==, this->m_cBackBuffers) UINT cBackBuffers,
    __out_ecount_full_opt(cBackBuffers) CD3DSurface * * const prgBackBuffers
        // required for subclasses
    )
    : m_pDXGISwapChain(pDXGISwapChain),
      m_cBackBuffers(cBackBuffers),
      m_rgBackBuffers(
            prgBackBuffers
          ? prgBackBuffers
          : reinterpret_cast<CD3DSurface **>(reinterpret_cast<BYTE *>(this) + sizeof(*this))
            )
{
    RtlZeroMemory(m_rgBackBuffers, m_cBackBuffers * sizeof(*m_rgBackBuffers));
    m_pDXGISwapChain->AddRef();

    m_fFlipModel = false;
    m_cDXGIBuffers = 1;
    m_pPreviousBackBuffer = NULL;
    m_cWholeCopiesPending = 0;
    m_rcPendingCopy.SetEmpty();
    m_rcPreviousFrame.SetEmpty();
}

#pragma warning( pop )

//+----------------------------------------------------------------------------
//
//  Member:    CD3DSwapChain::~CD3DSwapChain
//
//  Synopsis:  dtor
//
//-----------------------------------------------------------------------------
CD3DSwapChain::~CD3DSwapChain()
{
    ReleaseBackBuffers();

    ReleaseInterfaceNoNULL(m_pDXGISwapChain);
}

//+----------------------------------------------------------------------------
//
//  Member:    CD3DSwapChain::Init
//
//  Synopsis:  Inits the swap chain
//
//-----------------------------------------------------------------------------
HRESULT
CD3DSwapChain::Init(
    __inout_ecount(1) CD3DResourceManager *pResourceManager
    )
{
    HRESULT hr = S_OK;

    Assert(pResourceManager);

    //
    // Init the base class
    //

    CD3DResource::Init(pResourceManager, 0);

    IFC(AcquireBackBuffers());

Cleanup:
    RRETURN(hr);
}

//+------------------------------------------------------------------------
//
//  Member:    CD3DSwapChain::GetBackBuffer
//
//  Synopsis:  Get indexed back buffer from array
//
//-------------------------------------------------------------------------
HRESULT
CD3DSwapChain::GetBackBuffer(
    __in_range(<, this->m_cBackBuffers) UINT iBackBuffer,
    __deref_out_ecount(1) CD3DSurface **ppBackBuffer
    ) const
{
    HRESULT hr = S_OK;

    Assert(IsValid());

    if (iBackBuffer >= m_cBackBuffers)
    {
        IFC(WGXERR_INVALIDPARAMETER);
    }

    *ppBackBuffer = m_rgBackBuffers[iBackBuffer];
    (*ppBackBuffer)->AddRef();

Cleanup:
    RRETURN(hr);
}

//+----------------------------------------------------------------------------
//
//  Member:    CD3DSwapChain::ReleaseD3DResources
//
//  Synopsis:  Release the swap chain.  Note that all callers of public
//             methods should have called IsValid before using the swapchain,
//             so it should be ok to blow away our internal swap chain here.
//
//             This method may only be called by CD3DResourceManager because
//             there are restrictions around when a call to ReleaseD3DResources
//             is okay.
//
//-----------------------------------------------------------------------------
void
CD3DSwapChain::ReleaseD3DResources()
{
    // This resource should have been marked invalid already or at least be out
    // of use.
    Assert(!m_fResourceValid || (m_cRef == 0));
    Assert(IsValid() == m_fResourceValid);

    ReleaseBackBuffers();

    ReleaseInterface((*const_cast<IDXGISwapChain1 **>(&m_pDXGISwapChain)));

    return;
}

//+------------------------------------------------------------------------
//
//  Member:
//      CD3DSwapChain::GetDC
//
//  Synopsis:
//      Gets the DC for the specified backbuffer
//

HRESULT
CD3DSwapChain::GetDC(
    __in_range(<, this->m_cBackBuffers) UINT iBackBuffer,
    __in_ecount(1) const CMilRectU& rcDirty,
    __deref_out HDC *phdcBackBuffer
    ) const
{
    HRESULT hr = S_OK;

    UNREFERENCED_PARAMETER(rcDirty);

    Assert(iBackBuffer < m_cBackBuffers);

    IFC(m_rgBackBuffers[iBackBuffer]->GetDC(
        phdcBackBuffer
        ));

Cleanup:
    RRETURN(hr);
}


//+----------------------------------------------------------------------------
//
//  Member:
//      CD3DSwapChain::ReleaseDC
//
//  Synopsis:
//      Releases the DC returned by GetDC if necessary
//

HRESULT
CD3DSwapChain::ReleaseDC(
    __in_range(<, this->m_cBackBuffers) UINT iBackBuffer,
    __in HDC hdcBackBuffer
    ) const
{
    RRETURN(m_rgBackBuffers[iBackBuffer]->ReleaseDC(
        hdcBackBuffer
        ));
}

//+------------------------------------------------------------------------
//
//  D29 flip model support.
//
//  Buffer 0 is the only writable buffer and the runtime swaps what it refers
//  to on every present. Buffer 1 is read only and holds the frame that was
//  just presented. WPF redraws only invalid regions, so after a present the
//  new back buffer is brought up to date from buffer 1 for everything that
//  changed since that buffer was last complete.
//
//-------------------------------------------------------------------------

HRESULT
CD3DSwapChain::AcquireBackBuffers()
{
    HRESULT hr = S_OK;
    CD3DResourceManager *pResourceManager = &Manager();
    DXGI_SWAP_CHAIN_DESC1 desc;

    for (UINT i = 0; i < m_cBackBuffers; i++)
    {
        ID3D11Texture2D *pD3DBackBuffer = NULL;

        IFC(MilTranslateD3D11Error(m_pDXGISwapChain->GetBuffer(
            i,
            __uuidof(ID3D11Texture2D),
            reinterpret_cast<void **>(&pD3DBackBuffer)
            )));

        MIL_THR(CD3DSurface::Create(pResourceManager, pD3DBackBuffer, 0, &m_rgBackBuffers[i]));
        ReleaseInterfaceNoNULL(pD3DBackBuffer);

        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

    IFC(MilTranslateD3D11Error(m_pDXGISwapChain->GetDesc1(&desc)));

    m_fFlipModel =
           desc.SwapEffect == DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL
        || desc.SwapEffect == DXGI_SWAP_EFFECT_FLIP_DISCARD;

    if (m_fFlipModel && desc.BufferCount > 1)
    {
        IFC(MilTranslateD3D11Error(m_pDXGISwapChain->GetBuffer(
            1,
            __uuidof(ID3D11Texture2D),
            reinterpret_cast<void **>(&m_pPreviousBackBuffer)
            )));
    }

    // Every buffer but the one being rendered starts out undefined.
    m_cDXGIBuffers = desc.BufferCount;
    m_cWholeCopiesPending = (desc.BufferCount > 1) ? desc.BufferCount - 1 : 0;
    m_rcPendingCopy.SetEmpty();
    m_rcPreviousFrame.SetEmpty();

    MIL_DX11_LOG("swap chain %p: %s %ux%u buffers=%u",
                 m_pDXGISwapChain, m_fFlipModel ? "flip" : "bitblt",
                 desc.Width, desc.Height, desc.BufferCount);

Cleanup:
    RRETURN(hr);
}

void
CD3DSwapChain::ReleaseBackBuffers()
{
    for (UINT i = 0; i < m_cBackBuffers; i++)
    {
        ReleaseInterface(m_rgBackBuffers[i]);
    }

    ReleaseInterface(m_pPreviousBackBuffer);
}

HRESULT
CD3DSwapChain::Resize(
    UINT uWidth,
    UINT uHeight
    )
{
    HRESULT hr = S_OK;

    Assert(IsValid());
    Assert(m_fFlipModel);

    //
    // DXGI refuses while anything still references the buffers. Released
    // wrappers normally wait for the next present to be destroyed.
    //

    ReleaseBackBuffers();
    Manager().DestroyResources(CD3DResourceManager::WithDelay);

    IFC(MilTranslateD3D11Error(m_pDXGISwapChain->ResizeBuffers(
        0,                      // keep the buffer count
        uWidth,
        uHeight,
        DXGI_FORMAT_UNKNOWN,    // keep the format
        0
        )));

    IFC(AcquireBackBuffers());

Cleanup:
    if (FAILED(hr))
    {
        MIL_DX11_LOG("ResizeBuffers failed hr=0x%08x %ux%u", hr, uWidth, uHeight);
        ReleaseBackBuffers();
    }

    RRETURN(hr);
}

UINT
CD3DSwapChain::CollectDirtyRects(
    __in_ecount_opt(1) CMILSurfaceRect const *prcPresented,
    __in_ecount_opt(1) RGNDATA const *pDirtyRegion,
    __out_ecount_part(cMax, return) RECT *prgRects,
    UINT cMax
    )
{
    UINT cRects = 0;

    if (   pDirtyRegion
        && pDirtyRegion->rdh.nCount > 0
        && pDirtyRegion->rdh.nCount <= cMax)
    {
        RECT const *prgSource = reinterpret_cast<RECT const *>(pDirtyRegion->Buffer);

        for (UINT i = 0; i < pDirtyRegion->rdh.nCount; i++)
        {
            prgRects[cRects++] = prgSource[i];
        }
    }
    else if (prcPresented && cMax > 0)
    {
        // Too many rects or no region: the present rect bounds them all.
        prgRects[0].left = prcPresented->left;
        prgRects[0].top = prcPresented->top;
        prgRects[0].right = prcPresented->right;
        prgRects[0].bottom = prcPresented->bottom;
        cRects = 1;
    }

    return cRects;
}

void
CD3DSwapChain::SyncBackBufferAfterPresent(
    __inout_ecount(1) ID3D11DeviceContext *pContext,
    __in_ecount_opt(1) CMILSurfaceRect const *prcPresented,
    __in_ecount_opt(1) RGNDATA const *pDirtyRegion,
    bool fPresented
    ) const
{
    if (!m_fFlipModel || !m_pPreviousBackBuffer || !m_rgBackBuffers[0])
    {
        return;
    }

    D3D11_TEXTURE2D_DESC const &desc = m_rgBackBuffers[0]->Desc();
    CMILSurfaceRect const rcBuffer(
        0, 0, static_cast<INT>(desc.Width), static_cast<INT>(desc.Height), LTRB_Parameters);

    RECT rgRects[sc_cMaxDirtyRects + 2];
    UINT cRects = 0;
    const bool fUnknownExtent = (prcPresented == NULL && pDirtyRegion == NULL);

    if (!fPresented)
    {
        //
        // Nothing rotated: this buffer stays the newest. Remember what it
        // changed so the other buffers catch up after the next present.
        //

        if (fUnknownExtent)
        {
            m_cWholeCopiesPending = max(m_cWholeCopiesPending, 1u);
        }
        else
        {
            cRects = CollectDirtyRects(prcPresented, pDirtyRegion, rgRects, sc_cMaxDirtyRects);

            for (UINT i = 0; i < cRects; i++)
            {
                UnionInto(m_rcPendingCopy, rgRects[i]);
            }
        }

        return;
    }

    ID3D11Texture2D *pBackBuffer = m_rgBackBuffers[0]->ID3DTexture();

    if (m_cWholeCopiesPending > 0 || fUnknownExtent)
    {
        pContext->CopyResource(pBackBuffer, m_pPreviousBackBuffer);

        if (m_cWholeCopiesPending > 0)
        {
            m_cWholeCopiesPending--;
        }

        m_rcPreviousFrame = rcBuffer;
    }
    else
    {
        //
        // Buffer 1 holds the frame just presented. The new buffer 0 was last
        // complete m_cDXGIBuffers - 1 presents ago, so it misses this frame,
        // whatever skipped presents left behind, and (with three buffers)
        // the frame before this one.
        //

        cRects = CollectDirtyRects(prcPresented, pDirtyRegion, rgRects, sc_cMaxDirtyRects);

        CMILSurfaceRect rcThisFrame;
        rcThisFrame.SetEmpty();

        for (UINT i = 0; i < cRects; i++)
        {
            UnionInto(rcThisFrame, rgRects[i]);
        }

        if (!m_rcPendingCopy.IsEmpty())
        {
            rgRects[cRects].left = m_rcPendingCopy.left;
            rgRects[cRects].top = m_rcPendingCopy.top;
            rgRects[cRects].right = m_rcPendingCopy.right;
            rgRects[cRects].bottom = m_rcPendingCopy.bottom;
            cRects++;
            UnionInto(rcThisFrame, rgRects[cRects - 1]);
        }

        if (m_cDXGIBuffers > 2 && !m_rcPreviousFrame.IsEmpty())
        {
            rgRects[cRects].left = m_rcPreviousFrame.left;
            rgRects[cRects].top = m_rcPreviousFrame.top;
            rgRects[cRects].right = m_rcPreviousFrame.right;
            rgRects[cRects].bottom = m_rcPreviousFrame.bottom;
            cRects++;
        }

        for (UINT i = 0; i < cRects; i++)
        {
            CMILSurfaceRect rc(
                rgRects[i].left, rgRects[i].top, rgRects[i].right, rgRects[i].bottom, LTRB_Parameters);

            if (!rc.Intersect(rcBuffer))
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
                pBackBuffer, 0, rc.left, rc.top, 0,
                m_pPreviousBackBuffer, 0, &box);
        }

        m_rcPreviousFrame = rcThisFrame;
    }

    m_rcPendingCopy.SetEmpty();
}

void
CD3DSwapChain::UnionInto(
    __inout_ecount(1) CMILSurfaceRect &rcInto,
    __in_ecount(1) RECT const &rc
    )
{
    if (rc.right <= rc.left || rc.bottom <= rc.top)
    {
        return;
    }

    if (rcInto.IsEmpty())
    {
        rcInto.left = rc.left;
        rcInto.top = rc.top;
        rcInto.right = rc.right;
        rcInto.bottom = rc.bottom;
    }
    else
    {
        rcInto.left = min(rcInto.left, rc.left);
        rcInto.top = min(rcInto.top, rc.top);
        rcInto.right = max(rcInto.right, rc.right);
        rcInto.bottom = max(rcInto.bottom, rc.bottom);
    }
}
