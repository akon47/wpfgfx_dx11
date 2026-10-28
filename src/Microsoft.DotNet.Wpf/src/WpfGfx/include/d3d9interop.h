// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

//+-----------------------------------------------------------------------------
//
//  d3d9interop.h
//
//  D3DImage(IDirect3DSurface9, D7)와 EVR/DXVA2(D16)가 요구하는 D3D9 인터페이스 선언.
//  헤더만 쓰고 d3d9.lib은 링크하지 않으며 d3d9.dll을 로드하지도 않는다(D14).
//  mild3dtypes.h의 호환 매크로는 d3d9 헤더와 철자가 같아 뒤에 포함해도 충돌하지 않는다.
//  std.h가 D3DMATRIX를 deprecated로 표시하므로 그 경고만 끈다.
//
//------------------------------------------------------------------------------

#pragma once

#pragma warning(push)
#pragma warning(disable: 4995)   // 'D3DMATRIX': name was marked as #pragma deprecated
#include <d3d9.h>
#pragma warning(pop)

//+-----------------------------------------------------------------------------
//
//  Function:  MilD3D9FormatToPixelFormat
//
//  Synopsis:  D3DImage 서피스와 D3D9 기반 미디어 버퍼의 포맷을 MIL 픽셀 포맷으로.
//             바꾼다. 업스트림 D3DFormatToPixelFormat의 D3D9 분기를 그대로 옮겼다.
//
//------------------------------------------------------------------------------

inline MilPixelFormat::Enum
MilD3D9FormatToPixelFormat(
    D3DFORMAT d3dFmt,
    BOOL fPremultiplied
    )
{
    switch (d3dFmt)
    {
    case D3DFMT_R8G8B8:
        return MilPixelFormat::BGR24bpp;
    case D3DFMT_A8R8G8B8:
        return fPremultiplied ? MilPixelFormat::PBGRA32bpp : MilPixelFormat::BGRA32bpp;
    case D3DFMT_X8R8G8B8:
        return MilPixelFormat::BGR32bpp;
    case D3DFMT_R5G6B5:
        return MilPixelFormat::BGR16bpp565;
    case D3DFMT_X1R5G5B5:
        return MilPixelFormat::BGR16bpp555;
    case D3DFMT_P8:
        return MilPixelFormat::Indexed8bpp;
    case D3DFMT_L8:
        return MilPixelFormat::Gray8bpp;
    case D3DFMT_A2R10G10B10:
        return MilPixelFormat::BGR32bpp101010;
    case D3DFMT_A32B32G32R32F:
        return fPremultiplied ? MilPixelFormat::PRGBA128bppFloat : MilPixelFormat::RGBA128bppFloat;
    default:
        return MilPixelFormat::Undefined;
    }
}
