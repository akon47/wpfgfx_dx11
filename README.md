# wpfgfx_dx11

**A drop-in Direct3D 11 rendering engine for WPF.** This project ports WPF's native rendering engine,
WpfGfx (milcore, `wpfgfx_cor3.dll`), from Direct3D 9 to Direct3D 11. The managed side of WPF
(PresentationCore, PresentationFramework, WindowsBase) is untouched: the result is one native DLL that
replaces the stock `wpfgfx_cor3.dll` of any .NET 8, 9 or 10 WPF application.

[한국어 README](README.ko.md)

## Why

WPF still draws with Direct3D 9. Modern GPUs (Intel Arc, for example) ship no native D3D9 driver and run
WPF through the D3D9On12 translation layer, and dotnet/wpf has no plan to move to D3D11/12
([dotnet/wpf#7062](https://github.com/dotnet/wpf/issues/7062)). The engine source is public, and the
boundary between managed WPF and the engine (107 exports plus the DUCE command protocol) has not changed
from .NET 8 to main. Replacing the engine's internals therefore benefits every WPF version with one DLL.

## Status

Phases 0 to 3 of the [roadmap](docs/ROADMAP.md) are complete and most of Phase 4 is merged. Verified
against the stock engine on .NET 8 and .NET 10 (x64, and x86) by an automated gate
(`tests/run-smoke.ps1`):

- 2D rendering (brushes, geometry, text with ClearType and grayscale, effects, opacity, clipping, 3D viewports)
  matches the stock engine pixel for pixel apart from anti-aliasing noise.
- `ShaderEffect` with ps_2_0 / ps_3_0 bytecode, `D3DImage` with `IDirect3DSurface9`, `MediaElement`,
  layered (`AllowsTransparency`) windows, multi-monitor windows, `RenderTargetBitmap`, device-loss recovery.
- One D3D11 device per adapter, one flip-model swap chain per window, a texture pool, and a frame queue for
  apps that render with their own `ID3D11Device` (see below). A heavy 2D benchmark runs 30 to 50 percent
  faster than the stock engine, and about 2.3 times faster with the optional GPU anti-aliasing.
- Fixes for engine-side dotnet/wpf bugs: a clip geometry containing NaN no longer kills the render thread
  ([#7421](https://github.com/dotnet/wpf/issues/7421)), and thousands of `BitmapCache` elements no longer exhaust
  the process GDI object limit ([#8031](https://github.com/dotnet/wpf/issues/8031)).
- Builds for x64, x86 and arm64 (arm64 is build-only, not yet run).

What still differs from the stock engine: `MediaElement` mixes video on a Direct3D 9Ex device because the
standard EVR mixer only accepts D3D9 (so `d3d9.dll` is loaded while media plays), and the optional
GPU `RenderTargetBitmap` path produces slightly different pixels than the software rasterizer. Real GPU
resets (TDR) have only been tested through the engine's simulation hook.

## Getting the DLL

Build it from source (Visual Studio 2022 with the C++ workload and Windows SDK, .NET SDK 8 or later):

```powershell
$env:CL = "/wd4819"      # needed on non-UTF-8 code pages (Korean, Japanese, ...), harmless elsewhere
.\build.cmd -configuration Release -platform x64 -nodeReuse 0
```

The build produces two files that must always stay in the same folder:

| File | Role |
|---|---|
| `artifacts\bin\wpfgfx\x64\Release\wpfgfx_cor3.dll` | the engine (file version `42.42.42.42424` marks a local build) |
| `artifacts\bin\D3DCompiler\x64\Release\D3DCompiler_47_cor3.dll` | shader compiler the engine loads from its own folder |

Use `-platform x86` or `-platform arm64` for other architectures. The DLL must match the bitness of the
process that loads it. Details, requirements and troubleshooting: [docs/BUILD.md](docs/BUILD.md).

## Using the DLL in your application

A framework-dependent WPF app loads `wpfgfx_cor3.dll` from the shared framework folder
(`C:\Program Files\dotnet\shared\Microsoft.WindowsDesktop.App\<version>`). **Copying the DLL next to your
executable does nothing**, because that folder is not on the native search path for the engine. There are
three ways in, pick one:

### 1. A `DllImportResolver` in your app (recommended)

Register a resolver for the three WPF assemblies before WPF loads the engine, for example in the static
constructor of your `App` class. The resolver points `wpfgfx_cor3.dll` at your copy; everything else resolves
as usual.

```csharp
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows;

public partial class App : Application
{
    static App()
    {
        string dir = Path.Combine(AppContext.BaseDirectory, "wpfgfx");   // holds wpfgfx_cor3.dll + D3DCompiler_47_cor3.dll

        DllImportResolver resolver = (name, _, _) =>
        {
            if (!name.StartsWith("wpfgfx", StringComparison.OrdinalIgnoreCase)) return IntPtr.Zero;
            string candidate = Path.Combine(dir, name.EndsWith(".dll", StringComparison.OrdinalIgnoreCase) ? name : name + ".dll");
            return File.Exists(candidate) ? NativeLibrary.Load(candidate) : IntPtr.Zero;   // IntPtr.Zero = fall back to the stock engine
        };

        // Every WPF assembly that declares DllImport("wpfgfx_cor3.dll").
        NativeLibrary.SetDllImportResolver(typeof(System.Windows.Media.Visual).Assembly, resolver);       // PresentationCore
        NativeLibrary.SetDllImportResolver(typeof(System.Windows.Window).Assembly, resolver);             // PresentationFramework
        NativeLibrary.SetDllImportResolver(typeof(System.Windows.Threading.Dispatcher).Assembly, resolver); // WindowsBase
    }
}
```

Ship the two DLLs in a subfolder of your app (`wpfgfx` above) and the app runs on the D3D11 engine on any
machine, with the stock engine as the fallback when the files are missing. `SetDllImportResolver` can be
registered only once per assembly; if your app already does this for another reason, merge the two resolvers.
The smoke app in `tests/SmokeApp` (`App.xaml.cs`, option `--wpfgfx <folder>`) is a working example.

### 2. A private copy of the runtime (for apps you cannot change)

Copy the shared framework to a folder of your own, replace the two files there, and point the app at that
copy with `DOTNET_ROOT`. Nothing on the machine changes.

```powershell
$src = "C:\Program Files\dotnet"; $dst = "C:\wpf-dx11-runtime"; $ver = "10.0.12"   # the version your app resolves to
foreach ($p in "shared\Microsoft.NETCore.App\$ver", "shared\Microsoft.WindowsDesktop.App\$ver", "host\fxr\$ver") {
    robocopy "$src\$p" "$dst\$p" /E | Out-Null
}
Copy-Item "$src\dotnet.exe" $dst
Copy-Item .\wpfgfx_cor3.dll, .\D3DCompiler_47_cor3.dll "$dst\shared\Microsoft.WindowsDesktop.App\$ver\" -Force

$env:DOTNET_ROOT = $dst
.\YourApp.exe
```

`tests/run-drts.ps1` automates exactly this to run the upstream WPF test suite against the engine.
Self-contained apps are simpler: their own folder contains `wpfgfx_cor3.dll`, so just replace the two files there.

### 3. Replace the files in the shared framework

Replacing `wpfgfx_cor3.dll` and `D3DCompiler_47_cor3.dll` inside
`C:\Program Files\dotnet\shared\Microsoft.WindowsDesktop.App\<version>` switches every WPF app of that
runtime on the machine. Keep a copy of the originals; a runtime update will overwrite your files.

### Checking which engine is running

- `RenderCapability.Tier >> 16` is `2` on both engines; it does not tell them apart.
- The loaded module's path and file version do: enumerate `Process.GetCurrentProcess().Modules` for
  `wpfgfx_cor3.dll` (a local build has file version `42.42.42.42424`).
- With the helper assembly below, `Dx11Engine.IsAvailable` is `true` only on the D3D11 engine.
- Set `WPFGFX_DX11_LOG=<file>` before starting the app: the D3D11 engine writes a log there, the stock engine
  ignores the variable.

## Extra features of the D3D11 engine

Beyond the 107 exports WPF uses, the engine exports a small extra API (ABI version 2, see
`src/Microsoft.DotNet.Wpf/src/WpfGfx/core/api/api_dx11.cpp`). Two helper assemblies wrap it; both detect
the stock engine and report `IsAvailable == false` instead of failing, so an app can keep a fallback path.

| Assembly | Depends on WPF | Contents |
|---|---|---|
| `src/WpfGfx.Dx11` | yes | `Dx11Engine`, `D3D11Image` |
| `src/WpfGfx.Dx11.Interop` | no | `Dx11FrameQueue` (usable from a render library that must not reference WPF) |

### `Dx11Engine`

```csharp
using WpfGfx.Dx11;

if (Dx11Engine.IsAvailable)
{
    Dx11EngineInfo info = Dx11Engine.GetInfo();        // AbiVersion, FeatureLevel, DisplayCount, AdapterLuid
    IntPtr device = Dx11Engine.GetDevice(0);           // the engine's ID3D11Device, AddRef'd: create resources on it,
    Marshal.Release(device);                           // never touch its immediate context (it belongs to the render thread)
    Dx11Engine.HardwareRenderTargetBitmap = true;      // opt in: RenderTargetBitmap renders on the GPU, ps_3_0 effects included
}
```

The GPU `RenderTargetBitmap` path can also be switched on with the environment variable
`WPFGFX_DX11_HW_RTB=1`. It is off by default because its pixels differ slightly (anti-aliasing, text) from
the software rasterizer the stock engine always uses.

### GPU anti-aliasing (opt-in)

WPF anti-aliases shape edges on the CPU: every frame, each filled or stroked shape is split into coverage
trapezoids by the render thread. In shape-heavy, animated scenes that is about half of the render thread's
time. With GPU anti-aliasing the window renders into a 4x or 8x multisampled target, shapes are tessellated
without coverage, and the GPU produces the edges. On the smoke benchmark this raises the frame rate from about
23 to 38 fps.

```powershell
$env:WPFGFX_DX11_MSAA = "4"     # or 8; 1 = best the GPU supports. Set before the app starts.
```

```csharp
Dx11Engine.GpuAntialiasingSamples = 4;   // takes effect for windows created or resized afterwards
```

Edge pixels differ slightly from the stock output, and content drawn with `EdgeMode.Aliased` also gets smooth
edges. Off-screen intermediates (effects, `RenderTargetBitmap`) keep the CPU path. Memory grows by the window
size times the sample count times 4 bytes.

### `D3D11Image`: a `D3DImage` with an `ID3D11Texture2D` back buffer

```csharp
var image = new D3D11Image();          // or new D3D11Image(dpiX, dpiY) for 1:1 pixels
image.Lock();
image.SetBackBuffer(texture2D);        // IntPtr to ID3D11Texture2D: B8G8R8A8/B8G8R8X8, USAGE_DEFAULT, single slice.
image.AddDirtyRect(new Int32Rect(0, 0, width, height));
image.Unlock();
image.IsFrontBufferAvailableChanged += (_, _) => { if (image.IsFrontBufferAvailable) { /* set the back buffer again */ } };
```

The protocol is the ordinary `D3DImage` one (Lock, render, AddDirtyRect, Unlock). A texture created on your
own device needs `D3D11_RESOURCE_MISC_SHARED`; a texture created on `Dx11Engine.GetDevice()` does not.
Existing `D3DImage` code that passes an `IDirect3DSurface9` keeps working unchanged.

### `Dx11FrameQueue`: presenting from your own render thread without the UI thread

A frame queue is a ring of 2 to 4 shared textures that your render loop fills on any thread; the engine
composites the newest presented frame into a `D3D11Image` without `Lock`/`Unlock` per frame and without
blocking the producer when the UI thread is busy. Designed for apps that show many live previews.

```csharp
using WpfGfx.Dx11.Interop;

// Producer side (no WPF reference needed). Textures: B8G8R8A8_UNORM, D3D11_RESOURCE_MISC_SHARED_NTHANDLE | MISC_SHARED_KEYEDMUTEX,
// NT handles from IDXGIResource1::CreateSharedHandle. The engine duplicates the handles; close yours afterwards.
Dx11FrameQueue queue = Dx11FrameQueue.Create(width, height, ntHandles);
// or let the engine allocate the ring and open queue.SharedHandles with ID3D11Device1::OpenSharedResource1:
// Dx11FrameQueue queue = Dx11FrameQueue.CreateWithEngineTextures(width, height, bufferCount: 3);

void RenderFrame()
{
    if (!queue.TryAcquire(out int i)) return;          // no free buffer: skip this frame, never block
    keyedMutex[i].AcquireSync(0, timeout);              // producer owns key 0 ...
    Draw(renderTargetView[i]);
    keyedMutex[i].ReleaseSync(1);                       // ... and hands the frame over with key 1
    queue.Present(i);                                   // schedules a composition pass; returns false before the queue is attached
}

// UI side (WpfGfx.Dx11): attach once, re-attach after device loss.
image.Lock(); image.SetFrameQueue(queue); image.AddDirtyRect(new Int32Rect(0, 0, width, height)); image.Unlock();
image.IsFrontBufferAvailableChanged += (_, _) => { if (image.IsFrontBufferAvailable) { image.Lock(); image.SetFrameQueue(queue); image.Unlock(); } };

Dx11FrameQueueStats stats = queue.Stats;               // presented, displayed, dropped by producer, acquire timeouts
```

`Dx11FrameQueue.IsHardwareCapable(displayIndex)` tells whether the display is composed on a hardware device;
when it is false (software composition), or when `IsAvailable` is false (stock engine), fall back to your
previous path. Requirements, design and measurements: [docs/FRAME_QUEUE.md](docs/FRAME_QUEUE.md).

### Native exports (for C++ or other languages)

```
HRESULT WpfGfxDx11_GetInfo(WpfGfxDx11Info* info);                    // cbSize, abi, feature level, display count, adapter LUID
HRESULT WpfGfxDx11_GetDevice(UINT displayIndex, ID3D11Device** device);
HRESULT WpfGfxDx11_SetOption(UINT option, UINT value);               // 1 = hardware RenderTargetBitmap, 2 = MSAA samples
HRESULT WpfGfxDx11_FrameQueue_Create(const WpfGfxFrameQueueDesc*, IUnknown** queue);
HRESULT WpfGfxDx11_FrameQueue_GetSharedHandles(IUnknown*, HANDLE*, UINT count);
HRESULT WpfGfxDx11_FrameQueue_TryAcquire(IUnknown*, UINT* index);     // S_FALSE when no buffer is free
HRESULT WpfGfxDx11_FrameQueue_Present(IUnknown*, UINT index);         // S_FALSE when not attached to an image yet
HRESULT WpfGfxDx11_FrameQueue_Discard(IUnknown*, UINT index);
HRESULT WpfGfxDx11_FrameQueue_GetStats(IUnknown*, WpfGfxFrameQueueStats*);
HRESULT WpfGfxDx11_FrameQueue_QueryCapability(UINT displayIndex, BOOL* hardware);
```

The `IUnknown*` of a queue is what `D3DImage.SetBackBuffer(D3DResourceType.IDirect3DSurface9, ptr)` accepts
on this engine; the engine recognises it through `QueryInterface`.

## Diagnostics

| Environment variable | Effect |
|---|---|
| `WPFGFX_DX11_LOG=<file>` | Appends a log: device creation, swap chains (`flip`/`bitblt`), present calls, generated HLSL, D3D debug-layer messages, and every 300 presents a memory line with texture creation counters. |
| `WPFGFX_DX11_TEST_DEVICE_LOSS=<n>` | Treats the n-th present as a lost device to exercise the recovery path. |
| `WPFGFX_DX11_HW_RTB=1` | Hardware `RenderTargetBitmap` (same as `Dx11Engine.HardwareRenderTargetBitmap = true`). |
| `WPFGFX_DX11_MSAA=<4\|8\|1>` | GPU anti-aliasing for windows (same as `Dx11Engine.GpuAntialiasingSamples`). |
| `WPFGFX_DX11_DEBUG_LAYER=0` | Keep the log but skip the D3D debug layer (for timing). With the log on, the log also prints per-frame work counters every 300 presents. |

## Compatibility notes

- `D3DImage` with `IDirect3DSurface9` works: the engine keeps a Direct3D 9Ex device only for that interop and
  brings the surface into D3D11 through a shared surface.
- `ShaderEffect`: ps_2_0 and ps_3_0 bytecode is translated to Shader Model 4 inside the engine at load time,
  so existing `.ps` files and `PixelShader` objects are unchanged. `D3DCompiler_47_cor3.dll` must be present.
- `MediaElement` plays through the stock media pipeline; the EVR mixer runs on a Direct3D 9Ex device and the
  frames are shared into D3D11.
- `AllowsTransparency` windows are presented with `UpdateLayeredWindow` like the stock engine, so per-pixel
  hit testing behaves the same.
- Windows that span two monitors of one GPU render once with a single flip-model swap chain; monitors on
  different GPUs keep one target per monitor.
- `RenderOptions.ProcessRenderMode = RenderMode.SoftwareOnly` uses the software rasterizer, with 3D content
  drawn through WARP.

## Building, testing, contributing

- Build: see above and [docs/BUILD.md](docs/BUILD.md). The repository builds only `wpfgfx_cor3.dll` and
  `D3DCompiler_47_cor3.dll`; the source layout mirrors dotnet/wpf `release/10.0` so upstream changes can be
  cherry-picked.
- Gate: `.\tests\run-smoke.ps1` builds the smoke app for .NET 8 and .NET 10 and compares screenshots of the
  D3D11 engine with the stock engine (standard scene, resize, layered window, media, device loss,
  `RenderTargetBitmap`, frame queues, optional multi-monitor and benchmark rows).
- Upstream tests: `.\tests\run-drts.ps1` runs the milcore DRTs of dotnet/wpf-test on both engines.
- Design decisions are numbered in [docs/DECISIONS.md](docs/DECISIONS.md); pitfalls in
  [docs/LEARNINGS.md](docs/LEARNINGS.md); the D3D9 inventory and its D3D11 mapping in
  [docs/INVENTORY.md](docs/INVENTORY.md) and [docs/DESIGN.md](docs/DESIGN.md). The docs are written in Korean.

## Repository layout

| Path | Contents |
|---|---|
| `src/Microsoft.DotNet.Wpf/src/WpfGfx/` | The native engine, taken from dotnet/wpf `release/10.0` (commit `01dce087b`) with paths kept identical. |
| `src/Microsoft.DotNet.Wpf/src/Shared/`, `redist/D3DCompiler/` | Shared headers and the D3DCompiler redistribution project the engine build needs. |
| `src/WpfGfx.Dx11/`, `src/WpfGfx.Dx11.Interop/` | Managed helper assemblies for the extra API. |
| `eng/`, `build.cmd`, `global.json` | Upstream Arcade build infrastructure, trimmed to the two DLLs. |
| `tests/SmokeApp/`, `tests/run-smoke.ps1`, `tests/run-drts.ps1` | Smoke app, gate script, upstream DRT runner. |
| `docs/` | Build guide, roadmap, decisions, learnings, design, inventory, frame queue spec. |

## License

MIT, the same as dotnet/wpf: [LICENSE.TXT](LICENSE.TXT). Third-party notices are in
[THIRD-PARTY-NOTICES.TXT](THIRD-PARTY-NOTICES.TXT).
