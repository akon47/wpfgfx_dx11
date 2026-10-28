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
    for (UINT i = 0; i < m_cBackBuffers; i++)
    {
        ReleaseInterfaceNoNULL(m_rgBackBuffers[i]);
    }

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

    //
    // Store array of back buffers
    //

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

    // Release reference to wrapper resources for each of the back buffers
    // first: DXGI requires all buffer references to be gone before the swap
    // chain can be destroyed or resized.
    for (UINT i = 0; i < m_cBackBuffers; i++)
    {
        ReleaseInterface(m_rgBackBuffers[i]);
    }

    // This context is protected so it is safe to release the DXGI resource
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
