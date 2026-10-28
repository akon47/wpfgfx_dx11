// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//-----------------------------------------------------------------------------
//
//  Description:
//      2D 텍스처의 서브리소스 하나를 "서피스"로 감싼다. D3D9의 IDirect3DSurface9
//      래퍼였던 것을 ID3D11Texture2D + 서브리소스 인덱스 + 지연 생성 뷰로 바꿨다.
//

MtExtern(CD3DSurface);
MtExtern(D3DResource_Surface);

class CD3DSurface : public CD3DResource
{
private:

    DECLARE_METERHEAP_ALLOC(ProcessHeap, Mt(CD3DSurface));

public:
    static HRESULT Create(
        __inout_ecount(1) CD3DResourceManager *pResourceManager,
        __inout_ecount(1) ID3D11Texture2D *pD3DTexture,
        UINT uSubresource,
        __deref_out_ecount(1) CD3DSurface **ppSurface
        );

    // 서피스 수준의 설명. Width/Height는 서브리소스(밉 레벨)에 맞게 조정되어 있다.
    D3D11_TEXTURE2D_DESC const &Desc() const { return m_desc; }

    UINT Subresource() const { return m_uSubresource; }

    bool IsStaging() const { return m_desc.Usage == D3D11_USAGE_STAGING; }
    bool IsRenderTarget() const { return (m_desc.BindFlags & D3D11_BIND_RENDER_TARGET) != 0; }
    bool IsDepthStencil() const { return (m_desc.BindFlags & D3D11_BIND_DEPTH_STENCIL) != 0; }
    bool IsMultisampled() const { return m_desc.SampleDesc.Count > 1; }

    //
    // Map/Unmap. STAGING 또는 DYNAMIC 텍스처에서만 쓸 수 있다.
    //

    HRESULT LockRect(
        __out_ecount(1) MilLockedRect *pLockedRect,
        __in_ecount_opt(1) CONST RECT *pRect,
        DWORD Flags
        );

    HRESULT UnlockRect(
        );

    __out_ecount_opt(1) ID3D11Texture2D * GetD3DTextureNoAddRef() const
    {
        return m_pD3DTexture;
    }

    __out_ecount(1) ID3D11Texture2D * ID3DTexture() const
    {
        Assert(m_pD3DTexture);
        return m_pD3DTexture;
    }

    //
    // 뷰는 처음 요청될 때 만들고 리소스 해제까지 들고 있는다. 반환 포인터는 AddRef되지 않는다.
    //

    HRESULT GetRenderTargetViewNoRef(
        __deref_out_ecount(1) ID3D11RenderTargetView **ppRTV
        );

    HRESULT GetDepthStencilViewNoRef(
        __deref_out_ecount(1) ID3D11DepthStencilView **ppDSV
        );

    // Returns the depth stencil view only if one was already created.
    ID3D11DepthStencilView *GetDepthStencilViewNoRefNoCreate() const { return m_pDSV; }

    HRESULT GetShaderResourceViewNoRef(
        __deref_out_ecount(1) ID3D11ShaderResourceView **ppSRV
        );

    void GetSurfaceSize(
        __out_ecount(1) UINT *puWidth,
        __out_ecount(1) UINT *puHeight
        ) const;

    // GDI 호환(D3D11_RESOURCE_MISC_GDI_COMPATIBLE) 텍스처에서만 성공한다.
    HRESULT GetDC(
        __out_ecount(1) HDC *phdc
        );

    HRESULT ReleaseDC(
        HDC hdc
        );

    HRESULT ReadIntoSysMemBuffer(
        __in_ecount(1) const CMilRectU &rcSource,
        UINT cClipRects,
        __in_ecount_opt(cClipRects) const CMilRectU *rgClipRects,
        MilPixelFormat::Enum fmtBitmapOut,
        UINT nStrideOut,
        DBG_ANALYSIS_PARAM_COMMA(UINT cbBufferOut)
        __out_bcount_full(cbBufferOut) BYTE *pbBufferOut
        );

protected:
    CD3DSurface(
        __inout_ecount(1) ID3D11Texture2D * pD3DTexture,
        UINT uSubresource
        );
    ~CD3DSurface();

    HRESULT Init(
        __inout_ecount(1) CD3DResourceManager *pResourceManager
        );

    void ReadDesc();

    void ReleaseViews();

#if PERFMETER
    virtual PERFMETERTAG GetPerfMeterTag() const
    {
        return Mt(D3DResource_Surface);
    }
#endif

private:

    //
    // CD3DResource methods
    //

    virtual override void ReleaseD3DResources();

protected:

    // Pointer to the actual D3D resource.
    //  The pointer is constant to help enforce the modification restrictions
    //  of CD3DResource objects.
    ID3D11Texture2D * const m_pD3DTexture;

    UINT const m_uSubresource;

    D3D11_TEXTURE2D_DESC m_desc;

    ID3D11RenderTargetView *m_pRTV;
    ID3D11DepthStencilView *m_pDSV;
    ID3D11ShaderResourceView *m_pSRV;

    bool m_fLocked;
};
