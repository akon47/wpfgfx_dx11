# D3D9 사용 인벤토리와 D3D11 매핑

작성일 2026-10-28, 기준 커밋 01dce087b(dotnet/wpf release/10.0). 수치는 `tools/inventory/d3d9_inventory.py`가 만든
[`inventory/summary.md`](inventory/summary.md)에서 가져왔고, 호출 지점은 [`inventory/call-sites.md`](inventory/call-sites.md)에
파일:줄 단위로 있다. 이 문서는 그 수치의 해석과 D3D11 대응이다. 재생성: `python tools/inventory/d3d9_inventory.py`.

## 1. 한눈에

| 항목 | 값 |
|---|---|
| 검사한 소스 파일 | 1,134개 (WpfGfx 아래, codegen·exts·tools 제외) |
| D3D9를 참조하는 파일 | 155개 |
| `core/hw` | 82개 파일, 55,805줄. 인터페이스 참조 446, 메서드 호출 332, 열거형 909 — **전체의 80% 이상** |
| 그 밖의 참조 | `core/av`(미디어) 19개, `core/common`(로더·디스플레이·포맷 유틸) 10개, `core/resources`(효과) 10개, `core/fxjit`(DX9 바이트코드 파서, 유지) 5개, 나머지는 오류 코드·열거형 몇 개 |

**이음새.** 상위 레이어는 대부분 D3D9를 직접 부르지 않고 래퍼를 부른다.

- `CD3DDeviceLevel1` (`core/hw/d3ddevice.h/.cpp`, 7,866줄) — `IDirect3DDevice9(Ex)`를 감싸고 `SetTexture`, `SetRenderState`,
  `SetPixelShaderConstantF`, `DrawTriangleList`, `Present` 같은 D3D9 모양의 메서드를 제공한다. 효과(`core/resources`),
  셰이더 파이프라인, 컬러 소스가 여기에 의존한다.
- `CD3DResource` 계열 — `CD3DSurface`, `CD3DTexture`, `CD3DLockableTexture`, `CD3DVidMemOnlyTexture`, `CD3DTextureSurface`,
  `CD3DVertexBuffer*`, `CD3DIndexBuffer`, `CD3DSwapChain`, `CD3DSwapChainWithSwDC` (`core/hw/d3d*.cpp`).
- `CD3DRenderState` + `CHwRenderStateManager` (`core/hw/d3drenderstate.cpp`, `HwRenderStateManager.cpp`) — 렌더/샘플러/텍스처
  스테이지 상태 캐시. D3DRS_/D3DTSS_/D3DSAMP_ 사용의 90%가 여기 있다.
- `CD3DDeviceManager` (`core/hw/d3ddevicemanager.cpp`) — 어댑터별 디바이스 생성·공유. `CDisplaySet`/`CDisplay`
  (`core/common/display.cpp`) — 어댑터·모니터 열거, caps, Tier 계산. `CD3DLoader` (`core/common/d3dloader.cpp`) — `d3d9.dll` 로드.

그러나 D3D9 **타입**은 래퍼 바깥까지 새어 나와 있다. 치환 작업의 넓이는 이 타입들이 정한다.

| 타입 | 참조 | 파일 수 | 대응 |
|---|---:|---:|---|
| `IDirect3DSurface9` | 190 | 38 | `ID3D11Texture2D` + RTV/SRV를 묶은 자체 `CD3DSurface` |
| `D3DFMT_*` | 181 | 22 | `DXGI_FORMAT` (§4.1) |
| `D3DSURFACE_DESC` | 73 | — | `D3D11_TEXTURE2D_DESC` |
| `IDirect3DDevice9` | 72 | 38 | `ID3D11Device` + `ID3D11DeviceContext` (래퍼 안으로 숨김) |
| `IDirect3DTexture9` | 55 | 24 | `ID3D11Texture2D` + `ID3D11ShaderResourceView` |
| `D3DMULTISAMPLE_TYPE` | 36 | 24 | `DXGI_SAMPLE_DESC` |
| `D3DPOOL` | 37 | 18 | `D3D11_USAGE` + `CPUAccessFlags` |
| `D3DPRESENT_PARAMETERS` | 26 | 11 | `DXGI_SWAP_CHAIN_DESC1` |
| `D3DMATRIX` | 35 | — | 유지 (행렬 레이아웃은 그대로, 상수 버퍼로 전달) |

## 2. 영역별 분류와 조치

