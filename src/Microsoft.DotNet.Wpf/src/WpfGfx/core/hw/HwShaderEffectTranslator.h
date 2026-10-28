// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//
//  Description:
//      ShaderEffect 호환(D6). 앱이 넘긴 D3D9 픽셀 셰이더 바이트코드(ps_2_0 ~ ps_3_0)를
//      HLSL 텍스트로 역변환한다. 결과는 ps_4_0 으로 컴파일해 쓴다.
//
//      레지스터 규약은 상태 관리자와 같다: float 상수는 cbuffer b0 의 c[224],
//      int/bool 상수는 b1 의 i[16]/b[16], 샘플러 s#/텍스처 t# 는 같은 번호.
//      입력은 효과 버텍스 셰이더(ShaderEffectsVS.fx)가 내는 COLOR0 와 TEXCOORD0 뿐이다.
//
//      변환할 수 없는 명령(예측 레지스터, 서브루틴, 깊이 출력, 큐브/볼륨 텍스처)이 있으면
//      WGXERR_SHADER_COMPILE_FAILED 를 돌려주고 호출자가 소프트웨어 폴백으로 간다.
//
//------------------------------------------------------------------------------

#pragma once

#include <string>

HRESULT
MilTranslateD3D9PixelShaderToHLSL(
    __in_bcount(cbBytecode) const void *pvBytecode,
    SIZE_T cbBytecode,
    __out std::string *pHLSL
    );
