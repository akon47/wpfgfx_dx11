// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//
//  Description:
//      텍스처 설명(D3D11_TEXTURE2D_DESC)을 디바이스 능력에 맞게 다듬는 유틸리티.
//
//------------------------------------------------------------------------------

#include "precomp.hpp"

//+-----------------------------------------------------------------------------
//
//  Function:  PopulateSurfaceDesc
//
//  Synopsis:  2D 텍스처 설명을 채운다. 밉 1단계, 배열 1, 멀티샘플 없음.
//
//------------------------------------------------------------------------------

void
PopulateSurfaceDesc(
    DXGI_FORMAT fmtPixelFormat,
    D3D11_USAGE usage,
    UINT uBindFlags,
    UINT uCpuAccessFlags,
    UINT uTextureWidth,
    UINT uTextureHeight,
    __out_ecount(1) D3D11_TEXTURE2D_DESC *pDesc
    )
{
    Assert(uTextureWidth > 0 && uTextureHeight > 0);

    ZeroMemory(pDesc, sizeof(*pDesc));

    pDesc->Width = uTextureWidth;
    pDesc->Height = uTextureHeight;
    pDesc->MipLevels = 1;
    pDesc->ArraySize = 1;
    pDesc->Format = fmtPixelFormat;
    pDesc->SampleDesc.Count = 1;
    pDesc->SampleDesc.Quality = 0;
    pDesc->Usage = usage;
    pDesc->BindFlags = uBindFlags;
    pDesc->CPUAccessFlags = uCpuAccessFlags;
    pDesc->MiscFlags = 0;
}

//+-----------------------------------------------------------------------------
//
//  Function:  IsSupportedTextureFormat
//
//  Synopsis:  WpfGfx가 Feature Level 10.0 이상에서 텍스처로 만드는 포맷.
//             팔레트·24bpp·16bpp 소스 비트맵은 CPU에서 32bpp로 바꿔 올린다.
//
//------------------------------------------------------------------------------

bool
IsSupportedTextureFormat(
    DXGI_FORMAT fmt
    )
{
    switch (fmt)
    {
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_A8_UNORM:
    case DXGI_FORMAT_R8_UNORM:
        return true;
    default:
        return false;
    }
}

DeclareTagEx(tagTextureFixup, "MIL_HW", "Texture Create Description Fixup", FALSE /* fEnabled */);

//+-----------------------------------------------------------------------------
//
//  Function:  GetMinimalTextureDesc
//
//  Synopsis:  디바이스 한계에 맞게 크기와 포맷을 고친다. 크기를 줄였으면 S_FALSE.
//             Feature Level 10.0 이상에서는 비2의 거듭제곱 제약이 없다.
//
//------------------------------------------------------------------------------

HRESULT
GetMinimalTextureDesc(
    __in_ecount(1) MilDeviceCaps const *pcaps,     // The caps for the device
    __inout_ecount(1) D3D11_TEXTURE2D_DESC *pDesc,  // Texture description to fix up
    BOOL fPalUsesAlpha,                             // TRUE when an associated palette has alpha
    DWORD dwFlags                                   // Flags specifying which fields to check
    )
{
    HRESULT hr = S_OK;

#if DBG
    D3D11_TEXTURE2D_DESC descOrg = *pDesc;
#endif DBG

    if ((dwFlags & GMTD_IGNORE_WIDTH) == 0)
    {
        if (pDesc->Width > pcaps->MaxTextureWidth)
        {
            pDesc->Width = pcaps->MaxTextureWidth;
            hr = S_FALSE;
        }
    }

    if ((dwFlags & GMTD_IGNORE_HEIGHT) == 0)
    {
        if (pDesc->Height > pcaps->MaxTextureHeight)
        {
            pDesc->Height = pcaps->MaxTextureHeight;
            hr = S_FALSE;
        }
    }

    if ((dwFlags & GMTD_IGNORE_FORMAT) == 0)
    {
        HRESULT hrWH = hr;
        DXGI_FORMAT fmt = pDesc->Format;

        while (fmt != DXGI_FORMAT_UNKNOWN && !IsSupportedTextureFormat(fmt))
        {
            fmt = GetSuperiorSurfaceFormat(fmt, fPalUsesAlpha);
        }

        if (fmt == DXGI_FORMAT_UNKNOWN)
        {
            hr = E_FAIL;
        }
        else
        {
            pDesc->Format = fmt;
            hr = hrWH;
        }
    }

#if DBG
    if (IsTagEnabled(tagTextureFixup) &&
        SUCCEEDED(hr) &&
        (descOrg.Format != pDesc->Format ||
         descOrg.Width  != pDesc->Width ||
         descOrg.Height != pDesc->Height
       ))
    {
        TraceTag((tagTextureFixup,
                  "GetMinimalTextureDesc modified description:\n"
                  "  In:  Fmt: %lu  %lu x %lu\n"
                  " Out:  Fmt: %lu  %lu x %lu",
                  descOrg.Format, descOrg.Width, descOrg.Height,
                  pDesc->Format, pDesc->Width, pDesc->Height
                  ));
    }
#endif DBG

    return hr;
}

//+-----------------------------------------------------------------------------
//
//  Function:  GetSuperiorSurfaceFormat
//
//  Synopsis:  주어진 포맷을 표현할 수 있는 다음 상위 포맷. 더 없으면 UNKNOWN.
//
//------------------------------------------------------------------------------

DXGI_FORMAT
GetSuperiorSurfaceFormat(
    DXGI_FORMAT fmt,        // In: base surface format
    BOOL fPalUsesAlpha      // In: set to TRUE when an associated palette has alpha
    )
{
    switch (fmt)
    {
    case DXGI_FORMAT_B5G5R5A1_UNORM:
        return DXGI_FORMAT_B5G6R5_UNORM;

    case DXGI_FORMAT_B5G6R5_UNORM:
        return DXGI_FORMAT_B8G8R8X8_UNORM;

    case DXGI_FORMAT_R8_UNORM:
        return fPalUsesAlpha ? DXGI_FORMAT_B8G8R8A8_UNORM : DXGI_FORMAT_B8G8R8X8_UNORM;

    case DXGI_FORMAT_B8G8R8X8_UNORM:
        return DXGI_FORMAT_B8G8R8A8_UNORM;

    case DXGI_FORMAT_B8G8R8A8_UNORM:
    default:
        break;
    }

    return DXGI_FORMAT_UNKNOWN;
}
