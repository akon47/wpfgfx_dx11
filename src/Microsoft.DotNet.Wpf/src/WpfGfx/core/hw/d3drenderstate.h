// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//

//
//  $TAG ENGR

//      $Module:    win_mil_graphics_d3d
//      $Keywords:
//
//  $Description:
//      Contains CD3DRenderState implementation
//
//      There are 2 purposes to this class:
//
//      1.  Forward state setting calls to the CHwRenderStateManager
//      2.  Group states commonly set together into tables to make it easier to
//          specify rendering options.
//
//      For the second part there are several objects that contain a collection
//      of renderstates:
//
//          AlphaBlendMode FilterMode TextureStageOperation
//
//  $ENDTAG
//
//------------------------------------------------------------------------------

enum TextureBlendArgument
{
    TBA_None = 0,
    TBA_Diffuse = 1,
    TBA_Specular = 2
};

enum TextureBlendMode
{
    TBM_DEFAULT = 0,
    TBM_COPY = 1,
    TBM_APPLY_VECTOR_ALPHA = 2,
    TBM_ADD_COLORS = 3
};

enum PixelShader
{
    // Pixel shaders for text rendering.
    // Notation: PXS_<TextSmoothingType><ColorSource>, where:
    // TextSmoothingType: CT = clear type, GS = grey scale
    // ColorSource: SB = solid brush, TB = textured brush
    PXS_CTSB      = 0,
    PXS_GSSB      = 1,
    PXS_CTTB      = 2,
    PXS_GSTB      = 3,
    PXS_NUM
};

//+-----------------------------------------------------------------------------
//
//  Structure:
//      AlphaBlendMode
//
//  Synopsis:
//      Blend mode for the back-end alpha blender.
//
//------------------------------------------------------------------------------
struct AlphaBlendMode
{
    BOOL        AlphaBlendEnable;
    D3D11_BLEND SrcBlend;
    D3D11_BLEND DestBlend;
};

//+-----------------------------------------------------------------------------
//
//  Structure:
//      TextureStageOperation
//
//  Synopsis:
//      Description of the operation performed in a given texture stage. These
//      are the building blocks of the fixed-function (emulated) pixel shader.
//
//------------------------------------------------------------------------------
struct TextureStageOperation
{
    BOOL m_fUsesTexture;
    MilTextureStageState m_state;   // TexCoordIndex is not used from here
};

//+-----------------------------------------------------------------------------
//
//  Structure:
//      FilterMode
//
//  Synopsis:
//      A set of filter modes for a given sampler.
//
//------------------------------------------------------------------------------
struct FilterMode
{
    D3D11_FILTER Filter;
    BOOL         UseMipMaps;    // FALSE == D3D9 D3DTEXF_NONE mip filter
};

class CD3DSurface;

//+-----------------------------------------------------------------------------
//
//  Class:
//      CD3DRenderState
//
//  Synopsis:
//      The purpose of this class is to centralize all states used by our
//      rendering engine so that we can reduce state transitions for better
//      performance.  We also limit the states that can be set so that we can
//      understand the requirements for our engine.
//
//------------------------------------------------------------------------------
class CD3DRenderState
{
public:
    CD3DRenderState();
    ~CD3DRenderState();

    // Associate a d3d device with the render state object
    HRESULT Init(
        __in_ecount(1) CD3DDeviceLevel1 *pDevice,
        __inout_ecount(1) ID3D11Device *pD3DDevice,
        __inout_ecount(1) ID3D11DeviceContext *pD3DContext
        );

    HRESULT ResetState();

    HRESULT SetVertexShaderConstantF(
        UINT StartRegister,
        __in_ecount(4*RegisterCount) const float *pConstantData,
        UINT RegisterCount
        )
        {
            return m_pStateManager->SetVertexShaderConstantF(
                StartRegister,
                pConstantData,
                RegisterCount
                );
        }

