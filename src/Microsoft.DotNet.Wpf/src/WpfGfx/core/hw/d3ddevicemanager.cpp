// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//

//
//  $TAG ENGR

//      $Module:    win_mil_graphics_d3d
//      $Keywords:
//
//  $Description:
//      Implements CD3DDeviceManager which maintains a list of existing D3D11
//      devices via the CD3DDeviceLevel1 wrappers and creates new
//      CD3DDeviceLevel1's as they are needed.
//
//      Also keeps a shared WARP device for software 3D rendering.
//
//  $ENDTAG
//
//------------------------------------------------------------------------------

#include "precomp.hpp"
#include <d3d11_4.h>      // ID3D11Multithread

ExternTag(tagD3DStats);
DeclareTag(tagDisableHWGroupAdapterSupport, "MIL-HW", "Disable HW group adapter support");
DeclareTag(tagD3DDebugLayer, "MIL-HW", "Create D3D11 devices with the debug layer");

MtExtern(D3DDevice);

CD3DDeviceManager g_D3DDeviceManager;

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::Create
//
//  Synopsis:
//      Initialize global D3D device manager
//
//------------------------------------------------------------------------------
__checkReturn HRESULT
CD3DDeviceManager::Create()
{
    HRESULT hr = S_OK;

    Assert(!g_D3DDeviceManager.m_csManagement.IsValid());
    Assert(!g_D3DDeviceManager.m_fD3DLoaded);

    //
    // Call init
    //

    IFC(g_D3DDeviceManager.Init());

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::Delete
//
//  Synopsis:
//      Uninitialize global D3D device manager
//
//------------------------------------------------------------------------------
void
CD3DDeviceManager::Delete()
{
    if (g_D3DDeviceManager.m_fD3DLoaded)
    {
        ReleaseInterface(g_D3DDeviceManager.m_pDXGIFactory);
        ReleaseInterface(g_D3DDeviceManager.m_pDisplaySet);
        g_D3DDeviceManager.m_fD3DLoaded = false;
    }
    else
    {
        Assert(!g_D3DDeviceManager.m_pDXGIFactory);
        Assert(!g_D3DDeviceManager.m_pDisplaySet);
    }

    ReleaseInterface(g_D3DDeviceManager.m_pNextDisplaySet);

    return;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::Get
//
//  Synopsis:
//      Returns the global D3D device manager and increments possible callers to
//      Get and non-static methods
//
//------------------------------------------------------------------------------
CD3DDeviceManager *CD3DDeviceManager::Get()
{
    g_D3DDeviceManager.IncCallers();
    return &g_D3DDeviceManager;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::Release
//
//  Synopsis:
//      Decrements possible callers to Get and non-static methods
//
//------------------------------------------------------------------------------
void CD3DDeviceManager::Release()
{
    g_D3DDeviceManager.DecCallers();
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::NotifyDisplayChange
//
//------------------------------------------------------------------------------
void CD3DDeviceManager::NotifyDisplayChange(
    __in_ecount(1) CDisplaySet const * pOldDisplaySet,
    __in_ecount(1) CDisplaySet const * pNewDisplaySet
    )
{
    g_D3DDeviceManager.HandleDisplayChange(pOldDisplaySet, pNewDisplaySet);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::CD3DDeviceManager
//
//  Synopsis:
//      ctor
//
//------------------------------------------------------------------------------
CD3DDeviceManager::CD3DDeviceManager()
{
    m_cCallers = 0;
    m_pDXGIFactory = NULL;
    m_fD3DLoaded = false;
    m_pDisplaySet = NULL;
    m_pNextDisplaySet = NULL;
    m_iFirstUnusable = 0;
    m_pSWDevice = NULL;
#if DBG
    m_fDbgCreatingNewDevice = false;
#endif
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::~CD3DDeviceMaanger
//
//  Synopsis:
//      dtor
//
//------------------------------------------------------------------------------
CD3DDeviceManager::~CD3DDeviceManager()
{
#if DBG
    Assert(!m_fDbgCreatingNewDevice);

    INT iCount = m_rgDeviceList.GetCount();

    Assert(iCount == 0);
    Assert(m_iFirstUnusable == 0);

    for (INT i=0; i < iCount; i++)
    {
        CD3DDeviceLevel1 *pDeviceLevel1 = m_rgDeviceList[i].pDeviceLevel1;

        Assert(pDeviceLevel1);
        Assert(pDeviceLevel1->GetRefCount() == 0);

        delete pDeviceLevel1;
    }
#endif

    if (m_fD3DLoaded)
    {
        ReleaseInterfaceNoNULL(m_pSWDevice);
        ReleaseInterfaceNoNULL(m_pDXGIFactory);
    }

    ReleaseInterfaceNoNULL(m_pDisplaySet);
    ReleaseInterfaceNoNULL(m_pNextDisplaySet);

    m_csManagement.DeInit();
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::Init
//
//  Synopsis:
//      Initialize static D3D pointers and references
//
//------------------------------------------------------------------------------
HRESULT CD3DDeviceManager::Init(
    )
{
    HRESULT hr = S_OK;

    if (m_fD3DLoaded)
    {
        IFC(E_UNEXPECTED);
    }

    if (m_csManagement.IsValid())
    {
        IFC(E_UNEXPECTED);
    }

    IFC(m_csManagement.Init());

Cleanup:
    Assert(m_csManagement.IsValid() || FAILED(hr));

#pragma prefast(suppress: 15, "m_csManagement is deleted in destructor")
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::CreateD3D11Device
//
//  Synopsis:
//      D3D11CreateDevice 래퍼. feature level 11.1 부터 10.0 까지 내려가며 시도한다.
//      11.1 을 모르는 런타임은 E_INVALIDARG 를 돌려주므로 그 경우 11.0 부터 다시.
//      시도한다. (D5)
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceManager::CreateD3D11Device(
    __in_ecount_opt(1) IDXGIAdapter1 *pAdapter,
    MilDeviceKind type,
    DWORD dwBehaviorFlags,
    __deref_out_ecount(1) ID3D11Device **ppDevice,
    __deref_out_ecount(1) ID3D11DeviceContext **ppContext,
    __out_ecount(1) D3D_FEATURE_LEVEL *pFeatureLevel
    )
{
    HRESULT hr = S_OK;

    static const D3D_FEATURE_LEVEL rgLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    *ppDevice = NULL;
    *ppContext = NULL;
    *pFeatureLevel = D3D_FEATURE_LEVEL_9_1;

    UINT uFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    if (!(dwBehaviorFlags & MIL_DEVICE_BEHAVIOR_MULTITHREADED))
    {
        uFlags |= D3D11_CREATE_DEVICE_SINGLETHREADED;
    }

#if DBG
    if (IsTagEnabled(tagD3DDebugLayer))
    {
        uFlags |= D3D11_CREATE_DEVICE_DEBUG;
    }
#endif

    // 진단 로그가 켜져 있으면 디버그 레이어의 검증 메시지도 로그로 받는다.
    if (MilDx11LogEnabled())
    {
        uFlags |= D3D11_CREATE_DEVICE_DEBUG;
    }

    D3D_DRIVER_TYPE driverType;

    switch (type)
    {
    case MilDeviceKind_Hardware:
        // An explicit adapter requires D3D_DRIVER_TYPE_UNKNOWN
        driverType = pAdapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE;
        break;

    case MilDeviceKind_Warp:
    case MilDeviceKind_Software:
        driverType = D3D_DRIVER_TYPE_WARP;
        pAdapter = NULL;
        break;

    case MilDeviceKind_Reference:
        driverType = D3D_DRIVER_TYPE_REFERENCE;
        pAdapter = NULL;
        break;

    default:
        IFC(E_INVALIDARG);
    }

    {
        FPUStateSandbox oGuard;

        MIL_THR(D3D11CreateDevice(
            pAdapter,
            driverType,
            NULL,
            uFlags,
            rgLevels,
            ARRAYSIZE(rgLevels),
            D3D11_SDK_VERSION,
            ppDevice,
            pFeatureLevel,
            ppContext
            ));

        if (hr == DXGI_ERROR_SDK_COMPONENT_MISSING && (uFlags & D3D11_CREATE_DEVICE_DEBUG))
        {
            // SDK layers not installed: retry without the debug layer
            uFlags &= ~D3D11_CREATE_DEVICE_DEBUG;

            MIL_THR(D3D11CreateDevice(
                pAdapter,
                driverType,
                NULL,
                uFlags,
                rgLevels,
                ARRAYSIZE(rgLevels),
                D3D11_SDK_VERSION,
                ppDevice,
                pFeatureLevel,
                ppContext
                ));
        }

        if (hr == E_INVALIDARG)
        {
            // Runtime without 11.1 support: retry from 11.0
            MIL_THR(D3D11CreateDevice(
                pAdapter,
                driverType,
                NULL,
                uFlags,
                rgLevels + 1,
                ARRAYSIZE(rgLevels) - 1,
                D3D11_SDK_VERSION,
                ppDevice,
                pFeatureLevel,
                ppContext
                ));
        }

#if DBG
        if (FAILED(hr) && (uFlags & D3D11_CREATE_DEVICE_DEBUG))
        {
            // Debug layer not installed: retry without it
            uFlags &= ~D3D11_CREATE_DEVICE_DEBUG;

            MIL_THR(D3D11CreateDevice(
                pAdapter,
                driverType,
                NULL,
                uFlags,
                rgLevels + 1,
                ARRAYSIZE(rgLevels) - 1,
                D3D11_SDK_VERSION,
                ppDevice,
                pFeatureLevel,
                ppContext
                ));
        }
#endif
    }

    MIL_DX11_LOG("D3D11CreateDevice hr=0x%08x driverType=%d featureLevel=0x%x flags=0x%x", hr, driverType, *pFeatureLevel, uFlags);

    if (FAILED(hr))
    {
        hr = MilTranslateD3D11Error(hr);
        goto Cleanup;
    }

    //
    // The immediate context is also used from the UI thread (GPU
    // RenderTargetBitmap: render through the sync channel, read back through
    // CDeviceBitmap::Lock) and by anyone holding the device from
    // WpfGfxDx11_GetDevice. Like D3DCREATE_MULTITHREADED on the D3D9 device,
    // let the runtime serialize context calls; the engine's own device lock
    // stays on top of it.
    //
    {
        ID3D11Multithread *pMultithread = NULL;
        if (SUCCEEDED((*ppContext)->QueryInterface(__uuidof(ID3D11Multithread), reinterpret_cast<void **>(&pMultithread))))
        {
            pMultithread->SetMultithreadProtected(TRUE);
            pMultithread->Release();
        }
    }

    if (*pFeatureLevel < D3D_FEATURE_LEVEL_10_0)
    {
        IFC(WGXERR_INSUFFICIENT_GPU_CAPS);
    }

Cleanup:
    if (FAILED(hr))
    {
        ReleaseInterface(*ppContext);
        ReleaseInterface(*ppDevice);
    }

    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::GetSWDevice
//
//  Synopsis:
//      Creates and holds onto a single WARP device (replaces D3DDEVTYPE_SW).
//
//  Return Value:
//      S_OK if it was successfully able to use an existing or create a new
//           software device.
//
//------------------------------------------------------------------------------
HRESULT CD3DDeviceManager::GetSWDevice(
    __deref_out_ecount(1) CD3DDeviceLevel1 **ppDevice
    )
{
    HRESULT hr = S_OK;

    *ppDevice = NULL;

    // Start a critical section to make sure we only create one device.

    CGuard<CCriticalSection> oGuard(m_csManagement);

    ID3D11Device *pID3DDevice = NULL;
    ID3D11DeviceContext *pID3DContext = NULL;
    CD3DDeviceLevel1 *pDeviceLevel1 = NULL;

    if (m_pDXGIFactory == NULL)
    {
        //
        // Initialize d3d references
        //

        IFC(InitializeD3DReferences(NULL));
    }

    Assert(m_pDXGIFactory);
    Assert(m_pDisplaySet);
    Assert(m_pDisplaySet->DXGIFactory() == m_pDXGIFactory);

    //
    // Return our existing WARP device or create one
    //

    if (m_pSWDevice == NULL)
    {
        D3D_FEATURE_LEVEL featureLevel;

        DWORD dwBehaviorFlags = MIL_DEVICE_BEHAVIOR_MULTITHREADED;

        IFC(CreateD3D11Device(
            NULL,
            MilDeviceKind_Warp,
            dwBehaviorFlags,
            &pID3DDevice,
            &pID3DContext,
            &featureLevel
            ));

        IFC(CD3DDeviceLevel1::Create(
            pID3DDevice,
            pID3DContext,
            featureLevel,
            MilDeviceKind_Warp,
            m_pDisplaySet->Display(0),
            this,
            dwBehaviorFlags,
            &pDeviceLevel1
            ));

        // All tracked devices are potential non-static method callers
        m_cCallers++;

        // We hold a pointer to the m_pSWDevice without an AddRef (like
        // the other managed devices.)
        m_pSWDevice = pDeviceLevel1;
    }

    m_pSWDevice->AddRef();
    *ppDevice = m_pSWDevice;

Cleanup:
    if (hr == D3DERR_DEVICELOST)
    {
        hr = WGXERR_DISPLAYSTATEINVALID;
    }

    ReleaseInterfaceNoNULL(pID3DContext);
    ReleaseInterfaceNoNULL(pID3DDevice);
    ReleaseInterfaceNoNULL(pDeviceLevel1);

    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::IncCallers
//
//------------------------------------------------------------------------------
void CD3DDeviceManager::IncCallers()
{
    CGuard<CCriticalSection> oGuard(m_csManagement);

    m_cCallers++;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::DecCallers
//
//------------------------------------------------------------------------------
void CD3DDeviceManager::DecCallers()
{
    CGuard<CCriticalSection> oGuard(m_csManagement);

    Assert(m_cCallers > 0);

    --m_cCallers;

    if (m_cCallers == 0)
    {
        // Managed devices (including the SW device) are callers so
        // there should be none now.
        Assert(m_rgDeviceList.GetCount() == 0);
        Assert(!m_pSWDevice);

        if (m_fD3DLoaded)
        {
            if (m_pDXGIFactory)
            {
                CD3DRegistryDatabase::Cleanup();
                m_pDXGIFactory->Release();
                m_pDXGIFactory = NULL;
            }
            ReleaseInterface(m_pDisplaySet);
            m_fD3DLoaded = false;
        }

        ReleaseInterface(m_pNextDisplaySet);

        Assert(!m_pDXGIFactory);
        Assert(!m_pDisplaySet);
    }
}


//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::InitializeD3DReferences
//
//  Synopsis:
//      Initialize static D3D pointers and references
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceManager::InitializeD3DReferences(
    __in_ecount_opt(1) CDisplaySet const *pGivenDisplaySet
    )
{
    HRESULT hr = S_OK;

    CDisplaySet const *pDisplaySet = NULL;
    IDXGIFactory1 *pDXGIFactoryNoRef = NULL;

    IFC(g_DisplayManager.DangerousGetLatestDisplaySet(&pDisplaySet));

    Assert(pDisplaySet);

    if (pGivenDisplaySet && pGivenDisplaySet != pDisplaySet)
    {
        //
        // Caller has supplied pGivenDisplaySet that's obsolete.
        // We should not use it because we need not unworkable device,
        // and we should not use newest display set because it'll
        // cause mismatches with the caller that relies upon pGivenDisplaySet
        // and expects us to use it.
        // Just fail.
        //

        IFC(WGXERR_DISPLAYSTATEINVALID);
    }

    //
    // Make sure the DXGI factory is available
    //

    IFC(pDisplaySet->GetDXGIFactoryNoRef(&pDXGIFactoryNoRef));

    Assert(pDXGIFactoryNoRef);

    Assert(m_cCallers > 0);

    Assert(m_fD3DLoaded || (m_rgDeviceList.GetCount() == 0));

    //
    // Check if there is a new factory that we should be using
    //

    if (m_pDXGIFactory != pDXGIFactoryNoRef)
    {
        //
        // If there was a prior factory (and the new one is different) then
        // there must have been a mode change or other event to invalidate the
        // old one.  All old devices would now be unusable.
        // Every mode change is accompanied with HandleDisplayChange() call
        // that should release m_pDXGIFactory and m_pDisplaySet.
        //

        Assert(!m_pDXGIFactory);
        Assert(!m_pDisplaySet);

        //
        // Initialize registry
        //

        IFC(CD3DRegistryDatabase::InitializeFromRegistry(pDisplaySet->GetDisplayCount()));

        //
        // Save factory reference
        //

        m_pDXGIFactory = pDXGIFactoryNoRef;
        m_pDXGIFactory->AddRef();
        m_fD3DLoaded = true;

        //
        // Save DisplaySet reference
        //

        m_pDisplaySet = pDisplaySet; // Transfer reference
        pDisplaySet = NULL;
    }
    else
    {
        Assert(m_fD3DLoaded);
        Assert(m_pDisplaySet == pDisplaySet);
    }

    //
    // Now that we've settle onto current display set we can
    // release m_pNextDisplaySet that was only used to protect
    // against d3d module unloading.
    //

    ReleaseInterface(m_pNextDisplaySet);

Cleanup:
    ReleaseInterfaceNoNULL(pDisplaySet);
    RRETURN(hr);
}


//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::HandleDisplayChange
//
//  Synopsis:
//      Mark all D3D devices as unusable and release static D3D pointers and
//      references.
//
//------------------------------------------------------------------------------
void
CD3DDeviceManager::HandleDisplayChange(
    __in_ecount(1) CDisplaySet const * pOldDisplaySet,
    __in_ecount(1) CDisplaySet const * pNewDisplaySet
    )
{
    CGuard<CCriticalSection> oGuard(m_csManagement);

    //
    // Check whether we hold on display set that became obsolete;
    // if not so then we don't care.
    //
    if (pOldDisplaySet == m_pDisplaySet)
    {
        // Mark all usable D3D devices as unusable
        while (m_iFirstUnusable > 0)
        {
#if DBG
            UINT dbgOldFirstUnusable = m_iFirstUnusable;
#endif
            CD3DDeviceLevel1 *pDevice = m_rgDeviceList[m_iFirstUnusable - 1].pDeviceLevel1;
            ENTER_DEVICE_FOR_SCOPE(*pDevice);
            pDevice->MarkUnusable(false /* This call is entry protected by above Enter */);
#if DBG
            // We're depending upon MarkUnusable decrementing this number to avoid
            // an infinite loop.
            Assert(m_iFirstUnusable == dbgOldFirstUnusable - 1);
#endif
        }

        if (m_pSWDevice)
        {
            m_pSWDevice->MarkUnusable(TRUE);
        }

        if (m_fD3DLoaded)
        {
            Assert(m_cCallers > 0);

            if (m_pDXGIFactory)
            {
                CD3DRegistryDatabase::Cleanup();
                m_pDXGIFactory->Release();
                m_pDXGIFactory = NULL;
            }

            //
            // Do not release the load reference on D3D at this point
            // as there may still be devices using D3D.  Note: if m_cCallers
            // is 1 and the device count is 0, there is probably a factory
            // holding it normal reference.
            //
        }
        else
        {
            Assert(!m_pDXGIFactory);
        }

        ReleaseInterface(m_pDisplaySet);

        //
        // Hold on pNewDisplaySet to avoid releasing d3d module.
        //

        ReplaceInterface(m_pNextDisplaySet, pNewDisplaySet);
    }
}

//+-----------------------------------------------------------------------------
//
//  Function:
//      ChooseTargetFormat
//
//  Synopsis:
//      Selects render target format based on RT Init Flags
//
//------------------------------------------------------------------------------
void
ChooseTargetFormat(
    MilRTInitialization::Flags dwFlags,  // Presentation options
    __out_ecount(1) DXGI_FORMAT *pFmt    // The format of the target
    )
{
    if (dwFlags & MilRTInitialization::NeedDestinationAlpha)
    {
        *pFmt = DXGI_FORMAT_B8G8R8A8_UNORM;
    }
    else
    {
        *pFmt = DXGI_FORMAT_B8G8R8X8_UNORM;
    }
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::GetDisplayMode
//
//  Synopsis:
//      Get display mode for adapter
//
//------------------------------------------------------------------------------

HRESULT
CD3DDeviceManager::GetDisplayMode(
    __inout_ecount(1) D3DDeviceCreationParameters *pCreateParams,
    __out_xcount_full(pCreateParams->NumberOfAdaptersInGroup) MilDisplayMode *rgDisplayModes
    ) const
{
    HRESULT hr = S_OK;

    // Since we do not support fullscreen, we cannot create adapter groups.
    Assert(pCreateParams->NumberOfAdaptersInGroup == 1);

    if (pCreateParams->AdapterOrdinal >= m_pDisplaySet->GetDisplayCount())
    {
        IFC(WGXERR_NO_HARDWARE_DEVICE);
    }

    IFC(m_pDisplaySet->Display(pCreateParams->AdapterOrdinal)->GetMode(rgDisplayModes));

    // Every feature level 10 device supports the B8G8R8A8/X8 target formats;
    // there is nothing to check against the display mode here.

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::DoesWindowedHwDeviceExist
//
//  Synopsis:
//      Returns 'true' if a non-fullscreen, hardware device exists on the given
//      context and adapter id.
//
//------------------------------------------------------------------------------

bool
CD3DDeviceManager::DoesWindowedHwDeviceExist(
    UINT uAdapter
    )
{
    D3DDeviceCreationParameters oCreateParams;
    bool succeeded = false;
    HRESULT hr = S_OK;

    CGuard<CCriticalSection> oGuard(m_csManagement);

    IFC(InitializeD3DReferences(NULL));

    IFC(ComposeCreateParameters(
        GetDesktopWindow(),           // hwnd doesn't matter for non-fullscreen
        MilRTInitialization::Default,
        uAdapter,
        MilDeviceKind_Hardware,
        &oCreateParams
        ));

    IFC(FindDeviceMatch(
        &oCreateParams,
        0,                  // uStartIndex
        m_iFirstUnusable,   // uLimitPlusOne
        NULL                // ppDevice
        ));

    succeeded = true;

Cleanup:
    return succeeded;
}


//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::FindDeviceMatch
//
//  Synopsis:
//      Finds an existing CD3DDeviceLevel1 object that can satisfy the settings
//      given, with the list range given.
//
//  Return Value:
//      S_OK if an acceptable state manager was located
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceManager::FindDeviceMatch(
    __inout_ecount(1) D3DDeviceCreationParameters *pCreateParams,
    UINT uStartIndex,
    UINT uLimitPlusOne,
    __deref_opt_out_ecount(1) CD3DDeviceLevel1 **ppDeviceLevel1
    ) const
{
    HRESULT hr = E_FAIL;

    for (UINT i = uStartIndex; i < uLimitPlusOne; i++)
    {
        D3DDeviceInformation const &oDevInfo = m_rgDeviceList[i];

        Assert(oDevInfo.pDeviceLevel1);

        if (   (pCreateParams->DeviceType ==
                oDevInfo.CreateParams.DeviceType)
            && (   // nothing else matters for SW type since we share a device
                   (pCreateParams->DeviceType == MilDeviceKind_Warp)
                || (   (pCreateParams->AdapterOrdinal ==
                        oDevInfo.CreateParams.AdapterOrdinal)
                    && (pCreateParams->BehaviorFlags == oDevInfo.CreateParams.BehaviorFlags)
                   )
               )
           )
        {
            if (ppDeviceLevel1)
            {
                CD3DDeviceLevel1 *pDeviceLevel1 =
                    oDevInfo.pDeviceLevel1;

                Assert(pDeviceLevel1);

                pDeviceLevel1->AddRef();
                *ppDeviceLevel1 = pDeviceLevel1;

                // Update behavior flags
                pCreateParams->BehaviorFlags = oDevInfo.CreateParams.BehaviorFlags;
            }

            hr = S_OK;

            break;
        }
    }

    // No RRETURN because we don't want spew or capture here.
    return hr;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::GetAvaliableDevice
//
//  Synopsis:
//      Finds an available CD3DDeviceLevel1 object that satisfies the settings
//      given.
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceManager::GetAvailableDevice(
    __inout_ecount(1) D3DDeviceCreationParameters *pCreateParams,
    __deref_out_ecount(1) CD3DDeviceLevel1 **ppDeviceLevel1
    ) const
{
    HRESULT hr = S_OK;

    hr = FindDeviceMatch(
        pCreateParams,
        0,
        m_iFirstUnusable,
        ppDeviceLevel1
        );

#if DBG
    if (FAILED(hr))
    {
        if (SUCCEEDED(FindDeviceMatch(
            pCreateParams,
            m_iFirstUnusable,
            m_rgDeviceList.GetCount(),
            ppDeviceLevel1
            )))
        {
            ReleaseInterface(*ppDeviceLevel1);  // Get rid of useless reference
            TraceTag((tagMILWarning,
                      "A new D3D device will be created before its"
                      " matching predecessor will be completely freed."
                      ));
        }
    }
#endif DBG

    // No RRETURN because we don't want spew or capture here.
    return hr;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::GetD3DDeviceAndPresentParams
//
//  Synopsis:
//      Finds or creates a CD3DDeviceLevel1 object that can satisfy the settings
//      it was given.
//
//  Return Value:
//      S_OK if an acceptable state manager was located
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceManager::GetD3DDeviceAndPresentParams(
    __in_opt HWND hwnd,                                         // destination window

    MilRTInitialization::Flags dwFlags,                         // Presentation/
                                                                // fallback options

    __in_ecount_opt(1) CDisplay const *pDisplay,                // Targeted display -
                                                                // can be NULL for SW

    MilDeviceKind type,                                         // device kind

    __deref_out_ecount(1) CD3DDeviceLevel1 **ppDeviceLevel1,    // recieves the located
                                                                // D3D device

    __out_ecount_opt(1) MilSwapChainParams *pPresentParams,     // receives
                                                                // presentation
                                                                // parameters that
                                                                // should be used to
                                                                // create a swap chain

    __out_ecount_opt(1) UINT *pAdapterOrdinalInGroup
    )
{
    HRESULT hr = S_OK;

    Assert(m_cCallers > 0);

    *ppDeviceLevel1 = NULL;

    CGuard<CCriticalSection> oGuard(m_csManagement);

    D3DDeviceCreationParameters CreateParams;

    //
    // Ensure we have an adapter index to work with
    //

    UINT uAdapter = 0;

    if (pDisplay)
    {
        uAdapter = pDisplay->GetDisplayIndex();
    }
    else
    {
        if (type != MilDeviceKind_Warp)
        {
            IFC(WGXERR_INVALIDPARAMETER);
        }

        uAdapter = 0;
    }

    IFC(InitializeD3DReferences(pDisplay ? pDisplay->DisplaySet() : NULL));

    // Check our registry database to see if we are allowed to create the a
    // d3d device on this adapter

    if (type == MilDeviceKind_Hardware)
    {
        if (uAdapter >= m_pDisplaySet->GetDisplayCount())
        {
            IFC(WGXERR_NO_HARDWARE_DEVICE);
        }

        if (!m_pDisplaySet->Display(uAdapter)->HasDXGIAdapter())
        {
            TRACE_DEVICECREATE_FAILURE(uAdapter, "No DXGI adapter for this display", WGXERR_NO_HARDWARE_DEVICE);
            IFC(WGXERR_NO_HARDWARE_DEVICE);
        }

        //
        // Check for adapter/driver disables.
        //

        bool fEnabled;

        IFC(CD3DRegistryDatabase::IsAdapterEnabled(uAdapter, &fEnabled));

        if (!fEnabled)
        {
            TRACE_DEVICECREATE_FAILURE(
                    uAdapter,
                    "Registry settings disabled hw acceleration "
                    "(see HKEY_CURRENT_USER\\Software\\Microsoft\\Avalon.Graphics)", hr
                    );
            IFC(WGXERR_NO_HARDWARE_DEVICE);
        }
    }

    IFC(ComposeCreateParameters(
        hwnd,
        dwFlags,
        uAdapter,
        type,
        &CreateParams
        ));

    {
        //
        // Get display mode(s) and finalize group support
        //

        MilDisplayMode rgDisplayModes[1];

        IFC(GetDisplayMode(
            &CreateParams,
            rgDisplayModes
            ));

        //
        // Try to find an existing device
        //

        ASSIGN_HR(hr, GetAvailableDevice(
            &CreateParams,
            ppDeviceLevel1
            ));

        //
        // Write present parameters, for specified adapter
        //

        MilSwapChainParams oPresentParameters;

        if (!pPresentParams)
        {
            pPresentParams = &oPresentParameters;
        }

        ComposePresentParameters(
            rgDisplayModes[CreateParams.AdapterOrdinalInGroup],
            CreateParams,
            pPresentParams
            );

        if (FAILED(hr))
        {
            //
            // Create new device
            //

            IFC(CreateNewDevice(
                &CreateParams,
                pPresentParams,
                rgDisplayModes,
                ppDeviceLevel1
                ));
        }
    }

    if (pAdapterOrdinalInGroup)
    {
        *pAdapterOrdinalInGroup = CreateParams.AdapterOrdinalInGroup;
    }

Cleanup:

    //
    // If the mode has changed at this point, independent of success or failure
    // so far, release any device and return failure.
    //

    if (m_pDisplaySet && m_pDisplaySet->DangerousHasDisplayStateChanged())
    {
        ReleaseInterface(*ppDeviceLevel1);
        MIL_THR(WGXERR_DISPLAYSTATEINVALID);
    }

    if (FAILED(hr))
    {
        MIL_DX11_LOG("GetD3DDeviceAndPresentParams failed hr=0x%08x adapter=%u kind=%d flags=0x%x", hr, uAdapter, type, dwFlags);
    }

    RRETURN(hr);
}


//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::UnusedNotification
//
//  Synopsis:
//      Receives notification that one of its managed objects is no longer in
//      use.  The object may be tracked and saved for later use or
//      UnusedNotification should delete the object.
//
//------------------------------------------------------------------------------
void
CD3DDeviceManager::UnusedNotification(
    __inout_ecount(1) CMILPoolResource *pUnused // pointer to no longer used CD3DDeviceLevel1
    )
{
    if (m_fD3DLoaded)
    {
        CGuard<CCriticalSection> oGuard(m_csManagement);

        CD3DDeviceLevel1 *pDeviceLevel1 = DYNCAST(CD3DDeviceLevel1, pUnused);

        // Is pUnused the SW device?
        if (m_pSWDevice == pDeviceLevel1 && pDeviceLevel1->GetRefCount() == 0)
        {
            delete pDeviceLevel1;
            m_pSWDevice = NULL;

            DecCallers();
        }
        else
        {
            // If not look for it in the list of HW devices.
            UINT uCount = m_rgDeviceList.GetCount();

            for (UINT i = 0; i < uCount; i++)
            {
                if (m_rgDeviceList[i].pDeviceLevel1 == pDeviceLevel1)
                {
                    //
                    // Make sure this object hasn't since been handed
                    //  back out since its last Release.
                    //

                    if (pDeviceLevel1->GetRefCount() != 0)
                    {
                        break;
                    }

                    //
                    // Object destruction must be done before DecCallers so
                    // that the device may clean up all of its D3D resources
                    // before D3D is potentially unloaded.
                    //

                    NotifyDeviceLost(m_rgDeviceList[i]);
                    delete pDeviceLevel1;

                    //
                    // A match was found so remove it by replacing it
                    // with the last element in the list (if it isn't
                    // already) and then shrinking the list size by 1.
                    //

                    UINT iLast = uCount - 1;

                    if (i < m_iFirstUnusable)
                    {
                        // Move up the last usable
                        m_rgDeviceList[i] = m_rgDeviceList[--m_iFirstUnusable];
                        // Move up the last unusable into the first unusable
                        m_rgDeviceList[m_iFirstUnusable] = m_rgDeviceList[iLast];
                    }
                    else
                    {
                        // Just move up the last unusable
                        m_rgDeviceList[i] = m_rgDeviceList[iLast];
                    }

                    m_rgDeviceList.SetCount(iLast);

                    //
                    // When all of the entries have been freed, shrink
                    // to free all dynamic memory, since the debug
                    // memory trackers don't appreciate being called
                    // at process detatch.
                    //

                    if (iLast == 0)
                    {
                        m_rgDeviceList.ShrinkToSize();
                    }

                    // Remove this previously tracked device as a caller
                    DecCallers();

                    break;
                }
            }
        }
    }
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::UnusableNotification
//
//  Synopsis:
//      Receives notification that one of its managed objects is no longer
//      usable.  The object may not be handed out anymore, but needs to still be
//      tracked until it is know to have deleted all of its D3D resources.
//
//------------------------------------------------------------------------------
void
CD3DDeviceManager::UnusableNotification(
    __inout_ecount(1) CMILPoolResource *pUnusable // pointer to lost CD3DDeviceLevel1
    )
{
    Assert(m_fD3DLoaded);

    CGuard<CCriticalSection> oGuard(m_csManagement);

    const CD3DDeviceLevel1 *pDeviceLevel1 = DYNCAST(CD3DDeviceLevel1, pUnusable);

    //
    // Find entry and move to the unusable section of the list
    //

#if DBG
    bool fFound = false;
#endif

    for (UINT i = 0; i < m_iFirstUnusable; i++)
    {
        if (m_rgDeviceList[i].pDeviceLevel1 == pDeviceLevel1)
        {
            NotifyDeviceLost(m_rgDeviceList[i]);

            m_iFirstUnusable--;
            if (i != m_iFirstUnusable)
            {
                D3DDeviceInformation temp = m_rgDeviceList[i];
                // Move last usable into newly unusable's place
                m_rgDeviceList[i] = m_rgDeviceList[m_iFirstUnusable];
                // Move newly unusable into first unusable position
                m_rgDeviceList[m_iFirstUnusable] = temp;
            }

#if DBG
            fFound = true;
#endif
            break;
        }
    }

#if DBG
    //
    // Normally the device should be found in the usable list,
    // but if a mode change has happened then it might have been
    // moved to the unusable section; so, check there too.
    //
    // But only check if we are not currently creating a new device.  During
    // creation the device won't be in the list until after some tests.
    //
    // The SW device is not in the device list so ignore it.
    //

    if (!fFound && !m_fDbgCreatingNewDevice && pUnusable != m_pSWDevice)
    {
        UINT uTotal = m_rgDeviceList.GetCount();

        for (UINT i = m_iFirstUnusable; i < uTotal; i++)
        {
            if (m_rgDeviceList[i].pDeviceLevel1 == pDeviceLevel1)
            {
                TraceTag((tagMILWarning,
                          "Device was lost upon Present after mode change."));
                fFound = true;
                break;
            }
        }

        Assert(fFound);
    }
#endif
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::ComposeCreateParameters
//
//  Synopsis:
//      Takes MILRender's Initialize/Presentation parameters and translates them
//      into device creation parameters.
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceManager::ComposeCreateParameters(
    __in_opt HWND hwnd,                             // destination window
    MilRTInitialization::Flags dwFlags,             // RT/device options
    UINT uAdapter,                                  // display index
    MilDeviceKind type,                             // device kind
    __out_ecount(1) D3DDeviceCreationParameters *pD3DCreateParams
    ) const
{
    HRESULT hr = S_OK;

    Assert(m_fD3DLoaded);
    Assert(m_pDXGIFactory);

    // Make sure the window is valid if the hwnd is not NULL
    if (hwnd && !IsWindow(hwnd))
    {
        IFC(__HRESULT_FROM_WIN32(ERROR_INVALID_WINDOW_HANDLE));
    }

    // Set Creation Parameters
    pD3DCreateParams->AdapterOrdinal = uAdapter;
    pD3DCreateParams->DeviceType = type;
    pD3DCreateParams->hFocusWindow = hwnd;
    pD3DCreateParams->BehaviorFlags = 0;

    // We are multithreaded unless MilRTInitialization::SingleThreadedUsage is specified
    if ((dwFlags & MilRTInitialization::SingleThreadedUsage) == 0)
    {
        pD3DCreateParams->BehaviorFlags |= MIL_DEVICE_BEHAVIOR_MULTITHREADED;
    }

    pD3DCreateParams->MasterAdapterOrdinal = uAdapter;
    pD3DCreateParams->AdapterOrdinalInGroup = 0;
    pD3DCreateParams->NumberOfAdaptersInGroup = 1;

    pD3DCreateParams->RTInitFlags = dwFlags;

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::ComposePresentParameters
//
//  Synopsis:
//      Takes MILCore's Initialize/Presentation parameters and translates them
//      into swap chain parameters.
//
//------------------------------------------------------------------------------
void
CD3DDeviceManager::ComposePresentParameters(
    __in_ecount(1) MilDisplayMode const &displayMode,
    __in_ecount(1) D3DDeviceCreationParameters const &CreateParams,
    __out_ecount(1) MilSwapChainParams *pPresentParams
    )
{
    UNREFERENCED_PARAMETER(displayMode);

    ZeroMemory(pPresentParams, sizeof(*pPresentParams));

    // The caller sets the real size before creating a swap chain.
    pPresentParams->BackBufferWidth = 1;
    pPresentParams->BackBufferHeight = 1;

    ChooseTargetFormat(
        CreateParams.RTInitFlags,
        OUT &(pPresentParams->BackBufferFormat)
        );

    pPresentParams->SampleDesc.Count = 1;
    pPresentParams->SampleDesc.Quality = 0;

    pPresentParams->BackBufferCount = 1;
    pPresentParams->hDeviceWindow = CreateParams.hFocusWindow;

    //
    // GDI presentation (layered windows, right-to-left) needs a GDI compatible
    // back buffer so that IDXGISurface1::GetDC works.
    //
    pPresentParams->GdiCompatible =
        ((CreateParams.RTInitFlags & MilRTInitialization::PresentUsingMask) != MilRTInitialization::PresentUsingHal);

    if (CreateParams.RTInitFlags & MilRTInitialization::PresentImmediately)
    {
        pPresentParams->SyncInterval = 0;
    }
    else
    {
        pPresentParams->SyncInterval = 1;
    }

    return;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::CreateNewDevice
//
//  Synopsis:
//      CreateNewDevice creates a new D3D device and a new CD3DDeviceLevel1 to
//      use the D3D device, then adds the device wrapper to the tracking list.
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceManager::CreateNewDevice(
    __inout_ecount(1)
    D3DDeviceCreationParameters *pD3DCreateParams,  // D3D Device creation
                                                    // parameters
    __inout_ecount(1)
    MilSwapChainParams *pBasePresentParams,         // Base presentation
                                                    // parameters
    __in_xcount(pD3DCreateParams->NumberOfAdaptersInGroup)
    MilDisplayMode *rgDisplayModes,                 // Array of display modes

    __deref_out_ecount(1) CD3DDeviceLevel1 **ppDeviceLevel1
    )
{
    HRESULT hr = S_OK;

    UNREFERENCED_PARAMETER(rgDisplayModes);

    Assert(m_fD3DLoaded);
    Assert(m_pDXGIFactory);
    Assert(m_pDisplaySet);

    *ppDeviceLevel1 = NULL;

    ID3D11Device *pID3DDevice = NULL;
    ID3D11DeviceContext *pID3DContext = NULL;
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_9_1;

    Assert(pD3DCreateParams->NumberOfAdaptersInGroup == 1);

    //
    // Before trying to create a device make sure the display state hasn't
    // changed since we acquired mode information.
    //

    if (m_pDisplaySet->DangerousHasDisplayStateChanged())
    {
        IFC(WGXERR_DISPLAYSTATEINVALID);
    }

    {
        MtSetDefault(Mt(D3DDevice));

        CDisplay const *pDisplay = m_pDisplaySet->Display(pD3DCreateParams->AdapterOrdinal);

        MIL_THR(CreateD3D11Device(
            (pD3DCreateParams->DeviceType == MilDeviceKind_Hardware) ? pDisplay->DXGIAdapter() : NULL,
            pD3DCreateParams->DeviceType,
            pD3DCreateParams->BehaviorFlags,
            &pID3DDevice,
            &pID3DContext,
            &featureLevel
            ));

        if (FAILED(hr))
        {
            TRACE_DEVICECREATE_FAILURE(
                pD3DCreateParams->AdapterOrdinal,
                "Failed to create d3d11 device",
                hr
                );

            if (hr == D3DERR_DEVICELOST)
            {
                MIL_THR(WGXERR_DISPLAYSTATEINVALID);
            }
        }
        else
        {
            for (size_t i = 0; i < m_rgAdapterStatusListeners.GetCount(); i++)
            {
                m_rgAdapterStatusListeners[i]->NotifyAdapterStatus(
                    pD3DCreateParams->AdapterOrdinal,
                    true        // fIsValid
                    );
            }
        }
    }

Cleanup:

#if DBG
    // Note that device creation is in process incase the device becomes
    // unusable during initialization; so that UnusableNotification won't
    // assert that this new device is not yet in its tracking list.
    m_fDbgCreatingNewDevice = true;
#endif

    CD3DDeviceLevel1 *pDeviceLevel1 = NULL;

    if (SUCCEEDED(hr))
    {
        MIL_THR(CD3DDeviceLevel1::Create(
            pID3DDevice,
            pID3DContext,
            featureLevel,
            pD3DCreateParams->DeviceType,
            m_pDisplaySet->Display(pD3DCreateParams->AdapterOrdinal),
            this,
            pD3DCreateParams->BehaviorFlags,
            &pDeviceLevel1
            ));
    }

    if (SUCCEEDED(hr))
    {
        MIL_THR(pDeviceLevel1->CheckRenderTargetFormat(pBasePresentParams->BackBufferFormat));
    }

    if (SUCCEEDED(hr))
    {
        // Make sure there is space for the new entry
        MIL_THR(m_rgDeviceList.ReserveSpace(1));
    }

    if (SUCCEEDED(hr))
    {
        UINT iNewLast = m_rgDeviceList.GetCount();

        // Increase the length
        m_rgDeviceList.SetCount(iNewLast+1);

        // Move the first unusable entry to the end if needed
        if (m_iFirstUnusable < iNewLast)
        {
            m_rgDeviceList[iNewLast] = m_rgDeviceList[m_iFirstUnusable];
        }
        else
        {
            Assert(m_iFirstUnusable == iNewLast);
        }

        // Place the new entry in at the end of the usable list
        m_rgDeviceList[m_iFirstUnusable].pDeviceLevel1 = pDeviceLevel1;
        m_rgDeviceList[m_iFirstUnusable].CreateParams  = *pD3DCreateParams;
        m_rgDeviceList[m_iFirstUnusable].fIsDeviceLost = false;
        #if DBG
            // Hang on to the present parameters for dbg info.
            m_rgDeviceList[m_iFirstUnusable].DbgPresentParams = *pBasePresentParams;
        #endif

        m_iFirstUnusable++;

        // All tracked devices are potential non-static method callers
        m_cCallers++;

        //
        // Return the value and assign the reference from
        // creation to the returned pointer.
        //

        *ppDeviceLevel1 = pDeviceLevel1;
    }
    else
    {
        if (pDeviceLevel1)
        {
            //
            // This device was created but we were unable to track it; so,
            // Release to get the count back to zero and then delete to
            // actually free it.  During the Release this manager will
            // be called, but won't be able to find this resource in its
            // list and therefore won't delete it.
            //

            Verify(pDeviceLevel1->Release() == 0);
            delete pDeviceLevel1;
        }
    }

#if DBG
    // Device creation and tracking is complete
    m_fDbgCreatingNewDevice = false;
#endif

    ReleaseInterfaceNoNULL(pID3DContext);
    ReleaseInterfaceNoNULL(pID3DDevice);

    RRETURN(hr);
}


//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::AddAdapterStatusListener
//
//------------------------------------------------------------------------------

HRESULT
CD3DDeviceManager::AddAdapterStatusListener(__in IAdapterStatusListener *pListener)
{
    CGuard<CCriticalSection> oGuard(m_csManagement);

    RRETURN(m_rgAdapterStatusListeners.Add(pListener));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::RemoveAdapterStatusListener
//
//------------------------------------------------------------------------------

void
CD3DDeviceManager::RemoveAdapterStatusListener(__in IAdapterStatusListener *pListener)
{
    CGuard<CCriticalSection> oGuard(m_csManagement);

    m_rgAdapterStatusListeners.Remove(pListener);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceManager::NotifyDeviceLost
//
//  Synopsis:
//      Notifies all listeners that the device in "info" has been lost. This should be called
//      whenever a device is being destroyed. fIsDeviceLost protects us from incorrectly
//      overnotifying in the multi-window case.
//
//------------------------------------------------------------------------------

void
CD3DDeviceManager::NotifyDeviceLost(D3DDeviceInformation &info)
{
    if (!info.fIsDeviceLost)
    {
        info.fIsDeviceLost = true;

        for (size_t i = 0; i < m_rgAdapterStatusListeners.GetCount(); ++i)
        {
            m_rgAdapterStatusListeners[i]->NotifyAdapterStatus(
                info.CreateParams.AdapterOrdinal,
                false       // fIsValid
                );
        }
    }
}
