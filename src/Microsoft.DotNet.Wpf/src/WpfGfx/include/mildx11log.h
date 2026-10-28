// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

//+-----------------------------------------------------------------------------
//
//  mildx11log.h
//
//  D3D11 포팅 진단 로그. 환경 변수 WPFGFX_DX11_LOG 에 파일 경로가 있으면 그 파일에.
//  줄을 덧붙이고, 없으면 아무 일도 하지 않는다. 릴리스 빌드에서도 켤 수 있어서.
//  디버거 없이 렌더 스레드 실패의 원인(HRESULT, 셰이더 컴파일 오류)을 볼 수 있다.
//  스모크 앱: $env:WPFGFX_DX11_LOG = "<경로>" 뒤 실행. docs/BUILD.md 참고.
//
//------------------------------------------------------------------------------

#pragma once

void __cdecl MilDx11Log(__in_z const char *pszFormat, ...);

bool MilDx11LogEnabled();

// 여러 줄 텍스트(셰이더 소스 등)를 그대로 덧붙인다.
void MilDx11LogText(__in_z const char *pszTitle, __in_bcount(cb) const char *pText, size_t cb);

// 로그가 켜져 있고 디바이스가 디버그 레이어로 만들어졌으면 ID3D11InfoQueue에 쌓인.
// 검증 메시지를 로그에 쏟고 비운다. 아니면 아무 일도 하지 않는다.
void MilDx11LogD3DMessages(__in_opt IUnknown *pD3DDevice, __in_z const char *pszContext);

// 텍스처를 로그 폴더에 <name>.bmp 로 저장한다(진단용).
void MilDx11DumpTexture(__in IUnknown *pD3DDevice, __in IUnknown *pD3DContext, __in IUnknown *pTexture, __in_z const char *pszName);

// 테스트 훅: 환경 변수 WPFGFX_DX11_TEST_DEVICE_LOSS=<n> 이 있으면 n번째 Present 에서.
// 한 번만 true 를 돌려준다. 엔진은 그 Present 를 디바이스 손실로 취급해 복구 경로를 탄다.
// (실제 GPU 제거를 일으킬 공개 API 가 D3D11 에는 없다.) 변수가 없으면 항상 false.
bool MilDx11TestInjectDeviceLoss();

#define MIL_DX11_LOG(...) \
    do { if (MilDx11LogEnabled()) { MilDx11Log(__VA_ARGS__); } } while (0)
