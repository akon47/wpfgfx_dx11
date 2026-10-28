# D3D9 호출 지점 (자동 생성)

`tools/inventory/d3d9_inventory.py`가 만든다. 형식: `줄: 종류 이름 | 코드`. 정의 헤더 복사본은 제외.


## common/DirectXLayer/D3DX9/shader_compiler_dx9.hpp

- 173: enum `D3DERR_INVALIDCALL` | `if ((hResult == D3DERR_INVALIDCALL) \|\|`

## common/DirectXLayer/XMath/shader_compiler_xm.hpp

- 138: call `GetDeviceCaps` | `if (SUCCEEDED(pD3DDevice->GetDeviceCaps(&d3dCaps)))`
- 142: enum `D3DPS_VERSION` | `case D3DPS_VERSION(2, 0):`
- 145: enum `D3DPS_VERSION` | `case D3DPS_VERSION(2, 2):`
- 148: enum `D3DPS_VERSION` | `case D3DPS_VERSION(3, 0):`
- 151: enum `D3DPS_VERSION` | `case D3DPS_VERSION(4, 0):`
- 153: enum `D3DPS20CAPS_ARBITRARYSWIZZLE` | `(d3dCaps.PS20Caps.Caps & D3DPS20CAPS_ARBITRARYSWIZZLE) &&`
- 154: enum `D3DPS20CAPS_GRADIENTINSTRUCTIONS` | `(d3dCaps.PS20Caps.Caps & D3DPS20CAPS_GRADIENTINSTRUCTIONS) &&`
- 155: enum `D3DPS20CAPS_PREDICATION` | `(d3dCaps.PS20Caps.Caps & D3DPS20CAPS_PREDICATION) &&`
- 156: enum `D3DPS20CAPS_NODEPENDENTREADLIMIT` | `(d3dCaps.PS20Caps.Caps & D3DPS20CAPS_NODEPENDENTREADLIMIT) &&`
- 157: enum `D3DPS20CAPS_NOTEXINSTRUCTIONLIMIT` | `(d3dCaps.PS20Caps.Caps & D3DPS20CAPS_NOTEXINSTRUCTIONLIMIT))`
- 166: enum `D3DPS_VERSION` | `case D3DPS_VERSION(1, 1):`
- 167: enum `D3DPS_VERSION` | `case D3DPS_VERSION(1, 2):`
- 168: enum `D3DPS_VERSION` | `case D3DPS_VERSION(1, 3):`
- 169: enum `D3DPS_VERSION` | `case D3DPS_VERSION(1, 4):`
- 211: call `GetDeviceCaps` | `if (SUCCEEDED(pD3DDevice->GetDeviceCaps(&d3dCaps)))`
- 215: enum `D3DVS_VERSION` | `case D3DVS_VERSION(2, 0):`
- 218: enum `D3DVS_VERSION` | `case D3DVS_VERSION(2, 2):`
- 221: enum `D3DVS_VERSION` | `case D3DVS_VERSION(3, 0):`
- 224: enum `D3DVS_VERSION` | `case D3DVS_VERSION(4, 0):`
- 235: enum `D3DVS_VERSION` | `case D3DVS_VERSION(1, 1):`
- 236: enum `D3DVS_VERSION` | `case D3DVS_VERSION(1, 2):`
- 237: enum `D3DVS_VERSION` | `case D3DVS_VERSION(1, 3):`
- 238: enum `D3DVS_VERSION` | `case D3DVS_VERSION(1, 4):`
- 269: enum `D3DERR_INVALIDCALL` | `if ((hResult == D3DERR_INVALIDCALL) \|\|`

## common/shared/pixelformatutils.cpp

- 256: enum `D3DFMT_R8G8B8` | `case D3DFMT_R8G8B8:`
- 259: enum `D3DFMT_A8R8G8B8` | `case D3DFMT_A8R8G8B8:`
- 269: enum `D3DFMT_X8R8G8B8` | `case D3DFMT_X8R8G8B8:`
- 272: enum `D3DFMT_R5G6B5` | `case D3DFMT_R5G6B5:`
- 275: enum `D3DFMT_X1R5G5B5` | `case D3DFMT_X1R5G5B5:`
- 278: enum `D3DFMT_P8` | `case D3DFMT_P8:`
- 281: enum `D3DFMT_L8` | `case D3DFMT_L8:`
- 284: enum `D3DFMT_A2R10G10B10` | `case D3DFMT_A2R10G10B10:`
- 287: enum `D3DFMT_A32B32G32R32F` | `case D3DFMT_A32B32G32R32F:`
- 319: enum `D3DFMT_R8G8B8` | `return D3DFMT_R8G8B8;`
- 323: enum `D3DFMT_A8R8G8B8` | `return D3DFMT_A8R8G8B8;`
- 326: enum `D3DFMT_X8R8G8B8` | `return D3DFMT_X8R8G8B8;`
- 329: enum `D3DFMT_R5G6B5` | `return D3DFMT_R5G6B5;`
- 332: enum `D3DFMT_X1R5G5B5` | `return D3DFMT_X1R5G5B5;`
- 335: enum `D3DFMT_P8` | `return D3DFMT_P8;`
- 338: enum `D3DFMT_L8` | `return D3DFMT_L8;`
- 341: enum `D3DFMT_A2R10G10B10` | `return D3DFMT_A2R10G10B10;`
- 345: enum `D3DFMT_A32B32G32R32F` | `return D3DFMT_A32B32G32R32F;`
- 348: enum `D3DFMT_UNKNOWN` | `return D3DFMT_UNKNOWN;`
- 479: enum `D3DFMT_A32B32G32R32F` | `case D3DFMT_A32B32G32R32F:`
- 482: enum `D3DFMT_A8R8G8B8` | `case D3DFMT_A8R8G8B8:`
- 483: enum `D3DFMT_X8R8G8B8` | `case D3DFMT_X8R8G8B8:`
- 484: enum `D3DFMT_D24S8` | `case D3DFMT_D24S8:`
- 485: enum `D3DFMT_A2R10G10B10` | `case D3DFMT_A2R10G10B10:`
- 488: enum `D3DFMT_R8G8B8` | `case D3DFMT_R8G8B8:`
- 491: enum `D3DFMT_R5G6B5` | `case D3DFMT_R5G6B5:`
- 492: enum `D3DFMT_X1R5G5B5` | `case D3DFMT_X1R5G5B5:`
- 493: enum `D3DFMT_D16` | `case D3DFMT_D16:`
- 496: enum `D3DFMT_P8` | `case D3DFMT_P8:`
- 497: enum `D3DFMT_L8` | `case D3DFMT_L8:`

## core/api/api_factory.cpp

- 494: enum `D3DERR_OUTOFVIDEOMEMORY` | `&& hr != D3DERR_OUTOFVIDEOMEMORY)`

## core/api/api_rendercontext.h

- 138: enum `D3DCULL_NONE` | `CullMode3D = D3DCULL_NONE;`
- 139: enum `D3DCMP_LESSEQUAL` | `DepthBufferFunction3D = D3DCMP_LESSEQUAL;`

## core/api/exports.cpp

- 501: call `AddDirtyRect` | `IFC(THIS_PTR->AddDirtyRect(&rcDirty));`

## core/av/EvrPresenter.inl

- 32: enum `D3DERR_NOTAVAILABLE` | `return    hr == D3DERR_NOTAVAILABLE`
- 36: enum `D3DERR_DRIVERINTERNALERROR` | `\|\| hr == D3DERR_DRIVERINTERNALERROR`

## core/av/d3ddevicewrapper.h

- 108: enum `D3DPRESENT_PARAMETERS` | `D3DPRESENT_PARAMETERS* pPresentationParameters,`
- 121: enum `D3DPRESENT_PARAMETERS` | `D3DPRESENT_PARAMETERS* pPresentationParameters`
- 134: enum `D3DBACKBUFFER_TYPE` | `D3DBACKBUFFER_TYPE Type,`
- 213: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE MultiSample,`
- 224: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE MultiSample,`
- 751: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE MultiSample,`
- 769: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE MultiSample,`

## core/av/dxvamanagerwrapper.cpp

- 171: call `ResetDevice` | `IFC(m_pDXVAManager->ResetDevice(pDevice, resetToken));`
- 185: call `OpenDeviceHandle` | `IFC(m_pDXVAManager->OpenDeviceHandle(phDevice));`
- 197: call `CloseDeviceHandle` | `IFC(m_pDXVAManager->CloseDeviceHandle(hDevice));`
- 209: call `TestDevice` | `IFC(m_pDXVAManager->TestDevice(hDevice));`
- 245: call `LockDevice` | `IFC(m_pDXVAManager->LockDevice(hDevice, ppDevice, fBlock));`
- 267: call `UnlockDevice` | `IFC(m_pDXVAManager->UnlockDevice(hDevice, fSaveState));`
- 283: call `GetVideoService` | `IFC(m_pDXVAManager->GetVideoService(hDevice, riid, ppAccelServices));`

## core/av/evrpresenter.cpp

- 34: enum `D3DFMT_X8R8G8B8` | `D3DFMT_X8R8G8B8,`
- 35: enum `D3DFMT_A8R8G8B8` | `D3DFMT_A8R8G8B8`
- 1366: call `GetCreationParameters` | `DBG_CODE(IFC(pIMixerDevice->GetCreationParameters(&dcp)));`
- 1367: enum `D3DCREATE_MULTITHREADED` | `DBG_CODE(Assert((dcp.BehaviorFlags & D3DCREATE_MULTITHREADED) != 0));`
- 1382: call `ResetDevice` | `hr = THR(m_pDXVAManagerWrapper->ResetDevice(pIMixerDevice, m_ResetToken));`
- 1840: enum `D3DFMT_A8R8G8B8` | `msc_d3dFormatOrder[bestFormatIndex] == D3DFMT_A8R8G8B8,`
- 1841: enum `D3DFMT_X8R8G8B8` | `msc_d3dFormatOrder[bestFormatIndex] == D3DFMT_X8R8G8B8);`
- 3424: enum `D3DDEVTYPE_HAL` | `D3DDEVTYPE_HAL,`
- 3481: enum `D3DDEVTYPE_HAL` | `m_pCurrentRenderDevice == m_pSoftwareDevice ? D3DDEVTYPE_SW : D3DDEVTYPE_HAL));`
- 3481: enum `D3DDEVTYPE_SW` | `m_pCurrentRenderDevice == m_pSoftwareDevice ? D3DDEVTYPE_SW : D3DDEVTYPE_HAL));`

## core/av/fakepp.cpp

- 981: enum `D3DDEVTYPE_SW` | `D3DDEVTYPE_SW,`
- 1043: enum `D3DFMT_X8R8G8B8` | `D3DFMT_X8R8G8B8,`
- 1046: enum `D3DDEVTYPE_SW` | `D3DDEVTYPE_SW,`
- 1076: call `ColorFill` | `IFC(pIDevice->ColorFill(`

## core/av/hwmediabuffer.cpp

- 75: call `UnlockRect` | `IGNORE_HR(m_pIBitmapSurface->UnlockRect());`
- 200: call `GetSurfaceLevel` | `IFC(m_pIMixerTexture->GetSurfaceLevel(0, &m_pIMixerSurface));`
- 205: call `ColorFill` | `IFC(pIMixerDevice->ColorFill(m_pIMixerSurface, NULL, D3DCOLOR_XRGB(0, 0, 0)));`
- 207: enum `D3DPOOL_SYSTEMMEM` | `IFC(GetSurfaceDescription(D3DPOOL_SYSTEMMEM, &ddsd));`
- 214: call `CreateOffscreenPlainSurface` | `IFC(pIMixerDevice->CreateOffscreenPlainSurface(`
- 279: enum `D3DPOOL_DEFAULT` | `IFC(GetSurfaceDescription(D3DPOOL_DEFAULT, &ddsd));`
- 283: call `CreateTexture` | `IFC(pIMixerDevice->CreateTexture(`
- 381: enum `D3DRTYPE_TEXTURE` | `pD3DSurfaceDesc->Type = D3DRTYPE_TEXTURE;`
- 382: enum `D3DUSAGE_RENDERTARGET` | `pD3DSurfaceDesc->Usage = D3DUSAGE_RENDERTARGET;`
- 386: enum `D3DMULTISAMPLE_NONE` | `pD3DSurfaceDesc->MultiSampleType = D3DMULTISAMPLE_NONE;`
- 436: call `UnlockRect` | `IFC(m_pIBitmapSurface->UnlockRect());`
- 446: call `GetRenderTargetData` | `IFC(pIMixerDevice->GetRenderTargetData(m_pIMixerSurface, m_pIBitmapSurface));`
- 449: call `LockRect` | `IFC(m_pIBitmapSurface->LockRect(`
- 508: call `UnlockRect` | `IGNORE_HR(m_pIBitmapSurface->UnlockRect());`

## core/av/mediabuffer.cpp

- 137: enum `D3DDEVTYPE_HAL` | `case D3DDEVTYPE_HAL:`
- 154: enum `D3DDEVTYPE_SW` | `case D3DDEVTYPE_SW:`
- 211: call `Lock` | `m_pIMFMediaBuffer->Lock(`
- 233: call `Unlock` | `IFC(m_pIMFMediaBuffer->Unlock());`

## core/av/swmediabuffer.cpp

- 149: call `UnlockRect` | `IFC(m_pIBitmapSurface->UnlockRect());`
- 188: call `CreateRenderTarget` | `IFC(pIDevice->CreateRenderTarget(`
- 192: enum `D3DMULTISAMPLE_NONE` | `D3DMULTISAMPLE_NONE,`
- 299: call `UnlockRect` | `m_pIBitmapSurface->UnlockRect();`
- 305: call `LockRect` | `IFC(m_pIBitmapSurface->LockRect(`
- 341: call `UnlockRect` | `IGNORE_HR(m_pIBitmapSurface->UnlockRect());`

## core/common/Tier.cpp

- 77: enum `D3DPS_VERSION` | `D3DPS_VERSION(2, 0),`
- 87: enum `D3DPS_VERSION` | `D3DPS_VERSION(2, 0),`
- 88: enum `D3DVS_VERSION` | `D3DVS_VERSION(2, 0),`
- 193: enum `D3DPS_VERSION` | `Assert(caps.PixelShaderVersion >= D3DPS_VERSION(1,1));`

## core/common/d3dloader.cpp

- 426: enum `D3DERR_NOTAVAILABLE` | `if (D3DERR_NOTAVAILABLE == hr)`
- 738: call `RegisterSoftwareDevice` | `IFC(pID3D->RegisterSoftwareDevice(m_pfnSwRastD3D9GetSWInfo));`

## core/common/d3dutils.cpp

- 40: enum `D3DUSAGE_RENDERTARGET` | `Assert(   dwUsage == D3DUSAGE_RENDERTARGET`
- 46: enum `D3DRTYPE_TEXTURE` | `pDesc->Type = D3DRTYPE_TEXTURE;`
- 49: enum `D3DMULTISAMPLE_NONE` | `pDesc->MultiSampleType = D3DMULTISAMPLE_NONE;`
- 123: enum `D3DPTEXTURECAPS_POW2` | `Assert(!(pcaps->TextureCaps & D3DPTEXTURECAPS_POW2) \|\|`
- 129: enum `D3DPTEXTURECAPS_POW2` | `else if (pcaps->TextureCaps & D3DPTEXTURECAPS_POW2)`
- 133: enum `D3DPTEXTURECAPS_NONPOW2CONDITIONAL` | `Assert(pcaps->TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL);`
- 152: enum `D3DPTEXTURECAPS_POW2` | `Assert(!(pcaps->TextureCaps & D3DPTEXTURECAPS_POW2) \|\|`
- 158: enum `D3DPTEXTURECAPS_POW2` | `else if (pcaps->TextureCaps & D3DPTEXTURECAPS_POW2)`
- 162: enum `D3DPTEXTURECAPS_NONPOW2CONDITIONAL` | `Assert(pcaps->TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL);`
- 183: call `GetDirect3D` | `if (SUCCEEDED((hr = THR(pD3DDevice->GetDirect3D(&pd3d9)))))`
- 190: enum `D3DFMT_P8` | `if ((d3dFormat == D3DFMT_P8) &&`
- 192: enum `D3DPTEXTURECAPS_ALPHAPALETTE` | `!(pcaps->TextureCaps & D3DPTEXTURECAPS_ALPHAPALETTE))`
- 199: call `CheckDeviceFormat` | `hr = pd3d9->CheckDeviceFormat(`
- 216: enum `D3DFMT_UNKNOWN` | `} while ( d3dFormat != D3DFMT_UNKNOWN );`
- 226: enum `D3DFMT_DXT1` | `\|\| (   (d3dFormat != D3DFMT_DXT1)`
- 227: enum `D3DFMT_DXT2` | `&& (d3dFormat != D3DFMT_DXT2)`
- 228: enum `D3DFMT_DXT3` | `&& (d3dFormat != D3DFMT_DXT3)`
- 229: enum `D3DFMT_DXT4` | `&& (d3dFormat != D3DFMT_DXT4)`
- 230: enum `D3DFMT_DXT5` | `&& (d3dFormat != D3DFMT_DXT5)`
- 284: enum `D3DFMT_P8` | `case D3DFMT_P8:`
- 287: enum `D3DFMT_A8R8G8B8` | `return D3DFMT_A8R8G8B8;`
- 291: enum `D3DFMT_R8G8B8` | `return D3DFMT_R8G8B8;`
- 294: enum `D3DFMT_X1R5G5B5` | `case D3DFMT_X1R5G5B5:`
- 295: enum `D3DFMT_R5G6B5` | `return D3DFMT_R5G6B5;`
- 297: enum `D3DFMT_R5G6B5` | `case D3DFMT_R5G6B5:`
- 298: enum `D3DFMT_R8G8B8` | `return D3DFMT_R8G8B8;`
- 300: enum `D3DFMT_R8G8B8` | `case D3DFMT_R8G8B8:`
- 301: enum `D3DFMT_X8R8G8B8` | `return D3DFMT_X8R8G8B8;`
- 303: enum `D3DFMT_X8R8G8B8` | `case D3DFMT_X8R8G8B8:`
- 304: enum `D3DFMT_A8R8G8B8` | `return D3DFMT_A8R8G8B8;`
- 306: enum `D3DFMT_A8P8` | `case D3DFMT_A8P8:`
- 307: enum `D3DFMT_A8R8G8B8` | `return D3DFMT_A8R8G8B8;`
- 309: enum `D3DFMT_A1R5G5B5` | `case D3DFMT_A1R5G5B5:`
- 310: enum `D3DFMT_A8R8G8B8` | `return D3DFMT_A8R8G8B8;`
- 312: enum `D3DFMT_A8R8G8B8` | `case D3DFMT_A8R8G8B8:`
- 317: enum `D3DFMT_UNKNOWN` | `return D3DFMT_UNKNOWN;`

## core/common/d3dutils.h

- 81: enum `D3DTADDRESS_CLAMP` | `return ((taU == D3DTADDRESS_CLAMP) && (taV == D3DTADDRESS_CLAMP));`
- 81: enum `D3DTADDRESS_CLAMP` | `return ((taU == D3DTADDRESS_CLAMP) && (taV == D3DTADDRESS_CLAMP));`

## core/common/display.cpp

- 789: enum `D3DPS_VERSION` | `/* PixelShaderVersion = */      D3DPS_VERSION(0,0),`
- 790: enum `D3DVS_VERSION` | `/* VertexShaderVersion = */     D3DVS_VERSION(0,0),`
- 1618: call `GetAdapterCount` | `m_uD3DAdapterCount = m_pID3D->GetAdapterCount();`
- 1637: call `GetAdapterMonitor` | `HMONITOR hMonitor = m_pID3D->GetAdapterMonitor(i);`
- 1676: call `GetAdapterLUID` | `IGNORE_HR(m_pID3DEx->GetAdapterLUID(`
- 2341: enum `D3DFMT_UNKNOWN` | `m_DisplayMode.Format = D3DFMT_UNKNOWN;`
- 2474: enum `D3DFMT_UNKNOWN` | `Assert(m_DisplayMode.Format == D3DFMT_UNKNOWN);`
- 2486: call `GetAdapterDisplayModeEx` | `MIL_THR(pID3DEx->GetAdapterDisplayModeEx(`
- 2500: call `GetAdapterDisplayMode` | `MIL_THR(pID3D->GetAdapterDisplayMode(m_uDisplayIndex, &displayMode));`
- 2518: enum `D3DERR_NOTAVAILABLE` | `if (hr == D3DERR_NOTAVAILABLE)`
- 2531: enum `D3DFMT_UNKNOWN` | `if (   (m_DisplayMode.Format == D3DFMT_UNKNOWN)`
- 2557: enum `D3DFMT_UNKNOWN` | `if (m_DisplayMode.Format == D3DFMT_UNKNOWN)`
- 2573: enum `D3DFMT_X8R8G8B8` | `m_DisplayMode.Format = D3DFMT_X8R8G8B8;`
- 2577: enum `D3DFMT_R8G8B8` | `m_DisplayMode.Format = D3DFMT_R8G8B8;`
- 2582: enum `D3DFMT_R5G6B5` | `m_DisplayMode.Format = D3DFMT_R5G6B5;`
- 2586: enum `D3DFMT_P8` | `m_DisplayMode.Format = D3DFMT_P8;`
- 2590: enum `D3DFMT_UNKNOWN` | `m_DisplayMode.Format = D3DFMT_UNKNOWN;`
- 2619: enum `D3DERR_DEVICELOST` | `if (hr == D3DERR_DEVICELOST \|\| hr == D3DERR_DRIVERINTERNALERROR)`
- 2619: enum `D3DERR_DRIVERINTERNALERROR` | `if (hr == D3DERR_DEVICELOST \|\| hr == D3DERR_DRIVERINTERNALERROR)`
- 2673: enum `D3DDEVTYPE_HAL` | `D3DDEVTYPE_HAL,`
- 2680: call `GetDeviceCaps` | `if (SUCCEEDED(pID3D->GetDeviceCaps(`
- 2682: enum `D3DDEVTYPE_HAL` | `D3DDEVTYPE_HAL,`
- 2941: call `GetAdapterIdentifier` | `hr = pD3DEx->GetAdapterIdentifier(`
- 2951: call `GetAdapterIdentifier` | `IFC(pD3D->GetAdapterIdentifier(`

## core/fxjit/PixelShader/d3dhal.h

- 7: enum `D3DSI_GETREGNUM` | `#define D3DSI_GETREGNUM(token)  (token & D3DSP_REGNUM_MASK)`
- 7: enum `D3DSP_REGNUM_MASK` | `#define D3DSI_GETREGNUM(token)  (token & D3DSP_REGNUM_MASK)`
- 8: enum `D3DSHADER_PARAM_REGISTER_TYPE` | `#define D3DSI_GETREGTYPE(token) ((D3DSHADER_PARAM_REGISTER_TYPE)(((token & D3DSP_REGTYPE_MASK) >> D3DSP_REGTYPE_SHIFT) \| \`
- 8: enum `D3DSI_GETREGTYPE` | `#define D3DSI_GETREGTYPE(token) ((D3DSHADER_PARAM_REGISTER_TYPE)(((token & D3DSP_REGTYPE_MASK) >> D3DSP_REGTYPE_SHIFT) \| \`
- 8: enum `D3DSP_REGTYPE_MASK` | `#define D3DSI_GETREGTYPE(token) ((D3DSHADER_PARAM_REGISTER_TYPE)(((token & D3DSP_REGTYPE_MASK) >> D3DSP_REGTYPE_SHIFT) \| \`
- 8: enum `D3DSP_REGTYPE_SHIFT` | `#define D3DSI_GETREGTYPE(token) ((D3DSHADER_PARAM_REGISTER_TYPE)(((token & D3DSP_REGTYPE_MASK) >> D3DSP_REGTYPE_SHIFT) \| \`
- 9: enum `D3DSP_REGTYPE_MASK2` | `((token & D3DSP_REGTYPE_MASK2) >> D3DSP_REGTYPE_SHIFT2)))`
- 9: enum `D3DSP_REGTYPE_SHIFT2` | `((token & D3DSP_REGTYPE_MASK2) >> D3DSP_REGTYPE_SHIFT2)))`
- 10: enum `D3DSI_GETUSAGE` | `#define D3DSI_GETUSAGE(token) ((token & D3DSP_DCL_USAGE_MASK) >> D3DSP_DCL_USAGE_SHIFT)`
- 10: enum `D3DSP_DCL_USAGE_MASK` | `#define D3DSI_GETUSAGE(token) ((token & D3DSP_DCL_USAGE_MASK) >> D3DSP_DCL_USAGE_SHIFT)`
- 10: enum `D3DSP_DCL_USAGE_SHIFT` | `#define D3DSI_GETUSAGE(token) ((token & D3DSP_DCL_USAGE_MASK) >> D3DSP_DCL_USAGE_SHIFT)`
- 11: enum `D3DSI_GETUSAGEINDEX` | `#define D3DSI_GETUSAGEINDEX(token) ((token & D3DSP_DCL_USAGEINDEX_MASK) >> D3DSP_DCL_USAGEINDEX_SHIFT)`
- 11: enum `D3DSP_DCL_USAGEINDEX_MASK` | `#define D3DSI_GETUSAGEINDEX(token) ((token & D3DSP_DCL_USAGEINDEX_MASK) >> D3DSP_DCL_USAGEINDEX_SHIFT)`
- 11: enum `D3DSP_DCL_USAGEINDEX_SHIFT` | `#define D3DSI_GETUSAGEINDEX(token) ((token & D3DSP_DCL_USAGEINDEX_MASK) >> D3DSP_DCL_USAGEINDEX_SHIFT)`
- 12: enum `D3DSI_GETINSTLENGTH` | `#define D3DSI_GETINSTLENGTH(token) ((token & D3DSI_INSTLENGTH_MASK) >> D3DSI_INSTLENGTH_SHIFT)`
- 12: enum `D3DSI_INSTLENGTH_MASK` | `#define D3DSI_GETINSTLENGTH(token) ((token & D3DSI_INSTLENGTH_MASK) >> D3DSI_INSTLENGTH_SHIFT)`
- 12: enum `D3DSI_INSTLENGTH_SHIFT` | `#define D3DSI_GETINSTLENGTH(token) ((token & D3DSI_INSTLENGTH_MASK) >> D3DSI_INSTLENGTH_SHIFT)`
- 13: enum `D3DSHADER_COMPARISON` | `#define D3DSI_GETCOMPARISON(token) ((D3DSHADER_COMPARISON)((token & D3DSHADER_COMPARISON_MASK) >> D3DSHADER_COMPARISON_SHIFT))`
- 13: enum `D3DSHADER_COMPARISON_MASK` | `#define D3DSI_GETCOMPARISON(token) ((D3DSHADER_COMPARISON)((token & D3DSHADER_COMPARISON_MASK) >> D3DSHADER_COMPARISON_SHIFT))`
- 13: enum `D3DSHADER_COMPARISON_SHIFT` | `#define D3DSI_GETCOMPARISON(token) ((D3DSHADER_COMPARISON)((token & D3DSHADER_COMPARISON_MASK) >> D3DSHADER_COMPARISON_SHIFT))`
- 13: enum `D3DSI_GETCOMPARISON` | `#define D3DSI_GETCOMPARISON(token) ((D3DSHADER_COMPARISON)((token & D3DSHADER_COMPARISON_MASK) >> D3DSHADER_COMPARISON_SHIFT))`
- 14: enum `D3DSI_GETREGISTERPROPERTIES` | `#define D3DSI_GETREGISTERPROPERTIES(token) (token & D3DSP_REGISTERPROPERTIES_MASK)`
- 14: enum `D3DSP_REGISTERPROPERTIES_MASK` | `#define D3DSI_GETREGISTERPROPERTIES(token) (token & D3DSP_REGISTERPROPERTIES_MASK)`
- 15: enum `D3DSI_GETTEXTURETYPE` | `#define D3DSI_GETTEXTURETYPE(token) (token & D3DSP_TEXTURETYPE_MASK)`
- 15: enum `D3DSP_TEXTURETYPE_MASK` | `#define D3DSI_GETTEXTURETYPE(token) (token & D3DSP_TEXTURETYPE_MASK)`
- 16: enum `D3DSI_GETDSTMODIFIER` | `#define D3DSI_GETDSTMODIFIER(token) (token & D3DSP_DSTMOD_MASK)`
- 16: enum `D3DSP_DSTMOD_MASK` | `#define D3DSI_GETDSTMODIFIER(token) (token & D3DSP_DSTMOD_MASK)`
- 17: enum `D3DSI_GETSWIZZLECOMP` | `#define D3DSI_GETSWIZZLECOMP(source, component)  (source >> ((component << 1) + 16) & 0x3)`
- 18: enum `D3DSI_GETSWIZZLE` | `#define D3DSI_GETSWIZZLE(token)  (token & D3DVS_SWIZZLE_MASK)`
- 18: enum `D3DVS_SWIZZLE_MASK` | `#define D3DSI_GETSWIZZLE(token)  (token & D3DVS_SWIZZLE_MASK)`
- 19: enum `D3DSI_GETSRCMODIFIER` | `#define D3DSI_GETSRCMODIFIER(token) (token & D3DSP_SRCMOD_MASK)`
- 19: enum `D3DSP_SRCMOD_MASK` | `#define D3DSI_GETSRCMODIFIER(token) (token & D3DSP_SRCMOD_MASK)`
- 20: enum `D3DSI_GETADDRESSMODE` | `#define D3DSI_GETADDRESSMODE(token) (token & D3DVS_ADDRESSMODE_MASK)`
- 20: enum `D3DVS_ADDRESSMODE_MASK` | `#define D3DSI_GETADDRESSMODE(token) (token & D3DVS_ADDRESSMODE_MASK)`
- 22: enum `D3DSHADER_PARAM_REGISTER_TYPE` | `inline D3DSHADER_PARAM_REGISTER_TYPE D3DSI_GETREGTYPE_RESOLVING_CONSTANTS(DWORD token)`
- 22: enum `D3DSI_GETREGTYPE_RESOLVING_CONSTANTS` | `inline D3DSHADER_PARAM_REGISTER_TYPE D3DSI_GETREGTYPE_RESOLVING_CONSTANTS(DWORD token)`
- 24: enum `D3DSHADER_PARAM_REGISTER_TYPE` | `D3DSHADER_PARAM_REGISTER_TYPE RegType = D3DSI_GETREGTYPE(token);`
- 24: enum `D3DSI_GETREGTYPE` | `D3DSHADER_PARAM_REGISTER_TYPE RegType = D3DSI_GETREGTYPE(token);`
- 27: enum `D3DSPR_CONST4` | `case D3DSPR_CONST4:`
- 28: enum `D3DSPR_CONST3` | `case D3DSPR_CONST3:`
- 29: enum `D3DSPR_CONST2` | `case D3DSPR_CONST2:`
- 30: enum `D3DSPR_CONST` | `return D3DSPR_CONST;`
- 48: enum `D3DSI_GETREGNUM_RESOLVING_CONSTANTS` | `inline UINT D3DSI_GETREGNUM_RESOLVING_CONSTANTS(DWORD token)`
- 50: enum `D3DSHADER_PARAM_REGISTER_TYPE` | `D3DSHADER_PARAM_REGISTER_TYPE RegType = D3DSI_GETREGTYPE(token);`
- 50: enum `D3DSI_GETREGTYPE` | `D3DSHADER_PARAM_REGISTER_TYPE RegType = D3DSI_GETREGTYPE(token);`
- 51: enum `D3DSI_GETREGNUM` | `UINT RegNum = D3DSI_GETREGNUM(token);`
- 54: enum `D3DSPR_CONST4` | `case D3DSPR_CONST4:`
- 56: enum `D3DSPR_CONST3` | `case D3DSPR_CONST3:`
- 58: enum `D3DSPR_CONST2` | `case D3DSPR_CONST2:`

## core/fxjit/PixelShader/pshader.cpp

- 249: enum `D3DSTT_2D` | `if (m_pTranslated->GetSamplerRegDcl()[i] == D3DSTT_2D)`
- 256: enum `D3DSTT_UNKNOWN` | `else if (m_pTranslated->GetSamplerRegDcl()[i] != D3DSTT_UNKNOWN)`
- 258: enum `D3DSTT_2D` | `JIT_TRACE(L"Invalid texture type: only D3DSTT_2D supported");`

## core/fxjit/PixelShader/pstrans.cpp

