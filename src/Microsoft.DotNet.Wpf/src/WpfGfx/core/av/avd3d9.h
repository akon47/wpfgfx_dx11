// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//
//  Description:
//      미디어용 Direct3D 9Ex 디바이스 (D25).
//
//      표준 EVR 믹서와 DXVA2 는 Direct3D 9 전용이다(믹서는 IID_IDirect3DDevice9 만.
//      받는다). 그래서 비디오 디코드·믹싱은 엔진 디바이스와 같은 어댑터에 만든.
//      AV 전용 D3D9Ex 디바이스에서 돌리고, 믹서 출력 텍스처는 공유 핸들로 엔진의.
//      D3D11 디바이스에서 열어 그대로 샘플링한다(복사 없음). d3d9.dll 은 여기서만,
//      첫 재생 때 지연 로드한다. 디바이스는 CD3DDeviceLevel1 에 매달려 같이 산다.
//
//------------------------------------------------------------------------------

#pragma once

#include "mildx11log.h"

// pRenderDevice 에 미디어용 D3D9Ex 디바이스가 없으면 만든다. 실패하면.
// WGXERR_AV_VIDEOACCELERATIONNOTAVAILABLE.
HRESULT
AVEnsureMediaDevice9(
    __inout_ecount(1) CD3DDeviceLevel1 *pRenderDevice
    );

// pDevice 에 지금까지 제출된 GPU 작업이 끝날 때까지 기다린다(이벤트 쿼리).
// *ppQuery 는 호출자가 보관하는 쿼리 캐시(처음엔 NULL).
HRESULT
AVWaitForMediaDevice9(
    __in_ecount(1) IDirect3DDevice9 *pDevice,
    __deref_inout_ecount_opt(1) IDirect3DQuery9 **ppQuery
    );
