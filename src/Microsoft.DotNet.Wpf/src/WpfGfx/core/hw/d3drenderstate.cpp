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

#include "precomp.hpp"

extern HINSTANCE g_DllInstance;

DeclareTag(tagDisableHWText, "MIL-HW", "Disable HW text rendering");
ExternTag(tagWireframe);

//-------------------------------------------------------------------------
// Back-end alpha-blend modes
//-------------------------------------------------------------------------

const AlphaBlendMode

// "SrcCopy". This mode can also be used to implement SrcOver when
// all input colors are opaque.

CD3DRenderState::sc_abmSrcCopy =
{
    FALSE,
    D3D11_BLEND_ONE,            // Unused
    D3D11_BLEND_ZERO,           // Unused
},

// "SrcOver" - the most common alpha blend. The source and destination use
// premultiplied alpha.

CD3DRenderState::sc_abmSrcOverPremultiplied =
{
    TRUE,
    D3D11_BLEND_ONE,
    D3D11_BLEND_INV_SRC_ALPHA,
},

// "SrcUnder" - not the most common, but merely the opposite of SrcOver.
// Both source and destination use premultiplied alpha.

CD3DRenderState::sc_abmSrcUnderPremultiplied =
{
    TRUE,
    D3D11_BLEND_INV_DEST_ALPHA,
    D3D11_BLEND_ONE,
},

// "SrcAlphaMultiply."  Multiplies the destination by the source
// alpha.  Ignores source color.

CD3DRenderState::sc_abmSrcAlphaMultiply =
{
    TRUE,
    D3D11_BLEND_ZERO,
    D3D11_BLEND_SRC_ALPHA,
},

// "SrcInverseAlphaMultiply."  Like SrcOver but without adding the source.

CD3DRenderState::sc_abmSrcInverseAlphaMultiply =
{
    TRUE,
    D3D11_BLEND_ZERO,
    D3D11_BLEND_INV_SRC_ALPHA,
},

// "SrcOver" but with the source color using non-premultiplied alpha. (The
// destination still uses premultiplied alpha).

CD3DRenderState::sc_abmSrcOver_SrcNonPremultiplied =
{
    TRUE,
    D3D11_BLEND_SRC_ALPHA,
    D3D11_BLEND_INV_SRC_ALPHA,
},

// "SrcOver" but with the source color using non-premultiplied alpha and
// inverting the alpha before using.  The destination still uses
// premultiplied alpha.

CD3DRenderState::sc_abmSrcOver_InverseAlpha_SrcNonPremultiplied =
{
    TRUE,
    D3D11_BLEND_INV_SRC_ALPHA,
    D3D11_BLEND_SRC_ALPHA,
},

// Source color is accepted as vector alpha;
// real color source comes from blend factor.
// This is used for text rendering in both clear type and grey scale mode.

// Note: ClearType blends do not preserve the destination alpha channel.

CD3DRenderState::sc_abmSrcVectorAlphaWithBlendFactor =
{
    TRUE,
    D3D11_BLEND_BLEND_FACTOR,
    D3D11_BLEND_INV_SRC_COLOR,
},

// Source color is accepted as vector alpha;
// no real color source yet involved (we'll add the color
// in a second pass, using sc_abmAddSourceColor blend mode).
CD3DRenderState::sc_abmSrcVectorAlpha =
{
    TRUE,
    D3D11_BLEND_ZERO,
    D3D11_BLEND_INV_SRC_COLOR,
},

// The source and destination are added together. Used in 2-pass ClearType, to
// add in the brush color.
//
// ClearType blends do not preserve the destination alpha channel.

CD3DRenderState::sc_abmAddSourceColor =
{
    TRUE,
    D3D11_BLEND_ONE,
    D3D11_BLEND_ONE,
},

CD3DRenderState::sc_abmSrcAlphaWithInvDestColor =
{
    TRUE,
    D3D11_BLEND_INV_DEST_COLOR,
    D3D11_BLEND_SRC_ALPHA,
};

//------------------------------------------------------------------------------
// Texture stage operations for the emulated fixed function pixel shader
//------------------------------------------------------------------------------

#define TSO(usesTex, cop, ca1, ca2, aop, aa1, aa2) \
    { usesTex, { cop, ca1, ca2, aop, aa1, aa2, 0 } }

const TextureStageOperation