| 영역 | 파일 | 역할 | 조치 |
|---|---|---|---|
| `core/hw` | 82 | 하드웨어 렌더링 전부 | **치환 대상.** §7 체크리스트 |
| `core/common` | `d3dloader.cpp`, `display.cpp`, `Display.h`, `d3dutils.cpp`, `Tier.cpp` | d3d9.dll 로드, 어댑터·디스플레이 열거, caps→Tier, 포맷 변환 | DXGI 팩터리·어댑터·출력으로 치환. Tier 계산은 Feature Level 기반으로 (§6) |
| `core/av` | 19 | MediaElement: EVR 프레젠터, DXVA2 디바이스 매니저, 하드웨어/소프트웨어 미디어 버퍼 | AV 전용 D3D9Ex 디바이스에서 믹싱하고 공유 서피스로 D3D11에 연결(D25, M11 완료) |
| `core/resources` | `BlurEffect.cpp`, `DropShadowEffect.cpp`, `ShaderEffect.cpp`, `Effect.cpp`, `PixelShader.cpp` | 내장 효과와 ShaderEffect. 래퍼의 `SetTexture`/`SetSamplerState`/`SetPixelShaderConstantF`/`CreatePixelShader` 호출 | 래퍼 시그니처를 유지하면 거의 그대로. `CreatePixelShader`에 들어오는 ps_2_0/ps_3_0 바이트코드만 D6 변환 |
| `core/fxjit/PixelShader` | `pstrans.cpp` 등 | 소프트웨어 경로용 DX9 바이트코드→x86 JIT. `d3d9types.h`/`d3d9caps.h` 사설 복사본 포함 | **유지.** 바이트코드 파서는 D6 변환기의 앞단으로 재활용 |
| `common/DirectXLayer` | `XMath/*`, `D3DX9/*` | 수학 라이브러리 추상화. 빌드는 `DIRECTXMATH`(DirectXMath) 변종을 쓴다(`Graphics.props`). `shader_compiler_xm.hpp`가 `D3DCompile` 호출 | `D3DX9/`는 컴파일되지 않으므로 삭제. 셰이더 컴파일러는 프로필 문자열만 SM4로 |
| `common/shared/pixelformatutils.cpp` | 1 | `MilPixelFormat` ↔ `D3DFMT` 표 | `DXGI_FORMAT` 표로 교체 |
| `core/uce`, `core/api`, `core/meta`, `core/sw` | 12 | `D3DERR_*` 분기, `D3DCULL_`/`D3DCMP_` 상수, `D3DDEVTYPE` 선택, `WaitForVBlank`/`Present` 위임 | 오류 코드는 자체 HRESULT로 매핑(§6.2), 상수는 자체 열거형으로 |
| `include/std.h` | 1 | `#include <d3d9.h>` 전역 | `d3d11.h`, `dxgi1_2.h`로 교체. 이 한 줄이 바뀌는 순간 전체가 컴파일 오류가 되므로 치환의 마지막 단계에 |
| `core/hw/shaders/ShaderGen` | 4 | 텍스트 셰이더 리소스를 만들던 일회성 도구. 빌드되지 않음 | 삭제하고 fxc 빌드 단계로 대체 (§5) |

## 3. 서브시스템별 D3D9 → D3D11 매핑

