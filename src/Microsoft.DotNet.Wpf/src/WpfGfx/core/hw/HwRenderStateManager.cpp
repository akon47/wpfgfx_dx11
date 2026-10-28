// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


#include "precomp.hpp"
//------------------------------------------------------------------------------
//
//  Description:
//      Contains CHwRenderStateManager implementation. See the header.
//

using namespace dxlayer;

MtDefine(CHwRenderStateManager, MILRender, "CHwRenderStateManager");
DeclareTag(tagWireframe, "MIL-HW", "Wireframe");

//+----------------------------------------------------------------------------
//
//  Function:   CHwRenderStateManager::Create
//
//  Synopsis:   Creates the Manager.
//
//-----------------------------------------------------------------------------
__checkReturn HRESULT
CHwRenderStateManager::Create(
    __inout_ecount(1) ID3D11Device *pD3DDevice,
    __inout_ecount(1) ID3D11DeviceContext *pD3DContext,
    __in_ecount(1) const MilDeviceCaps &caps,
    __deref_out_ecount(1) CHwRenderStateManager **ppStateManager
    )
{
    HRESULT hr = S_OK;
    CHwRenderStateManager *pNewStateManager = NULL;

    pNewStateManager = new CHwRenderStateManager;
    IFCOOM(pNewStateManager);

    IFC(pNewStateManager->Init(
        pD3DDevice,
        pD3DContext,
        caps
        ));

    pNewStateManager->AddRef();

    *ppStateManager = pNewStateManager;
    pNewStateManager = NULL;

Cleanup:
    ReleaseInterfaceNoNULL(pNewStateManager);

    RRETURN(hr);
}

//+----------------------------------------------------------------------------
//
//  Function:   CHwRenderStateManager::ctor
//
//-----------------------------------------------------------------------------
CHwRenderStateManager::CHwRenderStateManager()
{
    m_pD3DDevice = NULL;
    m_pD3DContext = NULL;
    ZeroMemory(&m_caps, sizeof(m_caps));

    m_pRTV = NULL;
    m_uRenderTargetWidth = 0;
    m_uRenderTargetHeight = 0;
    m_pDSV = NULL;
    m_uDepthStencilSurfaceWidth  = 0;
    m_uDepthStencilSurfaceHeight = 0;

    ZeroMemory(&m_blend, sizeof(m_blend));
    m_blendFactor = 0;
    m_fBlendDirty = true;

    ZeroMemory(&m_depthStencil, sizeof(m_depthStencil));
    m_uStencilRef = 0;
    m_fDepthStencilDirty = true;

    ZeroMemory(&m_rasterizer, sizeof(m_rasterizer));
    m_fRasterizerDirty = true;

    ZeroMemory(m_rgSamplers, sizeof(m_rgSamplers));
    ZeroMemory(m_rgfSamplerDirty, sizeof(m_rgfSamplerDirty));
    m_fAnySamplerDirty = true;

    ZeroMemory(m_rgpTextures, sizeof(m_rgpTextures));
    ZeroMemory(m_rgfTextureKnown, sizeof(m_rgfTextureKnown));
    m_fTexturesDirty = true;

    ZeroMemory(m_rgStages, sizeof(m_rgStages));
    m_cActiveStages = 0;
    m_fStagesDirty = true;

    m_pVertexShader = NULL;
    m_fVertexShaderKnown = false;
    m_fVertexShaderDirty = true;
    m_pPixelShader = NULL;
    m_fPixelShaderKnown = false;
    m_fPixelShaderDirty = true;
    m_pBoundVertexShader = NULL;
    m_pBoundPixelShader = NULL;

    ZeroMemory(m_rgVSFloat4, sizeof(m_rgVSFloat4));
    ZeroMemory(m_rgPSFloat4, sizeof(m_rgPSFloat4));
    ZeroMemory(m_rgPSInt4, sizeof(m_rgPSInt4));
    ZeroMemory(m_rgPSBool4, sizeof(m_rgPSBool4));
    m_fVSConstantsDirty = true;
    m_fPSConstantsDirty = true;
    m_fPSIntBoolConstantsDirty = true;

    m_pVSConstantBuffer = NULL;
    m_pPSConstantBuffer = NULL;
    m_pPSIntBoolConstantBuffer = NULL;
    m_pFFConstantBuffer = NULL;

    for (UINT i = 0; i < MilTransform_Count; i++)
    {
        m_rgTransforms[i] = IdentityMatrix;
    }
    m_fFFTransformDirty = true;

    m_f2DTransformsUsedForFixedFunction = FALSE;
    m_f2DTransformUsedForVertexShader = FALSE;
    m_u2DTransformVertexShaderStartRegister = 0;

    m_vertexFormat = MilVertexFormat_XYZDUV2;
    m_fVertexFormatKnown = false;
    m_fVertexFormatDirty = true;

    m_pVertexBuffer = NULL;
    m_pNullVertexBuffer = NULL;
    m_uVertexStride = 0;
    m_fVertexBufferKnown = false;
    m_fVertexBufferDirty = true;

    m_pIndexBuffer = NULL;
    m_fIndexBufferKnown = false;
    m_fIndexBufferDirty = true;

    m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    m_fTopologyDirty = true;

    ZeroMemory(&m_rcScissorRect, sizeof(m_rcScissorRect));
    m_fScissorRectValid = false;
    m_fScissorDirty = true;

    m_fClipSet = FALSE;
    ZeroMemory(&m_rcClip, sizeof(m_rcClip));

    ZeroMemory(&m_rcViewport, sizeof(m_rcViewport));
    m_fViewportDirty = true;

    m_blendCache.c = 0;
    m_depthStencilCache.c = 0;
    m_rasterizerCache.c = 0;
    m_samplerCache.c = 0;
}

//+------------------------------------------------------------------------
//
//  Member:     CHwRenderStateManager::dtor
//
//-------------------------------------------------------------------------
CHwRenderStateManager::~CHwRenderStateManager()
{
    m_blendCache.Release();
    m_depthStencilCache.Release();
    m_rasterizerCache.Release();
    m_samplerCache.Release();

    ReleaseInterface(m_pVSConstantBuffer);
    ReleaseInterface(m_pPSConstantBuffer);
    ReleaseInterface(m_pPSIntBoolConstantBuffer);
    ReleaseInterface(m_pFFConstantBuffer);
    ReleaseInterface(m_pNullVertexBuffer);

    ReleaseInterface(m_pD3DContext);
    ReleaseInterface(m_pD3DDevice);
}

//+----------------------------------------------------------------------------
//
//  Function:   CHwRenderStateManager::Init
//
//-----------------------------------------------------------------------------
HRESULT
CHwRenderStateManager::Init(
    __inout_ecount(1) ID3D11Device *pD3DDevice,
    __inout_ecount(1) ID3D11DeviceContext *pD3DContext,
    __in_ecount(1) const MilDeviceCaps &caps
    )
{
    HRESULT hr = S_OK;

    Assert(pD3DDevice && !m_pD3DDevice);

    m_pD3DDevice = pD3DDevice;
    m_pD3DDevice->AddRef();

    m_pD3DContext = pD3DContext;
    m_pD3DContext->AddRef();

    m_caps = caps;

    IFC(m_fixedFunction.Init(pD3DDevice));

    IFC(CreateConstantBuffers());

    IFC(SetDefaultState());

Cleanup:
    RRETURN(hr);
}