- 349: enum `D3DPS_END` | `while (*pToken != D3DPS_END())`
- 358: enum `D3DSIO_COMMENT` | `if ( (Inst & D3DSI_OPCODE_MASK) == D3DSIO_COMMENT )`
- 358: enum `D3DSI_OPCODE_MASK` | `if ( (Inst & D3DSI_OPCODE_MASK) == D3DSIO_COMMENT )`
- 360: enum `D3DSI_COMMENTSIZE_MASK` | `pToken += (Inst & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT;`
- 360: enum `D3DSI_COMMENTSIZE_SHIFT` | `pToken += (Inst & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT;`
- 362: enum `D3DSIO_DEF` | `else if( (Inst & D3DSI_OPCODE_MASK) == D3DSIO_DEF )`
- 362: enum `D3DSI_OPCODE_MASK` | `else if( (Inst & D3DSI_OPCODE_MASK) == D3DSIO_DEF )`
- 364: enum `D3DSI_GETREGTYPE_RESOLVING_CONSTANTS` | `switch(D3DSI_GETREGTYPE_RESOLVING_CONSTANTS(*pToken))`
- 366: enum `D3DSPR_CONST` | `case D3DSPR_CONST:`
- 372: enum `D3DSIO_DEFI` | `else if( (Inst & D3DSI_OPCODE_MASK) == D3DSIO_DEFI )`
- 372: enum `D3DSI_OPCODE_MASK` | `else if( (Inst & D3DSI_OPCODE_MASK) == D3DSIO_DEFI )`
- 374: enum `D3DSI_GETREGTYPE_RESOLVING_CONSTANTS` | `switch(D3DSI_GETREGTYPE_RESOLVING_CONSTANTS(*pToken))`
- 376: enum `D3DSPR_CONSTINT` | `case D3DSPR_CONSTINT:`
- 382: enum `D3DSIO_DEFB` | `else if( (Inst & D3DSI_OPCODE_MASK) == D3DSIO_DEFB )`
- 382: enum `D3DSI_OPCODE_MASK` | `else if( (Inst & D3DSI_OPCODE_MASK) == D3DSIO_DEFB )`
- 384: enum `D3DSI_GETREGTYPE` | `switch(D3DSI_GETREGTYPE(*pToken))`
- 386: enum `D3DSPR_CONSTBOOL` | `case D3DSPR_CONSTBOOL:`
- 392: enum `D3DSIO_DCL` | `else if( (Inst & D3DSI_OPCODE_MASK) == D3DSIO_DCL )`
- 392: enum `D3DSI_OPCODE_MASK` | `else if( (Inst & D3DSI_OPCODE_MASK) == D3DSIO_DCL )`
- 394: enum `D3DSI_INSTLENGTH_MASK` | `DWORD DclLength = ((Inst & D3DSI_INSTLENGTH_MASK) >> D3DSI_INSTLENGTH_SHIFT);`
- 394: enum `D3DSI_INSTLENGTH_SHIFT` | `DWORD DclLength = ((Inst & D3DSI_INSTLENGTH_MASK) >> D3DSI_INSTLENGTH_SHIFT);`
- 414: enum `D3DSI_GETREGNUM` | `UINT RegNum = D3DSI_GETREGNUM(DclRegister);`
- 415: enum `D3DSHADER_PARAM_REGISTER_TYPE` | `D3DSHADER_PARAM_REGISTER_TYPE RegType = D3DSI_GETREGTYPE(DclRegister);`
- 415: enum `D3DSI_GETREGTYPE` | `D3DSHADER_PARAM_REGISTER_TYPE RegType = D3DSI_GETREGTYPE(DclRegister);`
- 418: enum `D3DSPR_SAMPLER` | `case D3DSPR_SAMPLER:`
- 430: enum `D3DSP_TEXTURETYPE_MASK` | `(D3DSAMPLER_TEXTURE_TYPE)(DclDesc & D3DSP_TEXTURETYPE_MASK);`
- 434: enum `D3DSPR_INPUT` | `case D3DSPR_INPUT:`
- 445: enum `D3DDECLUSAGE_POSITION` | `D3DDECLUSAGE Usage = D3DDECLUSAGE_POSITION;`
- 448: enum `D3DPS_VERSION` | `if( D3DPS_VERSION(3,0) <= Version )`
- 450: enum `D3DSI_GETUSAGE` | `Usage = (D3DDECLUSAGE)D3DSI_GETUSAGE(DclDesc);`
- 451: enum `D3DSI_GETUSAGEINDEX` | `Index = (BYTE)D3DSI_GETUSAGEINDEX(DclDesc);`
- 455: enum `D3DDECLUSAGE_COLOR` | `Usage = D3DDECLUSAGE_COLOR;`
- 460: enum `D3DDECLUSAGE_COLOR` | `Usage = D3DDECLUSAGE_COLOR;`
- 467: enum `D3DDECLUSAGE_COLOR` | `bDoCentroid = ((D3DSPDM_MSAMPCENTROID & DclRegister) \|\| ((D3DPS_VERSION(2,0) <= Version)&&(D3DDECLUSAGE_COLOR == Usage)));`
- 467: enum `D3DPS_VERSION` | `bDoCentroid = ((D3DSPDM_MSAMPCENTROID & DclRegister) \|\| ((D3DPS_VERSION(2,0) <= Version)&&(D3DDECLUSAGE_COLOR == Usage)));`
- 467: enum `D3DSPDM_MSAMPCENTROID` | `bDoCentroid = ((D3DSPDM_MSAMPCENTROID & DclRegister) \|\| ((D3DPS_VERSION(2,0) <= Version)&&(D3DDECLUSAGE_COLOR == Usage)));`
- 469: enum `D3DSP_WRITEMASK_ALL` | `(BYTE)((DclRegister & D3DSP_WRITEMASK_ALL)>>PSTR_COMPONENTMASK_SHIFT),`
- 477: enum `D3DSPR_TEXTURE` | `case D3DSPR_TEXTURE:`
- 488: enum `D3DPS_VERSION` | `Assert(D3DPS_VERSION(3,0) > Version);`
- 489: enum `D3DSPDM_MSAMPCENTROID` | `BOOL bDoCentroid = (D3DSPDM_MSAMPCENTROID & DclRegister);`
- 490: enum `D3DDECLUSAGE_TEXCOORD` | `if(FAILED(m_InputRegDclInfo.AddNewDcl(D3DDECLUSAGE_TEXCOORD,RegNum,PSTRREG_TEXTURE,RegNum,`
- 491: enum `D3DSP_WRITEMASK_ALL` | `(BYTE)((DclRegister & D3DSP_WRITEMASK_ALL)>>PSTR_COMPONENTMASK_SHIFT),bDoCentroid)))`
- 502: enum `D3DSIO_NOP` | `&& (Inst & D3DSI_OPCODE_MASK) == D3DSIO_NOP)`
- 502: enum `D3DSI_OPCODE_MASK` | `&& (Inst & D3DSI_OPCODE_MASK) == D3DSIO_NOP)`
- 607: enum `D3DPS_VERSION` | `if( (D3DPS_VERSION(1,3) >= *pCode) \|\|`
- 608: enum `D3DPS_VERSION` | `(D3DPS_VERSION(254,254) == *pCode ) )`
- 621: enum `D3DPS_END` | `while (*pToken != D3DPS_END())`
- 640: enum `D3DSI_OPCODE_MASK` | `switch( (*pToken) & D3DSI_OPCODE_MASK )`
- 642: enum `D3DSIO_COMMENT` | `case D3DSIO_COMMENT:`
- 648: enum `D3DSI_COMMENTSIZE_MASK` | `pInst->CommentSize = ((*pToken) & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT;`
- 648: enum `D3DSI_COMMENTSIZE_SHIFT` | `pInst->CommentSize = ((*pToken) & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT;`
- 652: enum `D3DSIO_DEF` | `case D3DSIO_DEF:`
- 658: enum `D3DSI_GETREGTYPE_RESOLVING_CONSTANTS` | `switch(D3DSI_GETREGTYPE_RESOLVING_CONSTANTS(pInst->DstParam))`
- 660: enum `D3DSPR_CONST` | `case D3DSPR_CONST:`
- 672: enum `D3DSI_GETREGNUM_RESOLVING_CONSTANTS` | `m_pConstDefsF[CurrConstDefF].RegNum = D3DSI_GETREGNUM_RESOLVING_CONSTANTS(pInst->DstParam);`
- 697: enum `D3DSIO_DEFI` | `case D3DSIO_DEFI:`
- 702: enum `D3DSI_GETREGTYPE_RESOLVING_CONSTANTS` | `switch(D3DSI_GETREGTYPE_RESOLVING_CONSTANTS(pInst->DstParam))`
- 704: enum `D3DSPR_CONSTINT` | `case D3DSPR_CONSTINT:`
- 715: enum `D3DSI_GETREGNUM` | `m_pConstDefsI[CurrConstDefI].RegNum = D3DSI_GETREGNUM(pInst->DstParam);`
- 737: enum `D3DSIO_DEFB` | `case D3DSIO_DEFB:`
- 742: enum `D3DSI_GETREGTYPE` | `switch(D3DSI_GETREGTYPE(pInst->DstParam))`
- 744: enum `D3DSPR_CONSTBOOL` | `case D3DSPR_CONSTBOOL:`
- 755: enum `D3DSI_GETREGNUM` | `m_pConstDefsB[CurrConstDefB].RegNum = D3DSI_GETREGNUM(pInst->DstParam);`
- 773: enum `D3DSIO_NOP` | `case D3DSIO_NOP:`
- 780: enum `D3DSHADER_INSTRUCTION_PREDICATED` | `pInst->bPredicated = D3DSHADER_INSTRUCTION_PREDICATED & *pToken;`
- 785: enum `D3DSI_OPCODE_MASK` | `switch(D3DSI_OPCODE_MASK & pInst->Opcode)`
- 787: enum `D3DSIO_DCL` | `case D3DSIO_DCL:`
- 811: enum `D3DSI_OPCODE_MASK` | `switch(D3DSI_OPCODE_MASK & pInst->Opcode)`
- 813: enum `D3DSIO_CALL` | `case D3DSIO_CALL:`
- 814: enum `D3DSIO_REP` | `case D3DSIO_REP:`
- 815: enum `D3DSIO_LOOP` | `case D3DSIO_LOOP:`
- 816: enum `D3DSIO_CALLNZ` | `case D3DSIO_CALLNZ:`
- 817: enum `D3DSIO_IF` | `case D3DSIO_IF:`
- 818: enum `D3DSIO_IFC` | `case D3DSIO_IFC:`
- 819: enum `D3DSIO_BREAKC` | `case D3DSIO_BREAKC:`
- 820: enum `D3DSIO_BREAKP` | `case D3DSIO_BREAKP:`
- 821: enum `D3DSIO_LABEL` | `case D3DSIO_LABEL:`
- 862: enum `D3DSHADER_ADDRMODE_RELATIVE` | `if( D3DSHADER_ADDRMODE_RELATIVE == D3DSI_GETADDRESSMODE(pInst->SrcParam[pInst->SrcParamCount]) )`
- 862: enum `D3DSI_GETADDRESSMODE` | `if( D3DSHADER_ADDRMODE_RELATIVE == D3DSI_GETADDRESSMODE(pInst->SrcParam[pInst->SrcParamCount]) )`
- 879: enum `D3DPS_VERSION` | `if( Version < D3DPS_VERSION(2,0) )`
- 883: enum `D3DSI_GETREGTYPE` | `if( D3DSI_GETREGTYPE(pInst->SrcParam[pInst->SrcParamCount]) == D3DSPR_INPUT )`
- 883: enum `D3DSPR_INPUT` | `if( D3DSI_GETREGTYPE(pInst->SrcParam[pInst->SrcParamCount]) == D3DSPR_INPUT )`
- 885: enum `D3DSI_GETREGNUM` | `UINT RegNum = D3DSI_GETREGNUM(pInst->SrcParam[pInst->SrcParamCount]);`
- 888: enum `D3DDECLUSAGE_COLOR` | `if(FAILED(m_InputRegDclInfo.AddNewDcl(D3DDECLUSAGE_COLOR,RegNum,`
- 904: enum `D3DSI_OPCODE_MASK` | `switch (pInst->Opcode & D3DSI_OPCODE_MASK)`
- 907: enum `D3DSIO_TEXBEM_LEGACY` | `case D3DSIO_TEXBEM_LEGACY:`
- 908: enum `D3DSIO_TEXBEML_LEGACY` | `case D3DSIO_TEXBEML_LEGACY:`
- 910: enum `D3DSIO_TEXCOORD` | `case D3DSIO_TEXCOORD:`
- 911: enum `D3DSIO_TEXKILL` | `case D3DSIO_TEXKILL:`
- 912: enum `D3DSIO_TEX` | `case D3DSIO_TEX:`
- 913: enum `D3DSIO_TEXBEM` | `case D3DSIO_TEXBEM:`
- 914: enum `D3DSIO_TEXBEML` | `case D3DSIO_TEXBEML:`
- 915: enum `D3DSIO_TEXLDD` | `case D3DSIO_TEXLDD:`
- 916: enum `D3DSIO_TEXLDL` | `case D3DSIO_TEXLDL:`
- 917: enum `D3DSIO_TEXREG2AR` | `case D3DSIO_TEXREG2AR:`
- 918: enum `D3DSIO_TEXREG2GB` | `case D3DSIO_TEXREG2GB:`
- 926: enum `D3DSIO_TEXDP3` | `case D3DSIO_TEXDP3:`
- 927: enum `D3DSIO_TEXREG2RGB` | `case D3DSIO_TEXREG2RGB:`
- 928: enum `D3DSIO_TEXDEPTH` | `case D3DSIO_TEXDEPTH:`
- 929: enum `D3DSIO_TEXDP3TEX` | `case D3DSIO_TEXDP3TEX:`
- 940: enum `D3DSI_GETREGNUM` | `max(m_cActiveTextureStages,D3DSI_GETREGNUM(pInst->DstParam)+1);`
- 941: enum `D3DSI_GETREGNUM` | `pInst->uiTSSNum = D3DSI_GETREGNUM(pInst->DstParam)-1;`
- 943: enum `D3DSI_GETREGNUM` | `UINT CoordSet = D3DSI_GETREGNUM(pInst->DstParam);`
- 953: enum `D3DSI_OPCODE_MASK` | `switch(pInst->Opcode & D3DSI_OPCODE_MASK)`
- 955: enum `D3DSIO_TEXBEM` | `case D3DSIO_TEXBEM:`
- 956: enum `D3DSIO_TEXBEML` | `case D3DSIO_TEXBEML:`
- 958: enum `D3DSI_GETREGNUM` | `Stage = D3DSI_GETREGNUM(pInst->DstParam);`
- 963: enum `D3DDECLUSAGE_TEXCOORD` | `if(FAILED(m_InputRegDclInfo.AddNewDcl(D3DDECLUSAGE_TEXCOORD,CoordSet,`
- 975: enum `D3DSIO_TEXDEPTH` | `case D3DSIO_TEXDEPTH:`
- 978: enum `D3DSIO_TEXCOORD` | `case D3DSIO_TEXCOORD:`
- 982: enum `D3DSI_GETREGNUM` | `Stage = D3DSI_GETREGNUM(pInst->DstParam);`
- 984: enum `D3DSI_GETREGTYPE` | `if( D3DSPR_TEXTURE == D3DSI_GETREGTYPE(pInst->DstParam) )`
- 984: enum `D3DSPR_TEXTURE` | `if( D3DSPR_TEXTURE == D3DSI_GETREGTYPE(pInst->DstParam) )`
- 986: enum `D3DSI_GETREGNUM` | `UINT CoordSet = D3DSI_GETREGNUM(pInst->DstParam);`
- 990: enum `D3DDECLUSAGE_TEXCOORD` | `if(FAILED(m_InputRegDclInfo.AddNewDcl(D3DDECLUSAGE_TEXCOORD,CoordSet,`
- 1002: enum `D3DSI_GETREGTYPE` | `(D3DSPR_TEXTURE == D3DSI_GETREGTYPE(pInst->SrcParam[0])))`
- 1002: enum `D3DSPR_TEXTURE` | `(D3DSPR_TEXTURE == D3DSI_GETREGTYPE(pInst->SrcParam[0])))`
- 1004: enum `D3DSI_GETREGNUM` | `UINT CoordSet = D3DSI_GETREGNUM(pInst->SrcParam[0]);`
- 1008: enum `D3DDECLUSAGE_TEXCOORD` | `if(FAILED(m_InputRegDclInfo.AddNewDcl(D3DDECLUSAGE_TEXCOORD,CoordSet,`
- 1020: enum `D3DSIO_TEXKILL` | `case D3DSIO_TEXKILL:`
- 1024: enum `D3DSI_GETREGNUM` | `Stage = D3DSI_GETREGNUM(pInst->DstParam);`
- 1026: enum `D3DPS_VERSION` | `if( D3DPS_VERSION(2,0) > Version )`
- 1028: enum `D3DSI_GETREGTYPE` | `if( D3DSPR_TEXTURE == D3DSI_GETREGTYPE(pInst->DstParam) )`
- 1028: enum `D3DSPR_TEXTURE` | `if( D3DSPR_TEXTURE == D3DSI_GETREGTYPE(pInst->DstParam) )`
- 1030: enum `D3DSI_GETREGNUM` | `UINT CoordSet = D3DSI_GETREGNUM(pInst->DstParam);`
- 1034: enum `D3DDECLUSAGE_TEXCOORD` | `if(FAILED(m_InputRegDclInfo.AddNewDcl(D3DDECLUSAGE_TEXCOORD,CoordSet,`
- 1047: enum `D3DSIO_TEX` | `case D3DSIO_TEX:`
- 1049: enum `D3DSI_GETREGNUM` | `Stage = (2 <= pInst->SrcParamCount) ? D3DSI_GETREGNUM(pInst->SrcParam[1]) :`
- 1050: enum `D3DSI_GETREGNUM` | `D3DSI_GETREGNUM(pInst->DstParam);`
- 1058: enum `D3DDECLUSAGE_TEXCOORD` | `if(FAILED(m_InputRegDclInfo.AddNewDcl(D3DDECLUSAGE_TEXCOORD,CoordSet,`
- 1069: enum `D3DPS_VERSION` | `else if( (D3DPS_VERSION(2,0)>Version) &&`
- 1070: enum `D3DSI_GETREGTYPE` | `(D3DSPR_TEXTURE == D3DSI_GETREGTYPE(pInst->SrcParam[0])) )`
- 1070: enum `D3DSPR_TEXTURE` | `(D3DSPR_TEXTURE == D3DSI_GETREGTYPE(pInst->SrcParam[0])) )`
- 1072: enum `D3DSI_GETREGNUM` | `UINT CoordSet = D3DSI_GETREGNUM(pInst->SrcParam[0]);`
- 1076: enum `D3DDECLUSAGE_TEXCOORD` | `if(FAILED(m_InputRegDclInfo.AddNewDcl(D3DDECLUSAGE_TEXCOORD,CoordSet,`
- 1090: enum `D3DSIO_TEXLDD` | `case D3DSIO_TEXLDD:`
- 1091: enum `D3DSI_GETREGNUM` | `Stage = D3DSI_GETREGNUM(pInst->SrcParam[1]);`
- 1093: enum `D3DSIO_TEXLDL` | `case D3DSIO_TEXLDL:`
- 1094: enum `D3DSI_GETREGNUM` | `Stage = D3DSI_GETREGNUM(pInst->SrcParam[1]);`
- 1097: enum `D3DPS_VERSION` | `if( D3DPS_VERSION(2,0) > Version)`
- 1099: enum `D3DSI_GETREGNUM` | `Stage = D3DSI_GETREGNUM(pInst->DstParam);`
- 1104: enum `D3DDECLUSAGE_TEXCOORD` | `if(FAILED(m_InputRegDclInfo.AddNewDcl(D3DDECLUSAGE_TEXCOORD,CoordSet,`
- 1133: enum `D3DPS_VERSION` | `if( ((D3DPS_VERSION(2,0) > Version) \|\| (D3DPS_VERSION(254,254) == Version) ) \|\|`
- 1133: enum `D3DPS_VERSION` | `if( ((D3DPS_VERSION(2,0) > Version) \|\| (D3DPS_VERSION(254,254) == Version) ) \|\|`
- 1134: enum `D3DSTT_UNKNOWN` | `(m_SamplerRegDcl[Stage] != D3DSTT_UNKNOWN))`
- 1152: enum `D3DSI_COISSUE` | `if( pInst->Opcode & D3DSI_COISSUE )`
- 1159: enum `D3DSI_COISSUE` | `if( !(pInst->Opcode & D3DSI_COISSUE) && (pPrevious_NonTrivial_Inst->Opcode & D3DSI_COISSUE) )`
- 1159: enum `D3DSI_COISSUE` | `if( !(pInst->Opcode & D3DSI_COISSUE) && (pPrevious_NonTrivial_Inst->Opcode & D3DSI_COISSUE) )`
- 1189: enum `D3DPS_END` | `pInst->Opcode = D3DPS_END();`
- 1191: enum `D3DSI_COISSUE` | `if(pPrevious_NonTrivial_Inst && (pPrevious_NonTrivial_Inst->Opcode & D3DSI_COISSUE))`
- 1202: enum `D3DDECLUSAGE_TEXCOORD` | `if(FAILED(m_InputRegDclInfo.AddNewDcl(D3DDECLUSAGE_TEXCOORD,CoordSet,`
- 1254: enum `D3DPS_VERSION` | `if( (D3DPS_VERSION(3,0) <= Version) &&`
- 1255: enum `D3DPS_VERSION` | `(D3DPS_VERSION(254,254) != Version))`
- 1283: enum `D3DPS_VERSION` | `if((D3DPS_VERSION(2,0)>Version)\|\|(Version==D3DPS_VERSION(254,254)))`
- 1283: enum `D3DPS_VERSION` | `if((D3DPS_VERSION(2,0)>Version)\|\|(Version==D3DPS_VERSION(254,254)))`
- 1287: enum `D3DPS_VERSION` | `if(D3DPS_VERSION(1,4)>Version\|\|(Version==D3DPS_VERSION(254,254)))`
- 1287: enum `D3DPS_VERSION` | `if(D3DPS_VERSION(1,4)>Version\|\|(Version==D3DPS_VERSION(254,254)))`
- 1312: enum `D3DSI_OPCODE_MASK` | `DWORD   Opcode = m_pWorkerData->pInst->Opcode & D3DSI_OPCODE_MASK;`
- 1313: enum `D3DSP_OPCODESPECIFICCONTROL_MASK` | `DWORD   OpcodeSpecificControl = m_pWorkerData->pInst->Opcode & D3DSP_OPCODESPECIFICCONTROL_MASK;`
- 1349: enum `D3DSPSM_NOT` | `m_pWorkerData->PredicateInfo.bInvertPredicate = (D3DSPSM_NOT == (D3DSP_SRCMOD_MASK & m_pWorkerData->pInst->SrcPredicateToken));`
- 1349: enum `D3DSP_SRCMOD_MASK` | `m_pWorkerData->PredicateInfo.bInvertPredicate = (D3DSPSM_NOT == (D3DSP_SRCMOD_MASK & m_pWorkerData->pInst->SrcPredicateToken));`
- 1350: enum `D3DSP_SWIZZLE_MASK` | `m_pWorkerData->PredicateInfo.PredicateSwizzle = (BYTE)((D3DSP_SWIZZLE_MASK & m_pWorkerData->pInst->SrcPredicateToken) >> D3DSP_SWIZZLE_SH...`
- 1350: enum `D3DSP_SWIZZLE_SHIFT` | `m_pWorkerData->PredicateInfo.PredicateSwizzle = (BYTE)((D3DSP_SWIZZLE_MASK & m_pWorkerData->pInst->SrcPredicateToken) >> D3DSP_SWIZZLE_SH...`
- 1351: enum `D3DSI_GETREGTYPE` | `Assert(D3DSPR_PREDICATE == D3DSI_GETREGTYPE(m_pWorkerData->pInst->SrcPredicateToken));`
- 1351: enum `D3DSPR_PREDICATE` | `Assert(D3DSPR_PREDICATE == D3DSI_GETREGTYPE(m_pWorkerData->pInst->SrcPredicateToken));`
- 1352: enum `D3DSI_GETREGNUM` | `m_pWorkerData->PredicateInfo.PredicateReg._Set(PSTRREG_PREDICATE,D3DSI_GETREGNUM(m_pWorkerData->pInst->SrcPredicateToken));`
- 1363: enum `D3DSIO_LABEL` | `/*case D3DSIO_LABEL:`
- 1371: enum `D3DSI_GETREGNUM` | `m_LabelTracker.AddLabel(D3DSI_GETREGNUM(m_pWorkerData->pInst->SrcParam[0]),`
- 1377: enum `D3DSIO_NOP` | `case D3DSIO_NOP:`
- 1383: enum `D3DSIO_DEF` | `case D3DSIO_DEF:`
- 1384: enum `D3DSIO_DEFI` | `case D3DSIO_DEFI:`
- 1385: enum `D3DSIO_DEFB` | `case D3DSIO_DEFB:`
- 1388: enum `D3DSIO_COMMENT` | `case D3DSIO_COMMENT:`
- 1389: enum `D3DSIO_PHASE` | `case D3DSIO_PHASE:`
- 1390: enum `D3DSIO_DCL` | `case D3DSIO_DCL:`
- 1396: enum `D3DSIO_END` | `case D3DSIO_END:`
- 1409: enum `D3DSIO_ELSE` | `case D3DSIO_ELSE:`
- 1410: enum `D3DSIO_ENDIF` | `case D3DSIO_ENDIF:`
- 1421: enum `D3DSI_GETREGNUM` | `UINT RegNum = D3DSI_GETREGNUM(m_pWorkerData->pInst->DstParam);`
- 1422: enum `D3DSI_GETREGTYPE` | `switch (D3DSI_GETREGTYPE(m_pWorkerData->pInst->DstParam))`
- 1424: enum `D3DSPR_TEXTURE` | `case D3DSPR_TEXTURE:`
- 1426: enum `D3DSPR_TEMP` | `case D3DSPR_TEMP:`
- 1428: enum `D3DSPR_COLOROUT` | `case D3DSPR_COLOROUT:`
- 1430: enum `D3DSIO_TEXKILL` | `if( D3DSIO_TEXKILL != Opcode )`
- 1435: enum `D3DSPR_PREDICATE` | `case D3DSPR_PREDICATE:`
- 1439: enum `D3DSPR_DEPTHOUT` | `case D3DSPR_DEPTHOUT:`
- 1444: enum `D3DSPR_INPUT` | `case D3DSPR_INPUT:`
- 1445: enum `D3DSIO_TEXKILL` | `if( D3DSIO_TEXKILL == Opcode )`
- 1455: enum `D3DSP_WRITEMASK_ALL` | `DstWriteMask = (BYTE)(m_pWorkerData->pInst->DstParam ? (m_pWorkerData->pInst->DstParam & D3DSP_WRITEMASK_ALL) >> PSTR_COMPONENTMASK_SHIFT`
- 1471: enum `D3DSI_GETREGNUM_RESOLVING_CONSTANTS` | `RegNum = D3DSI_GETREGNUM_RESOLVING_CONSTANTS(m_pWorkerData->pInst->SrcParam[i]);`
- 1472: enum `D3DSI_GETREGTYPE_RESOLVING_CONSTANTS` | `switch (D3DSI_GETREGTYPE_RESOLVING_CONSTANTS(m_pWorkerData->pInst->SrcParam[i]))`
- 1474: enum `D3DSPR_TEMP` | `case D3DSPR_TEMP:`
- 1476: enum `D3DSPR_TEXTURE` | `case D3DSPR_TEXTURE:`
- 1481: enum `D3DPS_VERSION` | `(D3DPS_VERSION(2,0) > Version)`
- 1483: enum `D3DPS_VERSION` | `(D3DPS_VERSION(254,254) == Version)));`
- 1485: enum `D3DSPR_INPUT` | `case D3DSPR_INPUT:`
- 1486: enum `D3DSHADER_ADDRMODE_RELATIVE` | `if( D3DSHADER_ADDRMODE_RELATIVE == D3DSI_GETADDRESSMODE(m_pWorkerData->pInst->SrcParam[i]) )`
- 1486: enum `D3DSI_GETADDRESSMODE` | `if( D3DSHADER_ADDRMODE_RELATIVE == D3DSI_GETADDRESSMODE(m_pWorkerData->pInst->SrcParam[i]) )`
- 1490: enum `D3DSI_GETREGTYPE` | `switch(D3DSI_GETREGTYPE(m_pWorkerData->pInst->SrcParamRelAddr[i]))`
- 1492: enum `D3DSPR_LOOP` | `case D3DSPR_LOOP:`
- 1495: enum `D3DSI_GETREGNUM` | `Assert(0 == D3DSI_GETREGNUM(m_pWorkerData->pInst->SrcParamRelAddr[i]));`
- 1502: enum `D3DSI_GETSWIZZLE` | `(BYTE)_SelectorFromSwizzle(D3DSI_GETSWIZZLE(m_pWorkerData->pInst->SrcParamRelAddr[i])>>D3DSP_SWIZZLE_SHIFT));`
- 1502: enum `D3DSP_SWIZZLE_SHIFT` | `(BYTE)_SelectorFromSwizzle(D3DSI_GETSWIZZLE(m_pWorkerData->pInst->SrcParamRelAddr[i])>>D3DSP_SWIZZLE_SHIFT));`
- 1507: enum `D3DPS_VERSION` | `Assert((D3DPS_VERSION(2,0) > Version) \|\|`
- 1512: enum `D3DSPR_CONST` | `case D3DSPR_CONST:`
- 1514: enum `D3DPS_VERSION` | `if( D3DPS_VERSION(2,0) > Version )`
- 1522: enum `D3DSPR_CONSTINT` | `case D3DSPR_CONSTINT:`
- 1525: enum `D3DSPR_CONSTBOOL` | `case D3DSPR_CONSTBOOL:`
- 1528: enum `D3DSPR_MISCTYPE` | `case D3DSPR_MISCTYPE:`
- 1539: enum `D3DSPR_SAMPLER` | `case D3DSPR_SAMPLER:`
- 1544: enum `D3DSPR_LOOP` | `case D3DSPR_LOOP:`
- 1545: enum `D3DSPR_LABEL` | `case D3DSPR_LABEL:`
- 1547: enum `D3DSPR_PREDICATE` | `case D3DSPR_PREDICATE:`
- 1555: enum `D3DSPSM_DZ` | `if( (D3DSPSM_DZ == (m_pWorkerData->pInst->SrcParam[i] & D3DSP_SRCMOD_MASK)) \|\|`
- 1555: enum `D3DSP_SRCMOD_MASK` | `if( (D3DSPSM_DZ == (m_pWorkerData->pInst->SrcParam[i] & D3DSP_SRCMOD_MASK)) \|\|`
- 1556: enum `D3DSPSM_DW` | `(D3DSPSM_DW == (m_pWorkerData->pInst->SrcParam[i] & D3DSP_SRCMOD_MASK)) )`
- 1556: enum `D3DSP_SRCMOD_MASK` | `(D3DSPSM_DW == (m_pWorkerData->pInst->SrcParam[i] & D3DSP_SRCMOD_MASK)) )`
- 1558: enum `D3DPS_VERSION` | `Assert( D3DPS_VERSION(1,4) == Version);`
- 1570: enum `D3DSP_SRCMOD_MASK` | `switch (m_pWorkerData->pInst->SrcParam[i] & D3DSP_SRCMOD_MASK)`
- 1573: enum `D3DSPSM_NONE` | `case D3DSPSM_NONE:`
- 1577: enum `D3DSPSM_NEG` | `case D3DSPSM_NEG:`
- 1582: enum `D3DSPSM_NOT` | `case D3DSPSM_NOT:`
- 1586: enum `D3DSPSM_BIASNEG` | `case D3DSPSM_BIASNEG:`
- 1588: enum `D3DSPSM_BIAS` | `case D3DSPSM_BIAS:`
- 1591: enum `D3DSPSM_SIGNNEG` | `case D3DSPSM_SIGNNEG:`
- 1593: enum `D3DSPSM_SIGN` | `case D3DSPSM_SIGN:`
- 1597: enum `D3DSPSM_COMP` | `case D3DSPSM_COMP:`
- 1600: enum `D3DSPSM_X2NEG` | `case D3DSPSM_X2NEG:`
- 1602: enum `D3DSPSM_X2` | `case D3DSPSM_X2:`
- 1605: enum `D3DSPSM_ABSNEG` | `case D3DSPSM_ABSNEG:`
- 1607: enum `D3DSPSM_ABS` | `case D3DSPSM_ABS:`
- 1616: enum `D3DSP_SWIZZLE_MASK` | `SrcSwizzle[i] = (BYTE)((m_pWorkerData->pInst->SrcParam[i] & D3DSP_SWIZZLE_MASK) >> D3DSP_SWIZZLE_SHIFT);`
- 1616: enum `D3DSP_SWIZZLE_SHIFT` | `SrcSwizzle[i] = (BYTE)((m_pWorkerData->pInst->SrcParam[i] & D3DSP_SWIZZLE_MASK) >> D3DSP_SWIZZLE_SHIFT);`
- 1617: enum `D3DSP_NOSWIZZLE` | `bEmitSwizzle[i] = (D3DSP_NOSWIZZLE != (m_pWorkerData->pInst->SrcParam[i] & D3DSP_SWIZZLE_MASK));`
- 1617: enum `D3DSP_SWIZZLE_MASK` | `bEmitSwizzle[i] = (D3DSP_NOSWIZZLE != (m_pWorkerData->pInst->SrcParam[i] & D3DSP_SWIZZLE_MASK));`
- 1620: enum `D3DSI_GETREGTYPE` | `(D3DSPR_SAMPLER == (D3DSI_GETREGTYPE(m_pWorkerData->pInst->SrcParam[i]))) )`
- 1620: enum `D3DSPR_SAMPLER` | `(D3DSPR_SAMPLER == (D3DSI_GETREGTYPE(m_pWorkerData->pInst->SrcParam[i]))) )`
- 1628: enum `D3DSI_GETREGTYPE` | `(D3DSPR_TEXTURE == (D3DSI_GETREGTYPE(m_pWorkerData->pInst->SrcParam[i]))) &&`
- 1628: enum `D3DSPR_TEXTURE` | `(D3DSPR_TEXTURE == (D3DSI_GETREGTYPE(m_pWorkerData->pInst->SrcParam[i]))) &&`
- 1629: enum `D3DSIO_TEX` | `((D3DSIO_TEXCOORD == Opcode) \|\| (D3DSIO_TEX == Opcode) ) )`
- 1629: enum `D3DSIO_TEXCOORD` | `((D3DSIO_TEXCOORD == Opcode) \|\| (D3DSIO_TEX == Opcode) ) )`
- 1638: enum `D3DSIO_SINCOS` | `case D3DSIO_SINCOS:`
- 1639: enum `D3DSIO_RSQ` | `case D3DSIO_RSQ:`
- 1640: enum `D3DSIO_RCP` | `case D3DSIO_RCP:`
- 1641: enum `D3DSIO_EXP` | `case D3DSIO_EXP:`
- 1642: enum `D3DSIO_LOG` | `case D3DSIO_LOG:`
- 1643: enum `D3DSIO_POW` | `case D3DSIO_POW:`
- 1644: enum `D3DSIO_SINCOS` | `if( (D3DSIO_SINCOS == Opcode) && (0 != i) )`
- 1649: enum `D3DPS_VERSION` | `if( (D3DSIO_SINCOS != Opcode) \|\| (D3DPS_VERSION(2,255) <= Version ) )`
- 1649: enum `D3DSIO_SINCOS` | `if( (D3DSIO_SINCOS != Opcode) \|\| (D3DPS_VERSION(2,255) <= Version ) )`
- 1654: enum `D3DSIO_CRS` | `case D3DSIO_CRS:`
- 1656: enum `D3DSP_NOSWIZZLE` | `Assert( D3DSP_NOSWIZZLE == SrcSwizzle[i]);`
- 1659: enum `D3DSIO_DP3` | `case D3DSIO_DP3:`
- 1660: enum `D3DSIO_DP4` | `case D3DSIO_DP4:`
- 1661: enum `D3DSIO_NRM` | `case D3DSIO_NRM:`
- 1663: enum `D3DSIO_BREAKP` | `case D3DSIO_BREAKP:`
- 1666: enum `D3DSIO_IF` | `case D3DSIO_IF:`
- 1667: enum `D3DSIO_CALLNZ` | `case D3DSIO_CALLNZ:`
- 1668: enum `D3DSIO_IF` | `if( (D3DSIO_IF == Opcode) && (D3DSPR_PREDICATE == D3DSI_GETREGTYPE(m_pWorkerData->pInst->SrcParam[i])) )`
- 1668: enum `D3DSI_GETREGTYPE` | `if( (D3DSIO_IF == Opcode) && (D3DSPR_PREDICATE == D3DSI_GETREGTYPE(m_pWorkerData->pInst->SrcParam[i])) )`
- 1668: enum `D3DSPR_PREDICATE` | `if( (D3DSIO_IF == Opcode) && (D3DSPR_PREDICATE == D3DSI_GETREGTYPE(m_pWorkerData->pInst->SrcParam[i])) )`
- 1673: enum `D3DSIO_CALLNZ` | `if( (D3DSIO_CALLNZ == Opcode) && (D3DSPR_PREDICATE == D3DSI_GETREGTYPE(m_pWorkerData->pInst->SrcParam[i])) && (1==i) )`
- 1673: enum `D3DSI_GETREGTYPE` | `if( (D3DSIO_CALLNZ == Opcode) && (D3DSPR_PREDICATE == D3DSI_GETREGTYPE(m_pWorkerData->pInst->SrcParam[i])) && (1==i) )`
- 1673: enum `D3DSPR_PREDICATE` | `if( (D3DSIO_CALLNZ == Opcode) && (D3DSPR_PREDICATE == D3DSI_GETREGTYPE(m_pWorkerData->pInst->SrcParam[i])) && (1==i) )`
- 1703: enum `D3DSPDM_SATURATE` | `if( D3DSPDM_SATURATE & m_pWorkerData->pInst->DstParam )`
- 1723: enum `D3DSP_DSTSHIFT_MASK` | `(m_pWorkerData->pInst->DstParam & D3DSP_DSTSHIFT_MASK) >> D3DSP_DSTSHIFT_SHIFT;`
- 1723: enum `D3DSP_DSTSHIFT_SHIFT` | `(m_pWorkerData->pInst->DstParam & D3DSP_DSTSHIFT_MASK) >> D3DSP_DSTSHIFT_SHIFT;`
- 1799: enum `D3DSIO_TEXCOORD` | `case D3DSIO_TEXCOORD:`
- 1800: enum `D3DSIO_TEXKILL` | `case D3DSIO_TEXKILL:`
- 1802: enum `D3DPS_VERSION` | `if(D3DPS_VERSION(2,0) > Version )`
- 1804: enum `D3DSIO_TEXKILL` | `if( !(  (D3DSIO_TEXKILL == Opcode)  &&`
- 1805: enum `D3DSI_GETREGTYPE` | `(D3DSPR_TEMP    == (D3DSI_GETREGTYPE(m_pWorkerData->pInst->DstParam)))`
- 1805: enum `D3DSPR_TEMP` | `(D3DSPR_TEMP    == (D3DSI_GETREGTYPE(m_pWorkerData->pInst->DstParam)))`
- 1809: enum `D3DSI_GETREGNUM` | `UINT CoordSet = m_pWorkerData->pInst->SrcParam[0] ? D3DSI_GETREGNUM(m_pWorkerData->pInst->SrcParam[0]) :`
- 1810: enum `D3DSI_GETREGNUM` | `D3DSI_GETREGNUM(m_pWorkerData->pInst->DstParam);`
- 1849: enum `D3DPS_VERSION` | `if ( (D3DSIO_TEXCOORD == Opcode) && (D3DPS_VERSION(1,3) >= Version) )`
- 1849: enum `D3DSIO_TEXCOORD` | `if ( (D3DSIO_TEXCOORD == Opcode) && (D3DPS_VERSION(1,3) >= Version) )`
- 1863: enum `D3DSIO_TEXKILL` | `if( (D3DSIO_TEXKILL == Opcode) )`
- 1867: enum `D3DPS_VERSION` | `_InstParam(PSTRINST_KILL).WriteMask     = (D3DPS_VERSION(2,0) <= Version) ? DstWriteMask :`
- 1882: enum `D3DSIO_TEX` | `case D3DSIO_TEX:`
- 1888: enum `D3DSI_GETREGNUM` | `UINT CoordSet = m_pWorkerData->pInst->SrcParam[0] ? D3DSI_GETREGNUM(m_pWorkerData->pInst->SrcParam[0]) :`
- 1889: enum `D3DSI_GETREGNUM` | `D3DSI_GETREGNUM(m_pWorkerData->pInst->DstParam);`
- 1928: enum `D3DSI_TEXLD_PROJECT` | `else if( D3DSI_TEXLD_PROJECT & OpcodeSpecificControl )`
- 1937: enum `D3DSI_GETREGNUM` | `D3DSI_GETREGNUM(m_pWorkerData->pInst->SrcParam[1]) :`
- 1938: enum `D3DSI_GETREGNUM` | `D3DSI_GETREGNUM(m_pWorkerData->pInst->DstParam);`
- 1943: enum `D3DSTT_2D` | `case D3DSTT_2D:`
- 1946: enum `D3DSTT_CUBE` | `case D3DSTT_CUBE:`
- 1947: enum `D3DSTT_VOLUME` | `case D3DSTT_VOLUME:`
- 1953: enum `D3DSI_TEXLD_BIAS` | `BOOL bAddLODBiasFromTexCoord_W = (D3DSI_TEXLD_BIAS & OpcodeSpecificControl);`
- 2041: enum `D3DSP_NOSWIZZLE` | `if( m_pWorkerData->pInst->SrcParam[1] && (D3DSP_NOSWIZZLE != (D3DSP_SWIZZLE_MASK & m_pWorkerData->pInst->SrcParam[1])) )`
- 2041: enum `D3DSP_SWIZZLE_MASK` | `if( m_pWorkerData->pInst->SrcParam[1] && (D3DSP_NOSWIZZLE != (D3DSP_SWIZZLE_MASK & m_pWorkerData->pInst->SrcParam[1])) )`
- 2076: enum `D3DSIO_TEXDP3` | `case D3DSIO_TEXDP3:`
- 2077: enum `D3DSIO_TEXDP3TEX` | `case D3DSIO_TEXDP3TEX:`
- 2080: enum `D3DSI_GETREGNUM` | `CoordReg._Set(PSTRREG_TEXTURE,D3DSI_GETREGNUM(m_pWorkerData->pInst->DstParam));`
- 2082: enum `D3DSIO_TEXDP3` | `if( D3DSIO_TEXDP3 == Opcode )`
- 2096: enum `D3DSI_GETREGNUM` | `UINT uiStage = D3DSI_GETREGNUM(m_pWorkerData->pInst->DstParam);`
- 2158: enum `D3DSIO_TEXREG2AR` | `case D3DSIO_TEXREG2AR:`
- 2159: enum `D3DSIO_TEXREG2GB` | `case D3DSIO_TEXREG2GB:`
- 2160: enum `D3DSIO_TEXREG2RGB` | `case D3DSIO_TEXREG2RGB:`
- 2166: enum `D3DSI_GETREGNUM` | `UINT            uiStage = D3DSI_GETREGNUM(m_pWorkerData->pInst->DstParam);`
- 2172: enum `D3DSIO_TEXREG2AR` | `case D3DSIO_TEXREG2AR:`
- 2177: enum `D3DSIO_TEXREG2GB` | `case D3DSIO_TEXREG2GB:`
- 2182: enum `D3DSIO_TEXREG2RGB` | `case D3DSIO_TEXREG2RGB:`
- 2188: enum `D3DSIO_TEXREG2AR` | `if( (D3DSIO_TEXREG2AR == Opcode) \|\| (D3DSIO_TEXREG2GB == Opcode) )`
- 2188: enum `D3DSIO_TEXREG2GB` | `if( (D3DSIO_TEXREG2AR == Opcode) \|\| (D3DSIO_TEXREG2GB == Opcode) )`
- 2251: enum `D3DSIO_TEXBEM` | `case D3DSIO_TEXBEM:`
- 2252: enum `D3DSIO_TEXBEML` | `case D3DSIO_TEXBEML:`
- 2253: enum `D3DSIO_TEXBEM_LEGACY` | `case D3DSIO_TEXBEM_LEGACY:`
- 2254: enum `D3DSIO_TEXBEML_LEGACY` | `case D3DSIO_TEXBEML_LEGACY:`
- 2256: enum `D3DSIO_TEXBEML` | `BOOL bDoLuminance = ((D3DSIO_TEXBEML == Opcode) \|\| (D3DSIO_TEXBEML_LEGACY == Opcode));`
- 2256: enum `D3DSIO_TEXBEML_LEGACY` | `BOOL bDoLuminance = ((D3DSIO_TEXBEML == Opcode) \|\| (D3DSIO_TEXBEML_LEGACY == Opcode));`
- 2258: enum `D3DSI_GETREGNUM` | `UINT uiStage = D3DSI_GETREGNUM(m_pWorkerData->pInst->DstParam);`
- 2259: enum `D3DSI_GETREGNUM` | `CoordReg._Set(PSTRREG_TEXTURE,D3DSI_GETREGNUM(m_pWorkerData->pInst->DstParam));`
- 2324: enum `D3DSIO_TEXDEPTH` | `case D3DSIO_TEXDEPTH:`
- 2381: enum `D3DSIO_TEXLDD` | `case D3DSIO_TEXLDD:`
- 2383: enum `D3DSI_GETREGNUM` | `UINT        uiSampler = D3DSI_GETREGNUM(m_pWorkerData->pInst->SrcParam[1]);`
- 2433: enum `D3DSP_NOSWIZZLE` | `if( D3DSP_NOSWIZZLE != (D3DSP_SWIZZLE_MASK & m_pWorkerData->pInst->SrcParam[1]) )`
- 2433: enum `D3DSP_SWIZZLE_MASK` | `if( D3DSP_NOSWIZZLE != (D3DSP_SWIZZLE_MASK & m_pWorkerData->pInst->SrcParam[1]) )`
- 2470: enum `D3DSIO_TEXLDL` | `case D3DSIO_TEXLDL:`
- 2472: enum `D3DSI_GETREGNUM` | `UINT        uiSampler = D3DSI_GETREGNUM(m_pWorkerData->pInst->SrcParam[1]);`
- 2494: enum `D3DSP_NOSWIZZLE` | `if( D3DSP_NOSWIZZLE != (D3DSP_SWIZZLE_MASK & m_pWorkerData->pInst->SrcParam[1]) )`
- 2494: enum `D3DSP_SWIZZLE_MASK` | `if( D3DSP_NOSWIZZLE != (D3DSP_SWIZZLE_MASK & m_pWorkerData->pInst->SrcParam[1]) )`
- 2532: enum `D3DSI_GETREGNUM` | `CoordReg._Set(PSTRREG_TEXTURE,D3DSI_GETREGNUM(m_pWorkerData->pInst->DstParam));`
- 2547: enum `D3DSI_OPCODE_MASK` | `BOOL bSecondPad = (D3DSIO_TEXM3x3PAD != ((m_pWorkerData->pInst + 1)->Opcode & D3DSI_OPCODE_MASK));`
- 2548: enum `D3DSI_OPCODE_MASK` | `BOOL bInVSPECSequence = (D3DSIO_TEXM3x3VSPEC == (((m_pWorkerData->pInst + (bSecondPad?1:2))->Opcode) & D3DSI_OPCODE_MASK));`
- 2550: enum `D3DSI_GETREGNUM` | `CoordReg._Set(PSTRREG_TEXTURE,D3DSI_GETREGNUM(m_pWorkerData->pInst->DstParam));`
- 2585: enum `D3DSI_GETREGNUM` | `UINT uiStage = D3DSI_GETREGNUM(m_pWorkerData->pInst->DstParam);`
- 2586: enum `D3DSI_GETREGNUM` | `CoordReg._Set(PSTRREG_TEXTURE,D3DSI_GETREGNUM(m_pWorkerData->pInst->DstParam));`
- 2587: enum `D3DSI_GETREGNUM` | `XFormCoordReg._Set(PSTRREG_TEXTURE,D3DSI_GETREGNUM(m_pWorkerData->pInst->DstParam)-(bIs3D?2:1));`
- 2868: enum `D3DSI_GETREGNUM` | `_InstParam(PSTRINST_SAMPLE).uiStage         = D3DSI_GETREGNUM(m_pWorkerData->pInst->DstParam);`
- 2879: enum `D3DSIO_ABS` | `case D3DSIO_ABS:`
- 2887: enum `D3DSIO_ADD` | `case D3DSIO_ADD:`
- 2898: enum `D3DSIO_BEM` | `case D3DSIO_BEM:`
- 2906: enum `D3DSI_GETREGNUM` | `_InstParam(PSTRINST_BEM).uiStage            = D3DSI_GETREGNUM(m_pWorkerData->pInst->DstParam);`
- 2909: enum `D3DSIO_BREAK` | `case D3DSIO_BREAK:`
- 2921: enum `D3DSIO_BREAKC` | `case D3DSIO_BREAKC:`
- 2928: enum `D3DSI_GETCOMPARISON` | `_InstParam(PSTRINST_SETPRED).Comparison = D3DSI_GETCOMPARISON(OpcodeSpecificControl);`
- 2947: enum `D3DSIO_BREAKP` | `case D3DSIO_BREAKP:`
- 2963: enum `D3DSIO_CALL` | `case D3DSIO_CALL:`
- 2968: enum `D3DSI_GETREGNUM` | `m_LabelTracker.NeedLabel(D3DSI_GETREGNUM(m_pWorkerData->pInst->SrcParam[0]),`
- 2974: enum `D3DSI_GETREGNUM` | `_InstParam(PSTRINST_CALL).Label = D3DSI_GETREGNUM(m_pWorkerData->pInst->SrcParam[0]);`
- 2977: enum `D3DSIO_LABEL` | `case D3DSIO_LABEL:`
- 2979: enum `D3DSI_GETREGNUM` | `_InstParam(PSTRINST_DEFINESUB).Label = D3DSI_GETREGNUM(m_pWorkerData->pInst->SrcParam[0]);`
- 2982: enum `D3DSIO_CALLNZ` | `case D3DSIO_CALLNZ:`
- 2984: enum `D3DSI_GETREGTYPE` | `if( D3DSPR_PREDICATE == D3DSI_GETREGTYPE(m_pWorkerData->pInst->SrcParam[1]) )`
- 2984: enum `D3DSPR_PREDICATE` | `if( D3DSPR_PREDICATE == D3DSI_GETREGTYPE(m_pWorkerData->pInst->SrcParam[1]) )`
- 2987: enum `D3DSI_GETREGNUM` | `_InstParam(PSTRINST_CALLNZ).Label = D3DSI_GETREGNUM(m_pWorkerData->pInst->SrcParam[0]);`
- 2999: enum `D3DSI_GETREGNUM` | `_InstParam(PSTRINST_CALLNZ).Label = D3DSI_GETREGNUM(m_pWorkerData->pInst->SrcParam[0]);`
- 3011: enum `D3DSI_GETREGTYPE` | `if( D3DSPR_PREDICATE == D3DSI_GETREGTYPE(m_pWorkerData->pInst->SrcParam[1]) )`
- 3011: enum `D3DSPR_PREDICATE` | `if( D3DSPR_PREDICATE == D3DSI_GETREGTYPE(m_pWorkerData->pInst->SrcParam[1]) )`
- 3045: enum `D3DSI_GETREGNUM` | `m_LabelTracker.NeedLabel(D3DSI_GETREGNUM(m_pWorkerData->pInst->SrcParam[0]),`
- 3055: enum `D3DSIO_CMP` | `case D3DSIO_CMP:`
- 3068: enum `D3DSIO_CND` | `case D3DSIO_CND:`
- 3081: enum `D3DSIO_CRS` | `case D3DSIO_CRS:`
- 3136: enum `D3DSIO_DP2ADD` | `case D3DSIO_DP2ADD:`
- 3149: enum `D3DSIO_DP3` | `case D3DSIO_DP3:`
- 3160: enum `D3DSIO_DP4` | `case D3DSIO_DP4:`
- 3171: enum `D3DSIO_DSX` | `case D3DSIO_DSX:`
- 3182: enum `D3DSIO_DSY` | `case D3DSIO_DSY:`
- 3193: enum `D3DSIO_ELSE` | `case D3DSIO_ELSE:`
- 3207: enum `D3DSIO_ENDIF` | `case D3DSIO_ENDIF:`
- 3215: enum `D3DSIO_ENDLOOP` | `case D3DSIO_ENDLOOP:`
- 3286: enum `D3DSIO_ENDREP` | `case D3DSIO_ENDREP:`
- 3335: enum `D3DSIO_EXP` | `case D3DSIO_EXP:`
- 3345: enum `D3DSIO_FRC` | `case D3DSIO_FRC:`
- 3354: enum `D3DSIO_IF` | `case D3DSIO_IF:`
- 3359: enum `D3DSI_GETREGTYPE` | `if( D3DSPR_PREDICATE == D3DSI_GETREGTYPE(m_pWorkerData->pInst->SrcParam[0]) )`
- 3359: enum `D3DSPR_PREDICATE` | `if( D3DSPR_PREDICATE == D3DSI_GETREGTYPE(m_pWorkerData->pInst->SrcParam[0]) )`
- 3364: enum `D3DSI_GETREGTYPE` | `else if( D3DSPR_CONSTBOOL == D3DSI_GETREGTYPE(m_pWorkerData->pInst->SrcParam[0]) )`
- 3364: enum `D3DSPR_CONSTBOOL` | `else if( D3DSPR_CONSTBOOL == D3DSI_GETREGTYPE(m_pWorkerData->pInst->SrcParam[0]) )`
- 3407: enum `D3DSIO_IFC` | `case D3DSIO_IFC:`
- 3414: enum `D3DSI_GETCOMPARISON` | `_InstParam(PSTRINST_SETPRED).Comparison = D3DSI_GETCOMPARISON(OpcodeSpecificControl);`
- 3434: enum `D3DSIO_LOG` | `case D3DSIO_LOG:`
- 3444: enum `D3DSIO_LOOP` | `case D3DSIO_LOOP:`
- 3508: enum `D3DSIO_LRP` | `case D3DSIO_LRP:`
- 3631: enum `D3DSIO_MAD` | `case D3DSIO_MAD:`
- 3644: enum `D3DSIO_MAX` | `case D3DSIO_MAX:`
- 3655: enum `D3DSIO_MIN` | `case D3DSIO_MIN:`
- 3666: enum `D3DSIO_MOV` | `case D3DSIO_MOV:`
- 3675: enum `D3DSIO_MUL` | `case D3DSIO_MUL:`
- 3686: enum `D3DSIO_NRM` | `case D3DSIO_NRM:`
- 3715: enum `D3DSIO_POW` | `case D3DSIO_POW:`
- 3753: enum `D3DSIO_RCP` | `case D3DSIO_RCP:`
- 3763: enum `D3DSIO_REP` | `case D3DSIO_REP:`
- 3809: enum `D3DSIO_RET` | `case D3DSIO_RET:`
- 3814: enum `D3DSIO_RSQ` | `case D3DSIO_RSQ:`
- 3824: enum `D3DSIO_SETP` | `case D3DSIO_SETP:`
- 3832: enum `D3DSI_GETCOMPARISON` | `_InstParam(PSTRINST_SETPRED).Comparison         = D3DSI_GETCOMPARISON(OpcodeSpecificControl);`
- 3834: enum `D3DSIO_SUB` | `case D3DSIO_SUB:`
- 3845: enum `D3DSIO_SINCOS` | `case D3DSIO_SINCOS:`
- 3846: enum `D3DPS_VERSION` | `if( D3DPS_VERSION(2,0) == Version )`
- 4025: enum `D3DPS_VERSION` | `if( D3DPS_VERSION(2,0) > Version )`
- 4598: enum `D3DSI_OPCODE_MASK` | `DWORD Opcode = pInst->Opcode & D3DSI_OPCODE_MASK;`
- 4608: enum `D3DSIO_TEX` | `case D3DSIO_TEX:`
- 4609: enum `D3DSIO_TEXLDD` | `case D3DSIO_TEXLDD:`
- 4610: enum `D3DSIO_TEXLDL` | `case D3DSIO_TEXLDL:`
- 4611: enum `D3DPS_VERSION` | `if( D3DPS_VERSION(2,0) <= dwVersion)`
- 4616: enum `D3DSI_GETREGTYPE` | `Assert( D3DSPR_SAMPLER == D3DSI_GETREGTYPE(pInst->SrcParam[1]));`
- 4616: enum `D3DSPR_SAMPLER` | `Assert( D3DSPR_SAMPLER == D3DSI_GETREGTYPE(pInst->SrcParam[1]));`
- 4617: enum `D3DSI_GETREGNUM` | `UINT SamplerNum = D3DSI_GETREGNUM(pInst->SrcParam[1]);`
- 4622: enum `D3DSTT_2D` | `case D3DSTT_2D:`
- 4625: enum `D3DSTT_CUBE` | `case D3DSTT_CUBE:`
- 4626: enum `D3DSTT_VOLUME` | `case D3DSTT_VOLUME:`
- 4632: enum `D3DSI_TEXLD_BIAS` | `if( (pInst->Opcode & D3DSI_TEXLD_PROJECT) \|\| (pInst->Opcode & D3DSI_TEXLD_BIAS) \|\|`
- 4632: enum `D3DSI_TEXLD_PROJECT` | `if( (pInst->Opcode & D3DSI_TEXLD_PROJECT) \|\| (pInst->Opcode & D3DSI_TEXLD_BIAS) \|\|`
- 4633: enum `D3DSIO_TEXLDL` | `(D3DSIO_TEXLDL == Opcode) )`
- 4641: enum `D3DSP_WRITEMASK_ALL` | `NeededComponents = (BYTE)((pInst->DstParam & D3DSP_WRITEMASK_ALL) >> PSTR_COMPONENTMASK_SHIFT);`
- 4643: enum `D3DSIO_TEXLDD` | `else if( (D3DSIO_TEXLDD == Opcode) && (2 <= i) )`
- 4645: enum `D3DSI_GETREGNUM` | `UINT SamplerNum = D3DSI_GETREGNUM(pInst->SrcParam[1]);`
- 4648: enum `D3DSTT_2D` | `case D3DSTT_2D:`
- 4651: enum `D3DSTT_CUBE` | `case D3DSTT_CUBE:`
- 4652: enum `D3DSTT_VOLUME` | `case D3DSTT_VOLUME:`
- 4664: enum `D3DPS_VERSION` | `else if( D3DPS_VERSION(1,4) == dwVersion )`
- 4674: enum `D3DSIO_TEXCOORD` | `case D3DSIO_TEXCOORD:`
- 4675: enum `D3DPS_VERSION` | `if( D3DPS_VERSION(1,4) == dwVersion )`
- 4685: enum `D3DSIO_TEXBEM` | `case D3DSIO_TEXBEM:`
- 4686: enum `D3DSIO_TEXBEML` | `case D3DSIO_TEXBEML:`
- 4689: enum `D3DSIO_DP2ADD` | `case D3DSIO_DP2ADD:`
- 4699: enum `D3DSIO_DP3` | `case D3DSIO_DP3:`
- 4700: enum `D3DSIO_CRS` | `case D3DSIO_CRS:`
- 4703: enum `D3DSIO_DP4` | `case D3DSIO_DP4:`
- 4706: enum `D3DSIO_BEM` | `case D3DSIO_BEM:`
- 4709: enum `D3DSIO_REP` | `case D3DSIO_REP:`
- 4712: enum `D3DSIO_LOOP` | `case D3DSIO_LOOP:`
- 4718: enum `D3DSIO_IF` | `case D3DSIO_IF:`
- 4719: enum `D3DSIO_CALLNZ` | `case D3DSIO_CALLNZ:`
- 4720: enum `D3DSIO_IFC` | `case D3DSIO_IFC:`
- 4721: enum `D3DSIO_BREAKC` | `case D3DSIO_BREAKC:`
- 4722: enum `D3DSIO_BREAKP` | `case D3DSIO_BREAKP:`
- 4741: enum `D3DSIO_SINCOS` | `case D3DSIO_SINCOS:`
- 4742: enum `D3DPS_VERSION` | `if( D3DPS_VERSION(3,0) > dwVersion )`
- 4749: enum `D3DSIO_NRM` | `case D3DSIO_NRM:`
- 4753: enum `D3DSP_WRITEMASK_ALL` | `NeededComponents = (BYTE)((pInst->DstParam & D3DSP_WRITEMASK_ALL) >> PSTR_COMPONENTMASK_SHIFT);`
- 4767: enum `D3DSP_SWIZZLE_MASK` | `ReadComponents \|= ComponentMask[((pInst->SrcParam[i] & D3DSP_SWIZZLE_MASK) >> (D3DVS_SWIZZLE_SHIFT + 2*j)) & 0x3];`
- 4767: enum `D3DVS_SWIZZLE_SHIFT` | `ReadComponents \|= ComponentMask[((pInst->SrcParam[i] & D3DSP_SWIZZLE_MASK) >> (D3DVS_SWIZZLE_SHIFT + 2*j)) & 0x3];`

