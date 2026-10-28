// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//
//  $TAG ENGR
//
//      $Module:    win_mil_graphics_d3d
//      $Keywords:
//
//  $Description:
//      Contains CHwRenderStateManager declaration
//
//      파이프라인 상태의 그림자 사본을 들고 있다가 Draw 직전(ApplyStateForDraw)에
//      바뀐 것만 D3D11 컨텍스트에 바인딩한다. 블렌드·깊이스텐실·래스터라이저·샘플러
//      상태는 desc를 키로 상태 객체를 캐시한다. 셰이더 상수는 D3D9와 같은 레지스터
//      번호 체계(float4 c#)를 유지하고 상수 버퍼 하나로 올린다.
//
//      버텍스 셰이더나 픽셀 셰이더가 NULL이면 고정 기능 에뮬레이션 셰이더를 쓴다
//      (CHwFixedFunctionShaders).
//
//  $ENDTAG
//
//------------------------------------------------------------------------------

#define MIL_TEXTURE_STAGE_COUNT  8
#define MIL_SAMPLER_COUNT        MIL_TEXTURE_STAGE_COUNT

#define MIL_VS_FLOAT4_CONSTANTS  256
#define MIL_PS_FLOAT4_CONSTANTS  224
#define MIL_PS_INT4_CONSTANTS    16
#define MIL_PS_BOOL_CONSTANTS    16

ExternTag(tagWireframe);

MtExtern(CHwRenderStateManager);

class CD3DDeviceLevel1;

//
// 변환 상태 (D3DTS_* 대체). 고정 기능 에뮬레이션과 Set*TransformForVertexShader가 쓴다.
//
enum MilTransformState
{
    MilTransform_World      = 0,
    MilTransform_View       = 1,
    MilTransform_Projection = 2,

    MilTransform_Count
};

//
// 샘플러 상태 그림자. 바뀌면 상태 객체를 다시 찾는다.
//
struct MilSamplerState
{
    D3D11_FILTER               Filter;
    D3D11_TEXTURE_ADDRESS_MODE AddressU;
    D3D11_TEXTURE_ADDRESS_MODE AddressV;
    D3DCOLOR                   BorderColor;
    UINT                       MaxAnisotropy;
    BOOL                       UseMipMaps;

    bool operator==(const MilSamplerState &o) const
    {
        return Filter == o.Filter && AddressU == o.AddressU && AddressV == o.AddressV
            && BorderColor == o.BorderColor && MaxAnisotropy == o.MaxAnisotropy
            && UseMipMaps == o.UseMipMaps;
    }
};

struct MilBlendState
{
    BOOL       Enable;
    D3D11_BLEND SrcBlend;
    D3D11_BLEND DestBlend;
    UINT8      WriteMask;

    bool operator==(const MilBlendState &o) const
    {
        return Enable == o.Enable && SrcBlend == o.SrcBlend && DestBlend == o.DestBlend && WriteMask == o.WriteMask;
    }
};

struct MilDepthStencilState
{
    BOOL                  DepthEnable;
    BOOL                  DepthWriteEnable;
    D3D11_COMPARISON_FUNC DepthFunc;
    BOOL                  StencilEnable;
    D3D11_COMPARISON_FUNC StencilFunc;
    UINT8                 StencilReadMask;
    UINT8                 StencilWriteMask;

    bool operator==(const MilDepthStencilState &o) const
    {
        return DepthEnable == o.DepthEnable && DepthWriteEnable == o.DepthWriteEnable && DepthFunc == o.DepthFunc
            && StencilEnable == o.StencilEnable && StencilFunc == o.StencilFunc
            && StencilReadMask == o.StencilReadMask && StencilWriteMask == o.StencilWriteMask;
    }
};

struct MilRasterizerState
{
    D3D11_FILL_MODE FillMode;
    D3D11_CULL_MODE CullMode;
    BOOL            ScissorEnable;
    BOOL            MultisampleEnable;
    BOOL            AntialiasedLineEnable;

    bool operator==(const MilRasterizerState &o) const
    {
        return FillMode == o.FillMode && CullMode == o.CullMode && ScissorEnable == o.ScissorEnable
            && MultisampleEnable == o.MultisampleEnable && AntialiasedLineEnable == o.AntialiasedLineEnable;
    }
};