    HRESULT SetPixelShaderConstantF(
        UINT StartRegister,
        __in_ecount(4*RegisterCount) const float *pConstantData,
        UINT RegisterCount
        )
        {
            return m_pStateManager->SetPixelShaderConstantF(
                StartRegister,
                pConstantData,
                RegisterCount
                );
        }

    HRESULT SetPixelShaderConstantI(
        UINT StartRegister,
        __in_ecount(4) const int *pConstantData
        )
        {
            return m_pStateManager->SetPixelShaderConstantI(
                StartRegister,
                pConstantData
                );
        }

    HRESULT SetPixelShaderConstantB(
        UINT StartRegister,
        BOOL constantData
        )
        {
            return m_pStateManager->SetPixelShaderConstantB(
                StartRegister,
                constantData
                );
        }

    // Get alpha texture format that becomes known during Init() and then never changes
    DXGI_FORMAT GetAlphaTextureFormat() const { return m_alphaTextureFormat; }

    // Setting render state for text
    HRESULT SetRenderState_Text_ClearType_SolidBrush(
        DWORD dwForegroundColor,
        UINT gammaIndex
        );
    HRESULT SetRenderState_Text_ClearType_TextureBrush(
        UINT gammaIndex,
        float flEffectAlpha
        );
    HRESULT SetRenderState_Text_GreyScale_SolidBrush(
        DWORD dwForegroundColor,
        UINT gammaIndex
        );
    HRESULT SetRenderState_Text_GreyScale_TextureBrush(
        UINT gammaIndex,
        float flEffectAlpha
        );

    HRESULT SetFilterMode(
        DWORD dwSampler,
        __in_ecount(1) const FilterMode *pfmNew
        );

    HRESULT SetAlphaBlendMode(
        __in_ecount(1) const AlphaBlendMode *pabmNew
        );

    HRESULT GetFillMode(
        __out_ecount(1) D3D11_FILL_MODE *pFillMode
        ) const;

    HRESULT GetDepthTestFunction(
        __out_ecount(1) D3D11_COMPARISON_FUNC *pDepthTestFunction
        ) const;

    // Auxiliary render state switching for text
    // (to be called after proper SetRenderState_Text_*())
    HRESULT SetColorChannelRed();
    HRESULT SetColorChannelGreen();
    HRESULT SetColorChannelBlue();
    HRESULT RestoreColorChannels();


    /////////////////////////////////////////////////
    //
    //   High-Level Render State Setting Methods
    //
    //-----------------------------------------------

    HRESULT SetRenderState_AlphaSolidBrush();

    HRESULT SetRenderState_Texture(
        TextureBlendMode blendMode,
        TextureBlendArgument eBlendArgument,
        MilBitmapInterpolationMode::Enum interpolationMode,
        UINT cMasks
    );


    //
    // Forwarded calls to the RenderStateManager
    //

    MIL_FORCEINLINE HRESULT DisableTextureStage(
        DWORD dwStage
        )
    {
        return m_pStateManager->DisableTextureStage(
            dwStage
            );
    }

    MIL_FORCEINLINE HRESULT SetTransform(
        MilTransformState state,
        __in_ecount(1) const CMILMatrix *pMatrix
        )
    {
        return m_pStateManager->SetTransform(
            state,
            pMatrix
            );
    }

    MIL_FORCEINLINE HRESULT SetWorldTransform(
        __in_ecount(1) const CBaseMatrix *pMatrix
        )
    {
        return m_pStateManager->SetTransform(
            MilTransform_World,
            pMatrix
            );
    }

    MIL_FORCEINLINE HRESULT SetNonWorldTransform(
        MilTransformState state,
        __in_ecount(1) const CMILMatrix *pMatrix
        )
    {
        Assert(state != MilTransform_World);
        return m_pStateManager->SetTransform(
            state,
            pMatrix
            );
    }

    MIL_FORCEINLINE HRESULT GetTransform(
        MilTransformState state,
        __out_ecount(1) CMILMatrix *pMatrix
        ) const
    {
        return m_pStateManager->GetTransform(
            state,
            pMatrix
            );
    }

