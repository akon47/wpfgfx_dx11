// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//
//  Description:
//      고정 기능 파이프라인 에뮬레이션 셰이더와 입력 레이아웃. HwFixedFunctionShaders.h 참고.
//
//------------------------------------------------------------------------------

#include "precomp.hpp"

MtDefine(CHwFixedFunctionShaders, MILRender, "CHwFixedFunctionShaders");

//
// 버텍스 셰이더: 위치를 WorldViewProj로 변환하고 나머지는 그대로 넘긴다.
// 입력 구조체는 버텍스 형식에 맞춰 조립된다.
//
static const char sc_szVertexShaderSource[] =
    "cbuffer MilFixedFunctionTransforms : register(b1)\n"
    "{\n"
    "    float4x4 WorldViewProj;\n"
    "};\n"
    MIL_HLSL_VERTEX_INPUT
    "struct VS_OUTPUT\n"
    "{\n"
    "    float4 Position : SV_POSITION;\n"
    "    float4 Diffuse  : COLOR0;\n"
    "    float4 Specular : COLOR1;\n"
    "    float2 UV0 : TEXCOORD0;\n"
    "    float2 UV1 : TEXCOORD1;\n"
    "    float2 UV2 : TEXCOORD2;\n"
    "    float2 UV3 : TEXCOORD3;\n"
    "    float2 UV4 : TEXCOORD4;\n"
    "    float2 UV5 : TEXCOORD5;\n"
    "    float2 UV6 : TEXCOORD6;\n"
    "    float2 UV7 : TEXCOORD7;\n"
    "};\n"
    "VS_OUTPUT VS(MilVertexInput input)\n"
    "{\n"
    "    VS_OUTPUT output;\n"
    "    output.Position = mul(float4(input.Position, 1.0f), WorldViewProj);\n"
    "    output.Diffuse = input.Diffuse;\n"
    "    output.Specular = input.Specular;\n"
    "    output.UV0 = input.UV0;\n"
    "    output.UV1 = input.UV1;\n"
    "    output.UV2 = input.UV2;\n"
    "    output.UV3 = input.UV3;\n"
    "    output.UV4 = input.UV4;\n"
    "    output.UV5 = input.UV5;\n"
    "    output.UV6 = input.UV6;\n"
    "    output.UV7 = input.UV7;\n"
    "    return output;\n"
    "}\n";

//+-----------------------------------------------------------------------------
//
//  Member:    CHwFixedFunctionShaders::CHwFixedFunctionShaders
//
//------------------------------------------------------------------------------
CHwFixedFunctionShaders::CHwFixedFunctionShaders()
{
    m_pDevice = NULL;
    m_pVertexShader = NULL;
    m_pVertexShaderBlob = NULL;
    ZeroMemory(m_rgpInputLayouts, sizeof(m_rgpInputLayouts));
    ZeroMemory(m_rgPixelShaders, sizeof(m_rgPixelShaders));
    m_cPixelShaders = 0;
}

//+-----------------------------------------------------------------------------
//
//  Member:    CHwFixedFunctionShaders::~CHwFixedFunctionShaders
//
//------------------------------------------------------------------------------
CHwFixedFunctionShaders::~CHwFixedFunctionShaders()
{
    for (UINT i = 0; i < MilVertexFormat_Count; i++)
    {
        ReleaseInterface(m_rgpInputLayouts[i]);
    }

    ReleaseInterface(m_pVertexShader);
    ReleaseInterface(m_pVertexShaderBlob);

    for (UINT i = 0; i < m_cPixelShaders; i++)
    {
        ReleaseInterface(m_rgPixelShaders[i].pShader);
    }

    ReleaseInterface(m_pDevice);
}

//+-----------------------------------------------------------------------------
//
//  Member:    CHwFixedFunctionShaders::Init
//
//------------------------------------------------------------------------------
HRESULT
CHwFixedFunctionShaders::Init(
    __in_ecount(1) ID3D11Device *pDevice
    )
{
    Assert(m_pDevice == NULL);

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    return S_OK;
}

