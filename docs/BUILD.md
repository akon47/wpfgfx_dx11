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
.\build.cmd -configuration Release -platform x64 -nodeReuse 0
```

- 소요 시간은 네이티브 프로젝트 28개 기준 수 분. 이후 증분 빌드는 더 짧다.
- Debug 빌드는 `-configuration Debug`. 어설션과 추적 로그가 켜진다.
- `-platform x86`, `-platform arm64`도 업스트림 설정은 남아 있으나 이 프로젝트의 검증 대상은 x64다.

### C4819 (비-UTF-8 코드 페이지 로캘)

시스템 로캘이 한국어(CP949) 등 비-UTF-8 코드 페이지이면 Windows SDK의 `wtypesbase.idl` 같은 파일의
비-ASCII 문자 때문에 경고 C4819가 나고, 저장소가 경고를 오류로 취급해 빌드가 실패한다.
`CL=/wd4819` 환경 변수로 모든 `cl.exe` 호출(midl이 부르는 전처리기 포함)에서 끈다.
MSBuild 노드가 환경 변수를 물려받으므로, 이전 빌드의 노드가 살아 있으면 먼저 끝내야 한다(`-nodeReuse 0`. PowerShell에서 `$false`를 쓰면 문자열 "False"로 전달되어 `Build.ps1`의 `[bool]` 매개변수 바인딩이 실패한다. cmd.exe에서는 `$false`도 된다).
시스템 로캘을 UTF-8(베타)로 바꾼 경우에는 필요 없다.

같은 이유로 BOM 없는 UTF-8 소스의 한글 주석 줄은 ASCII 문자로 끝나야 한다. 줄 끝 한글의 마지막 바이트가 줄바꿈과 묶여 다음 줄이
주석에 먹힌다(`docs/LEARNINGS.md` L11). `python tools/check_trailing_nonascii.py src/Microsoft.DotNet.Wpf/src/WpfGfx` 로 검사하고
`--fix` 로 고친다.

x86·arm64 도 같은 명령에 `-platform x86` / `-platform arm64` 로 빌드된다(산출물은 `artifacts\bin\wpfgfx\<platform>\Release`).

## 산출물

| 파일 | 경로 |
|---|---|
| 네이티브 엔진 | `artifacts\bin\wpfgfx\x64\Release\wpfgfx_cor3.dll` (+ `.pdb`) |
| 셰이더 컴파일러 | `artifacts\bin\D3DCompiler\x64\Release\D3DCompiler_47_cor3.dll` |

로컬 빌드의 파일 버전은 Arcade 기본값 `42.42.42.42424`다. 스톡 런타임의 DLL(예: 8.0.31은 `8.0.3126.42109`)과
프로세스 모듈 목록에서 바로 구분된다. `ProductVersion`에는 빌드한 커밋 해시가 들어간다.

## 교체 테스트: tests/SmokeApp

그라데이션, ClearType 텍스트, DropShadow/Blur 효과, OpacityMask, VisualBrush, 타일 브러시, 알파 블렌딩,
Viewport3D, Clip, 사용자 ShaderEffect(ps_2_0/ps_3_0), D3DImage(D3D9Ex 서피스, D3D11 텍스처)를 한 창에 그리고,
로드된 그래픽 DLL과 `RenderCapability`를 보고한다. 16번 셀(D3D11 텍스처)은 `src/WpfGfx.Dx11`을 참조하며 스톡 엔진에서는
"D3D11 engine not loaded"를 표시한다.

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
- `--left <x> --top <y>`: 창 위치. 두 모니터에 걸치게 놓으면 디스플레이별 스왑체인 경로를 탄다.
- `--transparent <png>`: `AllowsTransparency` 창을 주 창 위에 띄우고 `PrintWindow`로 캡처한다(레이어드 윈도우 프레젠트 경로).
  다른 창에 가려지지 않았으면 화면 복사본(`*-screen.png`)도 남긴다.
- `--media <file> --media-shot <png>`: `MediaElement` 창을 띄워 재생하고 캡처한다(`core/av` EVR 프레젠터 경로, D25).
- `--rtb <png> [--rtb-hw]`: 그라데이션·텍스트·ps_2_0·ps_3_0 효과·그림자를 `RenderTargetBitmap` 으로 그려 저장한다. `--rtb-hw` 는
  `Dx11Engine.HardwareRenderTargetBitmap = true`(GPU 경로, D27; 스톡 엔진에서는 예외).
- `--resize <steps>`: 창 크기를 단계별로 키우고 줄이며 매 단계 캡처해 검은 픽셀 비율을 보고한다(D29 플립 모델의 제자리 리사이즈 검증).
- `--software`: `RenderOptions.ProcessRenderMode = SoftwareOnly` 로 그린다(소프트웨어 래스터라이저 + WARP 3D 폴백, L19).
- `--bug-nan`, `--bug-aliased [점 파일]`, `--bug-bitmapcache [개수]`: 엔진 원인 업스트림 버그 재현(D34). 결과 줄 `bug: <이름> survived=...`.
- `--bug-maximize [횟수]`: WindowChrome 대화상자(980×500)를 최대화/복원 반복(D35). 최대화 창은 모니터보다 크다. UI 스레드가 8초 멈추면
  감시 스레드가 `survived=false hung UI thread` 를 찍고 끝낸다. 환경 변수 `WPFGFX_DX11_FLIP_BUFFERS=2|3`, `WPFGFX_DX11_TEST_SKIP_FULL_PRESENT=1` 과 함께 쓴다.
- Present·ResizeBuffers 가 실패하면 로그에 `DXGI[...]` 줄(DXGI 정보 큐의 거부 사유)과 더티 사각형이 남는다(디버그 레이어가 켜져 있을 때).
- GPU 안티에일리어싱(D32)은 환경 변수 `WPFGFX_DX11_MSAA=4`(또는 8)로 켠다. 게이트는 4x 로 표준 장면·스핀 떨림을 확인한다.
- `--fq <png> [--fq-queues N] [--fq-seconds S] [--fq-block]`: 프레임 큐(D19) 시나리오. 워커 스레드가 N개 큐에 60 fps 로 그리고
  `--fq-block` 이면 UI 스레드를 0.5초마다 100 ms 막는다. 결과 줄 `fq: ...` 에 카운터와 250 ms 간격 두 캡처의 변화율이 나온다.
  `tests/assets/smptebars.mp4`는 정지 SMPTE 막대(H.264 160×120, 4초)라 캡처 시점에 무관하다.
  재생성: `ffmpeg -f lavfi -i smptebars=size=160x120:rate=10 -t 4 -pix_fmt yuv420p -c:v libx264 -profile:v baseline -g 10 smptebars.mp4`.

보고서는 stdout과 exe 옆 `smoke-report.txt`에 남는다. 첫 줄 예:
`Tier=2 RenderMode=Default ps2=True ps3=True maxTex=16384,16384`.

앱 출력 폴더에 DLL을 복사하는 것만으로는 교체되지 않는다. 프레임워크 종속 앱의 `NATIVE_DLL_SEARCH_DIRECTORIES`에는
공유 프레임워크 폴더만 들어 있다. 머신 전체에 적용하려면
`%ProgramFiles%\dotnet\shared\Microsoft.WindowsDesktop.App\<버전>\wpfgfx_cor3.dll`을 교체한다(먼저 백업).

## 동반 어셈블리: src/WpfGfx.Dx11

D3D11 엔진에만 있는 기능(엔진 `ID3D11Device` 얻기, `ID3D11Texture2D`를 받는 `D3D11Image`)을 앱에 노출하는 관리 어셈블리(D8).
일반 SDK 프로젝트이며 Arcade와 무관하다. 스모크 앱이 `ProjectReference`로 쓴다.

```powershell
dotnet build src\WpfGfx.Dx11 -c Release      # 산출물: src\WpfGfx.Dx11\bin\Release\net8.0-windows\WpfGfx.Dx11.dll
```

`Dx11Engine.IsAvailable`은 프로세스에 이미 로드된 `wpfgfx_cor3.dll`에서 `WpfGfxDx11_GetInfo` export를 찾는다. 창이 하나 뜬 뒤에
호출해야 하고, 스톡 DX9 엔진이면 false다.

## 자동 게이트: tests/run-smoke.ps1

위 루프를 한 번에 돌린다. 스톡 DX9와 지정한 DLL로 .NET 8·10 각각 표준 장면, 투명 창, MediaElement, 모의 디바이스 손실(D24)을
실행해 클라이언트 영역 캡처를 셀 단위로 비교하고 표로 보고한다(`tests/out/report.md`). 하나라도 FAIL 이면 종료 코드 1.

```powershell
.\tests\run-smoke.ps1                                   # artifacts 빌드 결과, net8.0-windows + net10.0-windows
.\tests\run-smoke.ps1 -Dll <폴더> -Tfm net8.0-windows   # 다른 DLL, 한 런타임만
.\tests\run-smoke.ps1 -Bench -BenchSeconds 8            # 처리량 벤치마크 행 추가(아래)
.\tests\run-smoke.ps1 -StraddleLeft 2110                # 창을 두 모니터에 걸치는 시나리오 추가(모니터 배치에 맞춰 x 지정)
.\tests\run-smoke.ps1 -Rid win-x86 -Tfm net8.0-windows   # x86: 먼저 .\build.cmd -configuration Release -platform x86
```

행: engine, standard, spin jitter, resize(16 steps), software mode, bug #7421·#8031, maximize/restore(스톡, 플립 2·3버퍼 × MSAA, 재시도 경로), gpu antialiasing(MSAA 4x), transparent window, media, device loss recovery, RenderTargetBitmap(software / GPU opt-in),
frame queues(8, UI blocked), frame queues on stock(폴백), 선택 straddling, 선택 bench(일반 창·투명 창, 스톡·D3D11).

의도된 차이는 제외한다: 4번 셀(회전 애니메이션 위상), 16번 셀(스톡에서는 안내 문구). 캡처는 `PrintWindow(PW_CLIENTONLY)` 로
클라이언트 영역만 뜬다(제목 표시줄은 포커스에 따라 색이 달라진다).

`--bench <초>`(스모크 앱 옵션)는 사각형 1200개·텍스트 300개·방사형 타원 120개·블러 패널 60개를 매 프레임 움직이는 장면을 그려
초당 프레임, 프레임당 프로세스 CPU 시간, 프라이빗 메모리를 보고한다. WPF 는 컴포지터의 프레젠트 알림으로 다음 UI 프레임을
잡으므로 두 스레드 중 느린 쪽의 속도가 나온다.
`--bench-transparent` 는 벤치 창을 레이어드(`AllowsTransparency`) 창으로, `--bench-partial` 은 움직이는 요소를 24개(왼쪽 위 200×200)로
줄여 부분 갱신 상황으로 돌린다(D30·D31). 게이트 `-Bench` 는 네 조합(일반·투명·투명+부분)을 스톡과 D3D11 로 각각 기록한다.

## 확인된 사실 (Phase 0)

- 업스트림 소스 그대로 빌드한 DLL이 .NET 8.0.31 런타임에서 Tier 2(하드웨어)로 렌더링된다.
- 같은 DLL이 .NET 10.0.12 런타임(WPF 10.0 관리 어셈블리)에서도 Tier 2로 렌더링된다.
  관리 코드와 네이티브 엔진 사이의 계약(`wpfgfx.def` export 107개, DUCE 프로토콜 지문 `MIL_SDK_VERSION 0x200184C0`)이
  dotnet/wpf v8.0.31부터 main까지 동일하기 때문이다.
- 따라서 이 프로젝트의 산출물인 DLL 하나가 .NET 8, 9, 10 WPF 앱에 그대로 쓰일 수 있다.

## 진단 로그 (`WPFGFX_DX11_LOG`)

릴리스 빌드에서도 환경 변수 `WPFGFX_DX11_LOG=<파일 경로>` 를 주고 앱을 실행하면 엔진이 그 파일에 다음을 덧붙인다
(`include/mildx11log.h`, `core/common/mildx11log.cpp`). 변수가 없으면 아무 비용도 없다.

- 디바이스 생성(Feature Level), 스왑체인 생성/재생성, `Present1` 결과와 사각형, 렌더 스레드 실패 HRESULT, `HandleDIE` 로 들어온 실패.
- D3D11 디버그 레이어가 설치된 PC(`D3D11_3SDKLayers.dll`)에서는 디바이스를 디버그 레이어로 만들고, 검증 메시지를
  실패 시점과 매 `Present` 마다 로그로 쏟는다. 시그니처 불일치·잘못된 바인딩 같은 원인이 그대로 나온다.
- 파이프라인 조립기가 만든 HLSL 전문, 컴파일된 셰이더의 리소스 바인딩(텍스처/샘플러 슬롯)과 `$Globals` 변수 오프셋(리플렉션).
- 처음 300번의 `SetTexture` 호출(스테이지, 크기, 포맷), 처음 4번의 `Present` 직후 백버퍼를 로그 폴더에 `present1..4.bmp` 로 저장.
- GDI 프레젠트(투명 창)의 처음 2번은 `UpdateLayeredWindow`에 넘기는 백버퍼를 `gdipresent1..2.bmp`(32비트, 알파 포함)로 저장.
- `WPFGFX_DX11_TEST_DEVICE_LOSS=<n>`(로그와 별개): n번째 `Present`를 디바이스 손실로 처리해 복구 경로를 시험한다(D24).
- 로그의 `timing <이름>: avg=.. max=.. over 300` 줄: 레이어드 창 리드백(`gdi readback`), `UpdateLayeredWindowIndirect`(`ulw`),
  컴포지션의 프레젠트 대기(`wait to present`), 수직 동기 대기(`vblank wait`)를 300회 단위로 평균·최대 ms 로 찍는다(D31).
- 로그의 `per present (avg of 300): ...` 줄: 프레젠트당 드로 수, 상수 버퍼 업로드 수·KB, 임시 버퍼 드로, 렌더타깃 바인딩, 텍스처 Map,
  렌더·프레젠트·상태 적용 시간(ms), 새로 만든 텍스처 종류(렌더타깃/스테이징/초기 데이터/기타). 처음 60개의 새 렌더타깃 크기도 찍는다.
- `WPFGFX_DX11_DEBUG_LAYER=0`: 로그는 켜 두고 D3D 디버그 레이어만 끈다. 시간 측정은 이 상태로 한다(디버그 레이어가 CPU 를 크게 늘린다).

```powershell
$env:WPFGFX_DX11_LOG = "$PWD\tests\out\dx11.log"
.\tests\SmokeApp\bin\Release\net8.0-windows\win-x64\SmokeApp.exe --wpfgfx tests\out\dll --screenshot tests\out\net8.png --exit
```

## dotnet/wpf-test DRT 실행 (`tests/run-drts.ps1`)

업스트림 테스트 저장소의 milcore DRT(2D 브러시·드로잉, 3D, VisualTarget, Freezable, 애니메이션·타이밍·스토리보드, 미디어 API)를
스톡 엔진과 교체 엔진으로 각각 실행해 종료 코드를 비교한다. DRT 는 프레임워크 종속 앱이라 §5 의 (b) 방식이 필요한데, 머신의 공유
프레임워크 대신 **임시 폴더에 복사한 런타임**의 `wpfgfx_cor3.dll`·`D3DCompiler_47_cor3.dll` 만 바꾸고 `DOTNET_ROOT` 로 그 복사본을
가리킨다. 교체 엔진이 실제로 로드됐는지는 `WPFGFX_DX11_LOG` 로그가 생겼는지로 판정한다.

```powershell
# 1) 테스트 저장소(이 저장소와 같은 .NET 10 브랜치)를 옆에 받는다
git clone --depth 1 --branch release/10.0 https://github.com/dotnet/wpf-test.git ..\wpf-test

