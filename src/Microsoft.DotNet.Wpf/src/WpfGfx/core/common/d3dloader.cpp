// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+----------------------------------------------------------------------------
//
//  Abstract:        Maintains the primary DXGI factory reference and the
//                   display uniqueness query.
//
//-----------------------------------------------------------------------------

#include "precomp.hpp"

//#define DdQueryDisplaySettingsUniqueness    GdiEntry13
//
extern "C" typedef ULONG
(APIENTRY
*PDdQueryDisplaySettingsUniqueness)(
    VOID
    );

PDdQueryDisplaySettingsUniqueness pDdQueryDisplaySettingsUniqueness =
                (PDdQueryDisplaySettingsUniqueness) GetProcAddress(GetModuleHandle(TEXT("Gdi32.dll")), "GdiEntry13");

//+------------------------------------------------------------------------
//
//  Function:  CD3DLoader::GetLoadRef
//
//  Synopsis:  Increase graphics load reference for this module.
//
//-------------------------------------------------------------------------
void
CD3DLoader::GetLoadRef(
    )
{
    g_DisplayManager.IncrementUseRef();
    return;
}

//+------------------------------------------------------------------------
//
//  Function:  CD3DLoader::ReleaseLoadRef
//
//  Synopsis:  Handle release of graphics load reference for this module.
//
//-------------------------------------------------------------------------
void
CD3DLoader::ReleaseLoadRef(
    )
{
    g_DisplayManager.DecrementUseRef();
}



class CD3DModuleLoaderInternal
{
public:

    CD3DModuleLoaderInternal();
    ~CD3DModuleLoaderInternal();

private:

    friend CD3DModuleLoader;

    HRESULT Init();

    HRESULT CreateD3DObjects(
        __deref_out_ecount(1) IDXGIFactory1 **ppDXGIFactory
        );

    void ReleaseD3DLoadRef();

private:

    CCriticalSection m_csManagement;

    ULONG m_cD3DRefs;
};

// This object's scope is limited to this file.  Any interaction
// with it should go through the CD3DModuleLoader class.
static CD3DModuleLoaderInternal g_D3DModuleLoader;


//+------------------------------------------------------------------------
//
//  Function:  CD3DModuleLoader::Startup
//
//  Synopsis:  Initialize global module loader
//
//-------------------------------------------------------------------------
HRESULT
CD3DModuleLoader::Startup()
{
    return g_D3DModuleLoader.Init();
}

//+------------------------------------------------------------------------
//
//  Function:  CD3DModuleLoader::Shutdown
//
//  Synopsis:  Uninitialize global module loader
//
//-------------------------------------------------------------------------
void
CD3DModuleLoader::Shutdown()
{
    Assert(g_D3DModuleLoader.m_cD3DRefs == 0);
}


//+------------------------------------------------------------------------
//
//  Member:
//      CD3DModuleLoader::CreateD3DObjects
//
//  Synopsis:
//      Creates the DXGI factory and increments the load count.
//
//-------------------------------------------------------------------------
HRESULT
CD3DModuleLoader::CreateD3DObjects(
    __deref_out_ecount(1) IDXGIFactory1 **ppDXGIFactory
    )
{
    return g_D3DModuleLoader.CreateD3DObjects(ppDXGIFactory);
}

//+------------------------------------------------------------------------
//
//  Member:
//      CD3DModuleLoader::ReleaseD3DLoadRef
//
//  Synopsis:
//      Handle release of a load reference for this module.
//
//-------------------------------------------------------------------------
void
CD3DModuleLoader::ReleaseD3DLoadRef(
    )
{
    g_D3DModuleLoader.ReleaseD3DLoadRef();
}

//+----------------------------------------------------------------------------
//
//  Function:  CD3DModuleLoader::GetDisplayUniqueness
//
//  Synopsis:  Returns the current display uniqueness value
//
//-----------------------------------------------------------------------------

ULONG
CD3DModuleLoader::GetDisplayUniqueness(
    )
{
    return pDdQueryDisplaySettingsUniqueness();
}

//+------------------------------------------------------------------------
//
//  Function:  CD3DModuleLoaderInternal::CD3DModuleLoaderInternal
//
//  Synopsis:  ctor
//
//-------------------------------------------------------------------------
CD3DModuleLoaderInternal::CD3DModuleLoaderInternal()
{
    m_cD3DRefs = 0;
}

//+------------------------------------------------------------------------
//
//  Function:  CD3DModuleLoaderInternal::~CD3DModuleLoaderInternal
//
//  Synopsis:  dtor
//
//-------------------------------------------------------------------------
CD3DModuleLoaderInternal::~CD3DModuleLoaderInternal()
{
    m_csManagement.DeInit();
}

//+------------------------------------------------------------------------
//
//  Function:  CD3DModuleLoaderInternal::Init
//
//  Synopsis:  Initialize basic members
//
//-------------------------------------------------------------------------
HRESULT
CD3DModuleLoaderInternal::Init()
{
    HRESULT hr = S_OK;

    Assert(!m_csManagement.IsValid());

    hr = m_csManagement.Init();

    Assert(m_csManagement.IsValid() || FAILED(hr));

    RRETURN(hr);
}

//+------------------------------------------------------------------------
//
//  Member:
//      CD3DModuleLoaderInternal::CreateD3DObjects
//
//  Synopsis:
//      Creates the DXGI factory and increments the load count.
//
//-------------------------------------------------------------------------
HRESULT
CD3DModuleLoaderInternal::CreateD3DObjects(
    __deref_out_ecount(1) IDXGIFactory1 **ppDXGIFactory
    )
{
    HRESULT hr = S_OK;
    IDXGIFactory1 *pFactory = NULL;

    // 팩터리 생성은 여러 DLL 로드를 유발한다. 어딘가 잘못된 코드가 FPU 설정을.
    // 깨뜨려도 우리 코드가 영향을 받지 않도록 보호한다(D3D9 시절부터의 보호).
    FPUStateSandbox oGuard;

    {
        CGuard<CCriticalSection> oGuard(m_csManagement);
        m_cD3DRefs++;
    }

    MIL_THR(CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void **>(&pFactory)));

    if (FAILED(hr))
    {
        TraceTag((tagMILWarning, "DXGI factory creation failed (hr = 0x%08x).", hr));
        IFC(WGXERR_UNSUPPORTEDVERSION);
    }

    *ppDXGIFactory = pFactory;
    pFactory = NULL;

Cleanup:

    ReleaseInterfaceNoNULL(pFactory);

    if (FAILED(hr))
    {
        CGuard<CCriticalSection> oGuard(m_csManagement);
        Assert(m_cD3DRefs > 0);
        --m_cD3DRefs;
    }

    RRETURN(hr);
}

//+------------------------------------------------------------------------
//
//  Member:
//      CD3DModuleLoaderInternal::ReleaseD3DLoadRef
//
//  Synopsis:
//      Handle release of a load reference for this module.
//
//-------------------------------------------------------------------------
void
CD3DModuleLoaderInternal::ReleaseD3DLoadRef(
    )
{
    CGuard<CCriticalSection> oGuard(m_csManagement);

    Assert(m_cD3DRefs > 0);

    --m_cD3DRefs;

    return;
}
