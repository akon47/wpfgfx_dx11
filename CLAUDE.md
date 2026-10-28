# CLAUDE.md — wpfgfx_dx11

> 이 파일은 항상 로드되는 **계약서**다. 여기 규칙은 어기지 않는다.
> 상세는 `docs/`에 있다. 충돌 시 우선순위: `docs/DECISIONS.md` > `docs/ROADMAP.md` > 이 파일.
> 이 저장소는 **누구나 클론해서 이어서 작업할 수 있어야 한다.** 특정 PC에만 있는 파일·경로·계정·장비에 의존하는 내용을 코드·문서·커밋 메시지에 넣지 않는다. 참고가 필요한 자료는 저장소 안(`docs/`)에 넣고 그것을 가리킨다.

## 1. 프로젝트 한 줄 정의

WPF의 네이티브 렌더링 엔진 **WpfGfx(milcore, `wpfgfx_cor3.dll`)의 내부를 Direct3D 9에서 Direct3D 11로 옮긴다.** 관리 코드(PresentationCore 등)는 손대지 않는다. 산출물은 DLL 하나이며, .NET 8/9/10 WPF 앱의 `wpfgfx_cor3.dll`을 그대로 교체해서 쓴다.

## 2. 확정 결정 (임의 변경 금지 — 근거는 `docs/DECISIONS.md`)

| 항목 | 결정 |
|---|---|
| 기준 소스 | dotnet/wpf `release/10.0`, 커밋 `01dce087b`. (D2) |
| 저장소 범위 | `src/Microsoft.DotNet.Wpf/src/WpfGfx`와 그 빌드에 필요한 파일만(`Shared` 헤더 일부, `redist/D3DCompiler`, `eng` 빌드 인프라). 관리 어셈블리·PresentationNative·업스트림 테스트는 가져오지 않는다. **경로는 업스트림과 같게 유지**한다(diff·cherry-pick 용이). (D3) |
| 관리/네이티브 계약 동결 | `core/dll/wpfgfx.def`의 export 107개, `include/Generated/*`의 DUCE 명령 구조체, `MIL_SDK_VERSION`(0x200184C0)을 바꾸지 않는다. codegen을 돌리지 않는다. (D1) |
| 백엔드 | D3D9 하드웨어 경로를 D3D11로 **직접 치환**한다. 추상화 레이어로 두 백엔드를 공존시키지 않는다. `core/sw` 소프트웨어 래스터라이저는 폴백으로 유지한다. (D4) |
| 호환성 | `D3DImage`(IDirect3DSurface9)와 `ShaderEffect`(ps_2_0/ps_3_0 바이트코드)는 공개 API이므로 깨지 않는다. 방법은 D6·D7. 미확정 항목은 구현 전에 결정을 기록하고 사용자 확인을 받는다. |
| 빌드 시스템 | 업스트림 Arcade(`build.cmd`)를 유지한다. CMake 등으로 갈아타지 않는다. |
| 아키텍처 | x64 우선. x86·arm64는 Phase 4 이후. |

## 3. 빌드·검증 명령 (이 순서로 통과해야 "완료")

```powershell
# 1) 엔진 빌드. 비-UTF-8 코드 페이지 로캘(한국어 CP949 등)에서는 CL=/wd4819가 필수 — docs/BUILD.md
$env:CL = "/wd4819"
.\build.cmd -configuration Release -platform x64 -nodeReuse $false
# 산출물: artifacts\bin\wpfgfx\x64\Release\wpfgfx_cor3.dll
#         artifacts\bin\D3DCompiler\x64\Release\D3DCompiler_47_cor3.dll  (같은 폴더에 두고 테스트)

# 2) 교체 스모크 — 두 런타임 모두
cd tests\SmokeApp
dotnet build -c Release
dotnet build -c Release -p:SmokeTfm=net10.0-windows
.\bin\Release\net8.0-windows\win-x64\SmokeApp.exe  --wpfgfx <DLL 폴더> --screenshot ..\out\net8.png  --exit
.\bin\Release\net10.0-windows\win-x64\SmokeApp.exe --wpfgfx <DLL 폴더> --screenshot ..\out\net10.png --exit
```

합격 기준: 보고서의 `wpfgfx_cor3.dll` 경로가 지정한 폴더이고 파일 버전이 `42.42.42.42424`, `Tier=2`, 스크린샷이 스톡 DX9 출력과 비교해 **의도한 차이만** 있다.

## 4. 완료의 정의 (전부 충족)

- 빌드 경고 0, 오류 0 (저장소는 경고를 오류로 취급한다).
- §3 스모크가 .NET 8과 .NET 10에서 모두 통과.
- 바꾼 영역의 렌더링을 스톡 DLL 출력과 스크린샷으로 비교해 결과를 기록.
- 비자명한 문제의 해결과 설계 결정을 `docs/LEARNINGS.md`·`docs/DECISIONS.md`에 기록 (§8.5).
- §7 규칙으로 커밋.

## 5. 반드시 지킬 비자명 요구사항 (놓치기 쉬움)

