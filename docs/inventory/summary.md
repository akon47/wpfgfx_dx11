# D3D9 사용 집계 (자동 생성)

`tools/inventory/d3d9_inventory.py`가 만든다. 손으로 고치지 말 것. 해석과 D3D11 매핑은 `docs/INVENTORY.md`.

대상: WpfGfx 아래 소스 1134개 파일. D3D9를 참조하는 파일 155개. 사설 헤더 복사본 2개(core/fxjit/PixelShader/d3d9caps.h, core/fxjit/PixelShader/d3d9types.h)는 정의이므로 제외.


## 디렉터리별 밀도

| 디렉터리 | 파일 수 | 줄 수 | 인터페이스 참조 | 메서드 호출 | 열거형 사용 | 구조체 |
|---|---:|---:|---:|---:|---:|---:|
| `core/hw` | 82 | 55805 | 446 | 332 | 909 | 165 |
| `core/fxjit` | 5 | 9921 | 0 | 0 | 595 | 0 |
| `core/av` | 19 | 9922 | 103 | 26 | 28 | 27 |
| `core/common` | 10 | 5746 | 36 | 11 | 57 | 26 |
| `core/resources` | 10 | 6958 | 0 | 35 | 35 | 4 |
| `common/DirectXLayer` | 6 | 1379 | 8 | 2 | 23 | 20 |
| `common/shared` | 2 | 2087 | 0 | 0 | 30 | 6 |
| `core/uce` | 7 | 13699 | 0 | 3 | 16 | 3 |
| `core/api` | 7 | 3605 | 0 | 1 | 3 | 6 |
| `core/meta` | 1 | 1396 | 0 | 1 | 3 | 0 |
| `core/sw` | 1 | 2258 | 0 | 0 | 2 | 0 |
| `include` | 5 | 6027 | 1 | 0 | 0 | 11 |

## 파일별 밀도 (참조 합계 내림차순)

