# 빌드와 교체 테스트

## 요구 환경

| 항목 | 요구 사항 |
|---|---|
| OS | Windows 10/11 x64 |
| Visual Studio | 2022 (17.2 이상), **C++ 데스크톱 개발** 워크로드 |
| Windows SDK | 10.0.19041 이상 (셰이더 컴파일에 SDK의 `fxc.exe`를 씀) |
| .NET SDK | 엔진 빌드는 저장소가 `.dotnet\`에 필요한 SDK를 자동 설치한다. 스모크 앱 빌드에는 전역 .NET 10 SDK가 필요하다. |
| .NET 런타임 | 스모크 테스트용 `Microsoft.WindowsDesktop.App` 8.0.x와 10.0.x |

## 엔진 빌드

```powershell
$env:CL = "/wd4819"
.\build.cmd -configuration Release -platform x64 -nodeReuse $false
```

- 소요 시간은 네이티브 프로젝트 28개 기준 수 분. 이후 증분 빌드는 더 짧다.
- Debug 빌드는 `-configuration Debug`. 어설션과 추적 로그가 켜진다.
- `-platform x86`, `-platform arm64`도 업스트림 설정은 남아 있으나 이 프로젝트의 검증 대상은 x64다.

### C4819 (비-UTF-8 코드 페이지 로캘)

시스템 로캘이 한국어(CP949) 등 비-UTF-8 코드 페이지이면 Windows SDK의 `wtypesbase.idl` 같은 파일의
비-ASCII 문자 때문에 경고 C4819가 나고, 저장소가 경고를 오류로 취급해 빌드가 실패한다.
`CL=/wd4819` 환경 변수로 모든 `cl.exe` 호출(midl이 부르는 전처리기 포함)에서 끈다.
MSBuild 노드가 환경 변수를 물려받으므로, 이전 빌드의 노드가 살아 있으면 먼저 끝내야 한다(`-nodeReuse $false`).
시스템 로캘을 UTF-8(베타)로 바꾼 경우에는 필요 없다.

## 산출물

| 파일 | 경로 |
|---|---|
| 네이티브 엔진 | `artifacts\bin\wpfgfx\x64\Release\wpfgfx_cor3.dll` (+ `.pdb`) |
| 셰이더 컴파일러 | `artifacts\bin\D3DCompiler\x64\Release\D3DCompiler_47_cor3.dll` |

로컬 빌드의 파일 버전은 Arcade 기본값 `42.42.42.42424`다. 스톡 런타임의 DLL(예: 8.0.31은 `8.0.3126.42109`)과
프로세스 모듈 목록에서 바로 구분된다. `ProductVersion`에는 빌드한 커밋 해시가 들어간다.

## 교체 테스트: tests/SmokeApp

그라데이션, ClearType 텍스트, DropShadow/Blur 효과, OpacityMask, VisualBrush, 타일 브러시, 알파 블렌딩,
Viewport3D, Clip을 한 창에 그리고, 로드된 그래픽 DLL과 `RenderCapability`를 보고한다.

```powershell
# wpfgfx_cor3.dll과 D3DCompiler_47_cor3.dll을 한 폴더에 모은다
mkdir ..\out\dll
copy ..\..\artifacts\bin\wpfgfx\x64\Release\wpfgfx_cor3.dll ..\out\dll\
copy ..\..\artifacts\bin\D3DCompiler\x64\Release\D3DCompiler_47_cor3.dll ..\out\dll\

cd tests\SmokeApp
dotnet build -c Release                                   # net8.0-windows
dotnet build -c Release -p:SmokeTfm=net10.0-windows       # net10.0-windows
.\bin\Release\net8.0-windows\win-x64\SmokeApp.exe  --wpfgfx ..\out\dll --screenshot ..\out\net8-local.png  --exit
.\bin\Release\net10.0-windows\win-x64\SmokeApp.exe --wpfgfx ..\out\dll --screenshot ..\out\net10-local.png --exit
.\bin\Release\net8.0-windows\win-x64\SmokeApp.exe  --screenshot ..\out\net8-stock.png --exit   # 비교용 스톡 DX9
```

옵션:

- `--wpfgfx <dir>`: `NativeLibrary.SetDllImportResolver`로 PresentationCore의 P/Invoke를 그 폴더의
  `wpfgfx_cor3.dll`로 돌린다. 폴더에 `D3DCompiler_47_cor3.dll`도 있어야 한다(wpfgfx가 자기 폴더에서 지연 로드).
- `--screenshot <png>`: `PrintWindow`로 창만 캡처한다(다른 창에 가려져도 무관).
- `--exit`: 보고 후 종료. `--delay <ms>`: 렌더링 대기 시간(기본 2000).

보고서는 stdout과 exe 옆 `smoke-report.txt`에 남는다. 첫 줄 예:
`Tier=2 RenderMode=Default ps2=True ps3=True maxTex=16384,16384`.

앱 출력 폴더에 DLL을 복사하는 것만으로는 교체되지 않는다. 프레임워크 종속 앱의 `NATIVE_DLL_SEARCH_DIRECTORIES`에는
공유 프레임워크 폴더만 들어 있다. 머신 전체에 적용하려면
`%ProgramFiles%\dotnet\shared\Microsoft.WindowsDesktop.App\<버전>\wpfgfx_cor3.dll`을 교체한다(먼저 백업).

## 확인된 사실 (Phase 0)

- 업스트림 소스 그대로 빌드한 DLL이 .NET 8.0.31 런타임에서 Tier 2(하드웨어)로 렌더링된다.
- 같은 DLL이 .NET 10.0.12 런타임(WPF 10.0 관리 어셈블리)에서도 Tier 2로 렌더링된다.
  관리 코드와 네이티브 엔진 사이의 계약(`wpfgfx.def` export 107개, DUCE 프로토콜 지문 `MIL_SDK_VERSION 0x200184C0`)이
  dotnet/wpf v8.0.31부터 main까지 동일하기 때문이다.
- 따라서 이 프로젝트의 산출물인 DLL 하나가 .NET 8, 9, 10 WPF 앱에 그대로 쓰일 수 있다.
