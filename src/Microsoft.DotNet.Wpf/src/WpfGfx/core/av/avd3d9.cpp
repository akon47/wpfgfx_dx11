// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//
//  Description:
//      미디어용 Direct3D 9Ex 디바이스 생성과 동기화. avd3d9.h 참고.
//
//------------------------------------------------------------------------------

#include "precomp.hpp"

typedef HRESULT (WINAPI *PFN_DIRECT3DCREATE9EX)(UINT SDKVersion, IDirect3D9Ex **ppD3D);

//+-----------------------------------------------------------------------------
//
//  Function:
//      AVEnsureMediaDevice9
//
//  Synopsis:
//      Creates the D3D9Ex device for the adapter of pRenderDevice once and
//      hangs it off the render device. d3d9.dll stays loaded for the process.
//
//------------------------------------------------------------------------------

HRESULT
AVEnsureMediaDevice9(
    __inout_ecount(1) CD3DDeviceLevel1 *pRenderDevice
    )
{
    HRESULT hr = S_OK;
    IDirect3D9Ex *pD3D = NULL;
    IDirect3DDevice9Ex *pDevice = NULL;
    static HMODULE s_hD3D9 = NULL;                          // never freed
    static PFN_DIRECT3DCREATE9EX s_pfnDirect3DCreate9Ex = NULL;

    CGuard<CD3DDeviceLevel1> guard(*pRenderDevice);

    if (pRenderDevice->GetMediaDevice9NoRef() != NULL)
    {
        goto Cleanup;
    }

    if (s_pfnDirect3DCreate9Ex == NULL)
    {
        HMODULE hD3D9 = LoadLibraryW(L"d3d9.dll");
        IFCW32(hD3D9);

        PFN_DIRECT3DCREATE9EX pfn = reinterpret_cast<PFN_DIRECT3DCREATE9EX>(GetProcAddress(hD3D9, "Direct3DCreate9Ex"));
        IFCW32(pfn);

        s_hD3D9 = hD3D9;
        s_pfnDirect3DCreate9Ex = pfn;
    }

    IFC(s_pfnDirect3DCreate9Ex(D3D_SDK_VERSION, &pD3D));

    //
    // Pick the D3D9 adapter with the render device's LUID (WARP render
    // devices have none: use the default adapter).
    //
    UINT uAdapter = D3DADAPTER_DEFAULT;
    {
        const LUID luidWanted = pRenderDevice->GetD3DAdapterLUID();

        if (luidWanted.LowPart != 0 || luidWanted.HighPart != 0)
        {
            const UINT cAdapters = pD3D->GetAdapterCount();

            for (UINT i = 0; i < cAdapters; i++)
            {
                LUID luid;

                if (   SUCCEEDED(pD3D->GetAdapterLUID(i, &luid))
                    && luid.LowPart == luidWanted.LowPart
                    && luid.HighPart == luidWanted.HighPart)
                {
                    uAdapter = i;
                    break;
                }
            }
        }
    }

    {
        D3DPRESENT_PARAMETERS pp;
        ZeroMemory(&pp, sizeof(pp));
        pp.BackBufferWidth = 1;
        pp.BackBufferHeight = 1;
        pp.BackBufferFormat = D3DFMT_UNKNOWN;
        pp.BackBufferCount = 1;
        pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        pp.hDeviceWindow = GetDesktopWindow();
        pp.Windowed = TRUE;
        pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

        // DXVA2 and the EVR mixer need a multithreaded device. The device is
        // never used for presentation or vertex processing.
        const DWORD dwBaseFlags = D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE | D3DCREATE_NOWINDOWCHANGES;

        hr = pD3D->CreateDeviceEx(
            uAdapter,
            D3DDEVTYPE_HAL,
            pp.hDeviceWindow,
            dwBaseFlags | D3DCREATE_HARDWARE_VERTEXPROCESSING,
            &pp,
            NULL,
            &pDevice
            );

        if (FAILED(hr))
        {
            hr = pD3D->CreateDeviceEx(
                uAdapter,
                D3DDEVTYPE_HAL,
                pp.hDeviceWindow,
                dwBaseFlags | D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                &pp,
                NULL,
                &pDevice
                );
        }

        IFC(hr);
    }

    pRenderDevice->SetMediaDevice9(pDevice);

    MIL_DX11_LOG("media: D3D9Ex device created on D3D9 adapter %u for render device %p", uAdapter, pRenderDevice);

Cleanup:
    if (FAILED(hr))
    {
        MIL_DX11_LOG("media: D3D9Ex device creation failed hr=0x%08x", hr);
        hr = WGXERR_AV_VIDEOACCELERATIONNOTAVAILABLE;
    }

    ReleaseInterface(pDevice);
    ReleaseInterface(pD3D);

    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Function:
//      AVWaitForMediaDevice9
//
//  Synopsis:
//      Blocks until the GPU has finished everything submitted so far on the
//      media device, so a mixed frame is complete before the D3D11 engine
//      device samples the shared texture.
//
//------------------------------------------------------------------------------

HRESULT
AVWaitForMediaDevice9(
    __in_ecount(1) IDirect3DDevice9 *pDevice,
    __deref_inout_ecount_opt(1) IDirect3DQuery9 **ppQuery
    )
{
    HRESULT hr = S_OK;

    if (*ppQuery == NULL)
    {
        IFC(pDevice->CreateQuery(D3DQUERYTYPE_EVENT, ppQuery));
    }

    IFC((*ppQuery)->Issue(D3DISSUE_END));

    for (;;)
    {
        hr = (*ppQuery)->GetData(NULL, 0, D3DGETDATA_FLUSH);

        if (hr != S_FALSE)
        {
            break;
        }

        SwitchToThread();
    }

    IFC(hr);

Cleanup:
    RRETURN(hr);
}