//+----------------------------------------------------------------------------
//
//  Function:   CHwRenderStateManager::CreateConstantBuffers
//
//-----------------------------------------------------------------------------
HRESULT
CHwRenderStateManager::CreateConstantBuffers()
{
    HRESULT hr = S_OK;

    D3D11_BUFFER_DESC desc;

    ZeroMemory(&desc, sizeof(desc));
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    desc.ByteWidth = sizeof(m_rgVSFloat4);
    IFC(MilTranslateD3D11Error(m_pD3DDevice->CreateBuffer(&desc, NULL, &m_pVSConstantBuffer)));

    desc.ByteWidth = sizeof(m_rgPSFloat4);
    IFC(MilTranslateD3D11Error(m_pD3DDevice->CreateBuffer(&desc, NULL, &m_pPSConstantBuffer)));

    desc.ByteWidth = sizeof(m_rgPSInt4) + sizeof(m_rgPSBool4);
    IFC(MilTranslateD3D11Error(m_pD3DDevice->CreateBuffer(&desc, NULL, &m_pPSIntBoolConstantBuffer)));

    desc.ByteWidth = sizeof(MilFixedFunctionVSConstants);
    IFC(MilTranslateD3D11Error(m_pD3DDevice->CreateBuffer(&desc, NULL, &m_pFFConstantBuffer)));

Cleanup:
    RRETURN(hr);
}

//+----------------------------------------------------------------------------
//
//  Function:   CHwRenderStateManager::UploadConstantBuffer
//
//-----------------------------------------------------------------------------
HRESULT
CHwRenderStateManager::UploadConstantBuffer(
    __in_ecount(1) ID3D11Buffer *pBuffer,
    __in_bcount(cb) const void *pvData,
    UINT cb
    )
{
    HRESULT hr = S_OK;

    D3D11_MAPPED_SUBRESOURCE mapped;

    IFC(MilTranslateD3D11Error(m_pD3DContext->Map(pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)));

    CopyMemory(mapped.pData, pvData, cb);

    m_pD3DContext->Unmap(pBuffer, 0);

Cleanup:
    RRETURN(hr);
}

//+----------------------------------------------------------------------------
//
//  Function:   CHwRenderStateManager::InvalidateAll
//
//  Synopsis:   외부가 컨텍스트 상태를 바꿨을 수 있을 때 모든 상태를 다시 올리게 한다.
//
//-----------------------------------------------------------------------------
void
CHwRenderStateManager::InvalidateAll()
{
    m_fBlendDirty = true;
    m_fDepthStencilDirty = true;
    m_fRasterizerDirty = true;
    m_fAnySamplerDirty = true;
    for (UINT i = 0; i < MIL_SAMPLER_COUNT; i++) { m_rgfSamplerDirty[i] = true; }
    m_fTexturesDirty = true;
    m_fStagesDirty = true;
    m_fVertexShaderDirty = true;
    m_fPixelShaderDirty = true;
    m_pBoundVertexShader = NULL;
    m_pBoundPixelShader = NULL;
    m_fVSConstantsDirty = true;
    m_fPSConstantsDirty = true;
    m_fPSIntBoolConstantsDirty = true;
    m_fFFTransformDirty = true;
    m_fVertexFormatDirty = true;
    m_fVertexBufferDirty = true;
    m_fIndexBufferDirty = true;
    m_fTopologyDirty = true;
    m_fScissorDirty = true;
    m_fViewportDirty = true;

    if (m_pRTV || m_pDSV)
    {
        m_pD3DContext->OMSetRenderTargets(1, &m_pRTV, m_pDSV);
    }
}

//+----------------------------------------------------------------------------
//
//  Render targets
//
//-----------------------------------------------------------------------------

HRESULT
CHwRenderStateManager::SetRenderTargetView(
    __in_ecount_opt(1) ID3D11RenderTargetView *pRTV,
    UINT uWidth,
    UINT uHeight
    )
{
    if (pRTV != m_pRTV)
    {
        //
        // D3D11 forbids a resource bound as a shader resource and as the render
        // target at the same time; the runtime silently unbinds the SRV. Drop
        // only the conflicting bindings from the shadow state so textures the
        // caller set before switching targets (effects do this) stay bound and
        // a later SetTexture of the dropped one binds it again.
        //
        if (pRTV)
        {
            ID3D11Resource *pRTResource = NULL;
            pRTV->GetResource(&pRTResource);

            //
            // Compare against what the context has bound (the shadow state may
            // hold not yet applied changes) and against the shadow state.
            //
            ID3D11ShaderResourceView *rgBound[MIL_TEXTURE_STAGE_COUNT] = { 0 };
            m_pD3DContext->PSGetShaderResources(0, MIL_TEXTURE_STAGE_COUNT, rgBound);

            for (UINT i = 0; i < MIL_TEXTURE_STAGE_COUNT; i++)
            {
                bool fBoundConflict = false;
                bool fShadowConflict = false;

                if (rgBound[i] != NULL)
                {
                    ID3D11Resource *pResource = NULL;
                    rgBound[i]->GetResource(&pResource);
                    fBoundConflict = (pResource == pRTResource);
                    ReleaseInterface(pResource);
                }

                if (m_rgpTextures[i] != NULL)
                {
                    ID3D11Resource *pResource = NULL;
                    m_rgpTextures[i]->GetResource(&pResource);
                    fShadowConflict = (pResource == pRTResource);
                    ReleaseInterface(pResource);
                }

                if (fBoundConflict)
                {
                    // Unbind this slot in the context now. A pending (shadow)
                    // texture for the slot is kept and applied at the next draw.
                    ID3D11ShaderResourceView *pNull = NULL;
                    m_pD3DContext->PSSetShaderResources(i, 1, &pNull);
                    m_rgfTextureKnown[i] = false;
                    m_fTexturesDirty = true;
                }

                if (fShadowConflict)
                {
                    // The caller must set a new texture before drawing.
                    m_rgpTextures[i] = NULL;
                    m_rgfTextureKnown[i] = false;
                    m_fTexturesDirty = true;
                }

                ReleaseInterface(rgBound[i]);
            }

            ReleaseInterface(pRTResource);
        }

        m_pRTV = pRTV;
        m_pD3DContext->OMSetRenderTargets(1, &m_pRTV, m_pDSV);
    }

    m_uRenderTargetWidth = uWidth;
    m_uRenderTargetHeight = uHeight;

    return S_OK;
}

HRESULT
CHwRenderStateManager::SetDepthStencilView(
    __in_ecount_opt(1) ID3D11DepthStencilView *pDSV,
    UINT uWidth,
    UINT uHeight
    )
{
    if (pDSV != m_pDSV)
    {
        m_pDSV = pDSV;
        m_pD3DContext->OMSetRenderTargets(1, &m_pRTV, m_pDSV);
    }

    m_uDepthStencilSurfaceWidth = uWidth;
    m_uDepthStencilSurfaceHeight = uHeight;

    return S_OK;
}

bool
CHwRenderStateManager::IsDepthStencilSurfaceSmallerThan(
    UINT uWidth,
    UINT uHeight
    ) const
{
    if (m_pDSV == NULL)
    {
        // No surface is set and therefore not smaller
        return false;
    }

    return (   (m_uDepthStencilSurfaceWidth < uWidth)
            || (m_uDepthStencilSurfaceHeight < uHeight));
}

