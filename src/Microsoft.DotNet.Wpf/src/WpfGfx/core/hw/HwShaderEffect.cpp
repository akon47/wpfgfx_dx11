// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//

//
//------------------------------------------------------------------------------

#include "precomp.hpp"
#include "HwShaderEffectTranslator.h"

MtDefine(CHwPixelShaderEffect, CD3DDeviceLevel1, "CHwPixelShaderEffect");
MtDefine(CHwPixelShaderEffect_NotImplementedAndShouldAlwaysBeZero, CHwPixelShaderEffect, "Not implemented and therefore meaningless.");


//+-----------------------------------------------------------------------------
//
//  CHwPixelShaderEffect::Create
//
//  Synopsis:
//      This class on to a D3D specific shader resource.
//
//------------------------------------------------------------------------------

HRESULT 
CHwPixelShaderEffect::Create(
    __in CD3DDeviceLevel1 *pDevice,
    __in_bcount(sizeInBytes) BYTE *pPixelShaderByteCode,
    __in UINT sizeInBytes,
    __out CHwPixelShaderEffect **ppHwPixelShaderEffect)
{
    HRESULT hr = S_OK;

    CHwPixelShaderEffect* pSE = new CHwPixelShaderEffect();
    IFCOOM(pSE);
    pSE->AddRef(); // Resources start out with ref-count 0.

    IFC(pSE->Init(pDevice, pPixelShaderByteCode, sizeInBytes));

    *ppHwPixelShaderEffect = pSE;
    pSE = NULL;

Cleanup:
    ReleaseInterface(pSE);
    
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  CHwPixelShaderEffect::Init
//
//  Synopsis:
//      Initializes the class by allocating a device specific pixel shader.
//
//------------------------------------------------------------------------------

HRESULT 
CHwPixelShaderEffect::Init(
    __in CD3DDeviceLevel1 *pDevice,
    __in_bcount(sizeInBytes) BYTE *pPixelShaderByteCode,
    __in UINT sizeInBytes)
{
    HRESULT hr = S_OK;

    //
    // The bytecode is D3D9 ps_2_0/ps_3_0 (public ShaderEffect contract, D6):
    // translate it to HLSL and compile for ps_4_0. Translation failures leave
    // the caller to its pass-through fallback.
    //
    if (sizeInBytes >= sizeof(DWORD) && *reinterpret_cast<const DWORD *>(pPixelShaderByteCode) == 0x43425844 /* 'DXBC' */)
    {
        // Already SM4 bytecode (the engine's own pass-through shader resource).
        IFC(pDevice->CreatePixelShader(pPixelShaderByteCode, sizeInBytes, &m_pD3DPixelShader));
    }
    else
    {
        std::string hlsl;

        hr = MilTranslateD3D9PixelShaderToHLSL(pPixelShaderByteCode, sizeInBytes, &hlsl);

        if (FAILED(hr))
        {
            MIL_DX11_LOG("ShaderEffect translate failed hr=0x%08x cb=%u tok0=0x%08x tok1=0x%08x",
                         hr, sizeInBytes,
                         sizeInBytes >= 4 ? *reinterpret_cast<const DWORD *>(pPixelShaderByteCode) : 0,
                         sizeInBytes >= 8 ? *reinterpret_cast<const DWORD *>(pPixelShaderByteCode + 4) : 0);
            IFC(hr);
        }

        IFC(pDevice->CompileShaderEffectPixelShader(
            hlsl.c_str(),
            static_cast<UINT>(hlsl.size()),
            &m_pD3DPixelShader
            ));
    }

    CD3DResource::Init(pDevice->GetResourceManager(), sizeInBytes /* this is a guess for the shader size, but that is ok */);

#if DBG
    m_pDbgDeviceNoRef = pDevice;
#endif

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  CHwPixelShaderEffect::SendToDevice
//
//  Synopsis:
//      Set the pixel shader on the device. The device must be the same as the 
//      one against which the pixel shader has been created. 
//------------------------------------------------------------------------------


HRESULT 
CHwPixelShaderEffect::SendToDevice(
    __in CD3DDeviceLevel1 *pDevice)
{
    HRESULT hr = S_OK;

#if DBG
    Assert(m_pDbgDeviceNoRef == pDevice);
#endif

    IFC(pDevice->SetPixelShader(m_pD3DPixelShader));

Cleanup:
    RRETURN(hr);
}



//+-----------------------------------------------------------------------------
//
//  CHwPixelShaderEffect::ReleaseD3DResources (override CD3DResource)
//
//  Synopsis:
//     Called by the device to release D3D resources associated with it on 
//     device lost, shutdown, etc.
//      
//------------------------------------------------------------------------------

void
CHwPixelShaderEffect::ReleaseD3DResources()
{
    ReleaseInterface(m_pD3DPixelShader);
}


