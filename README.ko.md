# wpfgfx_dx11

**WPF 를 위한 드롭인 Direct3D 11 렌더링 엔진.** WPF 의 네이티브 렌더링 엔진 WpfGfx(milcore, `wpfgfx_cor3.dll`)의 내부를
Direct3D 9 에서 Direct3D 11 로 옮긴 프로젝트입니다. WPF 의 관리 코드(PresentationCore, PresentationFramework, WindowsBase)는
손대지 않습니다. 산출물은 네이티브 DLL 하나이고, .NET 8·9·10 WPF 앱의 스톡 `wpfgfx_cor3.dll` 자리에 그대로 들어갑니다.

[English README](README.md)

## 왜

WPF 는 아직 Direct3D 9 로 그립니다. 최근 GPU(예: Intel Arc)는 네이티브 D3D9 드라이버 없이 D3D9On12 변환 계층으로 돌아가고,
dotnet/wpf 에는 D3D11/12 전환 계획이 없습니다([dotnet/wpf#7062](https://github.com/dotnet/wpf/issues/7062)).
엔진 소스는 공개되어 있고 관리 코드와 엔진의 경계(export 107개 + DUCE 명령 프로토콜)는 .NET 8 부터 main 까지 바뀌지 않았으므로,
엔진 내부만 바꾸면 모든 버전의 WPF 앱이 DLL 하나로 혜택을 봅니다.

## 상태

[로드맵](docs/ROADMAP.md)의 Phase 0~3 이 끝났고 Phase 4 대부분이 병합됐습니다. 자동 게이트(`tests/run-smoke.ps1`)로 .NET 8 과
.NET 10(x64, 그리고 x86)에서 스톡 엔진과 비교해 검증합니다.

- 2D 렌더링(브러시, 지오메트리, ClearType·그레이스케일 텍스트, 효과, 불투명도, 클리핑, 3D 뷰포트)이 앤티에일리어싱 노이즈를 빼면
  스톡과 픽셀 단위로 같습니다.
- ps_2_0 / ps_3_0 바이트코드의 `ShaderEffect`, `IDirect3DSurface9` 를 쓰는 `D3DImage`, `MediaElement`, 레이어드(`AllowsTransparency`) 창,
  다중 모니터 창, `RenderTargetBitmap`, 디바이스 손실 복구.
- 어댑터당 D3D11 디바이스 하나, 창당 플립 모델 스왑체인 하나, 텍스처 풀, 그리고 자기 `ID3D11Device` 로 그리는 앱을 위한 프레임 큐(아래).
  무거운 2D 벤치가 스톡보다 30~50% 빠르고, 선택 기능인 GPU 안티에일리어싱을 켜면 약 2.3배 빠릅니다.
- 엔진이 원인인 dotnet/wpf 버그 수정: NaN 이 든 클립 기하가 더는 렌더 스레드를 죽이지 않고([#7421](https://github.com/dotnet/wpf/issues/7421)),
  수천 개의 `BitmapCache` 요소가 프로세스 GDI 객체 한도를 넘기지 않습니다([#8031](https://github.com/dotnet/wpf/issues/8031)).
- x64·x86·arm64 빌드(arm64 는 빌드만 확인, 실행 미검증).

스톡과 아직 다른 점: `MediaElement` 는 표준 EVR 믹서가 D3D9 만 받으므로 Direct3D 9Ex 디바이스에서 비디오를 믹싱합니다(미디어 재생
중에만 `d3d9.dll` 이 로드됩니다). 선택 기능인 GPU `RenderTargetBitmap` 경로는 소프트웨어 래스터라이저와 픽셀이 조금 다릅니다.
실제 GPU 리셋(TDR)은 엔진의 시뮬레이션 훅으로만 시험했습니다.

## DLL 구하기

소스에서 빌드합니다(Visual Studio 2022 C++ 워크로드 + Windows SDK, .NET SDK 8 이상):

```powershell
$env:CL = "/wd4819"      # 비-UTF-8 코드 페이지(한국어·일본어 등)에서 필수, 다른 환경에서는 무해
.\build.cmd -configuration Release -platform x64 -nodeReuse 0
```

빌드는 **항상 같은 폴더에 두어야 하는** 파일 두 개를 만듭니다.

| 파일 | 역할 |
|---|---|
| `artifacts\bin\wpfgfx\x64\Release\wpfgfx_cor3.dll` | 엔진(파일 버전 `42.42.42.42424` 는 로컬 빌드 표식) |
| `artifacts\bin\D3DCompiler\x64\Release\D3DCompiler_47_cor3.dll` | 엔진이 자기 폴더에서 로드하는 셰이더 컴파일러 |

다른 아키텍처는 `-platform x86`, `-platform arm64`. DLL 은 로드하는 프로세스와 비트 수가 같아야 합니다. 요구 환경·상세·문제 해결은
[docs/BUILD.md](docs/BUILD.md).

## 앱에서 DLL 쓰기

프레임워크 종속 WPF 앱은 `wpfgfx_cor3.dll` 을 공유 프레임워크 폴더
(`C:\Program Files\dotnet\shared\Microsoft.WindowsDesktop.App\<버전>`)에서 로드합니다. 그 폴더만 엔진의 네이티브 탐색 경로에 있으므로
**실행 파일 옆에 DLL 을 복사해도 아무 일도 일어나지 않습니다.** 방법은 세 가지이며 하나를 고릅니다.

### 1. 앱 안의 `DllImportResolver` (권장)

WPF 가 엔진을 로드하기 전에, 예를 들어 `App` 클래스의 정적 생성자에서 WPF 어셈블리 셋에 리졸버를 등록합니다. 리졸버는
`wpfgfx_cor3.dll` 만 내 복사본으로 돌리고 나머지는 평소대로 둡니다.

```csharp
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows;

public partial class App : Application
{
    static App()
    {
        string dir = Path.Combine(AppContext.BaseDirectory, "wpfgfx");   // wpfgfx_cor3.dll + D3DCompiler_47_cor3.dll 을 둔 폴더

        DllImportResolver resolver = (name, _, _) =>
        {
            if (!name.StartsWith("wpfgfx", StringComparison.OrdinalIgnoreCase)) return IntPtr.Zero;
            string candidate = Path.Combine(dir, name.EndsWith(".dll", StringComparison.OrdinalIgnoreCase) ? name : name + ".dll");
            return File.Exists(candidate) ? NativeLibrary.Load(candidate) : IntPtr.Zero;   // IntPtr.Zero = 스톡 엔진으로 폴백
        };

        // DllImport("wpfgfx_cor3.dll") 을 선언하는 WPF 어셈블리 전부.
        NativeLibrary.SetDllImportResolver(typeof(System.Windows.Media.Visual).Assembly, resolver);       // PresentationCore
        NativeLibrary.SetDllImportResolver(typeof(System.Windows.Window).Assembly, resolver);             // PresentationFramework
        NativeLibrary.SetDllImportResolver(typeof(System.Windows.Threading.Dispatcher).Assembly, resolver); // WindowsBase
    }
}
```

DLL 두 개를 앱의 하위 폴더(위에서는 `wpfgfx`)에 담아 배포하면 어느 머신에서나 D3D11 엔진으로 돌고, 파일이 없으면 스톡 엔진으로
폴백합니다. `SetDllImportResolver` 는 어셈블리당 한 번만 등록할 수 있으니 앱이 이미 다른 이유로 쓰고 있다면 두 리졸버를 합칩니다.
`tests/SmokeApp` 의 스모크 앱(`App.xaml.cs`, 옵션 `--wpfgfx <폴더>`)이 동작하는 예제입니다.

### 2. 런타임의 사설 복사본 (앱을 고칠 수 없을 때)

공유 프레임워크를 내 폴더에 복사해 그 안의 파일 두 개만 바꾸고 `DOTNET_ROOT` 로 그 복사본을 가리킵니다. 머신의 다른 것은 바뀌지 않습니다.

```powershell
$src = "C:\Program Files\dotnet"; $dst = "C:\wpf-dx11-runtime"; $ver = "10.0.12"   # 앱이 실제로 고르는 런타임 버전
foreach ($p in "shared\Microsoft.NETCore.App\$ver", "shared\Microsoft.WindowsDesktop.App\$ver", "host\fxr\$ver") {
    robocopy "$src\$p" "$dst\$p" /E | Out-Null
}
Copy-Item "$src\dotnet.exe" $dst
Copy-Item .\wpfgfx_cor3.dll, .\D3DCompiler_47_cor3.dll "$dst\shared\Microsoft.WindowsDesktop.App\$ver\" -Force

$env:DOTNET_ROOT = $dst
.\YourApp.exe
```

`tests/run-drts.ps1` 이 이 방법으로 업스트림 WPF 테스트 스위트를 엔진에 대해 실행합니다. 자체 포함(self-contained) 앱은 더 간단합니다.
앱 폴더 안에 `wpfgfx_cor3.dll` 이 있으니 거기서 두 파일을 바꾸면 됩니다.

### 3. 공유 프레임워크의 파일을 바꾸기

`C:\Program Files\dotnet\shared\Microsoft.WindowsDesktop.App\<버전>` 안의 `wpfgfx_cor3.dll` 과 `D3DCompiler_47_cor3.dll` 을 바꾸면
그 런타임을 쓰는 머신의 모든 WPF 앱이 바뀝니다. 원본을 보관해 두세요. 런타임 업데이트가 파일을 덮어씁니다.

### .NET Framework 4.8.x

같은 DLL 이 .NET Framework 4.8.x WPF 에서도 돕니다. 4.8.1 은 프로토콜 지문이 같고, 쓰는 export 도 모두 이 DLL 에 있습니다.
엔진을 `wpfgfx_v0400.dll` 이름으로 올리면 테스트 장면이 x64·x86 모두 스톡 엔진과 같게 나옵니다(`tests/run-netfx.ps1`,
[docs/DECISIONS.md](docs/DECISIONS.md) D37).

다만 앱별로 깔끔하게 바꾸는 방법은 없습니다. .NET Framework 는 `%WINDIR%\Microsoft.NET\Framework64\v4.0.30319\WPF\wpfgfx_v0400.dll`
(32비트는 `Framework`)을 전체 경로로 올립니다. 그래서 앱 옆에 두거나 미리 로드해도 소용이 없습니다. 테스트 호스트 `tests/NetFxHost` 는
자기 프로세스 안에서만 그 로드를 돌립니다. 시스템 파일을 바꾸면 머신 전체가 바뀝니다. 그 파일은 TrustedInstaller 소유이고
Windows Update 가 되돌릴 수 있습니다.

### 어느 엔진이 도는지 확인

- `RenderCapability.Tier >> 16` 은 두 엔진 모두 `2` 라서 구분이 안 됩니다.
- 로드된 모듈의 경로와 파일 버전으로 구분합니다. `Process.GetCurrentProcess().Modules` 에서 `wpfgfx_cor3.dll` 을 찾으세요
  (로컬 빌드는 파일 버전 `42.42.42.42424`).
- 아래 도우미 어셈블리의 `Dx11Engine.IsAvailable` 은 D3D11 엔진에서만 `true` 입니다.
- 앱 실행 전에 `WPFGFX_DX11_LOG=<파일>` 을 설정하면 D3D11 엔진은 거기에 로그를 쓰고 스톡 엔진은 변수를 무시합니다.

## D3D11 엔진의 추가 기능

WPF 가 쓰는 export 107개 외에 엔진은 작은 추가 API(ABI 버전 2, `src/Microsoft.DotNet.Wpf/src/WpfGfx/core/api/api_dx11.cpp`)를
export 합니다. 도우미 어셈블리 둘이 이를 감싸며, 둘 다 스톡 엔진을 감지하면 실패하는 대신 `IsAvailable == false` 를 보고하므로
앱이 폴백 경로를 유지할 수 있습니다.

| 어셈블리 | WPF 의존 | 내용 |
|---|---|---|
| `src/WpfGfx.Dx11` | 있음 | `Dx11Engine`, `D3D11Image` |
| `src/WpfGfx.Dx11.Interop` | 없음 | `Dx11FrameQueue` (WPF 를 참조하면 안 되는 렌더 라이브러리에서 사용 가능) |

### `Dx11Engine`

```csharp
using WpfGfx.Dx11;

if (Dx11Engine.IsAvailable)
{
    Dx11EngineInfo info = Dx11Engine.GetInfo();        // AbiVersion, FeatureLevel, DisplayCount, AdapterLuid
    IntPtr device = Dx11Engine.GetDevice(0);           // 엔진의 ID3D11Device(AddRef 됨): 리소스는 여기서 만들고,
    Marshal.Release(device);                           // 즉시 컨텍스트는 렌더 스레드 것이므로 건드리지 않는다
    Dx11Engine.HardwareRenderTargetBitmap = true;      // 옵트인: RenderTargetBitmap 을 GPU 로 그린다(ps_3_0 효과 포함)
}
```

GPU `RenderTargetBitmap` 경로는 환경 변수 `WPFGFX_DX11_HW_RTB=1` 로도 켤 수 있습니다. 스톡 엔진이 항상 쓰는 소프트웨어
래스터라이저와 픽셀(앤티에일리어싱, 텍스트)이 조금 달라 기본은 꺼져 있습니다.

### GPU 안티에일리어싱 (옵트인)

WPF 는 도형 가장자리의 안티에일리어싱을 CPU 에서 합니다. 매 프레임 렌더 스레드가 채우거나 그은 도형을 커버리지 사다리꼴로 쪼개는데,
도형이 많은 애니메이션에서는 이것이 렌더 스레드 시간의 절반쯤 됩니다. GPU 안티에일리어싱을 켜면 창이 4x 또는 8x 다중 표본 대상에
그려지고, 도형은 커버리지 없이 분할되며 가장자리는 GPU 가 만듭니다. 스모크 벤치에서 약 23 → 38 fps 가 됩니다.

```powershell
$env:WPFGFX_DX11_MSAA = "4"     # 또는 8. 1 = GPU 가 지원하는 최대. 앱 시작 전에 설정
```

```csharp
Dx11Engine.GpuAntialiasingSamples = 4;   // 이후에 만들어지거나 크기가 바뀌는 창부터 적용
```

MSAA 를 켜도 작은 도형과 얇은 선은 WPF 의 CPU 안티에일리어싱을 그대로 써서 아이콘과 1 px 테두리가 스톡과 같게 보입니다.
기준은 바운딩 박스 긴 변이 장치 픽셀 32 이하인 채우기, 두께 2 픽셀 이하인 선이며, 둘 다 바꿀 수 있습니다(`WPFGFX_DX11_MSAA_SMALL_SHAPE`,
`WPFGFX_DX11_MSAA_THIN_STROKE`, 또는 `Dx11Engine.GpuAntialiasingSmallShapePixels` / `GpuAntialiasingThinStrokePixels`. 0 이면 규칙을 끔).
작은 도형이 많이 움직이는 화면은 값을 낮추면 빨라집니다. 큰 도형의 가장자리는 스톡과 조금 다르고, `EdgeMode.Aliased` 콘텐츠도 부드러운 가장자리가 됩니다. 화면 밖 중간 대상(효과,
`RenderTargetBitmap`)은 CPU 경로를 그대로 씁니다. 메모리는 창 크기 × 표본 수 × 4바이트만큼 늘어납니다.

### `D3D11Image`: 백버퍼가 `ID3D11Texture2D` 인 `D3DImage`

```csharp
var image = new D3D11Image();          // 1:1 픽셀이 필요하면 new D3D11Image(dpiX, dpiY)
image.Lock();
image.SetBackBuffer(texture2D);        // ID3D11Texture2D 포인터: B8G8R8A8/B8G8R8X8, USAGE_DEFAULT, 슬라이스 하나
image.AddDirtyRect(new Int32Rect(0, 0, width, height));
image.Unlock();
image.IsFrontBufferAvailableChanged += (_, _) => { if (image.IsFrontBufferAvailable) { /* 백버퍼를 다시 설정 */ } };
```

프로토콜은 보통의 `D3DImage` 와 같습니다(Lock, 렌더링, AddDirtyRect, Unlock). 자기 디바이스에서 만든 텍스처는
`D3D11_RESOURCE_MISC_SHARED` 가 필요하고, `Dx11Engine.GetDevice()` 에서 만든 텍스처는 필요 없습니다. `IDirect3DSurface9` 를 넘기는
기존 `D3DImage` 코드는 그대로 동작합니다.

### `Dx11FrameQueue`: UI 스레드 없이 내 렌더 스레드에서 프레젠트

프레임 큐는 렌더 루프가 어느 스레드에서든 채우는 공유 텍스처 2~4개의 링입니다. 엔진이 마지막으로 프레젠트된 프레임을 프레임마다
`Lock`/`Unlock` 없이 `D3D11Image` 에 합성하며, UI 스레드가 바빠도 생산자를 막지 않습니다. 라이브 프리뷰를 여러 개 보여주는 앱을 위한
설계입니다.

```csharp
using WpfGfx.Dx11.Interop;

// 생산자 쪽(WPF 참조 불필요). 텍스처: B8G8R8A8_UNORM, D3D11_RESOURCE_MISC_SHARED_NTHANDLE | MISC_SHARED_KEYEDMUTEX,
// NT 핸들은 IDXGIResource1::CreateSharedHandle. 엔진이 핸들을 복제하므로 내 것은 닫아도 된다.
Dx11FrameQueue queue = Dx11FrameQueue.Create(width, height, ntHandles);
// 또는 엔진이 링을 만들게 하고 queue.SharedHandles 를 ID3D11Device1::OpenSharedResource1 로 연다:
// Dx11FrameQueue queue = Dx11FrameQueue.CreateWithEngineTextures(width, height, bufferCount: 3);

void RenderFrame()
{
    if (!queue.TryAcquire(out int i)) return;          // 빈 버퍼 없음: 이 프레임은 건너뛴다, 절대 막히지 않는다
    keyedMutex[i].AcquireSync(0, timeout);              // 생산자는 키 0 으로 잡고 ...
    Draw(renderTargetView[i]);
    keyedMutex[i].ReleaseSync(1);                       // ... 키 1 로 넘긴다
    queue.Present(i);                                   // 컴포지션 패스를 예약한다. 큐가 붙기 전에는 false
}

// UI 쪽(WpfGfx.Dx11): 한 번 붙이고, 디바이스 손실 뒤 다시 붙인다.
image.Lock(); image.SetFrameQueue(queue); image.AddDirtyRect(new Int32Rect(0, 0, width, height)); image.Unlock();
image.IsFrontBufferAvailableChanged += (_, _) => { if (image.IsFrontBufferAvailable) { image.Lock(); image.SetFrameQueue(queue); image.Unlock(); } };

Dx11FrameQueueStats stats = queue.Stats;               // 프레젠트·표시·생산자 드롭·획득 타임아웃 수
```

`Dx11FrameQueue.IsHardwareCapable(displayIndex)` 는 그 디스플레이가 하드웨어 디바이스로 합성되는지 알려 줍니다. false(소프트웨어
합성)이거나 `IsAvailable` 이 false(스톡 엔진)면 기존 경로로 폴백하세요. 요구사항·설계·측정: [docs/FRAME_QUEUE.md](docs/FRAME_QUEUE.md).

### 네이티브 export (C++ 등 다른 언어용)

```
HRESULT WpfGfxDx11_GetInfo(WpfGfxDx11Info* info);                    // cbSize, abi, 피처 레벨, 디스플레이 수, 어댑터 LUID
HRESULT WpfGfxDx11_GetDevice(UINT displayIndex, ID3D11Device** device);
HRESULT WpfGfxDx11_SetOption(UINT option, UINT value);               // 1 = 하드웨어 RenderTargetBitmap, 2 = MSAA 표본 수
HRESULT WpfGfxDx11_FrameQueue_Create(const WpfGfxFrameQueueDesc*, IUnknown** queue);
HRESULT WpfGfxDx11_FrameQueue_GetSharedHandles(IUnknown*, HANDLE*, UINT count);
HRESULT WpfGfxDx11_FrameQueue_TryAcquire(IUnknown*, UINT* index);     // 빈 버퍼가 없으면 S_FALSE
HRESULT WpfGfxDx11_FrameQueue_Present(IUnknown*, UINT index);         // 아직 이미지에 안 붙었으면 S_FALSE
HRESULT WpfGfxDx11_FrameQueue_Discard(IUnknown*, UINT index);
HRESULT WpfGfxDx11_FrameQueue_GetStats(IUnknown*, WpfGfxFrameQueueStats*);
HRESULT WpfGfxDx11_FrameQueue_QueryCapability(UINT displayIndex, BOOL* hardware);
```

큐의 `IUnknown*` 은 이 엔진에서 `D3DImage.SetBackBuffer(D3DResourceType.IDirect3DSurface9, ptr)` 가 받는 값이며, 엔진이
`QueryInterface` 로 알아봅니다.

## 진단

| 환경 변수 | 효과 |
|---|---|
| `WPFGFX_DX11_LOG=<파일>` | 로그를 덧붙인다: 디바이스 생성, 스왑체인(`flip`/`bitblt`), 프레젠트 호출, 생성된 HLSL, D3D 디버그 레이어 메시지, 300 프레젠트마다 텍스처 생성 카운터가 있는 메모리 줄. |
| `WPFGFX_DX11_TEST_DEVICE_LOSS=<n>` | n 번째 프레젠트를 디바이스 손실로 처리해 복구 경로를 시험한다. |
| `WPFGFX_DX11_HW_RTB=1` | 하드웨어 `RenderTargetBitmap`(`Dx11Engine.HardwareRenderTargetBitmap = true` 와 같음). |
| `WPFGFX_DX11_MSAA=<4\|8\|1>` | 창의 GPU 안티에일리어싱(`Dx11Engine.GpuAntialiasingSamples` 와 같음). |
| `WPFGFX_DX11_DEBUG_LAYER=0` | 로그는 켜 두고 D3D 디버그 레이어만 끈다(시간 측정용). 로그가 켜져 있으면 300 프레젠트마다 프레임당 작업 카운터도 찍는다. |

## 호환성 메모

- `IDirect3DSurface9` 를 쓰는 `D3DImage` 는 동작합니다. 엔진은 그 interop 만을 위해 Direct3D 9Ex 디바이스를 두고 공유 서피스로
  D3D11 에 가져옵니다.
- `ShaderEffect`: ps_2_0·ps_3_0 바이트코드는 로드 시점에 엔진 안에서 Shader Model 4 로 변환되므로 기존 `.ps` 파일과 `PixelShader`
  객체는 그대로입니다. `D3DCompiler_47_cor3.dll` 이 있어야 합니다.
- `MediaElement` 는 스톡 미디어 파이프라인으로 재생합니다. EVR 믹서는 Direct3D 9Ex 디바이스에서 돌고 프레임은 D3D11 로 공유됩니다.
- `AllowsTransparency` 창은 스톡과 같이 `UpdateLayeredWindow` 로 프레젠트하므로 픽셀 단위 히트 테스트가 같습니다.
- 한 GPU 의 두 모니터에 걸친 창은 플립 모델 스왑체인 하나로 한 번만 렌더링합니다. 다른 GPU 의 모니터는 모니터별 타깃을 유지합니다.
- `RenderOptions.ProcessRenderMode = RenderMode.SoftwareOnly` 는 소프트웨어 래스터라이저를 쓰고 3D 는 WARP 로 그립니다.

## 빌드·테스트·기여

- 빌드: 위 명령과 [docs/BUILD.md](docs/BUILD.md). 저장소는 `wpfgfx_cor3.dll` 과 `D3DCompiler_47_cor3.dll` 만 빌드하며, 소스 배치는
  dotnet/wpf `release/10.0` 과 같아 업스트림 변경을 체리픽할 수 있습니다.
- 게이트: `.\tests\run-smoke.ps1` 이 .NET 8·10 스모크 앱을 빌드하고 D3D11 엔진과 스톡 엔진의 스크린샷을 비교합니다(표준 장면,
  리사이즈, 레이어드 창, 미디어, 디바이스 손실, `RenderTargetBitmap`, 프레임 큐, 선택으로 다중 모니터·벤치 행).
- 업스트림 테스트: `.\tests\run-drts.ps1` 이 dotnet/wpf-test 의 milcore DRT 를 두 엔진으로 실행합니다.
- .NET Framework: `.\tests\run-netfx.ps1` 이 .NET Framework 4.8.x(x64, x86)에서 장면 하나를 두 엔진으로 실행합니다.
- 설계 결정은 [docs/DECISIONS.md](docs/DECISIONS.md)에 번호로, 함정은 [docs/LEARNINGS.md](docs/LEARNINGS.md)에, D3D9 인벤토리와
  D3D11 매핑은 [docs/INVENTORY.md](docs/INVENTORY.md)·[docs/DESIGN.md](docs/DESIGN.md)에 있습니다.

## 저장소 구성

| 경로 | 내용 |
|---|---|
| `src/Microsoft.DotNet.Wpf/src/WpfGfx/` | 네이티브 엔진. dotnet/wpf `release/10.0`(커밋 `01dce087b`)에서 가져왔고 경로를 그대로 유지합니다. |
| `src/Microsoft.DotNet.Wpf/src/Shared/`, `redist/D3DCompiler/` | 엔진 빌드에 필요한 공용 헤더와 D3DCompiler 재배포 프로젝트. |
| `src/WpfGfx.Dx11/`, `src/WpfGfx.Dx11.Interop/` | 추가 API 를 감싸는 관리 도우미 어셈블리. |
| `eng/`, `build.cmd`, `global.json` | 업스트림 Arcade 빌드 인프라. DLL 두 개만 빌드하도록 줄였습니다. |
| `tests/SmokeApp/`, `tests/run-smoke.ps1`, `tests/run-drts.ps1` | 스모크 앱, 게이트 스크립트, 업스트림 DRT 러너. |
| `tests/NetFxHost/`, `tests/run-netfx.ps1` | .NET Framework 4.8.x 호스트와 확인 스크립트. |
| `docs/` | 빌드 절차, 로드맵, 결정, 학습 기록, 설계, 인벤토리, 프레임 큐 명세. |

## 라이선스

dotnet/wpf 와 같은 MIT: [LICENSE.TXT](LICENSE.TXT). 서드파티 고지는 [THIRD-PARTY-NOTICES.TXT](THIRD-PARTY-NOTICES.TXT).