//+-----------------------------------------------------------------------------
//
//  Member:    CHwFixedFunctionShaders::BuildInputElements
//
//  Synopsis:  버텍스 형식의 D3D11 입력 요소. 시맨틱 이름은 파이프라인 셰이더.
//             조립기와 공유한다: POSITION, NORMAL, COLOR0/1, TEXCOORDn.
//
//------------------------------------------------------------------------------
void
CHwFixedFunctionShaders::BuildInputElements(
    MilVertexFormatId format,
    __out_ecount(MIL_VERTEX_INPUT_ELEMENT_COUNT) D3D11_INPUT_ELEMENT_DESC *rgElements,
    __out_ecount(1) UINT *pcElements
    )
{
    //
    // MIL_HLSL_VERTEX_INPUT 의 모든 속성을 선언한다. 형식에 있는 속성은 슬롯 0 의.
    // 버텍스 데이터에서, 없는 속성은 슬롯 MIL_NULL_VERTEX_SLOT 의 0 버퍼에서 읽는다.
    // (인스턴스 데이터, 단계 0 = 모든 버텍스가 첫 요소를 읽는다).
    //
    UINT c = 0;
    UINT uOffset = 0;
    UINT cTex = MilVertexFormatTexCoordCount(format);

    D3D11_INPUT_ELEMENT_DESC *p = rgElements;

    struct Attr { LPCSTR pszName; UINT uIndex; DXGI_FORMAT fmt; UINT cb; bool fPresent; };

    const Attr rgAttr[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 12, true },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 12, !!MilVertexFormatHasNormal(format) },
        { "COLOR",    0, DXGI_FORMAT_B8G8R8A8_UNORM,   4, !!MilVertexFormatHasDiffuse(format) },
        { "COLOR",    1, DXGI_FORMAT_B8G8R8A8_UNORM,   4, !!MilVertexFormatHasSpecular(format) },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,     8, cTex > 0 },
        { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,     8, cTex > 1 },
        { "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT,     8, cTex > 2 },
        { "TEXCOORD", 3, DXGI_FORMAT_R32G32_FLOAT,     8, cTex > 3 },
        { "TEXCOORD", 4, DXGI_FORMAT_R32G32_FLOAT,     8, cTex > 4 },
        { "TEXCOORD", 5, DXGI_FORMAT_R32G32_FLOAT,     8, cTex > 5 },
        { "TEXCOORD", 6, DXGI_FORMAT_R32G32_FLOAT,     8, cTex > 6 },
        { "TEXCOORD", 7, DXGI_FORMAT_R32G32_FLOAT,     8, cTex > 7 },
    };

    C_ASSERT(ARRAYSIZE(rgAttr) == MIL_VERTEX_INPUT_ELEMENT_COUNT);

    for (UINT i = 0; i < ARRAYSIZE(rgAttr); i++)
    {
        p[c].SemanticName = rgAttr[i].pszName;
        p[c].SemanticIndex = rgAttr[i].uIndex;
        p[c].Format = rgAttr[i].fmt;

        if (rgAttr[i].fPresent)
        {
            p[c].InputSlot = 0;
            p[c].AlignedByteOffset = uOffset;
            p[c].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
            p[c].InstanceDataStepRate = 0;
            uOffset += rgAttr[i].cb;
        }
        else
        {
            p[c].InputSlot = MIL_NULL_VERTEX_SLOT;
            p[c].AlignedByteOffset = 0;
            p[c].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
            p[c].InstanceDataStepRate = 0;
        }

        c++;
    }

    Assert(uOffset == MilVertexFormatStride(format));

    *pcElements = c;
}