## core/fxjit/PixelShader/pstrans.hpp

- 59: enum `D3DPS_TEMPREG_MAX_V3_0` | `const DWORD PSTR_MAX_NUMTEMPREG         = D3DPS_TEMPREG_MAX_V3_0;`
- 60: enum `D3DPS_INPUTREG_MAX_SW_DX9` | `const DWORD PSTR_MAX_NUMINPUTREG        = D3DPS_INPUTREG_MAX_SW_DX9;`
- 61: enum `D3DPS_CONSTREG_MAX_SW_DX9` | `const DWORD PSTR_MAX_NUMCONSTREG        = D3DPS_CONSTREG_MAX_SW_DX9;`
- 62: enum `D3DPS_CONSTINTREG_MAX_SW_DX9` | `const DWORD PSTR_MAX_NUMCONSTINTREG     = D3DPS_CONSTINTREG_MAX_SW_DX9;`
- 63: enum `D3DPS_CONSTBOOLREG_MAX_SW_DX9` | `const DWORD PSTR_MAX_NUMCONSTBOOLREG    = D3DPS_CONSTBOOLREG_MAX_SW_DX9;`
- 64: enum `D3DPS_TEXTUREREG_MAX_V2_0` | `const DWORD PSTR_MAX_NUMTEXTUREREG      = D3DPS_TEXTUREREG_MAX_V2_0;`
- 65: enum `D3DPS_COLOROUT_MAX_V3_0` | `const DWORD PSTR_MAX_NUMCOLOROUTREG     = D3DPS_COLOROUT_MAX_V3_0;`
- 69: enum `D3DPS_TEMPREG_MAX_V2_0` | `#define D3DPS_TEMPREG_MAX_V2_0          12`
- 70: enum `D3DPS_INPUTREG_MAX_V2_0` | `#define D3DPS_INPUTREG_MAX_V2_0         2`
- 71: enum `D3DPS_CONSTREG_MAX_V2_0` | `#define D3DPS_CONSTREG_MAX_V2_0         32`
- 72: enum `D3DPS_TEXTUREREG_MAX_V2_0` | `#define D3DPS_TEXTUREREG_MAX_V2_0       8`
- 73: enum `D3DPS_COLOROUT_MAX_V2_0` | `#define D3DPS_COLOROUT_MAX_V2_0         4`
- 74: enum `D3DPS_PREDICATE_MAX_V3_0` | `#define D3DPS_PREDICATE_MAX_V3_0        1`
- 76: enum `D3DPS_TEMPREG_MAX_V2_0` | `const DWORD PSTR_MAX_NUMTEMPREG         = D3DPS_TEMPREG_MAX_V2_0;`
- 77: enum `D3DPS_INPUTREG_MAX_V2_0` | `const DWORD PSTR_MAX_NUMINPUTREG        = D3DPS_INPUTREG_MAX_V2_0;`
- 78: enum `D3DPS_CONSTREG_MAX_V2_0` | `const DWORD PSTR_MAX_NUMCONSTREG        = D3DPS_CONSTREG_MAX_V2_0;`
- 79: enum `D3DPS_TEXTUREREG_MAX_V2_0` | `const DWORD PSTR_MAX_NUMTEXTUREREG      = D3DPS_TEXTUREREG_MAX_V2_0;`
- 80: enum `D3DPS_COLOROUT_MAX_V2_0` | `const DWORD PSTR_MAX_NUMCOLOROUTREG     = D3DPS_COLOROUT_MAX_V2_0;`
- 86: enum `D3DPS_PREDICATE_MAX_V3_0` | `const DWORD PSTR_MAX_NUMPREDICATEREG    = D3DPS_PREDICATE_MAX_V3_0 + 1;`
- 102: enum `D3DSHADER_INSTRUCTION_OPCODE_TYPE` | `#define D3DSIO_TEXBEM_LEGACY    ((D3DSHADER_INSTRUCTION_OPCODE_TYPE)0xC001)`
- 102: enum `D3DSIO_TEXBEM_LEGACY` | `#define D3DSIO_TEXBEM_LEGACY    ((D3DSHADER_INSTRUCTION_OPCODE_TYPE)0xC001)`
- 103: enum `D3DSHADER_INSTRUCTION_OPCODE_TYPE` | `#define D3DSIO_TEXBEML_LEGACY   ((D3DSHADER_INSTRUCTION_OPCODE_TYPE)0xC002)`
- 103: enum `D3DSIO_TEXBEML_LEGACY` | `#define D3DSIO_TEXBEML_LEGACY   ((D3DSHADER_INSTRUCTION_OPCODE_TYPE)0xC002)`
- 107: enum `D3DSP_WRITEMASK_0` | `#define PSTR_COMPONENTMASK_0        (D3DSP_WRITEMASK_0 >> PSTR_COMPONENTMASK_SHIFT)`
- 108: enum `D3DSP_WRITEMASK_1` | `#define PSTR_COMPONENTMASK_1        (D3DSP_WRITEMASK_1 >> PSTR_COMPONENTMASK_SHIFT)`
- 109: enum `D3DSP_WRITEMASK_2` | `#define PSTR_COMPONENTMASK_2        (D3DSP_WRITEMASK_2 >> PSTR_COMPONENTMASK_SHIFT)`
- 110: enum `D3DSP_WRITEMASK_3` | `#define PSTR_COMPONENTMASK_3        (D3DSP_WRITEMASK_3 >> PSTR_COMPONENTMASK_SHIFT)`
- 111: enum `D3DSP_WRITEMASK_ALL` | `#define PSTR_COMPONENTMASK_ALL      (D3DSP_WRITEMASK_ALL >> PSTR_COMPONENTMASK_SHIFT)`
- 114: enum `D3DSP_NOSWIZZLE` | `#define PSTR_NOSWIZZLE          (D3DSP_NOSWIZZLE >> D3DSP_SWIZZLE_SHIFT)`
- 114: enum `D3DSP_SWIZZLE_SHIFT` | `#define PSTR_NOSWIZZLE          (D3DSP_NOSWIZZLE >> D3DSP_SWIZZLE_SHIFT)`
- 115: enum `D3DSP_REPLICATERED` | `#define PSTR_REPLICATERED       (D3DSP_REPLICATERED >> D3DSP_SWIZZLE_SHIFT)`
- 115: enum `D3DSP_SWIZZLE_SHIFT` | `#define PSTR_REPLICATERED       (D3DSP_REPLICATERED >> D3DSP_SWIZZLE_SHIFT)`
- 116: enum `D3DSP_REPLICATEGREEN` | `#define PSTR_REPLICATEGREEN     (D3DSP_REPLICATEGREEN >> D3DSP_SWIZZLE_SHIFT)`
- 116: enum `D3DSP_SWIZZLE_SHIFT` | `#define PSTR_REPLICATEGREEN     (D3DSP_REPLICATEGREEN >> D3DSP_SWIZZLE_SHIFT)`
- 117: enum `D3DSP_REPLICATEBLUE` | `#define PSTR_REPLICATEBLUE      (D3DSP_REPLICATEBLUE >> D3DSP_SWIZZLE_SHIFT)`
- 117: enum `D3DSP_SWIZZLE_SHIFT` | `#define PSTR_REPLICATEBLUE      (D3DSP_REPLICATEBLUE >> D3DSP_SWIZZLE_SHIFT)`
- 118: enum `D3DSP_REPLICATEALPHA` | `#define PSTR_REPLICATEALPHA     (D3DSP_REPLICATEALPHA >> D3DSP_SWIZZLE_SHIFT)`
- 118: enum `D3DSP_SWIZZLE_SHIFT` | `#define PSTR_REPLICATEALPHA     (D3DSP_REPLICATEALPHA >> D3DSP_SWIZZLE_SHIFT)`
- 832: enum `D3DSHADER_COMPARISON` | `D3DSHADER_COMPARISON    Comparison;`
- 1273: enum `D3DSHADER_COMPARISON` | `D3DSHADER_COMPARISON GetOppositeComparison(D3DSHADER_COMPARISON Comparison);`
- 1273: enum `D3DSHADER_COMPARISON` | `D3DSHADER_COMPARISON GetOppositeComparison(D3DSHADER_COMPARISON Comparison);`

## core/fxjit/Public/typedefs.h

- 989: enum `D3DFMT_UNKNOWN` | `D3DFMT_UNKNOWN              =  0,`
- 991: enum `D3DFMT_R8G8B8` | `D3DFMT_R8G8B8               = 20,`
- 992: enum `D3DFMT_A8R8G8B8` | `D3DFMT_A8R8G8B8             = 21,`
- 993: enum `D3DFMT_X8R8G8B8` | `D3DFMT_X8R8G8B8             = 22,`
- 994: enum `D3DFMT_R5G6B5` | `D3DFMT_R5G6B5               = 23,`
- 995: enum `D3DFMT_X1R5G5B5` | `D3DFMT_X1R5G5B5             = 24,`
- 996: enum `D3DFMT_A1R5G5B5` | `D3DFMT_A1R5G5B5             = 25,`
- 997: enum `D3DFMT_A4R4G4B4` | `D3DFMT_A4R4G4B4             = 26,`
- 998: enum `D3DFMT_R3G3B2` | `D3DFMT_R3G3B2               = 27,`
- 999: enum `D3DFMT_A8` | `D3DFMT_A8                   = 28,`
- 1000: enum `D3DFMT_A8R3G3B2` | `D3DFMT_A8R3G3B2             = 29,`
- 1001: enum `D3DFMT_X4R4G4B4` | `D3DFMT_X4R4G4B4             = 30,`
- 1002: enum `D3DFMT_A2B10G10R10` | `D3DFMT_A2B10G10R10          = 31,`
- 1003: enum `D3DFMT_A8B8G8R8` | `D3DFMT_A8B8G8R8             = 32,`
- 1004: enum `D3DFMT_X8B8G8R8` | `D3DFMT_X8B8G8R8             = 33,`
- 1005: enum `D3DFMT_G16R16` | `D3DFMT_G16R16               = 34,`
- 1006: enum `D3DFMT_A2R10G10B10` | `D3DFMT_A2R10G10B10          = 35,`
- 1007: enum `D3DFMT_A16B16G16R16` | `D3DFMT_A16B16G16R16         = 36,`
- 1009: enum `D3DFMT_A8P8` | `D3DFMT_A8P8                 = 40,`
- 1010: enum `D3DFMT_P8` | `D3DFMT_P8                   = 41,`
- 1012: enum `D3DFMT_L8` | `D3DFMT_L8                   = 50,`
- 1013: enum `D3DFMT_A8L8` | `D3DFMT_A8L8                 = 51,`
- 1014: enum `D3DFMT_A4L4` | `D3DFMT_A4L4                 = 52,`
- 1016: enum `D3DFMT_V8U8` | `D3DFMT_V8U8                 = 60,`
- 1017: enum `D3DFMT_L6V5U5` | `D3DFMT_L6V5U5               = 61,`
- 1018: enum `D3DFMT_X8L8V8U8` | `D3DFMT_X8L8V8U8             = 62,`
- 1019: enum `D3DFMT_Q8W8V8U8` | `D3DFMT_Q8W8V8U8             = 63,`
- 1020: enum `D3DFMT_V16U16` | `D3DFMT_V16U16               = 64,`
- 1021: enum `D3DFMT_A2W10V10U10` | `D3DFMT_A2W10V10U10          = 67,`
- 1023: enum `D3DFMT_UYVY` | `D3DFMT_UYVY                 = MAKEFOURCC('U', 'Y', 'V', 'Y'),`
- 1024: enum `D3DFMT_R8G8_B8G8` | `D3DFMT_R8G8_B8G8            = MAKEFOURCC('R', 'G', 'B', 'G'),`
- 1025: enum `D3DFMT_YUY2` | `D3DFMT_YUY2                 = MAKEFOURCC('Y', 'U', 'Y', '2'),`
- 1026: enum `D3DFMT_G8R8_G8B8` | `D3DFMT_G8R8_G8B8            = MAKEFOURCC('G', 'R', 'G', 'B'),`
- 1027: enum `D3DFMT_DXT1` | `D3DFMT_DXT1                 = MAKEFOURCC('D', 'X', 'T', '1'),`
- 1028: enum `D3DFMT_DXT2` | `D3DFMT_DXT2                 = MAKEFOURCC('D', 'X', 'T', '2'),`
- 1029: enum `D3DFMT_DXT3` | `D3DFMT_DXT3                 = MAKEFOURCC('D', 'X', 'T', '3'),`
- 1030: enum `D3DFMT_DXT4` | `D3DFMT_DXT4                 = MAKEFOURCC('D', 'X', 'T', '4'),`
- 1031: enum `D3DFMT_DXT5` | `D3DFMT_DXT5                 = MAKEFOURCC('D', 'X', 'T', '5'),`
- 1033: enum `D3DFMT_D16_LOCKABLE` | `D3DFMT_D16_LOCKABLE         = 70,`
- 1034: enum `D3DFMT_D32` | `D3DFMT_D32                  = 71,`
- 1035: enum `D3DFMT_D15S1` | `D3DFMT_D15S1                = 73,`
- 1036: enum `D3DFMT_D24S8` | `D3DFMT_D24S8                = 75,`
- 1037: enum `D3DFMT_D24X8` | `D3DFMT_D24X8                = 77,`
- 1038: enum `D3DFMT_D24X4S4` | `D3DFMT_D24X4S4              = 79,`
- 1039: enum `D3DFMT_D16` | `D3DFMT_D16                  = 80,`
- 1041: enum `D3DFMT_D32F_LOCKABLE` | `D3DFMT_D32F_LOCKABLE        = 82,`
- 1042: enum `D3DFMT_D24FS8` | `D3DFMT_D24FS8               = 83,`
- 1045: enum `D3DFMT_D32_LOCKABLE` | `D3DFMT_D32_LOCKABLE         = 84,`
- 1046: enum `D3DFMT_S8_LOCKABLE` | `D3DFMT_S8_LOCKABLE          = 85,`
- 1050: enum `D3DFMT_L16` | `D3DFMT_L16                  = 81,`
- 1052: enum `D3DFMT_VERTEXDATA` | `D3DFMT_VERTEXDATA           =100,`
- 1053: enum `D3DFMT_INDEX16` | `D3DFMT_INDEX16              =101,`
- 1054: enum `D3DFMT_INDEX32` | `D3DFMT_INDEX32              =102,`
- 1056: enum `D3DFMT_Q16W16V16U16` | `D3DFMT_Q16W16V16U16         =110,`
- 1058: enum `D3DFMT_MULTI2_ARGB8` | `D3DFMT_MULTI2_ARGB8         = MAKEFOURCC('M','E','T','1'),`
- 1063: enum `D3DFMT_R16F` | `D3DFMT_R16F                 = 111,`
- 1064: enum `D3DFMT_G16R16F` | `D3DFMT_G16R16F              = 112,`
- 1065: enum `D3DFMT_A16B16G16R16F` | `D3DFMT_A16B16G16R16F        = 113,`
- 1068: enum `D3DFMT_R32F` | `D3DFMT_R32F                 = 114,`
- 1069: enum `D3DFMT_G32R32F` | `D3DFMT_G32R32F              = 115,`
- 1070: enum `D3DFMT_A32B32G32R32F` | `D3DFMT_A32B32G32R32F        = 116,`
- 1075: enum `D3DFMT_A1` | `D3DFMT_A1                   = 118,`
- 1079: enum `D3DFMT_BINARYBUFFER` | `D3DFMT_BINARYBUFFER            = 199,`
- 1088: enum `D3DPOOL_DEFAULT` | `D3DPOOL_DEFAULT                 = 0,`
- 1089: enum `D3DPOOL_MANAGED` | `D3DPOOL_MANAGED                 = 1,`
- 1090: enum `D3DPOOL_SYSTEMMEM` | `D3DPOOL_SYSTEMMEM               = 2,`
- 1091: enum `D3DPOOL_SCRATCH` | `D3DPOOL_SCRATCH                 = 3,`

