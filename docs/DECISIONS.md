# 설계 결정

번호는 바뀌지 않는다. 뒤집으면 새 번호로 기록하고 옛 항목에 링크한다.

## D1. 관리/네이티브 계약을 동결한다 (2026-10-28)

PresentationCore와 `wpfgfx_cor3.dll`의 경계는 두 가지뿐이다: `core/dll/wpfgfx.def`의 export 107개와,
그 함수로 흘려보내는 DUCE 명령 바이트 스트림(`include/Generated/*`, codegen 산출물).
이 둘을 바꾸지 않으면 관리 어셈블리는 그대로 두고 네이티브 DLL만 교체할 수 있다.
export 중 D3D 타입이 드러나는 것은 `InteropDeviceBitmap_*` 4개뿐이고 그마저 `void*`로 서피스를 받는다.
**근거**: 프로토콜 지문 `MIL_SDK_VERSION 0x200184C0`과 `.def`가 v8.0.31부터 main까지 동일하고,
업스트림 소스로 빌드한 DLL이 .NET 8.0.31과 10.0.12 런타임 양쪽에서 그대로 동작함을 확인했다.

## D2. 기준 소스는 dotnet/wpf release/10.0, 커밋 01dce087b (2026-10-28)

DLL은 어느 .NET 버전의 앱에서도 돌므로(D1) 기준 소스는 지원 기간과 네이티브 코드 최신성으로 고른다.
.NET 10은 LTS(2028-11까지)이고, 8.0 이후 들어간 네이티브 오버플로 검사 커밋이 포함된다.
WpfGfx는 2019년 이후 업스트림 커밋이 50개 미만이고 8.0 이후로는 기능 변경이 없어 어느 버전을 골라도 코드는 사실상 같다.

## D3. 저장소에는 WpfGfx와 그 빌드에 필요한 파일만 둔다 (2026-10-28)

산출물이 `wpfgfx_cor3.dll` 하나이므로 관리 어셈블리, PresentationNative, 업스트림 테스트, CI 설정은 가져오지 않는다.
가져온 것: `src/Microsoft.DotNet.Wpf/src/WpfGfx` 전체, 네이티브 빌드가 읽는 `Shared/inc`·`Shared/cpp`·`Shared/OSVersionHelper`·
`Shared/Tracing`·`Shared/internal/multimedia/inc`, `redist/D3DCompiler`, `eng` 빌드 인프라와 루트 빌드 파일.
어떤 파일이 필요한지는 업스트림 전체 빌드의 MSBuild tracker 로그(`*.read.*.tlog`)에서 네이티브 프로젝트가 읽은 파일을 추려 정했다.
**경로는 업스트림과 같게 유지**한다. 그래야 `git diff upstream/release/10.0 -- <경로>`와 cherry-pick이 그대로 된다.
`eng/Build.props`로 빌드 대상을 `wpfgfx.vcxproj`와 `D3DCompiler.vcxproj`로 한정했다.

## D4. D3D11로 직접 치환한다. 추상화 레이어를 두지 않는다 (2026-10-28)

D3D9와 D3D11을 함께 지탱하는 추상화는 일이 두 배이고, D3D9 지원을 유지할 이유가 없다(목적 자체가 D3D9 탈출).
`core/hw`의 D3D9 코드를 D3D11로 바꾸고 D3D9 경로는 지운다. `core/sw` 소프트웨어 래스터라이저는 폴백으로 유지한다.
D3D11에는 고정 기능 파이프라인이 없으므로 `hwffshaders.cpp`의 고정 기능 경로와 ps_1_1 텍스트 셰이더는 제거하고
셰이더 파이프라인 하나로 통일한다.

## D5. 최소 Feature Level은 10.0 (2026-10-28 확정)

D3D11의 Feature Level 9.x는 SM 4.0 level 9 프로필과 포맷·리소스 제약이 커서 별도 경로가 필요하다.
목적(최신 GPU에서 D3D9On12 회피)을 생각하면 10.0 미만 하드웨어는 소프트웨어 폴백(`core/sw`)으로 보내는 것이 단순하다.
디바이스 생성은 `D3D_FEATURE_LEVEL_11_1, 11_0, 10_1, 10_0` 순으로 시도하고, 실패하면 `RenderCapability.Tier`를 0으로 보고한다.

## D6. ShaderEffect 호환: DX9 바이트코드를 네이티브에서 SM4로 변환 (방향, 세부는 Phase 2)

관리 `PixelShader`는 바이트코드 헤더의 버전 2바이트만 읽고 DUCE로 그대로 넘긴다. 따라서 네이티브 안에서
ps_2_0/ps_3_0 바이트코드를 SM4로 변환하면 관리 코드와 기존 앱의 `.ps` 파일을 바꾸지 않아도 된다.
`core/fxjit/PixelShader`에 소프트웨어 경로용 DX9 바이트코드 파서가 이미 있어 앞단을 재활용한다.
SM4 바이트코드를 직접 받는 확장은 관리 쪽 공개 API(`RenderCapability.IsPixelShaderVersionSupported`)를
건드리므로 이 프로젝트 범위 밖으로 둔다.

## D7. D3DImage 호환: interop 전용 D3D9Ex 디바이스를 남긴다 (방향, 세부는 Phase 2)

앱이 넘기는 `IDirect3DSurface9`를 받으려면 D3D9를 완전히 걷어낼 수 없다. `InteropDeviceBitmap`에서만
D3D9Ex 디바이스를 만들어 공유 서피스를 거쳐 D3D11 텍스처로 가져온다. 그 외 경로에서 `d3d9.dll`을 로드하지 않는다.

