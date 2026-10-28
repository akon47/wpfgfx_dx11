// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//
//  $TAG ENGR
//
//      $Module:    win_mil_graphics_text
//      $Keywords:
//
//  $Description:
//      텍스트 렌더링 픽셀 셰이더. 예전 hlslTextShaders20A.fx 의 ps_2_0 어셈블리를.
//      SM4 HLSL로 옮긴 것이다. 글리프 알파 텍스처는 DXGI_FORMAT_A8_UNORM 이므로.
//      샘플 결과의 .a 에 알파가 있다.
//
//      상수 레지스터 번호는 CD3DRenderState 가 올리는 c# 번호와 같다.
//
//  $ENDTAG
//
//------------------------------------------------------------------------------

cbuffer MilPixelShaderConstants : register(b0)
{
    float4 c0 : packoffset(c0);   // 색 채널 마스크 (CTTB) / 평균 가중치 (GSTB)
    float4 c1 : packoffset(c1);   // ds,dt,0,0 (CT*SB) / (d4,d4,d4,d1) (*TB)
    float4 c2 : packoffset(c2);   // 브러시 알파 (SB) / (d5,d5,d5,d3) (TB)
    float4 c3 : packoffset(c3);   // 1,1,1,1 (SB) / (d6,d6,d6,d2) (TB)
    float4 c4 : packoffset(c4);   // g1*f + g2 (SB) / 1,1,1,1 (TB)
    float4 c5 : packoffset(c5);   // g3*f + g4 (SB) / .a = effect alpha (TB)
};

Texture2D    tex0 : register(t0);
SamplerState samp0 : register(s0);
Texture2D    tex1 : register(t1);
SamplerState samp1 : register(s1);

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Diffuse  : COLOR0;
    float4 Specular : COLOR1;
    float2 UV0      : TEXCOORD0;
    float2 UV1      : TEXCOORD1;
};

//-----------------------------------------------------------------------------
// clear type, solid brush
//
//   c1.rg = ds,dt   // red sub pixel offset in alpha texture coordinates
//   c2.rgb = a,a,a  // brush alpha
//   c3.rgb = 1,1,1
//   c4.rgb = g1*f + g2 // f = brush color (rgb)
//   c5.rgb = g3*f + g4 // f = brush color (rgb)
//   where g1, g2, g3 and g4 are alpha correction ratios
//-----------------------------------------------------------------------------
float4 PS_CTSB(PS_INPUT input) : SV_TARGET
{
    float2 t0 = input.UV0;

    // fetch glyph alpha for green channel
    float4 r1 = tex0.Sample(samp0, t0);

    // fetch glyph alpha for blue channel
    float4 r2 = tex0.Sample(samp0, t0 + c1.xy);

    // fetch glyph alpha for red channel
    float4 r0 = tex0.Sample(samp0, t0 - c1.xy);

    // combine color component alphas to single register
    float4 a;
    a.r = r0.a;
    a.g = r1.a;
    a.b = r2.a;
    a.a = 0;

    a.rgb = a.rgb * c2.rgb;     // multiply glyph alphas by brush alpha
    a.a = r1.a * c2.r;          // set final alpha to the green channel alpha to match greyscale behavior
                                // multiply final alpha by brush alpha

    // Corrected alpha is calculated in a.rgb as
    // new a = a + a*(1-a)*((g1*f + g2)*a + (g3*f + g4))*4

    float3 k = c4.rgb * a.rgb;          // (g1*f + g2)*a
    k = k + c5.rgb;                     // (g1*f + g2)*a + (g3*f + g4)
    float3 inv = c3.rgb - a.rgb;        // 1-a
    k = k * inv;                        // (1-a)*((g1*f + g2)*a + (g3*f + g4))
    k = k * 4;                          // ... *4
    a.rgb = a.rgb * k + a.rgb;          // a + a*(1-a)*((g1*f + g2)*a + (g3*f + g4))*4

    return a;
}

//-----------------------------------------------------------------------------
// grey scale, solid brush
//
//   c2.a = a           // brush alpha
//   c3.a = 1
//   c4.a = g1*f + g2 // f = brush color (rgb)
//   c5.a = g3*f + g4 // f = brush color (rgb)
//-----------------------------------------------------------------------------
float4 PS_GSSB(PS_INPUT input) : SV_TARGET
{
    // fetch glyph alpha
    float a = tex0.Sample(samp0, input.UV0).a;

    a = a * c2.a;   // multiply by alpha

    // Corrected alpha is calculated as
    // new a = a + a*(1-a)*((g1*f + g2)*a + (g3*f + g4))*4

    float k = c4.a * a;         // (g1*f + g2)*a
    k = k + c5.a;               // (g1*f + g2)*a + (g3*f + g4)
    float inv = c3.a - a;       // 1-a
    k = k * inv;                // (1-a)*((g1*f + g2)*a + (g3*f + g4))
    k = k * 4;
    a = a * k + a;              // a + a*(1-a)*((g1*f + g2)*a + (g3*f + g4))*4

    return float4(a, a, a, a);
}

