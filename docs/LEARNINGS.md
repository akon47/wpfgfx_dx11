# 학습 기록 (환경·빌드·라이브러리 함정)

항상 적용되는 것만 적는다. 치명적인 것은 `CLAUDE.md` §5로 승격한다.

## L1. 비-UTF-8 로캘에서는 C4819가 빌드를 깨뜨린다 (2026-10-28)

한국어(CP949) 로캘에서 Windows SDK의 `wtypesbase.idl`, 업스트림의 일부 소스가 C4819를 내고 저장소는 경고를 오류로 취급한다.
해결: `CL=/wd4819` 환경 변수. MSBuild 노드가 환경을 물려받으므로 바꾼 뒤에는 기존 노드를 끝낸다(`-nodeReuse 0`; PowerShell에서 `$false`는 문자열로 전달돼 바인딩 오류).

## L2. 앱 폴더에 wpfgfx_cor3.dll을 복사해도 교체되지 않는다 (2026-10-28)

프레임워크 종속 앱의 `NATIVE_DLL_SEARCH_DIRECTORIES`에는 공유 프레임워크 폴더만 들어 있고, deps.json에 없는
앱 로컬 네이티브 파일은 후보가 아니다. 교체는 `NativeLibrary.SetDllImportResolver`(스모크 앱 `--wpfgfx`) 또는
공유 프레임워크 폴더의 파일 교체로만 된다.

## L3. D3DCompiler_47_cor3.dll은 wpfgfx 옆에 있어야 한다 (2026-10-28)

wpfgfx는 `/dllrename`으로 이름을 바꾼 `D3DCompiler_47_cor3.dll`을 자기 폴더에서 지연 로드한다.
리졸버로 다른 폴더의 wpfgfx를 쓰면 그 폴더에 D3DCompiler도 같이 둬야 한다.
(D3D11 포팅 시 System32의 `d3dcompiler_47.dll`을 직접 쓰면 이 재배포 자체를 없앨 수 있다. Phase 2에서 검토.)

## L4. 창 캡처는 PrintWindow(PW_RENDERFULLCONTENT)로 한다 (2026-10-28)

`Graphics.CopyFromScreen`은 화면 영역을 복사하므로 겹친 다른 창이 같이 찍힌다. `PrintWindow`는 DWM을 통해
해당 창만 렌더링하므로 z-order와 무관하다. WPF(D3D) 창 내용도 정상 캡처된다.

## L5. 네이티브 툴 부트스트랩 경고는 무시해도 된다 (2026-10-28)

업스트림 `global.json`의 `native-tools`(strawberry-perl, .NET 4.8 참조 어셈블리)는 관리 코드 빌드용이다.
WpfGfx 빌드에는 필요 없어 이 저장소의 `global.json`에서 제거했다.

## L6. tests/ 아래 프로젝트는 Arcade에서 분리해야 한다 (2026-10-28)

저장소 루트의 `Directory.Build.props`/`global.json`은 Arcade를 끌어들인다. `tests/`에 자체 `global.json`과
빈 `Directory.Build.props`/`.targets`를 두어 스모크 앱이 전역 .NET SDK로 평범하게 빌드되게 했다.

## L7. d3d9.dll이 프로세스에 있다고 엔진이 로드한 것은 아니다 (2026-10-28)

일부 보안 소프트웨어(키보드 보안·화면 보호 모듈)는 모든 프로세스에 주입되고 그 DLL이 `d3d9.dll`을 가져온다. 그런 PC에서는
`tasklist /m d3d9.dll`에 콘솔 창까지 전부 나온다. 엔진 탓인지는 스모크 앱의 `d3d9 watch` 줄로 판단한다. "already loaded at
startup: True"면 관리 코드가 실행되기 전에 주입된 것이고, 엔진이 로드했다면 로드 시점의 관리 호출 스택이 함께 찍힌다.
보조 확인: `dumpbin /dependents wpfgfx_cor3.dll`에 d3d9.dll이 없어야 하고, 소스에 `Direct3DCreate9` 문자열이 없어야 한다.