//+-----------------------------------------------------------------------------
//
//  Class:
//      CHwRenderStateManager
//
//  Synopsis:
//      Acts as an intermediary device for setting state.
//
//------------------------------------------------------------------------------
class CHwRenderStateManager :
         public CMILRefCountBase
{
public:
    static __checkReturn HRESULT Create(
        __inout_ecount(1) ID3D11Device *pD3DDevice,
        __inout_ecount(1) ID3D11DeviceContext *pD3DContext,
        __in_ecount(1) const MilDeviceCaps &caps,
        __deref_out_ecount(1) CHwRenderStateManager **ppStateManager
        );

private:
    CHwRenderStateManager();
    ~CHwRenderStateManager();

public:

    DEFINE_REF_COUNT_BASE

    //
    // Render targets. 즉시 바인딩한다.
    //

    HRESULT SetRenderTargetView(
        __in_ecount_opt(1) ID3D11RenderTargetView *pRTV,
        UINT uWidth,
        UINT uHeight
        );

    __out_ecount_opt(1) ID3D11RenderTargetView *GetRenderTargetViewNoRef() const { return m_pRTV; }

    HRESULT SetDepthStencilView(
        __in_ecount_opt(1) ID3D11DepthStencilView *pDSV,
        UINT uWidth,
        UINT uHeight
        );

    __out_ecount_opt(1) ID3D11DepthStencilView *GetDepthStencilViewNoRef() const { return m_pDSV; }

    bool IsDepthStencilSurfaceSmallerThan(
        UINT uWidth,
        UINT uHeight
        ) const;

    HRESULT ReleaseUseOfDepthStencilBuffer(
        __in_ecount(1) ID3D11DepthStencilView *pDSV
        );

    // 어떤 리소스가 렌더타깃이 되기 전에 SRV 슬롯에서 떼어낸다.
    void UnbindAllTextures();

    //
    // Blend
    //

    HRESULT SetAlphaBlend(BOOL fEnable, D3D11_BLEND src, D3D11_BLEND dest);
    HRESULT SetBlendFactor(D3DCOLOR color);
    HRESULT SetColorWriteMask(UINT8 mask);

    //
    // Depth / stencil
    //

    HRESULT SetDepthEnable(BOOL fEnable);
    HRESULT SetDepthWriteEnable(BOOL fEnable);
    HRESULT SetDepthFunc(D3D11_COMPARISON_FUNC func);
    HRESULT SetStencilEnable(BOOL fEnable);
    HRESULT SetStencilFunc(D3D11_COMPARISON_FUNC func);
    HRESULT SetStencilRef(UINT uRef);
    HRESULT SetStencilWriteMask(UINT8 mask);

    D3D11_COMPARISON_FUNC GetDepthFunc() const { return m_depthStencil.DepthFunc; }

    //
    // Rasterizer
    //

    HRESULT SetFillMode(D3D11_FILL_MODE mode);
    HRESULT SetCullMode(D3D11_CULL_MODE mode);
    HRESULT SetMultisampleAntialias(BOOL fEnable);
    HRESULT SetAntialiasedLineEnable(BOOL fEnable);

    D3D11_FILL_MODE GetFillMode() const { return m_rasterizer.FillMode; }

    //
    // Samplers and textures
    //

    HRESULT SetSamplerFilter(DWORD dwSampler, D3D11_FILTER filter, BOOL fUseMipMaps);
    HRESULT SetSamplerAddressModes(DWORD dwSampler, D3D11_TEXTURE_ADDRESS_MODE u, D3D11_TEXTURE_ADDRESS_MODE v);
    HRESULT SetSamplerBorderColor(DWORD dwSampler, D3DCOLOR color);

    MIL_FORCEINLINE HRESULT SetTextureInline(
        DWORD dwStage,
        __in_ecount_opt(1) ID3D11ShaderResourceView *pSRV
        )
    {
        Assert(dwStage < MIL_TEXTURE_STAGE_COUNT);

        if (m_rgpTextures[dwStage] != pSRV || !m_rgfTextureKnown[dwStage])
        {
            m_rgpTextures[dwStage] = pSRV;
            m_rgfTextureKnown[dwStage] = true;
            m_fTexturesDirty = true;
        }

        return S_OK;
    }

    //
    // Fixed function texture stages (used only when the pixel shader is NULL)
    //

    HRESULT SetTextureStageOperation(
        DWORD dwStage,
        __in_ecount(1) const MilTextureStageState &op
        );

    HRESULT DisableTextureStage(DWORD dwStage);

    HRESULT SetTexCoordIndex(DWORD dwStage, UINT uTexCoordIndex);

    HRESULT SetDefaultTexCoordIndices();

    //
    // Shaders
    //

    MIL_FORCEINLINE HRESULT SetPixelShaderInline(
        __in_ecount_opt(1) ID3D11PixelShader *pPixelShader
        )
    {
        if (m_pPixelShader != pPixelShader || !m_fPixelShaderKnown)
        {
            m_pPixelShader = pPixelShader;
            m_fPixelShaderKnown = true;
            m_fPixelShaderDirty = true;
        }

        return S_OK;
    }

    MIL_FORCEINLINE HRESULT SetVertexShaderInline(
        __in_ecount_opt(1) ID3D11VertexShader *pVertexShader
        )
    {
        if (m_pVertexShader != pVertexShader || !m_fVertexShaderKnown)
        {
            m_pVertexShader = pVertexShader;
            m_fVertexShaderKnown = true;
            m_fVertexShaderDirty = true;
        }

        return S_OK;
    }

    //
    // Shader constants (D3D9 register model)
    //

    HRESULT SetVertexShaderConstantF(
        UINT nRegisterIndex,
        __in_ecount(4*nRegisterCount) const float *prConstantData,
        UINT nRegisterCount
        );

    HRESULT SetPixelShaderConstantF(
        UINT nRegisterIndex,
        __in_ecount(4*nRegisterCount) const float * prConstantData,
        UINT nRegisterCount
        );

    HRESULT SetPixelShaderConstantI(
        UINT nRegisterIndex,
        __in_ecount(4) const int * pConstantData
        );

    HRESULT SetPixelShaderConstantB(
        UINT nRegisterIndex,
        BOOL constantData
        );

    //
    // Transforms
    //

    HRESULT SetTransform(
        MilTransformState state,
        __in_ecount(1) const CBaseMatrix *pMatrix
        );

    HRESULT GetTransform(
        MilTransformState state,
        __out_ecount(1) CMILMatrix *pMatrix
        ) const;

    HRESULT Define2DTransforms(
        __in_ecount(1) const CMatrix<CoordinateSpace::DeviceHPC,CoordinateSpace::D3DHomogeneousClipIPC> *pProjection
        );

    HRESULT Set2DTransformForFixedFunction();

    HRESULT Set2DTransformForVertexShader(UINT uStartRegister);
    HRESULT Set3DTransformForVertexShader(UINT uStartRegister);

    //
    // Input assembler
    //

    HRESULT SetVertexFormat(MilVertexFormatId format);

    MIL_FORCEINLINE BOOL IsVertexFormatSet(
        MilVertexFormatId format
        ) const
    {
        return m_fVertexFormatKnown && m_vertexFormat == format;
    }

    MIL_FORCEINLINE HRESULT SetStreamSource(
        __in_ecount_opt(1) ID3D11Buffer *pStream,
        UINT uVertexStride
        )
    {
        if (   m_pVertexBuffer != pStream
            || m_uVertexStride != uVertexStride
            || !m_fVertexBufferKnown)
        {
            m_pVertexBuffer = pStream;
            m_uVertexStride = uVertexStride;
            m_fVertexBufferKnown = true;
            m_fVertexBufferDirty = true;
        }

        return S_OK;
    }

    MIL_FORCEINLINE HRESULT SetIndices(
        __in_ecount_opt(1) ID3D11Buffer *pStream
        )
    {
        if (m_pIndexBuffer != pStream || !m_fIndexBufferKnown)
        {
            m_pIndexBuffer = pStream;
            m_fIndexBufferKnown = true;
            m_fIndexBufferDirty = true;
        }

        return S_OK;
    }

    HRESULT SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology);

    //
    // ScissorRect Functions
    //
    void InvalidateScissorRect();
    void ScissorRectChanged(
        __in_ecount(1) const MilPointAndSizeL *prc
        );

    HRESULT SetScissorRect(
        __in_ecount_opt(1) const MilPointAndSizeL *prc
        );

    //
    // Clip Functions
    //
    void SetClipSet(BOOL fSet)
        { m_fClipSet = fSet; }

    BOOL IsClipSet(const MilPointAndSizeL *prcClip) const
    {
        return IsClipSet() && RtlEqualMemory(prcClip, &m_rcClip, sizeof(MilPointAndSizeL));
    }

    BOOL IsClipSet() const
        { return m_fClipSet; }

    MilPointAndSizeL GetClip() const
        { return m_rcClip; }

    void SetClip(MilPointAndSizeL rcClip)
        { m_rcClip = rcClip; }

    HRESULT SetViewport(
        __in_ecount(1) const MilPointAndSizeL *prcViewport
        );

    MilPointAndSizeL GetViewport() const
        { return m_rcViewport; }

    //
    // Apply and defaults
    //

    // Draw 직전에 부른다. 더티 상태를 컨텍스트에 바인딩한다.
    HRESULT ApplyStateForDraw();

    HRESULT SetDefaultState();

    // 컨텍스트 상태를 모르게 되었을 때(외부 코드가 컨텍스트를 썼을 때) 전부 다시 올리게 한다.
    void InvalidateAll();

    __out_ecount(1) CHwFixedFunctionShaders *GetFixedFunctionShaders() { return &m_fixedFunction; }