## core/hw/D3DDeviceManager.h

- 66: enum `D3DPRESENT_PARAMETERS` | `__out_ecount_opt(1) D3DPRESENT_PARAMETERS *pPresentParams,`
- 104: enum `D3DPRESENT_PARAMETERS` | `D3DPRESENT_PARAMETERS DbgPresentParams;`
- 133: enum `D3DPRESENT_PARAMETERS` | `__out_ecount(1) D3DPRESENT_PARAMETERS *pD3DPresentParams`
- 158: enum `D3DPRESENT_PARAMETERS` | `D3DPRESENT_PARAMETERS *pBasePresentParams,`

## core/hw/D3DTextureSurface.cpp

- 97: call `GetDesc` | `IFC(m_pD3DSurface->GetDesc(&m_d3dsd));`

## core/hw/HwBitBltDeviceBitmapColorSource.cpp

- 168: enum `D3DUSAGE_RENDERTARGET` | `Assert((d3dsd.Usage & D3DUSAGE_RENDERTARGET) == D3DUSAGE_RENDERTARGET);`
- 168: enum `D3DUSAGE_RENDERTARGET` | `Assert((d3dsd.Usage & D3DUSAGE_RENDERTARGET) == D3DUSAGE_RENDERTARGET);`
- 192: call `CreateRenderTarget` | `IFC(pDevice->CreateRenderTarget(`
- 196: enum `D3DMULTISAMPLE_NONE` | `D3DMULTISAMPLE_NONE,`
- 246: call `GetDC` | `IFC(pISrcSurface->GetDC(&hSrcDC));`
- 247: call `GetDC` | `IFC(m_pTransferSurface->GetDC(&hTransferDC));`
- 267: call `ReleaseDC` | `IFC(pISrcSurface->ReleaseDC(hSrcDC));`
- 269: call `ReleaseDC` | `IFC(m_pTransferSurface->ReleaseDC(hTransferDC));`
- 279: call `StretchRect` | `IFC(m_pDevice->StretchRect(`
- 284: enum `D3DTEXF_NONE` | `D3DTEXF_NONE`
- 292: call `ReleaseDC` | `IGNORE_HR(pISrcSurface->ReleaseDC(hSrcDC));`
- 297: call `ReleaseDC` | `IGNORE_HR(m_pTransferSurface->ReleaseDC(hTransferDC));`

## core/hw/HwBitmapCache.cpp

- 279: enum `D3DTADDRESS_CLAMP` | `\|\| (m_pDevice->SupportsNonPow2Conditionally() && taU == D3DTADDRESS_CLAMP && taV == D3DTADDRESS_CLAMP)`
- 279: enum `D3DTADDRESS_CLAMP` | `\|\| (m_pDevice->SupportsNonPow2Conditionally() && taU == D3DTADDRESS_CLAMP && taV == D3DTADDRESS_CLAMP)`

## core/hw/HwBitmapColorSource.h

- 383: enum `D3DUSAGE_RENDERTARGET` | `return m_d3dsdRequired.Usage & D3DUSAGE_RENDERTARGET;`

## core/hw/HwBoxColorSource.cpp

- 33: enum `D3DTADDRESS_CLAMP` | `D3DTADDRESS_CLAMP,`
- 34: enum `D3DTADDRESS_CLAMP` | `D3DTADDRESS_CLAMP`
- 132: call `SetTexture` | `IFC(m_pDevice->SetTexture(dwSampler, m_vidMemManager.GetVidMemTextureNoRef()));`
- 161: enum `D3DFMT_A8R8G8B8` | `D3DFMT_A8R8G8B8,`

## core/hw/HwCaps.h

- 36: enum `D3DCAPS2_CANSHARERESOURCE` | `return (caps.Caps2 & D3DCAPS2_CANSHARERESOURCE) != 0;`
- 43: enum `D3DDEVTYPE_SW` | `return caps.DeviceType == D3DDEVTYPE_SW;`
- 51: enum `D3DPMISCCAPS_COLORWRITEENABLE` | `return (caps.PrimitiveMiscCaps & D3DPMISCCAPS_COLORWRITEENABLE) != 0;`
- 59: enum `D3DPBLENDCAPS_BLENDFACTOR` | `return (caps.SrcBlendCaps & D3DPBLENDCAPS_BLENDFACTOR) != 0;`

## core/hw/HwRenderStateManager.cpp

- 328: call `SetTexture` | `IFC(m_pID3DDevice->SetTexture(`
- 360: enum `D3DTSS_TEXCOORDINDEX` | `if (state == D3DTSS_TEXCOORDINDEX)`
- 368: call `SetTextureStageState` | `IFC(m_pID3DDevice->SetTextureStageState(`
- 404: enum `D3DTSS_TEXCOORDINDEX` | `D3DTSS_TEXCOORDINDEX,`
- 434: call `SetSamplerState` | `IFC(m_pID3DDevice->SetSamplerState(`
- 465: call `SetFVF` | `IFC(m_pID3DDevice->SetFVF(`
- 494: call `SetVertexShader` | `IFC(m_pID3DDevice->SetVertexShader(`
- 523: call `SetPixelShader` | `IFC(m_pID3DDevice->SetPixelShader(`
- 554: call `SetDepthStencilSurface` | `IFC(m_pID3DDevice->SetDepthStencilSurface(`
- 587: call `SetRenderState` | `IFC(m_pID3DDevice->SetRenderState(`
- 620: call `SetTransform` | `IFC(m_pID3DDevice->SetTransform(`
- 651: call `SetTransform` | `IFC(m_pID3DDevice->SetTransform(`
- 718: call `SetPixelShaderConstantF` | `IFC(m_pID3DDevice->SetPixelShaderConstantF(`
- 781: call `SetPixelShaderConstantI` | `IFC(m_pID3DDevice->SetPixelShaderConstantI(`
- 838: call `SetPixelShaderConstantB` | `IFC(m_pID3DDevice->SetPixelShaderConstantB(`
- 912: call `SetVertexShaderConstantF` | `IFC(m_pID3DDevice->SetVertexShaderConstantF(`
- 945: enum `D3DRS_SCISSORTESTENABLE` | `m_renderStates.SetToUnknown(D3DRS_SCISSORTESTENABLE);`
- 1006: enum `D3DRS_SCISSORTESTENABLE` | `(m_renderStates.IsStateSet(D3DRS_SCISSORTESTENABLE, dwScissorStateFALSE) \|\|`
- 1019: call `SetScissorRect` | `MIL_THR(m_pID3DDevice->SetScissorRect(&rcScissor));`
- 1027: enum `D3DRS_SCISSORTESTENABLE` | `D3DRS_SCISSORTESTENABLE,`
- 1036: enum `D3DRS_SCISSORTESTENABLE` | `D3DRS_SCISSORTESTENABLE,`
- 1334: enum `D3DTOP_DISABLE` | `IFC(SetTextureStageStateInline(dwStage, D3DTSS_COLOROP, D3DTOP_DISABLE));`
- 1334: enum `D3DTSS_COLOROP` | `IFC(SetTextureStageStateInline(dwStage, D3DTSS_COLOROP, D3DTOP_DISABLE));`
- 1406: call `SetViewport` | `IFC(m_pID3DDevice->SetViewport(`
- 1431: call `SetStreamSource` | `IFC(m_pID3DDevice->SetStreamSource(`
- 1468: call `SetIndices` | `IFC(m_pID3DDevice->SetIndices(`
- 1499: enum `D3DRS_BLENDFACTOR` | `IFC(ForceSetRenderState(D3DRS_BLENDFACTOR,0));`
- 1509: enum `D3DRS_ZENABLE` | `IFC(ForceSetRenderState(D3DRS_ZENABLE, D3DZB_FALSE));`
- 1509: enum `D3DZB_FALSE` | `IFC(ForceSetRenderState(D3DRS_ZENABLE, D3DZB_FALSE));`
- 1510: enum `D3DRS_ZWRITEENABLE` | `IFC(ForceSetRenderState(D3DRS_ZWRITEENABLE, FALSE));`
- 1514: enum `D3DFILL_WIREFRAME` | `IFC(ForceSetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME));`
- 1514: enum `D3DRS_FILLMODE` | `IFC(ForceSetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME));`
- 1518: enum `D3DFILL_SOLID` | `IFC(ForceSetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID));`
- 1518: enum `D3DRS_FILLMODE` | `IFC(ForceSetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID));`
- 1522: enum `D3DRS_SHADEMODE` | `IFC(ForceSetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD));`
- 1526: enum `D3DRS_ALPHATESTENABLE` | `IFC(ForceSetRenderState(D3DRS_ALPHATESTENABLE, FALSE));`
- 1528: enum `D3DRS_LASTPIXEL` | `IFC(ForceSetRenderState(D3DRS_LASTPIXEL, FALSE));`
- 1530: enum `D3DRS_ANTIALIASEDLINEENABLE` | `IFC(ForceSetRenderState(D3DRS_ANTIALIASEDLINEENABLE, FALSE));`
- 1539: enum `D3DCULL_NONE` | `IFC(ForceSetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));`
- 1539: enum `D3DRS_CULLMODE` | `IFC(ForceSetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));`
- 1541: enum `D3DCMP_LESSEQUAL` | `IFC(ForceSetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL));`
- 1541: enum `D3DRS_ZFUNC` | `IFC(ForceSetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL));`
- 1549: enum `D3DRS_DITHERENABLE` | `IFC(ForceSetRenderState(D3DRS_DITHERENABLE, FALSE));`
- 1553: enum `D3DRS_FOGENABLE` | `IFC(ForceSetRenderState(D3DRS_FOGENABLE, FALSE));`
- 1562: enum `D3DRS_DEPTHBIAS` | `IFC(ForceSetRenderState(D3DRS_DEPTHBIAS, 0));`
- 1567: enum `D3DRS_STENCILENABLE` | `IFC(ForceSetRenderState(D3DRS_STENCILENABLE, FALSE));`
- 1569: enum `D3DRS_STENCILREF` | `IFC(ForceSetRenderState(D3DRS_STENCILREF, 0));`
- 1578: enum `D3DCMP_NOTEQUAL` | `IFC(ForceSetRenderState(D3DRS_STENCILFUNC,  D3DCMP_NOTEQUAL));`
- 1578: enum `D3DRS_STENCILFUNC` | `IFC(ForceSetRenderState(D3DRS_STENCILFUNC,  D3DCMP_NOTEQUAL));`
- 1579: enum `D3DRS_STENCILFAIL` | `IFC(ForceSetRenderState(D3DRS_STENCILFAIL,  D3DSTENCILOP_KEEP));`
- 1579: enum `D3DSTENCILOP_KEEP` | `IFC(ForceSetRenderState(D3DRS_STENCILFAIL,  D3DSTENCILOP_KEEP));`
- 1580: enum `D3DRS_STENCILZFAIL` | `IFC(ForceSetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP));`
- 1580: enum `D3DSTENCILOP_KEEP` | `IFC(ForceSetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP));`
- 1581: enum `D3DRS_STENCILPASS` | `IFC(ForceSetRenderState(D3DRS_STENCILPASS,  D3DSTENCILOP_KEEP));`
- 1581: enum `D3DSTENCILOP_KEEP` | `IFC(ForceSetRenderState(D3DRS_STENCILPASS,  D3DSTENCILOP_KEEP));`
- 1582: enum `D3DRS_STENCILMASK` | `IFC(ForceSetRenderState(D3DRS_STENCILMASK,  0xffffffff));`
- 1590: enum `D3DRS_STENCILWRITEMASK` | `IFC(ForceSetRenderState(D3DRS_STENCILWRITEMASK, 0x0));`
- 1592: enum `D3DRS_TWOSIDEDSTENCILMODE` | `IFC(ForceSetRenderState(D3DRS_TWOSIDEDSTENCILMODE, FALSE));`
- 1603: enum `D3DRS_CLIPPING` | `IFC(ForceSetRenderState(D3DRS_CLIPPING, TRUE));`
- 1612: enum `D3DRS_LIGHTING` | `IFC(ForceSetRenderState(D3DRS_LIGHTING, FALSE));`
- 1613: enum `D3DRS_SPECULARENABLE` | `IFC(ForceSetRenderState(D3DRS_SPECULARENABLE, FALSE));`
- 1618: enum `D3DRS_COLORVERTEX` | `IFC(ForceSetRenderState(D3DRS_COLORVERTEX, TRUE));`
- 1628: enum `D3DRS_NORMALIZENORMALS` | `IFC(ForceSetRenderState(D3DRS_NORMALIZENORMALS, FALSE));`
- 1630: enum `D3DRS_DIFFUSEMATERIALSOURCE` | `IFC(ForceSetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1));`
- 1632: enum `D3DRS_SPECULARMATERIALSOURCE` | `IFC(ForceSetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR1));`
- 1639: enum `D3DRS_AMBIENTMATERIALSOURCE` | `IFC(ForceSetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL));`
- 1653: enum `D3DRS_AMBIENT` | `IFC(ForceSetRenderState(D3DRS_AMBIENT, 0x0));`
- 1654: enum `D3DRS_VERTEXBLEND` | `IFC(ForceSetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE));`
- 1655: enum `D3DRS_CLIPPLANEENABLE` | `IFC(ForceSetRenderState(D3DRS_CLIPPLANEENABLE, 0));`
- 1669: enum `D3DRS_MULTISAMPLEANTIALIAS` | `IFC(ForceSetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE));`
- 1671: enum `D3DRS_MULTISAMPLEMASK` | `IFC(ForceSetRenderState(D3DRS_MULTISAMPLEMASK, 0xffffffff));`
- 1686: enum `D3DRS_COLORWRITEENABLE` | `IFC(ForceSetRenderState(D3DRS_COLORWRITEENABLE, 0x0000000f));`
- 1691: enum `D3DBLENDOP_ADD` | `IFC(ForceSetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD));`
- 1691: enum `D3DRS_BLENDOP` | `IFC(ForceSetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD));`
- 1706: enum `D3DRS_SCISSORTESTENABLE` | `IFC(ForceSetRenderState(D3DRS_SCISSORTESTENABLE, FALSE));`
- 1711: enum `D3DTOP_DISABLE` | `IFC(ForceSetTextureStageState(dwStage, D3DTSS_COLOROP, D3DTOP_DISABLE));`
- 1711: enum `D3DTSS_COLOROP` | `IFC(ForceSetTextureStageState(dwStage, D3DTSS_COLOROP, D3DTOP_DISABLE));`
- 1724: enum `D3DTSS_COLOROP` | `D3DTSS_COLOROP,`
- 1725: enum `D3DTOP_DISABLE` | `D3DTOP_DISABLE`
- 1740: enum `D3DSAMP_MAXANISOTROPY` | `D3DSAMP_MAXANISOTROPY,`
- 1758: enum `D3DTSS_TEXCOORDINDEX` | `D3DTSS_TEXCOORDINDEX,`
- 1772: enum `D3DTSS_TEXTURETRANSFORMFLAGS` | `D3DTSS_TEXTURETRANSFORMFLAGS,`
- 1773: enum `D3DTTFF_DISABLE` | `D3DTTFF_DISABLE`
- 1840: call `SetMaterial` | `IFC(m_pID3DDevice->SetMaterial(&material));`
- 1876: call `SetStreamSource` | `IFC(m_pID3DDevice->SetStreamSource(`
- 1914: enum `D3DRS_DIFFUSEMATERIALSOURCE` | `m_renderStates.SetSupported(D3DRS_DIFFUSEMATERIALSOURCE);`
- 1915: enum `D3DRS_SPECULARMATERIALSOURCE` | `m_renderStates.SetSupported(D3DRS_SPECULARMATERIALSOURCE);`
- 1921: enum `D3DRS_FILLMODE` | `m_renderStates.SetSupported(D3DRS_FILLMODE);`
- 1927: enum `D3DRS_ALPHABLENDENABLE` | `m_renderStates.SetSupported(D3DRS_ALPHABLENDENABLE);`
- 1928: enum `D3DRS_SRCBLEND` | `m_renderStates.SetSupported(D3DRS_SRCBLEND);`
- 1929: enum `D3DRS_DESTBLEND` | `m_renderStates.SetSupported(D3DRS_DESTBLEND);`
- 1935: enum `D3DRS_BLENDFACTOR` | `m_renderStates.SetSupported(D3DRS_BLENDFACTOR);`
- 1936: enum `D3DRS_COLORWRITEENABLE` | `m_renderStates.SetSupported(D3DRS_COLORWRITEENABLE);`
- 1942: enum `D3DRS_SCISSORTESTENABLE` | `m_renderStates.SetSupported(D3DRS_SCISSORTESTENABLE);`
- 1949: enum `D3DRS_ZENABLE` | `m_renderStates.SetSupported(D3DRS_ZENABLE);`
- 1950: enum `D3DRS_STENCILENABLE` | `m_renderStates.SetSupported(D3DRS_STENCILENABLE);`
- 1951: enum `D3DRS_ZWRITEENABLE` | `m_renderStates.SetSupported(D3DRS_ZWRITEENABLE);`
- 1952: enum `D3DRS_CULLMODE` | `m_renderStates.SetSupported(D3DRS_CULLMODE);`
- 1953: enum `D3DRS_ZFUNC` | `m_renderStates.SetSupported(D3DRS_ZFUNC);`
- 1954: enum `D3DRS_MULTISAMPLEANTIALIAS` | `m_renderStates.SetSupported(D3DRS_MULTISAMPLEANTIALIAS);`
- 1968: enum `D3DTSS_COLOROP` | `CalcTextureStageStatePos(uTextureStage,D3DTSS_COLOROP));`
- 1971: enum `D3DTSS_COLORARG1` | `CalcTextureStageStatePos(uTextureStage,D3DTSS_COLORARG1));`
- 1974: enum `D3DTSS_COLORARG2` | `CalcTextureStageStatePos(uTextureStage,D3DTSS_COLORARG2));`
- 1977: enum `D3DTSS_ALPHAOP` | `CalcTextureStageStatePos(uTextureStage,D3DTSS_ALPHAOP));`
- 1980: enum `D3DTSS_ALPHAARG1` | `CalcTextureStageStatePos(uTextureStage,D3DTSS_ALPHAARG1));`
- 1983: enum `D3DTSS_ALPHAARG2` | `CalcTextureStageStatePos(uTextureStage,D3DTSS_ALPHAARG2));`
- 1986: enum `D3DTSS_TEXTURETRANSFORMFLAGS` | `CalcTextureStageStatePos(uTextureStage,D3DTSS_TEXTURETRANSFORMFLAGS));`
- 1989: enum `D3DTSS_TEXCOORDINDEX` | `CalcTextureStageStatePos(uTextureStage,D3DTSS_TEXCOORDINDEX));`
- 2003: enum `D3DSAMP_MAGFILTER` | `CalcSamplerStatePos(uSamplerState,D3DSAMP_MAGFILTER));`
- 2006: enum `D3DSAMP_MINFILTER` | `CalcSamplerStatePos(uSamplerState,D3DSAMP_MINFILTER));`
- 2009: enum `D3DSAMP_MIPFILTER` | `CalcSamplerStatePos(uSamplerState,D3DSAMP_MIPFILTER));`
- 2014: enum `D3DSAMP_ADDRESSU` | `CalcSamplerStatePos(uSamplerState,D3DSAMP_ADDRESSU));`
- 2017: enum `D3DSAMP_ADDRESSV` | `CalcSamplerStatePos(uSamplerState,D3DSAMP_ADDRESSV));`
- 2020: enum `D3DSAMP_BORDERCOLOR` | `CalcSamplerStatePos(uSamplerState,D3DSAMP_BORDERCOLOR));`

## core/hw/HwRenderStateManager.h

- 225: enum `D3DRS_FILLMODE` | `if (state == D3DRS_FILLMODE)`
- 510: call `SetConvolutionMonoKernel` | `return m_pID3DDeviceEx->SetConvolutionMonoKernel(width, height, NULL, NULL);`

## core/hw/HwShaderEffect.cpp

- 68: call `CreatePixelShader` | `IFC(pDevice->CreatePixelShader(reinterpret_cast<DWORD*>(pPixelShaderByteCode), &m_pD3DPixelShader));`
- 100: call `SetPixelShader` | `IFC(pDevice->SetPixelShader(m_pD3DPixelShader));`

## core/hw/HwShaderPipeline.cpp

- 101: call `SetVertexShader` | `IFC(m_pDeviceNoRef->SetVertexShader(m_pVertexShader));`
- 102: call `SetPixelShader` | `IFC(m_pDeviceNoRef->SetPixelShader(m_pPixelShader));`
- 142: call `SetPixelShaderConstantF` | `IFC(m_pDeviceNoRef->SetPixelShaderConstantF(`
- 150: call `SetVertexShaderConstantF` | `IFC(m_pDeviceNoRef->SetVertexShaderConstantF(`
- 226: call `SetVertexShaderConstantF` | `IFC(m_pDeviceNoRef->SetVertexShaderConstantF(`
- 555: call `SetPixelShaderConstantF` | `IFC(m_pDeviceNoRef->SetPixelShaderConstantF(`
- 565: call `SetVertexShaderConstantF` | `IFC(m_pDeviceNoRef->SetVertexShaderConstantF(`

## core/hw/HwTexturedColorSource.h

- 104: enum `D3DTADDRESS_BORDER` | `m_taU = m_taV = D3DTADDRESS_BORDER;`

## core/hw/HwUtils.cpp

- 189: call `GetDesc` | `IFC(pSourceSurface->GetDesc(&d3dsd));`
- 191: enum `D3DPOOL_DEFAULT` | `if (d3dsd.Pool != D3DPOOL_DEFAULT)`
- 209: call `GetDevice` | `IFC(pSourceSurface->GetDevice(&pIDevice));`
- 230: call `CreateTexture` | `IFC(pIDevice->CreateTexture(`
- 236: enum `D3DPOOL_SYSTEMMEM` | `D3DPOOL_SYSTEMMEM,`
- 241: call `GetSurfaceLevel` | `IFC(pD3DLockableTexture->GetSurfaceLevel(`
- 251: enum `D3DMULTISAMPLE_NONE` | `\|\| d3dsd.MultiSampleType != D3DMULTISAMPLE_NONE)`
- 253: call `CreateRenderTarget` | `IFC(pIDevice->CreateRenderTarget(`
- 257: enum `D3DMULTISAMPLE_NONE` | `D3DMULTISAMPLE_NONE,`
- 264: call `StretchRect` | `IFC(pIDevice->StretchRect(`
- 269: enum `D3DTEXF_NONE` | `D3DTEXF_NONE`
- 277: call `GetRenderTargetData` | `IFC(pIDevice->GetRenderTargetData(`
- 285: call `LockRect` | `IFC(pD3DLockableSurface->LockRect(&rcLock, &rcDest, D3DLOCK_READONLY));`
- 285: enum `D3DLOCK_READONLY` | `IFC(pD3DLockableSurface->LockRect(&rcLock, &rcDest, D3DLOCK_READONLY));`
- 324: call `UnlockRect` | `IGNORE_HR(pD3DLockableSurface->UnlockRect());`

## core/hw/HwVertexBuffer.h

- 319: call `SetFVF` | `RRETURN(THR(pDevice->SetFVF(TVertex::Format)));`
- 365: call `DrawPrimitiveUP` | `IFC(pDevice->DrawPrimitiveUP(`
- 366: enum `D3DPT_TRIANGLELIST` | `D3DPT_TRIANGLELIST,`
- 396: call `DrawPrimitiveUP` | `IFC(pDevice->DrawPrimitiveUP(`
- 397: enum `D3DPT_TRIANGLESTRIP` | `D3DPT_TRIANGLESTRIP,`
- 410: call `DrawPrimitiveUP` | `IFC(pDevice->DrawPrimitiveUP(`
- 411: enum `D3DPT_LINELIST` | `D3DPT_LINELIST,`

## core/hw/HwVidMemTextureManager.cpp

- 216: call `LockRect` | `IFC(m_pSysMemSurface->LockRect(`
- 219: enum `D3DLOCK_NO_DIRTY_UPDATE` | `D3DLOCK_NO_DIRTY_UPDATE`
- 257: call `UnlockRect` | `IFC(m_pSysMemSurface->UnlockRect());`
- 323: call `UpdateSurface` | `IFC(m_pDeviceNoRef->UpdateSurface(`
- 364: enum `D3DRTYPE_TEXTURE` | `m_d3dsdRequiredForVidMem.Type = D3DRTYPE_TEXTURE;`
- 375: enum `D3DPOOL_DEFAULT` | `m_d3dsdRequiredForVidMem.Pool = D3DPOOL_DEFAULT;`
- 376: enum `D3DMULTISAMPLE_NONE` | `m_d3dsdRequiredForVidMem.MultiSampleType = D3DMULTISAMPLE_NONE;`

## core/hw/InteropDeviceBitmap.cpp

- 215: call `GetDeviceCaps` | `IFC(pID3DDevice->GetDeviceCaps(&caps));`
- 217: enum `D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES` | `if ((caps.DevCaps2 & D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES) == D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES)`
- 217: enum `D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES` | `if ((caps.DevCaps2 & D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES) == D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES)`
- 219: enum `D3DCAPS2_CANSHARERESOURCE` | `if (pID3DDeviceEx && (caps.Caps2 & D3DCAPS2_CANSHARERESOURCE) == D3DCAPS2_CANSHARERESOURCE)`
- 219: enum `D3DCAPS2_CANSHARERESOURCE` | `if (pID3DDeviceEx && (caps.Caps2 & D3DCAPS2_CANSHARERESOURCE) == D3DCAPS2_CANSHARERESOURCE)`
- 226: call `GetDC` | `IFC(pID3DSurface->GetDC(&hdc));`
- 227: call `ReleaseDC` | `IFC(pID3DSurface->ReleaseDC(hdc));`
- 274: call `GetDesc` | `IFC(pID3DUserSurface->GetDesc(&desc));`
- 284: enum `D3DFMT_A8R8G8B8` | `if (desc.Format != D3DFMT_A8R8G8B8 && desc.Format != D3DFMT_X8R8G8B8)`
- 284: enum `D3DFMT_X8R8G8B8` | `if (desc.Format != D3DFMT_A8R8G8B8 && desc.Format != D3DFMT_X8R8G8B8)`
- 289: enum `D3DUSAGE_DEPTHSTENCIL` | `if (   (desc.Usage & D3DUSAGE_DEPTHSTENCIL) == D3DUSAGE_DEPTHSTENCIL`
- 289: enum `D3DUSAGE_DEPTHSTENCIL` | `if (   (desc.Usage & D3DUSAGE_DEPTHSTENCIL) == D3DUSAGE_DEPTHSTENCIL`
- 290: enum `D3DUSAGE_RENDERTARGET` | `\|\| (desc.Usage & D3DUSAGE_RENDERTARGET) != D3DUSAGE_RENDERTARGET)`
- 290: enum `D3DUSAGE_RENDERTARGET` | `\|\| (desc.Usage & D3DUSAGE_RENDERTARGET) != D3DUSAGE_RENDERTARGET)`
- 295: enum `D3DPOOL_DEFAULT` | `if (desc.Pool != D3DPOOL_DEFAULT)`
- 301: enum `D3DRTYPE_SURFACE` | `Assert(desc.Type == D3DRTYPE_SURFACE);`
- 303: call `GetDevice` | `IFC(pID3DUserSurface->GetDevice(&pID3DUserDevice));`
- 312: call `CheckDeviceState` | `if (FAILED(pID3DUserDeviceEx->CheckDeviceState(/* hWindow = */ NULL)))`
- 319: call `TestCooperativeLevel` | `if (FAILED(pID3DUserDevice->TestCooperativeLevel()))`
- 334: enum `D3DMULTISAMPLE_NONE` | `if (   (desc.MultiSampleType != D3DMULTISAMPLE_NONE \|\| desc.MultiSampleQuality != 0)`
- 353: call `GetCreationParameters` | `IFC(pID3DUserDevice->GetCreationParameters(&oCreationParams));`
- 744: enum `D3DDEVTYPE_HAL` | `D3DDEVTYPE_HAL,`
- 844: call `GetDevice` | `IFC(m_pIUserSurface->GetDevice(&pID3DUserDevice));`
- 845: call `GetDirect3D` | `IFC(pID3DUserDevice->GetDirect3D(&pID3DUserObject));`
- 847: call `GetAdapterMonitor` | `HMONITOR hMon = pID3DUserObject->GetAdapterMonitor(m_uAdapter);`
- 893: call `UpdateSurface` | `IFC(m_poDeviceBitmapInfo->m_pbcs->UpdateSurface(`
- 1100: call `Lock` | `IFC(pBitmap->Lock(NULL, MilBitmapLock::Write, &pLock));`

## core/hw/d3ddevice.cpp

