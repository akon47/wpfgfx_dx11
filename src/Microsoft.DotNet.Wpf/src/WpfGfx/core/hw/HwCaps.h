// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//
//  Description:
//      디바이스 능력 기술. D3D9의 D3DCAPS9를 대체한다. D3D11에서는 Feature Level이
//      능력을 결정하므로 값은 Feature Level에서 상수로 도출한다. 관리 코드로 올라가는
//      PixelShaderVersion/VertexShaderVersion은 D3DPS_VERSION 인코딩을 유지한다.
//
//------------------------------------------------------------------------------

#pragma once

struct MilDeviceCaps
{
    MilDeviceKind     DeviceKind;
    D3D_FEATURE_LEVEL FeatureLevel;

    UINT  MaxTextureWidth;
    UINT  MaxTextureHeight;
    UINT  MaxSimultaneousTextures;
    UINT  MaxAnisotropy;

    DWORD PixelShaderVersion;                 // D3DPS_VERSION(major, minor)
    DWORD VertexShaderVersion;                // D3DVS_VERSION(major, minor)
    UINT  MaxPixelShader30InstructionSlots;

    bool  SupportsSharedResources;            // D3D9의 "LDDM/WDDM" 판정에 해당
    bool  SupportsNonPow2Unconditionally;
    bool  SupportsBorderColor;
    bool  SupportsScissorRect;
    bool  SupportsAutoGenMipMap;
    bool  SupportsBlendFactor;
    bool  SupportsIndependentWriteMasks;
    bool  Supports128BitFloatTextures;
};

namespace HwCaps
{
    //+-------------------------------------------------------------------------
    //
    //  Function:  FromFeatureLevel
    //
    //  Synopsis:  Feature Level과 디바이스 종류에서 능력 표를 채운다.
    //             D5에 따라 10.0 미만은 하드웨어로 쓰지 않는다.
    //
    //--------------------------------------------------------------------------
    inline void
    FromFeatureLevel(
        D3D_FEATURE_LEVEL featureLevel,
        MilDeviceKind deviceKind,
        __out_ecount(1) MilDeviceCaps *pCaps
        )
    {
        ZeroMemory(pCaps, sizeof(*pCaps));

        pCaps->DeviceKind = deviceKind;
        pCaps->FeatureLevel = featureLevel;

        bool fLevel11 = (featureLevel >= D3D_FEATURE_LEVEL_11_0);

        pCaps->MaxTextureWidth  = fLevel11 ? D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION : D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION;
        pCaps->MaxTextureHeight = pCaps->MaxTextureWidth;
        pCaps->MaxSimultaneousTextures = 8;
        pCaps->MaxAnisotropy = 16;

        // FL10.0 이상은 ps_3_0/vs_3_0이 요구하는 모든 것을 넘어선다.
        pCaps->PixelShaderVersion = D3DPS_VERSION(3, 0);
        pCaps->VertexShaderVersion = D3DVS_VERSION(3, 0);
        pCaps->MaxPixelShader30InstructionSlots = 512;

        pCaps->SupportsSharedResources = true;
        pCaps->SupportsNonPow2Unconditionally = true;
        pCaps->SupportsBorderColor = true;
        pCaps->SupportsScissorRect = true;
        pCaps->SupportsAutoGenMipMap = true;
        pCaps->SupportsBlendFactor = true;
        pCaps->SupportsIndependentWriteMasks = true;
        pCaps->Supports128BitFloatTextures = true;
    }

    //+-------------------------------------------------------------------------
    //
    //  Function:  CheckDeviceLevel1
    //
    //  Synopsis:  하드웨어 가속에 쓸 수 있는 최소 요건(D5: Feature Level 10.0).
    //
    //--------------------------------------------------------------------------
    inline HRESULT
    CheckDeviceLevel1(
        __in_ecount(1) const MilDeviceCaps &caps
        )
    {
        return (caps.FeatureLevel >= D3D_FEATURE_LEVEL_10_0) ? S_OK : E_FAIL;
    }

    MIL_FORCEINLINE bool
    IsLDDMDevice(
        __in_ecount(1) const MilDeviceCaps &caps
        )
    {
        return caps.SupportsSharedResources;
    }

    // D3D9의 D3DDEVTYPE_SW(RGB 래스터라이저)에 해당하는 것은 WARP다.
    MIL_FORCEINLINE bool
    IsSWDevice(
        __in_ecount(1) const MilDeviceCaps &caps)
    {
        return caps.DeviceKind == MilDeviceKind_Warp;
    }

    MIL_FORCEINLINE bool
    CanMaskColorChannels(
        __in_ecount(1) const MilDeviceCaps &caps
        )
    {
        return caps.SupportsIndependentWriteMasks;
    }

    MIL_FORCEINLINE bool
    CanHandleBlendFactor(
        __in_ecount(1) const MilDeviceCaps &caps
        )
    {
        return caps.SupportsBlendFactor;
    }
}
