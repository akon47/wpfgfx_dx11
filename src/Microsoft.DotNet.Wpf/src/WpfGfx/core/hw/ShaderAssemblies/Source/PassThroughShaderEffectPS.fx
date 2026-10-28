// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


struct VS_OUTPUT
{
    float4 Position  : SV_POSITION;
    float4 Color     : COLOR0;
    float2 UV        : TEXCOORD0;
};

Texture2D    g_texSrcColor : register(t0);
SamplerState g_samSrcColor : register(s0);

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    return g_texSrcColor.Sample(g_samSrcColor, input.UV);
}