## D9. 래퍼 API는 유지하고 내부만 바꾼다 (2026-10-28)

`CD3DDeviceLevel1`, `CD3DResource` 계열, `CD3DRenderState`/`CHwRenderStateManager`, `CD3DSwapChain`, `CHwSurfaceRenderTarget`의
클래스 이름과 메서드 모양은 유지한다. 상위 레이어 수십 개 파일이 이 API에 의존하고, D3D9 객체는 전부 이 래퍼 안에 있다.
바뀌는 것은 래퍼 내부 구현과, 래퍼 바깥으로 새어 나간 D3D9 타입(`IDirect3DSurface9*`, `D3DFORMAT`, `D3DSURFACE_DESC` 등)뿐이다.
`DESIGN.md` §2·§3.

## D10. 스왑체인은 비트블트 모델(SEQUENTIAL, 백버퍼 1개)로 시작한다 (2026-10-28)

WPF는 더티 사각형만 다시 그리고 백버퍼 내용이 보존된다고 가정한다(D3D9 `D3DSWAPEFFECT_COPY`). 플립 모델은 버퍼가 순환해 이 가정을
깨므로 영속 서피스와 추가 복사가 필요하다. 먼저 의미가 같은 `DXGI_SWAP_EFFECT_SEQUENTIAL` + `Present1` dirty rect로 동작을 맞추고,
플립 모델 전환은 성능 작업으로 따로 결정한다. 투명 창은 `GDI_COMPATIBLE` 플래그 + `IDXGISurface1::GetDC`.

## D11. 셰이더 상수는 레지스터 번호 체계를 유지한다 (2026-10-28)

파이프라인 조립기와 효과 코드가 `c#` 레지스터 인덱스(`MILSPHandle`)로 상수를 설정한다. D3D11에서는 `cbuffer`+`packoffset(cN)`로
같은 번호를 유지하고, `SetVertexShaderConstantF/PixelShaderConstantF`는 그림자 배열에 쓰고 Draw 직전 한 번 업로드한다.
조립기·효과·텍스트 셰이더의 상수 전달 코드를 손대지 않기 위해서다. 프로필은 `vs_4_0`/`ps_4_0`.

## D12. WIP는 dx11-port 브랜치, main은 항상 초록 (2026-10-28)

D3D9를 직접 치환하면(D4) 빌드가 깨진 구간이 길다. 그 구간은 `dx11-port` 브랜치에서 작업하고, 마일스톤(DESIGN.md §12)마다
빌드·스모크 게이트를 통과한 상태로 `main`에 병합한다. `main`의 모든 커밋은 CLAUDE.md §3 게이트를 통과한다.

## D13. 첫 마일스톤은 "D3D9 없이 컴파일 + 소프트웨어 렌더링" (2026-10-28)

하드웨어 경로가 하나도 없어도 WpfGfx는 소프트웨어 래스터라이저로 모든 것을 그릴 수 있다(Tier 0). 타입 치환과 래퍼 골격만으로
D3D9를 완전히 제거한 DLL을 먼저 만들어 `main`에 올리면, 이후 하드웨어 경로를 연산 단위(연산별 소프트웨어 폴백이 이미 있다)로
켜 나갈 수 있다.

## D14. d3d11.lib/dxgi.lib는 정적 링크, d3d9.dll은 로드하지 않는다 (2026-10-28)

모든 지원 OS에 d3d11.dll과 dxgi.dll이 있으므로 `d3dloader.cpp`의 동적 로드 체계는 필요 없다. D3DImage의 `IDirect3DSurface9` 호환은
사용자 디바이스의 vtable 호출만 쓰므로 `d3d9.h` 헤더만 필요하고 `d3d9.dll`을 링크하거나 로드하지 않는다(DESIGN.md §9).
예외는 미디어 재생이다: EVR 믹서가 D3D9 전용이라 첫 재생 때 `d3d9.dll`을 지연 로드한다(D25). 렌더링만 하는 앱에서는 여전히 로드되지 않는다.

## D15. 텍스트 셰이더는 HLSL로 다시 쓰고 ps_1_1·L8 변종과 ShaderGen은 삭제한다 (2026-10-28)

ps_2_0 어셈블리 4종은 수식이 주석으로 남아 있어 HLSL로 옮기기 쉽다. 글리프 텍스처는 `A8_UNORM` 하나로 통일한다
(FL10.0 필수 포맷). RCDATA 블롭 대신 빌드 때 fxc가 만든 헤더를 쓴다.

## D16. 미디어는 마지막에, 그 전에는 소프트웨어 미디어 버퍼만 (2026-10-28)

EVR 프레젠터(DXVA2/D3D9)는 `IMFDXGIDeviceManager`+`ID3D11VideoProcessor`로 다시 써야 하는 독립 작업이다. M11까지는
하드웨어 비디오 경로를 끄고 `swmediabuffer`(시스템 메모리 프레임 업로드)만 쓴다.
M11에서 `IMFDXGIDeviceManager` 방식은 표준 EVR 믹서와 맞지 않는 것으로 확인되어 D25로 대체했다.

## D8. 새 기능은 추가 export + 동반 관리 어셈블리로 노출한다 (방향, 세부는 Phase 2)

관리 코드를 바꾸지 않으므로(D1) DX11 엔진에만 있는 기능(예: D3D11 텍스처를 바로 받는 D3DImage, 엔진 D3D11 디바이스 공유)은
앱이 기본 WPF API로는 호출할 수 없다. 해결:

- 네이티브: 기존 export 107개는 그대로 두고 **새 export를 추가**한다(`WpfGfxDx11_*` 접두사, 첫 번째는
  `WpfGfxDx11_GetInfo`로 ABI 버전·Feature Level 등을 보고).
- 관리: 별도 어셈블리 `WpfGfx.Dx11`(이 저장소의 `src/WpfGfx.Dx11/`, NuGet 배포 가능)이 그 export를 P/Invoke한다.
  이미 프로세스에 로드된 `wpfgfx_cor3.dll` 모듈에 `DllImportResolver`로 붙어, 앱이 어떤 방법으로 DLL을 교체했든 같은 인스턴스를 쓴다.
- 탐지와 실패: `Dx11Engine.IsAvailable`은 `NativeLibrary.TryGetExport`로 새 export 유무를 검사한다. 스톡 DX9 엔진이면 false이고,
  새 기능 API는 `NotSupportedException`을 던진다. export 없는 DLL에 대고 호출해서 크래시하는 일은 없어야 한다.
- D3DImage의 D3D11 텍스처 수용: 공개 `D3DImage.SetBackBuffer(IDirect3DSurface9, IntPtr)`는 포인터를 검사 없이 네이티브
  `InteropDeviceBitmap_Create`에 넘긴다. 네이티브가 `QueryInterface(IID_ID3D11Texture2D)`로 먼저 판별하면 D3D11 텍스처를
  무복사로 받을 수 있고, 관리 코드는 그대로다. `WpfGfx.Dx11`의 `D3D11Image` 헬퍼가 `IsAvailable` 검사 후 이 경로를 쓴다.
  스톡 엔진에서는 헬퍼가 먼저 거부하므로 잘못된 포인터가 네이티브로 가지 않는다.

## D7·D8 구현 메모 (2026-10-28, M9)

**D3DImage(`IDirect3DSurface9`) — 공개 계약 유지(D7).** `CInteropDeviceBitmap::GetUpdateMethod`가 사용자 디바이스를 판별한다.
`IDirect3DDevice9Ex`이고 `D3DCAPS2_CANSHARERESOURCE`·`D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES`가 있으면 `SharedSurface`,
아니면 기존 `Software`(`GetRenderTargetData`로 읽어 올리기). 공유 경로는 방향이 업스트림과 반대다: 엔진이
`D3D11_RESOURCE_MISC_SHARED`(+`BIND_RENDER_TARGET`) 텍스처를 만들고 그 핸들을 **사용자의 D3D9Ex 디바이스**가
`CreateTexture(..., &hShared)`로 열어(D3D9Ex는 D3D11 공유 핸들을 A8R8G8B8/X8R8G8B8로 열 수 있다) `StretchRect`로 사용자 서피스를
복사한다. 복사 완료는 사용자 디바이스에 만든 1×1 lockable 렌더타깃에 `StretchRect`+`LockRect`로 기다린다(`Flush9`). 엔진은
`d3d9.dll`을 로드하지 않고 사용자 객체의 vtable만 호출한다(D14).

**D3DImage(`ID3D11Texture2D`) — 새 기능(D8).** `InteropDeviceBitmap_Create`가 넘어온 포인터에 `QueryInterface(ID3D11Texture2D)`를
먼저 시도해 성공하면 `CreateForD3D11Texture`로 간다(관리 코드는 그대로). 조건: 2D·배열 1·`B8G8R8A8/X8_UNORM`·`USAGE_DEFAULT`,
어댑터 LUID로 디스플레이를 고른다. 갱신 방법 `D3D11Copy`: 엔진 디바이스의 텍스처면 `CopySubresourceRegion`(무복사에 가깝다),
같은 어댑터의 다른 디바이스면 `MISC_SHARED` 핸들을 `OpenSharedResource`로 열고 사용자 즉시 컨텍스트 `Flush` 뒤 복사(키드 뮤텍스
없음 — 순서는 D3D9와 같이 Lock/Unlock 프로토콜이 보장), 다른 어댑터면 사용자 디바이스에서 스테이징으로 읽어 시스템 메모리
텍스처로 올린다(미검증). MSAA 원본은 `ResolveSubresource`.

**export와 동반 어셈블리.** `core/api/api_dx11.cpp`의 `WpfGfxDx11_GetInfo`(cbSize로 시작하는 구조체, ABI 1: Feature Level·디스플레이
수·어댑터 LUID)와 `WpfGfxDx11_GetDevice`(디스플레이 인덱스 → AddRef된 `ID3D11Device`). 디바이스는 미디어 프레젠터와 같은 경로
(`GetD3DDeviceAndPresentParams`)로 얻어 창이 없을 때도 만들어진다. `src/WpfGfx.Dx11/`(net8.0-windows): `Dx11Engine.IsAvailable`
(`GetModuleHandle` + `TryGetExport`), `GetInfo`, `GetDevice`, `D3D11Image : D3DImage`(스톡 엔진이면 `NotSupportedException`).
즉시 컨텍스트는 엔진 렌더 스레드 것이므로 앱은 쓰지 않는다. 사용자 디바이스+공유 텍스처 방식은 D19의 전제이기도 하다.

## D17. D3D11.1 런타임(Windows 8 이상)을 요구한다 (2026-10-28)

`ID3D11DeviceContext1::ClearView`(사각형 단위 클리어, D3D9 `Clear`의 rect 목록 대응)와 `IDXGISwapChain1::Present1`(dirty rect)을
쓴다. .NET 8 이상 WPF가 지원하는 OS는 모두 D3D11.1을 포함하므로 Windows 7 경로는 두지 않는다. Feature Level은 D5대로 10.0까지 내려간다.

