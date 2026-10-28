// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//
//  Description:
//      D3D11 엔진에만 있는 기능을 노출하는 추가 export (D8). 기존 107개 export 와.
//      DUCE 프로토콜은 그대로이며, 동반 관리 어셈블리 WpfGfx.Dx11 이 이 함수들을.
//      P/Invoke 한다. 스톡 DX9 엔진에는 이 export 가 없으므로 관리 쪽은.
//      NativeLibrary.TryGetExport 로 존재 여부를 먼저 확인한다.
//
//      ABI 규칙: 구조체는 cbSize 로 시작하고, 필드는 끝에만 덧붙인다. 동작이 바뀌면.
//      uAbiVersion 을 올린다.
//
//------------------------------------------------------------------------------

#include "precomp.hpp"

#define WPFGFX_DX11_ABI_VERSION 1

struct WpfGfxDx11Info
{
    UINT cbSize;            // sizeof(WpfGfxDx11Info), 호출자가 채운다.
    UINT uAbiVersion;       // WPFGFX_DX11_ABI_VERSION
    UINT uFeatureLevel;     // D3D_FEATURE_LEVEL of the primary display's device
    UINT uDisplayCount;     // displays with a DXGI adapter
    LUID luidAdapter;       // adapter of the primary display's device
};

//+-----------------------------------------------------------------------------
//
//  Function:  GetEngineDevice
//
//  Synopsis:  Returns (AddRef'd) the engine's hardware device wrapper for a
//             display, creating it when no window has needed it yet. Mirrors
//             what the media presenter does from its own thread.
//
//------------------------------------------------------------------------------

static HRESULT
GetEngineDevice(
    UINT uDisplayIndex,
    __deref_out CD3DDeviceLevel1 **ppDevice,
    __deref_opt_out CDisplaySet const **ppDisplaySet
    )
{
    HRESULT hr = S_OK;
    CFloatFPU oGuard;

    CD3DDeviceManager *pD3DDeviceManager = CD3DDeviceManager::Get();
    CDisplaySet const *pDisplaySet = NULL;

    *ppDevice = NULL;

    if (ppDisplaySet)
    {
        *ppDisplaySet = NULL;
    }

    if (g_DisplayManager.HasCurrentDisplaySet())
    {
        g_DisplayManager.GetCurrentDisplaySet(&pDisplaySet);
    }
    else
    {
        IFC(g_DisplayManager.DangerousGetLatestDisplaySet(&pDisplaySet));
    }

    if (!pDisplaySet->IsD3DAvailable() || pDisplaySet->GetNumD3DRecognizedAdapters() <= uDisplayIndex)
    {
        IFC(WGXERR_NO_HARDWARE_DEVICE);
    }

    IFC(pD3DDeviceManager->GetD3DDeviceAndPresentParams(
        GetDesktopWindow(),
        MilRTInitialization::Default,
        pDisplaySet->Display(uDisplayIndex),
        MilDeviceKind_Hardware,
        ppDevice,
        NULL,
        NULL));

    if (ppDisplaySet)
    {
        *ppDisplaySet = pDisplaySet;
        pDisplaySet = NULL;
    }

Cleanup:
    ReleaseInterface(pDisplaySet);

    if (pD3DDeviceManager)
    {
        CD3DDeviceManager::Release();
    }

    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Function:  WpfGfxDx11_GetInfo
//
//  Synopsis:  Reports the ABI version and the primary display device's
//             feature level and adapter.
//
//------------------------------------------------------------------------------

HRESULT WINAPI
WpfGfxDx11_GetInfo(
    __inout_ecount(1) WpfGfxDx11Info *pInfo
    )
{
    HRESULT hr = S_OK;
    CD3DDeviceLevel1 *pDevice = NULL;
    CDisplaySet const *pDisplaySet = NULL;

    CHECKPTRARG(pInfo);

    if (pInfo->cbSize < sizeof(WpfGfxDx11Info))
    {
        IFC(E_INVALIDARG);
    }

    IFC(GetEngineDevice(0, &pDevice, &pDisplaySet));

    pInfo->uAbiVersion = WPFGFX_DX11_ABI_VERSION;
    pInfo->uFeatureLevel = pDevice->GetD3DDeviceNoRef()->GetFeatureLevel();
    pInfo->uDisplayCount = pDisplaySet->GetNumD3DRecognizedAdapters();
    pInfo->luidAdapter = pDevice->GetD3DAdapterLUID();

Cleanup:
    ReleaseInterface(pDevice);
    ReleaseInterface(pDisplaySet);
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Function:  WpfGfxDx11_GetDevice
//
//  Synopsis:  Returns the engine's ID3D11Device (AddRef'd) for a display.
//             Resources created on it can be given to D3DImage.SetBackBuffer
//             without any cross-device copy. The immediate context belongs to
//             the engine's render thread: callers must not use it. Use a
//             deferred context or share resources created with
//             D3D11_RESOURCE_MISC_SHARED from a device of their own.
//
//------------------------------------------------------------------------------

HRESULT WINAPI
WpfGfxDx11_GetDevice(
    UINT uDisplayIndex,
    __deref_out ID3D11Device **ppD3DDevice
    )
{
    HRESULT hr = S_OK;
    CD3DDeviceLevel1 *pDevice = NULL;

    CHECKPTRARG(ppD3DDevice);
    *ppD3DDevice = NULL;

    IFC(GetEngineDevice(uDisplayIndex, &pDevice, NULL));

    *ppD3DDevice = pDevice->GetD3DDeviceNoRef();
    (*ppD3DDevice)->AddRef();

Cleanup:
    ReleaseInterface(pDevice);
    RRETURN(hr);
}