| 서브시스템 | D3D9 (현재) | D3D11 (목표) | 비고 |
|---|---|---|---|
| 로더 | `LoadLibrary("d3d9.dll")` → `Direct3DCreate9Ex` (`d3dloader.cpp`) | `D3D11CreateDevice`, `CreateDXGIFactory2` 동적 로드 (`d3d11.dll`, `dxgi.dll`) | 디버그 레이어는 레지스트리 플래그로 |
| 어댑터·디스플레이 | `IDirect3D9::GetAdapterCount/GetAdapterMonitor/GetAdapterLUID/GetAdapterDisplayModeEx`, `D3DCAPS9` (`display.cpp`) | `IDXGIFactory1::EnumAdapters1`, `IDXGIAdapter::EnumOutputs`, `DXGI_OUTPUT_DESC`, `DXGI_ADAPTER_DESC1.AdapterLuid` | 모니터↔어댑터 매핑은 출력(`IDXGIOutput`)에서 |
| 디바이스 | `CreateDeviceEx(D3DDEVTYPE_HAL, D3DCREATE_MULTITHREADED \| FPU_PRESERVE \| ...)`; 어댑터별 디바이스 (`d3ddevicemanager.cpp`); `D3DDEVTYPE_SW`는 소프트웨어 래스터라이저 표식 | `D3D11CreateDevice(adapter, HARDWARE, SINGLETHREADED 아님, FL 11_1→10_0)`; 즉시 컨텍스트 하나를 디바이스 락 아래서 사용 | D5. `D3DDEVTYPE_SW`/`REF`는 자체 열거형 `DeviceKind`로 |
| 디바이스 손실 | `TestCooperativeLevel`, `CheckDeviceState`, `Reset`, `D3DERR_DEVICELOST/DEVICEHUNG/DEVICEREMOVED` | `GetDeviceRemovedReason`, `DXGI_ERROR_DEVICE_REMOVED/RESET`; 디바이스 재생성 경로는 유지 | Reset 개념 없음: 스왑체인은 `ResizeBuffers` |
| 스왑체인·프레젠테이션 | `CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS{SWAPEFFECT_COPY/DISCARD, BackBufferFormat, MultiSample, hDeviceWindow})`, `Present(rect)`, `PresentEx`, `WaitForVBlank`, `GetRasterStatus` (`d3dswapchain.cpp`, `hwdisplayrt.cpp`, `hwhwndrt.cpp`, `d3ddevice.cpp`) | HWND별 `IDXGISwapChain1` (`CreateSwapChainForHwnd`, `FLIP_SEQUENTIAL`, 2버퍼), `Present1(sync, flags, DXGI_PRESENT_PARAMETERS{DirtyRects})`, `IDXGIOutput::WaitForVBlank`, `GetFrameStatistics` | 부분 프레젠트(`D3DSWAPEFFECT_COPY` + rect)는 `DXGI_PRESENT_PARAMETERS`의 dirty rect로. 멀티샘플 백버퍼는 플립 모델에 없으므로 MSAA 렌더타깃→`ResolveSubresource`→백버퍼 |
| GDI 프레젠트·레이어드 윈도우 | `PresentWithGDI`, `CD3DSwapChainWithSwDC`, `GetDC/ReleaseDC`, `D3DPRESENTFLAG_LOCKABLE_BACKBUFFER` | 스테이징 텍스처 `Map` → `UpdateLayeredWindow`/`BitBlt`. 또는 `IDXGISurface1::GetDC`(`DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE`, B8G8R8A8만) | 투명 창(AllowsTransparency)이 이 경로 |
| 렌더타깃 | `CreateRenderTarget`, `SetRenderTarget`, `GetBackBuffer`, `Clear(D3DCLEAR_TARGET\|ZBUFFER\|STENCIL)`, `StretchRect`, `ColorFill`, `GetRenderTargetData` (`hwsurfrt.cpp`, `d3ddevice.cpp`) | `ID3D11RenderTargetView`, `OMSetRenderTargets`, `ClearRenderTargetView`/`ClearDepthStencilView`, `CopySubresourceRegion`/`ResolveSubresource`, 스테이징 복사 후 `Map` | `StretchRect`의 확대·축소 필터링은 셰이더 블릿으로 |
| 깊이·스텐실 | `CreateDepthStencilSurface(D3DFMT_D24S8, multisample)`, `SetDepthStencilSurface`, `CheckDepthStencilMatch` | `ID3D11DepthStencilView`, `DXGI_FORMAT_D24_UNORM_S8_UINT` | 3D(Viewport3D)와 스텐실 클리핑에 사용 |
| 텍스처 | `CreateTexture(w,h,levels,usage,D3DFMT,D3DPOOL)`, `GetSurfaceLevel`, `LockRect/UnlockRect`, `AddDirtyRect`, `UpdateSurface`, `UpdateTexture`, `D3DUSAGE_AUTOGENMIPMAP`, `D3DPOOL_SYSTEMMEM` 참조 텍스처 (`d3dtexture.cpp`, `d3dlockabletexture.cpp`, `d3dvidmemonlytexture.cpp`, `HwVidMemTextureManager.cpp`, `hwbitmapcolorsource.cpp`) | `CreateTexture2D(DEFAULT/DYNAMIC/STAGING)`, `UpdateSubresource`, `Map(WRITE_DISCARD)`, `CopySubresourceRegion`, `GenerateMips`(`D3D11_RESOURCE_MISC_GENERATE_MIPS`) | SYSTEMMEM+UpdateSurface 패턴은 STAGING→DEFAULT 복사 또는 `UpdateSubresource`로 단순화 |
| 포맷 | `D3DFMT_*` 63종 등장, 실제 생성에 쓰는 것은 10종 미만 | §4.1 | `P8`·`R8G8B8`은 DXGI에 없음 → CPU에서 BGRA로 변환 |
| 버텍스·인덱스 | `CreateVertexBuffer/IndexBuffer(D3DUSAGE_WRITEONLY\|DYNAMIC)`, `Lock(DISCARD/NOOVERWRITE)`, `SetStreamSource`, `SetIndices`, `SetFVF`, `CreateVertexDeclaration` (`d3dvertex.h`, `hwvertexbuffer.cpp`, `hw3dgeometrystreambuffer.cpp`) | `D3D11_BIND_VERTEX_BUFFER/INDEX_BUFFER`, `Map(WRITE_DISCARD/WRITE_NO_OVERWRITE)`, `IASetVertexBuffers`, `IASetIndexBuffer(R16_UINT)`, `ID3D11InputLayout` (FVF 조합별 캐시) | FVF 9종 사용: XYZ, DIFFUSE, SPECULAR, NORMAL, TEX1/2/4/6/8 → 입력 레이아웃 5종 정도 |
| 그리기 | `DrawPrimitive(D3DPT_TRIANGLELIST/STRIP/FAN, LINELIST)`, `DrawIndexedPrimitive`, `DrawPrimitiveUP`, `DrawIndexedPrimitiveUP` | `IASetPrimitiveTopology`, `Draw`, `DrawIndexed`; `*UP`은 동적 링 버퍼에 복사 후 `Draw` | **TRIANGLEFAN은 D3D11에 없다**: 인덱스 변환 (`EndPrimitiveFan`) |
| 렌더 상태 | `SetRenderState` 41종 사용 (§4.2), `GetRenderState`, 상태 블록 | 블렌드·래스터라이저·깊이스텐실 **상태 객체 캐시**(desc 해시 → 객체) | 고정 기능 전용 상태는 삭제 |
| 샘플러 | `SetSamplerState(MIN/MAG/MIPFILTER, ADDRESSU/V, BORDERCOLOR, MAXANISOTROPY)` | `ID3D11SamplerState` 캐시, `PSSetSamplers` | 슬롯당 샘플러 |
| 텍스처 스테이지(고정 기능) | `SetTextureStageState(COLOROP/ALPHAOP/ARG1/ARG2, TEXCOORDINDEX, TEXTURETRANSFORMFLAGS)`, `SetTransform(TEXTURE)`, `D3DTA_*`, `D3DTOP_MODULATE/SELECTARG1/DISABLE` (`hwtexturedcolorsource.cpp`, `d3drenderstate.cpp`, `HwRenderStateManager.cpp`) | **없음.** 셰이더 파이프라인(`HwShaderPipeline` + `hwshaderfragmenttohlslconverter`)이 모든 브러시 조합을 HLSL로 만든다. 고정 기능 폴백(`CHwFFPipeline`, `hwffshaders.cpp`의 `CHwDiffuseShader/SpecularShader/EmissiveShader`)은 삭제하고 3D 조명도 셰이더로 | D4 |
| 변환 | `SetTransform(WORLD/VIEW/PROJECTION)`, `GetTransform`, `Set3DTransforms` | 상수 버퍼 (VS `cbuffer`) | 셰이더 경로는 이미 상수로 넘긴다 |
| 셰이더 | `CreatePixelShader/CreateVertexShader(DWORD* bytecode)`, `SetPixelShader/SetVertexShader`, `SetPixelShaderConstantF/I/B`, `SetVertexShaderConstantF`, `D3DCompile(…, "ps_2_0"/"vs_2_0"/"ps_3_0")` (`hwshader.cpp`, `HwShaderPipeline.cpp`, `HwShaderBuilder.cpp`, `shader_compiler_xm.hpp`) | `CreatePixelShader/CreateVertexShader(DXBC)`, `PSSetShader/VSSetShader`, 상수는 `cbuffer`(레지스터 c0..cN을 float4 배열로 유지하면 변경 최소), `D3DCompile(…, "ps_4_0"/"vs_4_0")` | HLSL 조립기 출력은 `tex2D`→`Texture2D.Sample`, 시맨틱, `cbuffer`로 손봐야 함 |
| ShaderEffect 바이트코드 | 앱이 준 ps_2_0/ps_3_0 바이트코드를 `CreatePixelShader`에 그대로 (`HwShaderEffect.cpp`) | DX9 바이트코드 → HLSL/SM4 변환기 (`fxjit/PixelShader/pstrans.cpp` 파서 재활용) | D6 |
| 쿼리·GPU 마커 | `CreateQuery(D3DQUERYTYPE_EVENT)`, `Issue(END)`, `GetData` (`gpumarker.cpp`, `d3dstats.cpp`) | `D3D11_QUERY_EVENT`, `End`, `GetData` | 1:1 |
| 글리프 캐시 | `d3dglyphbank.cpp`: 텍스처 아틀라스에 `LockRect`/`UpdateSurface`로 글리프 비트맵 업로드, `GetSurfaceLevel` | `UpdateSubresource` 또는 스테이징 복사 | 텍스트 셰이더는 §5 |
| 중간 렌더타깃·비트맵 캐시 | `hwtexturert.cpp`, `hwdestinationtexture.cpp`, `HwBitmapCache.cpp`: `D3DUSAGE_RENDERTARGET` 텍스처 | `BIND_RENDER_TARGET \| BIND_SHADER_RESOURCE` 텍스처 + RTV/SRV | 동일 리소스를 RT와 SRV로 번갈아 쓰므로 바인딩 해제 순서 주의 |
| D3DImage interop | `InteropDeviceBitmap.cpp`: 앱 `IDirect3DSurface9` → 같은 어댑터면 `StretchRect`, 아니면 `GetRenderTargetData`→시스템 메모리; `D3DCAPS2_CANSHARERESOURCE` | 앱 서피스가 D3D9Ex 공유 가능이면 공유 핸들→`OpenSharedResource`; 아니면 interop용 D3D9Ex 디바이스에서 복사. `ID3D11Texture2D`를 `QueryInterface`로 먼저 판별 | D7, D8 |
| 미디어 | `IDirect3DDeviceManager9`(DXVA2), EVR 프레젠터가 D3D9 서피스에 비디오 프레임, `hwmediabuffer.cpp`가 WPF 텍스처로 복사 | D3D9 그대로(AV 전용 D3D9Ex 디바이스), 믹서 텍스처를 공유 핸들로 `OpenSharedResource` 해 엔진 비트맵에 캐시 | M11 완료(D25) |
| 소프트웨어 폴백 | `swfallback.cpp`, `hwsw3dfallback.cpp`: `D3DPOOL_SYSTEMMEM` 서피스에 소프트웨어 렌더 후 `UpdateSurface` | STAGING/DYNAMIC 텍스처에 쓰고 `CopySubresourceRegion` | 유지 |
| caps | `D3DCAPS9` 23곳: `MaxTextureWidth/Height`, `PixelShaderVersion`, `VertexShaderVersion`, `TextureCaps(POW2, NONPOW2CONDITIONAL)`, `Caps2(CANAUTOGENMIPMAP, CANSHARERESOURCE)`, `DevCaps2(CAN_STRETCHRECT_FROM_TEXTURES)`, `RasterCaps(SCISSORTEST)`, `TextureAddressCaps(BORDER)`, `StretchRectFilterCaps`, `MaxStreams`, `MaxTextureBlendStages`, `MaxSimultaneousTextures` | Feature Level에서 상수로 도출: FL10 이상이면 텍스처 8192(FL11: 16384), 비2의 거듭제곱 무조건, 보더 지원, 시저 지원, 자동 밉 지원, 공유 지원 | `CheckDeviceFormat`/`CheckDeviceMultiSampleType` → `CheckFormatSupport`/`CheckMultisampleQualityLevels` |