## D18. 고정 기능 파이프라인은 셰이더로 에뮬레이션한다 (2026-10-28)

D3D11에는 고정 기능(FF) 정점 변환·텍스처 스테이지 결합기가 없다. `CHwFFPipeline`과 `CD3DRenderState::SetTextureStageOperation`
등 호출부를 유지하기 위해 `HwFixedFunctionShaders.cpp`가 정점 포맷별 입력 레이아웃·변환 VS와 텍스처 스테이지 조합별 PS를
런타임에 조립해 D3DCompile로 만든다. 범프 매핑(`sc_tsoBumpMapTexture`)은 Modulate로 두고 에뮬레이션하지 않는다(FL10 디바이스는
항상 셰이더 파이프라인을 택하므로 선택되지 않는다). 파이프라인 조립기의 D3D9 문법 HLSL은 M3까지
`D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY`로 컴파일한다.

## D19. 사용자 D3D11 렌더 루프는 MediaElement 방식의 프레임 큐로 컴포지션에 잇는다 (2026-10-28, 방향 → 구현은 docs/FRAME_QUEUE.md)

MediaElement가 빠른 이유는 (1) 컴포지션 엔진과 같은 멀티스레드 디바이스의 텍스처에 다른 스레드가 바로 그리고, (2) 컴포지션 스레드가
패스마다 가장 최근에 완성된 프레임 하나만 집어 그리며, (3) 새 프레임이 `CMilSlaveVideo::NewFrame` → `ScheduleCompositionPass`로
UI 스레드 없이 컴포지션을 깨우기 때문이다. D3DImage는 프레임마다 UI 스레드의 Lock/AddDirtyRect/Unlock을 거쳐야 하고 D3D9Ex 공유
서피스를 요구해 이 셋을 모두 놓친다. HWND마다 스왑체인을 두는 우회는 에어스페이스 문제가 있다.

결정: D8의 `WpfGfx.Dx11`에 "프레임 큐" 객체를 추가한다. 네이티브 export로 텍스처 링(엔진 디바이스 또는 사용자 디바이스의 공유
텍스처 + keyed mutex)을 만들고, 사용자는 어느 스레드에서든 `Acquire`/`Present`만 호출한다. `Present`가 컴포지션 패스를 예약하고,
컴포지션은 마지막 Present된 텍스처를 비디오처럼 그린다. 관리 코드는 기존 `D3DImage.SetBackBuffer`에 이 객체를 한 번 넘기는 것으로
끝난다(네이티브가 QueryInterface로 판별). 소프트웨어 컴포지션에서는 비디오와 같은 리드백 폴백을 쓴다. 세부 API는 M9 이후 확정.

## D10 개정. 프레젠트는 항상 백버퍼 전체를 복사한다 (2026-10-28)

`Present1`의 dirty rect는 창 서피스를 사각형 안만 갱신한다. D3D9는 창 서피스가 새로 만들어지면(표시·크기 변경) 전체를 다시
복사했지만 DXGI는 그러지 않아 첫 프레임 뒤 창이 검게 남았다. 비트블트 모델에서 전체 복사는 DWM 쪽 GPU 블릿 한 번이라 비용이 작다.
dirty rect 최적화는 플립 모델 전환과 함께 다시 검토한다.

## D20. 모든 버텍스 셰이더는 입력 선언 하나를 공유한다 (2026-10-28)

D3D11은 입력 레이아웃의 시맨틱·레지스터가 현재 VS 시그니처와 맞아야 한다. 버텍스 형식마다 다른 VS 선언을 쓰면 레이아웃이
VS마다 필요하다. 대신 모든 VS(고정 기능 에뮬레이션, 파이프라인 조립기, 효과)가 `MIL_HLSL_VERTEX_INPUT`(POSITION, NORMAL,
COLOR0/1, TEXCOORD0~7) 하나를 선언하고, 형식에 없는 속성은 입력 레이아웃이 슬롯 1 의 0 버퍼에서 인스턴스 단계 0 으로 읽는다.
레이아웃은 형식당 하나, VS 와 무관하게 재사용된다.

## D21. 조립 셰이더의 상수 레지스터 번호 유지 방법 (D11 보강, 2026-10-28)

- SM4 의 `$Globals` 는 참조하지 않는 전역도 자리를 차지하므로, 한 HLSL 안의 VS/PS 절반을 `#ifdef MIL_VS`/`MIL_PS` 로 감싸고
  단계별로 define 을 붙여 컴파일한다. 그래야 PS 상수가 c0 부터 시작한다.
- 엔진은 float~float4 상수를 레지스터 하나로 센다(D3D9 할당 규칙). SM4 는 작은 멤버를 한 레지스터에 묶으므로 조립기가 구조체
  멤버마다 패딩을 넣어 레지스터 하나씩 차지하게 한다.
- 샘플러는 `register(s<스테이지>)` 로 고정한다. 호환 모드에서 텍스처도 같은 슬롯 번호를 쓴다.

## D22. 진단 로그는 릴리스 빌드에 남긴다 (2026-10-28)

`WPFGFX_DX11_LOG` 환경 변수가 있을 때만 동작하는 로그(디버그 레이어 메시지, 조립 HLSL, 바인딩 리플렉션, 백버퍼 덤프)를 엔진에 둔다.
디버거 없이 렌더 스레드 실패의 원인을 바로 볼 수 있어 포팅 전 기간에 가치가 크고, 변수가 없으면 비용이 없다. `docs/BUILD.md`.

