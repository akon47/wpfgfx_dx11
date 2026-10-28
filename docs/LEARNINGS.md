# 학습 기록 (환경·빌드·라이브러리 함정)

항상 적용되는 것만 적는다. 치명적인 것은 `CLAUDE.md` §5로 승격한다.

## L1. 비-UTF-8 로캘에서는 C4819가 빌드를 깨뜨린다 (2026-10-28)

한국어(CP949) 로캘에서 Windows SDK의 `wtypesbase.idl`, 업스트림의 일부 소스가 C4819를 내고 저장소는 경고를 오류로 취급한다.
해결: `CL=/wd4819` 환경 변수. MSBuild 노드가 환경을 물려받으므로 바꾼 뒤에는 기존 노드를 끝낸다(`-nodeReuse $false`).

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