## 4. 열거형·상수 매핑 (실제 사용 값만)

### 4.1 포맷 (`D3DFMT_` → `DXGI_FORMAT`)

생성·변환 코드에서 실제로 쓰는 것. 나머지 50여 종은 `pixelformatutils.cpp`·`d3dutils.cpp`의 이름 표에만 있다.

| D3DFMT | 사용 | DXGI_FORMAT | 비고 |
|---|---:|---|---|
| `A8R8G8B8` | 31 | `B8G8R8A8_UNORM` | 기본 렌더타깃·텍스처. sRGB 변형 `B8G8R8A8_UNORM_SRGB` |
| `X8R8G8B8` | 18 | `B8G8R8X8_UNORM` | 불투명 백버퍼 |
| `A2R10G10B10` | 9 | `R10G10B10A2_UNORM` | **채널 순서가 다르다**(BGR→RGB). 10비트 출력에 쓰면 스위즐 필요. 사용처 확인 후 결정 |
| `A32B32G32R32F` | 7 | `R32G32B32A32_FLOAT` | 효과 중간 버퍼 (`Is128BitFPTextureSupported`) |
| `L8` | 8 | `R8_UNORM` | 셰이더에서 `.r`을 복제 |
| `A8` | 3 | `A8_UNORM` | 글리프 아틀라스 |
| `P8`, `A8P8` | 11 | 없음 | 팔레트. CPU에서 BGRA로 펼친다 |
| `R8G8B8` | 8 | 없음 | 24bpp. CPU에서 BGRX로 |
| `R5G6B5`, `X1R5G5B5`, `A1R5G5B5` | 14 | `B5G6R5_UNORM`, `B5G5R5A1_UNORM` | 16bpp 소스 비트맵. 지원 여부는 `CheckFormatSupport` |
| `D24S8`, `D16` | 5 | `D24_UNORM_S8_UINT`, `D16_UNORM` | 깊이·스텐실 |
| `INDEX16` | 3 | `R16_UINT` | 인덱스 버퍼 |
| `DXT1..5` | 10 | `BC1_UNORM`, `BC2_UNORM`, `BC3_UNORM` | 압축 텍스처 지원 질의에만 |
| `UNKNOWN` | 12 | `DXGI_FORMAT_UNKNOWN` | |