HRESULT
CHwRenderStateManager::ReleaseUseOfDepthStencilBuffer(
    __in_ecount(1) ID3D11DepthStencilView *pDSV
    )
{
    if (m_pDSV == pDSV)
    {
        return SetDepthStencilView(NULL, 0, 0);
    }

    return S_OK;
}

void
CHwRenderStateManager::UnbindAllTextures()
{
    bool fAnyBound = false;

    for (UINT i = 0; i < MIL_TEXTURE_STAGE_COUNT; i++)
    {
        if (m_rgpTextures[i] != NULL)
        {
            fAnyBound = true;
        }
        m_rgpTextures[i] = NULL;
        m_rgfTextureKnown[i] = false;
    }

    if (fAnyBound)
    {
        ID3D11ShaderResourceView *rgNull[MIL_TEXTURE_STAGE_COUNT] = { 0 };
        m_pD3DContext->PSSetShaderResources(0, MIL_TEXTURE_STAGE_COUNT, rgNull);
    }

    m_fTexturesDirty = true;
}

//+----------------------------------------------------------------------------
//
//  Blend
//
//-----------------------------------------------------------------------------

HRESULT
CHwRenderStateManager::SetAlphaBlend(BOOL fEnable, D3D11_BLEND src, D3D11_BLEND dest)
{
    if (m_blend.Enable != fEnable || m_blend.SrcBlend != src || m_blend.DestBlend != dest)
    {
        m_blend.Enable = fEnable;
        m_blend.SrcBlend = src;
        m_blend.DestBlend = dest;
        m_fBlendDirty = true;
    }
    return S_OK;
}

HRESULT
CHwRenderStateManager::SetBlendFactor(D3DCOLOR color)
{
    if (m_blendFactor != color)
    {
        m_blendFactor = color;
        m_fBlendDirty = true;
    }
    return S_OK;
}

HRESULT
CHwRenderStateManager::SetColorWriteMask(UINT8 mask)
{
    if (m_blend.WriteMask != mask)
    {
        m_blend.WriteMask = mask;
        m_fBlendDirty = true;
    }
    return S_OK;
}

//+----------------------------------------------------------------------------
//
//  Depth / stencil
//
//-----------------------------------------------------------------------------

HRESULT CHwRenderStateManager::SetDepthEnable(BOOL fEnable)
{
    if (m_depthStencil.DepthEnable != fEnable) { m_depthStencil.DepthEnable = fEnable; m_fDepthStencilDirty = true; }
    return S_OK;
}

HRESULT CHwRenderStateManager::SetDepthWriteEnable(BOOL fEnable)
{
    if (m_depthStencil.DepthWriteEnable != fEnable) { m_depthStencil.DepthWriteEnable = fEnable; m_fDepthStencilDirty = true; }
    return S_OK;
}

HRESULT CHwRenderStateManager::SetDepthFunc(D3D11_COMPARISON_FUNC func)
{
    if (m_depthStencil.DepthFunc != func) { m_depthStencil.DepthFunc = func; m_fDepthStencilDirty = true; }
    return S_OK;
}

HRESULT CHwRenderStateManager::SetStencilEnable(BOOL fEnable)
{
    if (m_depthStencil.StencilEnable != fEnable) { m_depthStencil.StencilEnable = fEnable; m_fDepthStencilDirty = true; }
    return S_OK;
}

HRESULT CHwRenderStateManager::SetStencilFunc(D3D11_COMPARISON_FUNC func)
{
    if (m_depthStencil.StencilFunc != func) { m_depthStencil.StencilFunc = func; m_fDepthStencilDirty = true; }
    return S_OK;
}

HRESULT CHwRenderStateManager::SetStencilRef(UINT uRef)
{
    if (m_uStencilRef != uRef) { m_uStencilRef = uRef; m_fDepthStencilDirty = true; }
    return S_OK;
}

HRESULT CHwRenderStateManager::SetStencilWriteMask(UINT8 mask)
{
    if (m_depthStencil.StencilWriteMask != mask) { m_depthStencil.StencilWriteMask = mask; m_fDepthStencilDirty = true; }
    return S_OK;
}

//+----------------------------------------------------------------------------
//
//  Rasterizer
//
//-----------------------------------------------------------------------------

HRESULT CHwRenderStateManager::SetFillMode(D3D11_FILL_MODE mode)
{
#if DBG
    if (IsTagEnabled(tagWireframe))
    {
        mode = D3D11_FILL_WIREFRAME;
    }
#endif
    if (m_rasterizer.FillMode != mode) { m_rasterizer.FillMode = mode; m_fRasterizerDirty = true; }
    return S_OK;
}

HRESULT CHwRenderStateManager::SetCullMode(D3D11_CULL_MODE mode)
{
    if (m_rasterizer.CullMode != mode) { m_rasterizer.CullMode = mode; m_fRasterizerDirty = true; }
    return S_OK;
}

HRESULT CHwRenderStateManager::SetMultisampleAntialias(BOOL fEnable)
{
    if (m_rasterizer.MultisampleEnable != fEnable) { m_rasterizer.MultisampleEnable = fEnable; m_fRasterizerDirty = true; }
    return S_OK;
}

HRESULT CHwRenderStateManager::SetAntialiasedLineEnable(BOOL fEnable)
{
    if (m_rasterizer.AntialiasedLineEnable != fEnable) { m_rasterizer.AntialiasedLineEnable = fEnable; m_fRasterizerDirty = true; }
    return S_OK;
}

//+----------------------------------------------------------------------------
//
//  Samplers
//
//-----------------------------------------------------------------------------

HRESULT CHwRenderStateManager::SetSamplerFilter(DWORD dwSampler, D3D11_FILTER filter, BOOL fUseMipMaps)
{
    Assert(dwSampler < MIL_SAMPLER_COUNT);
    if (m_rgSamplers[dwSampler].Filter != filter || m_rgSamplers[dwSampler].UseMipMaps != fUseMipMaps)
    {
        m_rgSamplers[dwSampler].Filter = filter;
        m_rgSamplers[dwSampler].UseMipMaps = fUseMipMaps;
        m_rgfSamplerDirty[dwSampler] = true;
        m_fAnySamplerDirty = true;
    }
    return S_OK;
}

HRESULT CHwRenderStateManager::SetSamplerAddressModes(DWORD dwSampler, D3D11_TEXTURE_ADDRESS_MODE u, D3D11_TEXTURE_ADDRESS_MODE v)
{
    Assert(dwSampler < MIL_SAMPLER_COUNT);
    if (m_rgSamplers[dwSampler].AddressU != u || m_rgSamplers[dwSampler].AddressV != v)
    {
        m_rgSamplers[dwSampler].AddressU = u;
        m_rgSamplers[dwSampler].AddressV = v;
        m_rgfSamplerDirty[dwSampler] = true;
        m_fAnySamplerDirty = true;
    }
    return S_OK;
}

