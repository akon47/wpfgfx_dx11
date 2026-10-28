// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//-----------------------------------------------------------------------------
// File: DropShadow.fx
//
// Desc: Effect file for image post-processing sample.  This effect contains
//       a single pixel shader that offsets and colors a texture for use
//       in creating a shadow.
//
//-----------------------------------------------------------------------------

// 픽셀 셰이더 상수는 b0 의 c# 레지스터 번호를 그대로 쓴다.
cbuffer MilPixelShaderConstants : register(b0)
{
    float2 offset      : packoffset(c0);
    float3 shadowColor : packoffset(c1);
    float  opacity     : packoffset(c2);
};

Texture2D    g_texSrcColor : register(t0);
SamplerState g_samSrcColor : register(s0);
Texture2D    g_texShdColor : register(t1);
SamplerState g_samShdColor : register(s1);

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos  : SV_POSITION;
    float4 Color: COLOR0;
    float2 TexCoord : TEXCOORD0;
};

//-----------------------------------------------------------------------------
// Pixel Shader: DropShadowPS
// Desc: Returns the source texture with an offset, colored shadow.
//       The shadow texture is intended to be a blurred image.
//-----------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_TARGET
{
    float2 Tex = input.TexCoord;

    float4 outColor;
    float4 srcColor = g_texSrcColor.Sample(g_samSrcColor, Tex);
    if (srcColor.a != 1.0f)
    {
        float2 shdTex;
        shdTex.x = Tex.x - offset.x;
        shdTex.y = Tex.y - offset.y;
        float4 shdColor = g_texShdColor.Sample(g_samShdColor, shdTex);
        shdColor.rgb = shadowColor * shdColor.a * opacity;
        shdColor.a = shdColor.a * opacity;
        outColor = (1 - srcColor.a)*shdColor + srcColor;
    }
    else
    {
        outColor = srcColor;
    }

    return outColor;
}