| 파일 | 줄 수 | 인터페이스 | 메서드 | 열거형 | 구조체 | 주요 인터페이스 |
|---|---:|---:|---:|---:|---:|---|
| `core/fxjit/PixelShader/pstrans.cpp` | 4777 | 0 | 0 | 435 | 0 |  |
| `core/hw/d3ddevice.cpp` | 6695 | 48 | 81 | 183 | 14 | IDirect3DSurface9(15), IDirect3DTexture9(9), IDirect3D9(5) |
| `core/hw/d3drenderstate.cpp` | 2048 | 3 | 16 | 283 | 0 | IDirect3DSurface9(2), IDirect3DDevice9(1) |
| `core/hw/HwRenderStateManager.cpp` | 2088 | 9 | 20 | 94 | 3 | IDirect3DDevice9(2), IDirect3DSurface9(2), IDirect3DBaseTexture9(1) |
| `core/av/d3ddevicewrapper.h` | 840 | 63 | 0 | 7 | 19 | IDirect3DSurface9(24), IDirect3DVertexBuffer9(6), IDirect3DDevice9(4) |
| `core/hw/shaders/ShaderGen/Device.h` | 185 | 58 | 0 | 7 | 19 | IDirect3DSurface9(23), IDirect3DVertexBuffer9(6), IDirect3DBaseTexture9(4) |
| `core/hw/d3ddevice.h` | 1171 | 42 | 1 | 28 | 12 | IDirect3DSurface9(11), IDirect3DTexture9(7), IDirect3DVertexShader9(5) |
| `core/hw/d3ddevicemanager.cpp` | 1880 | 6 | 9 | 51 | 15 | IDirect3DDevice9(2), IDirect3D9(2), IDirect3DDevice9Ex(1) |
| `core/fxjit/Public/typedefs.h` | 1380 | 0 | 0 | 67 | 0 |  |
| `core/hw/shaders/ShaderGen/Device.cpp` | 277 | 56 | 0 | 4 | 0 | IDirect3DSurface9(23), IDirect3DVertexBuffer9(6), IDirect3DVertexDeclaration9(4) |
| `core/hw/hwsurfrt.cpp` | 3471 | 2 | 25 | 28 | 3 | IDirect3DTexture9(1), IDirect3DSurface9(1) |
| `core/fxjit/PixelShader/d3dhal.h` | 66 | 0 | 0 | 49 | 0 |  |
| `core/hw/hwbitmapcolorsource.cpp` | 4753 | 8 | 9 | 22 | 8 | IDirect3DSurface9(7), IDirect3DTexture9(1) |
| `core/common/d3dutils.cpp` | 319 | 2 | 2 | 34 | 4 | IDirect3DDevice9(1), IDirect3D9(1) |
| `core/fxjit/PixelShader/pstrans.hpp` | 1278 | 0 | 0 | 41 | 0 |  |
| `core/hw/InteropDeviceBitmap.cpp` | 1163 | 11 | 13 | 14 | 3 | IDirect3DSurface9(4), IDirect3DDevice9(3), IDirect3DDevice9Ex(3) |
| `core/hw/hwdevicebitmapcolorsource.cpp` | 852 | 9 | 14 | 10 | 8 | IDirect3DSurface9(6), IDirect3DDevice9(2), IDirect3DTexture9(1) |
| `core/common/display.cpp` | 3011 | 7 | 8 | 16 | 6 | IDirect3D9(5), IDirect3D9Ex(2) |
| `common/shared/pixelformatutils.cpp` | 1064 | 0 | 0 | 30 | 3 |  |
| `common/DirectXLayer/XMath/shader_compiler_xm.hpp` | 293 | 4 | 2 | 22 | 2 | IDirect3DDevice9(4) |
| `core/hw/d3dglyphbank.cpp` | 1028 | 13 | 8 | 6 | 3 | IDirect3DSurface9(9), IDirect3DTexture9(4) |
| `core/hw/hwdisplayrt.cpp` | 1328 | 2 | 10 | 13 | 5 | IDirect3DSurface9(1), IDirect3DDevice9(1) |
| `core/hw/hwtexturedcolorsource.cpp` | 565 | 0 | 7 | 21 | 0 |  |
| `core/hw/HwRenderStateManager.h` | 810 | 23 | 1 | 1 | 1 | IDirect3DSurface9(4), IDirect3DDevice9(3), IDirect3DBaseTexture9(3) |
| `core/hw/HwUtils.cpp` | 354 | 8 | 9 | 6 | 2 | IDirect3DSurface9(3), IDirect3DTexture9(2), IDirect3DDevice9Ex(2) |
| `core/resources/Effect.cpp` | 349 | 0 | 8 | 16 | 1 |  |
| `core/hw/d3drenderstate.h` | 620 | 7 | 12 | 4 | 0 | IDirect3DPixelShader9(2), IDirect3DDevice9(1), IDirect3DBaseTexture9(1) |
| `core/hw/d3dsurface.cpp` | 488 | 6 | 6 | 10 | 1 | IDirect3DSurface9(5), IDirect3DDevice9(1) |
| `core/av/hwmediabuffer.cpp` | 522 | 3 | 9 | 5 | 4 | IDirect3DDevice9(3) |
| `core/hw/hw3dgeometrystreambuffer.cpp` | 1473 | 0 | 6 | 15 | 0 |  |
| `core/resources/BlurEffect.cpp` | 2247 | 0 | 12 | 9 | 0 |  |
| `core/hw/d3dvertex.h` | 329 | 0 | 0 | 14 | 4 |  |
| `core/hw/HwBitBltDeviceBitmapColorSource.cpp` | 322 | 3 | 8 | 4 | 2 | IDirect3DSurface9(3) |
| `core/av/evrpresenter.cpp` | 3744 | 5 | 2 | 8 | 1 | IDirect3DDeviceManager9(4), IDirect3DDevice9(1) |
| `core/common/d3dloader.cpp` | 749 | 13 | 1 | 1 | 0 | IDirect3D9(8), IDirect3D9Ex(5) |
| `core/hw/D3DDeviceManager.h` | 201 | 2 | 0 | 4 | 8 | IDirect3D9(1), IDirect3DDevice9(1) |
| `core/hw/HwBitmapColorSource.h` | 768 | 8 | 0 | 1 | 4 | IDirect3DSurface9(6), IDirect3DTexture9(2) |
| `core/hw/d3dstats.cpp` | 340 | 4 | 3 | 6 | 0 | IDirect3DDevice9(3), IDirect3DQuery9(1) |
| `core/hw/d3dtexture.cpp` | 355 | 3 | 5 | 4 | 1 | IDirect3DTexture9(2), IDirect3DSurface9(1) |
| `core/hw/hwsolidcolortexturesource.cpp` | 293 | 0 | 3 | 5 | 5 |  |
| `core/av/dxvamanagerwrapper.cpp` | 332 | 5 | 7 | 0 | 0 | IDirect3DDeviceManager9(3), IDirect3DDevice9(2) |
| `core/common/Display.h` | 634 | 10 | 0 | 0 | 2 | IDirect3D9(6), IDirect3D9Ex(4) |
| `core/hw/HwStateTable.h` | 576 | 12 | 0 | 0 | 0 | IDirect3DBaseTexture9(2), IDirect3DVertexShader9(2), IDirect3DPixelShader9(2) |
| `core/hw/d3dlockabletexture.cpp` | 442 | 2 | 5 | 2 | 3 | IDirect3DTexture9(2) |
| `core/hw/d3dswapchain.cpp` | 378 | 5 | 4 | 2 | 1 | IDirect3DSwapChain9(3), IDirect3DSurface9(1), IDirect3DSwapChain9Ex(1) |
| `core/hw/hwdestinationtexture.cpp` | 737 | 2 | 3 | 4 | 3 | IDirect3DSurface9(1), IDirect3DTexture9(1) |
| `core/hw/HwShaderPipeline.cpp` | 634 | 4 | 7 | 0 | 0 | IDirect3DVertexShader9(2), IDirect3DPixelShader9(2) |
| `core/hw/d3dsurface.h` | 123 | 5 | 3 | 0 | 3 | IDirect3DSurface9(5) |
| `core/hw/hwsw3dfallback.cpp` | 715 | 0 | 5 | 4 | 2 |  |
| `core/common/Tier.cpp` | 205 | 0 | 0 | 4 | 6 |  |
| `core/hw/HwVidMemTextureManager.cpp` | 395 | 2 | 3 | 4 | 1 | IDirect3DSurface9(2) |
| `core/hw/d3dglyphbank.h` | 299 | 10 | 0 | 0 | 0 | IDirect3DSurface9(7), IDirect3DTexture9(3) |
| `core/hw/d3dtexture.h` | 118 | 7 | 1 | 0 | 2 | IDirect3DTexture9(5), IDirect3DBaseTexture9(1), IDirect3DSurface9(1) |
| `core/hw/hwsurfrtdata.cpp` | 901 | 2 | 7 | 1 | 0 | IDirect3DPixelShader9(1), IDirect3DVertexShader9(1) |
| `core/hw/swfallback.cpp` | 837 | 0 | 0 | 8 | 2 |  |
| `core/hw/HwCaps.h` | 65 | 0 | 0 | 4 | 5 |  |
| `core/av/swmediabuffer.cpp` | 351 | 1 | 5 | 1 | 1 | IDirect3DDevice9(1) |
| `core/hw/HwDeviceBitmapColorSource.h` | 191 | 5 | 0 | 0 | 3 | IDirect3DSurface9(4), IDirect3DDevice9(1) |
| `core/hw/hwhwndrt.cpp` | 358 | 0 | 2 | 4 | 2 |  |
| `core/hw/hwtexturert.cpp` | 705 | 0 | 0 | 6 | 2 |  |
| `core/av/fakepp.cpp` | 1244 | 2 | 1 | 3 | 1 | IDirect3DDevice9(1), IDirect3DSurface9(1) |
| `core/hw/HwVertexBuffer.h` | 1355 | 0 | 4 | 3 | 0 |  |
| `core/hw/d3dvidmemonlytexture.cpp` | 179 | 3 | 1 | 2 | 1 | IDirect3DTexture9(3) |
| `core/hw/hwlineargradientcolorsource.cpp` | 470 | 0 | 1 | 5 | 1 |  |
| `core/resources/DropShadowEffect.cpp` | 1050 | 0 | 6 | 1 | 0 |  |
| `core/uce/rendertargetmanager.cpp` | 1367 | 0 | 1 | 6 | 0 |  |
| `common/DirectXLayer/XMath/base/color_base_t.hpp` | 225 | 0 | 0 | 0 | 6 |  |
| `common/DirectXLayer/XMath/base/matrix_base_t.hpp` | 334 | 0 | 0 | 0 | 6 |  |
| `core/common/d3dutils.h` | 109 | 1 | 0 | 2 | 3 | IDirect3DDevice9(1) |
| `core/hw/HwShaderPipeline.h` | 141 | 6 | 0 | 0 | 0 | IDirect3DVertexShader9(3), IDirect3DPixelShader9(3) |
| `core/hw/HwStateTable.cpp` | 39 | 6 | 0 | 0 | 0 | IDirect3DBaseTexture9(1), IDirect3DVertexShader9(1), IDirect3DPixelShader9(1) |
| `core/hw/d3dlockabletexture.h` | 184 | 2 | 2 | 0 | 2 | IDirect3DTexture9(2) |
| `core/hw/hwffshaders.cpp` | 512 | 0 | 3 | 3 | 0 |  |
| `core/resources/ShaderEffect.cpp` | 1274 | 0 | 6 | 0 | 0 |  |
| `common/DirectXLayer/D3DX9/dx9layercommon.hpp` | 28 | 0 | 0 | 0 | 5 |  |
| `common/DirectXLayer/D3DX9/shader_compiler_dx9.hpp` | 198 | 4 | 0 | 1 | 0 | IDirect3DDevice9(4) |
| `core/av/dxvamanagerwrapper.h` | 95 | 5 | 0 | 0 | 0 | IDirect3DDevice9(3), IDirect3DDeviceManager9(2) |
| `core/av/mediabuffer.cpp` | 418 | 1 | 2 | 2 | 0 | IDirect3DSurface9(1) |
| `core/hw/HwBoxColorSource.cpp` | 280 | 0 | 1 | 3 | 1 |  |
| `core/hw/InteropDeviceBitmap.h` | 142 | 5 | 0 | 0 | 0 | IDirect3DSurface9(3), IDirect3DDevice9(1), IDirect3DDevice9Ex(1) |
| `core/hw/d3dswapchainwithswdc.cpp` | 218 | 1 | 0 | 2 | 2 | IDirect3DSwapChain9(1) |
| `core/hw/hwpipeline.cpp` | 858 | 0 | 3 | 2 | 0 |  |
| `core/resources/modelrenderwalker.cpp` | 464 | 0 | 0 | 5 | 0 |  |
| `include/processed/wgx_core_types.h` | 3378 | 0 | 0 | 0 | 5 |  |
| `core/av/avloader.cpp` | 901 | 4 | 0 | 0 | 0 | IDirect3DDeviceManager9(4) |
| `core/av/hwmediabuffer.h` | 123 | 3 | 0 | 0 | 1 | IDirect3DSurface9(2), IDirect3DTexture9(1) |
| `core/av/util.cpp` | 203 | 4 | 0 | 0 | 0 | IDirect3DDevice9(4) |
| `core/hw/D3DTextureSurface.cpp` | 137 | 3 | 1 | 0 | 0 | IDirect3DSurface9(3) |
| `core/hw/Hw3DGeometryStreamBuffer.h` | 424 | 4 | 0 | 0 | 0 | IDirect3DVertexBuffer9(2), IDirect3DIndexBuffer9(2) |
| `core/hw/d3dstats.h` | 101 | 3 | 0 | 1 | 0 | IDirect3DDevice9(3) |
| `core/hw/d3dswapchain.h` | 134 | 4 | 0 | 0 | 0 | IDirect3DSwapChain9(3), IDirect3DSwapChain9Ex(1) |
| `core/hw/gpumarker.cpp` | 144 | 1 | 2 | 1 | 0 | IDirect3DQuery9(1) |
| `core/hw/hwdisplayrt.h` | 240 | 0 | 0 | 2 | 2 |  |
| `core/hw/hwsurfrt.h` | 466 | 1 | 0 | 3 | 0 | IDirect3DTexture9(1) |
| `core/hw/hwvertexbuffer.cpp` | 3046 | 0 | 4 | 0 | 0 |  |
| `core/meta/desktoprt.cpp` | 1396 | 0 | 1 | 3 | 0 |  |
| `common/shared/pixelformatutils.h` | 1023 | 0 | 0 | 0 | 3 |  |
| `core/common/D3DLoader.h` | 116 | 3 | 0 | 0 | 0 | IDirect3D9(2), IDirect3D9Ex(1) |
| `core/fxjit/PixelShader/pshader.cpp` | 2420 | 0 | 0 | 3 | 0 |  |
| `core/hw/d3dregistry.cpp` | 336 | 2 | 1 | 0 | 0 | IDirect3D9(2) |
| `core/hw/d3dvidmemonlytexture.h` | 62 | 2 | 0 | 0 | 1 | IDirect3DTexture9(2) |
| `core/resources/data_generated.h` | 619 | 0 | 0 | 0 | 3 |  |
| `core/uce/apifunc.cpp` | 1068 | 0 | 0 | 0 | 3 |  |
| `core/uce/composition.cpp` | 2507 | 0 | 1 | 2 | 0 |  |
| `core/uce/drawingcontext.cpp` | 6118 | 0 | 0 | 3 | 0 |  |
| `include/Generated/wgx_commands.h` | 1161 | 0 | 0 | 0 | 3 |  |
| `core/api/api_lightdata.cpp` | 1057 | 0 | 0 | 0 | 2 |  |
| `core/api/api_lightdata.h` | 195 | 0 | 0 | 0 | 2 |  |
| `core/api/api_rendercontext.h` | 252 | 0 | 0 | 2 | 0 |  |
| `core/av/EvrPresenter.inl` | 141 | 0 | 0 | 2 | 0 |  |
| `core/common/BaseMatrix.h` | 346 | 0 | 0 | 0 | 2 |  |
| `core/common/MILMatrix.h` | 225 | 0 | 0 | 0 | 2 |  |
| `core/hw/D3DTextureSurface.h` | 55 | 2 | 0 | 0 | 0 | IDirect3DSurface9(2) |
| `core/hw/HwBitBltDeviceBitmapColorSource.h` | 58 | 1 | 0 | 0 | 1 | IDirect3DSurface9(1) |
| `core/hw/HwBitmapCache.cpp` | 1413 | 0 | 0 | 2 | 0 |  |
| `core/hw/HwShaderEffect.cpp` | 125 | 0 | 2 | 0 | 0 |  |
| `core/hw/HwUtils.h` | 42 | 2 | 0 | 0 | 0 | IDirect3DTexture9(1), IDirect3DSurface9(1) |
| `core/hw/HwVidMemTextureManager.h` | 95 | 0 | 0 | 0 | 2 |  |
| `core/hw/d3dregistry.h` | 83 | 2 | 0 | 0 | 0 | IDirect3D9(2) |
| `core/hw/d3dswapchainwithswdc.h` | 74 | 2 | 0 | 0 | 0 | IDirect3DSwapChain9(2) |
| `core/hw/gpumarker.h` | 54 | 2 | 0 | 0 | 0 | IDirect3DQuery9(2) |
| `core/hw/hwhwndrt.h` | 113 | 0 | 0 | 1 | 1 |  |
| `core/resources/PixelShader.cpp` | 407 | 0 | 0 | 2 | 0 |  |
| `core/resources/prerenderwalker.cpp` | 216 | 0 | 2 | 0 | 0 |  |
| `core/resources/specularmaterial.cpp` | 169 | 0 | 0 | 2 | 0 |  |
| `core/sw/swlib/swsurfrt.cpp` | 2258 | 0 | 0 | 2 | 0 |  |
| `core/uce/hwndtarget.cpp` | 1334 | 0 | 1 | 1 | 0 |  |
| `core/uce/partitionmanager.cpp` | 1076 | 0 | 0 | 2 | 0 |  |
| `core/uce/render3dcontext.cpp` | 229 | 0 | 0 | 2 | 0 |  |
| `include/std.h` | 434 | 0 | 0 | 0 | 2 |  |
| `common/DirectXLayer/XMath/matrix_xm.hpp` | 301 | 0 | 0 | 0 | 1 |  |
| `core/api/api_factory.cpp` | 767 | 0 | 0 | 1 | 0 |  |
| `core/api/api_renderstate.cpp` | 168 | 0 | 0 | 0 | 1 |  |
| `core/api/api_renderstate.h` | 145 | 0 | 0 | 0 | 1 |  |
| `core/api/exports.cpp` | 1021 | 0 | 1 | 0 | 0 |  |
| `core/av/DummyPlayer.cpp` | 307 | 1 | 0 | 0 | 0 | IDirect3DDevice9(1) |
| `core/av/DummyPlayer.h` | 138 | 1 | 0 | 0 | 0 | IDirect3DDevice9(1) |
| `core/av/avloader.h` | 76 | 1 | 0 | 0 | 0 | IDirect3DDeviceManager9(1) |
| `core/av/internal.h` | 112 | 1 | 0 | 0 | 0 | IDirect3DDevice9(1) |
| `core/av/mediabuffer.h` | 182 | 1 | 0 | 0 | 0 | IDirect3DSurface9(1) |
| `core/av/swmediabuffer.h` | 97 | 1 | 0 | 0 | 0 | IDirect3DSurface9(1) |
| `core/av/util.h` | 96 | 1 | 0 | 0 | 0 | IDirect3DDevice9(1) |
| `core/common/Tier.h` | 32 | 0 | 0 | 0 | 1 |  |
| `core/hw/HwShaderEffect.h` | 142 | 1 | 0 | 0 | 0 | IDirect3DPixelShader9(1) |
| `core/hw/HwTexturedColorSource.h` | 236 | 0 | 0 | 1 | 0 |  |
| `core/hw/d3dglyphpainter.h` | 128 | 1 | 0 | 0 | 0 | IDirect3DTexture9(1) |
| `core/hw/d3dglyphrun.h` | 123 | 1 | 0 | 0 | 0 | IDirect3DTexture9(1) |
| `core/hw/d3dresource.cpp` | 1493 | 0 | 0 | 1 | 0 |  |
| `core/hw/hwshader.cpp` | 419 | 0 | 1 | 0 | 0 |  |
| `core/hw/hwtexturert.h` | 189 | 0 | 0 | 0 | 1 |  |
| `core/hw/shaders/ShaderGen/CodeGen.h` | 81 | 1 | 0 | 0 | 0 | IDirect3DDevice9(1) |
| `core/hw/shaders/ShaderGen/ResourceGen.h` | 83 | 1 | 0 | 0 | 0 | IDirect3DDevice9(1) |
| `core/resources/bitmapres.cpp` | 163 | 0 | 1 | 0 | 0 |  |
| `include/wgx_effect_types.h` | 49 | 0 | 0 | 0 | 1 |  |
| `include/wgx_render.h` | 1005 | 1 | 0 | 0 | 0 | IDirect3DDevice9(1) |