CD3DRenderState::sc_tsoDiffuse =
    TSO(FALSE, MilTexOp_SelectArg1, MilTexArg_Diffuse, MilTexArg_Current,
               MilTexOp_SelectArg1, MilTexArg_Diffuse, MilTexArg_Current),

// Completely ignore argument 2 and take only the texture's
// values
CD3DRenderState::sc_tsoSelectTexture =
    TSO(TRUE,  MilTexOp_SelectArg1, MilTexArg_Texture, MilTexArg_Current,
               MilTexOp_SelectArg1, MilTexArg_Texture, MilTexArg_Current),

// This is the default D3D state for stage 0 and it is
// used by diffuse material to minimize the number of
// state changes
CD3DRenderState::sc_tsoTextureXCurrentRGB =
    TSO(TRUE,  MilTexOp_Modulate,   MilTexArg_Texture, MilTexArg_Current,
               MilTexOp_SelectArg1, MilTexArg_Texture, MilTexArg_Current),

// This is no longer used but we'll leave it here
// in case we ever put specular back in the vertex format
CD3DRenderState::sc_tsoTextureXSpecularRGB =
    TSO(TRUE,  MilTexOp_Modulate,   MilTexArg_Texture, MilTexArg_Specular,
               MilTexOp_SelectArg1, MilTexArg_Texture, MilTexArg_Current),

CD3DRenderState::sc_tsoPremulTextureXCurrent =
    TSO(TRUE,  MilTexOp_Modulate,   MilTexArg_Texture, MilTexArg_Current,
               MilTexOp_Modulate,   MilTexArg_Texture, MilTexArg_Current),

CD3DRenderState::sc_tsoPremulTextureXDiffuse =
    TSO(TRUE,  MilTexOp_Modulate,   MilTexArg_Texture, MilTexArg_Diffuse,
               MilTexOp_Modulate,   MilTexArg_Texture, MilTexArg_Current),

// Ignore texture's alpha by treating it as opaque, then
// modulate (multiply) by current (which for alpha is
// effectively select current alpha).
CD3DRenderState::sc_tsoOpaqueTextureXCurrent =
    TSO(TRUE,  MilTexOp_Modulate,   MilTexArg_Texture, MilTexArg_Current,
               MilTexOp_SelectArg1, MilTexArg_Current, MilTexArg_Current),

// Ignore texture's alpha by treating it as opaque, then
// modulate (multiply) by diffuse (which for alpha is
// effectively select diffuse alpha).
CD3DRenderState::sc_tsoOpaqueTextureXDiffuse =
    TSO(TRUE,  MilTexOp_Modulate,   MilTexArg_Texture, MilTexArg_Diffuse,
               MilTexOp_SelectArg1, MilTexArg_Current, MilTexArg_Current),

CD3DRenderState::sc_tsoMaskTextureXCurrent =
    TSO(TRUE,  MilTexOp_Modulate,   MilTexArg_TextureAlphaReplicate, MilTexArg_Current,
               MilTexOp_Modulate,   MilTexArg_Texture, MilTexArg_Current),

// Bump mapping (D3DTOP_BUMPENVMAP) is not emulated; the fixed function
// pipeline is never selected on feature level 10 devices.
CD3DRenderState::sc_tsoBumpMapTexture =
    TSO(TRUE,  MilTexOp_Modulate,   MilTexArg_Texture, MilTexArg_Current,
               MilTexOp_Modulate,   MilTexArg_Texture, MilTexArg_Current),

// For color selects the texture, for alpha multiplies texture and diffuse.
CD3DRenderState::sc_tsoColorSelectTextureAlphaMultiplyDiffuse =
    TSO(TRUE,  MilTexOp_SelectArg1, MilTexArg_Texture, MilTexArg_Diffuse,
               MilTexOp_Modulate,   MilTexArg_Texture, MilTexArg_Diffuse),

// For color selects the texture, for alpha multiplies texture and current.
CD3DRenderState::sc_tsoColorSelectTextureAlphaMultiplyCurrent =
    TSO(TRUE,  MilTexOp_SelectArg1, MilTexArg_Texture, MilTexArg_Current,
               MilTexOp_Modulate,   MilTexArg_Texture, MilTexArg_Current),

