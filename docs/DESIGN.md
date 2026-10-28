# D3D11 치환 설계

작성일 2026-10-28. Phase 2 산출물. 근거 수치는 `INVENTORY.md`, 결정 번호는 `DECISIONS.md`.

## 0. 한 문장 요약

`core/hw`의 래퍼 클래스들(`CD3DDeviceLevel1`, `CD3DResource` 계열, `CD3DRenderState`/`CHwRenderStateManager`, `CD3DSwapChain`,
`CHwSurfaceRenderTarget`)은 **이름과 메서드 모양을 유지**하고, 그 안의 D3D9 객체와 D3D9 타입만 D3D11/DXGI로 바꾼다.
상위 레이어(파이프라인, 컬러 소스, 효과, 글리프, uce)는 타입 이름 치환 수준의 변경만 받는다.

## 1. 현재 구조 (읽은 그대로)

```
CHwSurfaceRenderTarget / CHwDisplayRenderTarget / CHwHWNDRenderTarget
    │  (RT 설정, 클리어, 3D 시작/끝, 프레젠트, 더티 사각형)
    ▼
CD3DDeviceLevel1 : CHwSurfaceRenderTargetSharedData, CD3DRenderState, CMILPoolResource
    │  m_pD3DDevice / m_pD3DDeviceEx, D3DCAPS9 m_caps, 리소스 관리자, 글리프 뱅크,
    │  고정 버텍스 버퍼 4종, GPU 마커(쿼리), 효과 파이프라인용 VS/PS
    │
    ├─ CD3DRenderState  ── CHwRenderStateManager (CStateTable<T>로 값이 바뀔 때만 D3D 호출)
    │                        렌더스테이트 210종, 텍스처 스테이지 8×33, 샘플러 8×14, 변환, 셰이더, 상수(float4/int/bool),
    │                        텍스처 슬롯, 깊이 서피스, FVF, 스트림, 인덱스, 시저, 뷰포트
    ├─ CD3DResourceManager ── CD3DResource(공통 기반: 크기 추적, 퇴거, 지연 해제)
    │        CD3DSurface(IDirect3DSurface9 + D3DSURFACE_DESC)
    │        CD3DTexture(IDirect3DTexture9 + 레벨별 CD3DSurface)
    │          └ CD3DLockableTexture, CD3DVidMemOnlyTexture
    │        CD3DSwapChain(IDirect3DSwapChain9 + 백버퍼 CD3DSurface[])  └ CD3DSwapChainWithSwDC
    │        CHwD3DVertexBuffer / CHwD3DIndexBuffer (동적 링 버퍼)
    └─ CD3DDeviceManager (어댑터별 디바이스 목록, NullRef 디바이스, SW 디바이스)
         CDisplaySet/CDisplay (core/common: 어댑터·모니터·caps·Tier)
         CD3DLoader (d3d9.dll)
```

그리기는 두 파이프라인 중 하나다. `CHwShaderPipeline`은 브러시·마스크 조합을 `HwPipelineItem[]`으로 만들고
`hwshaderfragmenttohlslconverter`가 HLSL을 조립해 `D3DCompile(ps_2_0/3_0, vs_2_0/3_0)`로 컴파일, 상수는 `c#` 레지스터 번호로 전달한다.
`CHwFFPipeline`은 텍스처 스테이지(고정 기능)로 같은 일을 한다. 버텍스 형식은 네 가지뿐이다:
`XYZDUV2`(32B, 거의 모든 2D), `XYZDUV6`(64B), `XYZDUV8`(블러), `XYZNDSUV4`(3D).

## 2. 목표 구조

