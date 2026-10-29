// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

//+-----------------------------------------------------------------------------
//
//  Description:
//      D3D11 포팅 진단 로그 구현. mildx11log.h 참고.
//
//------------------------------------------------------------------------------

#include "precomp.hpp"
#include <psapi.h>
#include <dxgi1_4.h>
#include <stdarg.h>
#include <stdio.h>
#include <d3d11sdklayers.h>
#include <strsafe.h>

static LONG s_lLogState = 0;            // 0 = 미확인, 1 = 꺼짐, 2 = 켜짐.
static char s_szLogPath[MAX_PATH] = {};

bool MilDx11LogEnabled()
{
    LONG lState = s_lLogState;

    if (lState == 0)
    {
        DWORD cch = GetEnvironmentVariableA("WPFGFX_DX11_LOG", s_szLogPath, ARRAYSIZE(s_szLogPath));

        lState = (cch > 0 && cch < ARRAYSIZE(s_szLogPath)) ? 2 : 1;

        InterlockedCompareExchange(&s_lLogState, lState, 0);
    }

    if (lState == 2)
    {
        g_fMilDx11Stats = true;
    }

    return lState == 2;
}

bool MilDx11TestInjectDeviceLoss()
{
    static LONG s_lTarget = -1;         // -1 = 미확인, 0 = 꺼짐, n = n번째 Present.
    static LONG s_cPresents = 0;

    if (s_lTarget < 0)
    {
        char szValue[16] = {};
        DWORD cch = GetEnvironmentVariableA("WPFGFX_DX11_TEST_DEVICE_LOSS", szValue, ARRAYSIZE(szValue));
        LONG lTarget = (cch > 0 && cch < ARRAYSIZE(szValue)) ? atol(szValue) : 0;
        InterlockedCompareExchange(&s_lTarget, max(lTarget, 0L), -1);
    }

    if (s_lTarget == 0)
    {
        return false;
    }

    return InterlockedIncrement(&s_cPresents) == s_lTarget;
}

void __cdecl MilDx11Log(__in_z const char *pszFormat, ...)
{
    if (!MilDx11LogEnabled())
    {
        return;
    }

    char szLine[1024];
    int cch = _snprintf_s(szLine, ARRAYSIZE(szLine), _TRUNCATE, "[%08x %5u] ",
                          GetTickCount(), GetCurrentThreadId());

    if (cch < 0)
    {
        cch = 0;
    }

    va_list args;
    va_start(args, pszFormat);
    _vsnprintf_s(szLine + cch, ARRAYSIZE(szLine) - cch, _TRUNCATE, pszFormat, args);
    va_end(args);

    FILE *pFile = NULL;

    if (fopen_s(&pFile, s_szLogPath, "a") == 0 && pFile != NULL)
    {
        fputs(szLine, pFile);
        fputc('\n', pFile);
        fclose(pFile);
    }
}

void MilDx11LogD3DMessages(__in_opt IUnknown *pD3DDevice, __in_z const char *pszContext)
{
    if (!MilDx11LogEnabled() || pD3DDevice == NULL)
    {
        return;
    }

    ID3D11InfoQueue *pInfoQueue = NULL;

    if (FAILED(pD3DDevice->QueryInterface(__uuidof(ID3D11InfoQueue), reinterpret_cast<void **>(&pInfoQueue))))
    {
        return;
    }

    UINT64 cMessages = pInfoQueue->GetNumStoredMessages();

    for (UINT64 i = 0; i < cMessages; i++)
    {
        SIZE_T cbMessage = 0;

        if (FAILED(pInfoQueue->GetMessage(i, NULL, &cbMessage)) || cbMessage == 0)
        {
            continue;
        }

        D3D11_MESSAGE *pMessage = static_cast<D3D11_MESSAGE *>(malloc(cbMessage));

        if (pMessage == NULL)
        {
            break;
        }

        if (SUCCEEDED(pInfoQueue->GetMessage(i, pMessage, &cbMessage)))
        {
            MilDx11Log("D3D11[%s] sev=%d id=%d: %.*s", pszContext, pMessage->Severity, pMessage->ID,
                       static_cast<int>(pMessage->DescriptionByteLength), pMessage->pDescription);
        }

        free(pMessage);
    }

    pInfoQueue->ClearStoredMessages();
    pInfoQueue->Release();
}

static LONG s_rgCreateCounts[MilDx11Site_Count] = {};

static LONG s_rgOptions[MilDx11Option_Count] = {};      // 0 = unresolved, 1 = off, 2 = on

