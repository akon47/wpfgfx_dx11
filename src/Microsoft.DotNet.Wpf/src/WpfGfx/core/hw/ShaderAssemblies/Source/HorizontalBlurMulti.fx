// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//-----------------------------------------------------------------------------
// File: HorizontalBlurMulti.fx
//
// Desc: Effect file for image post-processing sample.  This effect contains
//       a single pixel shader that blurs an image using a weighted sampling
//       kernel.
//
//-----------------------------------------------------------------------------

// Size is the height of the source texture for vertical shaders, and the width
// for horizontal shaders. 픽셀 셰이더 상수는 b0 의 c# 레지스터 번호를 그대로 쓴다.
cbuffer MilPixelShaderConstants : register(b0)
{
    float    size        : packoffset(c0);
    float    sampleStart : packoffset(c1);
    float    numSamples  : packoffset(c2);
    float4x4 weights     : packoffset(c3);
};

Texture2D    g_texSrcColor : register(t0);
SamplerState g_samSrcColor : register(s0);
Texture2D    g_texAccumColor : register(t1);
SamplerState g_samAccumColor : register(s1);

static const int maxSamples = 15; // 한 패스에 15 샘플 (ps_2_0 시절 한계를 유지해 가중치 상수 배치를 바꾸지 않는다)

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos  : SV_POSITION;
    float4 Color: COLOR0;
    float2 TexCoord : TEXCOORD0;
};

//-----------------------------------------------------------------------------
// Pixel Shader: HorizontalBlurMulti
// Desc: Blurs the image horizontally
//       adding the previously accumulated color to the next samples taken.
//-----------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_TARGET
{
    float2 Tex = input.TexCoord;
    float4 Color = g_texAccumColor.Sample(g_samAccumColor, Tex);
    float2 sampleTex;
    sampleTex.y = Tex.y;
    [unroll]
    for (int i = 0; i < maxSamples; i++)
    {
        sampleTex.x = Tex.x + ((sampleStart + i) / size);
        Color += (g_texSrcColor.Sample(g_samSrcColor, sampleTex) * weights[i%4][i/4]); // matrices are column-major by default
    }

    return Color;
}