### 4.2 렌더 상태 (`D3DRS_` 41종 사용 → 상태 객체)

| 그룹 | D3DRS | D3D11 |
|---|---|---|
| 블렌드 | `ALPHABLENDENABLE`, `SRCBLEND`, `DESTBLEND`, `BLENDOP`, `BLENDFACTOR`, `COLORWRITEENABLE`, `SEPARATEALPHABLENDENABLE`, `SRCBLENDALPHA`, `DESTBLENDALPHA`, `BLENDOPALPHA` | `D3D11_BLEND_DESC.RenderTarget[0]` + `OMSetBlendState(blendFactor, sampleMask)` |
| 깊이·스텐실 | `ZENABLE`, `ZWRITEENABLE`, `ZFUNC`, `STENCILENABLE`, `STENCILFUNC`, `STENCILREF`, `STENCILMASK`, `STENCILWRITEMASK`, `STENCILFAIL`, `STENCILZFAIL`, `STENCILPASS`, `TWOSIDEDSTENCILMODE` | `D3D11_DEPTH_STENCIL_DESC` + `OMSetDepthStencilState(ref)` |
| 래스터라이저 | `FILLMODE`, `CULLMODE`, `SCISSORTESTENABLE`, `MULTISAMPLEANTIALIAS`, `ANTIALIASEDLINEENABLE`, `DEPTHBIAS`, `SLOPESCALEDEPTHBIAS` | `D3D11_RASTERIZER_DESC` |
| 샘플 마스크 | `MULTISAMPLEMASK` | `OMSetBlendState`의 sampleMask |
| sRGB | `SRGBWRITEENABLE` | RTV를 `_SRGB` 포맷으로 생성 |
| 알파 테스트 | `ALPHATESTENABLE`, `ALPHAREF`, `ALPHAFUNC` | 셰이더 `clip()` |
| 고정 기능 전용 (삭제) | `LIGHTING`, `AMBIENT`, `DIFFUSEMATERIALSOURCE`, `SPECULARMATERIALSOURCE`, `AMBIENTMATERIALSOURCE`, `SPECULARENABLE`, `NORMALIZENORMALS`, `COLORVERTEX`, `SHADEMODE`, `FOGENABLE`, `CLIPPING`, `CLIPPLANEENABLE`, `LASTPIXEL`, `DITHERENABLE`, `TEXTUREFACTOR`, `WRAP0` | 셰이더가 흡수 |