bool MilDx11OptionEnabled(UINT uOption)
{
    if (uOption == MilDx11Option_None || uOption >= MilDx11Option_Count)
    {
        return false;
    }

    if (s_rgOptions[uOption] == 0)
    {
        const char *pszEnv = (uOption == MilDx11Option_HardwareRenderTargetBitmap) ? "WPFGFX_DX11_HW_RTB" : NULL;
        char szValue[8] = {};
        bool fOn = pszEnv != NULL
            && GetEnvironmentVariableA(pszEnv, szValue, ARRAYSIZE(szValue)) > 0
            && szValue[0] == '1';
        InterlockedCompareExchange(&s_rgOptions[uOption], fOn ? 2 : 1, 0);
    }

    return s_rgOptions[uOption] == 2;
}

void MilDx11SetOption(UINT uOption, bool fEnabled)
{
    if (uOption != MilDx11Option_None && uOption < MilDx11Option_Count)
    {
        InterlockedExchange(&s_rgOptions[uOption], fEnabled ? 2 : 1);
        MIL_DX11_LOG("option %u set to %d", uOption, fEnabled ? 1 : 0);
    }
}


void MilDx11CountCreate(MilDx11CreateSite site)
{
    InterlockedIncrement(&s_rgCreateCounts[site]);
}

void MilDx11LogLiveObjects(__in_opt IUnknown *pD3DDevice, __in_z const char *pszContext)
{
    if (!MilDx11LogEnabled() || pD3DDevice == NULL)
    {
        return;
    }

    //
    // Memory split: process private bytes (CPU heaps + driver) and the GPU
    // memory this process holds on the adapter (local = VRAM, non-local =
    // shared system memory). A leak in our code shows up in private bytes
    // only; driver pools show up in the DXGI numbers too.
    //
    {
        PROCESS_MEMORY_COUNTERS_EX pmc = { sizeof(pmc) };
        GetProcessMemoryInfo(GetCurrentProcess(), reinterpret_cast<PROCESS_MEMORY_COUNTERS *>(&pmc), sizeof(pmc));

        UINT64 uLocal = 0, uNonLocal = 0;
        IDXGIDevice *pDXGIDevice = NULL;
        IDXGIAdapter *pAdapter = NULL;
        IDXGIAdapter3 *pAdapter3 = NULL;

        if (   SUCCEEDED(pD3DDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void **>(&pDXGIDevice)))
            && SUCCEEDED(pDXGIDevice->GetAdapter(&pAdapter))
            && SUCCEEDED(pAdapter->QueryInterface(__uuidof(IDXGIAdapter3), reinterpret_cast<void **>(&pAdapter3))))
        {
            DXGI_QUERY_VIDEO_MEMORY_INFO info;
            if (SUCCEEDED(pAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info))) uLocal = info.CurrentUsage;
            if (SUCCEEDED(pAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &info))) uNonLocal = info.CurrentUsage;
        }

        if (pAdapter3) pAdapter3->Release();
        if (pAdapter) pAdapter->Release();
        if (pDXGIDevice) pDXGIDevice->Release();

        MilDx11Log("memory [%s]: private=%u MB workingSet=%u MB gpuLocal=%u MB gpuNonLocal=%u MB; created so far: rt=%ld tex=%ld ds=%ld msaa=%ld readback=%ld swapchain=%ld poolhit=%ld",
                   pszContext,
                   static_cast<UINT>(pmc.PrivateUsage >> 20), static_cast<UINT>(pmc.WorkingSetSize >> 20),
                   static_cast<UINT>(uLocal >> 20), static_cast<UINT>(uNonLocal >> 20),
                   s_rgCreateCounts[MilDx11Site_RenderTarget], s_rgCreateCounts[MilDx11Site_Texture],
                   s_rgCreateCounts[MilDx11Site_DepthStencil], s_rgCreateCounts[MilDx11Site_MsaaScratch],
                   s_rgCreateCounts[MilDx11Site_Readback], s_rgCreateCounts[MilDx11Site_SwapChain],
                   s_rgCreateCounts[MilDx11Site_PoolHit]);
    }

    ID3D11Debug *pDebug = NULL;

    if (SUCCEEDED(pD3DDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void **>(&pDebug))))
    {
        MilDx11Log("live objects [%s]:", pszContext);
        pDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
        MilDx11LogD3DMessages(pD3DDevice, "live");
        pDebug->Release();
    }
}

void MilDx11LogText(__in_z const char *pszTitle, __in_bcount(cb) const char *pText, size_t cb)
{
    if (!MilDx11LogEnabled())
    {
        return;
    }

    FILE *pFile = NULL;

    if (fopen_s(&pFile, s_szLogPath, "a") == 0 && pFile != NULL)
    {
        fprintf(pFile, "---- %s (%u bytes) ----\n", pszTitle, static_cast<UINT>(cb));
        fwrite(pText, 1, cb, pFile);
        fputs("\n---- end ----\n", pFile);
        fclose(pFile);
    }
}

