// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//
//  Description:
//      Graphics acceleration tier computation. 관리 코드의 RenderCapability.Tier가.
//      이 값을 받는다. Tier 2 = 전체 하드웨어 가속. 요구 조건은 D3D9 시절과 같은.
//      의미(PS/VS 2.0, 메모리)를 유지하되 능력 값은 MilDeviceCaps에서 읽는다.
//
//------------------------------------------------------------------------------

#include "precomp.hpp"

const TierType kDefaultTier = MIL_TIER(0, 0);

struct TierRequirements
{
    TierType Tier;
    UINT MemorySize;
    bool IgnoreMemorySizeIfLDDM;
    DWORD PixelShaderVersion;
    DWORD VertexShaderVersion;
    bool (* pfnCheckSpecificCaps)(__in_ecount(1) const MilDeviceCaps &);
    bool CheckedRegistry;
};

bool CheckTier1_0SpecificCaps(__in_ecount(1) const MilDeviceCaps &caps);
bool CheckTier2_0SpecificCaps(__in_ecount(1) const MilDeviceCaps &caps);

TierRequirements g_rgTierRequirements[] = {
    {
        MIL_TIER(0, 0),
        0*1024*1024,            // 0 MB
        FALSE,                  // => LDDM doesn't override mem req
        0,                      // No Pixel Shader
        0,                      // No Vertex Shader
        NULL,
        true    // No need to check to registry overrides
    },
    {
        MIL_TIER(1, 0),
        60*1024*1024,           // 60 MB
        TRUE,                   // => LDDM does override mem req
        D3DPS_VERSION(2, 0),    // Pixel Shader 2.0
        0,                      // No Vertex shader
        CheckTier1_0SpecificCaps,
        false
    },
    {
        MIL_TIER(2, 0),
        120*1024*1024,          // 120 MB
        TRUE,                   //  => LDDM overrides mem req
        D3DPS_VERSION(2, 0),    // Pixel Shader 2.0
        D3DVS_VERSION(2, 0),    // Vertex Shader 2.0
        CheckTier2_0SpecificCaps,
        false
    },
};

//+-----------------------------------------------------------------------------
//
//  Function:  GraphicsAccelerationTier::GetTier
//
//  Synopsis:  Compute the highest tier whose requirements the device meets.
//
//------------------------------------------------------------------------------

TierType
GraphicsAccelerationTier::GetTier(
    UINT uMemorySize,
    __in_ecount(1) const MilDeviceCaps &caps
    )
{
    Assert(g_rgTierRequirements[0].Tier == 0);

    UINT uTierIndex;

    for (uTierIndex = 1;
         uTierIndex < ARRAY_SIZE(g_rgTierRequirements);
         uTierIndex++)
    {
        const TierRequirements &req = g_rgTierRequirements[uTierIndex];

        if (   (   (uMemorySize < req.MemorySize)
                && !(req.IgnoreMemorySizeIfLDDM && HwCaps::IsLDDMDevice(caps)))
            || (caps.PixelShaderVersion < req.PixelShaderVersion)
            || (caps.VertexShaderVersion < req.VertexShaderVersion)
           )
        {
            break;
        }

        if (   req.pfnCheckSpecificCaps
            && !req.pfnCheckSpecificCaps(caps))
        {
            break;
        }
    }

    return g_rgTierRequirements[uTierIndex-1].Tier;
}

bool CheckTier1_0SpecificCaps(
    __in_ecount(1) const MilDeviceCaps &caps
    )
{
    return (SUCCEEDED(HwCaps::CheckDeviceLevel1(caps)));
}

bool CheckTier2_0SpecificCaps(
    __in_ecount(1) const MilDeviceCaps &caps
    )
{
    Assert(caps.PixelShaderVersion >= D3DPS_VERSION(1,1));

    return (   (caps.MaxSimultaneousTextures >= 4)
            && (HwCaps::CanHandleBlendFactor(caps)));
}