- ★ **앱 출력 폴더에 `wpfgfx_cor3.dll`을 복사해도 교체되지 않는다.** 프레임워크 종속 앱의 네이티브 DLL 탐색 경로에는 공유 프레임워크 폴더만 들어 있다. 교체 방법은 둘뿐이다: (a) 스모크 앱처럼 `NativeLibrary.SetDllImportResolver`로 돌리기, (b) 공유 프레임워크 폴더의 파일을 바꾸기(머신 전체에 영향).
- ★ **`D3DCompiler_47_cor3.dll`은 `wpfgfx_cor3.dll`과 같은 폴더**에 있어야 한다. wpfgfx가 자기 폴더에서 지연 로드한다.
- ★ `CL` 환경 변수는 MSBuild 노드가 물려받는다. 값을 바꾸면 살아 있는 노드를 먼저 끝낸다(`-nodeReuse $false`).
- ★ `include/Generated/`와 `codegen/`은 읽기 전용으로 취급한다. 명령 구조체가 바뀌면 관리 코드와 어긋나 즉시 깨진다.
- ★ 관리 코드(PresentationCore 등)를 봐야 하면 이 저장소가 아니라 업스트림에서 본다: `git remote add upstream https://github.com/dotnet/wpf.git && git fetch upstream release/10.0` 뒤 `git show upstream/release/10.0:<경로>`.
- ★ 로컬 빌드 파일 버전 `42.42.42.42424`는 의도된 값이다(Arcade 기본). 바꾸지 않는다. 스톡 DLL과 구분하는 표식이다.
- `RenderCapability.Tier`, `IsPixelShaderVersionSupported` 등 관리 API가 네이티브에 묻는 값의 의미를 바꾸지 않는다(Tier 2 = 하드웨어 가속).
- 비밀·개인 정보·절대 경로·PC 이름을 커밋하지 않는다. `tests/out`(스크린샷)·`artifacts`·`.dotnet`·`.packages`는 ignore 대상이다.

## 6. 코딩 컨벤션

- 기존 milcore 스타일을 따른다: `HRESULT` 반환, `IFC`/`IFCW32` 매크로, `ReleaseInterface`, `Assert`, `TraceTag`. 새 의존성(라이브러리·패키지) 추가 금지. D3D11/DXGI 헤더는 Windows SDK 것만 쓴다.
- D3D9 코드를 지울 때는 통째로 지운다. `#ifdef`로 두 경로를 남기지 않는다(D4). D3DImage interop용 D3D9Ex만 예외(D7).
- 셰이더: HLSL 소스는 `core/hw/shaders`, 런타임 조립은 `core/hw/hwshaderfragmenttohlslconverter.cpp`. 프로필은 SM4(`ps_4_0`/`vs_4_0`).
- 주석·문서·커밋 메시지는 한글. 코드 식별자는 영어.

## 7. 커밋 컨벤션

한글, `<type>: <서술형 설명>.` 예: `feat: 프레젠테이션을 DXGI 플립 모델 스왑체인으로 바꿉니다.` type: `feat`, `fix`, `modify`, `refactor`, `style`, `perf`, `build`, `docs`, `chore`, `test`, `release`. 한 커밋은 §3 게이트를 통과한 상태여야 한다(빌드가 깨진 중간 상태 커밋 금지).

## 8. 작업 절차

1. `docs/ROADMAP.md`에서 현재 Phase와 합격 기준을 확인한다.
2. 바꾸기 전에 영향 범위를 `grep`으로 확인한다. D3D9 타입·열거형 사용처는 `docs/INVENTORY.md`(Phase 1 산출물).
3. 수직 슬라이스로 간다. 작은 단위로 빌드 → 스모크 → 비교를 반복한다.
4. 막히면 추측으로 우회하지 않는다. `docs/`와 업스트림 소스를 다시 읽고, 그래도 답이 없으면 사용자에게 묻는다.
5. **학습 기록 루프(반복 실수 방지)**: 환경·빌드·라이브러리 함정은 `docs/LEARNINGS.md`, 설계 결정은 `docs/DECISIONS.md`에 **작업을 끝내기 전에** 기록한다. 새 세션은 읽은 파일만 안다. 사소한 건 적지 않는다.

## 9. 작업 원칙

1. **생각 먼저** — 코딩 전에 가정과 모호함을 드러낸다.
2. **단순함 우선** — 요구된 것만 구현한다. 투기성 기능·불필요한 추상화 금지.
3. **수술적 변경** — 필요한 코드만 고친다. 주변부 임의 개선 금지, 기존 스타일에 맞춘다.
4. **목표 주도 실행** — 작업을 검증 가능한 합격 기준(§3·ROADMAP)으로 바꿔 충족할 때까지 반복한다.

## 10. 먼저 읽을 문서

- `docs/BUILD.md` — 요구 환경, 빌드 명령, 산출물, 교체 테스트 루프.
- `docs/ROADMAP.md` — Phase 0~4와 각 합격 기준, 진행 상태.
- `docs/DECISIONS.md` — 번호 붙은 설계 결정과 근거.
- `docs/LEARNINGS.md` — 환경·빌드 함정 기록.
- `docs/INVENTORY.md` — D3D9 사용 인벤토리와 D3D11 매핑(Phase 1에서 작성).