## 인터페이스별 참조 횟수와 파일

| 인터페이스 | 참조 | 파일 수 | 파일 (상위 6) |
|---|---:|---:|---|
| IDirect3DSurface9 | 190 | 38 | `d3ddevicewrapper.h`(24), `Device.cpp`(23), `Device.h`(23), `d3ddevice.cpp`(15), `d3ddevice.h`(11), `d3dglyphbank.cpp`(9) |
| IDirect3DDevice9 | 72 | 38 | `shader_compiler_dx9.hpp`(4), `shader_compiler_xm.hpp`(4), `d3ddevicewrapper.h`(4), `util.cpp`(4), `d3ddevice.h`(4), `dxvamanagerwrapper.h`(3) |
| IDirect3DTexture9 | 55 | 24 | `d3ddevice.cpp`(9), `d3ddevice.h`(7), `d3dtexture.h`(5), `d3dglyphbank.cpp`(4), `d3dglyphbank.h`(3), `d3dvidmemonlytexture.cpp`(3) |
| IDirect3D9 | 42 | 15 | `d3dloader.cpp`(8), `Display.h`(6), `display.cpp`(5), `d3ddevice.cpp`(5), `d3ddevice.h`(3), `d3ddevicewrapper.h`(2) |
| IDirect3DVertexBuffer9 | 35 | 11 | `d3ddevicewrapper.h`(6), `Device.cpp`(6), `Device.h`(6), `d3ddevice.h`(4), `HwRenderStateManager.h`(3), `d3ddevice.cpp`(3) |
| IDirect3DVertexShader9 | 32 | 13 | `d3ddevice.h`(5), `d3ddevice.cpp`(4), `d3ddevicewrapper.h`(3), `HwRenderStateManager.h`(3), `HwShaderPipeline.h`(3), `Device.cpp`(3) |
| IDirect3DPixelShader9 | 32 | 14 | `d3ddevice.h`(4), `d3ddevicewrapper.h`(3), `HwRenderStateManager.h`(3), `HwShaderPipeline.h`(3), `d3ddevice.cpp`(3), `Device.cpp`(3) |
| IDirect3DBaseTexture9 | 22 | 10 | `d3ddevicewrapper.h`(4), `Device.cpp`(4), `Device.h`(4), `HwRenderStateManager.h`(3), `HwStateTable.h`(2), `HwRenderStateManager.cpp`(1) |
| IDirect3DIndexBuffer9 | 21 | 11 | `d3ddevicewrapper.h`(3), `HwRenderStateManager.h`(3), `Device.cpp`(3), `Device.h`(3), `Hw3DGeometryStreamBuffer.h`(2), `HwStateTable.h`(2) |
| IDirect3DSwapChain9 | 19 | 9 | `d3ddevice.cpp`(3), `d3dswapchain.cpp`(3), `d3dswapchain.h`(3), `d3ddevicewrapper.h`(2), `d3dswapchainwithswdc.h`(2), `Device.cpp`(2) |
| IDirect3DDeviceManager9 | 14 | 5 | `avloader.cpp`(4), `evrpresenter.cpp`(4), `dxvamanagerwrapper.cpp`(3), `dxvamanagerwrapper.h`(2), `avloader.h`(1) |
| IDirect3D9Ex | 13 | 5 | `d3dloader.cpp`(5), `Display.h`(4), `display.cpp`(2), `D3DLoader.h`(1), `d3ddevicemanager.cpp`(1) |
| IDirect3DVertexDeclaration9 | 12 | 3 | `d3ddevicewrapper.h`(4), `Device.cpp`(4), `Device.h`(4) |
| IDirect3DDevice9Ex | 9 | 6 | `InteropDeviceBitmap.cpp`(3), `HwUtils.cpp`(2), `HwRenderStateManager.h`(1), `InteropDeviceBitmap.h`(1), `d3ddevice.h`(1), `d3ddevicemanager.cpp`(1) |
| IDirect3DStateBlock9 | 8 | 5 | `d3ddevicewrapper.h`(2), `Device.cpp`(2), `Device.h`(2), `d3ddevice.cpp`(1), `d3ddevice.h`(1) |
| IDirect3DQuery9 | 8 | 7 | `gpumarker.h`(2), `d3ddevicewrapper.h`(1), `d3ddevice.cpp`(1), `d3dstats.cpp`(1), `gpumarker.cpp`(1), `Device.cpp`(1) |
| IDirect3DVolumeTexture9 | 3 | 3 | `d3ddevicewrapper.h`(1), `Device.cpp`(1), `Device.h`(1) |
| IDirect3DCubeTexture9 | 3 | 3 | `d3ddevicewrapper.h`(1), `Device.cpp`(1), `Device.h`(1) |
| IDirect3DSwapChain9Ex | 2 | 2 | `d3dswapchain.cpp`(1), `d3dswapchain.h`(1) |
| IDirect3DVideoDevice9 | 1 | 1 | `d3ddevicewrapper.h`(1) |
| IDirect3DDXVADevice9 | 1 | 1 | `d3ddevicewrapper.h`(1) |