### 4.3 그 밖의 열거형

| 가족 | 사용 값 | D3D11 |
|---|---|---|
| `D3DPT_` | `TRIANGLELIST`, `TRIANGLESTRIP`, `TRIANGLEFAN`, `LINELIST` | `D3D11_PRIMITIVE_TOPOLOGY_*`. FAN은 인덱스 변환 |
| `D3DBLEND_` | `ONE`, `ZERO`, `SRCALPHA`, `INVSRCALPHA`, `INVSRCCOLOR`, `BLENDFACTOR`, `INVDESTALPHA`, `INVDESTCOLOR` | `D3D11_BLEND_*` 1:1 (`BLENDFACTOR`→`BLEND_FACTOR`) |
| `D3DCMP_` | `LESSEQUAL`, `ALWAYS`, `NOTEQUAL` | `D3D11_COMPARISON_*` |
| `D3DCULL_` | `CW`, `CCW`, `NONE` | `D3D11_CULL_FRONT/BACK/NONE` + `FrontCounterClockwise` |
| `D3DFILL_` | `SOLID`, `WIREFRAME` | `D3D11_FILL_*` |
| `D3DSTENCILOP_` | `KEEP` | `D3D11_STENCIL_OP_KEEP` |
| `D3DTEXF_` | `NONE`, `LINEAR`, `POINT`, `ANISOTROPIC`, `CONVOLUTIONMONO` | `D3D11_FILTER_*` 조합. `CONVOLUTIONMONO`(ClearType용 D3D9Ex 전용)는 셰이더로 |
| `D3DTADDRESS_` | `CLAMP`, `WRAP`, `BORDER`, `MIRROR` | `D3D11_TEXTURE_ADDRESS_*` 1:1 |
| `D3DUSAGE_` | `RENDERTARGET`, `DEPTHSTENCIL`, `DYNAMIC`, `WRITEONLY`, `AUTOGENMIPMAP` | `BindFlags`, `D3D11_USAGE_DYNAMIC`, `MISC_GENERATE_MIPS` |
| `D3DPOOL_` | `DEFAULT`, `SYSTEMMEM`, `MANAGED`, `MANAGED_INTERNAL`, `SCRATCH` | `DEFAULT`→DEFAULT, `SYSTEMMEM`/`SCRATCH`→STAGING, `MANAGED`→DEFAULT(메모리 관리 없음) |
| `D3DLOCK_` | `READONLY`, `NO_DIRTY_UPDATE`, `DISCARD`, `NOOVERWRITE` | `D3D11_MAP_READ`, `WRITE_DISCARD`, `WRITE_NO_OVERWRITE` |
| `D3DCLEAR_` | `TARGET`, `ZBUFFER`, `STENCIL` | `ClearRenderTargetView`, `ClearDepthStencilView(DEPTH\|STENCIL)` |
| `D3DSWAPEFFECT_` | `COPY`, `DISCARD` | `DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL` + dirty rect |
| `D3DPRESENT_INTERVAL_` | `IMMEDIATE`, `ONE` | `Present1(0 또는 1, …)` |
| `D3DMULTISAMPLE_` | `NONE`, `2`, `4`, `16` | `DXGI_SAMPLE_DESC{Count, Quality}` + `CheckMultisampleQualityLevels` |
| `D3DDEVTYPE_` | `HAL`, `SW`, `REF` | 자체 `DeviceKind { Hardware, Software, Reference }` |
| `D3DCREATE_` | `MULTITHREADED`, `FPU_PRESERVE`, `PUREDEVICE`, `DISABLE_DRIVER_MANAGEMENT_EX`, `SOFTWARE_VERTEXPROCESSING` 등 | 없음. FPU 보존은 D3D11 기본 |
| `D3DFVF_`, `D3DDECLUSAGE_`, `D3DDECLTYPE_` | `XYZ`, `DIFFUSE`, `SPECULAR`, `NORMAL`, `TEX1..8`; `POSITION`, `COLOR`, `TEXCOORD`; `FLOAT2/3`, `D3DCOLOR` | `D3D11_INPUT_ELEMENT_DESC` (POSITION R32G32B32, COLOR B8G8R8A8_UNORM, TEXCOORDn R32G32) |
| `D3DRTYPE_` | `TEXTURE`, `SURFACE` | `D3D11_RESOURCE_DIMENSION_TEXTURE2D` |
| `D3DQUERYTYPE_` | `EVENT`, `DDISTATS`, `RESOURCEMANAGER`, `VERTEXSTATS` | `D3D11_QUERY_EVENT`. 통계 쿼리는 삭제 |
| `D3DTSS_`, `D3DTOP_`, `D3DTA_`, `D3DTTFF_` | 전부 | 없음 (§3 텍스처 스테이지) |