```
CD3DDeviceLevel1
    ID3D11Device* m_pDevice, ID3D11DeviceContext* m_pContext(즉시), IDXGIAdapter1*, D3D_FEATURE_LEVEL
    HwCaps m_caps  ← Feature Level에서 도출 (D3DCAPS9 대체, 같은 질문에 같은 의미로 답한다)
    (나머지 멤버 유지: 리소스 관리자, 글리프 뱅크, 버텍스 버퍼, 마커, 효과 파이프라인 VS/PS)

CHwRenderStateManager
    그림자 상태 + 더티 플래그 → Draw 직전 Apply()에서 상태 객체를 캐시에서 찾아 바인딩
      BlendDesc + factor + sampleMask   → ID3D11BlendState 캐시(desc 해시)
      DepthStencilDesc + ref            → ID3D11DepthStencilState 캐시
      RasterizerDesc(+scissor on/off)   → ID3D11RasterizerState 캐시
      SamplerDesc[8]                    → ID3D11SamplerState 캐시
      SRV[8], VS, PS, InputLayout(버텍스 형식 id), VB+stride, IB, scissor rect, viewport
      VS/PS 상수 그림자: float4[256], int4[16], bool[16] → cbuffer b0(VS), b0(PS) 한 번에 업로드
    (텍스처 스테이지·변환·FVF 상태는 삭제. 변환은 상수로만.)

CD3DSurface   = ID3D11Texture2D* + 서브리소스 인덱스 + 지연 생성 RTV/DSV/SRV + D3D11_TEXTURE2D_DESC
CD3DTexture   = ID3D11Texture2D* + SRV + 레벨별 CD3DSurface
CD3DLockableTexture = DYNAMIC(쓰기 전용) 또는 STAGING+DEFAULT 쌍(읽기)
CD3DSwapChain = IDXGISwapChain1* + 백버퍼 CD3DSurface  (CD3DSwapChainWithSwDC는 GDI 호환 플래그 + IDXGISurface1::GetDC)
CHwD3DVertexBuffer / IndexBuffer = D3D11_USAGE_DYNAMIC 버퍼, Map(WRITE_DISCARD/NO_OVERWRITE)
CD3DDeviceManager = 어댑터(LUID)별 디바이스, "SW 디바이스"는 D3D_DRIVER_TYPE_WARP
CDisplaySet/CDisplay = IDXGIFactory1 → EnumAdapters1 → EnumOutputs, DXGI_OUTPUT_DESC.Monitor
```

## 3. 타입 치환 (래퍼 바깥까지 새어 나간 것)

새 헤더 `core/hw/d3d11types.h`(가칭)에 아래를 정의하고 `include/std.h`의 `<d3d9.h>`를 `<d3d11_1.h>`, `<dxgi1_2.h>`로 바꾼다.

