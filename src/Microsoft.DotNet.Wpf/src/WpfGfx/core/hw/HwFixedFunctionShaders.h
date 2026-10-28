// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//
//  Description:
//      D3D11에는 고정 기능 파이프라인이 없다. WpfGfx는 몇 군데(RenderTexture, 글리프
//      텍스처 쌍 그리기 등)에서 "버텍스 셰이더 없음 + 변환 상태", "픽셀 셰이더 없음 +
//      텍스처 스테이지 연산"에 의존하므로, 그 조합을 작은 HLSL로 에뮬레이션한다.
//
//      - 버텍스 형식마다 입력 레이아웃과 변환 전용 버텍스 셰이더 하나
//      - 텍스처 스테이지 연산 조합(키)마다 픽셀 셰이더 하나
//
//      셰이더는 처음 쓸 때 D3DCompile로 만들고 디바이스 수명 동안 캐시한다.
//
//------------------------------------------------------------------------------

MtExtern(CHwFixedFunctionShaders);

//
// 텍스처 스테이지 연산. D3D9의 D3DTOP_/D3DTA_ 중 WpfGfx가 쓰는 부분집합.
//
enum MilTextureArg
{
    MilTexArg_Diffuse               = 0,
    MilTexArg_Current               = 1,
    MilTexArg_Texture               = 2,
    MilTexArg_Specular              = 3,
    MilTexArg_TextureAlphaReplicate = 4,    // D3DTA_TEXTURE | D3DTA_ALPHAREPLICATE

    MilTexArg_Count
};

enum MilTextureOp
{
    MilTexOp_Disable    = 0,
    MilTexOp_SelectArg1 = 1,
    MilTexOp_SelectArg2 = 2,
    MilTexOp_Modulate   = 3,

    MilTexOp_Count
};

struct MilTextureStageState
{
    MilTextureOp  ColorOp;
    MilTextureArg ColorArg1;
    MilTextureArg ColorArg2;
    MilTextureOp  AlphaOp;
    MilTextureArg AlphaArg1;
    MilTextureArg AlphaArg2;
    UINT          TexCoordIndex;

    bool operator==(const MilTextureStageState &o) const
    {
        return ColorOp == o.ColorOp && ColorArg1 == o.ColorArg1 && ColorArg2 == o.ColorArg2
            && AlphaOp == o.AlphaOp && AlphaArg1 == o.AlphaArg1 && AlphaArg2 == o.AlphaArg2
            && TexCoordIndex == o.TexCoordIndex;
    }
    bool operator!=(const MilTextureStageState &o) const { return !(*this == o); }
};

#define MIL_FF_STAGE_COUNT 8

//
// 고정 기능 에뮬레이션 상수 버퍼 (VS b1). 열 우선(packing) 규약은 파이프라인 셰이더와
// 같다: C++에서 전치해서 올리고 HLSL에서 mul(pos, m)으로 쓴다.
//
struct MilFixedFunctionVSConstants
{
    float WorldViewProj[16];
};

class CHwFixedFunctionShaders
{
public:
    DECLARE_METERHEAP_CLEAR(ProcessHeap, Mt(CHwFixedFunctionShaders));

    CHwFixedFunctionShaders();
    ~CHwFixedFunctionShaders();

    HRESULT Init(
        __in_ecount(1) ID3D11Device *pDevice
        );

    // 버텍스 형식의 입력 레이아웃. 처음 요청될 때 만든다. AddRef 없음.
    HRESULT GetInputLayoutNoRef(
        MilVertexFormatId format,
        __deref_out_ecount(1) ID3D11InputLayout **ppLayout
        );

    // 변환만 하는 버텍스 셰이더 (버텍스 셰이더 없이 그릴 때). AddRef 없음.
    HRESULT GetVertexShaderNoRef(
        MilVertexFormatId format,
        __deref_out_ecount(1) ID3D11VertexShader **ppShader
        );

    // 텍스처 스테이지 조합을 에뮬레이션하는 픽셀 셰이더. AddRef 없음.
    HRESULT GetPixelShaderNoRef(
        __in_ecount(MIL_FF_STAGE_COUNT) const MilTextureStageState *rgStages,
        UINT cActiveStages,
        __deref_out_ecount(1) ID3D11PixelShader **ppShader
        );

    static void BuildInputElements(
        MilVertexFormatId format,
        __out_ecount(16) D3D11_INPUT_ELEMENT_DESC *rgElements,
        __out_ecount(1) UINT *pcElements
        );

private:
    HRESULT EnsureVertexShader(MilVertexFormatId format);

    static HRESULT CompileShader(
        __in PCSTR pszSource,
        __in PCSTR pszEntry,
        __in PCSTR pszProfile,
        __deref_out ID3DBlob **ppBlob
        );

private:
    ID3D11Device *m_pDevice;

    ID3D11InputLayout *m_rgpInputLayouts[MilVertexFormat_Count];
    ID3D11VertexShader *m_rgpVertexShaders[MilVertexFormat_Count];

    struct PixelShaderEntry
    {
        MilTextureStageState rgStages[MIL_FF_STAGE_COUNT];
        UINT cActiveStages;
        ID3D11PixelShader *pShader;
    };

    enum { c_cMaxPixelShaders = 32 };
    PixelShaderEntry m_rgPixelShaders[c_cMaxPixelShaders];
    UINT m_cPixelShaders;
};