    //
    // Semantic render states (replace D3D9 SetRenderState)
    //

    MIL_FORCEINLINE HRESULT SetDepthEnable(BOOL fEnable)
        { return m_pStateManager->SetDepthEnable(fEnable); }

    MIL_FORCEINLINE HRESULT SetDepthWriteEnable(BOOL fEnable)
        { return m_pStateManager->SetDepthWriteEnable(fEnable); }

    MIL_FORCEINLINE HRESULT SetDepthFunc(D3D11_COMPARISON_FUNC func)
        { return m_pStateManager->SetDepthFunc(func); }

    MIL_FORCEINLINE HRESULT SetStencilEnable(BOOL fEnable)
        { return m_pStateManager->SetStencilEnable(fEnable); }

    MIL_FORCEINLINE HRESULT SetStencilFunc(D3D11_COMPARISON_FUNC func)
        { return m_pStateManager->SetStencilFunc(func); }

    MIL_FORCEINLINE HRESULT SetStencilRef(UINT uRef)
        { return m_pStateManager->SetStencilRef(uRef); }

    MIL_FORCEINLINE HRESULT SetStencilWriteMask(UINT8 mask)
        { return m_pStateManager->SetStencilWriteMask(mask); }

    MIL_FORCEINLINE HRESULT SetFillMode(D3D11_FILL_MODE mode)
        { return m_pStateManager->SetFillMode(mode); }

    MIL_FORCEINLINE HRESULT SetCullMode(D3D11_CULL_MODE mode)
        { return m_pStateManager->SetCullMode(mode); }

    MIL_FORCEINLINE HRESULT SetMultisampleAntialias(BOOL fEnable)
        { return m_pStateManager->SetMultisampleAntialias(fEnable); }

    MIL_FORCEINLINE HRESULT SetAntialiasedLineEnable(BOOL fEnable)
        { return m_pStateManager->SetAntialiasedLineEnable(fEnable); }

    MIL_FORCEINLINE HRESULT SetBlendFactor(D3DCOLOR color)
        { return m_pStateManager->SetBlendFactor(color); }

    MIL_FORCEINLINE HRESULT SetColorWriteMask(UINT8 mask)
        { return m_pStateManager->SetColorWriteMask(mask); }

    //
    // Sampler states (replace D3D9 SetSamplerState)
    //

    MIL_FORCEINLINE HRESULT SetSamplerAddressModes(
        DWORD dwSampler,
        D3D11_TEXTURE_ADDRESS_MODE u,
        D3D11_TEXTURE_ADDRESS_MODE v
        )
    {
        return m_pStateManager->SetSamplerAddressModes(dwSampler, u, v);
    }

    MIL_FORCEINLINE HRESULT SetSamplerBorderColor(
        DWORD dwSampler,
        D3DCOLOR color
        )
    {
        return m_pStateManager->SetSamplerBorderColor(dwSampler, color);
    }

    MIL_FORCEINLINE HRESULT SetTexCoordIndex(
        DWORD dwStage,
        UINT uTexCoordIndex
        )
    {
        return m_pStateManager->SetTexCoordIndex(dwStage, uTexCoordIndex);
    }

    MIL_FORCEINLINE HRESULT SetDefaultTexCoordIndices()
    {
        return m_pStateManager->SetDefaultTexCoordIndices();
    }

    MIL_FORCEINLINE HRESULT SetTexture(
        DWORD dwStage,
        __in_ecount_opt(1) ID3D11ShaderResourceView *pTexture
        )
    {
        return m_pStateManager->SetTextureInline(
            dwStage,
            pTexture
            );
    }

    MIL_FORCEINLINE HRESULT SetVertexShader(
        __in_ecount_opt(1) ID3D11VertexShader *pVertexShader
        )
    {
        return m_pStateManager->SetVertexShaderInline(
            pVertexShader
            );
    }