| D3D9 | 대체 | 방식 |
|---|---|---|
| `D3DFORMAT`, `D3DFMT_*` | `DXGI_FORMAT`, `DXGI_FORMAT_*` | INVENTORY §4.1 표로 기계적 치환. `P8`/`R8G8B8`은 CPU 변환 경로로 |
| `D3DSURFACE_DESC` | `D3D11_TEXTURE2D_DESC` | `PopulateSurfaceDesc`가 Usage/BindFlags/CPUAccess를 채움 |
| `D3DMULTISAMPLE_TYPE` + quality | `DXGI_SAMPLE_DESC` | |
| `D3DPOOL` | 없음 | desc의 Usage/CPUAccess로 흡수 |
| `D3DPRESENT_PARAMETERS` | `MilSwapChainParams{hwnd,w,h,fmt,sample,bufferCount,gdiCompatible,syncInterval}` | 자체 구조체 |
| `D3DPRIMITIVETYPE` | `D3D11_PRIMITIVE_TOPOLOGY` | FAN은 래퍼가 인덱스로 변환 |
| `D3DLOCKED_RECT` | `MilLockedRect{INT Pitch; void* pBits;}` | Map 결과를 담는 얇은 구조체(24곳 변경 최소화) |
| `D3DMATRIX`, `D3DVECTOR`, `D3DCOLORVALUE`, `D3DCOLOR`, `D3DRECT` | 같은 이름으로 자체 정의 | API 의존이 아니라 메모리 레이아웃. `BaseMatrix.h`가 기반으로 씀 |
| `D3DVIEWPORT9` | `D3D11_VIEWPORT` | |
| `D3DDEVTYPE` | `enum MilDeviceKind { Hardware, Warp, Reference }` | `D3DDEVTYPE_SW`는 소프트웨어 래스터라이저 표식이므로 `Software` 추가 |
| `D3DERR_*` | 같은 이름·같은 값으로 자체 정의 | DXGI 오류를 이 값으로 매핑(§8) |
| `D3DRS_/D3DSAMP_/D3DTEXF_/D3DTADDRESS_/D3DBLEND_/D3DCMP_/D3DCULL_/D3DFILL_` | 상태 관리자 API 자체가 바뀜(§4) | `AlphaBlendMode`/`FilterMode` 표를 D3D11 desc로 재작성 |
| `D3DTSS_/D3DTOP_/D3DTA_/D3DTTFF_` | 삭제 | 고정 기능 파이프라인 제거 |
| `D3DFVF_*` | `enum MilVertexFormatId { XYZDUV2, XYZDUV6, XYZDUV8, XYZNDSUV4 }` | 입력 레이아웃 캐시 키 |
| `IDirect3DSurface9*`가 시그니처에 드러난 메서드 | `CD3DSurface*` 또는 `ID3D11Texture2D*` | 래퍼 바깥 38개 파일 |

## 4. 상태 관리

- 기존 `SetRenderState(state, value)`류 범용 API는 없애고 **의미 단위 API**로 바꾼다:
  `SetAlphaBlendMode(const AlphaBlendMode*)`(이미 있음), `SetFilterMode(sampler, const FilterMode*)`(이미 있음),
  `SetDepthStencil(mode, ref)`, `SetRasterizer(fill, cull, scissorEnable, msaa)`, `SetColorWriteMask(mask)`, `SetScissorRect`, `SetViewport`.
  `CD3DRenderState`의 `sc_abm*`/`sc_fm*` 상수 표는 그대로 두고 내용만 D3D11 desc로 바꾼다. `sc_tso*`는 삭제.
- 상태 객체 캐시: desc를 바이트 해시한 소형 해시맵(`CHwStateObjectCache<DescT, ObjT>`), 디바이스당 하나.
- 상수: `SetVertexShaderConstantF/PixelShaderConstantF(reg, data, count)`는 유지하고 그림자 배열에 쓴다. Draw 직전 더티 범위를
  `Map(WRITE_DISCARD)`로 `cbuffer`에 올린다. HLSL 쪽은 `cbuffer Constants : register(b0) { float4 X : packoffset(cN); ... }`로
  레지스터 번호를 그대로 유지하므로 `MILSPHandle`(레지스터 인덱스) 체계가 살아남는다.
- 텍스처: `SetTexture(stage, CD3DTexture*)` → SRV 슬롯. 렌더타깃으로 바인딩된 텍스처를 SRV로 쓰려 하면 D3D11이 경고하며 해제하므로,
  `SetRenderTarget` 시 같은 리소스가 SRV 슬롯에 있으면 먼저 비운다(현재도 `ReleaseUseOfRenderTarget`가 비슷한 역할).

## 5. 스왑체인과 프레젠테이션 (D10)

WPF는 매 프레임 더티 사각형만 다시 그리고 나머지는 **백버퍼에 남아 있다고 가정**한다(D3D9 `D3DSWAPEFFECT_COPY`).
이 의미를 그대로 주는 것은 비트블트 모델 `DXGI_SWAP_EFFECT_SEQUENTIAL`(백버퍼 1개)이다. 플립 모델은 버퍼가 순환해 내용이
보존되지 않으므로 별도 영속 서피스와 복사가 필요하다. **1차 구현은 SEQUENTIAL + 백버퍼 1개 + `Present1`의 dirty rect**로 하고,
플립 모델 전환은 동작이 안정된 뒤 성능 작업으로 따로 결정한다.