HRESULT CHwRenderStateManager::SetSamplerBorderColor(DWORD dwSampler, D3DCOLOR color)
{
    Assert(dwSampler < MIL_SAMPLER_COUNT);
    if (m_rgSamplers[dwSampler].BorderColor != color)
    {
        m_rgSamplers[dwSampler].BorderColor = color;
        m_rgfSamplerDirty[dwSampler] = true;
        m_fAnySamplerDirty = true;
    }
    return S_OK;
}

//+----------------------------------------------------------------------------
//
//  Fixed function texture stages
//
//-----------------------------------------------------------------------------

HRESULT
CHwRenderStateManager::SetTextureStageOperation(
    DWORD dwStage,
    __in_ecount(1) const MilTextureStageState &op
    )
{
    Assert(dwStage < MIL_FF_STAGE_COUNT);
    Assert(op.ColorOp != MilTexOp_Disable);

    MilTextureStageState newState = op;
    newState.TexCoordIndex = m_rgStages[dwStage].TexCoordIndex;

    if (m_rgStages[dwStage] != newState)
    {
        m_rgStages[dwStage] = newState;
        m_fStagesDirty = true;
    }

    if (dwStage >= m_cActiveStages)
    {
        m_cActiveStages = dwStage + 1;
        m_fStagesDirty = true;
    }

    return S_OK;
}

HRESULT
CHwRenderStateManager::DisableTextureStage(DWORD dwStage)
{
    Assert(dwStage <= MIL_FF_STAGE_COUNT);

    if (dwStage < m_cActiveStages)
    {
        m_cActiveStages = dwStage;
        m_fStagesDirty = true;
    }

    return S_OK;
}

HRESULT
CHwRenderStateManager::SetTexCoordIndex(DWORD dwStage, UINT uTexCoordIndex)
{
    Assert(dwStage < MIL_FF_STAGE_COUNT);

    if (m_rgStages[dwStage].TexCoordIndex != uTexCoordIndex)
    {
        m_rgStages[dwStage].TexCoordIndex = uTexCoordIndex;
        m_fStagesDirty = true;
    }

    return S_OK;
}

HRESULT
CHwRenderStateManager::SetDefaultTexCoordIndices()
{
    for (UINT i = 0; i < MIL_FF_STAGE_COUNT; i++)
    {
        if (m_rgStages[i].TexCoordIndex != i)
        {
            m_rgStages[i].TexCoordIndex = i;
            m_fStagesDirty = true;
        }
    }

    return S_OK;
}

//+------------------------------------------------------------------------
//
//  Shader constants
//
//-------------------------------------------------------------------------

void
CHwRenderStateManager::MarkVSConstantsDirty(UINT nRegisterIndex, UINT nRegisterCount)
{
    UNREFERENCED_PARAMETER(nRegisterIndex);
    UNREFERENCED_PARAMETER(nRegisterCount);
    m_fVSConstantsDirty = true;
}

void
CHwRenderStateManager::MarkPSConstantsDirty(UINT nRegisterIndex, UINT nRegisterCount)
{
    UNREFERENCED_PARAMETER(nRegisterIndex);
    UNREFERENCED_PARAMETER(nRegisterCount);
    m_fPSConstantsDirty = true;
}

//+----------------------------------------------------------------------------
//
//  Function:   CHwRenderStateManager::Check2DTransformInVertexShader
//
//  Synopsis:   Check to see if the shader constants being set are overwriting
//              the 2d transform currently stored.
//
//-----------------------------------------------------------------------------
void
CHwRenderStateManager::Check2DTransformInVertexShader(
    UINT nRegisterIndex,
    UINT nRegisterCount
    )
{
    //
    // If the registers being set intersect with the 2d transform range, invalidate
    // the transform.
    //
    if (   nRegisterIndex + nRegisterCount > m_u2DTransformVertexShaderStartRegister
        && nRegisterIndex < m_u2DTransformVertexShaderStartRegister +
                ShaderConstantTraits<ShaderFunctionConstantData::Matrix4x4>::RegisterSize
           )
    {
        m_f2DTransformUsedForVertexShader = FALSE;
    }
}

HRESULT
CHwRenderStateManager::SetVertexShaderConstantF(
    UINT nRegisterIndex,
    __in_ecount(4*nRegisterCount) const float * prConstantData,
    UINT nRegisterCount
    )
{
    HRESULT hr = S_OK;

    if (nRegisterIndex + nRegisterCount > MIL_VS_FLOAT4_CONSTANTS)
    {
        IFC(WGXERR_INVALIDPARAMETER);
    }

    Check2DTransformInVertexShader(nRegisterIndex, nRegisterCount);

    {
        float *pDest = &m_rgVSFloat4[nRegisterIndex * 4];
        size_t cb = sizeof(float) * 4 * nRegisterCount;

        if (memcmp(pDest, prConstantData, cb) != 0)
        {
            CopyMemory(pDest, prConstantData, cb);
            MarkVSConstantsDirty(nRegisterIndex, nRegisterCount);
        }
    }

Cleanup:
    RRETURN(hr);
}

HRESULT
CHwRenderStateManager::SetPixelShaderConstantF(
    UINT nRegisterIndex,
    __in_ecount(4*nRegisterCount) const float * prConstantData,
    UINT nRegisterCount
    )
{
    HRESULT hr = S_OK;

    if (nRegisterIndex + nRegisterCount > MIL_PS_FLOAT4_CONSTANTS)
    {
        IFC(WGXERR_INVALIDPARAMETER);
    }

    {
        float *pDest = &m_rgPSFloat4[nRegisterIndex * 4];
        size_t cb = sizeof(float) * 4 * nRegisterCount;

        if (memcmp(pDest, prConstantData, cb) != 0)
        {
            CopyMemory(pDest, prConstantData, cb);
            MarkPSConstantsDirty(nRegisterIndex, nRegisterCount);
        }
    }

Cleanup:
    RRETURN(hr);
}

HRESULT
CHwRenderStateManager::SetPixelShaderConstantI(
    UINT nRegisterIndex,
    __in_ecount(4) const int * pConstantData
    )
{
    HRESULT hr = S_OK;

    if (nRegisterIndex >= MIL_PS_INT4_CONSTANTS)
    {
        IFC(WGXERR_INVALIDPARAMETER);
    }

    {
        int *pDest = &m_rgPSInt4[nRegisterIndex * 4];

        if (memcmp(pDest, pConstantData, sizeof(int) * 4) != 0)
        {
            CopyMemory(pDest, pConstantData, sizeof(int) * 4);
            m_fPSIntBoolConstantsDirty = true;
        }
    }

Cleanup:
    RRETURN(hr);
}

HRESULT
CHwRenderStateManager::SetPixelShaderConstantB(
    UINT nRegisterIndex,
    BOOL constantData
    )
{
    HRESULT hr = S_OK;

    if (nRegisterIndex >= MIL_PS_BOOL_CONSTANTS)
    {
        IFC(WGXERR_INVALIDPARAMETER);
    }

    {
        int *pDest = &m_rgPSBool4[nRegisterIndex * 4];
        int value = constantData ? 1 : 0;

        if (pDest[0] != value)
        {
            pDest[0] = pDest[1] = pDest[2] = pDest[3] = value;
            m_fPSIntBoolConstantsDirty = true;
        }
    }

Cleanup:
    RRETURN(hr);
}

//+------------------------------------------------------------------------
//
//  Transforms
//
//-------------------------------------------------------------------------

