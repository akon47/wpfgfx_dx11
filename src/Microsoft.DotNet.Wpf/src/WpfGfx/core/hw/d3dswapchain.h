// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//------------------------------------------------------------------------------
//
//  Description:
//      Contains CD3DSwapChain implementation
//

MtExtern(CD3DSwapChain);
MtExtern(D3DResource_SwapChain);

//------------------------------------------------------------------------------
//
//  Class: CD3DSwapChain
//
//  Description:
//     Abstracts the DXGI swapchain.  The main reason to wrap this object is
//     so that we can respond to mode changes and device removal on Present.
//
//------------------------------------------------------------------------------

class CD3DSwapChain : public CD3DResource
{
    // We present through CD3DDeviceLevel1 instead of the CD3DSwap chain so that
    // we can have CD3DDeviceLevel1 handle present failures centrally.

    friend HRESULT CD3DDeviceLevel1::Present(
        __in_ecount(1) CD3DSwapChain const *pD3DSwapChain,
        __in_ecount_opt(1) CMILSurfaceRect const *prcSource,
        __in_ecount_opt(1) CMILSurfaceRect const *prcDest,
        __in_ecount(1) CMILDeviceContext const *pMILDC,
        __in_ecount_opt(1) RGNDATA const * pDirtyRegion,
        DWORD dwD3DPresentFlags
        );

protected:

    inline void __cdecl operator delete(void * pv) { WPFFree(ProcessHeap, pv); }
    __allocator __bcount_opt(cb + cBackBuffers*sizeof(PVOID))
        void * __cdecl operator new(size_t cb, size_t cBackBuffers);
    inline void __cdecl operator delete(void* pv, size_t) { WPFFree(ProcessHeap, pv); }

public:

    static HRESULT Create(
        __inout_ecount(1) CD3DResourceManager *pResourceManager,
        __inout_ecount(1) IDXGISwapChain1 *pDXGISwapChain,
        UINT BackBufferCount,
        __in_ecount_opt(1) CMILDeviceContext const *pPresentContext,
        __deref_out_ecount(1) CD3DSwapChain **ppSwapChain
        );

    //
    // IDXGISwapChain like helper methods
    //

    HRESULT GetBackBuffer(
        __in_range(<, this->m_cBackBuffers) UINT iBackBuffer,
        __deref_out_ecount(1) CD3DSurface **ppBackBuffer
        ) const;

    virtual HRESULT GetDC(
        __in_range(<, this->m_cBackBuffers) UINT iBackBuffer,
        __in_ecount(1) const CMilRectU& rcDirty,
        __deref_out HDC *phdcBackBuffer
        ) const;

    virtual HRESULT ReleaseDC(
        __in_range(<, this->m_cBackBuffers) UINT iBackBuffer,
        __in HDC hdcBackBuffer
        ) const;

    __out_ecount_opt(1) IDXGISwapChain1 *GetDXGISwapChainNoRef() const
    {
        return m_pDXGISwapChain;
    }

    //
    // D29 flip model. The runtime rotates the buffers on every present while
    // WPF only redraws what changed, so the frame just presented (buffer 1,
    // read only) is copied into the new back buffer (buffer 0) afterwards.
    //

    static const UINT sc_cMaxDirtyRects = 16;

    bool IsFlipModel() const { return m_fFlipModel; }

    //
    // D35: DXGI rejects dirty rects until a flip chain has had one regular
    // (full) present after creation or ResizeBuffers ("Partial Presentation
    // ... is not valid until first submitting a regular Present"). The first
    // presents of a fresh chain (one per buffer) go out without dirty rects.
    //
    bool TakeFullPresent() const
    {
        if (m_cFullPresentsPending > 0)
        {
            m_cFullPresentsPending--;
            return true;
        }
        return false;
    }

    void RequireFullPresent() const
    {
        m_cFullPresentsPending = max(m_cFullPresentsPending, 1u);
    }

    HRESULT Resize(
        UINT uWidth,
        UINT uHeight
        );

    void SyncBackBufferAfterPresent(
        __inout_ecount(1) ID3D11DeviceContext *pContext,
        __in_ecount_opt(1) CMILSurfaceRect const *prcPresented,
        __in_ecount_opt(1) RGNDATA const *pDirtyRegion,
        bool fPresented
        ) const;

    static UINT CollectDirtyRects(
        __in_ecount_opt(1) CMILSurfaceRect const *prcPresented,
        __in_ecount_opt(1) RGNDATA const *pDirtyRegion,
        __out_ecount_part(cMax, return) RECT *prgRects,
        UINT cMax
        );

    static void UnionInto(
        __inout_ecount(1) CMILSurfaceRect &rcInto,
        __in_ecount(1) RECT const &rc
        );

#if DBG
    UINT DbgGetNumBackBuffers() const
    {
        return m_cBackBuffers;
    }
#endif

protected:
    CD3DSwapChain(
        __inout_ecount(1) IDXGISwapChain1 *pDXGISwapChain,
        __in_range(>, 0) __out_range(==, this->m_cBackBuffers) UINT cBackBuffers,
        __out_ecount_full_opt(cBackBuffers) CD3DSurface * * const prgBackBuffers
        );
    virtual ~CD3DSwapChain();

protected:
    virtual HRESULT Init(
        __inout_ecount(1) CD3DResourceManager *pResourceManager
        );

private:

#if PERFMETER
    virtual PERFMETERTAG GetPerfMeterTag() const
    {
        return Mt(D3DResource_SwapChain);
    }
#endif


    //
    // CD3DResource methods
    //

protected:
    void ReleaseD3DResources();

private:
    // Pointer to the actual DXGI resource.
    //  The pointer is constant to help enforce the modification restrictions
    //  of CD3DResource objects.
    IDXGISwapChain1 * const m_pDXGISwapChain;

protected:
    __field_range(>, 1) UINT const m_cBackBuffers;
    __field_ecount(m_cBackBuffers) CD3DSurface * * const m_rgBackBuffers;

private:
    HRESULT AcquireBackBuffers();
    void ReleaseBackBuffers();

    // D29 flip model state.
    bool m_fFlipModel;
    UINT m_cDXGIBuffers;                        // buffers in the DXGI chain (3 for flip)
    ID3D11Texture2D *m_pPreviousBackBuffer;    // buffer 1: frame just presented
    mutable UINT m_cWholeCopiesPending;        // fresh buffers still to be filled completely
    mutable UINT m_cFullPresentsPending;       // D35: presents that must not carry dirty rects
    mutable CMILSurfaceRect m_rcPendingCopy;   // changes a skipped present left behind
    mutable CMILSurfaceRect m_rcPreviousFrame; // what the previous rotated present changed
};
