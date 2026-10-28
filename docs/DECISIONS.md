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

## D5. 최소 Feature Level은 10.0 (제안, 사용자 확정 전)

D3D11의 Feature Level 9.x는 SM 4.0 level 9 프로필과 포맷·리소스 제약이 커서 별도 경로가 필요하다.
목적(최신 GPU에서 D3D9On12 회피)을 생각하면 10.0 미만 하드웨어는 소프트웨어 폴백으로 보내는 것이 단순하다.
Phase 2에서 확정한다.

## D6. ShaderEffect 호환: DX9 바이트코드를 네이티브에서 SM4로 변환 (방향, 세부는 Phase 2)

관리 `PixelShader`는 바이트코드 헤더의 버전 2바이트만 읽고 DUCE로 그대로 넘긴다. 따라서 네이티브 안에서
ps_2_0/ps_3_0 바이트코드를 SM4로 변환하면 관리 코드와 기존 앱의 `.ps` 파일을 바꾸지 않아도 된다.
`core/fxjit/PixelShader`에 소프트웨어 경로용 DX9 바이트코드 파서가 이미 있어 앞단을 재활용한다.
SM4 바이트코드를 직접 받는 확장은 관리 쪽 공개 API(`RenderCapability.IsPixelShaderVersionSupported`)를
건드리므로 이 프로젝트 범위 밖으로 둔다.

## D7. D3DImage 호환: interop 전용 D3D9Ex 디바이스를 남긴다 (방향, 세부는 Phase 2)

앱이 넘기는 `IDirect3DSurface9`를 받으려면 D3D9를 완전히 걷어낼 수 없다. `InteropDeviceBitmap`에서만
D3D9Ex 디바이스를 만들어 공유 서피스를 거쳐 D3D11 텍스처로 가져온다. 그 외 경로에서 `d3d9.dll`을 로드하지 않는다.

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