HRESULT
CHwRenderStateManager::SetTransform(
    MilTransformState state,
    __in_ecount(1) const CBaseMatrix *pMatrix
    )
{
    Assert(state < MilTransform_Count);

    CMILMatrix const &mat = *reinterpret_cast<CMILMatrix const *>(pMatrix);

    if (!(m_rgTransforms[state] == mat))
    {
        m_rgTransforms[state] = mat;
        m_fFFTransformDirty = true;
        m_f2DTransformsUsedForFixedFunction = FALSE;
        m_f2DTransformUsedForVertexShader = FALSE;
    }

    return S_OK;
}

HRESULT
CHwRenderStateManager::GetTransform(
    MilTransformState state,
    __out_ecount(1) CMILMatrix *pMatrix
    ) const
{
    Assert(state < MilTransform_Count);

    *pMatrix = m_rgTransforms[state];

    return S_OK;
}

HRESULT
CHwRenderStateManager::Define2DTransforms(
    __in_ecount(1) const CMatrix<CoordinateSpace::DeviceHPC,CoordinateSpace::D3DHomogeneousClipIPC> *pProjection
    )
{
    m_mat2DProjectionTransform = *pProjection;

    m_f2DTransformsUsedForFixedFunction = FALSE;
    m_f2DTransformUsedForVertexShader = FALSE;

    m_u2DTransformVertexShaderStartRegister = 0xffffffff;

    return S_OK;
}

HRESULT
CHwRenderStateManager::Set2DTransformForFixedFunction()
{
    HRESULT hr = S_OK;

    if (!m_f2DTransformsUsedForFixedFunction)
    {
        IFC(SetTransform(MilTransform_World, &IdentityMatrix));
        IFC(SetTransform(MilTransform_View, &IdentityMatrix));
        IFC(SetTransform(MilTransform_Projection, &m_mat2DProjectionTransform));

        m_f2DTransformsUsedForFixedFunction = TRUE;
    }

Cleanup:
    RRETURN(hr);
}

HRESULT
CHwRenderStateManager::Set2DTransformForVertexShader(
    UINT uStartRegister
    )
{
    HRESULT hr = S_OK;

    if (   !m_f2DTransformUsedForVertexShader
        || uStartRegister != m_u2DTransformVertexShaderStartRegister
           )
    {
        CMILMatrix matShaderTransform;

        //
        // D3D HLSL interprets transforms differently then fixed function,
        // so we need to transpose the matrix.
        //
        matShaderTransform = m_mat2DProjectionTransform.transpose();

        IFC(SetVertexShaderConstantF(
            uStartRegister,
            reinterpret_cast<const float *>(&matShaderTransform),
            4
            ));

        m_u2DTransformVertexShaderStartRegister = uStartRegister;
        m_f2DTransformUsedForVertexShader = TRUE;
    }

Cleanup:
    RRETURN(hr);
}

HRESULT
CHwRenderStateManager::Set3DTransformForVertexShader(
    UINT uStartRegister
    )
{
    HRESULT hr = S_OK;
    CMILMatrix matResult, matWorldView;

    // NOTE: must be in the same order as the Get3DTransforms shader
    //       pipeline item

    const UINT uNumRegisters = 4;
    UINT uRegisterIndex = uStartRegister;

    // Calculate WorldView
    matWorldView = m_rgTransforms[MilTransform_World] * m_rgTransforms[MilTransform_View];

    // Send WorldView
    {
        matResult = matWorldView.transpose();

        IFC(SetVertexShaderConstantF(
            uRegisterIndex,
            reinterpret_cast<const float *>(&matResult),
            uNumRegisters
            ));

        uRegisterIndex += uNumRegisters;
    }

    // Send WorldViewProj
    {
        matResult = matWorldView.multiply_transpose(m_rgTransforms[MilTransform_Projection]);

        IFC(SetVertexShaderConstantF(
            uRegisterIndex,
            reinterpret_cast<const float *>(&matResult),
            uNumRegisters
            ));

        uRegisterIndex += uNumRegisters;
    }

    // Send normal transform matrix.
    // Computed with more description in CMILMesh3D::PrecomputeLighting
    {
        MILMatrixAdjoint(&matResult, &matWorldView);
        matResult *= (matWorldView.GetDeterminant3D() < 0.f) ? -1.f : 1.f;

        // Since we normally transpose before sending a matrix,
        // NOT transposing gives us what we want

        IFC(SetVertexShaderConstantF(
            uRegisterIndex,
            reinterpret_cast<const float *>(&matResult),
            uNumRegisters
            ));
    }

Cleanup:
    RRETURN(hr);
}

//+------------------------------------------------------------------------
//
//  Input assembler
//
//-------------------------------------------------------------------------

HRESULT
CHwRenderStateManager::SetVertexFormat(MilVertexFormatId format)
{
    Assert(format < MilVertexFormat_Count);

    if (!m_fVertexFormatKnown || m_vertexFormat != format)
    {
        m_vertexFormat = format;
        m_fVertexFormatKnown = true;
        m_fVertexFormatDirty = true;

        // 고정 기능 VS는 형식에 따라 다르다
        if (m_pVertexShader == NULL)
        {
            m_fVertexShaderDirty = true;
        }
    }

    return S_OK;
}

HRESULT
CHwRenderStateManager::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
{
    if (m_topology != topology || m_fTopologyDirty)
    {
        m_topology = topology;
        m_fTopologyDirty = true;
    }

    return S_OK;
}

//+-----------------------------------------------------------------------------
//
//  Scissor rect and viewport
//
//------------------------------------------------------------------------------

void
CHwRenderStateManager::InvalidateScissorRect()
{
    m_fScissorRectValid = false;
    ZeroMemory(&m_rcScissorRect, sizeof(m_rcScissorRect));
    m_fScissorDirty = true;
}

void
CHwRenderStateManager::ScissorRectChanged(
    __in_ecount(1) const MilPointAndSizeL *prc
    )
{
    RtlCopyMemory(&m_rcScissorRect, prc, sizeof(m_rcScissorRect));
    m_fScissorRectValid = true;
    m_fScissorDirty = true;
}

HRESULT
CHwRenderStateManager::SetScissorRect(
    __in_ecount_opt(1) const MilPointAndSizeL *prc
    )
{
    Assert(prc == NULL ||
           prc->Width > 0 &&
           prc->Height > 0);

    if (prc != NULL)
    {
        if (!m_fScissorRectValid || !RtlEqualMemory(prc, &m_rcScissorRect, sizeof(*prc)))
        {
            m_rcScissorRect = *prc;
            m_fScissorRectValid = true;
            m_fScissorDirty = true;
        }

        if (!m_rasterizer.ScissorEnable)
        {
            m_rasterizer.ScissorEnable = TRUE;
            m_fRasterizerDirty = true;
        }
    }
    else
    {
        if (m_rasterizer.ScissorEnable)
        {
            m_rasterizer.ScissorEnable = FALSE;
            m_fRasterizerDirty = true;
        }
    }

    return S_OK;
}

HRESULT
CHwRenderStateManager::SetViewport(
    __in_ecount(1) const MilPointAndSizeL *prcViewport
    )
{
    m_rcViewport = *prcViewport;
    m_fViewportDirty = true;

    return S_OK;
}

//+------------------------------------------------------------------------
//
//  Apply
//
//-------------------------------------------------------------------------