## L8. mild3dtypes.h의 호환 매크로는 d3d9 헤더와 철자까지 같아야 한다 (2026-10-28)

D3DImage(D7)와 EVR/DXVA2(D16)는 `<d3d9.h>`를 포함해야 하고, `common/precomp.hpp`와 `av/av.h`를 통해 거의 모든 프로젝트에
들어온다. `D3DCLEAR_*`, `D3DLOCK_*`, `D3DPRESENT_DONOTWAIT`, `D3DERR_*`, `D3DPS_VERSION` 등을 `mild3dtypes.h`가 먼저 정의하므로,
토큰 열이 d3d9types.h/d3d9.h와 똑같지 않으면 C4005(재정의)가 오류가 된다(`0x00000001l`의 `l` 접미사까지). 포함은 항상
`include/d3d9interop.h`로 한다. `std.h`가 `D3DMATRIX`를 `#pragma deprecated`로 표시해 d3d9.h 안의 `SetTransform` 선언이 C4995를
내므로 그 헤더가 경고를 끄고 포함한다.

## L9. 소프트웨어 Begin3D는 D3DERR_NOTAVAILABLE/NOTFOUND만 삼킨다 (2026-10-28)

`CSwRenderTargetSurface::Begin3D`는 3D 폴백 디바이스 생성이 그 두 코드로 실패할 때만 "3D 불가"로 넘어가고, 다른 코드는 그대로
전파되어 **프레임 전체가 그려지지 않는다**(창이 흰색, 예외는 없음). 디바이스가 없는 상황을 알리는 코드는 `D3DERR_NOTAVAILABLE`로
통일한다(M1 게이트 `d3dloader.cpp`).

## L10. D3D9 가 암묵적으로 해 주던 것들 — D3D11 에서 직접 맞춰야 한 목록 (2026-10-28)

하드웨어 경로를 처음 켰을 때 디버그 레이어와 백버퍼 덤프로 찾은 차이. 모두 `docs/DESIGN.md` 설계대로 코드에 반영되어 있다.

- **Clear 는 클립을 따르지 않는다.** `CHwSurfaceRenderTarget::Clear` 는 클립(시저)만 걸고 `Clear(0, NULL)` 을 불렀다.
  D3D9 는 뷰포트·시저 안만 지웠지만 `ClearRenderTargetView` 는 전체를 지워 더티 영역 밖이 검게 사라졌다 → 클립이 있으면 `ClearView`.
- **입력 레이아웃과 VS 시그니처는 레지스터까지 맞아야 한다.** VS 마다 입력 선언이 다르면 "mismatched hardware registers" 오류.
  모든 VS 가 `MIL_HLSL_VERTEX_INPUT` 하나를 선언하고, 형식에 없는 속성은 슬롯 1 의 0 버퍼에서 읽는다(`HwFixedFunctionShaders.cpp`).
- **PS 입력은 VS 출력의 부분집합이어야 하고 순서가 같아야 한다.** 고정 기능 PS 가 UV0~7 을 요구하므로 VS 출력도 UV0~7 을 항상 낸다.
- **SM4 의 `$Globals` 는 쓰지 않는 전역도 자리를 차지한다.** 한 HLSL 에 VS/PS 상수를 같이 두면 PS 의 c0 이 VS 상수로 밀린다.
  조립기는 `#ifdef MIL_VS` / `MIL_PS` 로 절반씩 감싸고 컴파일 때 define 을 붙인다.
- **같은 리소스를 RT 와 SRV 로 동시에 묶을 수 없다.** 런타임이 SRV 를 조용히 풀어 버리므로 그림자 상태가 어긋난다.
  RT 를 바꿀 때 충돌하는 SRV 만 그림자 상태에서 지운다(효과는 텍스처를 먼저 걸고 RT 를 바꾼다. 전부 풀면 안 된다).
