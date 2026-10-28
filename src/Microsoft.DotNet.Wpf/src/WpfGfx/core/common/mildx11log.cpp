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
#include <stdarg.h>
#include <stdio.h>
#include <d3d11sdklayers.h>
#include <strsafe.h>

static LONG s_lLogState = 0;            // 0 = 미확인, 1 = 꺼짐, 2 = 켜짐
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

    return lState == 2;
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
//  Synopsis:  진단용. 2D 텍스처(백버퍼 등)를 스테이징으로 복사해 32bpp BMP 로
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