## D6 구현 메모. ShaderEffect 변환기는 토큰 수준에서 직접 HLSL 을 낸다 (2026-10-28)

`core/hw/HwShaderEffectTranslator.cpp`. fxjit 파서를 재사용하지 않고 d3d9types.h 의 토큰 정의로 직접 걷는다(의존 없음).
명령 하나가 HLSL 한 줄이고, r#/v#/t#/oC0 은 지역 float4, c#/i#/b# 는 상태 관리자와 같은 cbuffer 배열(b0 `c[224]`, b1 `i[16]`/`b[16]`),
샘플러는 같은 번호의 t#/s#. `def/defi/defb` 는 지역 상수로 둔다(D3D9 에서도 리터럴은 컴파일러가 빈 레지스터에 두므로 앱이 덮어쓰지
않는다). 입력은 효과 VS 가 내는 COLOR0 과 TEXCOORD0 뿐이고, 그 밖의 선언은 0 으로 채운다. 예측 레지스터·서브루틴·깊이 출력·
큐브/볼륨 샘플러·다중 렌더 타깃은 실패로 돌려 WPF 의 통과 셰이더 폴백(InvalidPixelShaderEncountered)을 타게 한다.
엔진 자체의 통과 셰이더 리소스는 이미 SM4(DXBC) 이므로 변환기를 거치지 않는다.

## D23. 디바이스는 디스플레이별로 두고, 한 HWND에 디스플레이 수만큼 스왑체인을 만든다 (2026-10-28)

스톡 D3D9 엔진은 모니터(D3D9 어댑터)마다 디바이스를 만들고 창이 걸친 모니터마다 스왑체인을 같은 HWND에 만들어 자기 영역만 프레젠트한다.
D3D11에서도 이 구조를 그대로 둔다(`CD3DDeviceManager::FindDeviceMatch`는 디스플레이 인덱스로 매칭). 같은 GPU의 두 모니터면 D3D11 디바이스가
두 개 생기지만, DXGI 비트블트 모델(`SEQUENTIAL`)은 한 HWND에 여러 스왑체인을 허용하므로 동작은 스톡과 같다(M10에서 확인).
LUID가 같은 디스플레이가 디바이스 하나를 공유하는 최적화(캐시·셰이더 중복 제거)는 Phase 4 성능 항목이다. 플립 모델로 바꾸면 HWND당 스왑체인이
하나여야 하므로 그때는 이 결정을 다시 본다(D10).

투명 창은 GDI 호환 스왑체인(`DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE`) + `CD3DSwapChainWithSwDC`(백버퍼를 시스템 메모리 DIB로 읽어
`UpdateLayeredWindow`)로 간다. 스톡도 lockable 백버퍼를 읽어 올렸으므로 비용은 같다. `IDXGISurface1::GetDC` 직접 경로는 쓰지 않는다.

## D24. 디바이스 손실 복구는 엔진 테스트 훅으로 검증한다 (2026-10-28)

D3D11에는 디바이스 제거를 일으키는 공개 API가 없고(D3D12의 `RemoveDevice`에 해당하는 것이 없다), TDR을 강제하는 방법은 PC 전체에 영향을 준다.
그래서 진단 로그(D22)와 같은 방식의 환경 변수 `WPFGFX_DX11_TEST_DEVICE_LOSS=<n>`을 두어 n번째 `Present`를 `D3DERR_DEVICELOST`로 처리하고
그 디바이스의 `CheckDeviceState`도 실패하게 한다. 이것으로 엔진의 복구 경로(`MarkUnusable` → 디바이스 목록 정리 → 새 디바이스·스왑체인·
리소스 재생성 → D3DImage 프런트 버퍼 알림)를 끝까지 검증한다. 드라이버가 실제로 리소스를 무효화하는 상황은 이 훅이 흉내내지 못하므로
실제 GPU 제거·TDR 검증은 Phase 4에 남긴다. 변수가 없으면 비용은 카운터 하나다.

## D25. 미디어는 AV 전용 D3D9Ex 디바이스에서 믹싱하고 공유 서피스로 D3D11에 잇는다 (2026-10-28, D14·D16 수정)

WPF의 미디어 스택은 WMP OCX + DirectShow EVR 필터에 **표준 믹서 + WpfGfx 커스텀 프레젠터**를 꽂은 구조다. Microsoft 문서
("How to Write an EVR Presenter")대로 표준 믹서와 프레젠터는 Direct3D 9를 쓰며, 표준 믹서와 함께 쓰는 커스텀 프레젠터는
`IMFVideoDeviceID`로 `IID_IDirect3DDevice9`를 보고해야 한다. 믹서에 `IMFDXGIDeviceManager`를 줄 방법은 없다. 믹서까지 바꾸면
(커스텀 `IMFTransform` + `ID3D11VideoProcessor`) DirectShow 디코더의 DXVA2 하드웨어 디코드를 잃고 `core/av` 3만 줄의 상당 부분을
다시 써야 한다. 그래서:

- 엔진 디바이스(`CD3DDeviceLevel1`)마다 같은 어댑터(LUID)의 **D3D9Ex 디바이스**를 하나 만들어 매단다(`core/av/avd3d9.cpp`
  `AVEnsureMediaDevice9`, `CD3DDeviceLevel1::SetMediaDevice9`). `MULTITHREADED|FPU_PRESERVE|NOWINDOWCHANGES`, 1×1 백버퍼, 프레젠트 없음.
  프레젠터의 `GetHWDevice`/`GetSWDevice`가 엔진 디바이스를 얻을 때 함께 만들고, DXVA2 매니저 `ResetDevice`에 이 디바이스를 준다.
  `GetUnderlyingDevice`는 이제 이 디바이스를 돌려준다. `d3d9.dll`은 여기서만 `LoadLibrary` 한다(D14 수정).
