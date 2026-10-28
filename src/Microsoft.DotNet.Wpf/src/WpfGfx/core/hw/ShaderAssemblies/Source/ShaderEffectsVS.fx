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
// core/hw/HwFixedFunctionShaders.h 의 MIL_HLSL_VERTEX_INPUT 과 같아야 한다.
struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float4 Diffuse  : COLOR0;
    float4 Specular : COLOR1;
    float2 UV0 : TEXCOORD0;
    float2 UV1 : TEXCOORD1;
    float2 UV2 : TEXCOORD2;
    float2 UV3 : TEXCOORD3;
    float2 UV4 : TEXCOORD4;
    float2 UV5 : TEXCOORD5;
    float2 UV6 : TEXCOORD6;
    float2 UV7 : TEXCOORD7;
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
    output.Position = mul(float4(input.Position, 1.0f), deviceTransform);
    output.UV = input.UV0;
    return output;
}