- `CreateSwapChainForHwnd(device, hwnd, {w,h,B8G8R8A8_UNORM, sample{1,0}, BufferCount=1, SEQUENTIAL, flags})`.
  투명 창(`PresentWithGDI`, `CD3DSwapChainWithSwDC`)은 `DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE` + `IDXGISurface1::GetDC`.
- `Present(prcSource, prcDest, dirtyRegion)` → `DXGI_PRESENT_PARAMETERS{DirtyRectsCount, pDirtyRects}`로 `Present1(syncInterval, 0, &pp)`.
  소스≠대상 사각형(스크롤 블릿)은 `pScrollRect/pScrollOffset`가 아니라 기존처럼 백버퍼 안에서 복사 후 프레젠트.
- 멀티샘플 백버퍼는 없다. 3D(`Begin3D`)는 지금처럼 중간 MSAA 렌더타깃을 쓰고 `End3D`에서 `ResolveSubresource`.
- `WaitForVBlank` → 연결된 `IDXGIOutput::WaitForVBlank`. `GetRasterStatus`(스캔라인) → `GetFrameStatistics`로 대체하거나 미지원으로 보고.
- 크기 변경 → `ResizeBuffers` (D3D9 `Reset` 경로가 하던 일). 디바이스 재생성은 `DXGI_ERROR_DEVICE_REMOVED/RESET`에서만.

## 6. 셰이더 (D11)

- 프로필: `vs_4_0`, `ps_4_0`. FL10.0 하드웨어까지 돈다(D5).
- 런타임 조립기(`hwshaderfragmenttohlslconverter.cpp`, `HwHLSLShaderFragments.cpp`): 조각 텍스트를 SM4 문법으로 고친다.
  `sampler s` + `tex2D(s, uv)` → `Texture2D t : register(tN); SamplerState s : register(sN);` + `t.Sample(s, uv)`.
  시맨틱은 `SV_POSITION`/`SV_TARGET`, 나머지(`COLOR0`, `TEXCOORDn`)는 그대로. 상수는 §4의 `cbuffer`+`packoffset`.
- 텍스트 셰이더(`shaders/hlslTextShaders20{A,L}.fx`, ps_2_0 어셈블리 4종): **HLSL로 다시 쓴다**(각 30줄 내외, 수식은 주석에 있음).
  레지스터 c0~c5 의미는 유지. `A` 변종(A8 텍스처, `.a` 샘플)만 남기고 `L` 변종(L8)과 ps_1_1 변종, `ShaderGen` 도구는 삭제.
  `Shaders.rc`의 RCDATA 블롭 대신 fxc가 빌드 때 만든 헤더(`/Fh`)를 쓴다(내장 효과 셰이더와 같은 방식).
- 내장 효과(`ShaderAssemblies/Source/*.fx`): `register(C0)` → `cbuffer`/`packoffset`, `tex2D` → `Sample`. 프로필만 바꾸면 되는 수준.
- 입력 레이아웃: 버텍스 형식 4종마다 입력 구조체가 고정이므로, 형식별 더미 VS를 한 번 컴파일해 그 시그니처로
  `ID3D11InputLayout`을 만들어 캐시한다. 파이프라인이 만드는 VS는 같은 입력 구조체를 선언한다.
- 고정 기능 파이프라인(`CHwFFPipeline`, `CHwFFPipelineBuilder`, `hwffshaders.cpp`, `sc_tso*`)은 삭제한다. 3D 조명은 이미
  셰이더 파이프라인에 조각이 있는지 확인하고 없으면 `CHwDiffuseShader` 등의 수식을 HLSL 조각으로 옮긴다.