//+-----------------------------------------------------------------------------
//
//  Member:    CHwFixedFunctionShaders::CompileShader
//
//------------------------------------------------------------------------------
HRESULT
CHwFixedFunctionShaders::CompileShader(
    __in PCSTR pszSource,
    __in PCSTR pszEntry,
    __in PCSTR pszProfile,
    __deref_out ID3DBlob **ppBlob
    )
{
    HRESULT hr = S_OK;
    ID3DBlob *pErrors = NULL;

    UINT uFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_WARNINGS_ARE_ERRORS;

    MIL_THR(D3DCompile(
        pszSource,
        strlen(pszSource),
        NULL,
        NULL,
        NULL,
        pszEntry,
        pszProfile,
        uFlags,
        0,
        ppBlob,
        &pErrors
        ));

    if (FAILED(hr))
    {
#if DBG
        if (pErrors)
        {
            TraceTag((tagError, "MIL-HW: fixed function shader compile failed:\n%s", static_cast<const char *>(pErrors->GetBufferPointer())));
        }
#endif
        MIL_DX11_LOG("fixed function shader compile failed hr=0x%08x entry=%s: %s", hr, pszEntry,
                     pErrors ? static_cast<const char *>(pErrors->GetBufferPointer()) : "");
        MIL_THR(WGXERR_SHADER_COMPILE_FAILED);
    }

    ReleaseInterface(pErrors);

    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:    CHwFixedFunctionShaders::EnsureVertexShader
//
//  Synopsis:  버텍스 형식의 VS와 입력 레이아웃을 만든다.
//
//------------------------------------------------------------------------------
HRESULT
CHwFixedFunctionShaders::EnsureVertexShader(
    MilVertexFormatId format
    )
{
    HRESULT hr = S_OK;

    Assert(format < MilVertexFormat_Count);

    if (m_pVertexShader == NULL)
    {
        IFC(CompileShader(sc_szVertexShaderSource, "VS", "vs_4_0", &m_pVertexShaderBlob));

        IFC(MilTranslateD3D11Error(m_pDevice->CreateVertexShader(
            m_pVertexShaderBlob->GetBufferPointer(),
            m_pVertexShaderBlob->GetBufferSize(),
            NULL,
            &m_pVertexShader
            )));
    }

    if (m_rgpInputLayouts[format] == NULL)
    {
        D3D11_INPUT_ELEMENT_DESC rgElements[MIL_VERTEX_INPUT_ELEMENT_COUNT];
        UINT cElements;

        BuildInputElements(format, rgElements, &cElements);

        IFC(MilTranslateD3D11Error(m_pDevice->CreateInputLayout(
            rgElements,
            cElements,
            m_pVertexShaderBlob->GetBufferPointer(),
            m_pVertexShaderBlob->GetBufferSize(),
            &m_rgpInputLayouts[format]
            )));
    }

Cleanup:
    if (FAILED(hr))
    {
        MIL_DX11_LOG("EnsureVertexShader(format=%d) failed hr=0x%08x", format, hr);
    }

    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:    CHwFixedFunctionShaders::GetInputLayoutNoRef
//
//------------------------------------------------------------------------------
HRESULT
CHwFixedFunctionShaders::GetInputLayoutNoRef(
    MilVertexFormatId format,
    __deref_out_ecount(1) ID3D11InputLayout **ppLayout
    )
{
    HRESULT hr = S_OK;

    IFC(EnsureVertexShader(format));

    *ppLayout = m_rgpInputLayouts[format];

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:    CHwFixedFunctionShaders::GetVertexShaderNoRef
//
//------------------------------------------------------------------------------
HRESULT
CHwFixedFunctionShaders::GetVertexShaderNoRef(
    MilVertexFormatId format,
    __deref_out_ecount(1) ID3D11VertexShader **ppShader
    )
{
    HRESULT hr = S_OK;

    IFC(EnsureVertexShader(format));

    *ppShader = m_pVertexShader;

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Function:  AppendArg
//
//  Synopsis:  텍스처 스테이지 인자를 HLSL 식으로.
//
//------------------------------------------------------------------------------
static PCSTR
ArgExpression(
    MilTextureArg arg,
    bool fAlpha
    )
{
    switch (arg)
    {
    case MilTexArg_Diffuse:               return fAlpha ? "diffuse.a"  : "diffuse.rgb";
    case MilTexArg_Current:               return fAlpha ? "cur.a"      : "cur.rgb";
    case MilTexArg_Texture:               return fAlpha ? "tex.a"      : "tex.rgb";
    case MilTexArg_Specular:              return fAlpha ? "specular.a" : "specular.rgb";
    case MilTexArg_TextureAlphaReplicate: return fAlpha ? "tex.a"      : "tex.aaa";
    default:
        Assert(false);
        return fAlpha ? "0.0" : "float3(0,0,0)";
    }
}

//+-----------------------------------------------------------------------------
//
//  Member:    CHwFixedFunctionShaders::GetPixelShaderNoRef
//
//  Synopsis:  스테이지 조합에 맞는 픽셀 셰이더를 찾거나 만든다.
//
//------------------------------------------------------------------------------
HRESULT
CHwFixedFunctionShaders::GetPixelShaderNoRef(
    __in_ecount(MIL_FF_STAGE_COUNT) const MilTextureStageState *rgStages,
    UINT cActiveStages,
    __deref_out_ecount(1) ID3D11PixelShader **ppShader
    )
{
    HRESULT hr = S_OK;

    ID3DBlob *pBlob = NULL;

    Assert(cActiveStages <= MIL_FF_STAGE_COUNT);

    //
    // Look for a cached shader
    //

    for (UINT i = 0; i < m_cPixelShaders; i++)
    {
        PixelShaderEntry const &e = m_rgPixelShaders[i];

        if (e.cActiveStages != cActiveStages)
        {
            continue;
        }

        bool fMatch = true;

        for (UINT s = 0; s < cActiveStages && fMatch; s++)
        {
            fMatch = (e.rgStages[s] == rgStages[s]);
        }

        if (fMatch)
        {
            *ppShader = e.pShader;
            goto Cleanup;
        }
    }

    if (m_cPixelShaders >= c_cMaxPixelShaders)
    {
        // 캐시가 찼다. 제일 오래된 것을 버린다.
        ReleaseInterface(m_rgPixelShaders[0].pShader);
        MoveMemory(&m_rgPixelShaders[0], &m_rgPixelShaders[1], sizeof(PixelShaderEntry) * (c_cMaxPixelShaders - 1));
        m_cPixelShaders--;
    }

    //
    // Generate HLSL
    //

    {
        char szSource[8192];
        szSource[0] = '\0';

        StringCchCatA(szSource, ARRAYSIZE(szSource),
            "struct PS_INPUT\n"
            "{\n"
            "    float4 Position : SV_POSITION;\n"
            "    float4 Diffuse  : COLOR0;\n"
            "    float4 Specular : COLOR1;\n"
            "    float2 UV0 : TEXCOORD0;\n"
            "    float2 UV1 : TEXCOORD1;\n"
            "    float2 UV2 : TEXCOORD2;\n"
            "    float2 UV3 : TEXCOORD3;\n"
            "    float2 UV4 : TEXCOORD4;\n"
            "    float2 UV5 : TEXCOORD5;\n"
            "    float2 UV6 : TEXCOORD6;\n"
            "    float2 UV7 : TEXCOORD7;\n"
            "};\n");

        for (UINT s = 0; s < cActiveStages; s++)
        {
            char szLine[160];
            StringCchPrintfA(szLine, ARRAYSIZE(szLine),
                "Texture2D tex%u : register(t%u);\nSamplerState samp%u : register(s%u);\n", s, s, s, s);
            StringCchCatA(szSource, ARRAYSIZE(szSource), szLine);
        }

        StringCchCatA(szSource, ARRAYSIZE(szSource),
            "float4 PS(PS_INPUT input) : SV_TARGET\n"
            "{\n"
            "    float4 diffuse = input.Diffuse;\n"
            "    float4 specular = input.Specular;\n"
            "    float4 cur = diffuse;\n"
            "    float4 tex = float4(0,0,0,0);\n"
            "    float2 uv[8] = { input.UV0, input.UV1, input.UV2, input.UV3, input.UV4, input.UV5, input.UV6, input.UV7 };\n");

        for (UINT s = 0; s < cActiveStages; s++)
        {
            MilTextureStageState const &st = rgStages[s];
            char szLine[512];

            if (st.ColorOp == MilTexOp_Disable)
            {
                break;
            }

            StringCchPrintfA(szLine, ARRAYSIZE(szLine),
                "    tex = tex%u.Sample(samp%u, uv[%u]);\n", s, s, min(st.TexCoordIndex, 7u));
            StringCchCatA(szSource, ARRAYSIZE(szSource), szLine);

            // color
            switch (st.ColorOp)
            {
            case MilTexOp_SelectArg1:
                StringCchPrintfA(szLine, ARRAYSIZE(szLine), "    float3 c%u = %s;\n", s, ArgExpression(st.ColorArg1, false));
                break;
            case MilTexOp_SelectArg2:
                StringCchPrintfA(szLine, ARRAYSIZE(szLine), "    float3 c%u = %s;\n", s, ArgExpression(st.ColorArg2, false));
                break;
            case MilTexOp_Modulate:
            default:
                StringCchPrintfA(szLine, ARRAYSIZE(szLine), "    float3 c%u = %s * %s;\n", s, ArgExpression(st.ColorArg1, false), ArgExpression(st.ColorArg2, false));
                break;
            }
            StringCchCatA(szSource, ARRAYSIZE(szSource), szLine);

            // alpha
            switch (st.AlphaOp)
            {
            case MilTexOp_Disable:
                StringCchPrintfA(szLine, ARRAYSIZE(szLine), "    float a%u = cur.a;\n", s);
                break;
            case MilTexOp_SelectArg1:
                StringCchPrintfA(szLine, ARRAYSIZE(szLine), "    float a%u = %s;\n", s, ArgExpression(st.AlphaArg1, true));
                break;
            case MilTexOp_SelectArg2:
                StringCchPrintfA(szLine, ARRAYSIZE(szLine), "    float a%u = %s;\n", s, ArgExpression(st.AlphaArg2, true));
                break;
            case MilTexOp_Modulate:
            default:
                StringCchPrintfA(szLine, ARRAYSIZE(szLine), "    float a%u = %s * %s;\n", s, ArgExpression(st.AlphaArg1, true), ArgExpression(st.AlphaArg2, true));
                break;
            }
            StringCchCatA(szSource, ARRAYSIZE(szSource), szLine);

            StringCchPrintfA(szLine, ARRAYSIZE(szLine), "    cur = float4(c%u, a%u);\n", s, s);
            StringCchCatA(szSource, ARRAYSIZE(szSource), szLine);
        }

        StringCchCatA(szSource, ARRAYSIZE(szSource),
            "    return cur;\n"
            "}\n");

        IFC(CompileShader(szSource, "PS", "ps_4_0", &pBlob));

        PixelShaderEntry &e = m_rgPixelShaders[m_cPixelShaders];

        IFC(MilTranslateD3D11Error(m_pDevice->CreatePixelShader(
            pBlob->GetBufferPointer(),
            pBlob->GetBufferSize(),
            NULL,
            &e.pShader
            )));

        CopyMemory(e.rgStages, rgStages, sizeof(MilTextureStageState) * MIL_FF_STAGE_COUNT);
        e.cActiveStages = cActiveStages;
        m_cPixelShaders++;

        *ppShader = e.pShader;
    }

Cleanup:
    ReleaseInterface(pBlob);

    RRETURN(hr);
}