- **DXGI 스왑체인은 B8G8R8X8 을 받지 않는다.** X8 은 "대상 알파 없음" 표시로만 두고 실제 렌더 타깃은 전부 B8G8R8A8 로 만든다
  (`CreateRenderTargetUntracked`). 포맷이 다르면 `CopySubresourceRegion`/`ResolveSubresource` 가 실패한다.
- **`Present1` 더티 사각형은 창 서피스를 사각형 안만 갱신한다.** D3D9 는 창 서피스가 새로 만들어지면 전체를 다시 복사했지만 DXGI 는
  하지 않아 창이 검게 남는다. 전체 프레젠트로 통일(D10 의 dirty rect 항목은 철회).
- **멀티샘플 서피스의 부분 리졸브.** `ResolveSubresource` 는 서브리소스 전체만 다룬다. 스크래치에 리졸브한 뒤 사각형을 복사한다.
- **깊이 버퍼는 RT 와 크기·샘플 수가 같아야 바인딩된다.** RT 를 바꿀 때 맞지 않는 깊이 버퍼를 먼저 떼어낸다.

## L11. 한글 주석 줄은 ASCII 문자로 끝낸다 (2026-10-28)

BOM 없는 UTF-8 소스를 CP949 로캘의 MSVC 는 CP949 로 읽는다(`/wd4819` 는 경고만 끈다). 줄 끝이 한글이면 마지막 UTF-8 바이트가
뒤따르는 줄바꿈과 2바이트 문자로 묶여 **다음 줄이 주석 속으로 사라진다**. 구조체 멤버 한 줄이 통째로 없어져 "멤버가 아닙니다"
오류가 났다. 규칙: 주석은 마침표 등 ASCII 로 끝낸다. 확인은 `python tools/check_trailing_nonascii.py src/Microsoft.DotNet.Wpf/src/WpfGfx`
(`--fix` 를 주면 주석 줄 끝에 마침표를 붙인다).

## L12. 레이어드 윈도우 캡처로는 알파를 검증할 수 없다 (2026-10-28)

`PrintWindow(PW_RENDERFULLCONTENT)`는 레이어드 윈도우를 검정 위에 합성한 결과를 알파 255로 돌려준다(프리멀티플라이드 색은 맞지만 알파는 사라진다).
화면 복사는 창이 가려지지 않아야 하는데, 백그라운드 프로세스는 `SetForegroundWindow`로 앞에 나올 수 없다(`Topmost`로 올리면 사용자의 다른 창 위에
뜬다). 그래서 알파는 엔진 쪽 덤프(`gdipresent1.bmp`)로 확인하고, 스모크 앱은 가려지지 않았을 때만 화면 복사본을 남긴다.

## L13. 프레임마다 텍스처를 만들면 D3D11 드라이버가 메모리를 요동시킨다 (2026-10-28)

스톡 엔진 코드는 효과 중간 렌더타깃과 글리프 업로드용 락 가능 텍스처를 프레임마다 만들고 버린다. D3D9 에서는 문제 없던 이 패턴이
D3D11 에서는 프라이빗 메모리 200 MB 이상·GPU 로컬 메모리 100 MB 요동으로 나타났다. 찾는 방법: `WPFGFX_DX11_LOG` 를 켜면 300 프레젠트마다
`memory [...]` 줄에 생성 사이트별 누적 카운터(rt/tex/ds/msaa/readback/swapchain/poolhit)와 `IDXGIAdapter3::QueryVideoMemoryInfo` 값이 찍힌다.
`ID3D11Debug::ReportLiveDeviceObjects` 요약(같은 로그)은 **누수**(객체 수 증가)와 **요동**(수는 일정, 생성·파괴 반복)을 구분해 준다. 해결은 D26.