## 메서드 호출 집계 (`->Name(` 기준, 근사치)

| 메서드 | 호출 | 소유 인터페이스 | 파일 (상위 5) |
|---|---:|---|---|
| UnlockRect | 24 | IDirect3DTexture9, IDirect3DSurface9 | `hwmediabuffer.cpp`(3), `swmediabuffer.cpp`(3), `hwdevicebitmapcolorsource.cpp`(3), `d3dglyphbank.cpp`(2), `d3dlockabletexture.h`(2) |
| SetPixelShaderConstantF | 21 | IDirect3DDevice9 | `d3drenderstate.cpp`(8), `BlurEffect.cpp`(4), `DropShadowEffect.cpp`(3), `HwShaderPipeline.cpp`(2), `ShaderEffect.cpp`(2) |
| SetTexture | 19 | IDirect3DDevice9 | `BlurEffect.cpp`(8), `DropShadowEffect.cpp`(3), `ShaderEffect.cpp`(2), `HwBoxColorSource.cpp`(1), `HwRenderStateManager.cpp`(1) |
| LockRect | 16 | IDirect3DTexture9, IDirect3DSurface9 | `d3dlockabletexture.cpp`(2), `hwdevicebitmapcolorsource.cpp`(2), `hwsw3dfallback.cpp`(2), `hwmediabuffer.cpp`(1), `swmediabuffer.cpp`(1) |
| GetSurfaceLevel | 13 | IDirect3DTexture9 | `d3ddevice.cpp`(3), `d3dglyphbank.cpp`(2), `hwmediabuffer.cpp`(1), `HwUtils.cpp`(1), `d3dtexture.cpp`(1) |
| StretchRect | 12 | IDirect3DDevice9 | `hwsurfrt.cpp`(3), `d3ddevice.cpp`(2), `hwdevicebitmapcolorsource.cpp`(2), `hwdisplayrt.cpp`(2), `HwBitBltDeviceBitmapColorSource.cpp`(1) |
| SetRenderState | 11 | IDirect3DDevice9 | `hwsurfrt.cpp`(7), `hwffshaders.cpp`(3), `HwRenderStateManager.cpp`(1) |
| ReleaseDC | 10 | IDirect3DSurface9 | `HwBitBltDeviceBitmapColorSource.cpp`(4), `d3ddevice.cpp`(2), `InteropDeviceBitmap.cpp`(1), `d3dsurface.cpp`(1), `d3dsurface.h`(1) |
| SetSamplerState | 10 | IDirect3DDevice9 | `Effect.cpp`(6), `hwtexturedcolorsource.cpp`(3), `HwRenderStateManager.cpp`(1) |
| CreateTexture | 9 | IDirect3DDevice9 | `d3ddevice.cpp`(3), `d3dglyphbank.cpp`(2), `hwmediabuffer.cpp`(1), `HwUtils.cpp`(1), `d3dvidmemonlytexture.cpp`(1) |
| Unlock | 9 | IDirect3DVertexBuffer9, IDirect3DIndexBuffer9 | `d3ddevice.cpp`(8), `mediabuffer.cpp`(1) |
| CreateRenderTarget | 8 | IDirect3DDevice9 | `swmediabuffer.cpp`(1), `HwBitBltDeviceBitmapColorSource.cpp`(1), `HwUtils.cpp`(1), `d3ddevice.cpp`(1), `hwdevicebitmapcolorsource.cpp`(1) |
| CheckDeviceFormat | 8 | IDirect3D9 | `d3ddevice.cpp`(7), `d3dutils.cpp`(1) |
| GetType | 8 | IDirect3DResource9, IDirect3DQuery9 | `hwsurfrtdata.cpp`(7), `hwsurfrt.cpp`(1) |
| Lock | 7 | IDirect3DVertexBuffer9, IDirect3DIndexBuffer9 | `d3ddevice.cpp`(4), `mediabuffer.cpp`(1), `InteropDeviceBitmap.cpp`(1), `hwbitmapcolorsource.cpp`(1) |
| GetDC | 7 | IDirect3DSurface9 | `HwBitBltDeviceBitmapColorSource.cpp`(2), `d3ddevice.cpp`(2), `InteropDeviceBitmap.cpp`(1), `d3dsurface.cpp`(1), `d3dswapchain.cpp`(1) |
| WaitForVBlank | 7 | IDirect3DDevice9Ex | `d3ddevice.cpp`(2), `hwdisplayrt.cpp`(1), `desktoprt.cpp`(1), `composition.cpp`(1), `hwndtarget.cpp`(1) |
| GetDeviceCaps | 6 | IDirect3D9, IDirect3DDevice9 | `shader_compiler_xm.hpp`(2), `display.cpp`(1), `InteropDeviceBitmap.cpp`(1), `d3ddevice.cpp`(1), `d3ddevicemanager.cpp`(1) |
| GetDesc | 6 | IDirect3DSurface9, IDirect3DVertexBuffer9, IDirect3DIndexBuffer9 | `D3DTextureSurface.cpp`(1), `HwUtils.cpp`(1), `InteropDeviceBitmap.cpp`(1), `d3dsurface.cpp`(1), `hwbitmapcolorsource.cpp`(1) |
| SetTextureStageState | 6 | IDirect3DDevice9 | `hwtexturedcolorsource.cpp`(3), `d3drenderstate.cpp`(2), `HwRenderStateManager.cpp`(1) |
| SetDepthStencilSurface | 6 | IDirect3DDevice9 | `hwsurfrt.cpp`(5), `HwRenderStateManager.cpp`(1) |
| SetVertexShaderConstantF | 6 | IDirect3DDevice9 | `HwShaderPipeline.cpp`(3), `HwRenderStateManager.cpp`(1), `d3drenderstate.h`(1), `Effect.cpp`(1) |
| GetDevice | 6 | IDirect3DSwapChain9, IDirect3DResource9 | `InteropDeviceBitmap.cpp`(2), `HwUtils.cpp`(1), `d3dsurface.cpp`(1), `hwdevicebitmapcolorsource.cpp`(1), `hwdisplayrt.cpp`(1) |
| SetRenderTarget | 6 | IDirect3DDevice9 | `d3ddevice.cpp`(4), `hwsurfrt.cpp`(2) |
| DrawPrimitive | 6 | IDirect3DDevice9 | `d3ddevice.cpp`(3), `hwpipeline.cpp`(1), `hwshader.cpp`(1), `hwvertexbuffer.cpp`(1) |
| AddDirtyRect | 5 | IDirect3DTexture9 | `d3dlockabletexture.cpp`(2), `exports.cpp`(1), `hwdevicebitmapcolorsource.cpp`(1), `bitmapres.cpp`(1) |
| ColorFill | 5 | IDirect3DDevice9 | `fakepp.cpp`(1), `hwmediabuffer.cpp`(1), `d3ddevice.cpp`(1), `hwdestinationtexture.cpp`(1), `hwsurfrt.cpp`(1) |
| SetStreamSource | 5 | IDirect3DDevice9 | `HwRenderStateManager.cpp`(2), `d3drenderstate.h`(1), `hw3dgeometrystreambuffer.cpp`(1), `hwvertexbuffer.cpp`(1) |
| DrawPrimitiveUP | 5 | IDirect3DDevice9 | `HwVertexBuffer.h`(3), `d3ddevice.cpp`(2) |
| UpdateSurface | 5 | IDirect3DDevice9 | `HwVidMemTextureManager.cpp`(1), `InteropDeviceBitmap.cpp`(1), `d3ddevice.cpp`(1), `d3dglyphbank.cpp`(1), `hwbitmapcolorsource.cpp`(1) |
| Clear | 5 | IDirect3DDevice9 | `d3ddevice.cpp`(2), `hwsurfrt.cpp`(2), `hwdisplayrt.cpp`(1) |
| GetDirect3D | 4 | IDirect3DDevice9 | `d3ddevice.cpp`(2), `d3dutils.cpp`(1), `InteropDeviceBitmap.cpp`(1) |
| SetPixelShader | 4 | IDirect3DDevice9 | `HwRenderStateManager.cpp`(1), `HwShaderEffect.cpp`(1), `HwShaderPipeline.cpp`(1), `hwpipeline.cpp`(1) |
| SetTransform | 4 | IDirect3DDevice9 | `HwRenderStateManager.cpp`(2), `d3drenderstate.h`(1), `hwtexturedcolorsource.cpp`(1) |
| SetIndices | 4 | IDirect3DDevice9 | `HwRenderStateManager.cpp`(1), `d3drenderstate.h`(1), `hw3dgeometrystreambuffer.cpp`(1), `hwvertexbuffer.cpp`(1) |
| GetBackBuffer | 4 | IDirect3DDevice9, IDirect3DSwapChain9 | `d3ddevice.cpp`(3), `d3dswapchain.cpp`(1) |
| GetTransform | 4 | IDirect3DDevice9 | `prerenderwalker.cpp`(2), `d3drenderstate.h`(1), `Effect.cpp`(1) |
| GetCreationParameters | 3 | IDirect3DDevice9 | `evrpresenter.cpp`(1), `InteropDeviceBitmap.cpp`(1), `d3ddevice.cpp`(1) |
| GetRenderTargetData | 3 | IDirect3DDevice9 | `hwmediabuffer.cpp`(1), `HwUtils.cpp`(1), `d3ddevice.cpp`(1) |
| GetAdapterCount | 3 | IDirect3D9 | `display.cpp`(1), `d3ddevicemanager.cpp`(1), `d3dregistry.cpp`(1) |
| GetAdapterDisplayMode | 3 | IDirect3D9 | `display.cpp`(1), `d3ddevicemanager.cpp`(1), `hwhwndrt.cpp`(1) |
| SetFVF | 3 | IDirect3DDevice9 | `HwRenderStateManager.cpp`(1), `HwVertexBuffer.h`(1), `hw3dgeometrystreambuffer.cpp`(1) |
| SetVertexShader | 3 | IDirect3DDevice9 | `HwRenderStateManager.cpp`(1), `HwShaderPipeline.cpp`(1), `hwpipeline.cpp`(1) |
| SetPixelShaderConstantI | 3 | IDirect3DDevice9 | `HwRenderStateManager.cpp`(1), `d3drenderstate.h`(1), `ShaderEffect.cpp`(1) |
| SetPixelShaderConstantB | 3 | IDirect3DDevice9 | `HwRenderStateManager.cpp`(1), `d3drenderstate.h`(1), `ShaderEffect.cpp`(1) |
| Present | 3 | IDirect3DDevice9, IDirect3DSwapChain9 | `hwdisplayrt.cpp`(2), `d3ddevice.cpp`(1) |
| CreateQuery | 3 | IDirect3DDevice9 | `d3ddevice.cpp`(2), `d3dstats.cpp`(1) |
| Reset | 3 | IDirect3DDevice9 | `hwsurfrt.cpp`(2), `d3ddevice.cpp`(1) |
| CreateDevice | 3 | IDirect3D9 | `d3ddevicemanager.cpp`(3) |
| GetRenderState | 3 | IDirect3DDevice9 | `d3drenderstate.cpp`(3) |
| ResetDevice | 2 | IDirect3DDeviceManager9 | `dxvamanagerwrapper.cpp`(1), `evrpresenter.cpp`(1) |
| CreateOffscreenPlainSurface | 2 | IDirect3DDevice9 | `hwmediabuffer.cpp`(1), `d3ddevice.cpp`(1) |
| GetAdapterMonitor | 2 | IDirect3D9 | `display.cpp`(1), `InteropDeviceBitmap.cpp`(1) |
| GetAdapterIdentifier | 2 | IDirect3D9 | `display.cpp`(2) |
| SetScissorRect | 2 | IDirect3DDevice9 | `HwRenderStateManager.cpp`(1), `d3drenderstate.h`(1) |
| SetViewport | 2 | IDirect3DDevice9 | `HwRenderStateManager.cpp`(1), `d3drenderstate.h`(1) |
| SetConvolutionMonoKernel | 2 | IDirect3DDevice9Ex | `HwRenderStateManager.h`(1), `d3drenderstate.h`(1) |
| CreatePixelShader | 2 | IDirect3DDevice9 | `HwShaderEffect.cpp`(1), `d3ddevice.cpp`(1) |
| CheckDeviceState | 2 | IDirect3DDevice9Ex | `InteropDeviceBitmap.cpp`(1), `d3ddevice.cpp`(1) |
| CheckDeviceMultiSampleType | 2 | IDirect3D9 | `d3ddevice.cpp`(2) |
| CreateAdditionalSwapChain | 2 | IDirect3DDevice9 | `d3ddevice.cpp`(1), `hwhwndrt.cpp`(1) |
| CreateVertexBuffer | 2 | IDirect3DDevice9 | `d3ddevice.cpp`(1), `hw3dgeometrystreambuffer.cpp`(1) |
| CreateIndexBuffer | 2 | IDirect3DDevice9 | `d3ddevice.cpp`(1), `hw3dgeometrystreambuffer.cpp`(1) |
| UpdateTexture | 2 | IDirect3DDevice9 | `d3ddevice.cpp`(1), `hwdevicebitmapcolorsource.cpp`(1) |
| DrawIndexedPrimitive | 2 | IDirect3DDevice9 | `d3ddevice.cpp`(2) |
| CreateDeviceEx | 2 | IDirect3D9Ex | `d3ddevicemanager.cpp`(2) |
| GetTextureStageState | 2 | IDirect3DDevice9 | `d3drenderstate.cpp`(2) |
| GetViewport | 2 | IDirect3DDevice9 | `d3drenderstate.h`(1), `hwvertexbuffer.cpp`(1) |
| Issue | 2 | IDirect3DQuery9 | `d3dstats.cpp`(1), `gpumarker.cpp`(1) |
| GetData | 2 | IDirect3DQuery9 | `d3dstats.cpp`(1), `gpumarker.cpp`(1) |
| GetLevelDesc | 2 | IDirect3DTexture9 | `d3dtexture.cpp`(2) |
| OpenDeviceHandle | 1 | IDirect3DDeviceManager9 | `dxvamanagerwrapper.cpp`(1) |
| CloseDeviceHandle | 1 | IDirect3DDeviceManager9 | `dxvamanagerwrapper.cpp`(1) |
| TestDevice | 1 | IDirect3DDeviceManager9 | `dxvamanagerwrapper.cpp`(1) |
| LockDevice | 1 | IDirect3DDeviceManager9 | `dxvamanagerwrapper.cpp`(1) |
| UnlockDevice | 1 | IDirect3DDeviceManager9 | `dxvamanagerwrapper.cpp`(1) |
| GetVideoService | 1 | IDirect3DDeviceManager9 | `dxvamanagerwrapper.cpp`(1) |
| RegisterSoftwareDevice | 1 | IDirect3D9 | `d3dloader.cpp`(1) |
| GetAdapterLUID | 1 | IDirect3D9Ex | `display.cpp`(1) |
| GetAdapterDisplayModeEx | 1 | IDirect3D9Ex | `display.cpp`(1) |
| SetMaterial | 1 | IDirect3DDevice9 | `HwRenderStateManager.cpp`(1) |
| TestCooperativeLevel | 1 | IDirect3DDevice9 | `InteropDeviceBitmap.cpp`(1) |
| CheckDepthStencilMatch | 1 | IDirect3D9 | `d3ddevice.cpp`(1) |
| GetSwapChain | 1 | IDirect3DDevice9 | `d3ddevice.cpp`(1) |
| ComposeRects | 1 | IDirect3DDevice9Ex | `d3ddevice.cpp`(1) |
| CreateStateBlock | 1 | IDirect3DDevice9 | `d3ddevice.cpp`(1) |
| BeginScene | 1 | IDirect3DDevice9 | `d3ddevice.cpp`(1) |
| EndScene | 1 | IDirect3DDevice9 | `d3ddevice.cpp`(1) |
| CreateVertexShader | 1 | IDirect3DDevice9 | `d3ddevice.cpp`(1) |
| CreateDepthStencilSurface | 1 | IDirect3DDevice9 | `d3ddevice.cpp`(1) |
| SetPaletteEntries | 1 | IDirect3DDevice9 | `d3ddevice.cpp`(1) |
| SetCurrentTexturePalette | 1 | IDirect3DDevice9 | `d3ddevice.cpp`(1) |
| DrawIndexedPrimitiveUP | 1 | IDirect3DDevice9 | `d3ddevice.cpp`(1) |
| GetRasterStatus | 1 | IDirect3DDevice9, IDirect3DSwapChain9 | `d3ddevice.h`(1) |
| CheckDeviceType | 1 | IDirect3D9 | `d3ddevicemanager.cpp`(1) |
| GetSamplerState | 1 | IDirect3DDevice9 | `d3drenderstate.cpp`(1) |
| GetPresentParameters | 1 | IDirect3DSwapChain9 | `d3dswapchain.cpp`(1) |
| GetLevelCount | 1 | IDirect3DBaseTexture9 | `d3dtexture.cpp`(1) |
| GenerateMipSubLevels | 1 | IDirect3DBaseTexture9 | `d3dtexture.cpp`(1) |
| GetIndices | 1 | IDirect3DDevice9 | `hw3dgeometrystreambuffer.cpp`(1) |

