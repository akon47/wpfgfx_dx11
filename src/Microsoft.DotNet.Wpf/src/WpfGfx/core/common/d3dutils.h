// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//
//  Description:
//      텍스처 설명(D3D11_TEXTURE2D_DESC)을 디바이스 능력에 맞게 다듬는 유틸리티.
//
//------------------------------------------------------------------------------

#pragma once

struct MilDeviceCaps;

// Flags for GetMinimalTextureDesc
#define GMTD_IGNORE_WIDTH     0x1
#define GMTD_IGNORE_HEIGHT    0x2
#define GMTD_IGNORE_FORMAT    0x4
#define GMTD_NONPOW2CONDITIONAL_OK  0x10

#define GMTD_CHECK_WIDTH      (GMTD_IGNORE_HEIGHT | \
                                         GMTD_IGNORE_FORMAT)
#define GMTD_CHECK_HEIGHT     (GMTD_IGNORE_WIDTH | \
                                         GMTD_IGNORE_FORMAT)
#define GMTD_CHECK_FORMAT     (GMTD_IGNORE_WIDTH | \
                                         GMTD_IGNORE_HEIGHT)

#define GMTD_CHECK_ALL        (GMTD_CHECK_WIDTH & \
                                         GMTD_CHECK_HEIGHT & \
                                         GMTD_CHECK_FORMAT)

#define GMTD_DEFAULT          GMTD_CHECK_ALL

enum TextureMipMapLevel
{
    TMML_All = 1,
    TMML_One = 0,
    TMML_Unknown
};

MIL_FORCEINLINE bool TextureAddressingAllowsConditionalNonPower2Usage(
    D3D11_TEXTURE_ADDRESS_MODE taU,
    D3D11_TEXTURE_ADDRESS_MODE taV
    )
{
    return ((taU == D3D11_TEXTURE_ADDRESS_CLAMP) && (taV == D3D11_TEXTURE_ADDRESS_CLAMP));
}

void PopulateSurfaceDesc(
    DXGI_FORMAT fmtPixelFormat,
    D3D11_USAGE usage,
    UINT uBindFlags,
    UINT uCpuAccessFlags,
    UINT uTextureWidth,
    UINT uTextureHeight,
    __out_ecount(1) D3D11_TEXTURE2D_DESC *pDesc
    );

HRESULT GetMinimalTextureDesc(
    __in_ecount(1) MilDeviceCaps const *pcaps,
    __inout_ecount(1) D3D11_TEXTURE2D_DESC *pDesc,
    BOOL fPalUsesAlpha,
    DWORD dwFlags
    );

bool IsSupportedTextureFormat(
    DXGI_FORMAT fmt
    );

DXGI_FORMAT GetSuperiorSurfaceFormat(
    DXGI_FORMAT fmt,
    BOOL fPalUsesAlpha
    );