- 42: enum `D3DFMT_D24S8` | `const D3DFORMAT kD3DDepthFormat = D3DFMT_D24S8;`
- 73: enum `D3DERR_DRIVERINTERNALERROR` | `DeclareTag(tagInjectDIE, "MIL-HW", "Inject D3DERR_DRIVERINTERNALERROR failures");`
- 246: enum `D3DPOOL_MANAGED` | `m_ManagedPool(D3DPOOL_MANAGED),`
- 337: call `GetDeviceCaps` | `MIL_THR(pID3DDevice->GetDeviceCaps(`
- 358: enum `D3DPS_VERSION` | `if (m_caps.PixelShaderVersion < D3DPS_VERSION(2,0)`
- 359: enum `D3DDEVTYPE_SW` | `&& (m_caps.DeviceType != D3DDEVTYPE_SW)`
- 379: enum `D3DPTFILTERCAPS_MAGFANISOTROPIC` | `bool fSupportsMagAniso = ((m_caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC) != 0);`
- 380: enum `D3DPTFILTERCAPS_MINFANISOTROPIC` | `bool fSupportsMinAniso = ((m_caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC) != 0);`
- 404: enum `D3DDEVTYPE_SW` | `if (m_caps.DeviceType == D3DDEVTYPE_SW)`
- 413: enum `D3DDEVTYPE_SW` | `if (m_caps.DeviceType != D3DDEVTYPE_SW)`
- 430: enum `D3DPOOL_MANAGED` | `m_ManagedPool = IsExtendedDevice() ? D3DPOOL_MANAGED_INTERNAL : D3DPOOL_MANAGED;`
- 430: enum `D3DPOOL_MANAGED_INTERNAL` | `m_ManagedPool = IsExtendedDevice() ? D3DPOOL_MANAGED_INTERNAL : D3DPOOL_MANAGED;`
- 436: enum `D3DDEVTYPE_SW` | `&& m_caps.DeviceType != D3DDEVTYPE_SW`
- 437: enum `D3DDEVTYPE_REF` | `&& m_caps.DeviceType != D3DDEVTYPE_REF)`
- 456: call `GetBackBuffer` | `IFC(m_pD3DDevice->GetBackBuffer(`
- 459: enum `D3DBACKBUFFER_TYPE_MONO` | `D3DBACKBUFFER_TYPE_MONO,`
- 470: enum `D3DERR_DEVICELOST` | `if (hr == D3DERR_DEVICELOST)`
- 483: call `GetDirect3D` | `IFC( m_pD3DDevice->GetDirect3D(&pd3d9) );`
- 607: call `CheckDeviceFormat` | `m_fSupportsD3DFMT_A8 = SUCCEEDED( pd3d9->CheckDeviceFormat(`
- 612: enum `D3DRTYPE_TEXTURE` | `D3DRTYPE_TEXTURE,`
- 613: enum `D3DFMT_A8` | `D3DFMT_A8`
- 615: call `CheckDeviceFormat` | `m_fSupportsD3DFMT_P8 = SUCCEEDED( pd3d9->CheckDeviceFormat(`
- 620: enum `D3DRTYPE_TEXTURE` | `D3DRTYPE_TEXTURE,`
- 621: enum `D3DFMT_P8` | `D3DFMT_P8`
- 623: call `CheckDeviceFormat` | `m_fSupportsD3DFMT_L8 = SUCCEEDED( pd3d9->CheckDeviceFormat(`
- 628: enum `D3DRTYPE_TEXTURE` | `D3DRTYPE_TEXTURE,`
- 629: enum `D3DFMT_L8` | `D3DFMT_L8`
- 640: call `CheckDeviceFormat` | `SUCCEEDED( pd3d9->CheckDeviceFormat(`
- 645: enum `D3DRTYPE_TEXTURE` | `D3DRTYPE_TEXTURE,`
- 646: enum `D3DFMT_A32B32G32R32F` | `D3DFMT_A32B32G32R32F`
- 655: call `CheckDeviceFormat` | `SUCCEEDED( pd3d9->CheckDeviceFormat(`
- 660: enum `D3DRTYPE_TEXTURE` | `D3DRTYPE_TEXTURE,`
- 661: enum `D3DFMT_A2R10G10B10` | `D3DFMT_A2R10G10B10`
- 666: call `CheckDeviceFormat` | `SUCCEEDED( pd3d9->CheckDeviceFormat(`
- 671: enum `D3DRTYPE_TEXTURE` | `D3DRTYPE_TEXTURE,`
- 672: enum `D3DFMT_A8R8G8B8` | `D3DFMT_A8R8G8B8`
- 677: call `CheckDeviceFormat` | `SUCCEEDED( pd3d9->CheckDeviceFormat(`
- 682: enum `D3DRTYPE_TEXTURE` | `D3DRTYPE_TEXTURE,`
- 683: enum `D3DFMT_X8R8G8B8` | `D3DFMT_X8R8G8B8`
- 711: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE`
- 716: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE MaxMultisampleType`
- 719: enum `D3DMULTISAMPLE_16_SAMPLES` | `Assert(MaxMultisampleType <= D3DMULTISAMPLE_16_SAMPLES);`
- 721: enum `D3DMULTISAMPLE_2_SAMPLES` | `while (MaxMultisampleType >= D3DMULTISAMPLE_2_SAMPLES)`
- 723: call `CheckDeviceMultiSampleType` | `if (SUCCEEDED(pd3d->CheckDeviceMultiSampleType(`
- 734: call `CheckDeviceMultiSampleType` | `\|\| SUCCEEDED(pd3d->CheckDeviceMultiSampleType(`
- 751: enum `D3DMULTISAMPLE_2_SAMPLES` | `if (MaxMultisampleType < D3DMULTISAMPLE_2_SAMPLES)`
- 753: enum `D3DMULTISAMPLE_NONE` | `MaxMultisampleType = D3DMULTISAMPLE_NONE;`
- 779: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE MaxMultisampleType =`
- 780: enum `D3DMULTISAMPLE_4_SAMPLES` | `IsLDDMDevice() ? D3DMULTISAMPLE_4_SAMPLES : D3DMULTISAMPLE_NONE;`
- 780: enum `D3DMULTISAMPLE_NONE` | `IsLDDMDevice() ? D3DMULTISAMPLE_4_SAMPLES : D3DMULTISAMPLE_NONE;`
- 800: enum `D3DFMT_X8R8G8B8` | `D3DFMT_X8R8G8B8,`
- 807: enum `D3DFMT_A8R8G8B8` | `D3DFMT_A8R8G8B8,`
- 814: enum `D3DFMT_A2R10G10B10` | `D3DFMT_A2R10G10B10,`
- 954: enum `D3DCREATE_MULTITHREADED` | `&& !(m_dwD3DBehaviorFlags & D3DCREATE_MULTITHREADED))`
- 960: enum `D3DCREATE_MULTITHREADED` | `if (   (m_dwD3DBehaviorFlags & D3DCREATE_MULTITHREADED)`
- 1036: call `GetDirect3D` | `MIL_THR(m_pD3DDevice->GetDirect3D(&pD3D));`
- 1043: call `GetCreationParameters` | `MIL_THR(m_pD3DDevice->GetCreationParameters(&d3dCreateParams));`
- 1050: enum `D3DDEVTYPE_HAL` | `if (m_caps.DeviceType == D3DDEVTYPE_HAL)`
- 1056: call `CheckDepthStencilMatch` | `MIL_THR(pD3D->CheckDepthStencilMatch(`
- 1058: enum `D3DDEVTYPE_HAL` | `D3DDEVTYPE_HAL,`
- 1088: enum `D3DMULTISAMPLE_NONE` | `D3DMULTISAMPLE_NONE,`
- 1105: enum `D3DPRESENT_PARAMETERS` | `D3DPRESENT_PARAMETERS d3dpp;`
- 1113: enum `D3DMULTISAMPLE_NONE` | `d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;`
- 1114: enum `D3DSWAPEFFECT_COPY` | `d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;`
- 1116: enum `D3DPRESENT_INTERVAL_IMMEDIATE` | `d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;`
- 1117: enum `D3DPRESENTFLAG_LOCKABLE_BACKBUFFER` | `d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;`
- 1130: call `GetBackBuffer` | `MIL_THR(pD3DSwapChain->GetBackBuffer(0,  &pD3DSurface));`
- 1161: enum `D3DRTYPE_TEXTURE` | `d3dsd.Type = D3DRTYPE_TEXTURE;`
- 1164: enum `D3DMULTISAMPLE_NONE` | `d3dsd.MultiSampleType = D3DMULTISAMPLE_NONE;`
- 1210: enum `D3DERR_OUTOFVIDEOMEMORY` | `if (   (hr == D3DERR_OUTOFVIDEOMEMORY)`
- 1212: enum `D3DERR_DRIVERINTERNALERROR` | `\|\| (hr == D3DERR_DRIVERINTERNALERROR))`
- 1305: enum `D3DDEVTYPE_SW` | `if (m_caps.DeviceType == D3DDEVTYPE_SW)`
- 1329: enum `D3DERR_OUTOFVIDEOMEMORY` | `(hr != D3DERR_OUTOFVIDEOMEMORY) &&`
- 1331: enum `D3DERR_DRIVERINTERNALERROR` | `(hr != D3DERR_DRIVERINTERNALERROR) &&`
- 1332: enum `D3DDEVTYPE_HAL` | `(m_caps.DeviceType == D3DDEVTYPE_HAL))`
- 1363: call `GetDC` | `MIL_THRX(hrGetDC, pD3DSurface->GetDC(&hTestDC));`
- 1371: call `ReleaseDC` | `pD3DSurface->ReleaseDC(hTestDC);`
- 1395: enum `D3DPTEXTURECAPS_POW2` | `if (caps.TextureCaps & D3DPTEXTURECAPS_POW2 &&`
- 1396: enum `D3DPTEXTURECAPS_NONPOW2CONDITIONAL` | `!(caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL))`
- 1406: enum `D3DPTEXTURECAPS_SQUAREONLY` | `if (caps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY)`
- 1444: enum `D3DPBLENDCAPS_ZERO` | `\| D3DPBLENDCAPS_ZERO`
- 1445: enum `D3DPBLENDCAPS_ONE` | `\| D3DPBLENDCAPS_ONE`
- 1446: enum `D3DPBLENDCAPS_SRCCOLOR` | `/*          \| D3DPBLENDCAPS_SRCCOLOR                unused*/`
- 1447: enum `D3DPBLENDCAPS_INVSRCCOLOR` | `/*          \| D3DPBLENDCAPS_INVSRCCOLOR             unused*/`
- 1448: enum `D3DPBLENDCAPS_SRCALPHA` | `\| D3DPBLENDCAPS_SRCALPHA`
- 1449: enum `D3DPBLENDCAPS_INVSRCALPHA` | `/*          \| D3DPBLENDCAPS_INVSRCALPHA             unused*/`
- 1450: enum `D3DPBLENDCAPS_DESTALPHA` | `/*          \| D3DPBLENDCAPS_DESTALPHA               unused*/`
- 1451: enum `D3DPBLENDCAPS_INVDESTALPHA` | `\| D3DPBLENDCAPS_INVDESTALPHA`
- 1452: enum `D3DPBLENDCAPS_DESTCOLOR` | `/*          \| D3DPBLENDCAPS_DESTCOLOR               unused*/`
- 1453: enum `D3DPBLENDCAPS_INVDESTCOLOR` | `/*          \| D3DPBLENDCAPS_INVDESTCOLOR            unused*/`
- 1454: enum `D3DPBLENDCAPS_SRCALPHASAT` | `/*          \| D3DPBLENDCAPS_SRCALPHASAT             unused*/`
- 1455: enum `D3DPBLENDCAPS_BOTHSRCALPHA` | `/*          \| D3DPBLENDCAPS_BOTHSRCALPHA            unused*/`
- 1456: enum `D3DPBLENDCAPS_BOTHINVSRCALPHA` | `/*          \| D3DPBLENDCAPS_BOTHINVSRCALPHA         unused*/`
- 1457: enum `D3DPBLENDCAPS_BLENDFACTOR` | `/*          \| D3DPBLENDCAPS_BLENDFACTOR      used but we allow unsupported blend factor*/`
- 1461: enum `D3DPBLENDCAPS_ZERO` | `\| D3DPBLENDCAPS_ZERO`
- 1462: enum `D3DPBLENDCAPS_ONE` | `\| D3DPBLENDCAPS_ONE`
- 1463: enum `D3DPBLENDCAPS_SRCCOLOR` | `/*          \| D3DPBLENDCAPS_SRCCOLOR                unused*/`
- 1464: enum `D3DPBLENDCAPS_INVSRCCOLOR` | `\| D3DPBLENDCAPS_INVSRCCOLOR`
- 1465: enum `D3DPBLENDCAPS_SRCALPHA` | `/*          \| D3DPBLENDCAPS_SRCALPHA                unused*/`
- 1466: enum `D3DPBLENDCAPS_INVSRCALPHA` | `\| D3DPBLENDCAPS_INVSRCALPHA`
- 1467: enum `D3DPBLENDCAPS_DESTALPHA` | `/*          \| D3DPBLENDCAPS_DESTALPHA               unused*/`
- 1468: enum `D3DPBLENDCAPS_INVDESTALPHA` | `/*          \| D3DPBLENDCAPS_INVDESTALPHA            unused*/`
- 1469: enum `D3DPBLENDCAPS_DESTCOLOR` | `/*          \| D3DPBLENDCAPS_DESTCOLOR               unused*/`
- 1470: enum `D3DPBLENDCAPS_INVDESTCOLOR` | `/*          \| D3DPBLENDCAPS_INVDESTCOLOR            unused*/`
- 1471: enum `D3DPBLENDCAPS_SRCALPHASAT` | `/*          \| D3DPBLENDCAPS_SRCALPHASAT             unused*/`
- 1472: enum `D3DPBLENDCAPS_BOTHSRCALPHA` | `/*          \| D3DPBLENDCAPS_BOTHSRCALPHA            unused*/`
- 1473: enum `D3DPBLENDCAPS_BOTHINVSRCALPHA` | `/*          \| D3DPBLENDCAPS_BOTHINVSRCALPHA         unused*/`
- 1474: enum `D3DPBLENDCAPS_BLENDFACTOR` | `/*          \| D3DPBLENDCAPS_BLENDFACTOR      used but we allow unsupported blend factor*/`
- 1514: enum `D3DFMT_X8R8G8B8` | `case D3DFMT_X8R8G8B8:`
- 1518: enum `D3DFMT_A8R8G8B8` | `case D3DFMT_A8R8G8B8:`
- 1522: enum `D3DFMT_A2R10G10B10` | `case D3DFMT_A2R10G10B10:`
- 1687: enum `D3DPRASTERCAPS_SCISSORTEST` | `m_caps.RasterCaps &= ~D3DPRASTERCAPS_SCISSORTEST;`
- 1689: enum `D3DPRASTERCAPS_SCISSORTEST` | `Assert((m_caps.RasterCaps & D3DPRASTERCAPS_SCISSORTEST) == 0);`
- 1713: enum `D3DERR_DRIVERINTERNALERROR` | `if (m_hrDisplayInvalid == D3DERR_DRIVERINTERNALERROR)`
- 1809: call `GetSwapChain` | `IFC(m_pD3DDevice->GetSwapChain(uGroupAdapterOrdinal, &pD3DSwapChain));`
- 1843: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE d3dMultiSampleType,`
- 1887: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE d3dMultiSampleType,`
- 1908: call `CreateRenderTarget` | `hr = m_pD3DDevice->CreateRenderTarget(`
- 1951: call `GetRenderTargetData` | `IFC(m_pD3DDevice->GetRenderTargetData(`
- 1957: enum `D3DERR_DEVICELOST` | `if (hr == D3DERR_DEVICELOST)`
- 1977: enum `D3DPRESENT_PARAMETERS` | `__in_ecount(1) D3DPRESENT_PARAMETERS *pD3DPresentParams,`
- 2010: enum `D3DERR_OUTOFVIDEOMEMORY` | `IFC(D3DERR_OUTOFVIDEOMEMORY);`
- 2022: call `CreateAdditionalSwapChain` | `hr = m_pD3DDevice->CreateAdditionalSwapChain(pD3DPresentParams, &pD3DSwapChain);`
- 2036: enum `D3DFMT_A8R8G8B8` | `\|\| (   pD3DPresentParams->BackBufferFormat != D3DFMT_A8R8G8B8`
- 2037: enum `D3DFMT_X8R8G8B8` | `&& pD3DPresentParams->BackBufferFormat != D3DFMT_X8R8G8B8`
- 2068: enum `D3DERR_DRIVERINTERNALERROR` | `case D3DERR_DRIVERINTERNALERROR:`
- 2069: enum `D3DERR_DRIVERINTERNALERROR` | `m_hrDisplayInvalid = D3DERR_DRIVERINTERNALERROR;`
- 2072: enum `D3DERR_DEVICELOST` | `case D3DERR_DEVICELOST:`
- 2111: call `CreateVertexBuffer` | `hr = m_pD3DDevice->CreateVertexBuffer(`
- 2148: call `CreateIndexBuffer` | `hr = m_pD3DDevice->CreateIndexBuffer(`
- 2192: call `ComposeRects` | `MIL_THR(m_pD3DDeviceEx->ComposeRects(`
- 2252: call `CreateTexture` | `hr = m_pD3DDevice->CreateTexture(`
- 2297: call `CreateStateBlock` | `hr = m_pD3DDevice->CreateStateBlock(`
- 2332: enum `D3DPOOL_SYSTEMMEM` | `(pSurfDesc->Pool == D3DPOOL_SYSTEMMEM));`
- 2336: enum `D3DUSAGE_AUTOGENMIPMAP` | `if (pSurfDesc->Usage & D3DUSAGE_AUTOGENMIPMAP)`
- 2350: call `CreateTexture` | `hr = m_pD3DDevice->CreateTexture(`
- 2429: call `CreateOffscreenPlainSurface` | `hr = m_pD3DDevice->CreateOffscreenPlainSurface(`
- 2433: enum `D3DPOOL_SYSTEMMEM` | `D3DPOOL_SYSTEMMEM,`
- 2468: enum `D3DRTYPE_TEXTURE` | `d3dsdSysMemTexture.Type = D3DRTYPE_TEXTURE;`
- 2470: enum `D3DPOOL_SYSTEMMEM` | `d3dsdSysMemTexture.Pool = D3DPOOL_SYSTEMMEM;`
- 2471: enum `D3DMULTISAMPLE_NONE` | `d3dsdSysMemTexture.MultiSampleType = D3DMULTISAMPLE_NONE;`
- 2482: call `GetSurfaceLevel` | `IFC(pD3DTexture->GetSurfaceLevel(`
- 2526: enum `D3DPOOL_SYSTEMMEM` | `Assert(pSurfDesc->Pool == D3DPOOL_SYSTEMMEM);`
- 2541: call `CreateTexture` | `hr = m_pD3DDevice->CreateTexture(`
- 2585: call `UpdateSurface` | `IFC(m_pD3DDevice->UpdateSurface(`
- 2615: call `UpdateTexture` | `IFC(m_pD3DDevice->UpdateTexture(`
- 2660: call `StretchRect` | `IFC(m_pD3DDevice->StretchRect(`
- 2717: call `SetRenderTarget` | `IFC(m_pD3DDevice->SetRenderTarget(0, pD3DSurface->ID3DSurface()));`
- 2810: call `SetRenderTarget` | `IFC(m_pD3DDevice->SetRenderTarget(0, pD3DSurface->ID3DSurface()));`
- 2918: call `SetRenderTarget` | `IGNORE_HR(m_pD3DDevice->SetRenderTarget(0, m_pD3DDummyBackBuffer));`
- 2973: enum `D3DCLEAR_STENCIL` | `Assert((dwFlags & (D3DCLEAR_ZBUFFER \| D3DCLEAR_STENCIL) ) == 0);`
- 2973: enum `D3DCLEAR_ZBUFFER` | `Assert((dwFlags & (D3DCLEAR_ZBUFFER \| D3DCLEAR_STENCIL) ) == 0);`
- 2982: call `Clear` | `IFC(m_pD3DDevice->Clear(`
- 3001: enum `D3DCLEAR_TARGET` | `if (dwFlags == D3DCLEAR_TARGET`
- 3016: call `Clear` | `IGNORE_HR(m_pD3DDevice->Clear(1, &d3dRect, D3DCLEAR_TARGET, colorGrid, rZValue, 0));`
- 3016: enum `D3DCLEAR_TARGET` | `IGNORE_HR(m_pD3DDevice->Clear(1, &d3dRect, D3DCLEAR_TARGET, colorGrid, rZValue, 0));`
- 3052: call `ColorFill` | `return m_pD3DDevice->ColorFill(`
- 3317: call `Present` | `hr = pD3DSwapChain->Present(`
- 3348: enum `D3DERR_DEVICELOST` | `hr = D3DERR_DEVICELOST;`
- 3431: call `SetRenderTarget` | `IGNORE_HR(m_pD3DDevice->SetRenderTarget(0, m_pD3DDummyBackBuffer));`
- 3449: enum `D3DERR_DRIVERINTERNALERROR` | `else if (hr == E_FAIL \|\| hr == D3DERR_DRIVERINTERNALERROR)`
- 3461: enum `D3DERR_DEVICELOST` | `hr = D3DERR_DEVICELOST;`
- 3476: enum `D3DERR_DEVICELOST` | `if (hr == D3DERR_DEVICELOST \|\|`
- 3477: enum `D3DERR_DEVICEHUNG` | `hr == D3DERR_DEVICEHUNG \|\|`
- 3480: enum `D3DERR_DEVICEREMOVED` | `hr == D3DERR_DEVICEREMOVED`
- 3535: call `GetBackBuffer` | `IFC(pD3DSwapChain->GetBackBuffer(`
- 3583: call `GetDC` | `IFC(pD3DSwapChain->GetDC(`
- 3655: call `ReleaseDC` | `MIL_THR_SECONDARY(pD3DSwapChain->ReleaseDC(`
- 3784: call `BeginScene` | `hr = m_pD3DDevice->BeginScene();`
- 3812: call `EndScene` | `IFC(m_pD3DDevice->EndScene());`
- 3859: enum `D3DPT_TRIANGLEFAN` | `D3DPT_TRIANGLEFAN,`
- 4297: call `CreateVertexShader` | `hr = m_pD3DDevice->CreateVertexShader(`
- 4328: call `CreatePixelShader` | `hr = m_pD3DDevice->CreatePixelShader(`
- 4410: enum `D3DRS_DIFFUSEMATERIALSOURCE` | `IFC(SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1));`
- 4411: enum `D3DRS_SPECULARMATERIALSOURCE` | `IFC(SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR1));`
- 4596: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE`
- 4601: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE MultisampleType;`
- 4786: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE MultisampleType,`
- 4802: call `CreateDepthStencilSurface` | `hr = m_pD3DDevice->CreateDepthStencilSurface(`
- 4820: enum `D3DERR_OUTOFVIDEOMEMORY` | `if ((hr == D3DERR_OUTOFVIDEOMEMORY) && (MultisampleType != D3DMULTISAMPLE_NONE))`
- 4820: enum `D3DMULTISAMPLE_NONE` | `if ((hr == D3DERR_OUTOFVIDEOMEMORY) && (MultisampleType != D3DMULTISAMPLE_NONE))`
- 4861: enum `D3DRS_ZENABLE` | `D3DRS_ZENABLE,`
- 4862: enum `D3DZB_TRUE` | `D3DZB_TRUE`
- 4868: enum `D3DRS_ZENABLE` | `D3DRS_ZENABLE,`
- 4869: enum `D3DZB_FALSE` | `D3DZB_FALSE`
- 4873: enum `D3DRS_STENCILENABLE` | `D3DRS_STENCILENABLE,`
- 4892: enum `D3DRS_ZENABLE` | `IGNORE_HR(SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE));`
- 4892: enum `D3DZB_FALSE` | `IGNORE_HR(SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE));`
- 4893: enum `D3DRS_STENCILENABLE` | `IGNORE_HR(SetRenderState(D3DRS_STENCILENABLE, D3DZB_FALSE));`
- 4893: enum `D3DZB_FALSE` | `IGNORE_HR(SetRenderState(D3DRS_STENCILENABLE, D3DZB_FALSE));`
- 4969: call `SetPaletteEntries` | `IFC( m_pD3DDevice->SetPaletteEntries(0, pPal) );`
- 4970: call `SetCurrentTexturePalette` | `IFC( m_pD3DDevice->SetCurrentTexturePalette(0) );`
- 5000: enum `D3DMULTISAMPLE_NONE` | `D3DMULTISAMPLE_NONE,`
- 5020: enum `D3DTEXF_NONE` | `D3DTEXF_NONE`
- 5059: enum `D3DTEXF_NONE` | `D3DTEXF_NONE`
- 5091: enum `D3DFILL_SOLID` | `D3DFILLMODE d3dOrigFillMode = D3DFILL_SOLID;`
- 5092: enum `D3DCMP_LESSEQUAL` | `D3DCMPFUNC d3dOrigDepthTest = D3DCMP_LESSEQUAL;`
- 5220: enum `D3DRS_FILLMODE` | `D3DRS_FILLMODE,`
- 5225: enum `D3DRS_ZFUNC` | `D3DRS_ZFUNC,`
- 5226: enum `D3DCMP_ALWAYS` | `D3DCMP_ALWAYS`
- 5246: enum `D3DRS_FILLMODE` | `D3DRS_FILLMODE,`
- 5251: enum `D3DRS_ZFUNC` | `D3DRS_ZFUNC,`
- 5259: enum `D3DRS_FILLMODE` | `D3DRS_FILLMODE,`
- 5264: enum `D3DRS_ZFUNC` | `D3DRS_ZFUNC,`
- 5334: call `Lock` | `hr = THR(m_pHwVertexBuffer->Lock(`
- 5345: call `Lock` | `hr = THR(m_pHwIndexBuffer->Lock(`
- 5375: call `DrawIndexedPrimitiveUP` | `IFC(m_pD3DDevice->DrawIndexedPrimitiveUP(`
- 5376: enum `D3DPT_TRIANGLELIST` | `D3DPT_TRIANGLELIST,`
- 5381: enum `D3DFMT_INDEX16` | `D3DFMT_INDEX16,`
- 5397: call `Unlock` | `IFC(m_pHwVertexBuffer->Unlock(uNumVertices));`
- 5406: call `Unlock` | `IFC(m_pHwIndexBuffer->Unlock());`
- 5434: call `DrawIndexedPrimitive` | `IFC(m_pD3DDevice->DrawIndexedPrimitive(`
- 5435: enum `D3DPT_TRIANGLELIST` | `D3DPT_TRIANGLELIST,`
- 5449: call `Unlock` | `IGNORE_HR(m_pHwVertexBuffer->Unlock(uNumVertices));`
- 5453: call `Unlock` | `IGNORE_HR(m_pHwIndexBuffer->Unlock());`
- 5480: call `DrawIndexedPrimitive` | `IFC(m_pD3DDevice->DrawIndexedPrimitive(`
- 5481: enum `D3DPT_TRIANGLELIST` | `D3DPT_TRIANGLELIST,`
- 5513: call `DrawPrimitive` | `IFC(m_pD3DDevice->DrawPrimitive(`
- 5514: enum `D3DPT_TRIANGLELIST` | `D3DPT_TRIANGLELIST,`
- 5543: call `DrawPrimitive` | `IFC(m_pD3DDevice->DrawPrimitive(`
- 5544: enum `D3DPT_TRIANGLESTRIP` | `D3DPT_TRIANGLESTRIP,`
- 5578: enum `D3DPT_LINELIST` | `uVertexStreamZeroStride * ((primitiveType == D3DPT_LINELIST) ? uPrimitiveCount*2 : uPrimitiveCount+2)`
- 5604: enum `D3DPT_LINELIST` | `case D3DPT_LINELIST:`
- 5608: enum `D3DPT_TRIANGLELIST` | `case D3DPT_TRIANGLELIST:`
- 5612: enum `D3DPT_TRIANGLESTRIP` | `case D3DPT_TRIANGLESTRIP:`
- 5656: call `DrawPrimitiveUP` | `IFC(m_pD3DDevice->DrawPrimitiveUP(`
- 5673: enum `D3DPT_LINELIST` | `case D3DPT_LINELIST:`
- 5680: enum `D3DPT_TRIANGLELIST` | `case D3DPT_TRIANGLELIST:`
- 5685: enum `D3DPT_TRIANGLESTRIP` | `case D3DPT_TRIANGLESTRIP:`
- 5721: call `GetSurfaceLevel` | `IFC(pD3DSourceTexture->GetSurfaceLevel(0, &pD3DSurfaceSource));`
- 5722: call `GetSurfaceLevel` | `IFC(pD3DDestinationTexture->GetSurfaceLevel(0, &pD3DSurfaceDestination));`
- 5724: call `StretchRect` | `IFC(m_pD3DDevice->StretchRect(`
- 5729: enum `D3DTEXF_NONE` | `D3DTEXF_NONE`
- 5764: enum `D3DPT_LINELIST` | `uVertexStreamZeroStride * ((primitiveType == D3DPT_LINELIST) ? uPrimitiveCount*2 : uPrimitiveCount+2)`
- 5785: enum `D3DPT_LINELIST` | `case D3DPT_LINELIST:`
- 5789: enum `D3DPT_TRIANGLELIST` | `case D3DPT_TRIANGLELIST:`
- 5793: enum `D3DPT_TRIANGLEFAN` | `case D3DPT_TRIANGLEFAN:`
- 5823: enum `D3DPT_TRIANGLESTRIP` | `case D3DPT_TRIANGLESTRIP:`
- 5880: call `Lock` | `hr = THR(m_pHwVertexBuffer->Lock(`
- 5907: call `DrawPrimitiveUP` | `IFC(m_pD3DDevice->DrawPrimitiveUP(`
- 5925: call `Unlock` | `IFC(m_pHwVertexBuffer->Unlock(uNumVertices));`
- 5949: call `DrawPrimitive` | `IFC(m_pD3DDevice->DrawPrimitive(`
- 5961: call `Unlock` | `IGNORE_HR(m_pHwVertexBuffer->Unlock(uNumVertices));`
- 6031: call `CheckDeviceState` | `MIL_THR(m_pD3DDeviceEx->CheckDeviceState(hwnd));`
- 6049: enum `D3DERR_DEVICELOST` | `else if (   hr == D3DERR_DEVICELOST`
- 6050: enum `D3DERR_DEVICEHUNG` | `\|\| hr == D3DERR_DEVICEHUNG`
- 6052: enum `D3DERR_DEVICEREMOVED` | `\|\| hr == D3DERR_DEVICEREMOVED`
- 6100: call `WaitForVBlank` | `MIL_THR(m_pD3DDeviceEx->WaitForVBlank(uSwapChainIndex));`
- 6118: call `WaitForVBlank` | `MIL_THR(m_pD3DDeviceEx->WaitForVBlank(uSwapChainIndex));`
- 6336: call `CreateQuery` | `MIL_THR(m_pD3DDevice->CreateQuery(`
- 6337: enum `D3DQUERYTYPE_EVENT` | `D3DQUERYTYPE_EVENT,`
- 6368: call `Reset` | `pMarker->Reset(ullMarkerId);`
- 6373: call `CreateQuery` | `IFC(m_pD3DDevice->CreateQuery(D3DQUERYTYPE_EVENT, &pQuery));`
- 6373: enum `D3DQUERYTYPE_EVENT` | `IFC(m_pD3DDevice->CreateQuery(D3DQUERYTYPE_EVENT, &pQuery));`
- 6397: enum `D3DERR_DEVICELOST` | `if (D3DERR_DEVICELOST == hr`
- 6398: enum `D3DERR_NOTAVAILABLE` | `\|\| D3DERR_NOTAVAILABLE == hr)`
- 6451: enum `D3DERR_DEVICELOST` | `if (hr == D3DERR_DEVICELOST)`
- 6552: enum `D3DERR_DRIVERINTERNALERROR` | `TraceTag((tagError, "MIL-HW: Injecting D3DERR_DRIVERINTERNALERROR!."));`
- 6553: enum `D3DERR_DRIVERINTERNALERROR` | `*pHR = D3DERR_DRIVERINTERNALERROR;`
- 6609: enum `D3DUSAGE_WRITEONLY` | `D3DUSAGE_WRITEONLY,`
- 6611: enum `D3DPOOL_DEFAULT` | `D3DPOOL_DEFAULT,`
- 6618: call `Lock` | `IFC(m_pEffectPipelineVertexBuffer->Lock( 0, 0, reinterpret_cast<void**>(&pVertices), 0));`
- 6629: call `Unlock` | `hr = m_pEffectPipelineVertexBuffer->Unlock();`
- 6638: call `Unlock` | `IGNORE_HR(m_pEffectPipelineVertexBuffer->Unlock());`

## core/hw/d3ddevice.h

- 45: enum `D3DPOOL_MANAGED_INTERNAL` | `#define D3DPOOL_MANAGED_INTERNAL ((D3DPOOL)6)`
- 274: enum `D3DPRESENT_PARAMETERS` | `__in_ecount(1) D3DPRESENT_PARAMETERS *pD3DPresentParams,`
- 282: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE d3dMultiSampleType,`
- 292: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE d3dMultiSampleType,`
- 435: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE MultisampleType,`
- 524: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE GetSupportedMultisampleType(`
- 548: enum `D3DDEVTYPE_HAL` | `bool IsHALDevice() const {return m_caps.DeviceType == D3DDEVTYPE_HAL;}`
- 550: enum `D3DCAPS2_CANAUTOGENMIPMAP` | `BOOL CanAutoGenMipMap() const {return m_caps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP;}`
- 552: enum `D3DPTFILTERCAPS_MINFLINEAR` | `bool CanStretchRectGenMipMap() const { return (m_caps.StretchRectFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) != 0; }`
- 553: enum `D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES` | `bool CanStretchRectFromTextures() const { return (m_caps.DevCaps2 & D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES) != 0; }`
- 559: enum `D3DPTADDRESSCAPS_BORDER` | `bool SupportsBorderColor() const { return (m_caps.TextureAddressCaps & D3DPTADDRESSCAPS_BORDER) != 0; }`
- 561: enum `D3DPTEXTURECAPS_NONPOW2CONDITIONAL` | `bool SupportsNonPow2Conditionally() const { return (m_caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL) && (m_caps.TextureCaps & D3D...`
- 561: enum `D3DPTEXTURECAPS_POW2` | `bool SupportsNonPow2Conditionally() const { return (m_caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL) && (m_caps.TextureCaps & D3D...`
- 562: enum `D3DPTEXTURECAPS_NONPOW2CONDITIONAL` | `bool SupportsNonPow2Unconditionally() const { return !(m_caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL) && !(m_caps.TextureCaps &...`
- 562: enum `D3DPTEXTURECAPS_POW2` | `bool SupportsNonPow2Unconditionally() const { return !(m_caps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL) && !(m_caps.TextureCaps &...`
- 598: enum `D3DPRASTERCAPS_SCISSORTEST` | `BOOL SupportsScissorRect() const {return m_caps.RasterCaps & D3DPRASTERCAPS_SCISSORTEST;}`
- 600: enum `D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION` | `BOOL SupportsLinearTosRGBPresentation() const {return m_caps.Caps3 & D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION;}`
- 605: enum `D3DCREATE_PUREDEVICE` | `bool IsPureDevice() const { return (m_dwD3DBehaviorFlags & D3DCREATE_PUREDEVICE) != 0; }`
- 700: enum `D3DPT_LINELIST` | `uVertexStreamZeroStride * ((primitiveType == D3DPT_LINELIST) ? uPrimitiveCount*2 : uPrimitiveCount+2)`
- 722: call `GetRasterStatus` | `return m_pD3DDevice->GetRasterStatus(uSwapChain, pRasterStatus);`
- 778: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE GetMaxMultisampleTypeWithDepthSupport(`
- 782: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE MaxMultisampleType`
- 833: enum `D3DPT_LINELIST` | `uVertexStreamZeroStride * ((primitiveType == D3DPT_LINELIST) ? uPrimitiveCount*2 : uPrimitiveCount+2)`
- 865: enum `D3DERR_DRIVERINTERNALERROR` | `if (hr == D3DERR_DRIVERINTERNALERROR)`
- 870: enum `D3DERR_DRIVERINTERNALERROR` | `m_hrDisplayInvalid = D3DERR_DRIVERINTERNALERROR;`
- 981: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE m_multisampleTypeFor32bppBGR;`
- 982: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE m_multisampleTypeFor32bppPBGRA;`
- 983: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE m_multisampleTypeFor32bppBGR101010;`
- 1096: enum `D3DPTFILTERCAPS_MINFLINEAR` | `return (m_caps.StretchRectFilterCaps & D3DPTFILTERCAPS_MINFLINEAR);`

## core/hw/d3ddevicemanager.cpp

- 292: enum `D3DPRESENT_PARAMETERS` | `D3DPRESENT_PARAMETERS presentParams;`
- 294: enum `D3DPRESENT_PARAMETERS` | `ZeroMemory(&presentParams, sizeof(D3DPRESENT_PARAMETERS));`
- 297: enum `D3DFMT_X8R8G8B8` | `presentParams.BackBufferFormat = D3DFMT_X8R8G8B8;`
- 299: enum `D3DSWAPEFFECT_DISCARD` | `presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;`
- 303: enum `D3DFMT_UNKNOWN` | `presentParams.AutoDepthStencilFormat = D3DFMT_UNKNOWN;`
- 306: enum `D3DCREATE_SOFTWARE_VERTEXPROCESSING` | `D3DCREATE_SOFTWARE_VERTEXPROCESSING \|`
- 307: enum `D3DCREATE_MULTITHREADED` | `D3DCREATE_MULTITHREADED \|`
- 308: enum `D3DCREATE_FPU_PRESERVE` | `D3DCREATE_FPU_PRESERVE \|`
- 309: enum `D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX` | `D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX;`
- 319: call `CreateDevice` | `IFC(m_pID3D->CreateDevice(`
- 321: enum `D3DDEVTYPE_SW` | `D3DDEVTYPE_SW,`
- 348: enum `D3DERR_DEVICELOST` | `if (hr == D3DERR_DEVICELOST)`
- 622: enum `D3DFMT_A8R8G8B8` | `*pFmt = D3DFMT_A8R8G8B8;`
- 626: enum `D3DFMT_X8R8G8B8` | `*pFmt = D3DFMT_X8R8G8B8;`
- 655: call `CheckDeviceType` | `IFC(pID3D->CheckDeviceType(`
- 686: enum `D3DCREATE_ADAPTERGROUP_DEVICE` | `Assert(!(pCreateParams->BehaviorFlags & D3DCREATE_ADAPTERGROUP_DEVICE));`
- 730: enum `D3DDEVTYPE_HAL` | `D3DDEVTYPE_HAL,`
- 787: enum `D3DDEVTYPE_SW` | `(pCreateParams->DeviceType == D3DDEVTYPE_SW)`
- 792: enum `D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX` | `& ~D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX) == 0)`
- 901: enum `D3DPRESENT_PARAMETERS` | `__out_ecount_opt(1) D3DPRESENT_PARAMETERS *pPresentParams,`
- 932: enum `D3DDEVTYPE_SW` | `if (type != D3DDEVTYPE_SW)`
- 945: enum `D3DDEVTYPE_HAL` | `if (type == D3DDEVTYPE_HAL)`
- 947: call `GetAdapterCount` | `if (uAdapter >= m_pID3D->GetAdapterCount())`
- 971: enum `D3DDEVTYPE_SW` | `if (type == D3DDEVTYPE_SW)`
- 1031: enum `D3DPRESENT_PARAMETERS` | `D3DPRESENT_PARAMETERS oPresentParameters;`
- 1322: call `GetDeviceCaps` | `IFC(m_pID3D->GetDeviceCaps(`
- 1339: enum `D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX` | `pD3DCreateParams->BehaviorFlags = D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX;`
- 1347: enum `D3DCAPS2_CANSHARERESOURCE` | `if ((caps.Caps2 & D3DCAPS2_CANSHARERESOURCE))`
- 1356: enum `D3DCREATE_SCREENSAVER` | `pD3DCreateParams->BehaviorFlags \|= D3DCREATE_SCREENSAVER;`
- 1363: enum `D3DCREATE_DISABLE_PSGP_THREADING` | `pD3DCreateParams->BehaviorFlags \|= D3DCREATE_DISABLE_PSGP_THREADING;`
- 1370: enum `D3DCREATE_FPU_PRESERVE` | `pD3DCreateParams->BehaviorFlags \|= D3DCREATE_FPU_PRESERVE;`
- 1375: enum `D3DCREATE_MULTITHREADED` | `pD3DCreateParams->BehaviorFlags \|= D3DCREATE_MULTITHREADED;`
- 1383: enum `D3DCREATE_DISABLE_DRIVER_MANAGEMENT` | `pD3DCreateParams->BehaviorFlags \|= D3DCREATE_DISABLE_DRIVER_MANAGEMENT;`
- 1393: enum `D3DDEVCAPS_HWTRANSFORMANDLIGHT` | `if ((caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0 &&`
- 1394: enum `D3DDEVTYPE_SW` | `type != D3DDEVTYPE_SW`
- 1397: enum `D3DCREATE_HARDWARE_VERTEXPROCESSING` | `pD3DCreateParams->BehaviorFlags \|= D3DCREATE_HARDWARE_VERTEXPROCESSING;`
- 1403: enum `D3DDEVCAPS_PUREDEVICE` | `if (((caps.DevCaps & D3DDEVCAPS_PUREDEVICE) != 0)`
- 1406: enum `D3DCREATE_PUREDEVICE` | `pD3DCreateParams->BehaviorFlags \|= D3DCREATE_PUREDEVICE;`
- 1411: enum `D3DCREATE_SOFTWARE_VERTEXPROCESSING` | `pD3DCreateParams->BehaviorFlags \|= D3DCREATE_SOFTWARE_VERTEXPROCESSING;`
- 1438: enum `D3DPRESENT_PARAMETERS` | `__out_ecount(1) D3DPRESENT_PARAMETERS *pD3DPresentParams`
- 1453: enum `D3DMULTISAMPLE_NONE` | `pD3DPresentParams->MultiSampleType = D3DMULTISAMPLE_NONE;`
- 1459: enum `D3DSWAPEFFECT_COPY` | `pD3DPresentParams->SwapEffect = D3DSWAPEFFECT_COPY;`
- 1463: enum `D3DSWAPEFFECT_DISCARD` | `pD3DPresentParams->SwapEffect = D3DSWAPEFFECT_DISCARD;`
- 1469: enum `D3DFMT_UNKNOWN` | `pD3DPresentParams->AutoDepthStencilFormat = D3DFMT_UNKNOWN;`
- 1477: enum `D3DPRESENTFLAG_DEVICECLIP` | `pD3DPresentParams->Flags \|= D3DPRESENTFLAG_DEVICECLIP;`
- 1482: enum `D3DPRESENT_INTERVAL_IMMEDIATE` | `pD3DPresentParams->PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;`
- 1486: enum `D3DPRESENT_INTERVAL_ONE` | `pD3DPresentParams->PresentationInterval = D3DPRESENT_INTERVAL_ONE;`
- 1495: enum `D3DPRESENTFLAG_LOCKABLE_BACKBUFFER` | `pD3DPresentParams->Flags \|= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;`
- 1520: enum `D3DPRESENT_PARAMETERS` | `D3DPRESENT_PARAMETERS *pBasePresentParams,`
- 1536: enum `D3DPRESENT_PARAMETERS` | `D3DPRESENT_PARAMETERS *rgPresentParams;`
- 1544: enum `D3DPRESENT_PARAMETERS` | `DynArrayIA<D3DPRESENT_PARAMETERS, 4> drgPresentParams;`
- 1582: call `GetAdapterDisplayMode` | `MIL_THR(m_pID3D->GetAdapterDisplayMode(`
- 1606: call `CreateDeviceEx` | `MIL_THR(pID3DEx->CreateDeviceEx(`
- 1623: call `CreateDevice` | `MIL_THR(m_pID3D->CreateDevice(`
- 1640: enum `D3DERR_INVALIDCALL` | `if (hr == D3DERR_INVALIDCALL`
- 1641: enum `D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX` | `&& (pD3DCreateParams->BehaviorFlags & D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX))`
- 1643: enum `D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX` | `pD3DCreateParams->BehaviorFlags &= ~D3DCREATE_DISABLE_DRIVER_MANAGEMENT_EX;`
- 1647: call `CreateDeviceEx` | `MIL_THR(pID3DEx->CreateDeviceEx(`
- 1664: call `CreateDevice` | `MIL_THR(m_pID3D->CreateDevice(`
- 1683: enum `D3DERR_DEVICELOST` | `if (hr == D3DERR_DEVICELOST)`

## core/hw/d3dglyphbank.cpp

- 337: call `LockRect` | `IFC(pTempSurface->LockRect(&lockedRect, &rcTemp, 0 ) );`
- 356: call `UnlockRect` | `IFC( pTempSurface->UnlockRect() );`
- 481: call `UnlockRect` | `IFC( pTempSurface->UnlockRect() );`
- 486: call `UpdateSurface` | `IFC( m_pDevice->UpdateSurface(`
- 537: enum `D3DRTYPE_TEXTURE` | `sd.Type = D3DRTYPE_TEXTURE;`
- 539: enum `D3DPOOL_SYSTEMMEM` | `sd.Pool = D3DPOOL_SYSTEMMEM;`
- 540: enum `D3DMULTISAMPLE_NONE` | `sd.MultiSampleType = D3DMULTISAMPLE_NONE;`
- 545: call `CreateTexture` | `IFC( m_pDevice->CreateTexture(&sd, 1, &pTexture) );`
- 547: call `GetSurfaceLevel` | `IFC( pTexture->GetSurfaceLevel(0, &pSurface) );`
- 592: enum `D3DRTYPE_TEXTURE` | `sd.Type = D3DRTYPE_TEXTURE;`
- 594: enum `D3DPOOL_DEFAULT` | `sd.Pool = D3DPOOL_DEFAULT;`
- 595: enum `D3DMULTISAMPLE_NONE` | `sd.MultiSampleType = D3DMULTISAMPLE_NONE;`
- 600: call `CreateTexture` | `IFC( m_pDevice->CreateTexture(&sd, 1, &pTexture) );`
- 602: call `GetSurfaceLevel` | `IFC( pTexture->GetSurfaceLevel(0, &pSurface) );`

## core/hw/d3dlockabletexture.cpp

- 148: enum `D3DPOOL_SYSTEMMEM` | `(m_sdLevel0.Pool == D3DPOOL_SYSTEMMEM));`
- 150: call `LockRect` | `IFC(m_pD3DTexture->LockRect(0, pLockedRect, pRect, dwFlags));`
- 170: call `UnlockRect` | `IFC(m_pD3DTexture->UnlockRect(0));`
- 192: call `AddDirtyRect` | `IFC(m_pD3DTexture->AddDirtyRect(&rc));`
- 422: call `LockRect` | `IFC(pTexture->LockRect(pD3DLockedRect, &rcLock, D3DLOCK_NO_DIRTY_UPDATE));`
- 422: enum `D3DLOCK_NO_DIRTY_UPDATE` | `IFC(pTexture->LockRect(pD3DLockedRect, &rcLock, D3DLOCK_NO_DIRTY_UPDATE));`
- 424: call `AddDirtyRect` | `IFC(pTexture->AddDirtyRect(rcLock));`

## core/hw/d3dlockabletexture.h

- 161: call `UnlockRect` | `IGNORE_HR(m_texturePair.m_pTextureMain->UnlockRect());`
- 165: call `UnlockRect` | `IGNORE_HR(m_texturePair.m_pTextureAux->UnlockRect());`

## core/hw/d3dregistry.cpp

- 244: call `GetAdapterCount` | `m_cNumAdapters = pD3D->GetAdapterCount();`

## core/hw/d3drenderstate.cpp

- 104: enum `D3DRS_ALPHABLENDENABLE` | `D3DRS_ALPHABLENDENABLE   ,`
- 105: enum `D3DRS_SRCBLEND` | `D3DRS_SRCBLEND           ,`
- 106: enum `D3DRS_DESTBLEND` | `D3DRS_DESTBLEND          ,`
- 163: enum `D3DTSS_COLORARG1` | `D3DTSS_COLORARG1            ,`
- 164: enum `D3DTSS_COLORARG2` | `D3DTSS_COLORARG2            ,`
- 165: enum `D3DTSS_ALPHAOP` | `D3DTSS_ALPHAOP              ,`
- 166: enum `D3DTSS_ALPHAARG1` | `D3DTSS_ALPHAARG1            ,`
- 167: enum `D3DTSS_ALPHAARG2` | `D3DTSS_ALPHAARG2            ,`
- 213: enum `D3DSAMP_MAGFILTER` | `D3DSAMP_MAGFILTER,`
- 214: enum `D3DSAMP_MINFILTER` | `D3DSAMP_MINFILTER,`
- 215: enum `D3DSAMP_MIPFILTER` | `D3DSAMP_MIPFILTER,`
- 240: enum `D3DBLEND_ONE` | `/* D3DRENDERSTATE_SRCBLEND           */ D3DBLEND_ONE    ,`
- 241: enum `D3DBLEND_ZERO` | `/* D3DRENDERSTATE_DESTBLEND          */ D3DBLEND_ZERO   ,`
- 250: enum `D3DBLEND_ONE` | `/* D3DRENDERSTATE_SRCBLEND           */ D3DBLEND_ONE         ,`
- 251: enum `D3DBLEND_INVSRCALPHA` | `/* D3DRENDERSTATE_DESTBLEND          */ D3DBLEND_INVSRCALPHA ,`
- 260: enum `D3DBLEND_INVDESTALPHA` | `/* D3DRENDERSTATE_SRCBLEND           */ D3DBLEND_INVDESTALPHA,`
- 261: enum `D3DBLEND_ONE` | `/* D3DRENDERSTATE_DESTBLEND          */ D3DBLEND_ONE         ,`
- 270: enum `D3DBLEND_ZERO` | `/* D3DRENDERSTATE_SRCBLEND           */ D3DBLEND_ZERO        ,`
- 271: enum `D3DBLEND_SRCALPHA` | `/* D3DRENDERSTATE_DESTBLEND          */ D3DBLEND_SRCALPHA    ,`
- 279: enum `D3DBLEND_ZERO` | `/* D3DRENDERSTATE_SRCBLEND           */ D3DBLEND_ZERO        ,`
- 280: enum `D3DBLEND_INVSRCALPHA` | `/* D3DRENDERSTATE_DESTBLEND          */ D3DBLEND_INVSRCALPHA ,`
- 289: enum `D3DBLEND_SRCALPHA` | `/* D3DRENDERSTATE_SRCBLEND           */ D3DBLEND_SRCALPHA    ,`
- 290: enum `D3DBLEND_INVSRCALPHA` | `/* D3DRENDERSTATE_DESTBLEND          */ D3DBLEND_INVSRCALPHA ,`
- 300: enum `D3DBLEND_INVSRCALPHA` | `/* D3DRENDERSTATE_SRCBLEND           */ D3DBLEND_INVSRCALPHA ,`
- 301: enum `D3DBLEND_SRCALPHA` | `/* D3DRENDERSTATE_DESTBLEND          */ D3DBLEND_SRCALPHA    ,`
- 313: enum `D3DBLEND_BLENDFACTOR` | `/* D3DRENDERSTATE_SRCBLEND           */ D3DBLEND_BLENDFACTOR ,`
- 314: enum `D3DBLEND_INVSRCCOLOR` | `/* D3DRENDERSTATE_DESTBLEND          */ D3DBLEND_INVSRCCOLOR ,`
- 323: enum `D3DBLEND_ZERO` | `/* D3DRENDERSTATE_SRCBLEND           */ D3DBLEND_ZERO        ,`
- 324: enum `D3DBLEND_INVSRCCOLOR` | `/* D3DRENDERSTATE_DESTBLEND          */ D3DBLEND_INVSRCCOLOR ,`
- 335: enum `D3DBLEND_ONE` | `/* D3DRENDERSTATE_SRCBLEND           */ D3DBLEND_ONE         ,`
- 336: enum `D3DBLEND_ONE` | `/* D3DRENDERSTATE_DESTBLEND          */ D3DBLEND_ONE         ,`
- 342: enum `D3DBLEND_INVDESTCOLOR` | `/* D3DRENDERSTATE_SRCBLEND           */ D3DBLEND_INVDESTCOLOR,`
- 343: enum `D3DBLEND_SRCALPHA` | `/* D3DRENDERSTATE_DESTBLEND          */ D3DBLEND_SRCALPHA    ,`
- 358: enum `D3DTSS_COLOROP` | `/* D3DTSS_COLOROP               */ MILTOP_UNKNOWN    ,`
- 359: enum `D3DTSS_COLORARG1` | `/* D3DTSS_COLORARG1             */ MILRS_UNKNOWN     ,`
- 360: enum `D3DTSS_COLORARG2` | `/* D3DTSS_COLORARG2             */ MILRS_UNKNOWN     ,`
- 361: enum `D3DTSS_ALPHAOP` | `/* D3DTSS_ALPHAOP               */ MILTOP_UNKNOWN    ,`
- 362: enum `D3DTSS_ALPHAARG1` | `/* D3DTSS_ALPHAARG1             */ MILRS_UNKNOWN     ,`
- 363: enum `D3DTSS_ALPHAARG2` | `/* D3DTSS_ALPHAARG2             */ MILRS_UNKNOWN     ,`
- 369: enum `D3DTOP_SELECTARG1` | `/* D3DTSS_COLOROP               */ D3DTOP_SELECTARG1 ,`
- 369: enum `D3DTSS_COLOROP` | `/* D3DTSS_COLOROP               */ D3DTOP_SELECTARG1 ,`
- 370: enum `D3DTA_DIFFUSE` | `/* D3DTSS_COLORARG1             */ D3DTA_DIFFUSE     ,`
- 370: enum `D3DTSS_COLORARG1` | `/* D3DTSS_COLORARG1             */ D3DTA_DIFFUSE     ,`
- 371: enum `D3DTA_CURRENT` | `/* D3DTSS_COLORARG2             */ D3DTA_CURRENT     ,/*UNUSED*/`
- 371: enum `D3DTSS_COLORARG2` | `/* D3DTSS_COLORARG2             */ D3DTA_CURRENT     ,/*UNUSED*/`
- 372: enum `D3DTOP_SELECTARG1` | `/* D3DTSS_ALPHAOP               */ D3DTOP_SELECTARG1 ,`
- 372: enum `D3DTSS_ALPHAOP` | `/* D3DTSS_ALPHAOP               */ D3DTOP_SELECTARG1 ,`
- 373: enum `D3DTA_DIFFUSE` | `/* D3DTSS_ALPHAARG1             */ D3DTA_DIFFUSE     ,`
- 373: enum `D3DTSS_ALPHAARG1` | `/* D3DTSS_ALPHAARG1             */ D3DTA_DIFFUSE     ,`
- 374: enum `D3DTA_CURRENT` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,/*UNUSED*/`
- 374: enum `D3DTSS_ALPHAARG2` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,/*UNUSED*/`
- 382: enum `D3DTOP_SELECTARG1` | `/* D3DTSS_COLOROP               */ D3DTOP_SELECTARG1 ,`
- 382: enum `D3DTSS_COLOROP` | `/* D3DTSS_COLOROP               */ D3DTOP_SELECTARG1 ,`
- 383: enum `D3DTA_TEXTURE` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,`
- 383: enum `D3DTSS_COLORARG1` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,`
- 384: enum `D3DTA_CURRENT` | `/* D3DTSS_COLORARG2             */ D3DTA_CURRENT     ,/*UNUSED*/`
- 384: enum `D3DTSS_COLORARG2` | `/* D3DTSS_COLORARG2             */ D3DTA_CURRENT     ,/*UNUSED*/`
- 385: enum `D3DTOP_SELECTARG1` | `/* D3DTSS_ALPHAOP               */ D3DTOP_SELECTARG1 ,`
- 385: enum `D3DTSS_ALPHAOP` | `/* D3DTSS_ALPHAOP               */ D3DTOP_SELECTARG1 ,`
- 386: enum `D3DTA_TEXTURE` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,`
- 386: enum `D3DTSS_ALPHAARG1` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,`
- 387: enum `D3DTA_CURRENT` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,/*UNUSED*/`
- 387: enum `D3DTSS_ALPHAARG2` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,/*UNUSED*/`
- 396: enum `D3DTOP_MODULATE` | `/* D3DTSS_COLOROP               */ D3DTOP_MODULATE   ,`
- 396: enum `D3DTSS_COLOROP` | `/* D3DTSS_COLOROP               */ D3DTOP_MODULATE   ,`
- 397: enum `D3DTA_TEXTURE` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,`
- 397: enum `D3DTSS_COLORARG1` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,`
- 398: enum `D3DTA_CURRENT` | `/* D3DTSS_COLORARG2             */ D3DTA_CURRENT     ,/*DIFFUSE in stage 0*/`
- 398: enum `D3DTSS_COLORARG2` | `/* D3DTSS_COLORARG2             */ D3DTA_CURRENT     ,/*DIFFUSE in stage 0*/`
- 399: enum `D3DTOP_SELECTARG1` | `/* D3DTSS_ALPHAOP               */ D3DTOP_SELECTARG1 ,`
- 399: enum `D3DTSS_ALPHAOP` | `/* D3DTSS_ALPHAOP               */ D3DTOP_SELECTARG1 ,`
- 400: enum `D3DTA_TEXTURE` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,`
- 400: enum `D3DTSS_ALPHAARG1` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,`
- 401: enum `D3DTA_CURRENT` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,/*UNUSED*/`
- 401: enum `D3DTSS_ALPHAARG2` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,/*UNUSED*/`
- 409: enum `D3DTOP_MODULATE` | `/* D3DTSS_COLOROP               */ D3DTOP_MODULATE   ,`
- 409: enum `D3DTSS_COLOROP` | `/* D3DTSS_COLOROP               */ D3DTOP_MODULATE   ,`
- 410: enum `D3DTA_TEXTURE` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,`
- 410: enum `D3DTSS_COLORARG1` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,`
- 411: enum `D3DTA_SPECULAR` | `/* D3DTSS_COLORARG2             */ D3DTA_SPECULAR    ,`
- 411: enum `D3DTSS_COLORARG2` | `/* D3DTSS_COLORARG2             */ D3DTA_SPECULAR    ,`
- 412: enum `D3DTOP_SELECTARG1` | `/* D3DTSS_ALPHAOP               */ D3DTOP_SELECTARG1 ,`
- 412: enum `D3DTSS_ALPHAOP` | `/* D3DTSS_ALPHAOP               */ D3DTOP_SELECTARG1 ,`
- 413: enum `D3DTA_TEXTURE` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,`
- 413: enum `D3DTSS_ALPHAARG1` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,`
- 414: enum `D3DTA_CURRENT` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,/*UNUSED*/`
- 414: enum `D3DTSS_ALPHAARG2` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,/*UNUSED*/`
- 420: enum `D3DTOP_MODULATE` | `/* D3DTSS_COLOROP               */ D3DTOP_MODULATE   ,`
- 420: enum `D3DTSS_COLOROP` | `/* D3DTSS_COLOROP               */ D3DTOP_MODULATE   ,`
- 421: enum `D3DTA_TEXTURE` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,`
- 421: enum `D3DTSS_COLORARG1` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,`
- 422: enum `D3DTA_CURRENT` | `/* D3DTSS_COLORARG2             */ D3DTA_CURRENT     ,/*DIFFUSE in stage 0*/`
- 422: enum `D3DTSS_COLORARG2` | `/* D3DTSS_COLORARG2             */ D3DTA_CURRENT     ,/*DIFFUSE in stage 0*/`
- 423: enum `D3DTOP_MODULATE` | `/* D3DTSS_ALPHAOP               */ D3DTOP_MODULATE   ,`
- 423: enum `D3DTSS_ALPHAOP` | `/* D3DTSS_ALPHAOP               */ D3DTOP_MODULATE   ,`
- 424: enum `D3DTA_TEXTURE` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,`
- 424: enum `D3DTSS_ALPHAARG1` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,`
- 425: enum `D3DTA_CURRENT` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,/*DIFFUSE in stage 0*/`
- 425: enum `D3DTSS_ALPHAARG2` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,/*DIFFUSE in stage 0*/`
- 431: enum `D3DTOP_MODULATE` | `/* D3DTSS_COLOROP               */ D3DTOP_MODULATE   ,`
- 431: enum `D3DTSS_COLOROP` | `/* D3DTSS_COLOROP               */ D3DTOP_MODULATE   ,`
- 432: enum `D3DTA_TEXTURE` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,`
- 432: enum `D3DTSS_COLORARG1` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,`
- 433: enum `D3DTA_DIFFUSE` | `/* D3DTSS_COLORARG2             */ D3DTA_DIFFUSE     ,`
- 433: enum `D3DTSS_COLORARG2` | `/* D3DTSS_COLORARG2             */ D3DTA_DIFFUSE     ,`
- 434: enum `D3DTOP_MODULATE` | `/* D3DTSS_ALPHAOP               */ D3DTOP_MODULATE   ,`
- 434: enum `D3DTSS_ALPHAOP` | `/* D3DTSS_ALPHAOP               */ D3DTOP_MODULATE   ,`
- 435: enum `D3DTA_TEXTURE` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,`
- 435: enum `D3DTSS_ALPHAARG1` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,`
- 436: enum `D3DTA_CURRENT` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,/*DIFFUSE in stage 0*/`
- 436: enum `D3DTSS_ALPHAARG2` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,/*DIFFUSE in stage 0*/`
- 445: enum `D3DTOP_MODULATE` | `/* D3DTSS_COLOROP               */ D3DTOP_MODULATE   ,`
- 445: enum `D3DTSS_COLOROP` | `/* D3DTSS_COLOROP               */ D3DTOP_MODULATE   ,`
- 446: enum `D3DTA_TEXTURE` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,`
- 446: enum `D3DTSS_COLORARG1` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,`
- 447: enum `D3DTA_CURRENT` | `/* D3DTSS_COLORARG2             */ D3DTA_CURRENT     ,/*DIFFUSE in stage 0*/`
- 447: enum `D3DTSS_COLORARG2` | `/* D3DTSS_COLORARG2             */ D3DTA_CURRENT     ,/*DIFFUSE in stage 0*/`
- 448: enum `D3DTOP_SELECTARG1` | `/* D3DTSS_ALPHAOP               */ D3DTOP_SELECTARG1 ,`
- 448: enum `D3DTSS_ALPHAOP` | `/* D3DTSS_ALPHAOP               */ D3DTOP_SELECTARG1 ,`
- 449: enum `D3DTA_CURRENT` | `/* D3DTSS_ALPHAARG1             */ D3DTA_CURRENT     ,/*DIFFUSE in stage 0*/`
- 449: enum `D3DTSS_ALPHAARG1` | `/* D3DTSS_ALPHAARG1             */ D3DTA_CURRENT     ,/*DIFFUSE in stage 0*/`
- 450: enum `D3DTA_CURRENT` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,/*UNUSED*/`
- 450: enum `D3DTSS_ALPHAARG2` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,/*UNUSED*/`
- 459: enum `D3DTOP_MODULATE` | `/* D3DTSS_COLOROP               */ D3DTOP_MODULATE   ,`
- 459: enum `D3DTSS_COLOROP` | `/* D3DTSS_COLOROP               */ D3DTOP_MODULATE   ,`
- 460: enum `D3DTA_TEXTURE` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,`
- 460: enum `D3DTSS_COLORARG1` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,`
- 461: enum `D3DTA_DIFFUSE` | `/* D3DTSS_COLORARG2             */ D3DTA_DIFFUSE     ,`
- 461: enum `D3DTSS_COLORARG2` | `/* D3DTSS_COLORARG2             */ D3DTA_DIFFUSE     ,`
- 462: enum `D3DTOP_SELECTARG1` | `/* D3DTSS_ALPHAOP               */ D3DTOP_SELECTARG1 ,`
- 462: enum `D3DTSS_ALPHAOP` | `/* D3DTSS_ALPHAOP               */ D3DTOP_SELECTARG1 ,`
- 463: enum `D3DTA_CURRENT` | `/* D3DTSS_ALPHAARG1             */ D3DTA_CURRENT     ,/*DIFFUSE in stage 0*/`
- 463: enum `D3DTSS_ALPHAARG1` | `/* D3DTSS_ALPHAARG1             */ D3DTA_CURRENT     ,/*DIFFUSE in stage 0*/`
- 464: enum `D3DTA_CURRENT` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,/*UNUSED*/`
- 464: enum `D3DTSS_ALPHAARG2` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,/*UNUSED*/`
- 470: enum `D3DTOP_MODULATE` | `/* D3DTSS_COLOROP               */ D3DTOP_MODULATE   ,`
- 470: enum `D3DTSS_COLOROP` | `/* D3DTSS_COLOROP               */ D3DTOP_MODULATE   ,`
- 471: enum `D3DTA_ALPHAREPLICATE` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE \| D3DTA_ALPHAREPLICATE,`
- 471: enum `D3DTA_TEXTURE` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE \| D3DTA_ALPHAREPLICATE,`
- 471: enum `D3DTSS_COLORARG1` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE \| D3DTA_ALPHAREPLICATE,`
- 472: enum `D3DTA_CURRENT` | `/* D3DTSS_COLORARG2             */ D3DTA_CURRENT     ,`
- 472: enum `D3DTSS_COLORARG2` | `/* D3DTSS_COLORARG2             */ D3DTA_CURRENT     ,`
- 473: enum `D3DTOP_MODULATE` | `/* D3DTSS_ALPHAOP               */ D3DTOP_MODULATE   ,`
- 473: enum `D3DTSS_ALPHAOP` | `/* D3DTSS_ALPHAOP               */ D3DTOP_MODULATE   ,`
- 474: enum `D3DTA_TEXTURE` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,`
- 474: enum `D3DTSS_ALPHAARG1` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,`
- 475: enum `D3DTA_CURRENT` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,`
- 475: enum `D3DTSS_ALPHAARG2` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,`
- 481: enum `D3DTOP_BUMPENVMAP` | `/* D3DTSS_COLOROP               */ D3DTOP_BUMPENVMAP ,`
- 481: enum `D3DTSS_COLOROP` | `/* D3DTSS_COLOROP               */ D3DTOP_BUMPENVMAP ,`
- 482: enum `D3DTA_TEXTURE` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,/*UNUSED*/`
- 482: enum `D3DTSS_COLORARG1` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,/*UNUSED*/`
- 483: enum `D3DTA_DIFFUSE` | `/* D3DTSS_COLORARG2             */ D3DTA_DIFFUSE     ,/*UNUSED*/`
- 483: enum `D3DTSS_COLORARG2` | `/* D3DTSS_COLORARG2             */ D3DTA_DIFFUSE     ,/*UNUSED*/`
- 484: enum `D3DTOP_MODULATE` | `/* D3DTSS_ALPHAOP               */ D3DTOP_MODULATE   ,/*UNUSED*/`
- 484: enum `D3DTSS_ALPHAOP` | `/* D3DTSS_ALPHAOP               */ D3DTOP_MODULATE   ,/*UNUSED*/`
- 485: enum `D3DTA_TEXTURE` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,/*UNUSED*/`
- 485: enum `D3DTSS_ALPHAARG1` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,/*UNUSED*/`
- 486: enum `D3DTA_CURRENT` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,/*UNUSED*/`
- 486: enum `D3DTSS_ALPHAARG2` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,/*UNUSED*/`
- 493: enum `D3DTOP_SELECTARG1` | `/* D3DTSS_COLOROP               */ D3DTOP_SELECTARG1 ,`
- 493: enum `D3DTSS_COLOROP` | `/* D3DTSS_COLOROP               */ D3DTOP_SELECTARG1 ,`
- 494: enum `D3DTA_TEXTURE` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,`
- 494: enum `D3DTSS_COLORARG1` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,`
- 495: enum `D3DTA_DIFFUSE` | `/* D3DTSS_COLORARG2             */ D3DTA_DIFFUSE     ,/*UNUSED*/`
- 495: enum `D3DTSS_COLORARG2` | `/* D3DTSS_COLORARG2             */ D3DTA_DIFFUSE     ,/*UNUSED*/`
- 496: enum `D3DTOP_MODULATE` | `/* D3DTSS_ALPHAOP               */ D3DTOP_MODULATE   ,`
- 496: enum `D3DTSS_ALPHAOP` | `/* D3DTSS_ALPHAOP               */ D3DTOP_MODULATE   ,`
- 497: enum `D3DTA_TEXTURE` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,`
- 497: enum `D3DTSS_ALPHAARG1` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,`
- 498: enum `D3DTA_DIFFUSE` | `/* D3DTSS_ALPHAARG2             */ D3DTA_DIFFUSE     ,`
- 498: enum `D3DTSS_ALPHAARG2` | `/* D3DTSS_ALPHAARG2             */ D3DTA_DIFFUSE     ,`
- 505: enum `D3DTOP_SELECTARG1` | `/* D3DTSS_COLOROP               */ D3DTOP_SELECTARG1 ,`
- 505: enum `D3DTSS_COLOROP` | `/* D3DTSS_COLOROP               */ D3DTOP_SELECTARG1 ,`
- 506: enum `D3DTA_TEXTURE` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,`
- 506: enum `D3DTSS_COLORARG1` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,`
- 507: enum `D3DTA_CURRENT` | `/* D3DTSS_COLORARG2             */ D3DTA_CURRENT     ,/*UNUSED*/`
- 507: enum `D3DTSS_COLORARG2` | `/* D3DTSS_COLORARG2             */ D3DTA_CURRENT     ,/*UNUSED*/`
- 508: enum `D3DTOP_MODULATE` | `/* D3DTSS_ALPHAOP               */ D3DTOP_MODULATE   ,`
- 508: enum `D3DTSS_ALPHAOP` | `/* D3DTSS_ALPHAOP               */ D3DTOP_MODULATE   ,`
- 509: enum `D3DTA_TEXTURE` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,`
- 509: enum `D3DTSS_ALPHAARG1` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,`
- 510: enum `D3DTA_CURRENT` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,`
- 510: enum `D3DTSS_ALPHAARG2` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,`
- 517: enum `D3DTOP_SELECTARG2` | `/* D3DTSS_COLOROP               */ D3DTOP_SELECTARG2 ,`
- 517: enum `D3DTSS_COLOROP` | `/* D3DTSS_COLOROP               */ D3DTOP_SELECTARG2 ,`
- 518: enum `D3DTA_TEXTURE` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,/*UNUSED*/`
- 518: enum `D3DTSS_COLORARG1` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,/*UNUSED*/`
- 519: enum `D3DTA_DIFFUSE` | `/* D3DTSS_COLORARG2             */ D3DTA_DIFFUSE     ,`
- 519: enum `D3DTSS_COLORARG2` | `/* D3DTSS_COLORARG2             */ D3DTA_DIFFUSE     ,`
- 520: enum `D3DTOP_MODULATE` | `/* D3DTSS_ALPHAOP               */ D3DTOP_MODULATE   ,`
- 520: enum `D3DTSS_ALPHAOP` | `/* D3DTSS_ALPHAOP               */ D3DTOP_MODULATE   ,`
- 521: enum `D3DTA_TEXTURE` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,`
- 521: enum `D3DTSS_ALPHAARG1` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,`
- 522: enum `D3DTA_DIFFUSE` | `/* D3DTSS_ALPHAARG2             */ D3DTA_DIFFUSE     ,`
- 522: enum `D3DTSS_ALPHAARG2` | `/* D3DTSS_ALPHAARG2             */ D3DTA_DIFFUSE     ,`
- 529: enum `D3DTOP_SELECTARG2` | `/* D3DTSS_COLOROP               */ D3DTOP_SELECTARG2 ,`
- 529: enum `D3DTSS_COLOROP` | `/* D3DTSS_COLOROP               */ D3DTOP_SELECTARG2 ,`
- 530: enum `D3DTA_TEXTURE` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,/*UNUSED*/`
- 530: enum `D3DTSS_COLORARG1` | `/* D3DTSS_COLORARG1             */ D3DTA_TEXTURE     ,/*UNUSED*/`
- 531: enum `D3DTA_CURRENT` | `/* D3DTSS_COLORARG2             */ D3DTA_CURRENT     ,`
- 531: enum `D3DTSS_COLORARG2` | `/* D3DTSS_COLORARG2             */ D3DTA_CURRENT     ,`
- 532: enum `D3DTOP_MODULATE` | `/* D3DTSS_ALPHAOP               */ D3DTOP_MODULATE   ,`
- 532: enum `D3DTSS_ALPHAOP` | `/* D3DTSS_ALPHAOP               */ D3DTOP_MODULATE   ,`
- 533: enum `D3DTA_TEXTURE` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,`
- 533: enum `D3DTSS_ALPHAARG1` | `/* D3DTSS_ALPHAARG1             */ D3DTA_TEXTURE     ,`
- 534: enum `D3DTA_CURRENT` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,`
- 534: enum `D3DTSS_ALPHAARG2` | `/* D3DTSS_ALPHAARG2             */ D3DTA_CURRENT     ,`
- 549: enum `D3DSAMP_MAGFILTER` | `/* D3DSAMP_MAGFILTER            */ MILTEXF_UNKNOWN  ,`
- 550: enum `D3DSAMP_MINFILTER` | `/* D3DSAMP_MINFILTER            */ MILTEXF_UNKNOWN  ,`
- 551: enum `D3DSAMP_MIPFILTER` | `/* D3DSAMP_MIPFILTER            */ MILTEXF_UNKNOWN  ,`
- 556: enum `D3DSAMP_MAGFILTER` | `/* D3DSAMP_MAGFILTER            */ D3DTEXF_POINT    ,`
- 556: enum `D3DTEXF_POINT` | `/* D3DSAMP_MAGFILTER            */ D3DTEXF_POINT    ,`
- 557: enum `D3DSAMP_MINFILTER` | `/* D3DSAMP_MINFILTER            */ D3DTEXF_POINT    ,`
- 557: enum `D3DTEXF_POINT` | `/* D3DSAMP_MINFILTER            */ D3DTEXF_POINT    ,`
- 558: enum `D3DSAMP_MIPFILTER` | `/* D3DSAMP_MIPFILTER            */ D3DTEXF_NONE     ,`
- 558: enum `D3DTEXF_NONE` | `/* D3DSAMP_MIPFILTER            */ D3DTEXF_NONE     ,`
- 563: enum `D3DSAMP_MAGFILTER` | `/* D3DSAMP_MAGFILTER            */ D3DTEXF_LINEAR   ,`
- 563: enum `D3DTEXF_LINEAR` | `/* D3DSAMP_MAGFILTER            */ D3DTEXF_LINEAR   ,`
- 564: enum `D3DSAMP_MINFILTER` | `/* D3DSAMP_MINFILTER            */ D3DTEXF_LINEAR   ,`
- 564: enum `D3DTEXF_LINEAR` | `/* D3DSAMP_MINFILTER            */ D3DTEXF_LINEAR   ,`
- 565: enum `D3DSAMP_MIPFILTER` | `/* D3DSAMP_MIPFILTER            */ D3DTEXF_NONE     ,`
- 565: enum `D3DTEXF_NONE` | `/* D3DSAMP_MIPFILTER            */ D3DTEXF_NONE     ,`
- 570: enum `D3DSAMP_MAGFILTER` | `/* D3DSAMP_MAGFILTER            */ D3DTEXF_LINEAR   ,`
- 570: enum `D3DTEXF_LINEAR` | `/* D3DSAMP_MAGFILTER            */ D3DTEXF_LINEAR   ,`
- 571: enum `D3DSAMP_MINFILTER` | `/* D3DSAMP_MINFILTER            */ D3DTEXF_LINEAR   ,`
- 571: enum `D3DTEXF_LINEAR` | `/* D3DSAMP_MINFILTER            */ D3DTEXF_LINEAR   ,`
- 572: enum `D3DSAMP_MIPFILTER` | `/* D3DSAMP_MIPFILTER            */ D3DTEXF_LINEAR   ,`
- 572: enum `D3DTEXF_LINEAR` | `/* D3DSAMP_MIPFILTER            */ D3DTEXF_LINEAR   ,`
- 577: enum `D3DSAMP_MAGFILTER` | `/* D3DSAMP_MAGFILTER            */ D3DTEXF_ANISOTROPIC  ,`
- 577: enum `D3DTEXF_ANISOTROPIC` | `/* D3DSAMP_MAGFILTER            */ D3DTEXF_ANISOTROPIC  ,`
- 578: enum `D3DSAMP_MINFILTER` | `/* D3DSAMP_MINFILTER            */ D3DTEXF_ANISOTROPIC  ,`
- 578: enum `D3DTEXF_ANISOTROPIC` | `/* D3DSAMP_MINFILTER            */ D3DTEXF_ANISOTROPIC  ,`
- 579: enum `D3DSAMP_MIPFILTER` | `/* D3DSAMP_MIPFILTER            */ D3DTEXF_LINEAR       ,`
- 579: enum `D3DTEXF_LINEAR` | `/* D3DSAMP_MIPFILTER            */ D3DTEXF_LINEAR       ,`
- 584: enum `D3DSAMP_MAGFILTER` | `/* D3DSAMP_MAGFILTER            */ D3DTEXF_LINEAR,`
- 584: enum `D3DTEXF_LINEAR` | `/* D3DSAMP_MAGFILTER            */ D3DTEXF_LINEAR,`
- 585: enum `D3DSAMP_MINFILTER` | `/* D3DSAMP_MINFILTER            */ D3DTEXF_ANISOTROPIC  ,`
- 585: enum `D3DTEXF_ANISOTROPIC` | `/* D3DSAMP_MINFILTER            */ D3DTEXF_ANISOTROPIC  ,`
- 586: enum `D3DSAMP_MIPFILTER` | `/* D3DSAMP_MIPFILTER            */ D3DTEXF_LINEAR       ,`
- 586: enum `D3DTEXF_LINEAR` | `/* D3DSAMP_MIPFILTER            */ D3DTEXF_LINEAR       ,`
- 591: enum `D3DSAMP_MAGFILTER` | `/* D3DSAMP_MAGFILTER            */ D3DTEXF_CONVOLUTIONMONO   ,`
- 591: enum `D3DTEXF_CONVOLUTIONMONO` | `/* D3DSAMP_MAGFILTER            */ D3DTEXF_CONVOLUTIONMONO   ,`
- 592: enum `D3DSAMP_MINFILTER` | `/* D3DSAMP_MINFILTER            */ D3DTEXF_CONVOLUTIONMONO   ,`
- 592: enum `D3DTEXF_CONVOLUTIONMONO` | `/* D3DSAMP_MINFILTER            */ D3DTEXF_CONVOLUTIONMONO   ,`
- 593: enum `D3DSAMP_MIPFILTER` | `/* D3DSAMP_MIPFILTER            */ D3DTEXF_NONE     ,`
- 593: enum `D3DTEXF_NONE` | `/* D3DSAMP_MIPFILTER            */ D3DTEXF_NONE     ,`
- 653: call `GetRenderState` | `IFC(m_pStateManager->GetRenderState(D3DRS_FILLMODE, &dwFillMode));`
- 653: enum `D3DRS_FILLMODE` | `IFC(m_pStateManager->GetRenderState(D3DRS_FILLMODE, &dwFillMode));`
- 678: call `GetRenderState` | `IFC(m_pStateManager->GetRenderState(D3DRS_ZFUNC, &dwDepthTestFunction));`
- 678: enum `D3DRS_ZFUNC` | `IFC(m_pStateManager->GetRenderState(D3DRS_ZFUNC, &dwDepthTestFunction));`
- 754: enum `D3DPS_VERSION` | `m_fCanDrawText = pDevice->GetPixelShaderVersion() >= D3DPS_VERSION(1,1)`
- 773: enum `D3DPS_VERSION` | `m_fDrawTextUsingPS20 = pDevice->GetPixelShaderVersion() >= D3DPS_VERSION(2,0);`
- 945: call `GetSamplerState` | `hr = m_pD3DDevice->GetSamplerState(dwSampler, pfmCurrent->Type(i), &dwValueCurrent);`
- 1030: call `GetRenderState` | `hr = m_pD3DDevice->GetRenderState(pabmCurrent->Type(i), &dwValueCurrent);`
- 1075: enum `D3DTOP_DISABLE` | `Assert(ptsoNew->m_opColor != D3DTOP_DISABLE);`
- 1079: enum `D3DTSS_COLOROP` | `IFC(m_pStateManager->SetTextureStageStateInline(dwStage, D3DTSS_COLOROP, ptsoNew->m_opColor) );`
- 1164: enum `D3DTOP_DISABLE` | `dwValueExpected = D3DTOP_DISABLE;`
- 1167: call `GetTextureStageState` | `hr = m_pD3DDevice->GetTextureStageState(dwStage, D3DTSS_COLOROP, &dwValueCurrent);`
- 1167: enum `D3DTSS_COLOROP` | `hr = m_pD3DDevice->GetTextureStageState(dwStage, D3DTSS_COLOROP, &dwValueCurrent);`
- 1175: enum `D3DTOP_DISABLE` | `Assert(dwValueCurrent == D3DTOP_DISABLE);`
- 1189: call `GetTextureStageState` | `hr = m_pD3DDevice->GetTextureStageState(dwStage, ptsoCurrent->Type(i), &dwValueCurrent);`
- 1261: call `SetPixelShaderConstantF` | `return m_pStateManager->SetPixelShaderConstantF(1, cregs[0], 5);`
- 1317: call `SetPixelShaderConstantF` | `return m_pStateManager->SetPixelShaderConstantF(2, cregs[0], 4);`
- 1348: call `SetPixelShaderConstantF` | `return m_pStateManager->SetPixelShaderConstantF(1, creg, 1);`
- 1395: call `SetPixelShaderConstantF` | `return m_pStateManager->SetPixelShaderConstantF(1, cregs[0], 5);`
- 1533: call `SetTextureStageState` | `IFC(m_pStateManager->SetTextureStageState(`
- 1535: enum `D3DTSS_TEXTURETRANSFORMFLAGS` | `D3DTSS_TEXTURETRANSFORMFLAGS,`
- 1536: enum `D3DTTFF_DISABLE` | `D3DTTFF_DISABLE`
- 1551: call `SetTextureStageState` | `IFC(m_pStateManager->SetTextureStageState(`
- 1553: enum `D3DTSS_TEXTURETRANSFORMFLAGS` | `D3DTSS_TEXTURETRANSFORMFLAGS,`
- 1554: enum `D3DTTFF_DISABLE` | `D3DTTFF_DISABLE`
- 1560: enum `D3DTOP_DISABLE` | `IFC(m_pStateManager->SetTextureStageStateInline(cMasks+1, D3DTSS_COLOROP, D3DTOP_DISABLE));`
- 1560: enum `D3DTSS_COLOROP` | `IFC(m_pStateManager->SetTextureStageStateInline(cMasks+1, D3DTSS_COLOROP, D3DTOP_DISABLE));`
- 1586: enum `D3DFMT_A8` | `m_alphaTextureFormat = D3DFMT_A8;`
- 1593: enum `D3DFMT_L8` | `m_alphaTextureFormat = D3DFMT_L8;`
- 1600: enum `D3DFMT_P8` | `m_alphaTextureFormat = D3DFMT_P8;`
- 1633: enum `D3DFMT_L8` | `if (m_alphaTextureFormat == D3DFMT_L8)`
- 1650: enum `D3DFMT_L8` | `if (m_alphaTextureFormat == D3DFMT_L8)`
- 1699: enum `D3DRS_DIFFUSEMATERIALSOURCE` | `D3DRS_DIFFUSEMATERIALSOURCE,`
- 1704: enum `D3DRS_SPECULARMATERIALSOURCE` | `D3DRS_SPECULARMATERIALSOURCE,`
- 1741: enum `D3DRS_BLENDFACTOR` | `D3DRS_BLENDFACTOR,`
- 1757: enum `D3DRS_BLENDFACTOR` | `D3DRS_BLENDFACTOR,`
- 1790: enum `D3DRS_DIFFUSEMATERIALSOURCE` | `D3DRS_DIFFUSEMATERIALSOURCE,`
- 1795: enum `D3DRS_SPECULARMATERIALSOURCE` | `D3DRS_SPECULARMATERIALSOURCE,`
- 1832: enum `D3DRS_DIFFUSEMATERIALSOURCE` | `D3DRS_DIFFUSEMATERIALSOURCE,`
- 1837: enum `D3DRS_SPECULARMATERIALSOURCE` | `D3DRS_SPECULARMATERIALSOURCE,`
- 1873: enum `D3DRS_BLENDFACTOR` | `D3DRS_BLENDFACTOR,`
- 1888: enum `D3DRS_BLENDFACTOR` | `D3DRS_BLENDFACTOR,`
- 1917: enum `D3DRS_DIFFUSEMATERIALSOURCE` | `D3DRS_DIFFUSEMATERIALSOURCE,`
- 1922: enum `D3DRS_SPECULARMATERIALSOURCE` | `D3DRS_SPECULARMATERIALSOURCE,`
- 1932: call `SetPixelShaderConstantF` | `IFC(m_pStateManager->SetPixelShaderConstantF(0, constantReg0, 1));`
- 1963: enum `D3DRS_COLORWRITEENABLE` | `IFC(SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED));`
- 1966: call `SetPixelShaderConstantF` | `IFC(m_pStateManager->SetPixelShaderConstantF(0, constantReg0, 1));`
- 1989: enum `D3DRS_COLORWRITEENABLE` | `IFC(SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_GREEN));`
- 1992: call `SetPixelShaderConstantF` | `IFC(m_pStateManager->SetPixelShaderConstantF(0, constantReg0, 1));`
- 2015: enum `D3DRS_COLORWRITEENABLE` | `IFC(SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_BLUE) );`
- 2018: call `SetPixelShaderConstantF` | `IFC(m_pStateManager->SetPixelShaderConstantF(0, constantReg0, 1));`
- 2039: enum `D3DRS_COLORWRITEENABLE` | `IFC(SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL));`

## core/hw/d3drenderstate.h

- 117: call `SetVertexShaderConstantF` | `return m_pStateManager->SetVertexShaderConstantF(`
- 130: call `SetPixelShaderConstantF` | `return m_pStateManager->SetPixelShaderConstantF(`
- 142: call `SetPixelShaderConstantI` | `return m_pStateManager->SetPixelShaderConstantI(`
- 153: call `SetPixelShaderConstantB` | `return m_pStateManager->SetPixelShaderConstantB(`
- 236: call `SetConvolutionMonoKernel` | `return m_pStateManager->SetConvolutionMonoKernel(`
- 247: call `SetTransform` | `return m_pStateManager->SetTransform(`
- 278: call `GetTransform` | `return m_pStateManager->GetTransform(`
- 317: enum `D3DTSS_TEXTURETRANSFORMFLAGS` | `D3DTSS_TEXTURETRANSFORMFLAGS,`
- 318: enum `D3DTTFF_DISABLE` | `D3DTTFF_DISABLE`
- 331: enum `D3DSAMP_ADDRESSU` | `Assert(dwValue \|\| state != D3DSAMP_ADDRESSU);`
- 336: enum `D3DSAMP_ADDRESSV` | `Assert(dwValue \|\| state != D3DSAMP_ADDRESSV);`
- 388: call `SetStreamSource` | `return m_pStateManager->SetStreamSource(`
- 399: call `SetIndices` | `return m_pStateManager->SetIndices(`
- 458: call `SetViewport` | `return m_pStateManager->SetViewport(prcViewport);`
- 463: call `GetViewport` | `return m_pStateManager->GetViewport();`
- 487: call `SetScissorRect` | `return m_pStateManager->SetScissorRect(prc);`

## core/hw/d3dresource.cpp

- 1267: enum `D3DERR_OUTOFVIDEOMEMORY` | `if (hD3DResult == D3DERR_OUTOFVIDEOMEMORY \|\|`

## core/hw/d3dstats.cpp

- 73: enum `D3DQUERYTYPE_DDISTATS` | `D3DQUERYTYPE_DDISTATS,`
- 94: enum `D3DQUERYTYPE_RESOURCEMANAGER` | `D3DQUERYTYPE_RESOURCEMANAGER,`
- 114: enum `D3DQUERYTYPE_VERTEXSTATS` | `D3DQUERYTYPE_VERTEXSTATS,`
- 172: call `CreateQuery` | `IFC(pD3DDevice->CreateQuery(d3dQueryType, &pD3DQuery));`
- 173: call `Issue` | `IFC(pD3DQuery->Issue(D3DISSUE_END));`
- 173: enum `D3DISSUE_END` | `IFC(pD3DQuery->Issue(D3DISSUE_END));`
- 174: call `GetData` | `IFC(pD3DQuery->GetData(pData, dwDataSize, 0));`
- 247: enum `D3DRTYPE_TEXTURE` | `const D3DRESOURCESTATS resourceStats = resourceManagerStats.stats[D3DRTYPE_TEXTURE];`
- 248: enum `D3DRTYPE_TEXTURE` | `const D3DRESOURCESTATS resourceStatsPrevious = m_resourceManagerStatsPrevious.stats[D3DRTYPE_TEXTURE];`

## core/hw/d3dstats.h

- 36: enum `D3DQUERYTYPE_DDISTATS` | `#define D3DQUERYTYPE_DDISTATS ( D3DQUERYTYPE( 7 ) )`

## core/hw/d3dsurface.cpp

- 110: call `GetDesc` | `IFC(m_pD3DSurface->GetDesc(&m_d3dsd));`
- 112: enum `D3DPOOL_SYSTEMMEM` | `if (m_d3dsd.Pool == D3DPOOL_SYSTEMMEM)`
- 120: enum `D3DMULTISAMPLE_2_SAMPLES` | `if (m_d3dsd.MultiSampleType >= D3DMULTISAMPLE_2_SAMPLES)`
- 122: enum `D3DMULTISAMPLE_2_SAMPLES` | `C_ASSERT(D3DMULTISAMPLE_2_SAMPLES == 2);`
- 159: enum `D3DUSAGE_RENDERTARGET` | `if (m_d3dsd.Usage & D3DUSAGE_RENDERTARGET)`
- 164: enum `D3DUSAGE_DEPTHSTENCIL` | `if (m_d3dsd.Usage & D3DUSAGE_DEPTHSTENCIL)`
- 212: call `GetDC` | `MIL_THR(m_pD3DSurface->GetDC(`
- 230: enum `D3DERR_DRIVERINTERNALERROR` | `D3DERR_DRIVERINTERNALERROR`
- 250: call `ReleaseDC` | `IGNORE_HR(m_pD3DSurface->ReleaseDC(hBackBuffer));`
- 320: enum `D3DPOOL_MANAGED` | `if ((m_d3dsd.Pool == D3DPOOL_MANAGED) \|\| (m_d3dsd.Pool == D3DPOOL_SYSTEMMEM))`
- 320: enum `D3DPOOL_SYSTEMMEM` | `if ((m_d3dsd.Pool == D3DPOOL_MANAGED) \|\| (m_d3dsd.Pool == D3DPOOL_SYSTEMMEM))`
- 339: call `GetDevice` | `if (SUCCEEDED(ID3DSurface()->GetDevice(&pID3DDevice)))`
- 408: enum `D3DTEXF_NONE` | `D3DTEXF_NONE`
- 426: call `LockRect` | `IFC(pD3DLockableSurface->LockRect(&d3dlr, &rcDest, D3DLOCK_READONLY));`
- 426: enum `D3DLOCK_READONLY` | `IFC(pD3DLockableSurface->LockRect(&d3dlr, &rcDest, D3DLOCK_READONLY));`
- 475: call `UnlockRect` | `IGNORE_HR(pD3DLockableSurface->UnlockRect());`

## core/hw/d3dsurface.h

- 42: call `LockRect` | `return ID3DSurface()->LockRect(pLockedRect, pRect, Flags);`
- 48: call `UnlockRect` | `return ID3DSurface()->UnlockRect();`
- 76: call `ReleaseDC` | `return ID3DSurface()->ReleaseDC(hdc);`

## core/hw/d3dswapchain.cpp

- 61: enum `D3DPRESENT_PARAMETERS` | `D3DPRESENT_PARAMETERS d3dpp;`
- 63: call `GetPresentParameters` | `IFC(pID3DSwapChain->GetPresentParameters(&d3dpp));`
- 245: call `GetBackBuffer` | `IFC(m_pD3DSwapChain->GetBackBuffer(i,  D3DBACKBUFFER_TYPE_MONO, &pD3DBackBuffer));`
- 245: enum `D3DBACKBUFFER_TYPE_MONO` | `IFC(m_pD3DSwapChain->GetBackBuffer(i,  D3DBACKBUFFER_TYPE_MONO, &pD3DBackBuffer));`
- 346: call `GetDC` | `IFC(m_rgBackBuffers[iBackBuffer]->GetDC(`
- 370: call `ReleaseDC` | `RRETURN(m_rgBackBuffers[iBackBuffer]->ReleaseDC(`

## core/hw/d3dswapchainwithswdc.cpp

- 95: enum `D3DFMT_A8R8G8B8` | `Assert(   surfDesc.Format == D3DFMT_A8R8G8B8`
- 96: enum `D3DFMT_X8R8G8B8` | `\|\| surfDesc.Format == D3DFMT_X8R8G8B8`

## core/hw/d3dtexture.cpp

- 76: call `GetLevelCount` | `m_cLevels = pD3DTexture->GetLevelCount();`
- 83: call `GetLevelDesc` | `IFC(pD3DTexture->GetLevelDesc(0, &m_sdLevel0));`
- 120: call `GetLevelDesc` | `IFC(pD3DTexture->GetLevelDesc(uLevel, &sd));`
- 126: enum `D3DERR_WRONGTEXTUREFORMAT` | `IFC(D3DERR_WRONGTEXTUREFORMAT);`
- 221: call `GetSurfaceLevel` | `IFC(m_pD3DTexture->GetSurfaceLevel(`
- 267: call `GenerateMipSubLevels` | `GetD3DTextureNoRef()->GenerateMipSubLevels();`
- 276: enum `D3DTEXF_LINEAR` | `IFC(Device().StretchRect(pSurfaceSrc, NULL, pSurfaceDst, NULL, D3DTEXF_LINEAR));`
- 329: enum `D3DUSAGE_AUTOGENMIPMAP` | `*pdwUsage = D3DUSAGE_AUTOGENMIPMAP;`
- 345: enum `D3DUSAGE_RENDERTARGET` | `*pdwUsage = D3DUSAGE_RENDERTARGET;`

## core/hw/d3dtexture.h

- 68: call `GetSurfaceLevel` | `RRETURN(THR(pD3DTextureNoRef->GetSurfaceLevel(`

## core/hw/d3dvertex.h

- 123: enum `D3DFVF_DIFFUSE` | `enum {Format = D3DFVF_XYZ \| D3DFVF_DIFFUSE \| D3DFVF_TEX2};`
- 123: enum `D3DFVF_TEX2` | `enum {Format = D3DFVF_XYZ \| D3DFVF_DIFFUSE \| D3DFVF_TEX2};`
- 123: enum `D3DFVF_XYZ` | `enum {Format = D3DFVF_XYZ \| D3DFVF_DIFFUSE \| D3DFVF_TEX2};`
- 196: enum `D3DFVF_DIFFUSE` | `enum {Format = D3DFVF_XYZ \| D3DFVF_NORMAL \| D3DFVF_DIFFUSE \| D3DFVF_SPECULAR \| D3DFVF_TEX4};`
- 196: enum `D3DFVF_NORMAL` | `enum {Format = D3DFVF_XYZ \| D3DFVF_NORMAL \| D3DFVF_DIFFUSE \| D3DFVF_SPECULAR \| D3DFVF_TEX4};`
- 196: enum `D3DFVF_SPECULAR` | `enum {Format = D3DFVF_XYZ \| D3DFVF_NORMAL \| D3DFVF_DIFFUSE \| D3DFVF_SPECULAR \| D3DFVF_TEX4};`
- 196: enum `D3DFVF_TEX4` | `enum {Format = D3DFVF_XYZ \| D3DFVF_NORMAL \| D3DFVF_DIFFUSE \| D3DFVF_SPECULAR \| D3DFVF_TEX4};`
- 196: enum `D3DFVF_XYZ` | `enum {Format = D3DFVF_XYZ \| D3DFVF_NORMAL \| D3DFVF_DIFFUSE \| D3DFVF_SPECULAR \| D3DFVF_TEX4};`
- 242: enum `D3DFVF_DIFFUSE` | `enum {Format = D3DFVF_XYZ \| D3DFVF_DIFFUSE \| D3DFVF_TEX6};`
- 242: enum `D3DFVF_TEX6` | `enum {Format = D3DFVF_XYZ \| D3DFVF_DIFFUSE \| D3DFVF_TEX6};`
- 242: enum `D3DFVF_XYZ` | `enum {Format = D3DFVF_XYZ \| D3DFVF_DIFFUSE \| D3DFVF_TEX6};`
- 284: enum `D3DFVF_DIFFUSE` | `enum {Format = D3DFVF_XYZ \| D3DFVF_DIFFUSE \| D3DFVF_TEX8};`
- 284: enum `D3DFVF_TEX8` | `enum {Format = D3DFVF_XYZ \| D3DFVF_DIFFUSE \| D3DFVF_TEX8};`
- 284: enum `D3DFVF_XYZ` | `enum {Format = D3DFVF_XYZ \| D3DFVF_DIFFUSE \| D3DFVF_TEX8};`

## core/hw/d3dvidmemonlytexture.cpp

- 43: enum `D3DPOOL_DEFAULT` | `Assert(pSurfDesc->Pool == D3DPOOL_DEFAULT);`
- 49: call `CreateTexture` | `IFC(pDevice->CreateTexture(`
- 109: enum `D3DPOOL_DEFAULT` | `Assert(pVidMemOnlyTexture->m_sdLevel0.Pool == D3DPOOL_DEFAULT);`

## core/hw/gpumarker.cpp

- 95: call `Issue` | `IFC(m_pQuery->Issue(D3DISSUE_END));`
- 95: enum `D3DISSUE_END` | `IFC(m_pQuery->Issue(D3DISSUE_END));`
- 128: call `GetData` | `MIL_THR(m_pQuery->GetData(NULL, 0, dwFlags));`

## core/hw/hw3dgeometrystreambuffer.cpp

- 42: enum `D3DFVF_NORMAL` | `D3DFVF = D3DFVF_NORMAL,`
- 52: enum `D3DFVF_DIFFUSE` | `D3DFVF = D3DFVF_DIFFUSE,`
- 123: enum `D3DLOCK_NOOVERWRITE` | `*pdwLockFlags = D3DLOCK_NOOVERWRITE;`
- 127: enum `D3DLOCK_DISCARD` | `*pdwLockFlags = D3DLOCK_DISCARD;`
- 270: call `CreateVertexBuffer` | `IFC(pD3DDevice->CreateVertexBuffer(`
- 272: enum `D3DUSAGE_DYNAMIC` | `D3DUSAGE_WRITEONLY \| D3DUSAGE_DYNAMIC,`
- 272: enum `D3DUSAGE_WRITEONLY` | `D3DUSAGE_WRITEONLY \| D3DUSAGE_DYNAMIC,`
- 274: enum `D3DPOOL_DEFAULT` | `D3DPOOL_DEFAULT,`
- 376: enum `D3DERR_DRIVERINTERNALERROR` | `IFC(D3DERR_DRIVERINTERNALERROR);`
- 505: call `CreateIndexBuffer` | `IFC(pD3DDevice->CreateIndexBuffer(`
- 507: enum `D3DUSAGE_DYNAMIC` | `D3DUSAGE_WRITEONLY \| D3DUSAGE_DYNAMIC,`
- 507: enum `D3DUSAGE_WRITEONLY` | `D3DUSAGE_WRITEONLY \| D3DUSAGE_DYNAMIC,`
- 508: enum `D3DFMT_INDEX16` | `D3DFMT_INDEX16,`
- 509: enum `D3DPOOL_DEFAULT` | `D3DPOOL_DEFAULT,`
- 610: enum `D3DERR_DRIVERINTERNALERROR` | `IFC(D3DERR_DRIVERINTERNALERROR);`
- 1182: enum `D3DFVF_TEX1` | `DWORD dwFVF = D3DFVF_XYZ \| VertexDataTypeTraits<TDiffuseOrNormal>::D3DFVF \| D3DFVF_TEX1;`
- 1182: enum `D3DFVF_XYZ` | `DWORD dwFVF = D3DFVF_XYZ \| VertexDataTypeTraits<TDiffuseOrNormal>::D3DFVF \| D3DFVF_TEX1;`
- 1184: call `SetFVF` | `IFC(pDevice->SetFVF(dwFVF));`
- 1193: call `SetStreamSource` | `IFC(pDevice->SetStreamSource(`
- 1200: call `SetIndices` | `IFC(pDevice->SetIndices(`
- 1273: call `GetIndices` | `pMesh3D->GetIndices(puIndices, dontCare);`

## core/hw/hwbitmapcolorsource.cpp

- 1311: enum `D3DPTEXTURECAPS_POW2` | `else if (pDevice->SupportsTextureCap(D3DPTEXTURECAPS_POW2))`
- 1328: enum `D3DTADDRESS_CLAMP` | `oRealizationParams.dlU.d3dta = D3DTADDRESS_CLAMP;`
- 1373: enum `D3DTADDRESS_CLAMP` | `oRealizationParams.dlV.d3dta = D3DTADDRESS_CLAMP;`
- 1463: enum `D3DRTYPE_TEXTURE` | `pd3dsdRequired->Type = D3DRTYPE_TEXTURE;`
- 1474: enum `D3DPOOL_DEFAULT` | `pd3dsdRequired->Pool = D3DPOOL_DEFAULT;`
- 1475: enum `D3DMULTISAMPLE_NONE` | `pd3dsdRequired->MultiSampleType = D3DMULTISAMPLE_NONE;`
- 1560: enum `D3DTADDRESS_WRAP` | `case D3DTADDRESS_WRAP:`
- 1565: enum `D3DTADDRESS_CLAMP` | `dl.d3dta = D3DTADDRESS_CLAMP;`
- 1573: enum `D3DTADDRESS_MIRROR` | `case D3DTADDRESS_MIRROR:`
- 1578: enum `D3DTADDRESS_CLAMP` | `dl.d3dta = D3DTADDRESS_CLAMP;`
- 1586: enum `D3DTADDRESS_CLAMP` | `case D3DTADDRESS_CLAMP:`
- 1591: enum `D3DTADDRESS_BORDER` | `case D3DTADDRESS_BORDER:`
- 1703: enum `D3DUSAGE_RENDERTARGET` | `d3dsd.Usage \|= D3DUSAGE_RENDERTARGET;`
- 2600: enum `D3DFMT_UNKNOWN` | `Assert(m_d3dsdRequired.Format != D3DFMT_UNKNOWN);`
- 2601: enum `D3DRTYPE_TEXTURE` | `Assert(m_d3dsdRequired.Type == D3DRTYPE_TEXTURE);`
- 2603: enum `D3DUSAGE_AUTOGENMIPMAP` | `Assert((m_d3dsdRequired.Usage & ~(D3DUSAGE_AUTOGENMIPMAP \| D3DUSAGE_RENDERTARGET)) == 0);`
- 2603: enum `D3DUSAGE_RENDERTARGET` | `Assert((m_d3dsdRequired.Usage & ~(D3DUSAGE_AUTOGENMIPMAP \| D3DUSAGE_RENDERTARGET)) == 0);`
- 2604: enum `D3DPOOL_DEFAULT` | `Assert(m_d3dsdRequired.Pool == D3DPOOL_DEFAULT);`
- 2605: enum `D3DMULTISAMPLE_NONE` | `Assert(m_d3dsdRequired.MultiSampleType == D3DMULTISAMPLE_NONE);`
- 3239: call `Lock` | `IFC(m_pBitmap->Lock(`
- 3600: call `StretchRect` | `IFC(m_pDevice->StretchRect(`
- 3605: enum `D3DTEXF_NONE` | `D3DTEXF_NONE`
- 3708: call `LockRect` | `IFC(pD3DSysMemSurface->LockRect(`
- 3711: enum `D3DLOCK_NO_DIRTY_UPDATE` | `D3DLOCK_NO_DIRTY_UPDATE`
- 3870: call `UnlockRect` | `IFC(pD3DSysMemSurface->UnlockRect());`
- 3937: call `UpdateSurface` | `IFC(m_pDevice->UpdateSurface(`
- 3968: call `UnlockRect` | `IGNORE_HR(pD3DSysMemSurface->UnlockRect());`
- 4197: call `SetTexture` | `IFC(m_pDevice->SetTexture(dwSampler, m_pVidMemOnlyTexture));`
- 4350: call `GetDesc` | `Verify(SUCCEEDED(pD3DSysMemSurface->GetDesc(&desc)));`
- 4354: enum `D3DPOOL_SYSTEMMEM` | `Assert(desc.Pool ==                 D3DPOOL_SYSTEMMEM);`
- 4375: call `GetSurfaceLevel` | `Verify(SUCCEEDED(pD3DSysMemTexture->GetSurfaceLevel(`

## core/hw/hwdestinationtexture.cpp

- 218: call `SetTexture` | `IFC(m_pDevice->SetTexture(dwSampler, m_pBackgroundTexture));`
- 457: enum `D3DPOOL_DEFAULT` | `D3DPOOL_DEFAULT,`
- 458: enum `D3DUSAGE_RENDERTARGET` | `D3DUSAGE_RENDERTARGET,`
- 504: enum `D3DTADDRESS_CLAMP` | `D3DTADDRESS_CLAMP,`
- 505: enum `D3DTADDRESS_CLAMP` | `D3DTADDRESS_CLAMP`
- 711: call `GetSurfaceLevel` | `IFC(pDestTextureNoRef->GetSurfaceLevel(`
- 720: call `ColorFill` | `IFC(pDevice->ColorFill(`

## core/hw/hwdevicebitmapcolorsource.cpp

- 134: enum `D3DTADDRESS_CLAMP` | `oRealizationDesc.dlU.d3dta   = D3DTADDRESS_CLAMP;`
- 138: enum `D3DTADDRESS_CLAMP` | `oRealizationDesc.dlV.d3dta   = D3DTADDRESS_CLAMP;`
- 165: enum `D3DUSAGE_RENDERTARGET` | `d3dsd.Usage \|= D3DUSAGE_RENDERTARGET;`
- 642: call `CreateRenderTarget` | `IFC(pID3DDevice->CreateRenderTarget(`
- 646: enum `D3DMULTISAMPLE_NONE` | `D3DMULTISAMPLE_NONE,`
- 653: call `StretchRect` | `IFC(pID3DDevice->StretchRect(`
- 658: enum `D3DTEXF_NONE` | `D3DTEXF_NONE`
- 662: call `LockRect` | `IFC(pIFlushSurface->LockRect(`
- 665: enum `D3DLOCK_READONLY` | `D3DLOCK_READONLY`
- 671: call `UnlockRect` | `IGNORE_HR(pIFlushSurface->UnlockRect());`
- 706: call `GetDevice` | `IFC(pISrcSurface->GetDevice(&pID3DSrcDevice));`
- 710: call `CreateTexture` | `IFC(pID3DSrcDevice->CreateTexture(`
- 721: call `GetSurfaceLevel` | `IFC(pIDestTexture->GetSurfaceLevel(0, &pIDestSurface));`
- 729: call `StretchRect` | `IFC(pID3DSrcDevice->StretchRect(`
- 734: enum `D3DTEXF_NONE` | `D3DTEXF_NONE`
- 780: call `GetDesc` | `IFC(pISrcSurface->GetDesc(&descSrc));`
- 784: enum `D3DPOOL_SYSTEMMEM` | `descSrc.Pool = D3DPOOL_SYSTEMMEM;`
- 785: enum `D3DMULTISAMPLE_NONE` | `descSrc.MultiSampleType = D3DMULTISAMPLE_NONE;`
- 793: call `LockRect` | `IFC(m_pSysMemTexture->LockRect(&rcLock, NULL, D3DLOCK_NO_DIRTY_UPDATE));`
- 793: enum `D3DLOCK_NO_DIRTY_UPDATE` | `IFC(m_pSysMemTexture->LockRect(&rcLock, NULL, D3DLOCK_NO_DIRTY_UPDATE));`
- 809: call `AddDirtyRect` | `IFC(m_pSysMemTexture->AddDirtyRect(reinterpret_cast<const RECT *>(prgDirtyRects)[i]));`
- 814: call `UnlockRect` | `IFC(m_pSysMemTexture->UnlockRect());`
- 819: call `UpdateTexture` | `IFC(m_pDevice->UpdateTexture(`
- 827: call `UnlockRect` | `IGNORE_HR(m_pSysMemTexture->UnlockRect());`

## core/hw/hwdisplayrt.cpp

- 58: enum `D3DPRESENT_PARAMETERS` | `D3DPRESENT_PARAMETERS D3DPresentParams;`
- 186: enum `D3DPRESENT_PARAMETERS` | `__in_ecount(1) D3DPRESENT_PARAMETERS const &D3DPresentParams,`
- 427: enum `D3DSWAPEFFECT_COPY` | `if (m_D3DPresentParams.SwapEffect == D3DSWAPEFFECT_COPY)`
- 449: enum `D3DSWAPEFFECT_DISCARD` | `if (m_D3DPresentParams.SwapEffect == D3DSWAPEFFECT_DISCARD)`
- 472: call `Clear` | `IGNORE_HR(m_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, fGreen ? green: purple, 0, 0));`
- 472: enum `D3DCLEAR_TARGET` | `IGNORE_HR(m_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, fGreen ? green: purple, 0, 0));`
- 547: enum `D3DSWAPEFFECT_COPY` | `if (m_D3DPresentParams.SwapEffect == D3DSWAPEFFECT_COPY)`
- 549: call `Present` | `IFC(m_pD3DDevice->Present(`
- 564: call `Present` | `IFC(m_pD3DDevice->Present(`
- 757: enum `D3DSWAPEFFECT_COPY` | `if (m_D3DPresentParams.SwapEffect != D3DSWAPEFFECT_COPY)`
- 791: call `StretchRect` | `hr = THR(m_pD3DDevice->StretchRect(`
- 797: enum `D3DTEXF_LINEAR` | `D3DTEXF_LINEAR : D3DTEXF_NONE)`
- 797: enum `D3DTEXF_NONE` | `D3DTEXF_LINEAR : D3DTEXF_NONE)`
- 856: enum `D3DPOOL_SYSTEMMEM` | `(d3dsd.Pool == D3DPOOL_SYSTEMMEM))`
- 870: call `GetDevice` | `MIL_THRX(hrLocking, pD3DSurface->ID3DSurface()->GetDevice(&pID3DDevice));`
- 877: call `CreateRenderTarget` | `MIL_THRX(hrLocking, pID3DDevice->CreateRenderTarget(`
- 890: call `StretchRect` | `MIL_THRX(hrLocking, pID3DDevice->StretchRect(`
- 895: enum `D3DTEXF_NONE` | `D3DTEXF_NONE`
- 911: call `LockRect` | `hrLocking = THR(pID3DLockableSurface->LockRect(`
- 914: enum `D3DLOCK_READONLY` | `D3DLOCK_READONLY));`
- 956: call `UnlockRect` | `IGNORE_HR(pID3DLockableSurface->UnlockRect());`
- 1025: enum `D3DFMT_A8R8G8B8` | `D3DFMT_A8R8G8B8,`
- 1300: call `WaitForVBlank` | `MIL_THR(m_pD3DDevice->WaitForVBlank(0));`

## core/hw/hwdisplayrt.h

- 51: enum `D3DPRESENT_PARAMETERS` | `__in_ecount(1) D3DPRESENT_PARAMETERS const &D3DPresentParams,`
- 191: enum `D3DPRESENT_PARAMETERS` | `D3DPRESENT_PARAMETERS m_D3DPresentParams;`

## core/hw/hwffshaders.cpp

- 248: call `SetRenderState` | `IFC(m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, 1));`
- 248: enum `D3DRS_ZWRITEENABLE` | `IFC(m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, 1));`
- 372: call `SetRenderState` | `IFC(m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, 0));`
- 372: enum `D3DRS_ZWRITEENABLE` | `IFC(m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, 0));`
- 488: call `SetRenderState` | `IFC(m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, 0));`
- 488: enum `D3DRS_ZWRITEENABLE` | `IFC(m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, 0));`

## core/hw/hwhwndrt.cpp

- 47: enum `D3DPRESENT_PARAMETERS` | `__in_ecount(1) D3DPRESENT_PARAMETERS const &D3DPresentParams,`
- 98: enum `D3DFMT_A2R10G10B10` | `if (m_D3DPresentParams.BackBufferFormat == D3DFMT_A2R10G10B10)`
- 102: call `GetAdapterDisplayMode` | `IFC(pDisplay->D3DObject()->GetAdapterDisplayMode(`
- 107: enum `D3DFMT_A2R10G10B10` | `if (d3ddm.Format != D3DFMT_A2R10G10B10)`
- 114: enum `D3DPRESENT_LINEAR_CONTENT` | `m_dwPresentFlags \|= D3DPRESENT_LINEAR_CONTENT;`
- 304: call `CreateAdditionalSwapChain` | `IFCSUB1(m_pD3DDevice->CreateAdditionalSwapChain(`

## core/hw/hwhwndrt.h

- 62: enum `D3DPRESENT_PARAMETERS` | `__in_ecount(1) D3DPRESENT_PARAMETERS const &D3DPresentParams,`

## core/hw/hwlineargradientcolorsource.cpp

- 197: enum `D3DTADDRESS_WRAP` | `taU = D3DTADDRESS_WRAP;`
- 201: enum `D3DTADDRESS_CLAMP` | `taU = D3DTADDRESS_CLAMP;`
- 224: enum `D3DTADDRESS_CLAMP` | `D3DTADDRESS_CLAMP`
- 261: enum `D3DFMT_A8R8G8B8` | `if (rectTexture.Pitch < static_cast<INT>(m_cRealizedTextureWidth * D3DFormatSize(D3DFMT_A8R8G8B8)))`
- 385: enum `D3DFMT_A8R8G8B8` | `D3DFMT_A8R8G8B8,`
- 461: call `SetTexture` | `IFC(m_pDevice->SetTexture(dwSampler, m_vidMemManager.GetVidMemTextureNoRef()));`

## core/hw/hwpipeline.cpp

- 99: call `DrawPrimitive` | `IFC(m_pVB->DrawPrimitive(m_pDevice));`
- 458: call `SetPixelShader` | `IFC(m_pDevice->SetPixelShader(NULL));`
- 459: call `SetVertexShader` | `IFC(m_pDevice->SetVertexShader(NULL));`
- 816: enum `D3DPS_VERSION` | `if (   dwPixelShaderVersion  < D3DPS_VERSION(2,0)`
- 817: enum `D3DVS_VERSION` | `\|\| dwVertexShaderVersion < D3DVS_VERSION(2,0)`

## core/hw/hwshader.cpp

- 384: call `DrawPrimitive` | `IFC(pVertexBuffer->DrawPrimitive(m_pDevice));`

## core/hw/hwsolidcolortexturesource.cpp

- 48: enum `D3DTADDRESS_CLAMP` | `D3DTADDRESS_CLAMP,`
- 49: enum `D3DTADDRESS_CLAMP` | `D3DTADDRESS_CLAMP`
- 146: call `SetTexture` | `IFC(m_pDevice->SetTexture(dwSampler, m_pLockableTexture));`
- 212: enum `D3DFMT_A8R8G8B8` | `d3dsd.Format = D3DFMT_A8R8G8B8;`
- 213: enum `D3DRTYPE_TEXTURE` | `d3dsd.Type = D3DRTYPE_TEXTURE;`
- 216: enum `D3DMULTISAMPLE_NONE` | `d3dsd.MultiSampleType = D3DMULTISAMPLE_NONE;`
- 273: call `LockRect` | `IFC(m_pLockableTexture->LockRect(`
- 284: call `UnlockRect` | `IFC(m_pLockableTexture->UnlockRect());`

## core/hw/hwsurfrt.cpp

- 193: call `Clear` | `IFC(m_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, color, 0, 0));`
- 193: enum `D3DCLEAR_TARGET` | `IFC(m_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, color, 0, 0));`
- 242: enum `D3DMULTISAMPLE_NONE` | `D3DMULTISAMPLE_TYPE multisampleTypeRequested = D3DMULTISAMPLE_NONE;`
- 242: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE multisampleTypeRequested = D3DMULTISAMPLE_NONE;`
- 257: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE multisampleTypeRecevied = multisampleTypeRequested;`
- 299: enum `D3DMULTISAMPLE_TYPE` | `__inout_ecount(1) D3DMULTISAMPLE_TYPE &MultisampleType`
- 346: call `Clear` | `IFC(m_pD3DDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, rZ, 0));`
- 346: enum `D3DCLEAR_ZBUFFER` | `IFC(m_pD3DDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, rZ, 0));`
- 364: enum `D3DMULTISAMPLE_TYPE` | `__inout_ecount(1) D3DMULTISAMPLE_TYPE &MultisampleType`
- 402: enum `D3DERR_OUTOFVIDEOMEMORY` | `if ((hr == D3DERR_OUTOFVIDEOMEMORY) && (MultisampleType != D3DMULTISAMPLE_NONE))`
- 402: enum `D3DMULTISAMPLE_NONE` | `if ((hr == D3DERR_OUTOFVIDEOMEMORY) && (MultisampleType != D3DMULTISAMPLE_NONE))`
- 404: enum `D3DMULTISAMPLE_NONE` | `MultisampleType = D3DMULTISAMPLE_NONE;`
- 425: call `StretchRect` | `IFC(m_pD3DDevice->StretchRect(`
- 430: enum `D3DTEXF_NONE` | `D3DTEXF_NONE`
- 470: call `StretchRect` | `IFC(m_pD3DDevice->StretchRect(`
- 475: enum `D3DTEXF_NONE` | `D3DTEXF_NONE`
- 790: enum `D3DFILL_WIREFRAME` | `D3DFILL_WIREFRAME,`
- 1390: call `Reset` | `pShapeWiden->Reset();`
- 1978: enum `D3DPTEXTURECAPS_POW2` | `&& (   (   m_pD3DDevice->SupportsTextureCap(D3DPTEXTURECAPS_POW2)`
- 2019: call `GetType` | `if (pBrushNoRef->GetType() == BrushBitmap)`
- 2359: call `ColorFill` | `IFC(m_pD3DDevice->ColorFill(`
- 2605: call `Reset` | `pScratchFill->Reset();`
- 2888: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE MultisampleType`
- 2904: enum `D3DMULTISAMPLE_NONE` | `if (   MultisampleType != D3DMULTISAMPLE_NONE`
- 2956: call `CreateRenderTarget` | `IFC(m_pD3DDevice->CreateRenderTarget(`
- 3033: call `SetDepthStencilSurface` | `IFC(m_pD3DDevice->SetDepthStencilSurface(m_pD3DStencilSurface));`
- 3038: call `SetDepthStencilSurface` | `IGNORE_HR(m_pD3DDevice->SetDepthStencilSurface(NULL));`
- 3099: call `SetRenderTarget` | `IFC(m_pD3DDevice->SetRenderTarget(`
- 3119: call `SetRenderTarget` | `IFC(m_pD3DDevice->SetRenderTarget(m_pD3DTargetSurfaceFor3DNoRef));`
- 3147: call `SetDepthStencilSurface` | `IFC(m_pD3DDevice->SetDepthStencilSurface(NULL));`
- 3160: call `SetRenderState` | `IFC(m_pD3DDevice->SetRenderState(`
- 3161: enum `D3DRS_CULLMODE` | `D3DRS_CULLMODE,`
- 3162: enum `D3DCULL_NONE` | `D3DCULL_NONE`
- 3165: call `SetRenderState` | `IFC(m_pD3DDevice->SetRenderState(`
- 3166: enum `D3DRS_ZFUNC` | `D3DRS_ZFUNC,`
- 3167: enum `D3DCMP_LESSEQUAL` | `D3DCMP_LESSEQUAL`
- 3170: call `SetRenderState` | `IFC(m_pD3DDevice->SetRenderState(`
- 3171: enum `D3DRS_ZWRITEENABLE` | `D3DRS_ZWRITEENABLE,`
- 3175: enum `D3DMULTISAMPLE_NONE` | `if (m_pD3DTargetSurface->Desc().MultiSampleType != D3DMULTISAMPLE_NONE)`
- 3177: call `SetRenderState` | `IFC(m_pD3DDevice->SetRenderState(`
- 3178: enum `D3DRS_MULTISAMPLEANTIALIAS` | `D3DRS_MULTISAMPLEANTIALIAS,`
- 3216: call `SetRenderState` | `IFC(m_pD3DDevice->SetRenderState(`
- 3217: enum `D3DRS_CULLMODE` | `D3DRS_CULLMODE,`
- 3224: call `SetDepthStencilSurface` | `IFC(m_pD3DDevice->SetDepthStencilSurface(m_pD3DStencilSurface));`
- 3226: call `SetRenderState` | `IFC(m_pD3DDevice->SetRenderState(`
- 3227: enum `D3DRS_ZFUNC` | `D3DRS_ZFUNC,`
- 3233: call `SetDepthStencilSurface` | `IFC(m_pD3DDevice->SetDepthStencilSurface(NULL));`
- 3236: enum `D3DMULTISAMPLE_NONE` | `if (m_pD3DTargetSurfaceFor3DNoRef->Desc().MultiSampleType != D3DMULTISAMPLE_NONE)`
- 3238: call `SetRenderState` | `IFC(m_pD3DDevice->SetRenderState(`
- 3239: enum `D3DRS_MULTISAMPLEANTIALIAS` | `D3DRS_MULTISAMPLEANTIALIAS,`
- 3425: call `GetSurfaceLevel` | `IFC(pD3DTexture->GetSurfaceLevel(`
- 3430: enum `D3DTEXF_NONE` | `const D3DTEXTUREFILTERTYPE d3dFilter = D3DTEXF_NONE;`
- 3432: call `StretchRect` | `IFC(m_pD3DDevice->StretchRect(`

## core/hw/hwsurfrt.h

- 223: enum `D3DMULTISAMPLE_TYPE` | `__inout_ecount(1) D3DMULTISAMPLE_TYPE &MultisampleType`
- 279: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE MultisampleType`
- 389: enum `D3DMULTISAMPLE_TYPE` | `__inout_ecount(1) D3DMULTISAMPLE_TYPE &MultisampleType`

## core/hw/hwsurfrtdata.cpp

- 305: call `GetType` | `if (   pBrush->GetType() != BrushGradientLinear`
- 306: call `GetType` | `&& pBrush->GetType() != BrushGradientRadial`
- 458: call `GetType` | `switch (pBrush->GetType())`
- 596: call `GetType` | `switch (pShader->GetType())`
- 635: call `GetType` | `switch (pShader->GetType())`
- 686: call `GetType` | `NO_DEFAULT("Has pShader->GetType() changed?");`
- 692: call `GetType` | `NO_DEFAULT("Has pShader->GetType() changed?");`
- 874: enum `D3DERR_OUTOFVIDEOMEMORY` | `if (hrReasonForFallback == D3DERR_OUTOFVIDEOMEMORY)`

## core/hw/hwsw3dfallback.cpp

- 201: call `LockRect` | `IFC(m_pD3DTargetSurface->LockRect(`
- 259: call `UnlockRect` | `IFC(m_pD3DTargetSurface->UnlockRect());`
- 267: enum `D3DMULTISAMPLE_NONE` | `D3DMULTISAMPLE_TYPE MultisampleType = D3DMULTISAMPLE_NONE;`
- 267: enum `D3DMULTISAMPLE_TYPE` | `D3DMULTISAMPLE_TYPE MultisampleType = D3DMULTISAMPLE_NONE;`
- 276: enum `D3DMULTISAMPLE_NONE` | `Assert(MultisampleType == D3DMULTISAMPLE_NONE);`
- 460: call `CreateRenderTarget` | `IFC(m_pD3DDevice->CreateRenderTarget(`
- 464: enum `D3DMULTISAMPLE_NONE` | `D3DMULTISAMPLE_NONE,`
- 531: call `LockRect` | `IFC(m_pD3DTargetSurface->LockRect(`
- 647: call `UnlockRect` | `MIL_THR_SECONDARY(m_pD3DTargetSurface->UnlockRect());`

## core/hw/hwtexturedcolorsource.cpp

- 326: call `SetSamplerState` | `IFC(m_pDevice->SetSamplerState(`
- 328: enum `D3DSAMP_ADDRESSU` | `D3DSAMP_ADDRESSU,`
- 332: call `SetSamplerState` | `IFC(m_pDevice->SetSamplerState(`
- 334: enum `D3DSAMP_ADDRESSV` | `D3DSAMP_ADDRESSV,`
- 338: enum `D3DTADDRESS_BORDER` | `if (m_taU == D3DTADDRESS_BORDER)`
- 340: call `SetSamplerState` | `IFC(m_pDevice->SetSamplerState(`
- 342: enum `D3DSAMP_BORDERCOLOR` | `D3DSAMP_BORDERCOLOR,`
- 347: call `SetTextureStageState` | `IFC(m_pDevice->SetTextureStageState(`
- 349: enum `D3DTSS_TEXCOORDINDEX` | `D3DTSS_TEXCOORDINDEX,`
- 363: call `SetTextureStageState` | `IFC(m_pDevice->SetTextureStageState(`
- 365: enum `D3DTSS_TEXTURETRANSFORMFLAGS` | `D3DTSS_TEXTURETRANSFORMFLAGS,`
- 366: enum `D3DTTFF_DISABLE` | `D3DTTFF_DISABLE`
- 383: call `SetTransform` | `IFC(m_pDevice->SetTransform(`
- 388: call `SetTextureStageState` | `IFC(m_pDevice->SetTextureStageState(`
- 390: enum `D3DTSS_TEXTURETRANSFORMFLAGS` | `D3DTSS_TEXTURETRANSFORMFLAGS,`
- 391: enum `D3DTTFF_COUNT2` | `D3DTTFF_COUNT2`
- 448: enum `D3DTADDRESS_CLAMP` | `*ptaU = D3DTADDRESS_CLAMP;`
- 449: enum `D3DTADDRESS_CLAMP` | `*ptaV = D3DTADDRESS_CLAMP;`
- 453: enum `D3DTADDRESS_MIRROR` | `*ptaU = D3DTADDRESS_MIRROR;`
- 454: enum `D3DTADDRESS_WRAP` | `*ptaV = D3DTADDRESS_WRAP;`
- 458: enum `D3DTADDRESS_WRAP` | `*ptaU = D3DTADDRESS_WRAP;`
- 459: enum `D3DTADDRESS_MIRROR` | `*ptaV = D3DTADDRESS_MIRROR;`
- 463: enum `D3DTADDRESS_MIRROR` | `*ptaU = D3DTADDRESS_MIRROR;`
- 464: enum `D3DTADDRESS_MIRROR` | `*ptaV = D3DTADDRESS_MIRROR;`
- 468: enum `D3DTADDRESS_WRAP` | `*ptaU = D3DTADDRESS_WRAP;`
- 469: enum `D3DTADDRESS_WRAP` | `*ptaV = D3DTADDRESS_WRAP;`
- 473: enum `D3DTADDRESS_BORDER` | `*ptaU = D3DTADDRESS_BORDER;`
- 474: enum `D3DTADDRESS_BORDER` | `*ptaV = D3DTADDRESS_BORDER;`

## core/hw/hwtexturert.cpp

- 49: enum `D3DFMT_A8R8G8B8` | `D3DFMT_A8R8G8B8`
- 61: enum `D3DFMT_A8R8G8B8` | `D3DFMT_A8R8G8B8,`
- 103: enum `D3DRTYPE_TEXTURE` | `sdLevel0.Type = D3DRTYPE_TEXTURE;`
- 104: enum `D3DUSAGE_RENDERTARGET` | `sdLevel0.Usage = D3DUSAGE_RENDERTARGET;`
- 108: enum `D3DPOOL_DEFAULT` | `sdLevel0.Pool = D3DPOOL_DEFAULT;`
- 109: enum `D3DMULTISAMPLE_NONE` | `sdLevel0.MultiSampleType = D3DMULTISAMPLE_NONE;`

## core/hw/hwvertexbuffer.cpp

- 1028: call `SetStreamSource` | `IFC(m_pDeviceNoRef->SetStreamSource(`
- 1033: call `SetIndices` | `IFC(m_pDeviceNoRef->SetIndices(`
- 1371: call `GetViewport` | `MilPointAndSizeL rcViewport = m_pDeviceNoRef->GetViewport();`
- 2636: call `DrawPrimitive` | `IFC(m_pVB->DrawPrimitive(m_pDeviceNoRef));`

## core/hw/shaders/ShaderGen/Device.cpp

- 107: enum `D3DERR_INVALIDCALL` | `return D3DERR_INVALIDCALL;`
- 124: enum `D3DERR_INVALIDCALL` | `return D3DERR_INVALIDCALL;`
- 272: enum `D3DMULTISAMPLE_TYPE` | `HRESULT CFakeDevice::CreateRenderTargetEx(UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, DWORD, BOOL, IDirect3DSurface9 **, HANDLE *, DWORD)...`
- 274: enum `D3DMULTISAMPLE_TYPE` | `HRESULT CFakeDevice::CreateDepthStencilSurfaceEx(UINT,UINT,D3DFORMAT,D3DMULTISAMPLE_TYPE,DWORD,BOOL,IDirect3DSurface9 **,HANDLE *,DWORD) ...`

## core/hw/shaders/ShaderGen/Device.h

- 56: enum `D3DPRESENT_PARAMETERS` | `STDMETHOD(CreateAdditionalSwapChain)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain);`
- 59: enum `D3DPRESENT_PARAMETERS` | `STDMETHOD(Reset)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters);`
- 61: enum `D3DBACKBUFFER_TYPE` | `STDMETHOD(GetBackBuffer)(THIS_ UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer);`
- 71: enum `D3DMULTISAMPLE_TYPE` | `STDMETHOD(CreateRenderTarget)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL...`
- 72: enum `D3DMULTISAMPLE_TYPE` | `STDMETHOD(CreateDepthStencilSurface)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuali...`
- 177: enum `D3DMULTISAMPLE_TYPE` | `STDMETHOD(CreateRenderTargetEx)(UINT, UINT, D3DFORMAT, D3DMULTISAMPLE_TYPE, DWORD, BOOL, IDirect3DSurface9 **, HANDLE *, DWORD);`
- 179: enum `D3DMULTISAMPLE_TYPE` | `STDMETHOD(CreateDepthStencilSurfaceEx)(UINT,UINT,D3DFORMAT,D3DMULTISAMPLE_TYPE,DWORD,BOOL,IDirect3DSurface9 **,HANDLE *,DWORD);`

## core/hw/swfallback.cpp

- 68: enum `D3DFMT_A8R8G8B8` | `d3dsd.Format = D3DFMT_A8R8G8B8;`
- 69: enum `D3DRTYPE_TEXTURE` | `d3dsd.Type = D3DRTYPE_TEXTURE;`
- 72: enum `D3DMULTISAMPLE_NONE` | `d3dsd.MultiSampleType = D3DMULTISAMPLE_NONE;`
- 83: enum `D3DFMT_A8R8G8B8` | `Assert(d3dsd.Format == D3DFMT_A8R8G8B8);`
- 141: enum `D3DFMT_A8R8G8B8` | `d3dsd.Format = D3DFMT_A8R8G8B8;`
- 142: enum `D3DRTYPE_TEXTURE` | `d3dsd.Type = D3DRTYPE_TEXTURE;`
- 145: enum `D3DMULTISAMPLE_NONE` | `d3dsd.MultiSampleType = D3DMULTISAMPLE_NONE;`
- 156: enum `D3DFMT_A8R8G8B8` | `Assert(d3dsd.Format == D3DFMT_A8R8G8B8);`

## core/meta/desktoprt.cpp

- 315: enum `D3DDEVTYPE_HAL` | `D3DDEVTYPE d3dDeviceType = D3DDEVTYPE_HAL;`
- 319: enum `D3DDEVTYPE_REF` | `d3dDeviceType = D3DDEVTYPE_REF;`
- 324: enum `D3DDEVTYPE_SW` | `d3dDeviceType = D3DDEVTYPE_SW;`
- 1277: call `WaitForVBlank` | `hr = m_rgMetaData[0].pInternalRTHWND->WaitForVBlank();`

## core/resources/BlurEffect.cpp

- 972: call `SetTexture` | `IFC(pDevice->SetTexture(0, pTextureNoRef_A));`
- 1009: enum `D3DCLEAR_TARGET` | `IFC(pDevice->Clear(0, NULL, D3DCLEAR_TARGET, colBlank, 0, 0));`
- 1037: enum `D3DFMT_A32B32G32R32F` | `D3DFMT_A32B32G32R32F,`
- 1047: enum `D3DFMT_A32B32G32R32F` | `D3DFMT_A32B32G32R32F,`
- 1072: call `SetTexture` | `IFC(pDevice->SetTexture(0, pTextureNoRef_A));`
- 1106: enum `D3DCLEAR_TARGET` | `IFC(pDevice->Clear(0, NULL, D3DCLEAR_TARGET, colBlank, 0, 0));`
- 1121: call `SetTexture` | `IFC(pDevice->SetTexture(0, pTextureNoRef_A));`
- 1127: call `SetTexture` | `IFC(pDevice->SetTexture(0, pPipelineDestRT));`
- 1181: enum `D3DCLEAR_TARGET` | `IFC(pDevice->Clear(0, NULL, D3DCLEAR_TARGET, colBlank, 0, 0));`
- 1204: enum `D3DFMT_A8R8G8B8` | `D3DFMT_A8R8G8B8,`
- 1227: call `SetTexture` | `IFC(pDevice->SetTexture(0, pTextureNoRef_A));`
- 1234: enum `D3DCLEAR_TARGET` | `IFC(pDevice->Clear(0, NULL, D3DCLEAR_TARGET, colBlank, 0, 0));`
- 1268: enum `D3DCLEAR_TARGET` | `IFC(pDevice->Clear(0, NULL, D3DCLEAR_TARGET, colBlank, 0, 0));`
- 1295: call `SetTexture` | `IFC(pDevice->SetTexture(0, pTextureNoRef_A));`
- 1412: call `SetTexture` | `IFC(pDevice->SetTexture(1, (passNumber%2 == 1) ? pTexture_C : pTexture_B));`
- 1415: enum `D3DCLEAR_TARGET` | `IFC(pDevice->Clear(0, NULL, D3DCLEAR_TARGET, colBlank, 0, 0));`
- 1442: call `SetTexture` | `IFC(pDevice->SetTexture(0, (useTextureCAsSource) ? pTexture_C : pTexture_B));`
- 1582: call `SetPixelShaderConstantF` | `IFC(pDevice->SetPixelShaderConstantF(0, arrSize, 1));`
- 1586: call `SetPixelShaderConstantF` | `IFC(pDevice->SetPixelShaderConstantF(1, arrSamplingIndex, 1));`
- 1589: call `SetPixelShaderConstantF` | `IFC(pDevice->SetPixelShaderConstantF(2, arrcSamples, 1));`
- 1607: call `SetPixelShaderConstantF` | `IFC(pDevice->SetPixelShaderConstantF(3, arrWeights, 4));`

## core/resources/DropShadowEffect.cpp

- 581: enum `D3DFMT_A8R8G8B8` | `D3DFMT_A8R8G8B8,`
- 615: call `SetTexture` | `IFC(pDevice->SetTexture(0, pTextureNoRef_A));`
- 667: call `SetTexture` | `IFC(pDevice->SetTexture(0, pTextureNoRef_A));`
- 668: call `SetTexture` | `IFC(pDevice->SetTexture(1, pTexture_B));`
- 1033: call `SetPixelShaderConstantF` | `IFC(pDevice->SetPixelShaderConstantF(0, arrOffset, 1));`
- 1037: call `SetPixelShaderConstantF` | `IFC(pDevice->SetPixelShaderConstantF(1, arrColor, 1));`
- 1041: call `SetPixelShaderConstantF` | `IFC(pDevice->SetPixelShaderConstantF(2, arrOpacity, 1));`

## core/resources/Effect.cpp

- 94: enum `D3DRTYPE_TEXTURE` | `sdLevel0.Type = D3DRTYPE_TEXTURE;`
- 95: enum `D3DUSAGE_RENDERTARGET` | `sdLevel0.Usage = D3DUSAGE_RENDERTARGET;`
- 99: enum `D3DPOOL_DEFAULT` | `sdLevel0.Pool = D3DPOOL_DEFAULT;`
- 100: enum `D3DMULTISAMPLE_NONE` | `sdLevel0.MultiSampleType = D3DMULTISAMPLE_NONE;`
- 194: call `GetTransform` | `IFC(pDevice->GetTransform(`
- 230: call `SetVertexShaderConstantF` | `IFC(pDevice->SetVertexShaderConstantF(0, reinterpret_cast<const float *>(&matVertexTransform), 4));`
- 259: call `SetSamplerState` | `IFC(pDevice->SetSamplerState(`
- 261: enum `D3DSAMP_ADDRESSU` | `D3DSAMP_ADDRESSU,`
- 262: enum `D3DTADDRESS_CLAMP` | `D3DTADDRESS_CLAMP));`
- 264: call `SetSamplerState` | `IFC(pDevice->SetSamplerState(`
- 266: enum `D3DSAMP_ADDRESSV` | `D3DSAMP_ADDRESSV,`
- 267: enum `D3DTADDRESS_CLAMP` | `D3DTADDRESS_CLAMP));`
- 273: call `SetSamplerState` | `IFC(pDevice->SetSamplerState(`
- 275: enum `D3DSAMP_MINFILTER` | `D3DSAMP_MINFILTER,`
- 276: enum `D3DTEXF_LINEAR` | `D3DTEXF_LINEAR));`
- 278: call `SetSamplerState` | `IFC(pDevice->SetSamplerState(`
- 280: enum `D3DSAMP_MAGFILTER` | `D3DSAMP_MAGFILTER,`
- 281: enum `D3DTEXF_LINEAR` | `D3DTEXF_LINEAR));`
- 285: call `SetSamplerState` | `IFC(pDevice->SetSamplerState(`
- 287: enum `D3DSAMP_MINFILTER` | `D3DSAMP_MINFILTER,`
- 288: enum `D3DTEXF_POINT` | `D3DTEXF_POINT));`
- 290: call `SetSamplerState` | `IFC(pDevice->SetSamplerState(`
- 292: enum `D3DSAMP_MAGFILTER` | `D3DSAMP_MAGFILTER,`
- 293: enum `D3DTEXF_POINT` | `D3DTEXF_POINT));`

## core/resources/PixelShader.cpp

- 205: enum `D3DERR_OUTOFVIDEOMEMORY` | `if (hr == E_OUTOFMEMORY \|\| hr == D3DERR_OUTOFVIDEOMEMORY)`
- 306: enum `D3DERR_OUTOFVIDEOMEMORY` | `if (hr == E_OUTOFMEMORY \|\| hr == D3DERR_OUTOFVIDEOMEMORY)`

## core/resources/ShaderEffect.cpp

- 284: call `SetPixelShaderConstantF` | `IFC(pDevice->SetPixelShaderConstantF(m_data.m_DdxUvDdyUvRegisterIndex, &(registerValue[0]), 1));`
- 317: call `SetTexture` | `IFC(pDevice->SetTexture(pCurrentSamplerData->GetSamplerRegister(), NULL));`
- 622: call `SetTexture` | `IFC(pDevice->SetTexture(`
- 916: call `SetPixelShaderConstantF` | `IFC(pDevice->SetPixelShaderConstantF(*pFloatRegisterIndices++, pFloatValues, 1));`
- 926: call `SetPixelShaderConstantI` | `IFC(pDevice->SetPixelShaderConstantI(*pIntRegisterIndices++, pIntValues));`
- 936: call `SetPixelShaderConstantB` | `IFC(pDevice->SetPixelShaderConstantB(*pBoolRegisterIndices++, *pBoolValues));`

## core/resources/bitmapres.cpp

- 100: call `AddDirtyRect` | `IFC(m_pIBitmap->AddDirtyRect(pDirtyRect));`

## core/resources/modelrenderwalker.cpp

- 133: enum `D3DCULL_CCW` | `Assert(m_pCtxState->CullMode3D == D3DCULL_CW \|\| m_pCtxState->CullMode3D == D3DCULL_CCW);`
- 133: enum `D3DCULL_CW` | `Assert(m_pCtxState->CullMode3D == D3DCULL_CW \|\| m_pCtxState->CullMode3D == D3DCULL_CCW);`
- 282: enum `D3DCULL_CW` | `(m_pCtxState->CullMode3D == D3DCULL_CW) ^`
- 286: enum `D3DCULL_CCW` | `m_pCtxState->CullMode3D = fCullIsCW ? D3DCULL_CW : D3DCULL_CCW;`
- 286: enum `D3DCULL_CW` | `m_pCtxState->CullMode3D = fCullIsCW ? D3DCULL_CW : D3DCULL_CCW;`

## core/resources/prerenderwalker.cpp

- 128: call `GetTransform` | `CMilTransform3DDuce *pTransform = pModel->GetTransform();`
- 208: call `GetTransform` | `if (pModel->GetTransform())`

## core/resources/specularmaterial.cpp

- 69: enum `D3DCULL_CCW` | `Assert(pContextState->CullMode3D == D3DCULL_CW \|\| pContextState->CullMode3D == D3DCULL_CCW);`
- 69: enum `D3DCULL_CW` | `Assert(pContextState->CullMode3D == D3DCULL_CW \|\| pContextState->CullMode3D == D3DCULL_CCW);`

## core/sw/swlib/swsurfrt.cpp

- 438: enum `D3DERR_NOTAVAILABLE` | `if (   hr == D3DERR_NOTAVAILABLE`
- 439: enum `D3DERR_NOTFOUND` | `\|\| hr == D3DERR_NOTFOUND)`

## core/uce/composition.cpp

- 477: enum `D3DERR_NOTAVAILABLE` | `if (hr != D3DERR_NOTAVAILABLE)`
- 488: enum `D3DERR_NOTAVAILABLE` | `TraceTag((tagMILWarning, "CComposition::ProcessCommandBatch: ignoring D3DERR_NOTAVAILABLE"));`
- 1899: call `WaitForVBlank` | `RRETURN(m_pRenderTargetManager->WaitForVBlank());`

## core/uce/drawingcontext.cpp

- 318: enum `D3DCMP_LESSEQUAL` | `m_contextState.DepthBufferFunction3D = D3DCMP_LESSEQUAL;`
- 5280: enum `D3DSHADER_VERSION_MAJOR` | `byte majorVSVersion = D3DSHADER_VERSION_MAJOR(caps.VertexShaderVersion);`
- 5281: enum `D3DSHADER_VERSION_MAJOR` | `byte majorPSVersion = D3DSHADER_VERSION_MAJOR(caps.PixelShaderVersion);`

## core/uce/hwndtarget.cpp

- 1049: enum `D3DERR_OUTOFVIDEOMEMORY` | `case D3DERR_OUTOFVIDEOMEMORY:`
- 1237: call `WaitForVBlank` | `MIL_THR(m_pRenderTarget->WaitForVBlank());`

## core/uce/partitionmanager.cpp

- 522: enum `D3DERR_OUTOFVIDEOMEMORY` | `if (hrFailureCode == D3DERR_OUTOFVIDEOMEMORY)`
- 524: enum `D3DERR_OUTOFVIDEOMEMORY` | `pPartition->m_hrZombieNotificationFailureReason = D3DERR_OUTOFVIDEOMEMORY;`

## core/uce/render3dcontext.cpp

- 116: enum `D3DCULL_CCW` | `pContextState->CullMode3D = det < 0 ? D3DCULL_CCW : D3DCULL_CW;`
- 116: enum `D3DCULL_CW` | `pContextState->CullMode3D = det < 0 ? D3DCULL_CCW : D3DCULL_CW;`

## core/uce/rendertargetmanager.cpp

- 422: enum `D3DERR_OUTOFVIDEOMEMORY` | `if (hr == D3DERR_OUTOFVIDEOMEMORY \|\|`
- 431: enum `D3DERR_NOTAVAILABLE` | `case D3DERR_NOTAVAILABLE:`
- 434: enum `D3DERR_NOTAVAILABLE` | `TraceTag((tagMILWarning, "CRenderTargetManager::Render (intermediate): ignoring D3DERR_NOTAVAILABLE"));`
- 485: enum `D3DERR_OUTOFVIDEOMEMORY` | `if (hr == D3DERR_OUTOFVIDEOMEMORY \|\|`
- 494: enum `D3DERR_NOTAVAILABLE` | `case D3DERR_NOTAVAILABLE:`
- 501: enum `D3DERR_NOTAVAILABLE` | `TraceTag((tagMILWarning, "CRenderTargetManager::Present: Ignoring D3DERR_NOTAVAILABLE"));`
- 1061: call `WaitForVBlank` | `hr = pTarget->WaitForVBlank();`