## 열거형 가족별 사용 값 (정의 헤더 제외)


### D3DFMT_ — 63종, 181회, 파일 22개

`D3DFMT_A8R8G8B8`(31), `D3DFMT_X8R8G8B8`(18), `D3DFMT_UNKNOWN`(12), `D3DFMT_A2R10G10B10`(9), `D3DFMT_P8`(9), `D3DFMT_L8`(8), `D3DFMT_R8G8B8`(8), `D3DFMT_A32B32G32R32F`(7), `D3DFMT_R5G6B5`(7), `D3DFMT_X1R5G5B5`(5), `D3DFMT_A8`(3), `D3DFMT_D24S8`(3), `D3DFMT_INDEX16`(3), `D3DFMT_A1R5G5B5`(2), `D3DFMT_A8P8`(2), `D3DFMT_D16`(2), `D3DFMT_DXT1`(2), `D3DFMT_DXT2`(2), `D3DFMT_DXT3`(2), `D3DFMT_DXT4`(2), `D3DFMT_DXT5`(2), `D3DFMT_A1`(1), `D3DFMT_A16B16G16R16`(1), `D3DFMT_A16B16G16R16F`(1), `D3DFMT_A2B10G10R10`(1), `D3DFMT_A2W10V10U10`(1), `D3DFMT_A4L4`(1), `D3DFMT_A4R4G4B4`(1), `D3DFMT_A8B8G8R8`(1), `D3DFMT_A8L8`(1), `D3DFMT_A8R3G3B2`(1), `D3DFMT_BINARYBUFFER`(1), `D3DFMT_D15S1`(1), `D3DFMT_D16_LOCKABLE`(1), `D3DFMT_D24FS8`(1), `D3DFMT_D24X4S4`(1), `D3DFMT_D24X8`(1), `D3DFMT_D32`(1), `D3DFMT_D32F_LOCKABLE`(1), `D3DFMT_D32_LOCKABLE`(1), `D3DFMT_G16R16`(1), `D3DFMT_G16R16F`(1), `D3DFMT_G32R32F`(1), `D3DFMT_G8R8_G8B8`(1), `D3DFMT_INDEX32`(1), `D3DFMT_L16`(1), `D3DFMT_L6V5U5`(1), `D3DFMT_MULTI2_ARGB8`(1), `D3DFMT_Q16W16V16U16`(1), `D3DFMT_Q8W8V8U8`(1), `D3DFMT_R16F`(1), `D3DFMT_R32F`(1), `D3DFMT_R3G3B2`(1), `D3DFMT_R8G8_B8G8`(1), `D3DFMT_S8_LOCKABLE`(1), `D3DFMT_UYVY`(1), `D3DFMT_V16U16`(1), `D3DFMT_V8U8`(1), `D3DFMT_VERTEXDATA`(1), `D3DFMT_X4R4G4B4`(1), `D3DFMT_X8B8G8R8`(1), `D3DFMT_X8L8V8U8`(1), `D3DFMT_YUY2`(1)