//-----------------------------------------------------------------------------
// clear type, textured brush
//
//   c0: color channel mask: (1,0,0,0) = red; (0,1,0,0) = green; (0,0,1,0) = blue
//   c1: (d4,d4,d4,d1) // gamma
//   c2: (d5,d5,d5,d3) //   correction
//   c3: (d6,d6,d6,d2) //     ratios
//   c4: ( 1, 1, 1, 1)
//   c5.a: effect alpha
//-----------------------------------------------------------------------------
float4 PS_CTTB(PS_INPUT input) : SV_TARGET
{
    float4 r0 = tex0.Sample(samp0, input.UV0);   // fetch brush color (rgba, alpha-premultiplied)
    float4 r1 = tex1.Sample(samp1, input.UV1);   // fetch the alpha (a)

    r0 = saturate(r0 * c5.a);                   // combine brush color with effect alpha

    r1 = saturate(r0 * r1.a);                   // combine alphas

    r1.rgb = saturate(dot(r1.rgb, c0.rgb));     // replicate desired channel to r1.rgb (r1.rgb = af, r1.a = a)

    // corrected alpha value is calculated in r0.a as:
    // new a = a + (1-a)*(d2*af + (d3*af + d1*a)*a)*4

    // corrected premultiplied foreground
    // is calculated in r0.rgb as following:
    // new af = af + af*(1-a)*(d4 + d5*a + d6*af)*4

    r0.a   = c1.a * r1.a;               // d1*a
    r0.rgb = c2.rgb * r1.a;             // d5*a
    r0.rgb = r0.rgb + c1.rgb;           // d4 + d5*a
    r0.a   = c2.a * r1.b + r0.a;        // d3*af + d1*a
    r0.rgb = c3.rgb * r1.rgb + r0.rgb;  // d4 + d5*a + d6*af
    r0.a   = r1.a * r0.a;               // d3*a*af + d1*a*a
    float4 r3 = c4 - r1;
    r0.rgb = r3.a * r0.rgb;             // (1-a)*(d4 + d5*a + d6*af)
    r0.a   = c3.a * r1.b + r0.a;        // (d2*af + d3*a*af + d1*a*a)
    r0 = r0 * 4;
    r0.rgb = r1.rgb * r0.rgb + r1.rgb;  // af + af*(1-a)*(d4 + d5*a + d6*af)*4
    r0.a   = r3.a * r0.a + r1.a;        // a + (1-a)*(d2*af + d3*a*af + d1*a*a)*4

    return r0;
}

//-----------------------------------------------------------------------------
// grey scale, textured brush
//
//   c0: color channel weight mask: rgba = (.25,.5,.25,0)
//   c1: (d4,d4,d4,d1) // gamma
//   c2: (d5,d5,d5,d3) //   correction
//   c3: (d6,d6,d6,d2) //     ratios
//   c4: ( 1, 1, 1, 1)
//   c5.a: effect alpha
//-----------------------------------------------------------------------------
float4 PS_GSTB(PS_INPUT input) : SV_TARGET
{
    float4 r2 = tex0.Sample(samp0, input.UV0);   // fetch brush color (rgba, alpha-premultiplied)
    float4 r0 = tex1.Sample(samp1, input.UV1);   // fetch the alpha (a)

    r2 = saturate(r2 * c5.a);                   // combine brush color with effect alpha

    float4 r1 = saturate(r2 * r0.a);            // combine alphas

    float afa = dot(r1.rgb, c0.rgb);            // calculate average (premultiplied) color (r2.b in the asm)

    // corrected alpha value is calculated in r0.a as:
    // new a = a + (1-a)*(d2*af_a + (d3*af_a + d1*a)*a)*4

    // corrected premultiplied foreground
    // is calculated in r0.rgb as following:
    // new af = af + af*(1-a)*(d4 + d5*a + d6*af)*4

    r0.a   = c1.a * r1.a;               // d1*a
    r0.rgb = c2.rgb * r1.a;             // d5*a
    r0.rgb = r0.rgb + c1.rgb;           // d4 + d5*a
    r0.a   = c2.a * afa + r0.a;         // d3*af_a + d1*a
    r0.rgb = c3.rgb * r1.rgb + r0.rgb;  // d4 + d5*a + d6*af
    r0.a   = r1.a * r0.a;               // d3*a*af_a + d1*a*a
    float4 r3 = c4 - r1;
    r0.rgb = r3.a * r0.rgb;             // (1-a)*(d4 + d5*a + d6*af)
    r0.a   = c3.a * afa + r0.a;         // (d2*af_a + d3*a*af_a + d1*a*a)
    r0 = r0 * 4;
    r0.rgb = r1.rgb * r0.rgb + r1.rgb;  // af + af*(1-a)*(d4 + d5*a + d6*af)*4
    r0.a   = r3.a * r0.a + r1.a;        // a + (1-a)*(d2*af + d3*a*af + d1*a*a)*4

    return r0;
}