//+-----------------------------------------------------------------------------
//
//  Function:  MilDx11DumpTexture
//
//  Synopsis:  진단용. 2D 텍스처(백버퍼 등)를 스테이징으로 복사해 32bpp BMP 로.
//             저장한다. 로그 파일과 같은 폴더에 <pszName>.bmp 로 쓴다.
//
//------------------------------------------------------------------------------

void MilDx11DumpTexture(__in IUnknown *pD3DDevice, __in IUnknown *pD3DContext, __in IUnknown *pTexture, __in_z const char *pszName)
{
    if (!MilDx11LogEnabled())
    {
        return;
    }

    ID3D11Device *pDevice = NULL;
    ID3D11DeviceContext *pContext = NULL;
    ID3D11Texture2D *pSource = NULL;
    ID3D11Texture2D *pStaging = NULL;
    ID3D11Texture2D *pResolved = NULL;
    FILE *pFile = NULL;

    if (   FAILED(pD3DDevice->QueryInterface(__uuidof(ID3D11Device), reinterpret_cast<void **>(&pDevice)))
        || FAILED(pD3DContext->QueryInterface(__uuidof(ID3D11DeviceContext), reinterpret_cast<void **>(&pContext)))
        || FAILED(pTexture->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&pSource))))
    {
        goto Cleanup;
    }

    {
        D3D11_TEXTURE2D_DESC desc;
        pSource->GetDesc(&desc);

        if (desc.SampleDesc.Count > 1)
        {
            D3D11_TEXTURE2D_DESC descResolved = desc;
            descResolved.SampleDesc.Count = 1;
            descResolved.SampleDesc.Quality = 0;
            descResolved.BindFlags = D3D11_BIND_RENDER_TARGET;
            descResolved.MiscFlags = 0;

            if (FAILED(pDevice->CreateTexture2D(&descResolved, NULL, &pResolved)))
            {
                goto Cleanup;
            }

            pContext->ResolveSubresource(pResolved, 0, pSource, 0, desc.Format);
            pSource->Release();
            pSource = pResolved;
            pResolved = NULL;
        }

        D3D11_TEXTURE2D_DESC descStaging = desc;
        descStaging.SampleDesc.Count = 1;
        descStaging.SampleDesc.Quality = 0;
        descStaging.Usage = D3D11_USAGE_STAGING;
        descStaging.BindFlags = 0;
        descStaging.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        descStaging.MiscFlags = 0;
        descStaging.MipLevels = 1;
        descStaging.ArraySize = 1;

        if (FAILED(pDevice->CreateTexture2D(&descStaging, NULL, &pStaging)))
        {
            goto Cleanup;
        }

        pContext->CopySubresourceRegion(pStaging, 0, 0, 0, 0, pSource, 0, NULL);

        D3D11_MAPPED_SUBRESOURCE mapped;

        if (FAILED(pContext->Map(pStaging, 0, D3D11_MAP_READ, 0, &mapped)))
        {
            goto Cleanup;
        }

        char szPath[MAX_PATH];
        StringCchCopyA(szPath, ARRAYSIZE(szPath), s_szLogPath);

        char *pszSlash = strrchr(szPath, '\\');
        char *pszSlash2 = strrchr(szPath, '/');
        if (pszSlash2 > pszSlash) pszSlash = pszSlash2;
        if (pszSlash) { pszSlash[1] = '\0'; } else { szPath[0] = '\0'; }
        StringCchCatA(szPath, ARRAYSIZE(szPath), pszName);
        StringCchCatA(szPath, ARRAYSIZE(szPath), ".bmp");

        if (fopen_s(&pFile, szPath, "wb") == 0 && pFile != NULL)
        {
            UINT cbRow = desc.Width * 4;
            UINT cbImage = cbRow * desc.Height;

            BITMAPFILEHEADER bfh;
            ZeroMemory(&bfh, sizeof(bfh));
            bfh.bfType = 0x4D42;
            bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
            bfh.bfSize = bfh.bfOffBits + cbImage;

            BITMAPINFOHEADER bih;
            ZeroMemory(&bih, sizeof(bih));
            bih.biSize = sizeof(bih);
            bih.biWidth = static_cast<LONG>(desc.Width);
            bih.biHeight = -static_cast<LONG>(desc.Height);   // top-down
            bih.biPlanes = 1;
            bih.biBitCount = 32;
            bih.biCompression = BI_RGB;
            bih.biSizeImage = cbImage;

            fwrite(&bfh, sizeof(bfh), 1, pFile);
            fwrite(&bih, sizeof(bih), 1, pFile);

            for (UINT y = 0; y < desc.Height; y++)
            {
                fwrite(static_cast<const BYTE *>(mapped.pData) + y * mapped.RowPitch, 1, cbRow, pFile);
            }

            fclose(pFile);
            pFile = NULL;

            MilDx11Log("dumped %s (%ux%u fmt=%d)", szPath, desc.Width, desc.Height, desc.Format);
        }

        pContext->Unmap(pStaging, 0);
    }

