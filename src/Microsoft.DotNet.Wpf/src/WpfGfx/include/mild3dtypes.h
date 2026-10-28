// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

//+-----------------------------------------------------------------------------
//
//  mild3dtypes.h
//
//  WpfGfx는 더 이상 <d3d9.h>를 포함하지 않는다. 이 헤더는 세 가지를 제공한다.
//
//   1. d3d9types.h와 레이아웃·가드가 같은 값 타입(D3DMATRIX, D3DVECTOR, D3DRECT,
//      D3DCOLOR, D3DCOLORVALUE). DUCE 프로토콜(wgx_core_types.h, wgx_commands.h)과.
//      CBaseMatrix가 이 레이아웃을 쓰므로 이름과 배치를 유지한다. 가드 매크로를.
//      d3d9types.h와 똑같이 두어 fxjit의 사설 복사본과 같은 번역 단위에서도 충돌하지 않는다.
//   2. 상위 레이어와 관리 코드가 분기하는 HRESULT 값(D3DERR_*)과 셰이더 버전 인코딩.
//      관리 코드의 RenderCapability가 PixelShaderVersion을 이 인코딩으로 읽는다.
//   3. D3D11/DXGI 위에서 래퍼 클래스들이 공통으로 쓰는 보조 타입.
//
//------------------------------------------------------------------------------

#pragma once

#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>

//
// 1. 값 타입 (d3d9types.h 호환 레이아웃)
//

#ifndef D3DCOLOR_DEFINED
typedef DWORD D3DCOLOR;
#define D3DCOLOR_DEFINED
#endif

#ifndef D3DCOLOR_ARGB
#define D3DCOLOR_ARGB(a,r,g,b) \
    ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#define D3DCOLOR_RGBA(r,g,b,a) D3DCOLOR_ARGB(a,r,g,b)
#define D3DCOLOR_XRGB(r,g,b)   D3DCOLOR_ARGB(0xff,r,g,b)
#endif

#ifndef D3DVECTOR_DEFINED
typedef struct _D3DVECTOR {
    float x;
    float y;
    float z;
} D3DVECTOR;
#define D3DVECTOR_DEFINED
#endif

#ifndef D3DCOLORVALUE_DEFINED
typedef struct _D3DCOLORVALUE {
    float r;
    float g;
    float b;
    float a;
} D3DCOLORVALUE;
#define D3DCOLORVALUE_DEFINED
#endif

#ifndef D3DRECT_DEFINED
typedef struct _D3DRECT {
    LONG x1;
    LONG y1;
    LONG x2;
    LONG y2;
} D3DRECT;
#define D3DRECT_DEFINED
#endif

#ifndef D3DMATRIX_DEFINED
typedef struct _D3DMATRIX {
    union {
        struct {
            float        _11, _12, _13, _14;
            float        _21, _22, _23, _24;
            float        _31, _32, _33, _34;
            float        _41, _42, _43, _44;
        };
        float m[4][4];
    };
} D3DMATRIX;
#define D3DMATRIX_DEFINED
#endif

//
// 2. 셰이더 버전 인코딩과 HRESULT
//

#ifndef D3DPS_VERSION
#define D3DPS_VERSION(_Major,_Minor) (0xFFFF0000|((_Major)<<8)|(_Minor))
#endif
#ifndef D3DVS_VERSION
#define D3DVS_VERSION(_Major,_Minor) (0xFFFE0000|((_Major)<<8)|(_Minor))
#endif
#ifndef D3DSHADER_VERSION_MAJOR
#define D3DSHADER_VERSION_MAJOR(_Version) (((_Version)>>8)&0xFF)
#define D3DSHADER_VERSION_MINOR(_Version) (((_Version)>>0)&0xFF)
#endif

// d3d9.h와 같은 값. 디바이스 손실·메모리 부족 등의 분기(uce, api, sw, hw)와.
// 관리 코드로 올라가는 실패 사유가 이 값에 묶여 있다.
#ifndef _FACD3D
#define _FACD3D  0x876
#define MAKE_D3DHRESULT( code )  MAKE_HRESULT( 1, _FACD3D, code )
#define MAKE_D3DSTATUS( code )   MAKE_HRESULT( 0, _FACD3D, code )
#endif

#ifndef D3DERR_DEVICELOST
#define D3DERR_WRONGTEXTUREFORMAT       MAKE_D3DHRESULT(2072)
#define D3DERR_DRIVERINTERNALERROR      MAKE_D3DHRESULT(2087)
#define D3DERR_NOTFOUND                 MAKE_D3DHRESULT(2150)
#define D3DERR_DEVICELOST               MAKE_D3DHRESULT(2152)
#define D3DERR_DEVICENOTRESET           MAKE_D3DHRESULT(2153)
#define D3DERR_NOTAVAILABLE             MAKE_D3DHRESULT(2154)
#define D3DERR_OUTOFVIDEOMEMORY         MAKE_D3DHRESULT(380)
#define D3DERR_INVALIDCALL              MAKE_D3DHRESULT(2156)
#define D3DERR_DEVICEREMOVED            MAKE_D3DHRESULT(2160)
#define D3DERR_DEVICEHUNG               MAKE_D3DHRESULT(2164)
#define D3DERR_WASSTILLDRAWING          MAKE_D3DHRESULT(540)
#define S_PRESENT_MODE_CHANGED          MAKE_D3DSTATUS(2167)
#define S_PRESENT_OCCLUDED              MAKE_D3DSTATUS(2168)
#endif