## L14. 다른 디바이스가 보는 keyed mutex 해제는 Flush 해야 바로 반영된다 (2026-10-28)

`IDXGIKeyedMutex::ReleaseSync` 는 명령 스트림에 기록될 뿐이라, 컨텍스트가 제출되기 전에는 다른 디바이스의 `AcquireSync` 가 대기한다.
프레임 큐에서 컴포지션이 복사 뒤 `ReleaseSync(0)` 만 하고 다음 `Present` 까지 제출하지 않자 생산자의 `AcquireSync(0, 1ms)` 가 33% 타임아웃
됐다. 복사 직후 `ID3D11DeviceContext::Flush` 를 부른다.

## L15. 효과 중간 텍스처에 여유 크기를 주면 회전하는 요소가 떨린다 (2026-10-28)

텍스처 풀(D26)에서 효과용 중간 렌더타깃에 "조금 더 큰 텍스처"를 재사용하게 했더니 회전하는 셀(그림자 효과)이 프레임마다 미세하게 흔들렸다.
효과 파이프라인은 중간 텍스처를 실제 크기 기준 UV(0..1)로 샘플링하므로 여유 크기가 프레임마다 달라지면 내용이 이동한다. 사용자가 눈으로
먼저 발견했고, 스모크 `--spin-jitter 24`(회전 셀 몸체 무게중심의 최대 편차: 스톡 0.27 px, 결함 상태 8.9 px)를 게이트 행으로 넣어 다시는
놓치지 않게 했다. 게이트의 셀 4 제외(애니메이션 위상)만으로는 이런 결함이 숨는다.

## L16. export 구조체의 HANDLE 은 관리 쪽에서 IntPtr 로 둬야 x86 이 맞는다 (2026-10-28)

프레임 큐 서술자(`WpfGfxFrameQueueDesc`)의 `HANDLE rgSharedHandles[4]` 를 관리 쪽에서 `long` 네 개로 두었더니 x64 에서는 돌아가고
x86 에서는 두 번째 핸들이 NULL 로 읽혀 `E_INVALIDARG` 가 났다(HANDLE 은 포인터 크기). 새 export 구조체는 `IntPtr` 필드로 쓰고,
게이트를 `-Rid win-x86` 으로도 돌린다. 실패한 시나리오가 창을 남기면 앱이 끝나지 않으므로 테스트는 `finally` 로 정리한다.

## L17. DXGI 플립 모델은 프레젠트 뒤 백버퍼를 떼고, ResizeBuffers 는 "지금" 참조가 없어야 한다 (2026-10-28)

- 플립 체인에서 `Present1` 이 성공하면 런타임이 백버퍼 0 을 OM 스테이지에서 떼어낸다. 엔진은 "현재 렌더타깃" 을 캐시하므로
  (`m_pCurrentRenderTargetNoRef`) 그대로 두면 다음 `SetRenderTarget` 이 생략되고 프레임이 아무 데도 안 그려진다. 프레젠트 뒤 캐시를 비운다.
- 버퍼 1 은 읽기 전용이고 방금 프레젠트한 프레임이다. 부분 갱신 엔진은 프레젠트 뒤 더티 영역을 버퍼 1→0 으로 복사하면 중간 서피스 없이
  "백버퍼 내용 보존" 의미를 얻는다(D29).
- `ResizeBuffers` 는 래퍼가 쥔 참조·뷰·파이프라인 바인딩이 전부 없어야 성공한다. 이 엔진은 놓은 `CD3DResource` 를 SList 에 모아 두고
  **다음 프레젠트에** 파괴하므로(`DestroyResources`), 리사이즈 전에 `DestroyResources(WithDelay)` 를 직접 불러야 한다. 안 그러면
  `DXGI_ERROR_INVALID_CALL` 이 나고 폴백(재생성)이 조용히 일어난다. 로그의 `ResizeBuffers failed` 로 확인한다.