### D3DSIO_ — 65종, 168회, 파일 2개

`D3DSIO_TEXKILL`(7), `D3DSIO_TEXCOORD`(6), `D3DSIO_CALLNZ`(5), `D3DSIO_IF`(5), `D3DSIO_SINCOS`(5), `D3DSIO_TEX`(5), `D3DSIO_TEXBEML`(5), `D3DSIO_TEXLDD`(5), `D3DSIO_TEXLDL`(5), `D3DSIO_BREAKP`(4), `D3DSIO_TEXBEM`(4), `D3DSIO_TEXBEML_LEGACY`(4), `D3DSIO_TEXREG2AR`(4), `D3DSIO_TEXREG2GB`(4), `D3DSIO_BREAKC`(3), `D3DSIO_COMMENT`(3), `D3DSIO_CRS`(3), `D3DSIO_DCL`(3), `D3DSIO_DEF`(3), `D3DSIO_DEFB`(3), `D3DSIO_DEFI`(3), `D3DSIO_DP3`(3), `D3DSIO_DP4`(3), `D3DSIO_IFC`(3), `D3DSIO_LABEL`(3), `D3DSIO_LOOP`(3), `D3DSIO_NOP`(3), `D3DSIO_NRM`(3), `D3DSIO_REP`(3), `D3DSIO_TEXBEM_LEGACY`(3), `D3DSIO_TEXDEPTH`(3), `D3DSIO_TEXDP3`(3), `D3DSIO_TEXREG2RGB`(3), `D3DSIO_BEM`(2), `D3DSIO_CALL`(2), `D3DSIO_DP2ADD`(2), `D3DSIO_ELSE`(2), `D3DSIO_ENDIF`(2), `D3DSIO_EXP`(2), `D3DSIO_LOG`(2), `D3DSIO_POW`(2), `D3DSIO_RCP`(2), `D3DSIO_RSQ`(2), `D3DSIO_TEXDP3TEX`(2), `D3DSIO_ABS`(1), `D3DSIO_ADD`(1), `D3DSIO_BREAK`(1), `D3DSIO_CMP`(1), `D3DSIO_CND`(1), `D3DSIO_DSX`(1), `D3DSIO_DSY`(1), `D3DSIO_END`(1), `D3DSIO_ENDLOOP`(1), `D3DSIO_ENDREP`(1), `D3DSIO_FRC`(1), `D3DSIO_LRP`(1), `D3DSIO_MAD`(1), `D3DSIO_MAX`(1), `D3DSIO_MIN`(1), `D3DSIO_MOV`(1), `D3DSIO_MUL`(1), `D3DSIO_PHASE`(1), `D3DSIO_RET`(1), `D3DSIO_SETP`(1), `D3DSIO_SUB`(1)

### D3DSI_ — 23종, 136회, 파일 2개

`D3DSI_GETREGNUM`(53), `D3DSI_GETREGTYPE`(24), `D3DSI_OPCODE_MASK`(15), `D3DSI_GETREGTYPE_RESOLVING_CONSTANTS`(6), `D3DSI_COISSUE`(4), `D3DSI_GETCOMPARISON`(4), `D3DSI_GETADDRESSMODE`(3), `D3DSI_GETREGNUM_RESOLVING_CONSTANTS`(3), `D3DSI_COMMENTSIZE_MASK`(2), `D3DSI_COMMENTSIZE_SHIFT`(2), `D3DSI_GETSWIZZLE`(2), `D3DSI_GETUSAGE`(2), `D3DSI_GETUSAGEINDEX`(2), `D3DSI_INSTLENGTH_MASK`(2), `D3DSI_INSTLENGTH_SHIFT`(2), `D3DSI_TEXLD_BIAS`(2), `D3DSI_TEXLD_PROJECT`(2), `D3DSI_GETDSTMODIFIER`(1), `D3DSI_GETINSTLENGTH`(1), `D3DSI_GETREGISTERPROPERTIES`(1), `D3DSI_GETSRCMODIFIER`(1), `D3DSI_GETSWIZZLECOMP`(1), `D3DSI_GETTEXTURETYPE`(1)

### D3DTSS_ — 8종, 119회, 파일 4개

`D3DTSS_COLOROP`(22), `D3DTSS_ALPHAARG1`(17), `D3DTSS_ALPHAARG2`(17), `D3DTSS_ALPHAOP`(17), `D3DTSS_COLORARG1`(17), `D3DTSS_COLORARG2`(17), `D3DTSS_TEXTURETRANSFORMFLAGS`(7), `D3DTSS_TEXCOORDINDEX`(5)

### D3DRS_ — 41종, 103회, 파일 6개

`D3DRS_FILLMODE`(8), `D3DRS_ZFUNC`(8), `D3DRS_DIFFUSEMATERIALSOURCE`(7), `D3DRS_SPECULARMATERIALSOURCE`(7), `D3DRS_BLENDFACTOR`(6), `D3DRS_COLORWRITEENABLE`(6), `D3DRS_SCISSORTESTENABLE`(6), `D3DRS_ZWRITEENABLE`(6), `D3DRS_ZENABLE`(5), `D3DRS_CULLMODE`(4), `D3DRS_MULTISAMPLEANTIALIAS`(4), `D3DRS_STENCILENABLE`(4), `D3DRS_ALPHABLENDENABLE`(2), `D3DRS_DESTBLEND`(2), `D3DRS_SRCBLEND`(2), `D3DRS_ALPHATESTENABLE`(1), `D3DRS_AMBIENT`(1), `D3DRS_AMBIENTMATERIALSOURCE`(1), `D3DRS_ANTIALIASEDLINEENABLE`(1), `D3DRS_BLENDOP`(1), `D3DRS_CLIPPING`(1), `D3DRS_CLIPPLANEENABLE`(1), `D3DRS_COLORVERTEX`(1), `D3DRS_DEPTHBIAS`(1), `D3DRS_DITHERENABLE`(1), `D3DRS_FOGENABLE`(1), `D3DRS_LASTPIXEL`(1), `D3DRS_LIGHTING`(1), `D3DRS_MULTISAMPLEMASK`(1), `D3DRS_NORMALIZENORMALS`(1), `D3DRS_SHADEMODE`(1), `D3DRS_SPECULARENABLE`(1), `D3DRS_STENCILFAIL`(1), `D3DRS_STENCILFUNC`(1), `D3DRS_STENCILMASK`(1), `D3DRS_STENCILPASS`(1), `D3DRS_STENCILREF`(1), `D3DRS_STENCILWRITEMASK`(1), `D3DRS_STENCILZFAIL`(1), `D3DRS_TWOSIDEDSTENCILMODE`(1), `D3DRS_VERTEXBLEND`(1)

### D3DMULTISAMPLE_ — 5종, 79회, 파일 24개

`D3DMULTISAMPLE_NONE`(37), `D3DMULTISAMPLE_TYPE`(36), `D3DMULTISAMPLE_2_SAMPLES`(4), `D3DMULTISAMPLE_16_SAMPLES`(1), `D3DMULTISAMPLE_4_SAMPLES`(1)

### D3DPS_ — 14종, 70회, 파일 8개

