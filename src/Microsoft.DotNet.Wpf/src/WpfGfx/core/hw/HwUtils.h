// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//
//  $TAG ENGR
//
//      $Module:    win_mil_graphics
//      $Keywords:
//
//  $Description:
//      Provides utililites. Visibile outside hw directory.
//
//  $ENDTAG
//
//------------------------------------------------------------------------------


HRESULT CacheHwTextureOnBitmap(
    __inout_ecount(1) ID3D11Texture2D *pITexture,
    __inout_ecount(1) IWGXBitmap *pBitmap,
    __inout_ecount(1) CD3DDeviceLevel1 *pDevice
    );

HRESULT ReadRenderTargetIntoSysMemBuffer(
    __in ID3D11Texture2D *pSourceTexture,
    __in const CMilRectU &rcCopy,
    MilPixelFormat::Enum fmtOut,
    UINT uStrideOut,
    DBG_ANALYSIS_PARAM_COMMA(UINT cbBufferOut)
    __out_bcount_full(cbBufferOut) BYTE *pbBufferOut
    );

bool IsD3DFailure(HRESULT hr);

//
// D3DImage 사용자의 D3D9 서피스를 시스템 메모리로 읽는다 (사용자 디바이스로
// GetRenderTargetData). InteropDeviceBitmap.cpp 에 정의되어 있다.
//
interface IDirect3DSurface9;

HRESULT ReadD3D9SurfaceIntoSysMemBuffer(
    __in IDirect3DSurface9 *pSourceSurface,
    __in const CMilRectU &rcCopy,
    MilPixelFormat::Enum fmtOut,
    UINT uStrideOut,
    DBG_ANALYSIS_PARAM_COMMA(UINT cbBufferOut)
    __out_bcount_full(cbBufferOut) BYTE *pbBufferOut
    );