- 믹서 출력 텍스처(`CHWMFMediaBuffer`)는 D3D9Ex에서 **공유 핸들**로 만들고, 엔진 디바이스가 `OpenSharedResource`로 열어
  `CacheHwTextureOnBitmap`으로 비트맵의 색 소스로 캐시한다(스톡과 같은 무복사 경로). 다른 어댑터의 디스플레이는 기존처럼
  `GetRenderTargetData`→시스템 메모리 복사.
- 동기화: 믹서 `ProcessOutput`이 성공할 때마다 `CMFMediaBuffer::NotifyMixerWrote`가 D3D9 이벤트 쿼리로 GPU 완료를 기다린 뒤
  샘플을 컴포지션에 넘긴다(프레젠터 스레드만 멈춘다). 반대 방향(컴포지션 드로우가 실행되기 전에 믹서가 같은 텍스처를 다시 쓰는
  경우)은 샘플 큐 깊이와 제출 순서에 기대며, 스톡 D3DImage와 같은 수준의 보장이다.
- 소프트웨어 폴백(렌더 디바이스가 WARP)도 믹싱은 D3D9Ex에서 한다. EVR 믹서에는 시스템 메모리 모드가 없다. D3D9 HAL이 없으면
  미디어는 `WGXERR_AV_VIDEOACCELERATIONNOTAVAILABLE`로 실패한다(Windows 8 이후 기본 렌더 드라이버가 D3D9도 제공하므로 드물다).
- 장기: WMP OCX를 `IMFMediaEngine`(DXGI 디바이스 매니저)으로 바꾸면 D3D9 의존이 없어지지만 `wmpstateengine` 등 재작성이 필요해
  Phase 4 이후 별도 결정으로 둔다.

## D26. 중간 텍스처를 디바이스 풀에서 재사용한다 (2026-10-28)

WPF 는 효과·브러시 중간 렌더타깃과 글리프·비트맵 업로드용 락 가능 텍스처(기본+스테이징 쌍)를 **프레임마다 만들고 버린다**.
D3D9 드라이버는 이 패턴을 싸게 처리했지만 D3D11(NVIDIA)에서는 `CreateTexture2D`/파괴가 비싸고 지연 파괴 풀이 커져 프라이빗
메모리가 수십 MB 요동했다(`WPFGFX_DX11_LOG` 의 생성 카운터로 프레임당 약 5개 확인). 그래서 `CD3DDeviceLevel1` 에 텍스처 풀을 둔다.

- 반납: `CD3DTexture::ReleaseD3DResources`(리소스 매니저가 디바이스 락 아래서 호출)와 소멸자에서 `ReturnTextureToPool`. 자기 서피스
  래퍼가 아직 쥔 참조만 남은 텍스처만 받는다(래퍼는 곧 같이 사라진다). 대상: `USAGE_DEFAULT` + RT/SRV 바인드, 또는 `STAGING`.
  공유·밉맵·배열·4096 초과·디바이스 손실 중은 제외. 48개·96 MB 상한, 가득 차면 가장 오래된 항목을 버린다(LRU), 300 프레젠트
  동안 안 쓰인 항목도 버린다.
- 대여: `CreateTexture` 가 **같은 서술자**를 먼저 풀에서 찾는다. 스테이징 텍스처는 복사가 진행 중일 수 있어 한 프레젠트 쉬고 재사용한다.
  회전하는 요소의 효과용 중간 렌더타깃은 크기가 매 프레임 1~2픽셀씩 달라져 정확 일치로는 재사용이 안 된다. "각 변 64픽셀 이내로 더 큰
  텍스처 재사용 + 16픽셀 올림"을 시도했으나 효과 파이프라인이 중간 텍스처를 실제 크기 기준 UV 로 샘플링해 그림자가 프레임마다
  떨렸으므로 뺐다(`fAllowLargerFromPool` 은 남겨 두되 모든 호출이 false). 이 패턴의 생성 비용은 남는다.
- 풀은 D3D11 객체 수명을 바꾸지 않는다(참조 하나를 풀이 보유). 디바이스 손실 때 전부 비운다.

## D27. RenderTargetBitmap 하드웨어 경로는 옵트인이고, 즉시 컨텍스트는 런타임 멀티스레드 보호를 켠다 (2026-10-28)