HRESULT
CHwRenderStateManager::ApplyBlendState()
{
    HRESULT hr = S_OK;

    ID3D11BlendState *pState = m_blendCache.Find(m_blend);

    if (pState == NULL)
    {
        D3D11_BLEND_DESC desc;

        ZeroMemory(&desc, sizeof(desc));
        desc.AlphaToCoverageEnable = FALSE;
        desc.IndependentBlendEnable = FALSE;
        desc.RenderTarget[0].BlendEnable = m_blend.Enable;
        desc.RenderTarget[0].SrcBlend = m_blend.Enable ? m_blend.SrcBlend : D3D11_BLEND_ONE;
        desc.RenderTarget[0].DestBlend = m_blend.Enable ? m_blend.DestBlend : D3D11_BLEND_ZERO;
        desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        // D3D9의 단일 블렌드 모드는 알파 채널에도 같은 인자를 쓴다. 색 전용 인자는
        // 알파에 맞는 것으로 바꾼다.
        desc.RenderTarget[0].SrcBlendAlpha = desc.RenderTarget[0].SrcBlend;
        desc.RenderTarget[0].DestBlendAlpha = desc.RenderTarget[0].DestBlend;
        if (desc.RenderTarget[0].SrcBlendAlpha == D3D11_BLEND_SRC_COLOR) desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
        if (desc.RenderTarget[0].SrcBlendAlpha == D3D11_BLEND_INV_SRC_COLOR) desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        if (desc.RenderTarget[0].SrcBlendAlpha == D3D11_BLEND_DEST_COLOR) desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_DEST_ALPHA;
        if (desc.RenderTarget[0].SrcBlendAlpha == D3D11_BLEND_INV_DEST_COLOR) desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
        if (desc.RenderTarget[0].DestBlendAlpha == D3D11_BLEND_SRC_COLOR) desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_SRC_ALPHA;
        if (desc.RenderTarget[0].DestBlendAlpha == D3D11_BLEND_INV_SRC_COLOR) desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        if (desc.RenderTarget[0].DestBlendAlpha == D3D11_BLEND_DEST_COLOR) desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
        if (desc.RenderTarget[0].DestBlendAlpha == D3D11_BLEND_INV_DEST_COLOR) desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
        desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].RenderTargetWriteMask = m_blend.WriteMask;

        IFC(MilTranslateD3D11Error(m_pD3DDevice->CreateBlendState(&desc, &pState)));

        m_blendCache.Add(m_blend, pState);
        pState->Release();  // cache holds the reference
    }

    {
        float rgFactor[4];
        rgFactor[0] = MIL_COLOR_GET_RED(m_blendFactor) / 255.0f;
        rgFactor[1] = MIL_COLOR_GET_GREEN(m_blendFactor) / 255.0f;
        rgFactor[2] = MIL_COLOR_GET_BLUE(m_blendFactor) / 255.0f;
        rgFactor[3] = MIL_COLOR_GET_ALPHA(m_blendFactor) / 255.0f;

        m_pD3DContext->OMSetBlendState(pState, rgFactor, 0xffffffff);
    }

    m_fBlendDirty = false;

Cleanup:
    RRETURN(hr);
}

HRESULT
CHwRenderStateManager::ApplyDepthStencilState()
{
    HRESULT hr = S_OK;

    ID3D11DepthStencilState *pState = m_depthStencilCache.Find(m_depthStencil);

    if (pState == NULL)
    {
        D3D11_DEPTH_STENCIL_DESC desc;

        ZeroMemory(&desc, sizeof(desc));
        desc.DepthEnable = m_depthStencil.DepthEnable;
        desc.DepthWriteMask = m_depthStencil.DepthWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.DepthFunc = m_depthStencil.DepthFunc;
        desc.StencilEnable = m_depthStencil.StencilEnable;
        desc.StencilReadMask = m_depthStencil.StencilReadMask;
        desc.StencilWriteMask = m_depthStencil.StencilWriteMask;
        desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilFunc = m_depthStencil.StencilFunc;
        desc.BackFace = desc.FrontFace;

        IFC(MilTranslateD3D11Error(m_pD3DDevice->CreateDepthStencilState(&desc, &pState)));

        m_depthStencilCache.Add(m_depthStencil, pState);
        pState->Release();
    }

    m_pD3DContext->OMSetDepthStencilState(pState, m_uStencilRef);

    m_fDepthStencilDirty = false;

Cleanup:
    RRETURN(hr);
}

HRESULT
CHwRenderStateManager::ApplyRasterizerState()
{
    HRESULT hr = S_OK;

    ID3D11RasterizerState *pState = m_rasterizerCache.Find(m_rasterizer);

    if (pState == NULL)
    {
        D3D11_RASTERIZER_DESC desc;

        ZeroMemory(&desc, sizeof(desc));
        desc.FillMode = m_rasterizer.FillMode;
        desc.CullMode = m_rasterizer.CullMode;
        // D3D9 D3DCULL_CW culls clockwise triangles; D3D11 with FrontCounterClockwise=FALSE
        // treats clockwise as front. Keep the D3D9 meaning: CW == cull front faces.
        desc.FrontCounterClockwise = FALSE;
        desc.DepthBias = 0;
        desc.DepthBiasClamp = 0.0f;
        desc.SlopeScaledDepthBias = 0.0f;
        desc.DepthClipEnable = TRUE;
        desc.ScissorEnable = m_rasterizer.ScissorEnable;
        desc.MultisampleEnable = m_rasterizer.MultisampleEnable;
        desc.AntialiasedLineEnable = m_rasterizer.AntialiasedLineEnable;

        IFC(MilTranslateD3D11Error(m_pD3DDevice->CreateRasterizerState(&desc, &pState)));

        m_rasterizerCache.Add(m_rasterizer, pState);
        pState->Release();
    }

    m_pD3DContext->RSSetState(pState);

    m_fRasterizerDirty = false;

Cleanup:
    RRETURN(hr);
}

HRESULT
CHwRenderStateManager::ApplySamplers()
{
    HRESULT hr = S_OK;

    ID3D11SamplerState *rgpSamplers[MIL_SAMPLER_COUNT];

    for (UINT i = 0; i < MIL_SAMPLER_COUNT; i++)
    {
        MilSamplerState const &s = m_rgSamplers[i];

        ID3D11SamplerState *pState = m_samplerCache.Find(s);

        if (pState == NULL)
        {
            D3D11_SAMPLER_DESC desc;

            ZeroMemory(&desc, sizeof(desc));
            desc.Filter = s.Filter;
            desc.AddressU = s.AddressU;
            desc.AddressV = s.AddressV;
            desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.MipLODBias = 0.0f;
            desc.MaxAnisotropy = max(1u, s.MaxAnisotropy);
            desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
            desc.BorderColor[0] = MIL_COLOR_GET_RED(s.BorderColor) / 255.0f;
            desc.BorderColor[1] = MIL_COLOR_GET_GREEN(s.BorderColor) / 255.0f;
            desc.BorderColor[2] = MIL_COLOR_GET_BLUE(s.BorderColor) / 255.0f;
            desc.BorderColor[3] = MIL_COLOR_GET_ALPHA(s.BorderColor) / 255.0f;
            desc.MinLOD = 0.0f;
            desc.MaxLOD = s.UseMipMaps ? D3D11_FLOAT32_MAX : 0.0f;

            IFC(MilTranslateD3D11Error(m_pD3DDevice->CreateSamplerState(&desc, &pState)));

            m_samplerCache.Add(s, pState);
            pState->Release();
        }

        rgpSamplers[i] = pState;
        m_rgfSamplerDirty[i] = false;
    }

    m_pD3DContext->PSSetSamplers(0, MIL_SAMPLER_COUNT, rgpSamplers);

    m_fAnySamplerDirty = false;

Cleanup:
    RRETURN(hr);
}