Cleanup:
    if (pFile) fclose(pFile);
    if (pStaging) pStaging->Release();
    if (pResolved) pResolved->Release();
    if (pSource) pSource->Release();
    if (pContext) pContext->Release();
    if (pDevice) pDevice->Release();
}

//+------------------------------------------------------------------------
//  Timing probes
//-------------------------------------------------------------------------

double MilDx11QpcMs()
{
    static LARGE_INTEGER s_freq = {};
    LARGE_INTEGER now;

    if (s_freq.QuadPart == 0)
    {
        QueryPerformanceFrequency(&s_freq);
    }

    QueryPerformanceCounter(&now);
    return now.QuadPart * 1000.0 / s_freq.QuadPart;
}

void MilDx11TimingAdd(MilDx11TimingSlot slot, double ms)
{
    static const char *const s_rgNames[MilDx11Timing_Count] = { "gdi readback", "ulw", "wait to present", "vblank wait" };
    static double s_rgSum[MilDx11Timing_Count];
    static double s_rgMax[MilDx11Timing_Count];
    static LONG s_rgCount[MilDx11Timing_Count];

    if (!MilDx11LogEnabled() || slot < 0 || slot >= MilDx11Timing_Count)
    {
        return;
    }

    s_rgSum[slot] += ms;
    if (ms > s_rgMax[slot]) s_rgMax[slot] = ms;

    if (++s_rgCount[slot] % 300 == 0)
    {
        MIL_DX11_LOG("timing %s: avg=%.3f ms max=%.3f ms over 300", s_rgNames[slot], s_rgSum[slot] / 300.0, s_rgMax[slot]);
        s_rgSum[slot] = 0;
        s_rgMax[slot] = 0;
    }
}

//+------------------------------------------------------------------------
//  Per-frame work counters
//-------------------------------------------------------------------------

bool g_fMilDx11Stats = false;
static volatile LONG64 s_rgStats[MilDx11Stat_Count];

void MilDx11StatAddSlow(MilDx11Stat stat, unsigned long long n)
{
    if (stat >= 0 && stat < MilDx11Stat_Count)
    {
        InterlockedAdd64(&s_rgStats[stat], static_cast<LONG64>(n));
    }
}

void MilDx11StatsReport(unsigned int cPresents)
{
    if (!MilDx11LogEnabled() || cPresents == 0)
    {
        return;
    }

    LONG64 rg[MilDx11Stat_Count];

    for (int i = 0; i < MilDx11Stat_Count; i++)
    {
        rg[i] = InterlockedExchange64(&s_rgStats[i], 0);
    }

    MIL_DX11_LOG(
        "per present (avg of %u): draws=%.1f constantUploads=%.1f constantKB=%.1f tempBufferDraws=%.2f rtBinds=%.1f textureMaps=%.1f textureMapKB=%.1f textureMapMs=%.3f renderMs=%.3f presentMs=%.3f applyStateMs=%.3f newTex(rt/staging/data/other)=%.1f/%.1f/%.1f/%.1f",
        cPresents,
        rg[MilDx11Stat_Draws] / double(cPresents),
        rg[MilDx11Stat_ConstantUploads] / double(cPresents),
        rg[MilDx11Stat_ConstantBytes] / 1024.0 / cPresents,
        rg[MilDx11Stat_TempBufferDraws] / double(cPresents),
        rg[MilDx11Stat_RenderTargetBinds] / double(cPresents),
        rg[MilDx11Stat_TextureMaps] / double(cPresents),
        rg[MilDx11Stat_TextureMapBytes] / 1024.0 / cPresents,
        rg[MilDx11Stat_TextureMapUs] / 1000.0 / cPresents,
        rg[MilDx11Stat_RenderUs] / 1000.0 / cPresents,
        rg[MilDx11Stat_PresentUs] / 1000.0 / cPresents,
        rg[MilDx11Stat_ApplyStateUs] / 1000.0 / cPresents,
        rg[MilDx11Stat_TexNewRenderTarget] / double(cPresents),
        rg[MilDx11Stat_TexNewStaging] / double(cPresents),
        rg[MilDx11Stat_TexNewWithData] / double(cPresents),
        rg[MilDx11Stat_TexNewOther] / double(cPresents));
}
