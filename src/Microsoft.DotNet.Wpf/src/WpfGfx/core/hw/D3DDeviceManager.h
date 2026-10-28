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
//      Declares CD3DDeviceManager which maintains a list of existing D3D11
//      devices via the CD3DDeviceLevel1 wrappers and creates new
//      CD3DDeviceLevel1's as they are needed.
//
//      Also keeps a shared WARP device for software 3D rendering.
//
//  $Notes:
//      None at this time.
//
//  $ENDTAG
//
//------------------------------------------------------------------------------


class CD3DDeviceLevel1;

// Implement this interface and add yourself to the manager to receive
// notification when adapters are created or destroyed
class IAdapterStatusListener
{
public:
    virtual void NotifyAdapterStatus(UINT uAdapter, bool fIsValid) = 0;
};

class CD3DDeviceManager :
    public IMILPoolManager
{
public:

    static __checkReturn HRESULT Create();
    static void Delete();

    static CD3DDeviceManager *Get();
    static void Release();

    static void NotifyDisplayChange(
        __in_ecount(1) CDisplaySet const * pOldDisplaySet,
        __in_ecount(1) CDisplaySet const * pNewDisplaySet
        );

    CD3DDeviceManager();
    ~CD3DDeviceManager();

    HRESULT GetD3DDeviceAndPresentParams(
        __in_opt HWND hwnd,
        MilRTInitialization::Flags dwFlags,
        __in_ecount_opt(1) CDisplay const *pDisplay,
        MilDeviceKind type,
        __deref_out_ecount(1) CD3DDeviceLevel1 **ppDeviceLevel1,
        __out_ecount_opt(1) MilSwapChainParams *pPresentParams,
        __out_ecount_opt(1) UINT *pAdapterOrdinalInGroup
        );

    HRESULT AddAdapterStatusListener(__in IAdapterStatusListener *pListener);
    void RemoveAdapterStatusListener(__in IAdapterStatusListener *pListener);

    HRESULT GetSWDevice(__deref_out_ecount(1) CD3DDeviceLevel1 **ppDevice);

    void UnusedNotification(
        __inout_ecount(1) CMILPoolResource *pUnused
        );

    void UnusableNotification(
        __inout_ecount(1) CMILPoolResource *pUnusable
        );

    bool DoesWindowedHwDeviceExist(
        UINT uAdapter
        );

private:

    struct D3DDeviceCreationParameters
    {
        UINT AdapterOrdinal;            // display index (CDisplay::GetDisplayIndex)
        MilDeviceKind DeviceType;
        HWND hFocusWindow;
        DWORD BehaviorFlags;            // MIL_DEVICE_BEHAVIOR_*
        UINT MasterAdapterOrdinal;      // ordinal of master adapter for adapter group
        UINT AdapterOrdinalInGroup;     // ordinal inside the adapter group
        UINT NumberOfAdaptersInGroup;
        MilRTInitialization::Flags RTInitFlags;
    };

    struct D3DDeviceInformation
    {
        CD3DDeviceLevel1 *pDeviceLevel1;
        D3DDeviceCreationParameters CreateParams;
        bool fIsDeviceLost;

        #if DBG
            MilSwapChainParams DbgPresentParams;
        #endif
    };

    HRESULT Init();

    HRESULT InitializeD3DReferences(
        __in_ecount_opt(1) CDisplaySet const *pDisplaySet
        );

    void IncCallers();
    void DecCallers();

    void HandleDisplayChange(
        __in_ecount(1) CDisplaySet const * pOldDisplaySet,
        __in_ecount(1) CDisplaySet const * pNewDisplaySet
        );

    HRESULT ComposeCreateParameters(
        __in_opt HWND hwnd,
        MilRTInitialization::Flags dwFlags,
        UINT uAdapter,
        MilDeviceKind type,
        __out_ecount(1) D3DDeviceCreationParameters *pCreateParams
        ) const;

    static void ComposePresentParameters(
        __in_ecount(1) MilDisplayMode const &displayMode,
        __in_ecount(1) D3DDeviceCreationParameters const &CreateParams,
        __out_ecount(1) MilSwapChainParams *pPresentParams
        );

    HRESULT GetDisplayMode(
        __inout_ecount(1) D3DDeviceCreationParameters *pCreateParams,
        __out_xcount_full(pCreateParams->NumberOfAdaptersInGroup) MilDisplayMode *rgDisplayModes
        ) const;

    // D28: displays on the same adapter (LUID) share one hardware device.
    bool DisplaysShareAdapter(
        UINT uDisplayA,
        UINT uDisplayB
        ) const;

    // Adapter status goes to every display that shares the device's adapter.
    void NotifyAdapterStatusForGroup(
        UINT uAdapterOrdinal,
        bool fIsValid
        );

    HRESULT FindDeviceMatch(
        __inout_ecount(1) D3DDeviceCreationParameters *pCreateParams,
        UINT uStartIndex,
        UINT uLimitPlusOne,
        __deref_opt_out_ecount(1) CD3DDeviceLevel1 **ppDeviceLevel1
        ) const;

    HRESULT GetAvailableDevice(
        __inout_ecount(1) D3DDeviceCreationParameters *pCreateParams,
        __deref_out_ecount(1) CD3DDeviceLevel1 **ppDeviceLevel1
        ) const;

    HRESULT CreateNewDevice(
        __inout_ecount(1)
        D3DDeviceCreationParameters *pD3DCreateParams,  // D3D Device creation
                                                        // parameters
        __inout_ecount(1)
        MilSwapChainParams *pBasePresentParams,         // Base presentation
                                                        // parameters
        __in_xcount(pD3DCreateParams->NumberOfAdaptersInGroup)
        MilDisplayMode *rgDisplayModes,                 // Array of display modes

        __deref_out_ecount(1) CD3DDeviceLevel1 **ppDeviceLevel1
        );

    static HRESULT CreateD3D11Device(
        __in_ecount_opt(1) IDXGIAdapter1 *pAdapter,
        MilDeviceKind type,
        DWORD dwBehaviorFlags,
        __deref_out_ecount(1) ID3D11Device **ppDevice,
        __deref_out_ecount(1) ID3D11DeviceContext **ppContext,
        __out_ecount(1) D3D_FEATURE_LEVEL *pFeatureLevel
        );

    void NotifyDeviceLost(D3DDeviceInformation &info);


private:

    CCriticalSection m_csManagement;

    ULONG m_cCallers;
    IDXGIFactory1 *m_pDXGIFactory;
    bool m_fD3DLoaded;

    CDisplaySet const *m_pDisplaySet;
    CDisplaySet const *m_pNextDisplaySet;

    CD3DDeviceLevel1 *m_pSWDevice;

    DynArray<D3DDeviceInformation> m_rgDeviceList;
    UINT m_iFirstUnusable;

    CPtrArray<IAdapterStatusListener> m_rgAdapterStatusListeners;

#if DBG
private:
    bool m_fDbgCreatingNewDevice;   // Indicator that device creation and
                                    // testing is in progress to avoid
                                    // UnusableNotification assert when device
                                    // is not yet in tracking list.
#endif
};