`RenderTargetBitmap` 은 스톡에서 항상 소프트웨어 래스터라이저로 그려져 ps_3_0 `ShaderEffect` 가 통과 셰이더로 대체된다(dotnet/wpf#7856).
D3D11 엔진은 `CMILFactory::CreateBitmapRenderTarget` 에서 `CHwTextureRenderTarget::CreateForBitmap`(GPU 텍스처 타깃, 요청 DPI, 투명 클리어)을
쓸 수 있다. 출력이 소프트웨어 경로와 픽셀 단위로 다르므로(AA·텍스트) **기본은 꺼짐**이고 `WpfGfxDx11_SetOption(1, 1)` /
`Dx11Engine.HardwareRenderTargetBitmap = true` / 환경 변수 `WPFGFX_DX11_HW_RTB=1` 로 켠다(ABI 2). 비트맵은 `CDeviceBitmap` 이라 UI 스레드의
`CopyPixels` 가 GPU 리드백을 하고, 같은 디바이스의 이미지로 쓰면 복사 없이 텍스처를 샘플링한다.

UI 스레드가 렌더 스레드와 같은 즉시 컨텍스트를 쓰게 되므로(RTB 렌더는 동기 채널, 리드백은 `CDeviceBitmap::Lock`) 디바이스 생성 때
`ID3D11Multithread::SetMultithreadProtected(TRUE)` 를 켠다. D3D9 의 `D3DCREATE_MULTITHREADED` 에 해당하며 엔진의 디바이스 락은 그 위에 그대로 있다.
`WpfGfxDx11_GetDevice` 로 디바이스를 받은 앱 코드도 같은 보호를 받는다.

## D28. 같은 어댑터(LUID)의 디스플레이는 하드웨어 디바이스 하나를 공유한다 (2026-10-28, D23 수정)

D23 은 스톡처럼 디스플레이마다 디바이스를 두었다. 같은 GPU 의 모니터 두 개면 D3D11 디바이스가 둘이고(드라이버 커밋이 디바이스마다
약 180 MB), 걸친 창은 양쪽 디바이스에 리소스를 두 벌 둔다. 이제 `CD3DDeviceManager::FindDeviceMatch` 가 디스플레이 인덱스 대신
**디스플레이의 어댑터 LUID** 로 하드웨어 디바이스를 찾는다(`DisplaysShareAdapter`). 디스플레이별 렌더타깃·스왑체인 구조는 그대로이며
(한 HWND 에 디스플레이 수만큼 비트블트 스왑체인), 디바이스·텍스처 캐시·텍스처 풀·미디어 D3D9Ex 디바이스만 공유된다.

- 어댑터 상태 알림(`NotifyAdapterStatus`)은 디바이스가 생기거나 사라질 때 그 LUID 의 **모든** 디스플레이 인덱스로 나간다
  (`NotifyAdapterStatusForGroup`). D3DImage·프레임 큐는 자기 디스플레이 인덱스만 보므로 바뀐 것이 없다.
- `CD3DDeviceLevel1::m_uAdapterOrdinal` 은 처음 만든 디스플레이의 인덱스로 남는다(레지스트리 어댑터 기록 용도). 디스플레이-디바이스
  일치 Assert 는 LUID 비교로 바꿨다.
- 결과(모니터 2개·GPU 1개, 2026-10-28): 디바이스 2→1. 걸친 창 프라이빗 536→390 MB, GPU 로컬 371→130 MB. 한 모니터 안의 창은
  344→336 MB(두 번째 디바이스는 거의 놀고 있었다). 걸친 창에서 디바이스 손실을 일으켜도 디바이스 하나가 다시 만들어지고 양쪽이 복구된다.
- 디바이스는 하나인데 렌더타깃이 디스플레이마다 있으면(이 결정만 적용한 상태) 걸친 창의 블러 셀이 스톡과 1.3% 달랐다(차이>64 는 0).
  두 렌더타깃이 실현 캐시를 공유하면서 생기는 차이로, D29 가 창마다 렌더타깃 하나로 만들어 없앴다(0.04%). 풀 재사용 때 렌더타깃을
  클리어해 봤지만 원인이 아니어서 넣지 않았다.

## D29. 어댑터가 같은 디스플레이들은 창마다 하드웨어 렌더타깃 하나를 쓰고, 그 스왑체인은 플립 모델이다 (2026-10-28, D10·D23 수정)

**창 하나에 렌더타깃 하나.** 모든 디스플레이의 LUID 가 같으면(`CDisplaySet::AllDisplaysShareAdapter`) `CDesktopRenderTarget::Init` 이
디스플레이 수만큼이 아니라 **하나의** `CHwHWNDRenderTarget` 을 만들고 가상 디바이스 경계를 데스크톱 전체로 둔다(소프트웨어 경로의
`SetSingleSubRT` 와 같은 구조). 창이 두 모니터에 걸쳐도 한 번만 렌더링되고 HWND 에 스왑체인이 하나다. 만들기에 실패하면 디스플레이별
렌더타깃으로 되돌아간다. 어댑터가 다른 디스플레이(GPU 두 개)는 종전처럼 디스플레이별 렌더타깃·비트블트 스왑체인이다.
결과(모니터 2개·GPU 1개): 걸친 창이 스톡과 차이>8 0.04%(D28 만으로는 1.31%), 프라이빗 메모리 390→331 MB(한 모니터 안의 창과 같다).

**플립 모델.** HWND 의 유일한 하드웨어 렌더타깃이고 HAL 로 프레젠트하면(투명 창의 GDI 경로 제외) `MilSwapChainParams::FlipModel` 을
켜고 `CreateAdditionalSwapChain` 이 `DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL`·백버퍼 2개로 만든다(DXGI 가 거부하면 비트블트로 폴백).
D10 이 걱정한 "더티 사각형만 다시 그린다" 가정은 이렇게 지킨다.

- DXGI 는 프레젠트마다 버퍼를 돌리고 버퍼 1 은 **방금 보여준 프레임**(읽기 전용)이 된다. `CD3DSwapChain::SyncBackBufferAfterPresent` 가
  프레젠트 직후 이번 프레임의 더티 사각형(없으면 프레젠트 사각형, 16개 초과면 경계 사각형)을 버퍼 1 에서 새 백버퍼로 복사해 백버퍼를
  완전한 상태로 돌려 놓는다. 중간 서피스가 없으니 추가 메모리는 백버퍼 하나뿐이다. 새로 만들거나 크기를 바꾼 뒤 첫 프레젠트와 범위를
  모르는 경우는 전체를 복사하고, 돌지 않은 프레젠트(가려짐)는 바뀐 영역을 모아 두었다가 다음 프레젠트 뒤에 복사한다.
- `Present1` 에 같은 더티 사각형을 넘겨 DWM 이 바뀐 부분만 합성하게 한다(비트블트 경로는 종전대로 전체 프레젠트). 사각형은 백버퍼
  안으로 잘라 넘긴다(벗어나면 `DXGI_ERROR_INVALID_CALL`).
- 런타임이 플립 프레젠트 뒤 백버퍼를 파이프라인에서 떼므로 디바이스의 렌더타깃 캐시를 비운다(`ReleaseUseOfRenderTarget`).
  안 하면 다음 프레임이 아무 데도 안 그려진다.
- 크기 변경은 `CD3DSwapChain::Resize` 가 `ResizeBuffers` 로 제자리에서 한다. 플립 체인을 다시 만들면 그 사이 창이 한 번도 칠한 적 없는
  리디렉션 서피스를 보이기 때문이다. `ResizeBuffers` 는 버퍼 참조가 전부 사라져야 하는데 엔진은 놓은 래퍼를 다음 프레젠트에 파괴하므로
  `DestroyResources(WithDelay)` 로 즉시 파괴하고, 바인딩된 백버퍼는 먼저 뗀다. 스모크 `--resize 16`: 매 단계 검은 픽셀 0.12% 로 스톡과
  같다(게이트 `resize` 행). 로그에 `ResizeBuffers failed` 가 없어야 한다.
- 프레젠트 동기 간격은 WPF 가 요구하는 대로 0(`PresentImmediately`)이라 렌더 스레드가 수직 동기에 막히지 않는다.
- **백버퍼는 3개다(2026-10-28 수정).** 2개면 DWM 이 수직 동기 때 버퍼를 놓아줄 때까지 GPU 가 다음 프레임을 시작하지 못하고, 그 뒤에
  줄 선 같은 프로세스의 모든 GPU 동기(레이어드 창 리드백, `CopyPixels`)가 평균 8 ms 를 기다렸다(L20). 3개면 GPU 가 항상 빈 버퍼를
  갖는다. 비용은 창 크기 버퍼 하나. 프레젠트 뒤 복사 범위는 "이번 프레임 ∪ 건너뛴 프레젠트가 남긴 영역 ∪ **직전 프레임**" 이 된다
  (새 백버퍼는 두 프레젠트 전 상태이므로). 새로 만든 뒤 첫 두 프레젠트는 전체를 복사한다.

측정(게이트 `bench` 행, 2026-10-28)은 `docs/ROADMAP.md` Phase 4 참조.

## D30. 투명 창(AllowsTransparency)은 UpdateLayeredWindow 경로를 유지한다 (2026-10-28)

ROADMAP 의 "투명 창을 DirectComposition + 프리멀티플라이드 알파 스왑체인으로 프레젠트" 항목은 **하지 않기로** 했다.

- WPF 의 투명 창은 관리 코드가 `WS_EX_LAYERED` 로 만들고, OS 는 `UpdateLayeredWindow` 로 넘긴 비트맵의 알파로 **픽셀 단위 히트 테스트**를
  한다(완전히 투명한 픽셀은 클릭이 아래 창으로 간다). DirectComposition 으로 그리면 보이는 것은 바꿀 수 있어도 히트 테스트는 여전히 레이어드
  비트맵을 보므로, 동작을 지키려면 매 프레임 리드백 + `UpdateLayeredWindow` 가 그대로 필요하다. 그러면 얻는 것이 없다.
- `WS_EX_NOREDIRECTIONBITMAP` 창은 창을 만드는 관리 코드를 바꿔야 해서 D1 밖이다.
- 스톡 엔진도 같은 리드백(D3D9 `GetRenderTargetData` → ULW)을 하므로 지금 경로가 스톡과 같은 비용 구조다. 비용은 게이트 `bench ...
  (transparent window)` 행으로 측정한다.

## D31. 레이어드 창의 리드백은 창 크기의 스테이징 텍스처 하나로 더티 영역만 한다 (2026-10-28)

`AllowsTransparency` 창은 `UpdateLayeredWindowIndirect` 에 넘길 DIB 를 매 프레임 GPU 에서 읽어 내린다(D30). 종전 구현은 더티
사각형 크기의 스테이징 텍스처를 **프레젠트마다 만들고 버렸고**(L13 과 같은 부류), 스왑체인에 `GDI_COMPATIBLE` 플래그를 달았지만
실제로는 서피스의 `GetDC` 를 쓰지 않았다. 이제 `CD3DSwapChainWithSwDC` 가 DIB 와 같은 크기의 스테이징 텍스처를 한 번 만들어 두고,
프레젠트마다 더티 사각형만 `CopySubresourceRegion` → `Map(READ)` → DIB 로 옮긴다. `GDI_COMPATIBLE` 은 BGRA 가 아닌 형식(서피스
`GetDC` 가 필요한 경로)에만 남긴다. `UpdateLayeredWindowIndirect` 의 `prcDirty` 는 전부터 더티 사각형이었다.

측정(벤치, 900×700 투명 창): 전체 갱신 17.0 → 19.5 fps, 부분 갱신(움직이는 요소 24개) 59.7 → 107.6 fps(스톡 98). 부분 갱신의
60 fps 상한은 리드백이 아니라 D29 의 2버퍼 플립 체인이 만든 GPU 정체였고, 3버퍼로 풀렸다(L20).
`WPFGFX_DX11_LOG` 에 `timing gdi readback/ulw/wait to present/vblank wait` 줄(300회 평균·최대)이 추가됐다.