// For color selects the diffuse, for alpha multiplies texture and diffuse.
CD3DRenderState::sc_tsoColorSelectDiffuseAlphaMultiplyTexture =
    TSO(TRUE,  MilTexOp_SelectArg2, MilTexArg_Texture, MilTexArg_Diffuse,
               MilTexOp_Modulate,   MilTexArg_Texture, MilTexArg_Diffuse),

// For color selects the current, for alpha multiplies texture and current.
CD3DRenderState::sc_tsoColorSelectCurrentAlphaMultiplyTexture =
    TSO(TRUE,  MilTexOp_SelectArg2, MilTexArg_Texture, MilTexArg_Current,
               MilTexOp_Modulate,   MilTexArg_Texture, MilTexArg_Current);

#undef TSO

//-------------------------------------------------------------------------
// Texture Filter Modes
//-------------------------------------------------------------------------

const FilterMode

CD3DRenderState::sc_fmNearest =
{
    D3D11_FILTER_MIN_MAG_MIP_POINT,
    FALSE,
},

CD3DRenderState::sc_fmLinear =
{
    D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,
    FALSE,
},

CD3DRenderState::sc_fmTriLinear =
{
    D3D11_FILTER_MIN_MAG_MIP_LINEAR,
    TRUE,
},

CD3DRenderState::sc_fmAnisotropic =
{
    D3D11_FILTER_ANISOTROPIC,
    TRUE,
},

// D3D11 has no min-only anisotropic filter; anisotropic magnification is
// equivalent to linear magnification for our purposes.
CD3DRenderState::sc_fmMinOnlyAnisotropic =
{
    D3D11_FILTER_ANISOTROPIC,
    TRUE,
};

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::CD3DRenderState
//
//  Synopsis:
//      ctor
//
//------------------------------------------------------------------------------