## 5. 셰이더 자산

| 자산 | 현재 | 목표 |
|---|---|---|
| 내장 효과 (`core/hw/ShaderAssemblies/Source/*.fx`: `ShaderEffectsVS`, `PassThroughShaderEffectPS`, `HorizontalBlur[Multi]`, `VerticalBlur[Multi]`, `DropShadow`) | 빌드 때 `fxc`로 `vs_2_0`/`vs_3_0`/`ps_2_0` 컴파일 → 헤더(`ShaderAssemblies.targets`) | 같은 소스를 `vs_4_0`/`ps_4_0`으로. HLSL은 `tex2D`→`Sample`, 시맨틱 정리 |
| 텍스트 셰이더 (`core/hw/shaders/hlslTextShaders{11,20}{A,L}.fx`) | 일회성 도구 ShaderGen.exe가 만든 ps_1_1/ps_2_0 바이트코드 22개가 `Shaders.rc`에 RCDATA로 박혀 있음(ClearType/그레이스케일 × 단색/텍스처 브러시 × 패스) | `hlslTextShaders20*.fx`만 `ps_4_0`으로 빌드 때 컴파일. ps_1_1 변종과 ShaderGen 삭제 |
| 파이프라인 셰이더 (`hwshaderfragmenttohlslconverter.cpp`, `HwHLSLShaderFragments.cpp`, `HwShaderBuilder.cpp`) | 브러시·마스크 조합별로 런타임에 HLSL 조립 → `D3DCompile(ps_2_0/ps_3_0, vs_2_0/vs_3_0)` → 캐시 | 조각 템플릿을 SM4 문법으로, 프로필 `ps_4_0`/`vs_4_0`, 상수는 `cbuffer`로 |
| 고정 기능 폴백 (`hwffshaders.cpp`, `HwFFPipeline`) | 셰이더 없는 하드웨어용 | 삭제 (D4) |
| ShaderEffect (앱 바이트코드) | ps_2_0/ps_3_0 DXBC 이전 포맷 | D6 변환기 |

## 6. 관리 코드가 네이티브에 묻는 값

### 6.1 caps와 Tier (`core/common/display.cpp`, `Tier.cpp`)

`MilGraphicsAccelerationCaps { TierValue, HasWDDMSupport, PixelShaderVersion, VertexShaderVersion, MaxTextureWidth, MaxTextureHeight,
WindowCompatibleMode, BitsPerPixel, HasSSE2Support, MaxPixelShader30InstructionSlots }`가 `MILFactory...GetGraphicsAccelerationCaps` export를
통해 관리 코드의 `RenderCapability`로 간다. `TierValue`는 `MIL_TIER(2,0)`가 "전체 하드웨어", `PixelShaderVersion`은 `D3DPS_VERSION(3,0)` 형식의
DWORD(상위 워드 0xFFFF, 하위 메이저·마이너)다. **형식은 그대로 두고 값만 Feature Level에서 만든다**: FL10.0 이상 → Tier 2, PS 3.0, VS 3.0,
최대 텍스처 8192(FL11은 16384), 명령 슬롯 512. 디바이스 생성 실패 → Tier 0.

### 6.2 오류 코드

`core/uce`, `core/api`, `core/sw`, `core/hw`가 `D3DERR_OUTOFVIDEOMEMORY`(장애 통지 사유), `D3DERR_NOTAVAILABLE`(무시),
`D3DERR_DEVICELOST`/`DEVICEHUNG`/`DEVICEREMOVED`(디바이스 재생성), `D3DERR_DRIVERINTERNALERROR`, `D3DERR_INVALIDCALL`로 분기한다.
D3D11의 `DXGI_ERROR_DEVICE_REMOVED/DEVICE_RESET/DEVICE_HUNG`, `E_OUTOFMEMORY`를 **기존 D3DERR 값으로 되돌려 매핑**해서 상위 분기와
관리 코드에 보이는 실패 사유(`UCEERR_*`)를 유지한다. `D3DERR_*` 상수 자체는 값만 정의하면 `d3d9.h` 없이도 쓸 수 있다.

## 7. core/hw 파일별 체크리스트