HRESULT
CHwRenderStateManager::ApplyTextures()
{
    m_pD3DContext->PSSetShaderResources(0, MIL_TEXTURE_STAGE_COUNT, m_rgpTextures);

    for (UINT i = 0; i < MIL_TEXTURE_STAGE_COUNT; i++)
    {
        m_rgfTextureKnown[i] = true;
    }

    m_fTexturesDirty = false;

    return S_OK;
}

HRESULT
CHwRenderStateManager::ApplyShaders()
{
    HRESULT hr = S_OK;

    //
    // Vertex shader (fixed function emulation when NULL)
    //

    if (m_fVertexShaderDirty)
    {
        ID3D11VertexShader *pVS = m_pVertexShader;

        if (pVS == NULL)
        {
            IFC(m_fixedFunction.GetVertexShaderNoRef(m_vertexFormat, &pVS));
        }

        if (pVS != m_pBoundVertexShader)
        {
            m_pD3DContext->VSSetShader(pVS, NULL, 0);
            m_pBoundVertexShader = pVS;
        }

        m_fVertexShaderDirty = false;
    }

    //
    // Pixel shader (fixed function emulation when NULL)
    //

    if (m_fPixelShaderDirty || (m_pPixelShader == NULL && m_fStagesDirty))
    {
        ID3D11PixelShader *pPS = m_pPixelShader;

        if (pPS == NULL)
        {
            IFC(m_fixedFunction.GetPixelShaderNoRef(m_rgStages, m_cActiveStages, &pPS));
        }

        if (pPS != m_pBoundPixelShader)
        {
            m_pD3DContext->PSSetShader(pPS, NULL, 0);
            m_pBoundPixelShader = pPS;
        }

        m_fPixelShaderDirty = false;
        m_fStagesDirty = false;
    }

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:    CHwRenderStateManager::CreateNullVertexBuffer
//
//  Synopsis:  버텍스 형식에 없는 속성(법선, 반사색, 추가 텍스처 좌표)을 0 으로
//             공급하는 작은 버퍼. 입력 레이아웃이 슬롯 MIL_NULL_VERTEX_SLOT 에서
//             인스턴스 단계 0 으로 읽는다(HwFixedFunctionShaders.cpp).
//
//------------------------------------------------------------------------------
HRESULT
CHwRenderStateManager::CreateNullVertexBuffer()
{
    HRESULT hr = S_OK;
    ID3D11Device *pDevice = NULL;

    static const float rgZero[16] = {};

    D3D11_BUFFER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.ByteWidth = sizeof(rgZero);
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA data;
    ZeroMemory(&data, sizeof(data));
    data.pSysMem = rgZero;

    m_pD3DContext->GetDevice(&pDevice);

    IFC(MilTranslateD3D11Error(pDevice->CreateBuffer(&desc, &data, &m_pNullVertexBuffer)));

Cleanup:
    ReleaseInterface(pDevice);
    RRETURN(hr);
}

HRESULT
CHwRenderStateManager::ApplyInputAssembler()
{
    HRESULT hr = S_OK;

    if (m_fVertexFormatDirty)
    {
        ID3D11InputLayout *pLayout = NULL;

        IFC(m_fixedFunction.GetInputLayoutNoRef(m_vertexFormat, &pLayout));

        m_pD3DContext->IASetInputLayout(pLayout);
        m_fVertexFormatDirty = false;
    }

    if (m_fVertexBufferDirty)
    {
        if (m_pNullVertexBuffer == NULL)
        {
            IFC(CreateNullVertexBuffer());
        }

        ID3D11Buffer *rgBuffers[2] = { m_pVertexBuffer, m_pNullVertexBuffer };
        UINT rgStrides[2] = { m_uVertexStride, 16 };
        UINT rgOffsets[2] = { 0, 0 };

        C_ASSERT(MIL_NULL_VERTEX_SLOT == 1);

        m_pD3DContext->IASetVertexBuffers(0, 2, rgBuffers, rgStrides, rgOffsets);
        m_fVertexBufferDirty = false;
    }

    if (m_fIndexBufferDirty)
    {
        m_pD3DContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
        m_fIndexBufferDirty = false;
    }

    if (m_fTopologyDirty)
    {
        m_pD3DContext->IASetPrimitiveTopology(m_topology);
        m_fTopologyDirty = false;
    }

Cleanup:
    RRETURN(hr);
}

HRESULT
CHwRenderStateManager::ApplyConstants()
{
    HRESULT hr = S_OK;

    if (m_fVSConstantsDirty)
    {
        IFC(UploadConstantBuffer(m_pVSConstantBuffer, m_rgVSFloat4, sizeof(m_rgVSFloat4)));
        m_fVSConstantsDirty = false;
    }

    if (m_fPSConstantsDirty)
    {
        IFC(UploadConstantBuffer(m_pPSConstantBuffer, m_rgPSFloat4, sizeof(m_rgPSFloat4)));
        m_fPSConstantsDirty = false;
    }

    if (m_fPSIntBoolConstantsDirty)
    {
        BYTE rgData[sizeof(m_rgPSInt4) + sizeof(m_rgPSBool4)];
        CopyMemory(rgData, m_rgPSInt4, sizeof(m_rgPSInt4));
        CopyMemory(rgData + sizeof(m_rgPSInt4), m_rgPSBool4, sizeof(m_rgPSBool4));

        IFC(UploadConstantBuffer(m_pPSIntBoolConstantBuffer, rgData, sizeof(rgData)));
        m_fPSIntBoolConstantsDirty = false;
    }

    if (m_fFFTransformDirty && m_pVertexShader == NULL)
    {
        MilFixedFunctionVSConstants ff;

        CMILMatrix matWVP =
            (m_rgTransforms[MilTransform_World] * m_rgTransforms[MilTransform_View])
            * m_rgTransforms[MilTransform_Projection];

        CMILMatrix matT = matWVP.transpose();

        CopyMemory(ff.WorldViewProj, &matT, sizeof(ff.WorldViewProj));

        IFC(UploadConstantBuffer(m_pFFConstantBuffer, &ff, sizeof(ff)));
        m_fFFTransformDirty = false;
    }

    {
        ID3D11Buffer *rgVS[2] = { m_pVSConstantBuffer, m_pFFConstantBuffer };
        ID3D11Buffer *rgPS[2] = { m_pPSConstantBuffer, m_pPSIntBoolConstantBuffer };

        m_pD3DContext->VSSetConstantBuffers(0, 2, rgVS);
        m_pD3DContext->PSSetConstantBuffers(0, 2, rgPS);
    }

Cleanup:
    RRETURN(hr);
}

HRESULT
CHwRenderStateManager::ApplyViewportAndScissor()
{
    if (m_fViewportDirty)
    {
        D3D11_VIEWPORT vp;

        vp.TopLeftX = static_cast<float>(m_rcViewport.X);
        vp.TopLeftY = static_cast<float>(m_rcViewport.Y);
        vp.Width = static_cast<float>(m_rcViewport.Width);
        vp.Height = static_cast<float>(m_rcViewport.Height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;

        m_pD3DContext->RSSetViewports(1, &vp);
        m_fViewportDirty = false;
    }

    if (m_fScissorDirty)
    {
        D3D11_RECT rc;

        if (m_fScissorRectValid)
        {
            rc.left = m_rcScissorRect.X;
            rc.top = m_rcScissorRect.Y;
            rc.right = m_rcScissorRect.X + m_rcScissorRect.Width;
            rc.bottom = m_rcScissorRect.Y + m_rcScissorRect.Height;
        }
        else
        {
            rc.left = 0;
            rc.top = 0;
            rc.right = static_cast<LONG>(m_uRenderTargetWidth);
            rc.bottom = static_cast<LONG>(m_uRenderTargetHeight);
        }

        m_pD3DContext->RSSetScissorRects(1, &rc);
        m_fScissorDirty = false;
    }

    return S_OK;
}

//+------------------------------------------------------------------------
//
//  Function:   CHwRenderStateManager::ApplyStateForDraw
//
//  Synopsis:   Bind every dirty piece of state. Called before each draw.
//
//-------------------------------------------------------------------------
HRESULT
CHwRenderStateManager::ApplyStateForDraw()
{
    HRESULT hr = S_OK;

    if (m_fBlendDirty)
    {
        IFC(ApplyBlendState());
    }

    if (m_fDepthStencilDirty)
    {
        IFC(ApplyDepthStencilState());
    }

    if (m_fRasterizerDirty)
    {
        IFC(ApplyRasterizerState());
    }

    if (m_fAnySamplerDirty)
    {
        IFC(ApplySamplers());
    }

    if (m_fTexturesDirty)
    {
        IFC(ApplyTextures());
    }

    IFC(ApplyShaders());

    IFC(ApplyInputAssembler());

    IFC(ApplyConstants());

    IFC(ApplyViewportAndScissor());

Cleanup:
    RRETURN(hr);
}

//+------------------------------------------------------------------------
//
//  Function:   CHwRenderStateManager::SetDefaultState
//
//  Synopsis:   Sets the default values for all the state. D3D9 시절 기본값과
//              같은 의미를 유지한다.
//
//-------------------------------------------------------------------------
HRESULT
CHwRenderStateManager::SetDefaultState()
{
    //
    // Blend: no blending, write all channels, blend factor 0
    //
    m_blend.Enable = FALSE;
    m_blend.SrcBlend = D3D11_BLEND_ONE;
    m_blend.DestBlend = D3D11_BLEND_ZERO;
    m_blend.WriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    m_blendFactor = 0;
    m_fBlendDirty = true;

    //
    // Depth / stencil: both off. Stencil clipping uses ref 0, NOTEQUAL, write mask 0.
    //
    m_depthStencil.DepthEnable = FALSE;
    m_depthStencil.DepthWriteEnable = FALSE;
    m_depthStencil.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    m_depthStencil.StencilEnable = FALSE;
    m_depthStencil.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
    m_depthStencil.StencilReadMask = 0xff;
    m_depthStencil.StencilWriteMask = 0x00;
    m_uStencilRef = 0;
    m_fDepthStencilDirty = true;

    //
    // Rasterizer: solid, no culling (negative scales are valid 2D transforms),
    // scissor off until a rectangular clip is set, multisample on.
    //
    m_rasterizer.FillMode = D3D11_FILL_SOLID;
#if DBG
    if (IsTagEnabled(tagWireframe))
    {
        m_rasterizer.FillMode = D3D11_FILL_WIREFRAME;
    }
#endif
    m_rasterizer.CullMode = D3D11_CULL_NONE;
    m_rasterizer.ScissorEnable = FALSE;
    m_rasterizer.MultisampleEnable = TRUE;
    m_rasterizer.AntialiasedLineEnable = FALSE;
    m_fRasterizerDirty = true;

    //
    // Samplers: point filtering, clamp, max anisotropy from caps
    //
    for (UINT i = 0; i < MIL_SAMPLER_COUNT; i++)
    {
        m_rgSamplers[i].Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        m_rgSamplers[i].AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        m_rgSamplers[i].AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        m_rgSamplers[i].BorderColor = 0;
        m_rgSamplers[i].MaxAnisotropy = min(4u, max(1u, m_caps.MaxAnisotropy));
        m_rgSamplers[i].UseMipMaps = FALSE;
        m_rgfSamplerDirty[i] = true;
    }
    m_fAnySamplerDirty = true;

    //
    // Textures: none
    //
    for (UINT i = 0; i < MIL_TEXTURE_STAGE_COUNT; i++)
    {
        m_rgpTextures[i] = NULL;
        m_rgfTextureKnown[i] = true;
    }
    m_fTexturesDirty = true;

    //
    // Fixed function stages: all disabled, default texcoord indices
    //
    for (UINT i = 0; i < MIL_FF_STAGE_COUNT; i++)
    {
        m_rgStages[i].ColorOp = MilTexOp_Disable;
        m_rgStages[i].ColorArg1 = MilTexArg_Texture;
        m_rgStages[i].ColorArg2 = MilTexArg_Current;
        m_rgStages[i].AlphaOp = MilTexOp_Disable;
        m_rgStages[i].AlphaArg1 = MilTexArg_Texture;
        m_rgStages[i].AlphaArg2 = MilTexArg_Current;
        m_rgStages[i].TexCoordIndex = i;
    }
    m_cActiveStages = 0;
    m_fStagesDirty = true;

    //
    // Shaders: fixed function emulation
    //
    m_pVertexShader = NULL;
    m_fVertexShaderKnown = true;
    m_fVertexShaderDirty = true;
    m_pPixelShader = NULL;
    m_fPixelShaderKnown = true;
    m_fPixelShaderDirty = true;

    //
    // Transforms: identity
    //
    for (UINT i = 0; i < MilTransform_Count; i++)
    {
        m_rgTransforms[i] = IdentityMatrix;
    }
    m_fFFTransformDirty = true;
    m_f2DTransformsUsedForFixedFunction = FALSE;
    m_f2DTransformUsedForVertexShader = FALSE;

    //
    // Streams
    //
    m_pVertexBuffer = NULL;
    m_pNullVertexBuffer = NULL;
    m_uVertexStride = 0;
    m_fVertexBufferKnown = true;
    m_fVertexBufferDirty = true;

    m_pIndexBuffer = NULL;
    m_fIndexBufferKnown = true;
    m_fIndexBufferDirty = true;

    m_fVertexFormatDirty = true;
    m_fTopologyDirty = true;

    //
    // Scissor Rect
    //
    ZeroMemory(&m_rcScissorRect, sizeof(m_rcScissorRect));
    m_fScissorRectValid = false;
    m_fScissorDirty = true;

    m_fClipSet = FALSE;

    m_fViewportDirty = true;

    m_fVSConstantsDirty = true;
    m_fPSConstantsDirty = true;
    m_fPSIntBoolConstantsDirty = true;

    return S_OK;
}