`D3DPS_VERSION`(48), `D3DPS_END`(3), `D3DPS_TEXTUREREG_MAX_V2_0`(3), `D3DPS_COLOROUT_MAX_V2_0`(2), `D3DPS_CONSTREG_MAX_V2_0`(2), `D3DPS_INPUTREG_MAX_V2_0`(2), `D3DPS_PREDICATE_MAX_V3_0`(2), `D3DPS_TEMPREG_MAX_V2_0`(2), `D3DPS_COLOROUT_MAX_V3_0`(1), `D3DPS_CONSTBOOLREG_MAX_SW_DX9`(1), `D3DPS_CONSTINTREG_MAX_SW_DX9`(1), `D3DPS_CONSTREG_MAX_SW_DX9`(1), `D3DPS_INPUTREG_MAX_SW_DX9`(1), `D3DPS_TEMPREG_MAX_V3_0`(1)

### D3DERR_ — 9종, 65회, 파일 22개

`D3DERR_DRIVERINTERNALERROR`(16), `D3DERR_OUTOFVIDEOMEMORY`(15), `D3DERR_DEVICELOST`(12), `D3DERR_NOTAVAILABLE`(11), `D3DERR_INVALIDCALL`(5), `D3DERR_DEVICEHUNG`(2), `D3DERR_DEVICEREMOVED`(2), `D3DERR_NOTFOUND`(1), `D3DERR_WRONGTEXTUREFORMAT`(1)

### D3DTA_ — 5종, 57회, 파일 1개

`D3DTA_TEXTURE`(24), `D3DTA_CURRENT`(22), `D3DTA_DIFFUSE`(9), `D3DTA_ALPHAREPLICATE`(1), `D3DTA_SPECULAR`(1)

### D3DSP_ — 28종, 56회, 파일 3개

`D3DSP_SWIZZLE_SHIFT`(8), `D3DSP_SWIZZLE_MASK`(7), `D3DSP_NOSWIZZLE`(6), `D3DSP_WRITEMASK_ALL`(6), `D3DSP_SRCMOD_MASK`(5), `D3DSP_TEXTURETYPE_MASK`(2), `D3DSP_DCL_USAGEINDEX_MASK`(1), `D3DSP_DCL_USAGEINDEX_SHIFT`(1), `D3DSP_DCL_USAGE_MASK`(1), `D3DSP_DCL_USAGE_SHIFT`(1), `D3DSP_DSTMOD_MASK`(1), `D3DSP_DSTSHIFT_MASK`(1), `D3DSP_DSTSHIFT_SHIFT`(1), `D3DSP_OPCODESPECIFICCONTROL_MASK`(1), `D3DSP_REGISTERPROPERTIES_MASK`(1), `D3DSP_REGNUM_MASK`(1), `D3DSP_REGTYPE_MASK`(1), `D3DSP_REGTYPE_MASK2`(1), `D3DSP_REGTYPE_SHIFT`(1), `D3DSP_REGTYPE_SHIFT2`(1), `D3DSP_REPLICATEALPHA`(1), `D3DSP_REPLICATEBLUE`(1), `D3DSP_REPLICATEGREEN`(1), `D3DSP_REPLICATERED`(1), `D3DSP_WRITEMASK_0`(1), `D3DSP_WRITEMASK_1`(1), `D3DSP_WRITEMASK_2`(1), `D3DSP_WRITEMASK_3`(1)

### D3DSPR_ — 16종, 50회, 파일 2개

`D3DSPR_PREDICATE`(8), `D3DSPR_TEXTURE`(8), `D3DSPR_CONST`(4), `D3DSPR_CONSTBOOL`(4), `D3DSPR_INPUT`(4), `D3DSPR_SAMPLER`(4), `D3DSPR_CONSTINT`(3), `D3DSPR_TEMP`(3), `D3DSPR_CONST2`(2), `D3DSPR_CONST3`(2), `D3DSPR_CONST4`(2), `D3DSPR_LOOP`(2), `D3DSPR_COLOROUT`(1), `D3DSPR_DEPTHOUT`(1), `D3DSPR_LABEL`(1), `D3DSPR_MISCTYPE`(1)

### D3DSAMP_ — 7종, 42회, 파일 5개

`D3DSAMP_MAGFILTER`(11), `D3DSAMP_MINFILTER`(11), `D3DSAMP_MIPFILTER`(9), `D3DSAMP_ADDRESSU`(4), `D3DSAMP_ADDRESSV`(4), `D3DSAMP_BORDERCOLOR`(2), `D3DSAMP_MAXANISOTROPY`(1)

### D3DTADDRESS_ — 4종, 39회, 파일 11개

`D3DTADDRESS_CLAMP`(23), `D3DTADDRESS_WRAP`(6), `D3DTADDRESS_BORDER`(5), `D3DTADDRESS_MIRROR`(5)

### D3DTEXF_ — 5종, 38회, 파일 11개

`D3DTEXF_NONE`(17), `D3DTEXF_LINEAR`(12), `D3DTEXF_POINT`(4), `D3DTEXF_ANISOTROPIC`(3), `D3DTEXF_CONVOLUTIONMONO`(2)

### D3DPOOL_ — 5종, 37회, 파일 18개

`D3DPOOL_DEFAULT`(16), `D3DPOOL_SYSTEMMEM`(14), `D3DPOOL_MANAGED`(4), `D3DPOOL_MANAGED_INTERNAL`(2), `D3DPOOL_SCRATCH`(1)

### D3DTOP_ — 5종, 35회, 파일 2개

`D3DTOP_MODULATE`(15), `D3DTOP_SELECTARG1`(10), `D3DTOP_DISABLE`(7), `D3DTOP_SELECTARG2`(2), `D3DTOP_BUMPENVMAP`(1)

### D3DDEVTYPE_ — 3종, 31회, 파일 10개

`D3DDEVTYPE_SW`(16), `D3DDEVTYPE_HAL`(13), `D3DDEVTYPE_REF`(2)

### D3DPRESENT_ — 4종, 30회, 파일 11개

`D3DPRESENT_PARAMETERS`(26), `D3DPRESENT_INTERVAL_IMMEDIATE`(2), `D3DPRESENT_INTERVAL_ONE`(1), `D3DPRESENT_LINEAR_CONTENT`(1)

### D3DPBLENDCAPS_ — 14종, 29회, 파일 2개

`D3DPBLENDCAPS_BLENDFACTOR`(3), `D3DPBLENDCAPS_BOTHINVSRCALPHA`(2), `D3DPBLENDCAPS_BOTHSRCALPHA`(2), `D3DPBLENDCAPS_DESTALPHA`(2), `D3DPBLENDCAPS_DESTCOLOR`(2), `D3DPBLENDCAPS_INVDESTALPHA`(2), `D3DPBLENDCAPS_INVDESTCOLOR`(2), `D3DPBLENDCAPS_INVSRCALPHA`(2), `D3DPBLENDCAPS_INVSRCCOLOR`(2), `D3DPBLENDCAPS_ONE`(2), `D3DPBLENDCAPS_SRCALPHA`(2), `D3DPBLENDCAPS_SRCALPHASAT`(2), `D3DPBLENDCAPS_SRCCOLOR`(2), `D3DPBLENDCAPS_ZERO`(2)

### D3DUSAGE_ — 5종, 26회, 파일 14개

`D3DUSAGE_RENDERTARGET`(15), `D3DUSAGE_AUTOGENMIPMAP`(3), `D3DUSAGE_DEPTHSTENCIL`(3), `D3DUSAGE_WRITEONLY`(3), `D3DUSAGE_DYNAMIC`(2)

### D3DRTYPE_ — 2종, 24회, 파일 12개

`D3DRTYPE_TEXTURE`(23), `D3DRTYPE_SURFACE`(1)

### D3DPT_ — 4종, 23회, 파일 3개

`D3DPT_LINELIST`(8), `D3DPT_TRIANGLELIST`(8), `D3DPT_TRIANGLESTRIP`(5), `D3DPT_TRIANGLEFAN`(2)

### D3DBLEND_ — 8종, 22회, 파일 1개

`D3DBLEND_ONE`(5), `D3DBLEND_INVSRCALPHA`(4), `D3DBLEND_SRCALPHA`(4), `D3DBLEND_ZERO`(4), `D3DBLEND_INVSRCCOLOR`(2), `D3DBLEND_BLENDFACTOR`(1), `D3DBLEND_INVDESTALPHA`(1), `D3DBLEND_INVDESTCOLOR`(1)

### D3DCREATE_ — 10종, 21회, 파일 4개

`D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX`(5), `D3DCREATE_MULTITHREADED`(5), `D3DCREATE_FPU_PRESERVE`(2), `D3DCREATE_PUREDEVICE`(2), `D3DCREATE_SOFTWARE_VERTEXPROCESSING`(2), `D3DCREATE_ADAPTERGROUP_DEVICE`(1), `D3DCREATE_DISABLE_DRIVER_MANAGEMENT`(1), `D3DCREATE_DISABLE_PSGP_THREADING`(1), `D3DCREATE_HARDWARE_VERTEXPROCESSING`(1), `D3DCREATE_SCREENSAVER`(1)

### D3DSHADER_ — 8종, 18회, 파일 4개