    MIL_FORCEINLINE HRESULT SetPixelShader(
        __in_ecount_opt(1) ID3D11PixelShader *pPixelShader
        )
    {
        return m_pStateManager->SetPixelShaderInline(
            pPixelShader
            );
    }

    MIL_FORCEINLINE HRESULT Define2DTransforms(
        __in_ecount(1) const CMatrix<CoordinateSpace::DeviceHPC,CoordinateSpace::D3DHomogeneousClipIPC> *pProjection
        )
    {
        return m_pStateManager->Define2DTransforms(
            pProjection
            );
    }

    MIL_FORCEINLINE HRESULT SetStreamSource(
        __in_ecount_opt(1) ID3D11Buffer *pStream,
        UINT uVertexStride
        )
    {
        return m_pStateManager->SetStreamSource(
            pStream,
            uVertexStride
            );
    }

    MIL_FORCEINLINE HRESULT SetIndices(
        __in_ecount_opt(1) ID3D11Buffer *pStream
        )
    {
        return m_pStateManager->SetIndices(
            pStream
            );
    }

    MIL_FORCEINLINE BOOL IsVertexFormatSet(
        MilVertexFormatId format
        ) const
    {
        return m_pStateManager->IsVertexFormatSet(format);
    }

    MIL_FORCEINLINE HRESULT SetVertexFormat(MilVertexFormatId format)
    {
        return m_pStateManager->SetVertexFormat(format);
    }

    MIL_FORCEINLINE HRESULT Set2DTransformForFixedFunction()
    {
        return m_pStateManager->Set2DTransformForFixedFunction();
    }

    MIL_FORCEINLINE HRESULT Set2DTransformForVertexShader(
        UINT uStartRegister
        )
    {
        return m_pStateManager->Set2DTransformForVertexShader(
            uStartRegister
            );
    }

    MIL_FORCEINLINE HRESULT Set3DTransformForVertexShader(
        UINT uStartRegister
        )
    {
        return m_pStateManager->Set3DTransformForVertexShader(
            uStartRegister
            );
    }

    MIL_FORCEINLINE MilPointAndSizeL GetClip() const
    {
        return m_pStateManager->GetClip();
    }

    MIL_FORCEINLINE BOOL IsClipSet() const
    {
        return m_pStateManager->IsClipSet();
    }

    MIL_FORCEINLINE BOOL IsClipSet(const MilPointAndSizeL *prcClip) const
    {
        return m_pStateManager->IsClipSet(prcClip);
    }

    MIL_FORCEINLINE void SetClip(MilPointAndSizeL rcClip)
    {
        m_pStateManager->SetClip(rcClip);
    }

    MIL_FORCEINLINE HRESULT SetViewport(
        __in_ecount(1) const MilPointAndSizeL *prcViewport
        )
    {
        return m_pStateManager->SetViewport(prcViewport);
    }

    MIL_FORCEINLINE MilPointAndSizeL GetViewport() const
    {
        return m_pStateManager->GetViewport();
    }

    MIL_FORCEINLINE void SetClipSet(BOOL fSet)
    {
        return m_pStateManager->SetClipSet(fSet);
    }

    MIL_FORCEINLINE void ScissorRectChanged(
        __in_ecount(1) const MilPointAndSizeL *prc
        )
    {
        return m_pStateManager->ScissorRectChanged(prc);
    }

    MIL_FORCEINLINE HRESULT SetScissorRect(
        __in_ecount_opt(1) const MilPointAndSizeL *prc
        )
    {
        return m_pStateManager->SetScissorRect(prc);
    }

    MIL_FORCEINLINE HRESULT SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
    {
        return m_pStateManager->SetPrimitiveTopology(topology);
    }

    // Bind all dirty state to the context. Must be called before every draw.
    MIL_FORCEINLINE HRESULT ApplyStateForDraw()
    {
        return m_pStateManager->ApplyStateForDraw();
    }