## 7. ShaderEffect 호환 (D6 세부)

앱이 넘긴 ps_2_0/ps_3_0 바이트코드 → **HLSL 텍스트로 역변환** → `D3DCompile(ps_4_0)` → 캐시. 변환기는 `core/fxjit/PixelShader`의
바이트코드 파서(`pstrans.cpp`)로 명령 스트림을 얻어 명령당 한 줄 HLSL을 낸다(`mov`, `add`, `mad`, `texld` → `t[n].Sample(s[n], uv)` 등).
레지스터는 `float4 r[32], c[224]`, 입력 `t[n]`/`v[n]`, 출력 `oC0`. ps_3_0의 정적·동적 흐름 제어(`if/loop/rep/break`)와 예측 레지스터는
지원하되, 변환할 수 없는 명령이 나오면 실패를 돌려주고 **기존 소프트웨어 폴백(fxjit JIT)**으로 간다. 관리 코드와 앱의 `.ps` 파일은 그대로다.

## 8. 디바이스, 오류, 스레딩

- 생성: `D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, 0, flags, {11_1, 11_0, 10_1, 10_0}, ...)`. 플래그에 `SINGLETHREADED`는 넣지 않는다
  (디바이스 객체의 리소스 생성은 스레드 안전). 즉시 컨텍스트는 기존 `Enter/Leave` 임계 영역 안에서만 쓴다.
  디버그 레이어는 기존 레지스트리 플래그 체계(`d3dregistry.cpp`)에 키 하나를 더해 켠다.
- "SW 디바이스"(`D3DDEVTYPE_SW`)는 WPF 자체 소프트웨어 래스터라이저를 뜻하며 D3D11과 무관하게 유지한다. `D3DDEVTYPE_REF`는 WARP로 매핑.
- 오류: `DXGI_ERROR_DEVICE_REMOVED`·`DEVICE_RESET`→`D3DERR_DEVICELOST`, `DEVICE_HUNG`→`D3DERR_DEVICEHUNG`, `E_OUTOFMEMORY`(리소스 생성)→
  `D3DERR_OUTOFVIDEOMEMORY`, `DXGI_ERROR_INVALID_CALL`→`D3DERR_INVALIDCALL`, 드라이버 내부 오류→`D3DERR_DRIVERINTERNALERROR`.
  상위(uce, api)와 관리 코드에 보이는 실패 사유는 바뀌지 않는다.
- caps(`HwCaps`): FL10.0+ → MaxTextureWidth/Height 8192(FL11: 16384), PS/VS 버전 3.0, 비2의 거듭제곱 무조건, 보더 색, 시저, 자동 밉,
  공유 리소스, 128bpp float 텍스처 모두 지원. `CheckDeviceFormat` 질의는 `CheckFormatSupport`로.

## 9. D3DImage interop (D7 세부)

`CInteropDeviceBitmap::Create(IUnknown* pUserSurface)`에서 먼저 `QueryInterface`로 종류를 가른다.

- `ID3D11Texture2D`: 같은 디바이스면 바로 SRV. 다른 디바이스면 `IDXGIResource::GetSharedHandle`로 열고(공유 생성이 아니면 실패),
  그것도 안 되면 사용자 디바이스의 즉시 컨텍스트로 스테이징 복사 → CPU → 업로드(소프트웨어 방식).
- `IDirect3DSurface9`: 사용자 디바이스가 9Ex이면 **WPF 쪽 D3D11 텍스처를 `D3D11_RESOURCE_MISC_SHARED`로 만들고 그 핸들을
  사용자의 `IDirect3DDevice9Ex::CreateTexture(..., &hShared)`로 열어** 사용자 디바이스에서 `StretchRect(user → shared)`를 한다.
  D3D9Ex와 D3D11은 공유 핸들을 서로 열 수 있다. 복사는 사용자 디바이스가 하므로 **WPF는 d3d9.dll을 로드하지 않는다**(헤더만 필요).
  9Ex가 아니면 기존 `BitBlt`(GetDC) 또는 `Software`(GetRenderTargetData) 방식.