private:

    HRESULT Init(
        __inout_ecount(1) ID3D11Device *pD3DDevice,
        __inout_ecount(1) ID3D11DeviceContext *pD3DContext,
        __in_ecount(1) const MilDeviceCaps &caps
        );

    HRESULT CreateConstantBuffers();

    HRESULT ApplyBlendState();
    HRESULT ApplyDepthStencilState();
    HRESULT ApplyRasterizerState();
    HRESULT ApplySamplers();
    HRESULT ApplyTextures();
    HRESULT ApplyShaders();
    HRESULT ApplyInputAssembler();
    HRESULT ApplyConstants();
    HRESULT ApplyViewportAndScissor();

    HRESULT UploadConstantBuffer(
        __in_ecount(1) ID3D11Buffer *pBuffer,
        __in_bcount(cb) const void *pvData,
        UINT cb
        );

    void Check2DTransformInVertexShader(
        UINT nRegisterIndex,
        UINT nRegisterCount
        );

    void MarkVSConstantsDirty(UINT nRegisterIndex, UINT nRegisterCount);
    void MarkPSConstantsDirty(UINT nRegisterIndex, UINT nRegisterCount);

private:
    DECLARE_METERHEAP_ALLOC(ProcessHeap, Mt(CHwRenderStateManager));

    //
    // D3D Device our state setting functions will be called on
    //
    ID3D11Device *m_pD3DDevice;
    ID3D11DeviceContext *m_pD3DContext;

    MilDeviceCaps m_caps;

    CHwFixedFunctionShaders m_fixedFunction;

    //
    // Render targets
    //
    ID3D11RenderTargetView *m_pRTV;
    UINT m_uRenderTargetWidth;
    UINT m_uRenderTargetHeight;

    ID3D11DepthStencilView *m_pDSV;
    UINT m_uDepthStencilSurfaceWidth;
    UINT m_uDepthStencilSurfaceHeight;

    //
    // Blend
    //
    MilBlendState m_blend;
    D3DCOLOR m_blendFactor;
    bool m_fBlendDirty;

    //
    // Depth / stencil
    //
    MilDepthStencilState m_depthStencil;
    UINT m_uStencilRef;
    bool m_fDepthStencilDirty;

    //
    // Rasterizer
    //
    MilRasterizerState m_rasterizer;
    bool m_fRasterizerDirty;

    //
    // Samplers and textures
    //
    MilSamplerState m_rgSamplers[MIL_SAMPLER_COUNT];
    bool m_rgfSamplerDirty[MIL_SAMPLER_COUNT];
    bool m_fAnySamplerDirty;

    ID3D11ShaderResourceView *m_rgpTextures[MIL_TEXTURE_STAGE_COUNT];
    bool m_rgfTextureKnown[MIL_TEXTURE_STAGE_COUNT];
    bool m_fTexturesDirty;

    //
    // Fixed function texture stages
    //
    MilTextureStageState m_rgStages[MIL_FF_STAGE_COUNT];
    UINT m_cActiveStages;
    bool m_fStagesDirty;

    //
    // Shaders
    //
    ID3D11VertexShader *m_pVertexShader;
    bool m_fVertexShaderKnown;
    bool m_fVertexShaderDirty;

    ID3D11PixelShader *m_pPixelShader;
    bool m_fPixelShaderKnown;
    bool m_fPixelShaderDirty;

    // 마지막으로 컨텍스트에 바인딩한 실제 셰이더 (에뮬레이션 포함)
    ID3D11VertexShader *m_pBoundVertexShader;
    ID3D11PixelShader *m_pBoundPixelShader;

    //
    // Shader constants (shadow copies, uploaded on draw)
    //
    float m_rgVSFloat4[MIL_VS_FLOAT4_CONSTANTS * 4];
    float m_rgPSFloat4[MIL_PS_FLOAT4_CONSTANTS * 4];
    int   m_rgPSInt4[MIL_PS_INT4_CONSTANTS * 4];
    int   m_rgPSBool4[MIL_PS_BOOL_CONSTANTS * 4];

    bool m_fVSConstantsDirty;
    bool m_fPSConstantsDirty;
    bool m_fPSIntBoolConstantsDirty;

    ID3D11Buffer *m_pVSConstantBuffer;
    ID3D11Buffer *m_pPSConstantBuffer;
    ID3D11Buffer *m_pPSIntBoolConstantBuffer;
    ID3D11Buffer *m_pFFConstantBuffer;

    //
    // Transforms
    //
    CMILMatrix m_rgTransforms[MilTransform_Count];
    bool m_fFFTransformDirty;

    CMatrix<CoordinateSpace::DeviceHPC,CoordinateSpace::D3DHomogeneousClipIPC>
        m_mat2DProjectionTransform;

    BOOL m_f2DTransformsUsedForFixedFunction;
    BOOL m_f2DTransformUsedForVertexShader;
    UINT m_u2DTransformVertexShaderStartRegister;

    //
    // Input assembler
    //
    MilVertexFormatId m_vertexFormat;
    bool m_fVertexFormatKnown;
    bool m_fVertexFormatDirty;

    ID3D11Buffer *m_pVertexBuffer;
    UINT m_uVertexStride;
    bool m_fVertexBufferKnown;
    bool m_fVertexBufferDirty;

    ID3D11Buffer *m_pIndexBuffer;
    bool m_fIndexBufferKnown;
    bool m_fIndexBufferDirty;

    D3D11_PRIMITIVE_TOPOLOGY m_topology;
    bool m_fTopologyDirty;

    //
    // Scissor rect and viewport
    //
    MilPointAndSizeL m_rcScissorRect;
    bool m_fScissorRectValid;
    bool m_fScissorDirty;

    BOOL m_fClipSet;
    MilPointAndSizeL m_rcClip;

    MilPointAndSizeL m_rcViewport;
    bool m_fViewportDirty;

    //
    // State object caches
    //
    template<class DescT, class ObjT>
    struct CStateObjectCache
    {
        enum { c_cMax = 32 };
        struct Entry { DescT desc; ObjT *pObject; } rg[c_cMax];
        UINT c;

        void Release()
        {
            for (UINT i = 0; i < c; i++) { ReleaseInterface(rg[i].pObject); }
            c = 0;
        }

        ObjT *Find(const DescT &desc) const
        {
            for (UINT i = 0; i < c; i++) { if (rg[i].desc == desc) return rg[i].pObject; }
            return NULL;
        }

        void Add(const DescT &desc, ObjT *pObject)
        {
            if (c >= c_cMax)
            {
                ReleaseInterface(rg[0].pObject);
                MoveMemory(&rg[0], &rg[1], sizeof(Entry) * (c_cMax - 1));
                c--;
            }
            rg[c].desc = desc;
            rg[c].pObject = pObject;
            pObject->AddRef();
            c++;
        }
    };

    CStateObjectCache<MilBlendState, ID3D11BlendState> m_blendCache;
    CStateObjectCache<MilDepthStencilState, ID3D11DepthStencilState> m_depthStencilCache;
    CStateObjectCache<MilRasterizerState, ID3D11RasterizerState> m_rasterizerCache;
    CStateObjectCache<MilSamplerState, ID3D11SamplerState> m_samplerCache;
};