    // Context state is unknown (external code used the context).
    MIL_FORCEINLINE void InvalidateContextState()
    {
        m_pStateManager->InvalidateAll();
    }

    MIL_FORCEINLINE void UnbindAllTextures()
    {
        m_pStateManager->UnbindAllTextures();
    }

protected:
    //
    // Functions only for CD3DDeviceLevel1
    //

    bool IsDepthStencilSurfaceSmallerThan(
        UINT uWidth,
        UINT uHeight
        ) const
    {
        return m_pStateManager->IsDepthStencilSurfaceSmallerThan(uWidth, uHeight);
    }

    HRESULT SetRenderTargetInternal(
        __in_ecount_opt(1) CD3DSurface *pRenderTarget
        );

    HRESULT SetDepthStencilSurfaceInternal(
        __in_ecount_opt(1) CD3DSurface *pDepthStencilBuffer
        );

    HRESULT ReleaseUseOfDepthStencilSurfaceInternal(
        __in_ecount(1) CD3DSurface *pDepthStencilBuffer
        );

public:

    /////////////////////////////////////////////////
    //
    //   Low-Level Render State Setting Methods
    //
    //-----------------------------------------------
    HRESULT SetTextureStageOperation(
        DWORD dwStage,
        __in_ecount(1) const TextureStageOperation *ptsoNew
        );

    BOOL CanDrawText() const { return m_fCanDrawText; }
    BOOL CanDrawTextUsingPS20() const { return m_fCanDrawText; }

    HRESULT SetClearTypeOffsets(float ds, float dt);

public:

    // Render state variants
    static const AlphaBlendMode
        sc_abmSrcCopy,
        sc_abmDestCopy,
        sc_abmSrcOverPremultiplied,
        sc_abmSrcUnderPremultiplied,
        sc_abmSrcAlphaMultiply,
        sc_abmSrcInverseAlphaMultiply,
        sc_abmSrcOver_SrcNonPremultiplied,
        sc_abmSrcOver_InverseAlpha_SrcNonPremultiplied,
        sc_abmSrcVectorAlphaWithBlendFactor,
        sc_abmSrcVectorAlpha,
        sc_abmAddSourceColor,
        sc_abmSrcAlphaWithInvDestColor;

    static const FilterMode
        sc_fmNearest,
        sc_fmLinear,
        sc_fmTriLinear,
        sc_fmAnisotropic,
        sc_fmMinOnlyAnisotropic;

    static const TextureStageOperation
        sc_tsoDiffuse,
        sc_tsoTextureXCurrentRGB,
        sc_tsoTextureXSpecularRGB,
        sc_tsoSelectTexture,
        sc_tsoPremulTextureXCurrent,
        sc_tsoPremulTextureXDiffuse,
        sc_tsoOpaqueTextureXCurrent,
        sc_tsoOpaqueTextureXDiffuse,
        sc_tsoMaskTextureXCurrent,
        sc_tsoBumpMapTexture,
        sc_tsoColorSelectTextureAlphaMultiplyDiffuse,
        sc_tsoColorSelectTextureAlphaMultiplyCurrent,
        sc_tsoColorSelectDiffuseAlphaMultiplyTexture,
        sc_tsoColorSelectCurrentAlphaMultiplyTexture;

private:
    CHwRenderStateManager *m_pStateManager;
    CD3DDeviceLevel1 *m_pDeviceNoRef;

    // format of the textures used to represent glyph run shapes
    DXGI_FORMAT m_alphaTextureFormat;

    BOOL m_fCanDrawText;

    ID3D11PixelShader* m_pPixelShaders[PXS_NUM];
    const FilterMode* m_pTextFilterMode;

    HRESULT InitPixelShaders();

    HRESULT SetConstantRegisters_SolidBrush_PS20(
        DWORD dwColor,
        UINT gammaIndex
        );

    HRESULT SetConstantRegisters_TexturedBrush(
        UINT gammaIndex,
        float flEffectAlpha
        );

};