- `Present1` 의 더티 사각형이 백버퍼를 벗어나면 `DXGI_ERROR_INVALID_CALL` 로 프레젠트 자체가 실패한다. 잘라서 넘긴다.
- 같은 HWND 에 플립 체인은 하나만 둔다. 디스플레이별 렌더타깃이 둘 이상이면(어댑터가 다른 모니터) 비트블트를 유지한다.

## L18. dotnet/wpf-test 는 이 환경에서 Arcade 빌드가 돌지 않아 DRT 를 직접 빌드한다 (2026-10-28)

- `release/10.0` 의 `global.json` 은 .NET SDK 12 알파를 요구한다. Arcade 가 `.dotnet` 에 내려받지만(tar.gz) 그 SDK 로 "No .NET SDKs were
  found" 가 난다. 설치된 SDK 10 으로 바꾸면 VS 2022 17.14 의 MSBuild 가 "SDK 10 은 MSBuild 18 이 필요" 라며 거부하고,
  `-msbuildEngine dotnet` 으로 바꾸면 `Microsoft.DotNet.Arcade.Wpf.Sdk` 의 `ExtendedAssemblyInfo.targets` 가 .NET MSBuild 에 없는
  `CodeTaskFactory` 를 쓴다. 또 `src\Test\mil\MIL.sln` 은 공개 저장소에 없는 프로젝트(DrtText, DrtImaging)를 참조한다.
- 해결: DRT csproj 를 `dotnet build ... -p:GenerateAssemblyInfo=false` 로 직접 빌드한다(그 타깃만 건너뛴다). 산출물은
  `publish\Release\x64\Test\DRT` 에 자동 배치된다. 절차는 `docs/BUILD.md`, 실행은 `tests/run-drts.ps1`.
- DrtAnimation 이 참조하는 `Common\Code\Microsoft\Test\Graphics\MathEx.cs` 는 Latin-1 파일이라 CP949 컴파일러가 문자열 리터럴의 `§`(0xA7)
  뒤 따옴표를 삼킨다(L11 과 같은 부류). `-p:CodePage=1252` 는 UTF-8 파일(TestRuntime)을 깨뜨리므로, 그 리터럴만 `"\u00A7"` 로 바꾸고 빌드한다.
- 교체 엔진으로 실행하는 가장 깨끗한 방법은 공유 프레임워크를 임시 폴더에 복사해 DLL 두 개만 바꾸고 `DOTNET_ROOT` 를 가리키는 것이다.
  스모크 앱의 `SetDllImportResolver` 는 남의 앱에 쓸 수 없고, 머신 공유 프레임워크를 바꾸면 다른 앱까지 영향을 받는다.

## L19. 소프트웨어 모드의 3D 는 렌더타깃을 Lock 할 수 없다: 스테이징 텍스처를 거친다 (2026-10-28)

`RenderOptions.ProcessRenderMode = SoftwareOnly` 에서 3D 콘텐츠는 `CHw3DSoftwareSurface`(hwsw3dfallback.cpp)가 WARP 디바이스의
렌더타깃에 그린 뒤 소프트웨어 타깃에 합성한다. D3D9 는 "lockable 렌더타깃" 을 직접 Lock 했지만 D3D11 은 `USAGE_DEFAULT` 텍스처를
Map 할 수 없어 `CD3DSurface::LockRect` 가 `WGXERR_INVALIDCALL` 을 내고, 관리 쪽에는 `UCEERR_RENDERTHREADFAILURE` 로 올라가
앱이 죽는다(dotnet/wpf-test DrtBasic3D 로 발견). 렌더타깃과 같은 크기의 스테이징 텍스처를 짝으로 두고, 3D 전에는 스테이징에 쓴
뒤 `CopySubresourceRegion` 으로 올리고, 3D 뒤에는 `GetRenderTargetData` 로 내려받아 읽는다. 게이트의 `software mode` 행이 이 경로를 지킨다.