- 새 export(D8)로 "D3D11 텍스처를 직접 받는" 경로를 노출하고, 공개 `D3DImage.SetBackBuffer`에 D3D11 포인터가 들어와도 위 분기로 동작한다.

## 10. 미디어 (`core/av`)

EVR 프레젠터는 DXVA2/D3D9에 묶여 있다. Phase 3의 마지막 단계에서 `IMFDXGIDeviceManager` + `ID3D11VideoProcessor`로 다시 쓴다.
그 전까지는 하드웨어 비디오 경로를 비활성화하고 소프트웨어 미디어 버퍼(`swmediabuffer.cpp`: 시스템 메모리 프레임 → 텍스처 업로드)만 쓴다.

## 11. 빌드

- 링크: `d3d11.lib`, `dxgi.lib` 정적 링크(모든 지원 OS에 있음). `d3dloader.cpp`의 동적 로드는 삭제. `D3DCompiler_47_cor3.dll` 지연 로드는 유지.
- fxc 프로필 `vs_4_0`/`ps_4_0`, 텍스트 셰이더도 `ShaderAssemblies.targets`에 추가. `Shaders.rc`의 셰이더 블롭·`ShaderGen`·`shaders/*11*.fx`·
  `common/DirectXLayer/D3DX9/` 삭제.
- `include/std.h`: `<d3d9.h>` → `<d3d11_1.h>`, `<dxgi1_2.h>`, `<d3dcompiler.h>`.

## 12. Phase 3 마일스톤 (작업 순서)

WIP는 `dx11-port` 브랜치, `main`은 항상 빌드·스모크 통과 상태(D12).

| M | 내용 | 합격 |
|---|---|---|
| M1 | 타입 치환과 래퍼 재작성으로 **D3D9 없이 컴파일**. 디바이스 생성은 아직 실패(Tier 0) → 전부 소프트웨어 래스터라이저 | 빌드 통과, 스모크 두 런타임에서 Tier 0로 전 셀 정상(3D 셀 제외: 소프트웨어 3D는 WARP 디바이스를 쓰므로 M7), 엔진이 `d3d9.dll`을 로드하지 않음 |
| M2 | 디바이스·스왑체인·클리어·프레젠트. 그리기는 기존 연산별 소프트웨어 폴백(`SoftwareFillPath` 등)으로 | Tier 2 보고, 창 배경 클리어가 D3D11, 스모크 전 셀 정상 |
| M3 | 셰이더 파이프라인(SM4 조립기, 상수 버퍼, 입력 레이아웃): 단색·그라데이션 채우기, 경로 | 스모크 1·2·7·10·12번 셀이 하드웨어 경로 |
| M4 | 텍스처·비트맵·타일 브러시·비트맵 캐시 | 5·8번 셀 |
| M5 | 텍스트(글리프 뱅크, ClearType/그레이스케일 셰이더) | 3·9번 셀 |
| M6 | 중간 렌더타깃·Opacity·VisualBrush·내장 효과(Blur/DropShadow)·클립 | 4·6·9·12번 셀 |
| M7 | 3D(Viewport3D): 깊이 버퍼, MSAA 리졸브, 조명 셰이더 | 11번 셀 |
| M8 | ShaderEffect 변환기(D6) | 사용자 `.ps` 효과 테스트 셀 추가 |
| M9 | D3DImage(D7), 새 export와 `WpfGfx.Dx11`(D8) | interop 테스트 앱 |
| M10 | 투명 창(GDI 프레젠트), 멀티 어댑터, 디바이스 제거 복구 | 투명 창 테스트 셀, 어댑터 변경 시 복구 |
| M11 | 미디어(`core/av`) | MediaElement 재생 |