CD3DRenderState::CD3DRenderState()
{
    for (int i = 0; i < ARRAYSIZE(m_pPixelShaders); i++)
    {
        m_pPixelShaders[i] = NULL;
    }

    m_pStateManager = NULL;
    m_pDeviceNoRef = NULL;
    m_alphaTextureFormat = DXGI_FORMAT_A8_UNORM;
    m_fCanDrawText = FALSE;
    m_pTextFilterMode = &sc_fmLinear;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::~CD3DRenderState
//
//  Synopsis:
//      dtor
//
//------------------------------------------------------------------------------

CD3DRenderState::~CD3DRenderState()
{
    for (int i = 0; i < ARRAYSIZE(m_pPixelShaders); i++)
    {
        ReleaseInterface(m_pPixelShaders[i]);
    }

    ReleaseInterface(m_pStateManager);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::GetFillMode
//
//  Synopsis:
//      Retrieves the current fillmode
//
//------------------------------------------------------------------------------
HRESULT
CD3DRenderState::GetFillMode(
    __out_ecount(1) D3D11_FILL_MODE *pFillMode
    ) const
{
    *pFillMode = m_pStateManager->GetFillMode();

    return S_OK;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::GetDepthTestFunction
//
//  Synopsis:
//      Retrieves the current depth test function
//
//------------------------------------------------------------------------------
HRESULT
CD3DRenderState::GetDepthTestFunction(
    __out_ecount(1) D3D11_COMPARISON_FUNC *pDepthTestFunction
    ) const
{
    *pDepthTestFunction = m_pStateManager->GetDepthFunc();

    return S_OK;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::ResetState
//
//  Synopsis:
//      This function resets all render state to the default values.
//
//------------------------------------------------------------------------------
HRESULT
CD3DRenderState::ResetState()
{
    return m_pStateManager->SetDefaultState();
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::Init
//
//  Synopsis:
//      Associated a d3d device for this manager
//
//------------------------------------------------------------------------------

HRESULT
CD3DRenderState::Init(
    __in_ecount(1) CD3DDeviceLevel1 *pDevice,
    __inout_ecount(1) ID3D11Device *pD3DDevice,
    __inout_ecount(1) ID3D11DeviceContext *pD3DContext
    )
{
    HRESULT hr = S_OK;

    Assert(pDevice && !m_pDeviceNoRef);

    //
    // Cache device
    //

    m_pDeviceNoRef = pDevice; // No AddRef because it would be a circular reference.

    IFC(CHwRenderStateManager::Create(
        pD3DDevice,
        pD3DContext,
        m_pDeviceNoRef->GetDeviceCaps(),
        &m_pStateManager
        ));

    m_pStateManager->InvalidateScissorRect();

    //
    // Every feature level 10 device supports the text pipeline:
    // A8 textures, blend factor and color write masks.
    //

    m_alphaTextureFormat = DXGI_FORMAT_A8_UNORM;

    m_fCanDrawText = !IsTagEnabled(tagDisableHWText);

    if (m_fCanDrawText)
    {
        IFC(InitPixelShaders());
    }

    m_pTextFilterMode = &sc_fmLinear;

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::SetFilterMode
//
//  Synopsis:
//      Set the filter mode for a given sampler.
//
//------------------------------------------------------------------------------

HRESULT CD3DRenderState::SetFilterMode(
    DWORD dwSampler,
    __in_ecount(1) const FilterMode *pfmNew
    )
{
    Assert(dwSampler < MIL_SAMPLER_COUNT);
    AssertMsg(pfmNew, "Trying to set an undefined filter mode");

    return m_pStateManager->SetSamplerFilter(
        dwSampler,
        pfmNew->Filter,
        pfmNew->UseMipMaps
        );
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::SetRenderTargetInternal
//
//  Synopsis:
//      Grabs the render target view and forwards it to the state manager.
//
//------------------------------------------------------------------------------
HRESULT
CD3DRenderState::SetRenderTargetInternal(
    __in_ecount_opt(1) CD3DSurface *pRenderTarget
    )
{
    HRESULT hr = S_OK;
    ID3D11RenderTargetView *pRTVNoRef = NULL;
    UINT uWidth = 0;
    UINT uHeight = 0;

    if (pRenderTarget != NULL)
    {
        Assert(pRenderTarget->IsValid());

        IFC(pRenderTarget->GetRenderTargetViewNoRef(&pRTVNoRef));

        pRenderTarget->GetSurfaceSize(&uWidth, &uHeight);
    }

    IFC(m_pStateManager->SetRenderTargetView(pRTVNoRef, uWidth, uHeight));

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::SetDepthStencilSurfaceInternal
//
//  Synopsis:
//      Grabs the d3d surface and then forwards the call to the state manager.
//
//------------------------------------------------------------------------------
HRESULT
CD3DRenderState::SetDepthStencilSurfaceInternal(
    __in_ecount_opt(1) CD3DSurface *pDepthStencilBuffer
    )
{
    HRESULT hr = S_OK;
    ID3D11DepthStencilView *pDSVNoRef = NULL;
    UINT uWidth = 0;
    UINT uHeight = 0;

    if (pDepthStencilBuffer != NULL)
    {
        Assert(pDepthStencilBuffer->IsValid());

        IFC(pDepthStencilBuffer->GetDepthStencilViewNoRef(&pDSVNoRef));

        Assert(pDSVNoRef);

        pDepthStencilBuffer->GetSurfaceSize(
            &uWidth,
            &uHeight
            );
    }

    IFC(m_pStateManager->SetDepthStencilView(
        pDSVNoRef,
        uWidth,
        uHeight
        ));

Cleanup:

    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::ReleaseUseOfDepthStencilSurfaceInternal
//
//  Synopsis:
//      Grabs the d3d surface and then forwards the call to the state manager.
//
//------------------------------------------------------------------------------
HRESULT
CD3DRenderState::ReleaseUseOfDepthStencilSurfaceInternal(
    __in_ecount(1) CD3DSurface *pDepthStencilBuffer
    )
{
    HRESULT hr = S_OK;

    ID3D11DepthStencilView *pDSVNoRef = pDepthStencilBuffer->GetDepthStencilViewNoRefNoCreate();

    if (pDSVNoRef)
    {
        IFC(m_pStateManager->ReleaseUseOfDepthStencilBuffer(
            pDSVNoRef
            ));
    }

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::SetAlphaBlendMode
//
//  Synopsis:
//      Set the alpha-blend mode.
//
//------------------------------------------------------------------------------

HRESULT CD3DRenderState::SetAlphaBlendMode(
    __in_ecount(1) const AlphaBlendMode *pabmNew
    )
{
    AssertMsg(pabmNew, "Trying to set an undefined blend mode");

    return m_pStateManager->SetAlphaBlend(
        pabmNew->AlphaBlendEnable,
        pabmNew->SrcBlend,
        pabmNew->DestBlend
        );
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::SetTextureStageOperation
//
//  Synopsis:
//      Set the texture operation for a given texture stage.
//
//------------------------------------------------------------------------------

HRESULT
CD3DRenderState::SetTextureStageOperation(
    DWORD dwStage,
    __in_ecount(1) const TextureStageOperation *ptsoNew
    )
{
    AssertMsg(ptsoNew, "Trying to set an undefined texture stage operation");
    Assert(dwStage < MIL_TEXTURE_STAGE_COUNT);

    HRESULT hr = S_OK;

    // Texture stages should not be disabled in this way - use
    // DisableTextureStage instead.
    Assert(ptsoNew->m_state.ColorOp != MilTexOp_Disable);

    IFC(m_pStateManager->SetTextureStageOperation(dwStage, ptsoNew->m_state));

    // If the new operation doesn't use a texture, we'll make sure that the
    // texture for this stage is set to NULL. Otherwise, the caller must set
    // the texture later using CD3DRenderState::SetTexture.

    if (!ptsoNew->m_fUsesTexture)
    {
        IFC(m_pStateManager->SetTextureInline(dwStage, NULL));
    }

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::SetConstantRegisters_SolidBrush_PS20()
//
//  Synopsis:
//      Adjust text pixel shader for gamma correction in solid brush mode.
//
//------------------------------------------------------------------------------
HRESULT
CD3DRenderState::SetConstantRegisters_SolidBrush_PS20(
    DWORD dwColor,
    UINT gammaIndex
    )
{
    float cregs[4][4];

    int ir = MIL_COLOR_GET_RED  (dwColor);
    int ig = MIL_COLOR_GET_GREEN(dwColor);
    int ib = MIL_COLOR_GET_BLUE (dwColor);
    int ia = MIL_COLOR_GET_ALPHA(dwColor);

    // average intensity
    int ii = (ir+ig+ig+ib)>>2;

    float fr = float(ir*(1./255));
    float fg = float(ig*(1./255));
    float fb = float(ib*(1./255));
    float fa = float(ia*(1./255));
    float fi = float(ii*(1./255));

    const GammaRatios& coefs = CGammaHandler::sc_gammaRatios[gammaIndex];

    cregs[0][0] = fa;
    cregs[0][1] = fa;
    cregs[0][2] = fa;
    cregs[0][3] = fa;

    cregs[1][0] = 1.f;
    cregs[1][1] = 1.f;
    cregs[1][2] = 1.f;
    cregs[1][3] = 1.f;

    cregs[2][0] = coefs.g1*fr + coefs.g2; // for clear type rendering
    cregs[2][1] = coefs.g1*fg + coefs.g2; // for clear type rendering
    cregs[2][2] = coefs.g1*fb + coefs.g2; // for clear type rendering
    cregs[2][3] = coefs.g1*fi + coefs.g2; // for grey scale rendering

    cregs[3][0] = coefs.g3*fr + coefs.g4; // for clear type rendering
    cregs[3][1] = coefs.g3*fg + coefs.g4; // for clear type rendering
    cregs[3][2] = coefs.g3*fb + coefs.g4; // for clear type rendering
    cregs[3][3] = coefs.g3*fi + coefs.g4; // for grey scale rendering

    return m_pStateManager->SetPixelShaderConstantF(2, cregs[0], 4);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::SetClearTypeOffsets()
//
//  Synopsis:
//      Adjust text pixel shader for clear type rendering. Given values are
//      texture coordinate offsets for sampling alpha for blue color component.
//      Green one is sampled without offset, red uses offsets opposite to blue.
//
//------------------------------------------------------------------------------
HRESULT
CD3DRenderState::SetClearTypeOffsets(float ds, float dt)
{
    float creg[4];

    creg[0] = ds;
    creg[1] = dt;
    creg[2] = 0.f;
    creg[3] = 0.f;

    return m_pStateManager->SetPixelShaderConstantF(1, creg, 1);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::SetConstantRegisters_TexturedBrush()
//
//  Synopsis:
//      Adjust pixel shader for gamma correction in textured brush mode.
//
//------------------------------------------------------------------------------
HRESULT
CD3DRenderState::SetConstantRegisters_TexturedBrush(
    UINT gammaIndex,
    float flEffectAlpha
    )
{
    float cregs[5][4];

    const GammaRatios& coefs = CGammaHandler::sc_gammaRatios[gammaIndex];

    cregs[0][0] =
    cregs[0][1] =
    cregs[0][2] = coefs.d4;
    cregs[0][3] = coefs.d1;

    cregs[1][0] =
    cregs[1][1] =
    cregs[1][2] = coefs.d5;
    cregs[1][3] = coefs.d2;

    cregs[2][0] =
    cregs[2][1] =
    cregs[2][2] = coefs.d6;
    cregs[2][3] = coefs.d2;

    cregs[3][0] = 1.f;
    cregs[3][1] = 1.f;
    cregs[3][2] = 1.f;
    cregs[3][3] = 1.f;

    cregs[4][0] = 0; // not in use
    cregs[4][1] = 0; // not in use
    cregs[4][2] = 0; // not in use
    cregs[4][3] = flEffectAlpha;

    return m_pStateManager->SetPixelShaderConstantF(1, cregs[0], 5);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::SetRenderState_AlphaSolidBrush
//
//  Synopsis:
//      Set up the pipeline to do regular SourceOver with a solid brush (brush
//      color supplied in the vertex diffuse color)
//
//------------------------------------------------------------------------------

HRESULT
CD3DRenderState::SetRenderState_AlphaSolidBrush()
{
    HRESULT hr = S_OK;

    IFC(SetAlphaBlendMode(&sc_abmSrcOverPremultiplied));
    IFC(SetPixelShader(NULL));
    IFC(SetVertexShader(NULL));

    IFC(SetTextureStageOperation(0, &sc_tsoDiffuse));
    IFC(m_pStateManager->DisableTextureStage(1));

    // FilterMode: Unused

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::SetRenderState_Texture
//
//  Synopsis:
//      Set up the pipeline to draw from a texture
//
//  blendMode notes:
//      TBM_COPY: for using when texels are fully opaque (or texture has no alpha)
//      TBM_APPLY_PREMULTIPLIED: most common case, assuming the texture has alpha
//                               and it's color values are already multiplied by alpha.
//      TBM_APPLY_VECTOR_ALPHA and TBM_ADD_COLORS:
//        these two guys are for doing the same as TBM_APPLY_PREMULTIPLIED do,
//        but when alpha values are vectors (i.e. alphaR, alphaG and alphaB).
//        Since normal texture can't keep six numbers per pixel, we are forced
//        to use two textures and do blending in two passes. First is controlled
//        by TBM_APPLY_VECTOR_ALPHA that accepts color values as alphas,
//        and the second by TBM_ADD_COLORS.
//
//------------------------------------------------------------------------------

HRESULT
CD3DRenderState::SetRenderState_Texture(
    TextureBlendMode blendMode,
    TextureBlendArgument eBlendArgument,
    MilBitmapInterpolationMode::Enum interpolationMode,
    UINT cMasks
    )
{
    HRESULT hr = S_OK;

    const FilterMode *pfm = NULL;
    const TextureStageOperation *ptso = NULL;
    const AlphaBlendMode *pabm = NULL;

    switch (interpolationMode)
    {
    case MilBitmapInterpolationMode::NearestNeighbor:
        pfm = &sc_fmNearest;
        break;

    case MilBitmapInterpolationMode::Linear:
        pfm = &sc_fmLinear;
        break;

    case MilBitmapInterpolationMode::TriLinear:
        pfm = &sc_fmTriLinear;
        break;

    default:
        AssertMsg(false, "MIL-HW: Unsupported interpolation mode.\n");
        IFC(E_FAIL);
    }

    switch (eBlendArgument)
    {
    case TBA_None:
        ptso = &sc_tsoSelectTexture;
        break;

    case TBA_Diffuse:
        ptso = &sc_tsoTextureXCurrentRGB;
        break;

    case TBA_Specular:
        ptso = &sc_tsoTextureXSpecularRGB;
        break;

    default:
        AssertMsg(false, "MIL_HW: Unsupported diffuse blend mode.\n");
    }

    switch(blendMode)
    {
    case TBM_COPY:
        pabm = &sc_abmSrcCopy;
        break;

    case TBM_APPLY_VECTOR_ALPHA:
        pabm = &sc_abmSrcVectorAlpha;
        break;

    case TBM_ADD_COLORS:
        pabm = &sc_abmAddSourceColor;
        break;

    default:
        Assert(blendMode == TBM_DEFAULT);
        pabm = &sc_abmSrcOverPremultiplied;
        break;

    }

    IFC(SetPixelShader(NULL));
    IFC(SetVertexShader(NULL));

    Assert(pfm);
    IFC(SetFilterMode(0, pfm));

    Assert(pabm);
    IFC(SetAlphaBlendMode(pabm));

    Assert(ptso);
    IFC(SetTextureStageOperation(0, ptso));
    IFC(m_pStateManager->SetTexCoordIndex(0, 0));

    Assert(cMasks + 1 <= m_pDeviceNoRef->GetMaxTextureBlendStages());
    for (UINT i = 1; i <= cMasks; i++)
    {
        // Same filter mode == correct behavior?
        IFC(SetFilterMode(i, pfm));

        IFC(SetTextureStageOperation(
            i,
            &sc_tsoMaskTextureXCurrent
            ));

        IFC(m_pStateManager->SetTexCoordIndex(i, i));
    }

    // disabling the color stage of texture stage cMasks+1 will disable all subsequent texture stages
    IFC(m_pStateManager->DisableTextureStage(cMasks+1));

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::InitPixelShaders
//
//  Synopsis:
//      create an array of pixel shaders
//
//------------------------------------------------------------------------------

HRESULT
CD3DRenderState::InitPixelShaders()
{
    HRESULT hr = S_OK;

    UINT rgResourceIds[PXS_NUM];

    rgResourceIds[PXS_CTSB] = PS_Text_CTSB;
    rgResourceIds[PXS_GSSB] = PS_Text_GSSB;
    rgResourceIds[PXS_CTTB] = PS_Text_CTTB;
    rgResourceIds[PXS_GSTB] = PS_Text_GSTB;

    C_ASSERT(ARRAYSIZE(m_pPixelShaders) == PXS_NUM);

    for (int i = 0; i < PXS_NUM; i++)
    {
        IFC(m_pDeviceNoRef->CreatePixelShaderFromResource(
            rgResourceIds[i],
            &m_pPixelShaders[i]
            ));
    }

Cleanup:
    return hr;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::SetRenderState_Text_ClearType_SolidBrush
//
//  Synopsis:
//      Set up the pipeline to do blend a glyph alpha-mask in clear type mode,
//      using solid brush
//
//------------------------------------------------------------------------------
HRESULT
CD3DRenderState::SetRenderState_Text_ClearType_SolidBrush(
    DWORD dwForegroundColor,
    UINT gammaIndex
)
{
    HRESULT hr = S_OK;

    IFC(SetPixelShader(m_pPixelShaders[PXS_CTSB]));
    IFC(SetVertexShader(NULL));

    IFC(SetAlphaBlendMode(&sc_abmSrcVectorAlphaWithBlendFactor));
    IFC(SetFilterMode(0, m_pTextFilterMode));

    //
    // The shader outputs alpha values rather than colors. We pass the brush
    // alpha to the shader in a constant register, and the shader then combines
    // that with the ClearType alphas to produce the 4 final alpha values for
    // the 4 channels (alpha included, so there's an "alpha alpha").
    //
    // The actual brush color is passed to the blend stage as the blend factor.
    // The blend mode used here uses the blend factor as the source coefficient
    // and (1 - source) as the destination coefficient, giving
    //     output = (BlendFactor)(Source) + (1 - Source)(Destination)
    //            = (Brush)(ShaderAlphas) + (1 - ShaderAlphas)(Destination)
    //
    // Note that the equation for "a" would double count the brush alpha. The
    // brush alpha is already included in the alpha values calculated by the
    // shader, so we force the value of "a_brush" passed in through the blend
    // factor to be 0xFF here to prevent double counting it.
    //
    IFC(SetBlendFactor(dwForegroundColor | 0xFF000000));

    IFC(SetConstantRegisters_SolidBrush_PS20(dwForegroundColor, gammaIndex));

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::SetRenderState_Text_ClearType_TextureBrush
//
//  Synopsis:
//      Set up the pipeline to do blend a glyph alpha-mask in clear type mode,
//      using textured brush
//
//------------------------------------------------------------------------------
HRESULT
CD3DRenderState::SetRenderState_Text_ClearType_TextureBrush(
    UINT gammaIndex,
    float flEffectAlpha
    )
{
    HRESULT hr = S_OK;

    IFC(SetPixelShader(m_pPixelShaders[PXS_CTTB]));
    IFC(SetVertexShader(NULL));

    IFC(SetConstantRegisters_TexturedBrush(gammaIndex, flEffectAlpha));

    IFC(SetAlphaBlendMode(&sc_abmSrcOverPremultiplied));

    IFC(SetFilterMode(0, &sc_fmLinear));
    IFC(SetFilterMode(1, m_pTextFilterMode));

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::SetRenderState_Text_GreyScale_SolidBrush
//
//  Synopsis:
//      Set up the pipeline to do blend a glyph alpha-mask in grey scale, using
//      solid brush
//
//------------------------------------------------------------------------------
HRESULT
CD3DRenderState::SetRenderState_Text_GreyScale_SolidBrush(
    DWORD dwForegroundColor,
    UINT gammaIndex
    )
{
    HRESULT hr = S_OK;

    IFC(SetPixelShader(m_pPixelShaders[PXS_GSSB]));
    IFC(SetVertexShader(NULL));

    IFC(SetAlphaBlendMode(&sc_abmSrcVectorAlphaWithBlendFactor));
    IFC(SetFilterMode(0, m_pTextFilterMode));

    // See SetRenderState_Text_ClearType_SolidBrush for the blend factor alpha.
    IFC(SetBlendFactor(dwForegroundColor | 0xFF000000));

    IFC(SetConstantRegisters_SolidBrush_PS20(dwForegroundColor, gammaIndex));

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::SetRenderState_Text_GreyScale_TextureBrush
//
//  Synopsis:
//      Set up the pipeline to do blend a glyph alpha-mask in grey scale mode,
//      using textured brush
//
//------------------------------------------------------------------------------
HRESULT
CD3DRenderState::SetRenderState_Text_GreyScale_TextureBrush(
    UINT gammaIndex,
    float flEffectAlpha
    )
{
    HRESULT hr = S_OK;

    IFC(SetPixelShader(m_pPixelShaders[PXS_GSTB]));
    IFC(SetVertexShader(NULL));

    IFC(SetAlphaBlendMode(&sc_abmSrcOverPremultiplied));

    static const float constantReg0[4] = {.25f, .5f, .25f, 0};
    IFC(m_pStateManager->SetPixelShaderConstantF(0, constantReg0, 1));

    IFC(SetConstantRegisters_TexturedBrush(gammaIndex, flEffectAlpha));

    IFC(SetFilterMode(0, &sc_fmLinear));
    IFC(SetFilterMode(1, m_pTextFilterMode));

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::SetColorChannelRed
//
//  Synopsis:
//      Set up the red-only color write mask to output rasterizer, set up
//      corresponding color mask to pixel shader constant register #0
//
//------------------------------------------------------------------------------
HRESULT
CD3DRenderState::SetColorChannelRed()
{
    HRESULT hr = S_OK;

    IFC(SetColorWriteMask(D3D11_COLOR_WRITE_ENABLE_RED));

    static const float constantReg0[4] = {1, 0, 0, 0};
    IFC(m_pStateManager->SetPixelShaderConstantF(0, constantReg0, 1));

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::SetColorChannelGreen
//
//------------------------------------------------------------------------------
HRESULT
CD3DRenderState::SetColorChannelGreen()
{
    HRESULT hr = S_OK;

    IFC(SetColorWriteMask(D3D11_COLOR_WRITE_ENABLE_GREEN));

    static const float constantReg0[4] = {0, 1, 0, 0};
    IFC(m_pStateManager->SetPixelShaderConstantF(0, constantReg0, 1));

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::SetColorChannelBlue
//
//------------------------------------------------------------------------------
HRESULT
CD3DRenderState::SetColorChannelBlue()
{
    HRESULT hr = S_OK;

    IFC(SetColorWriteMask(D3D11_COLOR_WRITE_ENABLE_BLUE));

    static const float constantReg0[4] = {0, 0, 1, 0};
    IFC(m_pStateManager->SetPixelShaderConstantF(0, constantReg0, 1));

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DRenderState::RestoreColorChannels
//
//  Synopsis:
//      Reset the color write mask to default state
//
//------------------------------------------------------------------------------
HRESULT
CD3DRenderState::RestoreColorChannels()
{
    HRESULT hr = S_OK;

    IFC(SetColorWriteMask(D3D11_COLOR_WRITE_ENABLE_ALL));

Cleanup:
    RRETURN(hr);
}