# 2) 이 환경(VS 2022 17.14 + .NET SDK 10)에서는 저장소의 Arcade 빌드가 그대로 돌지 않는다(L18). DRT 프로젝트를 직접 빌드한다.
#    global.json 의 tools.dotnet 을 설치된 SDK(예: 10.0.401)로 바꾸고 native-tools 항목을 지운 뒤:
cd ..\wpf-test
# 한국어 등 CP949 환경: src\Test\Common\Code\Microsoft\Test\Graphics\MathEx.cs 의 "§" 리터럴을 "\u00A7" 로 바꾼다(L18)
Get-ChildItem -Recurse -Filter *.csproj src\Test\mil\DRT | ForEach-Object {
    dotnet build $_.FullName -c Release -p:Platform=x64 -p:GenerateAssemblyInfo=false -nologo -v m
}
# 산출물: ..\wpf-test\publish\Release\x64\Test\DRT\Drt*.exe (+ DrtFiles\)

# 3) 실행
cd <이 저장소>
.\tests\run-drts.ps1 -TestDir ..\wpf-test\publish\Release\x64\Test\DRT            # 엔진 DLL 기본: tests\out\dll
.\tests\run-drts.ps1 -TestDir <DRT 폴더> -Dll <DLL 폴더> -Drts DrtMil2D,DrtBasic3D   # 일부만
```

결과는 마크다운 표(DRT, 스톡 종료 코드, D3D11 종료 코드, 엔진 로드 여부, 소요 시간)로 나오고 로그는 `tests\out\drt\` 에 남는다.
합격: 스톡에서 0 으로 끝나는 DRT 가 교체 엔진에서도 0 으로 끝나고 엔진 로그가 있다. 스톡에서도 실패하는 DRT 는 같은 코드면 통과로 본다.
DRT 는 대부분 API·동작 검증이고 픽셀 비교는 하지 않는다(렌더링 경로가 예외·행 없이 끝까지 도는지 확인하는 용도).