//+-----------------------------------------------------------------------------
//
//  Function:  MilTranslateD3D11Error
//
//  Synopsis:  D3D11/DXGI가 돌려준 HRESULT를 상위 레이어가 아는 D3DERR 값으로 바꾼다.
//             성공 코드와 모르는 실패 코드는 그대로 둔다.
//
//------------------------------------------------------------------------------

inline HRESULT MilTranslateD3D11Error(HRESULT hr)
{
    switch (hr)
    {
    case DXGI_ERROR_DEVICE_REMOVED:
    case DXGI_ERROR_DEVICE_RESET:
        return D3DERR_DEVICELOST;
    case DXGI_ERROR_DEVICE_HUNG:
        return D3DERR_DEVICEHUNG;
    case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
        return D3DERR_DRIVERINTERNALERROR;
    case DXGI_ERROR_INVALID_CALL:
        return D3DERR_INVALIDCALL;
    case DXGI_ERROR_UNSUPPORTED:
    case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:
        return D3DERR_NOTAVAILABLE;
    case DXGI_ERROR_NOT_FOUND:
        return D3DERR_NOTFOUND;
    case DXGI_ERROR_WAS_STILL_DRAWING:
        return D3DERR_WASSTILLDRAWING;
    case E_OUTOFMEMORY:
        return D3DERR_OUTOFVIDEOMEMORY;
    case DXGI_STATUS_OCCLUDED:
        return S_PRESENT_OCCLUDED;
    case DXGI_STATUS_MODE_CHANGED:
        return S_PRESENT_MODE_CHANGED;
    default:
        return hr;
    }
}

//
// 3. 래퍼 공통 보조 타입.
//

// 디바이스 종류. D3D9의 D3DDEVTYPE_HAL/REF/SW에 대응한다. Software는 WpfGfx 자체.
// 소프트웨어 래스터라이저(core/sw)를 뜻하며 D3D11 디바이스가 없다.
enum MilDeviceKind
{
    MilDeviceKind_Hardware  = 0,
    MilDeviceKind_Warp      = 1,
    MilDeviceKind_Reference = 2,
    MilDeviceKind_Software  = 3,
};

// Map 결과. D3DLOCKED_RECT와 같은 모양이라 호출 코드를 거의 바꾸지 않아도 된다.
struct MilLockedRect
{
    INT   Pitch;
    void *pBits;
};

// CD3DDeviceLevel1::Clear 플래그. 철자까지 d3d9types.h와 같아서 <d3d9.h>(D3DImage·EVR interop)를.
// 뒤에 포함해도 재정의 경고가 나지 않는다.
#define D3DCLEAR_TARGET            0x00000001l
#define D3DCLEAR_ZBUFFER           0x00000002l
#define D3DCLEAR_STENCIL           0x00000004l

// LockRect 플래그 (값은 D3DLOCK_*와 같다). Map 종류로 번역된다.
#define D3DLOCK_READONLY           0x00000010L
#define D3DLOCK_DISCARD            0x00002000L
#define D3DLOCK_NOOVERWRITE        0x00001000L
#define D3DLOCK_NO_DIRTY_UPDATE     0x00008000L

// Present 플래그 (D3DPRESENT_* 대체). DXGI_PRESENT_DO_NOT_WAIT 로 번역된다.
#define D3DPRESENT_DONOTWAIT                   0x00000001L

// StretchRect 필터. D3DTEXTUREFILTERTYPE 대체.
enum MilStretchFilter
{
    MilStretchFilter_None   = 0,    // 같은 크기 복사.
    MilStretchFilter_Point  = 1,
    MilStretchFilter_Linear = 2,
};

// 스왑체인 생성 인자. D3DPRESENT_PARAMETERS 대체.
struct MilSwapChainParams
{
    HWND             hDeviceWindow;
    UINT             BackBufferWidth;
    UINT             BackBufferHeight;
    DXGI_FORMAT      BackBufferFormat;
    DXGI_SAMPLE_DESC SampleDesc;
    UINT             BackBufferCount;
    bool             GdiCompatible;      // IDXGISurface1::GetDC 가 필요한 창(투명 창)
    UINT             SyncInterval;       // 0 = 즉시, 1 = 수직 동기.
};

// 2D 텍스처 서브리소스 하나를 가리키는 "서피스" 설명. D3DSURFACE_DESC 대체.
// CD3DSurface/CD3DTexture가 D3D11_TEXTURE2D_DESC를 그대로 들고 있으므로 별도 타입은 두지 않는다.

//+-----------------------------------------------------------------------------
//
//  Function:  MilDxgiFormatBitsPerPixel
//
//  Synopsis:  WpfGfx가 쓰는 포맷의 픽셀당 바이트 수. D3DFormatSize 대체.
//
//------------------------------------------------------------------------------

inline UINT MilDxgiFormatSize(DXGI_FORMAT fmt)
{
    switch (fmt)
    {
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
        return 16;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UNORM:
        return 8;
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R16G16_UNORM:
        return 4;
    case DXGI_FORMAT_B5G6R5_UNORM:
    case DXGI_FORMAT_B5G5R5A1_UNORM:
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_R8G8_UNORM:
        return 2;
    case DXGI_FORMAT_A8_UNORM:
    case DXGI_FORMAT_R8_UNORM:
        return 1;
    default:
        return 0;
    }
}