`D3DSHADER_PARAM_REGISTER_TYPE`(5), `D3DSHADER_COMPARISON`(4), `D3DSHADER_ADDRMODE_RELATIVE`(2), `D3DSHADER_INSTRUCTION_OPCODE_TYPE`(2), `D3DSHADER_VERSION_MAJOR`(2), `D3DSHADER_COMPARISON_MASK`(1), `D3DSHADER_COMPARISON_SHIFT`(1), `D3DSHADER_INSTRUCTION_PREDICATED`(1)

### D3DFVF_ — 9종, 18회, 파일 2개

`D3DFVF_DIFFUSE`(5), `D3DFVF_XYZ`(5), `D3DFVF_NORMAL`(2), `D3DFVF_SPECULAR`(1), `D3DFVF_TEX1`(1), `D3DFVF_TEX2`(1), `D3DFVF_TEX4`(1), `D3DFVF_TEX6`(1), `D3DFVF_TEX8`(1)

### D3DPTEXTURECAPS_ — 4종, 16회, 파일 5개

`D3DPTEXTURECAPS_POW2`(9), `D3DPTEXTURECAPS_NONPOW2CONDITIONAL`(5), `D3DPTEXTURECAPS_ALPHAPALETTE`(1), `D3DPTEXTURECAPS_SQUAREONLY`(1)

### D3DSPSM_ — 14종, 15회, 파일 1개

`D3DSPSM_NOT`(2), `D3DSPSM_ABS`(1), `D3DSPSM_ABSNEG`(1), `D3DSPSM_BIAS`(1), `D3DSPSM_BIASNEG`(1), `D3DSPSM_COMP`(1), `D3DSPSM_DW`(1), `D3DSPSM_DZ`(1), `D3DSPSM_NEG`(1), `D3DSPSM_NONE`(1), `D3DSPSM_SIGN`(1), `D3DSPSM_SIGNNEG`(1), `D3DSPSM_X2`(1), `D3DSPSM_X2NEG`(1)

### D3DVS_ — 4종, 14회, 파일 6개

`D3DVS_VERSION`(11), `D3DVS_ADDRESSMODE_MASK`(1), `D3DVS_SWIZZLE_MASK`(1), `D3DVS_SWIZZLE_SHIFT`(1)

### D3DDECLUSAGE_ — 3종, 14회, 파일 1개

`D3DDECLUSAGE_TEXCOORD`(9), `D3DDECLUSAGE_COLOR`(4), `D3DDECLUSAGE_POSITION`(1)

### D3DSTT_ — 4종, 13회, 파일 2개

`D3DSTT_2D`(5), `D3DSTT_CUBE`(3), `D3DSTT_VOLUME`(3), `D3DSTT_UNKNOWN`(2)

### D3DCLEAR_ — 3종, 13회, 파일 4개

`D3DCLEAR_TARGET`(10), `D3DCLEAR_ZBUFFER`(2), `D3DCLEAR_STENCIL`(1)

### D3DCULL_ — 3종, 12회, 파일 6개

`D3DCULL_CW`(5), `D3DCULL_CCW`(4), `D3DCULL_NONE`(3)

### D3DLOCK_ — 4종, 10회, 파일 8개

`D3DLOCK_NO_DIRTY_UPDATE`(4), `D3DLOCK_READONLY`(4), `D3DLOCK_DISCARD`(1), `D3DLOCK_NOOVERWRITE`(1)

### D3DSWAPEFFECT_ — 2종, 8회, 파일 3개

`D3DSWAPEFFECT_COPY`(5), `D3DSWAPEFFECT_DISCARD`(3)

### D3DCMP_ — 3종, 7회, 파일 5개

`D3DCMP_LESSEQUAL`(5), `D3DCMP_ALWAYS`(1), `D3DCMP_NOTEQUAL`(1)

### D3DTTFF_ — 2종, 6회, 파일 4개

`D3DTTFF_DISABLE`(5), `D3DTTFF_COUNT2`(1)

### D3DQUERYTYPE_ — 4종, 6회, 파일 3개

`D3DQUERYTYPE_DDISTATS`(2), `D3DQUERYTYPE_EVENT`(2), `D3DQUERYTYPE_RESOURCEMANAGER`(1), `D3DQUERYTYPE_VERTEXSTATS`(1)

### D3DPS20CAPS_ — 5종, 5회, 파일 1개

`D3DPS20CAPS_ARBITRARYSWIZZLE`(1), `D3DPS20CAPS_GRADIENTINSTRUCTIONS`(1), `D3DPS20CAPS_NODEPENDENTREADLIMIT`(1), `D3DPS20CAPS_NOTEXINSTRUCTIONLIMIT`(1), `D3DPS20CAPS_PREDICATION`(1)

### D3DCAPS2_ — 2종, 5회, 파일 4개

`D3DCAPS2_CANSHARERESOURCE`(4), `D3DCAPS2_CANAUTOGENMIPMAP`(1)

### D3DZB_ — 2종, 5회, 파일 2개

`D3DZB_FALSE`(4), `D3DZB_TRUE`(1)

### D3DBACKBUFFER_ — 2종, 4회, 파일 4개

`D3DBACKBUFFER_TYPE`(2), `D3DBACKBUFFER_TYPE_MONO`(2)

### D3DFILL_ — 2종, 4회, 파일 3개

`D3DFILL_SOLID`(2), `D3DFILL_WIREFRAME`(2)

### D3DPTFILTERCAPS_ — 3종, 4회, 파일 2개

`D3DPTFILTERCAPS_MINFLINEAR`(2), `D3DPTFILTERCAPS_MAGFANISOTROPIC`(1), `D3DPTFILTERCAPS_MINFANISOTROPIC`(1)

### D3DSPDM_ — 2종, 3회, 파일 1개

`D3DSPDM_MSAMPCENTROID`(2), `D3DSPDM_SATURATE`(1)

### D3DSTENCILOP_ — 1종, 3회, 파일 1개

`D3DSTENCILOP_KEEP`(3)

### D3DDEVCAPS2_ — 1종, 3회, 파일 2개

`D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES`(3)

### D3DPRESENTFLAG_ — 2종, 3회, 파일 2개

`D3DPRESENTFLAG_LOCKABLE_BACKBUFFER`(2), `D3DPRESENTFLAG_DEVICECLIP`(1)

### D3DPRASTERCAPS_ — 1종, 3회, 파일 2개

`D3DPRASTERCAPS_SCISSORTEST`(3)

### D3DDEVCAPS_ — 2종, 2회, 파일 1개

`D3DDEVCAPS_HWTRANSFORMANDLIGHT`(1), `D3DDEVCAPS_PUREDEVICE`(1)

### D3DISSUE_ — 1종, 2회, 파일 2개

`D3DISSUE_END`(2)

### D3DPMISCCAPS_ — 1종, 1회, 파일 1개

`D3DPMISCCAPS_COLORWRITEENABLE`(1)

### D3DBLENDOP_ — 1종, 1회, 파일 1개

`D3DBLENDOP_ADD`(1)

### D3DPTADDRESSCAPS_ — 1종, 1회, 파일 1개

`D3DPTADDRESSCAPS_BORDER`(1)

### D3DCAPS3_ — 1종, 1회, 파일 1개

`D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION`(1)

## 구조체·타입 참조

`D3DSURFACE_DESC`(73), `D3DMATRIX`(35), `D3DPRESENT_PARAMETERS`(26), `D3DCAPS9`(23), `D3DLOCKED_RECT`(23), `D3DCOLOR`(21), `D3DDISPLAYMODEEX`(14), `D3DCOLORVALUE`(8), `D3DDEVICE_CREATION_PARAMETERS`(6), `D3DDISPLAYMODE`(5), `D3DRECT`(5), `D3DVIEWPORT9`(5), `D3DMATERIAL9`(5), `D3DLIGHT9`(4), `D3DRASTER_STATUS`(3), `D3DVECTOR`(3), `D3DXMATRIX`(2), `D3DVERTEXELEMENT9`(2), `D3DXVECTOR2`(1), `D3DXVECTOR3`(1), `D3DXVECTOR4`(1), `D3DXCOLOR`(1), `D3DADAPTER_IDENTIFIER9`(1)

## d3d9 계열 헤더를 include하는 파일

- `common/DirectXLayer/D3DX9/dx9layercommon.hpp`: d3dx9math.h
- `common/DirectXLayer/D3DX9/matrix_dx9.hpp`: d3dx9math.h
- `common/DirectXLayer/D3DX9/quaternion_dx9.hpp`: d3dx9math.h
- `common/DirectXLayer/D3DX9/shader_compiler_dx9.hpp`: d3dx9shader.h
- `common/DirectXLayer/D3DX9/vector3_dx9.hpp`: d3dx9math.h
- `common/DirectXLayer/D3DX9/vector4_dx9.hpp`: d3dx9math.h
- `common/DirectXLayer/XMath/shader_compiler_xm.hpp`: d3d9.h
- `common/DirectXLayer/XMath/xmcommon.hpp`: d3d9types.h
- `common/DirectXLayer/interfaces/color_t.hpp`: d3dx9.h
- `core/av/av.h`: dxva2api.h, evr.h
- `core/common/exts/precomp.hpp`: d3dx9.h
- `core/common/precomp.hpp`: dxva2api.h, evr.h
- `core/fxjit/PixelShader/precomp.h`: d3d9caps.h, d3d9types.h
- `core/hw/shaders/ShaderGen/precomp.h`: d3dx9.h
- `include/std.h`: d3d9.h
