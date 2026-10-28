// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


// ShaderEffect 파이프라인의 버텍스 셰이더. 버텍스 상수 버퍼 b0 의 c0..c3 에
// 디바이스 변환이 들어온다 (CHwRenderStateManager::SetVertexShaderConstantF).

cbuffer MilVertexShaderConstants : register(b0)
{
    float4x4 deviceTransform : packoffset(c0);
};

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Position : POSITION;
    float4 Diffuse  : COLOR0;
    float2 UV0      : TEXCOORD0;
    float2 UV1      : TEXCOORD1;
};

struct VS_OUTPUT
{
    float4 Position  : SV_POSITION;
    float4 Color     : COLOR0;
    float2 UV        : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Position = mul(input.Position, deviceTransform);
    output.UV = input.UV0;
    return output;
}
