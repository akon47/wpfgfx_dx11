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

// 옵트인 옵션(D8). 기본은 모두 꺼짐. 환경 변수(테스트용)나 WpfGfxDx11_SetOption export 로 켠다.
enum MilDx11Option
{
    MilDx11Option_None = 0,
    MilDx11Option_HardwareRenderTargetBitmap = 1,   // env WPFGFX_DX11_HW_RTB=1
    MilDx11Option_GpuAntialiasing = 2,              // env WPFGFX_DX11_MSAA=<samples>; value = samples (D32)
    MilDx11Option_Count
};
bool MilDx11OptionEnabled(UINT uOption);

// D32: requested MSAA sample count for window targets (0 = off). Takes effect
// for swap chains created or resized afterwards.
UINT MilDx11GpuAntialiasSamples();
void MilDx11SetGpuAntialiasSamples(UINT cSamples);
void MilDx11SetOption(UINT uOption, bool fEnabled);


// 리소스 생성 횟수 카운터(누적). 메모리 로그 줄에 같이 찍혀 프레임마다 만들어졌다 사라지는.
// 리소스(churn)를 찾는 데 쓴다. 로그가 꺼져 있어도 카운터는 센다(원자 증가 하나).
enum MilDx11CreateSite
{
    MilDx11Site_RenderTarget = 0,
    MilDx11Site_Texture,
    MilDx11Site_DepthStencil,
    MilDx11Site_MsaaScratch,
    MilDx11Site_Readback,
    MilDx11Site_SwapChain,
    MilDx11Site_PoolHit,
    MilDx11Site_Count
};
void MilDx11CountCreate(MilDx11CreateSite site);

// 디버그 레이어가 켜져 있으면 ID3D11Debug::ReportLiveDeviceObjects 요약을 로그에 쏟는다.
// (객체 종류별 생존 수. 누수 추적용.)
void MilDx11LogLiveObjects(__in_opt IUnknown *pD3DDevice, __in_z const char *pszContext);

// 텍스처를 로그 폴더에 <name>.bmp 로 저장한다(진단용).
void MilDx11DumpTexture(__in IUnknown *pD3DDevice, __in IUnknown *pD3DContext, __in IUnknown *pTexture, __in_z const char *pszName);

// 테스트 훅: 환경 변수 WPFGFX_DX11_TEST_DEVICE_LOSS=<n> 이 있으면 n번째 Present 에서.
// 한 번만 true 를 돌려준다. 엔진은 그 Present 를 디바이스 손실로 취급해 복구 경로를 탄다.
// (실제 GPU 제거를 일으킬 공개 API 가 D3D11 에는 없다.) 변수가 없으면 항상 false.
bool MilDx11TestInjectDeviceLoss();

#define MIL_DX11_LOG(...) \
    do { if (MilDx11LogEnabled()) { MilDx11Log(__VA_ARGS__); } } while (0)

// Timing probes (log-gated): accumulate per slot, print avg/max every 300 samples.
enum MilDx11TimingSlot
{
    MilDx11Timing_GdiReadback = 0,
    MilDx11Timing_Ulw,
    MilDx11Timing_WaitToPresent,
    MilDx11Timing_VBlankWait,
    MilDx11Timing_Count
};

void MilDx11TimingAdd(MilDx11TimingSlot slot, double ms);
double MilDx11QpcMs();

// Per-frame work counters (log-gated). Averages per present are printed with
// the 300-present memory line.
enum MilDx11Stat
{
    MilDx11Stat_Draws = 0,
    MilDx11Stat_ConstantUploads,
    MilDx11Stat_ConstantBytes,
    MilDx11Stat_TempBufferDraws,
    MilDx11Stat_RenderTargetBinds,
    MilDx11Stat_TextureMaps,
    MilDx11Stat_TextureMapBytes,
    MilDx11Stat_TextureMapUs,
    MilDx11Stat_RenderUs,
    MilDx11Stat_PresentUs,
    MilDx11Stat_ApplyStateUs,
    MilDx11Stat_TexNewRenderTarget,
    MilDx11Stat_TexNewStaging,
    MilDx11Stat_TexNewWithData,
    MilDx11Stat_TexNewOther,
    MilDx11Stat_Count
};

extern bool g_fMilDx11Stats;

void MilDx11StatAddSlow(MilDx11Stat stat, unsigned long long n);
void MilDx11StatsReport(unsigned int cPresents);

inline void MilDx11StatAdd(MilDx11Stat stat, unsigned long long n = 1)
{
    if (g_fMilDx11Stats)
    {
        MilDx11StatAddSlow(stat, n);
    }
}