치환 때 손대는 순서의 기준. 줄 수와 D3D9 참조 수(인터페이스/메서드/열거형)는 `inventory/summary.md`.

| 역할 | 파일 | 줄 | 비고 |
|---|---|---:|---|
| 디바이스 래퍼 | `d3ddevice.cpp/.h` | 7,866 | 핵심. 48/81/183 참조 |
| 디바이스 관리 | `d3ddevicemanager.cpp`, `D3DDeviceManager.h` | 2,081 | 어댑터별 디바이스, 생성 플래그 |
| 상태 캐시 | `d3drenderstate.cpp/.h`, `HwRenderStateManager.cpp/.h`, `HwStateTable.cpp/.h` | 6,181 | 상태 객체 캐시로 재작성 |
| 리소스 기반 | `d3dresource.cpp/.h` | 1,493+ | D3D9 참조 거의 없음. 유지 |
| 서피스·텍스처 | `d3dsurface.cpp/.h`, `d3dtexture.cpp/.h`, `d3dlockabletexture.cpp/.h`, `d3dvidmemonlytexture.cpp/.h`, `D3DTextureSurface.cpp/.h`, `HwVidMemTextureManager.cpp/.h` | 2,733 | Texture2D + 뷰 |
| 스왑체인·프레젠트 | `d3dswapchain.cpp/.h`, `d3dswapchainwithswdc.cpp/.h`, `hwdisplayrt.cpp/.h`, `hwhwndrt.cpp/.h` | 2,843 | DXGI |
| 렌더타깃 | `hwsurfrt.cpp/.h`, `hwsurfrtdata.cpp`, `hwtexturert.cpp/.h`, `hwdestinationtexture.cpp` | 6,469 | RTV/DSV, 클리어, 복사 |
| 버텍스 | `d3dvertex.h`, `hwvertexbuffer.cpp`, `HwVertexBuffer.h`, `hw3dgeometrystreambuffer.cpp`, `Hw3DGeometryStreamBuffer.h` | 6,627 | 입력 레이아웃, 동적 버퍼 |
| 셰이더 | `hwshader.cpp`, `HwShaderPipeline.cpp/.h`, `HwShaderBuilder.cpp`, `HwShaderEffect.cpp/.h`, `hwshaderfragmenttohlslconverter.cpp`, `HwHLSLShaderFragments.cpp/.h`, `hwpipeline.cpp/.h`, `hwpipelinebuilder.cpp`, `HwPipelineBuilder.h` | 9,000+ | SM4 전환, FF 삭제 |
| 고정 기능 | `hwffshaders.cpp` | 512 | 삭제 |
| 컬러 소스 | `hwbitmapcolorsource.cpp`, `HwBitmapColorSource.h`, `hwdevicebitmapcolorsource.cpp`, `HwDeviceBitmapColorSource.h`, `HwBitBltDeviceBitmapColorSource.cpp/.h`, `hwtexturedcolorsource.cpp`, `HwTexturedColorSource.h`, `hwsolidcolortexturesource.cpp`, `hwlineargradientcolorsource.cpp`, `HwBoxColorSource.cpp`, `HwBitmapCache.cpp` | 10,000+ | 텍스처 생성·업로드, 샘플러, 텍스처 스테이지 제거 |
| 글리프 | `d3dglyphbank.cpp/.h`, `d3dglyphpainter.h`, `d3dglyphrun.h` | 1,578 | 아틀라스 업로드 |
| interop | `InteropDeviceBitmap.cpp/.h` | 1,305 | D7 |
| 폴백 | `swfallback.cpp`, `hwsw3dfallback.cpp` | 1,552 | 스테이징 경로 |
| 통계·마커 | `d3dstats.cpp/.h`, `gpumarker.cpp/.h`, `d3dregistry.cpp/.h` | 1,058 | 쿼리 1:1, 레지스트리 키 이름 유지 |
| 유틸·caps | `HwUtils.cpp/.h`, `HwCaps.h` | 461 | |
| 셰이더 자산 | `ShaderAssemblies/*`, `shaders/*`, `Shaders.rc` | — | §5 |

## 8. 치환 순서에 대한 함의

1. `include/std.h`의 `d3d9.h`를 빼는 순간 전체가 깨지므로, 먼저 **자체 타입**(`DXGI_FORMAT` 별칭, `DeviceKind`, 토폴로지, 오류 코드 상수)을
   도입해 래퍼 바깥의 D3D9 타입 참조를 그것으로 바꾼다. 이 단계는 D3D9를 그대로 둔 채 빌드가 되어야 한다.
2. 그다음 `CD3DDeviceLevel1`과 `CD3DResource` 계열, 상태 캐시, 스왑체인을 D3D11로 재작성한다. 여기가 WIP 브랜치에서 빌드가 깨져 있을 구간이다.
3. 셰이더 자산과 HLSL 조립기를 SM4로 바꾼 뒤 스모크 1번 셀(단색)부터 복구한다.
4. interop, 미디어, 폴백은 각각 독립적으로 뒤따른다.
