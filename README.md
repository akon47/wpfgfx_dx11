# wpfgfx_dx11

WPF의 네이티브 렌더링 엔진 **WpfGfx(milcore, `wpfgfx_cor3.dll`)** 를 Direct3D 9에서 **Direct3D 11**로 옮기는 프로젝트입니다.
관리 코드(PresentationCore, PresentationFramework 등)는 바꾸지 않습니다. 결과물은 DLL 하나이고,
.NET 8/9/10 WPF 앱의 `wpfgfx_cor3.dll`을 그대로 교체해서 씁니다.

*Ports WPF's native rendering engine (WpfGfx / milcore) from Direct3D 9 to Direct3D 11 behind an unchanged managed/native contract, producing a drop-in `wpfgfx_cor3.dll` for .NET 8/9/10 WPF apps.*

## 왜

WPF는 아직 Direct3D 9로 그립니다. 최근 GPU(예: Intel Arc)는 네이티브 D3D9 드라이버 없이 D3D9On12 변환 계층으로 돌아가고,
dotnet/wpf에는 D3D11/12 전환 계획이 없습니다([dotnet/wpf#7062](https://github.com/dotnet/wpf/issues/7062)).
WpfGfx 소스는 공개되어 있고 관리 코드와의 경계(export 107개 + DUCE 명령 프로토콜)는 .NET 8부터 main까지 바뀌지 않았으므로,
엔진 내부만 바꾸면 모든 버전의 WPF 앱이 혜택을 봅니다.

## 저장소 구성

| 경로 | 내용 |
|---|---|
| `src/Microsoft.DotNet.Wpf/src/WpfGfx/` | 네이티브 렌더링 엔진. dotnet/wpf `release/10.0`(커밋 `01dce087b`)에서 가져왔고 **경로를 그대로 유지**합니다. |
| `src/Microsoft.DotNet.Wpf/src/Shared/`, `redist/D3DCompiler/` | WpfGfx 빌드에 필요한 공용 헤더·소스와 D3DCompiler 재배포 프로젝트. |
| `eng/`, `build.cmd`, `global.json` | 업스트림 Arcade 빌드 인프라. `wpfgfx_cor3.dll`과 `D3DCompiler_47_cor3.dll`만 빌드하도록 줄였습니다. |
| `tests/SmokeApp/` | 빌드한 DLL을 .NET 8/10 앱에 꽂아 렌더링을 확인하는 스모크 앱. |
| `docs/` | 빌드 절차, 로드맵, 설계 결정, 학습 기록. |
| `CLAUDE.md` | 이 저장소에서 작업할 때 지켜야 할 규칙. |

## 빌드와 테스트

요구 환경과 자세한 절차는 [`docs/BUILD.md`](docs/BUILD.md)를 보세요. 요약:

```powershell
$env:CL = "/wd4819"      # 비-UTF-8 로캘(한국어 등)에서 필수
.\build.cmd -configuration Release -platform x64 -nodeReuse $false

cd tests\SmokeApp
dotnet build -c Release
.\bin\Release\net8.0-windows\win-x64\SmokeApp.exe --wpfgfx ..\..\artifacts\bin\wpfgfx\x64\Release --screenshot ..\out\net8.png --exit
```

## 진행 상태

[`docs/ROADMAP.md`](docs/ROADMAP.md)에 Phase별 상태가 있습니다.

## 라이선스

dotnet/wpf와 같은 [MIT](LICENSE.TXT)입니다. 서드파티 고지는 [`THIRD-PARTY-NOTICES.TXT`](THIRD-PARTY-NOTICES.TXT)에 있습니다.
