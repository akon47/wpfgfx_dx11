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
//      Implementation of CD3DDeviceLevel1
//
//      Abstracts the core D3D11 device to provide the following functionality:
//         1. Restrict access to methods of ID3D11Device to those we need.
//         2. Provide correct information for device caps
//         3. Centralize resource creation so that it can be tracked.
//            Tracking created resources is important for responding to mode
//            changes
//         4. Respond to mode changes on present call
//         5. Provide testing functionality for determining if a graphics
//            card meets the level1 criteria for hw acceleration
//
//  $ENDTAG
//
//------------------------------------------------------------------------------

#include "precomp.hpp"
#include <d3d11shader.h>
#include "HwGraphicsCards.h"

using namespace dxlayer;

const DXGI_FORMAT kD3DDepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

MtDefine(CD3DDeviceLevel1, MILRender, "CD3DDeviceLevel1");
MtDefine(NormalTraceVectors, MILRender, "Normal vectors drawn for testing");
MtDefine(MILHwMetrics, Metrics, "MIL HW");
MtDefine(VerticesPerFrame, MILHwMetrics, "Vertices per frame");
MtDefine(TrianglesPerFrame, MILHwMetrics, "Triangles per frame");

MtDefine(D3DAllocatedData, MILRender, "Data Allocated by D3D");

MtDefine(D3DSwapChain,            D3DAllocatedData, "Data Allocated for a D3D swap chain");
MtDefine(D3DColorFill,              D3DAllocatedData, "Data Allocated for a D3D ColorFill");
MtDefine(D3DTexture,              D3DAllocatedData, "Data Allocated for a D3D Texture");
MtDefine(D3DDevice,               D3DAllocatedData, "Data Allocated for a D3D Device");
MtDefine(D3DDrawPrimitiveUP,      D3DAllocatedData, "Data Allocated for a D3D DrawPrimitiveUP Call");
MtDefine(D3DStencilSurface,       D3DAllocatedData, "Data Allocated for a D3D Stencil Surface");
MtDefine(D3DDrawIndexedPrimitive, D3DAllocatedData, "Data Allocated for a D3D DrawIndexedPrimitive call");
MtDefine(D3DDrawIndexedPrimitiveUP, D3DAllocatedData, "Data Allocated for a D3D DrawIndexedPrimitiveUP call");
MtDefine(D3DDrawPrimitive,        D3DAllocatedData, "Data Allocated for a D3D DrawPrimitive call");
MtDefine(D3DVertexBuffer,         D3DAllocatedData, "Data Allocated for a D3D Vertex Buffer");
MtDefine(D3DIndexBuffer,          D3DAllocatedData, "Data Allocated for a D3D Index Buffer");
MtDefine(matrix_t_get_scaling,    D3DAllocatedData, "Data Allocated for Matrix Scaling");
MtDefine(D3DRenderTarget,         D3DAllocatedData, "Data Allocated for a D3D RenderTarget");

MtDefine(D3DCompiler,             D3DAllocatedData, "Data Allocated for a compiled shader");

DeclareTag(tagD3DStats, "MIL-HW", "Output d3d stats");
DeclareTag(tagPixelZoomMode, "MIL-HW", "Pixel zoom mode");
DeclareTag(tagInjectDIE, "MIL-HW", "Inject D3DERR_DRIVERINTERNALERROR failures");

void DbgInjectDIE(__inout_ecount(1) HRESULT * const pHR);

#define GPU_MARKERS_MAX_ARRAY_SIZE 35

#if !DBG
MIL_FORCEINLINE void DbgInjectDIE(__inout_ecount(1) HRESULT *pHR)
{
}
#endif

extern HINSTANCE g_DllInstance;
extern volatile DWORD g_dwTextureUpdatesPerFrame;
LONG g_lPixelsFilledPerFrame = 0;

//+-----------------------------------------------------------------------------
//
//  D3DDevice allocation macros
//
//  Synopsis:
//      Put these around any D3D interface call that can result in
//      D3DERR_OUTOFVIDEOMEMORY. These macros assume that "hr" contains the
//      HRRESULT from the device call.
//
//------------------------------------------------------------------------------

#define BEGIN_DEVICE_ALLOCATION \
    do \
    { \

#define END_DEVICE_ALLOCATION \
    } while (m_resourceManager.FreeSomeVideoMemory((hr))); \

//+-----------------------------------------------------------------------------
//
//  Function:
//      DbgIsPixelZoomMode()
//
//------------------------------------------------------------------------------

#if DBG
// Pick an odd number for the zoom factor so that it is clear what
// side of the center of the pixel an edge lies on.
int c_dbgPixelZoomModeScale = 11;

// Top left corner zoom mode
POINT g_dbgMousePosition = {0, 0};

// HWND from last present
HWND g_dbgHWND;

bool
DbgIsPixelZoomMode()
{
    wpf::util::DpiAwarenessScope<HWND> dpiScope(g_dbgHWND);

    if (IsTagEnabled(tagPixelZoomMode))
    {
        short keyState = GetKeyState(MK_RBUTTON);

        if (keyState & 8000)
        {
            GetCursorPos(&g_dbgMousePosition);
            ScreenToClient(g_dbgHWND, &g_dbgMousePosition);
            return false;
        }
        else
        {
            return true;
        }
    }

    return false;
}
#endif

//+-----------------------------------------------------------------------------
//
//  Function:
//      RectToBox
//
//  Synopsis:
//      RECT -> D3D11_BOX (single 2D slice)
//
//------------------------------------------------------------------------------
static void
RectToBox(
    __in_ecount(1) const RECT &rc,
    __out_ecount(1) D3D11_BOX *pBox
    )
{
    pBox->left = rc.left;
    pBox->top = rc.top;
    pBox->front = 0;
    pBox->right = rc.right;
    pBox->bottom = rc.bottom;
    pBox->back = 1;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::Create
//
//  Synopsis:
//      Create the d3ddevice and test for level1
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::Create(
    __inout_ecount(1) ID3D11Device *pID3DDevice,
    __inout_ecount(1) ID3D11DeviceContext *pID3DContext,
    D3D_FEATURE_LEVEL featureLevel,
    MilDeviceKind deviceKind,
    __in_ecount(1) const CDisplay *pPrimaryDisplay,
    __in_ecount(1) IMILPoolManager *pManager,
    DWORD dwBehaviorFlags,
    __deref_out_ecount(1) CD3DDeviceLevel1 ** const ppDevice
    )
{
    HRESULT hr = S_OK;

    *ppDevice = NULL;

    //
    // Create CD3DDeviceLevel1
    //

    *ppDevice = new CD3DDeviceLevel1(pManager, dwBehaviorFlags);
    IFCOOM(*ppDevice);

    //
    // Call init
    //

    IFC((*ppDevice)->Init(pID3DDevice, pID3DContext, featureLevel, deviceKind, pPrimaryDisplay));

Cleanup:
    if (FAILED(hr))
    {
        // We need to delete here since we've partially initialized the device
        // and the Release call relies on the pool manager being hooked up.
        delete (*ppDevice);
        *ppDevice = NULL;
    }
    else
    {
        (*ppDevice)->AddRef(); // CD3DDeviceLevel1::ctor sets ref count == 0
    }

    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::CD3DDeviceLevel1
//
//------------------------------------------------------------------------------
CD3DDeviceLevel1::CD3DDeviceLevel1(
    __in_ecount(1) IMILPoolManager *pManager,
    DWORD dwBehaviorFlags
    ) : CMILPoolResource(pManager),
        m_Tier(MIL_TIER(0,0)),
        m_dwD3DBehaviorFlags(dwBehaviorFlags),
        m_uFrameNumber(0),
        m_ullLastMarkerId(0),
        m_ullLastConsumedMarkerId(0),
        m_uNumSuccessfulPresentsSinceMarkerFlush(0),
        m_fMultisampleFailed(false),
        m_pCachedAnisoFilterMode(NULL)
{
    m_pMSAAResolveScratch = NULL;
    m_cEntry = 0;
    m_dwThreadId = 0;

    m_pD3DDevice = NULL;
    m_pD3DContext = NULL;
    m_pD3DContext1 = NULL;
    m_pDXGIOutput = NULL;

    m_RenderTargetTestStatusB8G8R8X8.hrTest = WGXERR_NOTINITIALIZED;
    m_RenderTargetTestStatusB8G8R8X8.hrTestGetDC = WGXERR_NOTINITIALIZED;
    m_RenderTargetTestStatusB8G8R8A8.hrTest = WGXERR_NOTINITIALIZED;
    m_RenderTargetTestStatusB8G8R8A8.hrTestGetDC = WGXERR_NOTINITIALIZED;
    m_RenderTargetTestStatusR10G10B10A2.hrTest = WGXERR_NOTINITIALIZED;
    m_RenderTargetTestStatusR10G10B10A2.hrTestGetDC = WGXERR_NOTINITIALIZED;

    m_pCurrentRenderTargetNoRef = NULL;
    m_pDepthStencilBufferForCurrentRTNoRef = NULL;

    m_uAdapterOrdinal = 0;
    ZeroMemory(&m_caps, sizeof(m_caps));
    ZeroMemory(&m_displayMode, sizeof(m_displayMode));

    m_fInScene = false;

    m_fDeviceLostProcessed = false;
    m_fTestDeviceLost = false;
    m_pMediaDevice9 = NULL;

    m_cTexturePool = 0;
    m_cbTexturePool = 0;
    m_uPresentCount = 0;
    m_fTexturePoolEnabled = SUCCEEDED(m_csTexturePool.Init());
    m_hrDisplayInvalid = S_OK;

    m_dwGPUMarkerFlags = 0;

    m_fmtSupportFor32bppBGR =
        m_fmtSupportFor32bppPBGRA =
        m_fmtSupportFor32bppBGR101010 =
        m_fmtSupportFor128bppRGBFloat =
        m_fmtSupportFor128bppPRGBAFloat = MilPixelFormat::Undefined;

    m_multisampleTypeFor32bppBGR.Count = 1;
    m_multisampleTypeFor32bppBGR.Quality = 0;
    m_multisampleTypeFor32bppPBGRA = m_multisampleTypeFor32bppBGR;
    m_multisampleTypeFor32bppBGR101010 = m_multisampleTypeFor32bppBGR;

    if (g_pMediaControl)
    {
        m_dwMetricsVerticesPerFrame = 0;
        m_dwMetricsTrianglesPerFrame = 0;
    }

    ZeroMemory(&m_desc, sizeof(m_desc));

    m_uCacheIndex = CMILResourceCache::InvalidToken;

    m_matSurfaceToClip.reset_to_identity();

    m_pHwIndexBuffer = NULL;
    m_pHwVertexBuffer = NULL;

    m_rgwFanIndices = NULL;
    m_cFanIndices = 0;

    m_pEffectPipelineVertexShader = NULL;
    m_pEffectPipelineVertexBuffer = NULL;
    m_pEffectPipelinePassThroughPixelShader = NULL;

    m_presentFailureWindowMessage = 0;

#if DBG_STEP_RENDERING
    m_fDbgInStepRenderingPresent = false;
    m_pDbgSaveSurface = NULL;
#endif DBG_STEP_RENDERING
}


//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::Init
//
//  Synopsis:
//      1. Adopts the D3D11 device
//      2. Tests it for level1 support
//      3. Initializes this class
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::Init(
    __inout_ecount(1) ID3D11Device *pID3DDevice,
    __inout_ecount(1) ID3D11DeviceContext *pID3DContext,
    D3D_FEATURE_LEVEL featureLevel,
    MilDeviceKind deviceKind,
    __in_ecount(1) const CDisplay *pDisplay
    )
{
    HRESULT hr = S_OK;

    Assert(m_pD3DDevice == NULL);
    Assert(m_pD3DContext == NULL);

    // Initialize the resource manager as early as possible since the resource manager asserts on
    // shutdown that it has a valid device associated. If not, failures in the hardware detection code
    // below will lead to asserts firing in the D3DResourceManager code on shutdown.
    m_resourceManager.Init(this);

    //
    // Initialize basic members
    //

    m_luidD3DAdapter = pDisplay->GetLUID();
    m_uAdapterOrdinal = pDisplay->GetDisplayIndex();

    HwCaps::FromFeatureLevel(featureLevel, deviceKind, &m_caps);

    if (m_caps.FeatureLevel < D3D_FEATURE_LEVEL_10_0)
    {
        TRACE_DEVICECREATE_FAILURE(m_uAdapterOrdinal, "Adapter does not support feature level 10.0", WGXERR_INSUFFICIENT_GPU_CAPS);
        IFC(WGXERR_INSUFFICIENT_GPU_CAPS);
    }

    if (m_caps.MaxAnisotropy == 0)
    {
        m_caps.MaxAnisotropy = 1;
    }

    m_pCachedAnisoFilterMode =
        (m_caps.MaxAnisotropy > 1) ? &CD3DRenderState::sc_fmAnisotropic : &CD3DRenderState::sc_fmLinear;

    //
    // There is only ever one software (WARP) device and it is shared so we
    // must protect it with critical section.
    //

    if (deviceKind == MilDeviceKind_Warp)
    {
        IFC(m_csDeviceEntry.Init());
    }

    //
    // Determine Graphics Acceleration Tier
    //

    if (deviceKind != MilDeviceKind_Warp)
    {
        m_Tier = pDisplay->GetTier();
    }

    {
    #pragma warning(push)
    // error C4328: 'CGuard<LOCK>::CGuard(LOCK &)' : indirection alignment of formal parameter 1 (16) is greater than the actual argument alignment (8)
    #pragma warning(disable : 4328)
    ENTER_DEVICE_FOR_SCOPE(*this);
    #pragma warning(pop)

    m_pD3DDevice = pID3DDevice;
    m_pD3DDevice->AddRef();

    m_pD3DContext = pID3DContext;
    m_pD3DContext->AddRef();

    //
    // D3D11.1 context is required (ClearView, Present1). Available on every
    // OS that .NET 8+ supports.
    //

    MIL_THR(m_pD3DContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void **>(&m_pD3DContext1)));
    if (FAILED(hr))
    {
        TRACE_DEVICECREATE_FAILURE(m_uAdapterOrdinal, "D3D11.1 runtime is required", hr);
        IFC(WGXERR_INSUFFICIENT_GPU_CAPS);
    }

    m_pDXGIOutput = pDisplay->DXGIOutput();
    if (m_pDXGIOutput)
    {
        m_pDXGIOutput->AddRef();
    }

    //
    // If we are rendering with WARP we don't need to check the drivers.
    //
    if (   CD3DRegistryDatabase::ShouldSkipDriverCheck() == false
        && deviceKind == MilDeviceKind_Hardware)
    {
        IFC(CheckBadDeviceDrivers(pDisplay));
    }

    //
    // Detect display mode
    //

    MIL_THR(pDisplay->GetMode(&m_displayMode));
    if (FAILED(hr))
    {
        if (hr == D3DERR_DEVICELOST)
        {
            MIL_THR(WGXERR_DISPLAYSTATEINVALID);
        }

        TRACE_DEVICECREATE_FAILURE(m_uAdapterOrdinal, "Failed to get adapter display mode",  hr);
        goto Cleanup;
    }

    //
    // Detect supported texture formats and multisample types
    //

    GatherSupportedTextureFormats();

    GatherSupportedMultisampleTypes();

    //
    // Request a global cache index
    //

    IGNORE_HR(AcquireIndex());

    //
    // Initialize render state object
    //

    IFC(CD3DRenderState::Init(
        this,
        m_pD3DDevice,
        m_pD3DContext
        ));

    //
    // Initialize hw surface render target shared data
    //

    IFC(InitSharedData(this));

    //
    // Initialize glyph bank
    //

    IFC(m_glyphBank.Init(this, &m_resourceManager));

    //
    // Create our fast path IB/VB
    //
    // 20001 was picked because it's around 625kb for the vertex buffer.
    //

    const UINT uHwVertexBufferSize = 20001*sizeof(CD3DVertexXYZDUV2);
    const UINT uHwIndexBufferSize = 20001*3*sizeof(WORD);

    IFC(CHwD3DIndexBuffer::Create(
        &m_resourceManager,
        this,
        uHwIndexBufferSize,
        &m_pHwIndexBuffer
        ));

    IFC(CHwD3DVertexBuffer::Create(
        &m_resourceManager,
        this,
        uHwVertexBufferSize,
        &m_pHwVertexBuffer
        ));

    // Do basic device tests
    IFC(TestLevel1Device());

    m_presentFailureWindowMessage = RegisterWindowMessage(L"NeedsRePresentOnWake");

    } // Leave device scope

Cleanup:
    if (FAILED(hr))
    {
        ReleaseInterface(m_pDXGIOutput);
        ReleaseInterface(m_pD3DContext1);
        ReleaseInterface(m_pD3DContext);
        ReleaseInterface(m_pD3DDevice);
    }
    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::GatherSupportedTextureFormats
//
//  Synopsis:
//      Inspect texture formats supported by device and setup our mappings from
//      given MilPixelFormat::Enum to supported MilPixelFormat::Enum
//
//------------------------------------------------------------------------------
void
CD3DDeviceLevel1::GatherSupportedTextureFormats()
{
    const UINT uRequired = D3D11_FORMAT_SUPPORT_TEXTURE2D | D3D11_FORMAT_SUPPORT_RENDER_TARGET;

    UINT uSupport = 0;

    bool fSupports128F =
           SUCCEEDED(m_pD3DDevice->CheckFormatSupport(DXGI_FORMAT_R32G32B32A32_FLOAT, &uSupport))
        && ((uSupport & uRequired) == uRequired);

    m_fmtSupportFor128bppPRGBAFloat = fSupports128F ? MilPixelFormat::PRGBA128bppFloat : MilPixelFormat::Undefined;

    // There is no specific support in D3D for 128bpp w/o alpha; so,
    // use the alpha form.
    m_fmtSupportFor128bppRGBFloat = m_fmtSupportFor128bppPRGBAFloat;

    uSupport = 0;
    bool fSupports101010 =
           SUCCEEDED(m_pD3DDevice->CheckFormatSupport(DXGI_FORMAT_R10G10B10A2_UNORM, &uSupport))
        && ((uSupport & uRequired) == uRequired);

    m_fmtSupportFor32bppBGR101010 = fSupports101010 ? MilPixelFormat::BGR32bpp101010 : m_fmtSupportFor128bppRGBFloat;

    uSupport = 0;
    bool fSupportsBGRA =
           SUCCEEDED(m_pD3DDevice->CheckFormatSupport(DXGI_FORMAT_B8G8R8A8_UNORM, &uSupport))
        && ((uSupport & uRequired) == uRequired);

    m_fmtSupportFor32bppPBGRA = fSupportsBGRA ? MilPixelFormat::PBGRA32bpp : m_fmtSupportFor128bppPRGBAFloat;

    uSupport = 0;
    bool fSupportsBGRX =
           SUCCEEDED(m_pD3DDevice->CheckFormatSupport(DXGI_FORMAT_B8G8R8X8_UNORM, &uSupport))
        && ((uSupport & uRequired) == uRequired);

    m_fmtSupportFor32bppBGR =
        fSupportsBGRX ? MilPixelFormat::BGR32bpp :
        ((m_fmtSupportFor32bppPBGRA != MilPixelFormat::Undefined) ?
            m_fmtSupportFor32bppPBGRA :
            m_fmtSupportFor32bppBGR101010);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::GetMaxMultisampleTypeWithDepthSupport
//
//  Synopsis:
//      Search for highest sample count supported for given target surface and
//      depth format.  If none are supported {1,0} is returned.
//
//------------------------------------------------------------------------------
DXGI_SAMPLE_DESC
CD3DDeviceLevel1::GetMaxMultisampleTypeWithDepthSupport(
    DXGI_FORMAT fmtTarget,
    DXGI_FORMAT fmtDepth,
    UINT uMaxSampleCount
    ) const
{
    DXGI_SAMPLE_DESC sd;
    sd.Count = 1;
    sd.Quality = 0;

    UINT uCount = min(uMaxSampleCount, static_cast<UINT>(D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT));

    while (uCount >= 2)
    {
        UINT uLevelsTarget = 0;
        UINT uLevelsDepth = 0;

        if (   SUCCEEDED(m_pD3DDevice->CheckMultisampleQualityLevels(fmtTarget, uCount, &uLevelsTarget))
            && uLevelsTarget > 0
            && (   fmtTarget == fmtDepth
                || (   SUCCEEDED(m_pD3DDevice->CheckMultisampleQualityLevels(fmtDepth, uCount, &uLevelsDepth))
                    && uLevelsDepth > 0)))
        {
            sd.Count = uCount;
            sd.Quality = 0;
            break;
        }

        uCount--;
    }

    return sd;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::GatherSupportedMultisampleTypes
//
//------------------------------------------------------------------------------
void
CD3DDeviceLevel1::GatherSupportedMultisampleTypes()
{
    DWORD dwMaxSampleCount = 4;

    // Get default multi-sample max from the registry
    CDisplayRegKey keyDisplay(HKEY_LOCAL_MACHINE, _T(""));

    keyDisplay.ReadDWORD(
        _T("MaxMultisampleType"),
        &dwMaxSampleCount
        );

    m_multisampleTypeFor32bppBGR = GetMaxMultisampleTypeWithDepthSupport(
        DXGI_FORMAT_B8G8R8X8_UNORM,
        kD3DDepthFormat,
        dwMaxSampleCount
        );

    m_multisampleTypeFor32bppPBGRA = GetMaxMultisampleTypeWithDepthSupport(
        DXGI_FORMAT_B8G8R8A8_UNORM,
        kD3DDepthFormat,
        dwMaxSampleCount
        );

    m_multisampleTypeFor32bppBGR101010 = GetMaxMultisampleTypeWithDepthSupport(
        DXGI_FORMAT_R10G10B10A2_UNORM,
        kD3DDepthFormat,
        dwMaxSampleCount
        );
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::~CD3DDeviceLevel1
//
//------------------------------------------------------------------------------
CD3DDeviceLevel1::~CD3DDeviceLevel1()
{
    Assert(m_cEntry == 0);
    Assert(m_dwThreadId == 0);
    Enter();

    ResetMarkers();

    ReleaseInterface(m_pMediaDevice9);

    TrimTexturePool(true);
    if (m_fTexturePoolEnabled)
    {
        m_csTexturePool.DeInit();
        m_fTexturePoolEnabled = false;
    }

    ReleaseInterface(m_pEffectPipelineVertexShader);
    ReleaseInterface(m_pEffectPipelineVertexBuffer);
    ReleaseInterface(m_pEffectPipelinePassThroughPixelShader);
    m_pCurrentRenderTargetNoRef = NULL;   // No longer care about what is set.

    ReleaseInterface(m_pHwIndexBuffer);
    ReleaseInterface(m_pHwVertexBuffer);
    ReleaseInterface(m_pMSAAResolveScratch);

    WPFFree(ProcessHeap, m_rgwFanIndices);
    m_rgwFanIndices = NULL;

    m_resourceManager.DestroyAllResources();

    // m_pDepthStencilBufferForCurrentRTNoRef should have been released from
    // use at this point
    Assert(m_pDepthStencilBufferForCurrentRTNoRef == NULL);

    if (m_pD3DContext)
    {
        m_pD3DContext->ClearState();
        m_pD3DContext->Flush();
    }

    ReleaseInterface(m_pDXGIOutput);
    ReleaseInterface(m_pD3DContext1);
    ReleaseInterface(m_pD3DContext);
    ReleaseInterface(m_pD3DDevice);

#if DBG
    // Only need to leave for entry assert checks in ~CAssertEntry
    Leave();
#endif
}


//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::Enter and Leave
//
//------------------------------------------------------------------------------

void
CD3DDeviceLevel1::Enter(
    )
{
    if (IsEnsuringCorrectMultithreadedRendering())
    {
        m_csDeviceEntry.Enter();
    }

#if DBG
    // Attempt to catch simultaneous entry from two threads
    m_oDbgEntryCheck.Enter();
#endif

    m_cEntry++;
    m_dwThreadId = GetCurrentThreadId();

    Assert(m_cEntry > 0);
}

void
CD3DDeviceLevel1::Leave(
    )
{
    Assert(m_cEntry > 0);

    // Should leave using same thread we entered on.
    Assert(m_dwThreadId == GetCurrentThreadId());

    if (--m_cEntry == 0)
    {
        m_dwThreadId = 0;
    }

#if DBG
    m_oDbgEntryCheck.Leave();
#endif

    if (IsEnsuringCorrectMultithreadedRendering())
    {
        m_csDeviceEntry.Leave();
    }
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::IsProtected
//
//------------------------------------------------------------------------------

bool
CD3DDeviceLevel1::IsProtected(
    bool fForceEntryConfirmation
    ) const
{
    bool fProtected = (   (   !fForceEntryConfirmation
                           && !(m_dwD3DBehaviorFlags & MIL_DEVICE_BEHAVIOR_MULTITHREADED))
                       || (m_dwThreadId == GetCurrentThreadId()));

    if (fProtected)
    {
        AssertEntry(m_oDbgEntryCheck);
        if (   (m_dwD3DBehaviorFlags & MIL_DEVICE_BEHAVIOR_MULTITHREADED)
            || fForceEntryConfirmation)
        {
            Assert(m_cEntry > 0);
            Assert(m_dwThreadId == GetCurrentThreadId());
        }
    }

    return fProtected;
}


//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::IsEntered
//
//------------------------------------------------------------------------------

bool
CD3DDeviceLevel1::IsEntered(
    ) const
{
    AssertEntry(m_oDbgEntryCheck);

    bool fEntered = (m_cEntry > 0);

    if (fEntered)
    {
        Assert(m_dwThreadId == GetCurrentThreadId());
    }
    else
    {
        Assert(m_dwThreadId == 0);
    }

    return fEntered;
}


//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::TestRenderTargetFormat
//
//  Synopsis:
//      Test the device to see if it is usable with this render target format
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::TestRenderTargetFormat(
    DXGI_FORMAT fmtRenderTarget,
    __inout_ecount(1) TargetFormatTestStatus *pFormatTestEntry
    )
{
    AssertDeviceEntry(*this);

    HRESULT hr = S_OK;
    CD3DSurface *pD3DSurface = NULL;
    CD3DLockableTexture *pLockableTexture = NULL;
    D3D11_TEXTURE2D_DESC desc;
    DXGI_SAMPLE_DESC sampleDesc = { 1, 0 };

    // Render Target create routines are about to be used and they expect this
    // format to have been successfully tested.  So, set status to success now
    // with the expectation that the real status will be set later.
    WHEN_DBG_ANALYSIS(pFormatTestEntry->hrTest = WGXHR_INTERNALTEMPORARYSUCCESS);

    //
    // Try to create a render target
    //

    hr = CreateRenderTarget(
        128,
        128,
        fmtRenderTarget,
        sampleDesc,
        true,
        &pD3DSurface
        );

    if (FAILED(hr))
    {
        TRACE_DEVICECREATE_FAILURE(m_uAdapterOrdinal, "Failed to create render target",  hr);
        goto Cleanup;
    }

    TestGetDC(pD3DSurface, IN OUT pFormatTestEntry);

    MIL_THR(SetRenderTarget(pD3DSurface));
    if (FAILED(hr))
    {
        TRACE_DEVICECREATE_FAILURE(m_uAdapterOrdinal, "Failed to set render target",  hr);
        goto Cleanup;
    }

    MIL_THR(SetDepthStencilSurface(NULL));
    if (FAILED(hr))
    {
        TRACE_DEVICECREATE_FAILURE(m_uAdapterOrdinal, "Failed to reset depth stencil surface",  hr);
        goto Cleanup;
    }

    //
    // Check that we can get our favorite texture format and
    // try to render it
    //

    ZeroMemory(&desc, sizeof(desc));
    desc.Format = PixelFormatToD3DFormat(MilPixelFormat::BGRA32bpp);
    desc.Width = 128;
    desc.Height = 128;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.SampleDesc = sampleDesc;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    MIL_THR(CreateLockableTexture(
        &desc,
        &pLockableTexture
        ));

    if (FAILED(hr))
    {
        TRACE_DEVICECREATE_FAILURE(m_uAdapterOrdinal, "Failed to create 32-bit ARGB texture",  hr);
        goto Cleanup;
    }

    {
        MilPointAndSizeL rc = {0, 0, 128, 128};
        MIL_THR(RenderTexture(pLockableTexture, rc, TBM_DEFAULT/* premultiplied */));
        if (FAILED(hr))
        {
            TRACE_DEVICECREATE_FAILURE(m_uAdapterOrdinal, "Failed to draw texture",  hr);
            goto Cleanup;
        }
    }

Cleanup:

    pFormatTestEntry->hrTest = hr;

    if (FAILED(hr))
    {
        if (   (hr == D3DERR_OUTOFVIDEOMEMORY)
            || (hr == E_OUTOFMEMORY)
            || (hr == D3DERR_DRIVERINTERNALERROR))
        {
            // This case doesn't actually determine usability; so reset back to
            // untested (not initialized).
            pFormatTestEntry->hrTest = WGXERR_NOTINITIALIZED;
        }
        else
        {
            if (pFormatTestEntry->hrTestGetDC == WGXERR_NOTINITIALIZED)
            {
                pFormatTestEntry->hrTestGetDC = pFormatTestEntry->hrTest;
            }
        }
    }

    if (pD3DSurface)
    {
        ReleaseUseOfRenderTarget(pD3DSurface);
    }

    ReleaseInterfaceNoNULL(pD3DSurface);
    ReleaseInterfaceNoNULL(pLockableTexture);

    RRETURN(hr); // D3DERR_DRIVERINTERNALERROR OK here
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::TestLevel1Device
//
//  Synopsis:
//      Test the device to see if it is basically usable
//
//------------------------------------------------------------------------------

HRESULT
CD3DDeviceLevel1::TestLevel1Device(
    )
{
    HRESULT hr = S_OK;

    //
    // Check Device Caps
    //

    IFC(HwCaps::CheckDeviceLevel1(m_caps));

    //
    // Test some render states
    //

#define IFC_RSCHECK(expr) {MIL_THR(expr); if (FAILED(hr)) {                              \
    TRACE_DEVICECREATE_FAILURE(m_uAdapterOrdinal, "Failed to set render states", hr); \
    goto Cleanup;}}

    IFC_RSCHECK( SetRenderState_AlphaSolidBrush() );
    IFC_RSCHECK( SetRenderState_Texture( TBM_DEFAULT, TBA_None, MilBitmapInterpolationMode::NearestNeighbor, 0) );
    IFC_RSCHECK( SetRenderState_Texture( TBM_DEFAULT, TBA_None, MilBitmapInterpolationMode::Linear, 0) );
    IFC_RSCHECK( SetRenderState_Texture( TBM_DEFAULT, TBA_None, MilBitmapInterpolationMode::TriLinear, 0) );
    IFC_RSCHECK( SetRenderState_Texture( TBM_DEFAULT, TBA_Diffuse, MilBitmapInterpolationMode::NearestNeighbor, 0) );
    IFC_RSCHECK( SetRenderState_Texture( TBM_DEFAULT, TBA_Diffuse, MilBitmapInterpolationMode::Linear, 0) );
    IFC_RSCHECK( SetRenderState_Texture( TBM_DEFAULT, TBA_Diffuse, MilBitmapInterpolationMode::TriLinear, 0) );
    IFC_RSCHECK( SetRenderState_Texture( TBM_DEFAULT, TBA_Specular, MilBitmapInterpolationMode::NearestNeighbor, 0) );
    IFC_RSCHECK( SetRenderState_Texture( TBM_DEFAULT, TBA_Specular, MilBitmapInterpolationMode::Linear, 0) );
    IFC_RSCHECK( SetRenderState_Texture( TBM_DEFAULT, TBA_Specular, MilBitmapInterpolationMode::TriLinear, 0) );

    IFC_RSCHECK( SetRenderState_AlphaSolidBrush() );
    IFC_RSCHECK( SetRenderState_Texture( TBM_DEFAULT, TBA_None, MilBitmapInterpolationMode::NearestNeighbor, 1) );
    IFC_RSCHECK( SetRenderState_Texture( TBM_DEFAULT, TBA_None, MilBitmapInterpolationMode::Linear, 1) );
    IFC_RSCHECK( SetRenderState_Texture( TBM_DEFAULT, TBA_None, MilBitmapInterpolationMode::TriLinear, 1) );
    IFC_RSCHECK( SetRenderState_Texture( TBM_DEFAULT, TBA_Diffuse, MilBitmapInterpolationMode::NearestNeighbor, 1) );
    IFC_RSCHECK( SetRenderState_Texture( TBM_DEFAULT, TBA_Diffuse, MilBitmapInterpolationMode::Linear, 1) );
    IFC_RSCHECK( SetRenderState_Texture( TBM_DEFAULT, TBA_Diffuse, MilBitmapInterpolationMode::TriLinear, 1) );
    IFC_RSCHECK( SetRenderState_Texture( TBM_DEFAULT, TBA_Specular, MilBitmapInterpolationMode::NearestNeighbor, 1) );
    IFC_RSCHECK( SetRenderState_Texture( TBM_DEFAULT, TBA_Specular, MilBitmapInterpolationMode::Linear, 1) );
    IFC_RSCHECK( SetRenderState_Texture( TBM_DEFAULT, TBA_Specular, MilBitmapInterpolationMode::TriLinear, 1) );
#undef IFC_RSCHECK

    //
    // Let people know that everything is ok!
    //

    if (IsSWDevice())
    {
        TraceTag((tagError, "MIL-HW(adapter=%d): WARP device tested successfully. (For SW 3D use only.)", m_uAdapterOrdinal));
    }
    else
    {
        TraceTag((tagError, "MIL-HW(adapter=%d): d3d11 device tested successfully.", m_uAdapterOrdinal));
    }

Cleanup:
    if (FAILED(hr) &&
        (hr != D3DERR_OUTOFVIDEOMEMORY) &&
        (hr != E_OUTOFMEMORY) &&
        (hr != D3DERR_DRIVERINTERNALERROR) &&
        IsHALDevice())
    {
        IGNORE_HR(CD3DRegistryDatabase::DisableAdapter(m_uAdapterOrdinal));
    }

    RRETURN(hr); // D3DERR_DRIVERINTERNALERROR OK here
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::TestGetDC
//
//  Synopsis:
//      Test to see if we can obtain a DC from this surface.  Record the result
//      into the device (TargetFormatTestStatus *), so we can avoid creating
//      surfaces that will require a DC.
//
//------------------------------------------------------------------------------
void
CD3DDeviceLevel1::TestGetDC(
    __inout_ecount(1) CD3DSurface * const pD3DSurface,
    __inout_ecount(1) TargetFormatTestStatus * const pFormatTestEntry
    )
{
    HDC hTestDC = NULL;
    HRESULT hrGetDC;

    Assert(pFormatTestEntry->hrTestGetDC == WGXERR_NOTINITIALIZED);

    // Make a test GetDC call
    MIL_THRX(hrGetDC, pD3DSurface->GetDC(&hTestDC));

    Assert(hrGetDC != WGXERR_NOTINITIALIZED);

    pFormatTestEntry->hrTestGetDC = hrGetDC;

    if (hTestDC)
    {
        pD3DSurface->ReleaseDC(hTestDC);
    }

    return;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::GetRenderTargetFormatTestEntry
//
//------------------------------------------------------------------------------

HRESULT
CD3DDeviceLevel1::GetRenderTargetFormatTestEntry(
    DXGI_FORMAT fmtRenderTarget,
    __deref_out_ecount(1) TargetFormatTestStatus * &pFormatTestEntry
    )
{
    HRESULT hr = S_OK;

    AssertDeviceEntry(*this);

    pFormatTestEntry = NULL;

    switch (fmtRenderTarget)
    {
    case DXGI_FORMAT_B8G8R8X8_UNORM:
        pFormatTestEntry = &m_RenderTargetTestStatusB8G8R8X8;
        break;

    case DXGI_FORMAT_B8G8R8A8_UNORM:
        pFormatTestEntry = &m_RenderTargetTestStatusB8G8R8A8;
        break;

    case DXGI_FORMAT_R10G10B10A2_UNORM:
        pFormatTestEntry = &m_RenderTargetTestStatusR10G10B10A2;
        break;

    default:
        RIP("Unsupported render target format.");
        IFC(E_INVALIDARG);
        break;
    }

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::CheckRenderTargetFormat
//
//  Synopsis:
//      Check the device to see if it is usable with this render target format.
//      Testing will only happen the first time the format is introduced to the
//      device.
//
//------------------------------------------------------------------------------
HRESULT CD3DDeviceLevel1::CheckRenderTargetFormat(
    DXGI_FORMAT fmtRenderTarget,
    __deref_opt_out_ecount(1) HRESULT const ** const pphrTestGetDC
    )
{
    #pragma warning(push)
    #pragma warning(disable : 4328)
    ENTER_DEVICE_FOR_SCOPE(*this);
    #pragma warning(pop)

    HRESULT hr = S_OK;
    TargetFormatTestStatus *pRenderTargetTestStatus = NULL;

    IFC(GetRenderTargetFormatTestEntry(
        fmtRenderTarget,
        OUT pRenderTargetTestStatus
        ));

    // Return pointer to GetDC test status if requested.
    if (pphrTestGetDC)
    {
        *pphrTestGetDC = &pRenderTargetTestStatus->hrTestGetDC;
    }

    if (pRenderTargetTestStatus->hrTest != WGXERR_NOTINITIALIZED)
    {
        IFC(pRenderTargetTestStatus->hrTest);
        goto Cleanup;
    }

    IFC(TestRenderTargetFormat(fmtRenderTarget, pRenderTargetTestStatus));

Cleanup:
    if (FAILED(hr))
    {
        TraceTag((tagError, "MIL-HW(adapter=%d): d3d device failed testing.", m_uAdapterOrdinal));
    }

    RRETURN(hr); // Let DIE through
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::AssertRenderFormatIsTestedSuccessfully
//
//------------------------------------------------------------------------------

void
CD3DDeviceLevel1::AssertRenderFormatIsTestedSuccessfully(
    DXGI_FORMAT fmtRenderTarget
    )
{
#if DBG_ANALYSIS
    #pragma warning(push)
    #pragma warning(disable : 4328)
    ENTER_DEVICE_FOR_SCOPE(*this);
    #pragma warning(pop)

    TargetFormatTestStatus *pRenderTargetTestStatus = NULL;

    Assert(SUCCEEDED(GetRenderTargetFormatTestEntry(
        fmtRenderTarget,
        OUT pRenderTargetTestStatus
        )));

    if (pRenderTargetTestStatus)
    {
        Assert(pRenderTargetTestStatus->hrTest != WGXERR_NOTINITIALIZED);
        Assert(SUCCEEDED(pRenderTargetTestStatus->hrTest));
    }
#else
    UNREFERENCED_PARAMETER(fmtRenderTarget);
#endif
}


//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::CheckBadDeviceDrivers
//
//  Synopsis:
//      Reject devices with known bad drivers
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::CheckBadDeviceDrivers(
    __in_ecount(1) const CDisplay *pDisplay
    )
{
    AssertDeviceEntry(*this);

    HRESULT hr = S_OK;

    // D28: the device may serve several displays of one adapter.
    Assert(   m_luidD3DAdapter.LowPart == pDisplay->GetLUID().LowPart
           && m_luidD3DAdapter.HighPart == pDisplay->GetLUID().HighPart);

    if (!pDisplay->IsRecentDriver())
    {
        IFC(E_FAIL);
    }
    else if (pDisplay->IsDeviceDriverBad())
    {
        MIL_THR(E_FAIL);

        TRACE_DEVICECREATE_FAILURE(
            m_uAdapterOrdinal,
            "Device has been disabled due to driver problems.",
            hr);

        goto Cleanup;
    }

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::MarkUnusable
//
//  Synopsis:
//      Mark this device as unusable and notify manager. Future calls to Present
//      will return device lost.
//
//------------------------------------------------------------------------------
void
CD3DDeviceLevel1::MarkUnusable(
    bool fMayBeMultithreadedCall
    )
{
    // No entry check as this method is thread safe

    if (m_hrDisplayInvalid == D3DERR_DRIVERINTERNALERROR)
    {
        IGNORE_HR(CD3DRegistryDatabase::HandleAdapterUnexpectedError(m_uAdapterOrdinal));
    }

    //
    // Future calls to Present will return display invalid
    //

    m_hrDisplayInvalid = WGXERR_DISPLAYSTATEINVALID;

    if (   !m_fDeviceLostProcessed
        && IsProtected(fMayBeMultithreadedCall)
#if DBG_STEP_RENDERING
        && !DbgInStepRenderingPresent()
#endif DBG_STEP_RENDERING
       )
    {
        // Destroy all GPUMarkers created using this device
        ResetMarkers();

        m_fDeviceLostProcessed = true;

        //
        // Notify the manager this device is unusable
        //

        m_pManager->UnusableNotification(this);

        //
        // Attempt to destroy all resources that are now also lost/unusable
        //
        // *** NOTE: UnusableNotification is depending upon this for D3DImage
        //

        m_resourceManager.DestroyAllResources();
        TrimTexturePool(true);
    }
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::CreateRenderTarget
//
//  Synopsis:
//      Create a render target texture and place a resource wrapper around it
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::CreateRenderTarget(
    UINT uiWidth,
    UINT uiHeight,
    DXGI_FORMAT fmtSurface,
    __in_ecount(1) const DXGI_SAMPLE_DESC &sampleDesc,
    bool fLockable,
    __deref_out_ecount(1) CD3DSurface ** const ppD3DSurface
    )
{
    HRESULT hr = S_OK;
    ID3D11Texture2D *pID3DTexture = NULL;

    IFC(CreateRenderTargetUntracked(
        uiWidth,
        uiHeight,
        fmtSurface,
        sampleDesc,
        fLockable,
        &pID3DTexture
        ));

    IFC(CD3DSurface::Create(&m_resourceManager, pID3DTexture, 0, ppD3DSurface));

Cleanup:
    ReleaseInterface(pID3DTexture);

    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::CreateRenderTargetUntracked
//
//  Synopsis:
//      Create a render target texture that is not tracked by our resource
//      management system.  fLockable requests a GDI compatible texture
//      (IDXGISurface1::GetDC) which is only possible for B8G8R8A8 targets.
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::CreateRenderTargetUntracked(
    UINT uiWidth,
    UINT uiHeight,
    DXGI_FORMAT fmtSurface,
    __in_ecount(1) const DXGI_SAMPLE_DESC &sampleDesc,
    bool fLockable,
    __deref_out_ecount(1) ID3D11Texture2D ** const ppD3DTexture
    )
{
    HRESULT hr = S_OK;

    ID3D11Texture2D *pID3DTexture = NULL;

    *ppD3DTexture = NULL;

    AssertDeviceEntry(*this);

    //
    // B8G8R8X8 only marks "no destination alpha" (ChooseTargetFormat). The swap
    // chain back buffer is B8G8R8A8 and copies/resolves between the two formats
    // are illegal in D3D11, so every render target is B8G8R8A8.
    //
    if (fmtSurface == DXGI_FORMAT_B8G8R8X8_UNORM)
    {
        fmtSurface = DXGI_FORMAT_B8G8R8A8_UNORM;
    }

    AssertRenderFormatIsTestedSuccessfully(fmtSurface);

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));

    desc.Width = uiWidth;
    desc.Height = uiHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = fmtSurface;
    desc.SampleDesc = sampleDesc;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    if (sampleDesc.Count <= 1)
    {
        desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
    }
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    if (   fLockable
        && sampleDesc.Count <= 1
        && fmtSurface == DXGI_FORMAT_B8G8R8A8_UNORM)
    {
        desc.MiscFlags |= D3D11_RESOURCE_MISC_GDI_COMPATIBLE;
    }

    BEGIN_DEVICE_ALLOCATION;

    {
        MtSetDefault(Mt(D3DRenderTarget));

        MilDx11CountCreate(MilDx11Site_RenderTarget);
        hr = MilTranslateD3D11Error(m_pD3DDevice->CreateTexture2D(
            &desc,
            NULL,
            &pID3DTexture
            ));
    }

    END_DEVICE_ALLOCATION;

    IFC(hr); // placed here to avoid stack capturing multiple times in allocation loop

    *ppD3DTexture = pID3DTexture; // Steal ref
    pID3DTexture = NULL;

Cleanup:
    ReleaseInterfaceNoNULL(pID3DTexture);

    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::GetRenderTargetData
//
//  Synopsis:
//      Copies data from a render target source texture to a staging
//      destination texture (resolving multisampled sources).
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::GetRenderTargetData(
    __in_ecount(1) ID3D11Texture2D *pSourceTexture,
    UINT uSourceSubresource,
    __inout_ecount(1) ID3D11Texture2D *pDestinationStagingTexture
    )
{
    HRESULT hr = S_OK;

    D3D11_TEXTURE2D_DESC descSrc;
    D3D11_TEXTURE2D_DESC descDst;

    pSourceTexture->GetDesc(&descSrc);
    pDestinationStagingTexture->GetDesc(&descDst);

    if (descSrc.SampleDesc.Count > 1)
    {
        m_pD3DContext->ResolveSubresource(
            pDestinationStagingTexture,
            0,
            pSourceTexture,
            uSourceSubresource,
            descDst.Format
            );
    }
    else
    {
        m_pD3DContext->CopySubresourceRegion(
            pDestinationStagingTexture,
            0,
            0, 0, 0,
            pSourceTexture,
            uSourceSubresource,
            NULL
            );
    }

    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::CreateAdditionalSwapChain
//
//  Synopsis:
//      Creates a DXGI swap chain for the window (bitblt model, D10).
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::CreateAdditionalSwapChain(
    __in_ecount_opt(1) CMILDeviceContext const *pMILDC,
    __in_ecount(1) const MilSwapChainParams *pParams,
    __deref_out_ecount(1) CD3DSwapChain ** const ppSwapChain
    )
{
    AssertDeviceEntry(*this);

    HRESULT hr = S_OK;
    IDXGIDevice *pDXGIDevice = NULL;
    IDXGIAdapter *pDXGIAdapter = NULL;
    IDXGIFactory2 *pDXGIFactory = NULL;
    IDXGISwapChain1 *pDXGISwapChain = NULL;

    *ppSwapChain = NULL;

    //
    // It is possible for caller to try creating a render target even though
    // the device is now invalid.  Check here for such a situation.
    //

    IFC(m_hrDisplayInvalid);

    if (pParams->BackBufferWidth > m_caps.MaxTextureWidth ||
        pParams->BackBufferHeight > m_caps.MaxTextureHeight)
    {
        // Any error except WGXERR_DISPLAYSTATEINVALID will try fallback to SW.
        IFC(D3DERR_OUTOFVIDEOMEMORY);
    }

    //
    // Find the DXGI factory that owns this device
    //

    IFC(m_pD3DDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void **>(&pDXGIDevice)));
    IFC(pDXGIDevice->GetAdapter(&pDXGIAdapter));
    IFC(pDXGIAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void **>(&pDXGIFactory)));

    {
        MtSetDefault(Mt(D3DSwapChain));

        DXGI_SWAP_CHAIN_DESC1 desc;
        ZeroMemory(&desc, sizeof(desc));

        desc.Width = pParams->BackBufferWidth;
        desc.Height = pParams->BackBufferHeight;
        //
        // DXGI swap chains accept B8G8R8A8 but not B8G8R8X8. The X8 target
        // format only marks "no destination alpha" for the render target;
        // the real back buffer is B8G8R8A8 and DXGI_ALPHA_MODE_IGNORE drops
        // the alpha at present. GDI compatible swap chains must be B8G8R8A8.
        //
        desc.Format =
            (pParams->GdiCompatible || pParams->BackBufferFormat == DXGI_FORMAT_B8G8R8X8_UNORM)
            ? DXGI_FORMAT_B8G8R8A8_UNORM
            : pParams->BackBufferFormat;

        IFC(CheckRenderTargetFormat(desc.Format));
        desc.Stereo = FALSE;
        desc.SampleDesc = pParams->SampleDesc;
        // SHADER_INPUT: Begin3D copies the back buffer into the multisample
        // intermediate by drawing it (StretchRectScaled).
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
        //
        // D29: flip model (two buffers). WPF keeps redrawing only what
        // changed; CD3DSwapChain::SyncBackBufferAfterPresent carries the
        // previous frame into the rotated back buffer so that still holds.
        //
        const bool fFlipModel =
               pParams->FlipModel
            && !pParams->GdiCompatible
            && pParams->SampleDesc.Count <= 1;

        // Three buffers: with two, the GPU cannot start the next frame until DWM
        // releases the previous one at vblank, and every GPU sync in the process
        // (layered window readback, CopyPixels) waits behind that stall.
        desc.BufferCount = fFlipModel ? 3 : max(1u, pParams->BackBufferCount);
        desc.Scaling = DXGI_SCALING_STRETCH;
        desc.SwapEffect = fFlipModel ? DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL : DXGI_SWAP_EFFECT_SEQUENTIAL;

        if (fFlipModel)
        {
            // A released flip chain of this HWND must be gone before DXGI
            // accepts a new one; released wrappers normally wait a frame.
            m_resourceManager.DestroyResources(CD3DResourceManager::WithDelay);
        }
        desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        //
        // D31: a BGRA back buffer presented through GDI is read back by
        // CD3DSwapChainWithSwDC's own staging texture, so the surface itself
        // never needs GDI access. Only other formats keep the flag.
        //
        const bool fSwDC =
               pMILDC != NULL
            && !pMILDC->PresentWithHAL()
            && (   pParams->BackBufferFormat == DXGI_FORMAT_B8G8R8A8_UNORM
                || pParams->BackBufferFormat == DXGI_FORMAT_B8G8R8X8_UNORM);

        desc.Flags = (pParams->GdiCompatible && !fSwDC) ? DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE : 0;

        AssertRenderFormatIsTestedSuccessfully(desc.Format);

        BEGIN_DEVICE_ALLOCATION;

        MilDx11CountCreate(MilDx11Site_SwapChain);
        hr = MilTranslateD3D11Error(pDXGIFactory->CreateSwapChainForHwnd(
            m_pD3DDevice,
            pParams->hDeviceWindow,
            &desc,
            NULL,
            NULL,
            &pDXGISwapChain
            ));

        if (FAILED(hr) && fFlipModel)
        {
            MIL_DX11_LOG("flip model swap chain refused hr=0x%08x, using bitblt", hr);

            desc.BufferCount = max(1u, pParams->BackBufferCount);
            desc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;

            hr = MilTranslateD3D11Error(pDXGIFactory->CreateSwapChainForHwnd(
                m_pD3DDevice,
                pParams->hDeviceWindow,
                &desc,
                NULL,
                NULL,
                &pDXGISwapChain
                ));
        }
        END_DEVICE_ALLOCATION;

        IFC(hr); // placed here to avoid jumping to cleanup immediately on OOVM

        // DXGI must not take over Alt+Enter or react to window changes.
        IGNORE_HR(pDXGIFactory->MakeWindowAssociation(
            pParams->hDeviceWindow,
            DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER
            ));
    }

    //
    // Create swap chain wrapper
    //

    if (   pMILDC
        && (   pMILDC->PresentWithHAL()
            || (   pParams->BackBufferFormat != DXGI_FORMAT_B8G8R8A8_UNORM
                && pParams->BackBufferFormat != DXGI_FORMAT_B8G8R8X8_UNORM
               )
           )
       )
    {
        //
        // The MILDC, if passed to CD3DSwapChain::Create, will cause us to
        // implement GetDC ourselves by copying the swap chain surface to a
        // software bitmap.
        //
        pMILDC = NULL;
    }

    IFC(CD3DSwapChain::Create(
        &m_resourceManager,
        pDXGISwapChain,
        max(1u, pParams->BackBufferCount),
        pMILDC,
        ppSwapChain
        ));

Cleanup:
    // Can't use HandleDIE because if we can't create swap chain
    // present won't be called.

    switch (hr)
    {
    case D3DERR_DRIVERINTERNALERROR:
        m_hrDisplayInvalid = D3DERR_DRIVERINTERNALERROR;
        __fallthrough;

    case D3DERR_DEVICELOST:
        hr = WGXERR_DISPLAYSTATEINVALID;
        MarkUnusable(false /* This call is already entry protected. */);
    }

    MIL_DX11_LOG("CreateAdditionalSwapChain hr=0x%08x %ux%u fmt=%d gdi=%d flip=%d hwnd=%p",
                 hr, pParams->BackBufferWidth, pParams->BackBufferHeight,
                 pParams->BackBufferFormat, pParams->GdiCompatible, pParams->FlipModel, pParams->hDeviceWindow);

    ReleaseInterface(pDXGISwapChain);
    ReleaseInterface(pDXGIFactory);
    ReleaseInterface(pDXGIAdapter);
    ReleaseInterface(pDXGIDevice);
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::CreateVertexBuffer / CreateIndexBuffer
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::CreateVertexBuffer(
    UINT cbLength,
    bool fDynamic,
    __deref_out_ecount(1) ID3D11Buffer ** const ppVertexBuffer
    )
{
    AssertDeviceEntry(*this);

    HRESULT hr = S_OK;

    MtSetDefault(Mt(D3DVertexBuffer));

    D3D11_BUFFER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.ByteWidth = cbLength;
    desc.Usage = fDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = fDynamic ? D3D11_CPU_ACCESS_WRITE : 0;

    BEGIN_DEVICE_ALLOCATION;

    hr = MilTranslateD3D11Error(m_pD3DDevice->CreateBuffer(&desc, NULL, ppVertexBuffer));

    END_DEVICE_ALLOCATION;

    MIL_THR(hr);

    RRETURN(HandleDIE(hr));
}

HRESULT
CD3DDeviceLevel1::CreateIndexBuffer(
    UINT cbLength,
    bool fDynamic,
    __deref_out_ecount(1) ID3D11Buffer ** const ppIndexBuffer
    )
{
    AssertDeviceEntry(*this);

    HRESULT hr = S_OK;

    MtSetDefault(Mt(D3DIndexBuffer));

    D3D11_BUFFER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.ByteWidth = cbLength;
    desc.Usage = fDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    desc.CPUAccessFlags = fDynamic ? D3D11_CPU_ACCESS_WRITE : 0;

    BEGIN_DEVICE_ALLOCATION;

    hr = MilTranslateD3D11Error(m_pD3DDevice->CreateBuffer(&desc, NULL, ppIndexBuffer));

    END_DEVICE_ALLOCATION;

    MIL_THR(hr);

    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::CreateTexture
//
//  Synopsis:
//      Creates a texture.  If pSharedHandle is non-NULL and points to a
//      non-NULL handle the existing shared texture is opened instead; if it
//      points to NULL a shareable texture is created and the handle returned.
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::CreateTexture(
    __in_ecount(1) const D3D11_TEXTURE2D_DESC *pDesc,
    __in_ecount_opt(1) const D3D11_SUBRESOURCE_DATA *pInitialData,
    __deref_out_ecount(1) ID3D11Texture2D ** const ppD3DTexture,
    __deref_opt_inout_ecount(1) HANDLE * const pSharedHandle,
    bool fAllowLargerFromPool
    )
{
    AssertDeviceEntry(*this);

    HRESULT hr = S_OK;
    IDXGIResource *pDXGIResource = NULL;

    *ppD3DTexture = NULL;

    if (m_fDeviceLostProcessed == true)
    {
        MIL_THR(WGXERR_DISPLAYSTATEINVALID);
        RRETURN(hr);
    }

    if (pSharedHandle && *pSharedHandle)
    {
        //
        // Open an existing shared texture
        //

        MIL_THR(MilTranslateD3D11Error(m_pD3DDevice->OpenSharedResource(
            *pSharedHandle,
            __uuidof(ID3D11Texture2D),
            reinterpret_cast<void **>(ppD3DTexture)
            )));

        goto Cleanup;
    }

    {
        MtSetDefault(Mt(D3DTexture));

        D3D11_TEXTURE2D_DESC desc = *pDesc;

        if (pSharedHandle)
        {
            desc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED;
        }

        if (   pInitialData == NULL
            && pSharedHandle == NULL
            && TryGetPooledTexture(&desc, ppD3DTexture, fAllowLargerFromPool))
        {
            MilDx11CountCreate(MilDx11Site_PoolHit);
            goto Cleanup;
        }

        // Note: rounding sizes up (so animated intermediates reuse each other)
        // was tried and rejected: the effect pipeline samples intermediates by
        // the texture's real size, so padding shifts the content per frame.

        BEGIN_DEVICE_ALLOCATION;

        MilDx11CountCreate(MilDx11Site_Texture);
        if (g_fMilDx11Stats)
        {
            MilDx11StatAdd(
                  pInitialData ? MilDx11Stat_TexNewWithData
                : (desc.BindFlags & D3D11_BIND_RENDER_TARGET) ? MilDx11Stat_TexNewRenderTarget
                : (desc.Usage == D3D11_USAGE_STAGING) ? MilDx11Stat_TexNewStaging
                : MilDx11Stat_TexNewOther);

            static LONG s_cLogged = 0;
            if ((desc.BindFlags & D3D11_BIND_RENDER_TARGET) && InterlockedIncrement(&s_cLogged) <= 60)
            {
                MIL_DX11_LOG("new RT texture %ux%u fmt=%d samples=%u", desc.Width, desc.Height, desc.Format, desc.SampleDesc.Count);
            }
        }
        hr = MilTranslateD3D11Error(m_pD3DDevice->CreateTexture2D(
            &desc,
            pInitialData,
            ppD3DTexture
            ));

        END_DEVICE_ALLOCATION;

        DbgInjectDIE(&hr);

        MIL_THR(hr); // placed here to avoid stack capturing multiple times in allocation loop
    }

    if (SUCCEEDED(hr) && pSharedHandle)
    {
        IFC((*ppD3DTexture)->QueryInterface(__uuidof(IDXGIResource), reinterpret_cast<void **>(&pDXGIResource)));
        IFC(pDXGIResource->GetSharedHandle(pSharedHandle));
    }

Cleanup:
    ReleaseInterface(pDXGIResource);

    if (FAILED(hr))
    {
        ReleaseInterface(*ppD3DTexture);
    }

    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::CreateLockableTexture
//
//  Synopsis:
//      Creates a GPU texture plus a staging twin and wraps them.
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::CreateLockableTexture(
    __in_ecount(1) const D3D11_TEXTURE2D_DESC *pDesc,
    __deref_out_ecount(1) CD3DLockableTexture ** const ppLockableTexture
    )
{
    AssertDeviceEntry(*this);

    HRESULT hr = S_OK;
    ID3D11Texture2D *pD3DTexture = NULL;
    ID3D11Texture2D *pD3DStaging = NULL;

    *ppLockableTexture = NULL;

    D3D11_TEXTURE2D_DESC descGpu = *pDesc;
    descGpu.ArraySize = 1;
    descGpu.Usage = D3D11_USAGE_DEFAULT;
    descGpu.CPUAccessFlags = 0;
    descGpu.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

    if (descGpu.MipLevels == 0 || descGpu.MipLevels > 1)
    {
        // Mip chain: generated on the GPU from level 0
        descGpu.BindFlags |= D3D11_BIND_RENDER_TARGET;
        descGpu.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
    }

    D3D11_TEXTURE2D_DESC descStaging = *pDesc;
    descStaging.ArraySize = 1;
    descStaging.MipLevels = 1;
    descStaging.Usage = D3D11_USAGE_STAGING;
    descStaging.BindFlags = 0;
    descStaging.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    descStaging.MiscFlags = 0;

    IFC(CreateTexture(&descGpu, NULL, &pD3DTexture));
    IFC(CreateTexture(&descStaging, NULL, &pD3DStaging));

    //
    // Create the texture wrapper
    //

    IFC(CD3DLockableTexture::Create(
        &m_resourceManager,
        pD3DTexture,
        pD3DStaging,
        ppLockableTexture
        ));

Cleanup:
    ReleaseInterface(pD3DStaging);
    ReleaseInterface(pD3DTexture);
    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::CreateSysMemUpdateSurface
//
//  Synopsis:
//      Creates a staging texture, optionally initialized with the given
//      tightly packed pixels.
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::CreateSysMemUpdateSurface(
    UINT uWidth,
    UINT uHeight,
    DXGI_FORMAT fmtTexture,
    __in_xcount_opt(uWidth * uHeight * D3DFormatSize(fmtTexture)) const void *pvPixels,
    __deref_out_ecount(1) ID3D11Texture2D ** const ppD3DSysMemTexture
    )
{
    HRESULT hr = S_OK;

    *ppD3DSysMemTexture = NULL;

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = uWidth;
    desc.Height = uHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = fmtTexture;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA init;
    init.pSysMem = pvPixels;
    init.SysMemPitch = uWidth * D3DFormatSize(fmtTexture);
    init.SysMemSlicePitch = 0;

    IFC(CreateTexture(&desc, pvPixels ? &init : NULL, ppD3DSysMemTexture));

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::UpdateSurface
//
//  Synopsis:
//      Copies a rect from one texture (subresource) to another.
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::UpdateSurface(
    __in_ecount(1) ID3D11Texture2D *pD3DSrcTexture,
    UINT uSrcSubresource,
    __in_ecount_opt(1) const RECT *pSourceRect,
    __inout_ecount(1) ID3D11Texture2D *pD3DDestTexture,
    UINT uDestSubresource,
    __in_ecount_opt(1) const POINT *pDestPoint
    )
{
    HRESULT hr = S_OK;

    AssertDeviceEntry(*this);

    D3D11_BOX box;
    D3D11_BOX *pBox = NULL;

    if (pSourceRect)
    {
        RectToBox(*pSourceRect, &box);
        pBox = &box;
    }

    m_pD3DContext->CopySubresourceRegion(
        pD3DDestTexture,
        uDestSubresource,
        pDestPoint ? pDestPoint->x : 0,
        pDestPoint ? pDestPoint->y : 0,
        0,
        pD3DSrcTexture,
        uSrcSubresource,
        pBox
        );

    RRETURN(HandleDIE(hr));
}

HRESULT
CD3DDeviceLevel1::UpdateSurface(
    __in_ecount(1) CD3DSurface *pD3DSysMemSrcSurface,
    __in_ecount_opt(1) const RECT *pSourceRect,
    __inout_ecount(1) CD3DSurface *pD3DPoolDefaultDestSurface,
    __in_ecount_opt(1) const POINT *pDestPoint
    )
{
    return UpdateSurface(
        pD3DSysMemSrcSurface->ID3DTexture(),
        pD3DSysMemSrcSurface->Subresource(),
        pSourceRect,
        pD3DPoolDefaultDestSurface->ID3DTexture(),
        pD3DPoolDefaultDestSurface->Subresource(),
        pDestPoint
        );
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::UpdateTexture
//
//------------------------------------------------------------------------------
HRESULT CD3DDeviceLevel1::UpdateTexture(
    __in_ecount(1) ID3D11Texture2D *pD3DSrcTexture,
    __inout_ecount(1) ID3D11Texture2D *pD3DDestTexture
    )
{
    HRESULT hr = S_OK;

    m_pD3DContext->CopyResource(pD3DDestTexture, pD3DSrcTexture);

    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::StretchRect
//
//  Synopsis:
//      Copies (and resolves or scales) a rectangle between surfaces.
//
//------------------------------------------------------------------------------
HRESULT CD3DDeviceLevel1::StretchRect(
    __in_ecount(1) CD3DSurface *pSourceSurface,
    __in_ecount_opt(1) const RECT *pSourceRect,
    __inout_ecount(1) CD3DSurface *pDestSurface,
    __in_ecount_opt(1) const RECT *pDestRect,
    MilStretchFilter Filter
    )
{
    HRESULT hr = S_OK;

    AssertDeviceEntry(*this);

    RECT rcSrc;
    RECT rcDst;

    const D3D11_TEXTURE2D_DESC &descSrc = pSourceSurface->Desc();
    const D3D11_TEXTURE2D_DESC &descDst = pDestSurface->Desc();

    if (pSourceRect)
    {
        rcSrc = *pSourceRect;
    }
    else
    {
        rcSrc.left = 0; rcSrc.top = 0;
        rcSrc.right = descSrc.Width; rcSrc.bottom = descSrc.Height;
    }

    if (pDestRect)
    {
        rcDst = *pDestRect;
    }
    else
    {
        rcDst.left = 0; rcDst.top = 0;
        rcDst.right = descDst.Width; rcDst.bottom = descDst.Height;
    }

    if (g_pMediaControl)
    {
        g_lPixelsFilledPerFrame += (rcSrc.right - rcSrc.left) * (rcSrc.bottom - rcSrc.top);
    }

    bool fSameSize =
           (rcSrc.right - rcSrc.left) == (rcDst.right - rcDst.left)
        && (rcSrc.bottom - rcSrc.top) == (rcDst.bottom - rcDst.top);

    bool fFullSurfaces =
           rcSrc.left == 0 && rcSrc.top == 0
        && rcSrc.right == static_cast<LONG>(descSrc.Width) && rcSrc.bottom == static_cast<LONG>(descSrc.Height)
        && rcDst.left == 0 && rcDst.top == 0
        && rcDst.right == static_cast<LONG>(descDst.Width) && rcDst.bottom == static_cast<LONG>(descDst.Height);

    if (descSrc.SampleDesc.Count > 1)
    {
        if (fSameSize && fFullSurfaces && descDst.SampleDesc.Count <= 1)
        {
            m_pD3DContext->ResolveSubresource(
                pDestSurface->ID3DTexture(),
                pDestSurface->Subresource(),
                pSourceSurface->ID3DTexture(),
                pSourceSurface->Subresource(),
                descDst.Format
                );
        }
        else if (fSameSize && descDst.SampleDesc.Count <= 1 && descSrc.Format == descDst.Format)
        {
            IFC(ResolveMultisampleRect(pSourceSurface, rcSrc, pDestSurface, rcDst));
        }
        else
        {
            IFC(StretchRectScaled(pSourceSurface, rcSrc, pDestSurface, rcDst, Filter));
        }
    }
    else if (   fSameSize
             && descDst.SampleDesc.Count <= 1
             && descSrc.Format == descDst.Format)
    {
        POINT ptDst = { rcDst.left, rcDst.top };

        IFC(UpdateSurface(
            pSourceSurface->ID3DTexture(),
            pSourceSurface->Subresource(),
            &rcSrc,
            pDestSurface->ID3DTexture(),
            pDestSurface->Subresource(),
            &ptDst
            ));
    }
    else
    {
        IFC(StretchRectScaled(pSourceSurface, rcSrc, pDestSurface, rcDst, Filter));
    }

Cleanup:
    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::StretchRectScaled
//
//  Synopsis:
//      Scaled copy by drawing a textured quad into the destination.  The
//      current render target, depth buffer and clip are restored afterwards.
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::StretchRectScaled(
    __in_ecount(1) CD3DSurface *pSourceSurface,
    __in_ecount(1) const RECT &rcSource,
    __inout_ecount(1) CD3DSurface *pDestSurface,
    __in_ecount(1) const RECT &rcDest,
    MilStretchFilter Filter
    )
{
    HRESULT hr = S_OK;

    CD3DSurface *pPrevRTNoRef = m_pCurrentRenderTargetNoRef;
    CD3DSurface *pPrevDSNoRef = m_pDepthStencilBufferForCurrentRTNoRef;
    bool fPrevClipSet = IsClipSet();
    MilPointAndSizeL rcPrevClip = GetClip();

    ID3D11ShaderResourceView *pSRVNoRef = NULL;
    const D3D11_TEXTURE2D_DESC &descSrc = pSourceSurface->Desc();

    IFC(pSourceSurface->GetShaderResourceViewNoRef(&pSRVNoRef));

    IFC(SetRenderTarget(pDestSurface));
    IFC(SetDepthStencilSurface(NULL));
    IFC(SetClipRect(NULL));

    IFC(SetD3DTexture(0, pSRVNoRef));

    IFC(SetRenderState_Texture(
        TBM_COPY,
        TBA_None,
        (Filter == MilStretchFilter_Linear) ? MilBitmapInterpolationMode::Linear : MilBitmapInterpolationMode::NearestNeighbor,
        0
        ));

    IFC(SetSamplerAddressModes(0, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP));

    {
        FLOAT rLeft   = static_cast<FLOAT>(rcDest.left);
        FLOAT rTop    = static_cast<FLOAT>(rcDest.top);
        FLOAT rRight  = static_cast<FLOAT>(rcDest.right);
        FLOAT rBottom = static_cast<FLOAT>(rcDest.bottom);

        FLOAT ruLeft   = static_cast<FLOAT>(rcSource.left)   / static_cast<FLOAT>(descSrc.Width);
        FLOAT rvTop    = static_cast<FLOAT>(rcSource.top)    / static_cast<FLOAT>(descSrc.Height);
        FLOAT ruRight  = static_cast<FLOAT>(rcSource.right)  / static_cast<FLOAT>(descSrc.Width);
        FLOAT rvBottom = static_cast<FLOAT>(rcSource.bottom) / static_cast<FLOAT>(descSrc.Height);

        CD3DVertexBufferDUV2 *pBuffer;
        CD3DVertexXYZDUV2 *pVertex;

        IFC(StartPrimitive(&pBuffer));

        IFC(pBuffer->GetNewVertices(4, &pVertex));

        pVertex[0].SetXYDUV0(rLeft, rTop, 0xffffffff, ruLeft, rvTop);
        pVertex[1].SetXYDUV0(rLeft, rBottom, 0xffffffff, ruLeft, rvBottom);
        pVertex[2].SetXYDUV0(rRight, rBottom, 0xffffffff, ruRight, rvBottom);
        pVertex[3].SetXYDUV0(rRight, rTop, 0xffffffff, ruRight, rvTop);

        IFC(EndPrimitiveFan(&m_vbBufferDUV2));
    }

Cleanup:
    IGNORE_HR(SetD3DTexture(0, NULL));
    UnbindAllTextures();   // the source may become a render target next

    //
    // Restore the previous target state
    //

    if (pPrevRTNoRef)
    {
        MIL_THR_SECONDARY(SetRenderTarget(pPrevRTNoRef));
        MIL_THR_SECONDARY(SetDepthStencilSurface(pPrevDSNoRef));

        if (fPrevClipSet)
        {
            CMILSurfaceRect rcClip(
                rcPrevClip.X,
                rcPrevClip.Y,
                rcPrevClip.X + rcPrevClip.Width,
                rcPrevClip.Y + rcPrevClip.Height,
                LTRB_Parameters
                );
            MIL_THR_SECONDARY(SetClipRect(&rcClip));
        }
    }
    else
    {
        ReleaseUseOfRenderTarget(pDestSurface);
    }

    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::SetRenderTargetForEffectPipeline
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::SetRenderTargetForEffectPipeline(
    __in_ecount(1) CD3DSurface *pD3DSurface
    )
{
    AssertDeviceEntry(*this);

    HRESULT hr = S_OK;

    if (pD3DSurface == m_pCurrentRenderTargetNoRef)
    {
        goto Cleanup;
    }

    m_fInScene = false;

    m_desc = pD3DSurface->Desc();


    DropIncompatibleDepthStencil();

    IFC(SetRenderTargetInternal(pD3DSurface));
    m_pCurrentRenderTargetNoRef = pD3DSurface;

    SetClipSet(false);


    //
    // D3D9 SetRenderTarget reset the viewport and scissor rectangle to the
    // whole target; D3D11 keeps the previous ones, so do it here. The effect
    // pipeline supplies its own vertex transform, hence no clipping matrix.
    //
    {
        MilPointAndSizeL rcViewport;

        rcViewport.X = 0;
        rcViewport.Y = 0;
        rcViewport.Width = m_desc.Width;
        rcViewport.Height = m_desc.Height;

        if (SupportsScissorRect())
        {
            ScissorRectChanged(&rcViewport);
        }

        IFC(SetViewport(&rcViewport));
    }
    m_fInScene = true;

Cleanup:

    if (FAILED(hr) && m_pCurrentRenderTargetNoRef)
    {
        ReleaseUseOfRenderTarget(m_pCurrentRenderTargetNoRef);
    }

    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::DropIncompatibleDepthStencil
//
//  Synopsis:
//      D3D11 rejects OMSetRenderTargets when the depth stencil view does not
//      match the render target (sample count, size). Release the use of the
//      current depth buffer when the new target (m_desc) does not fit; 3D
//      rendering re-establishes depth state through EnsureDepthState.
//
//------------------------------------------------------------------------------
void
CD3DDeviceLevel1::DropIncompatibleDepthStencil()
{
    if (m_pDepthStencilBufferForCurrentRTNoRef)
    {
        const D3D11_TEXTURE2D_DESC &descDS = m_pDepthStencilBufferForCurrentRTNoRef->Desc();

        if (   descDS.SampleDesc.Count != m_desc.SampleDesc.Count
            || descDS.SampleDesc.Quality != m_desc.SampleDesc.Quality
            || descDS.Width != m_desc.Width
            || descDS.Height != m_desc.Height)
        {
            ReleaseUseOfDepthStencilSurface(m_pDepthStencilBufferForCurrentRTNoRef);
        }
    }
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::ResolveMultisampleRect
//
//  Synopsis:
//      Resolves a rectangle of a multisample surface into a single sample
//      surface. ResolveSubresource only resolves whole subresources, so the
//      source is resolved into a scratch texture and the rectangle is copied.
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::ResolveMultisampleRect(
    __in_ecount(1) CD3DSurface *pSourceSurface,
    __in_ecount(1) const RECT &rcSource,
    __inout_ecount(1) CD3DSurface *pDestSurface,
    __in_ecount(1) const RECT &rcDest
    )
{
    HRESULT hr = S_OK;

    const D3D11_TEXTURE2D_DESC &descSrc = pSourceSurface->Desc();

    if (m_pMSAAResolveScratch)
    {
        D3D11_TEXTURE2D_DESC descScratch;
        m_pMSAAResolveScratch->GetDesc(&descScratch);

        if (   descScratch.Width != descSrc.Width
            || descScratch.Height != descSrc.Height
            || descScratch.Format != descSrc.Format)
        {
            ReleaseInterface(m_pMSAAResolveScratch);
        }
    }

    if (m_pMSAAResolveScratch == NULL)
    {
        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width = descSrc.Width;
        desc.Height = descSrc.Height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = descSrc.Format;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_RENDER_TARGET;

        BEGIN_DEVICE_ALLOCATION;

        MilDx11CountCreate(MilDx11Site_MsaaScratch);
        hr = MilTranslateD3D11Error(m_pD3DDevice->CreateTexture2D(&desc, NULL, &m_pMSAAResolveScratch));

        END_DEVICE_ALLOCATION;

        IFC(hr);
    }

    m_pD3DContext->ResolveSubresource(
        m_pMSAAResolveScratch,
        0,
        pSourceSurface->ID3DTexture(),
        pSourceSurface->Subresource(),
        descSrc.Format
        );

    {
        D3D11_BOX box;
        box.left = rcSource.left;
        box.top = rcSource.top;
        box.front = 0;
        box.right = rcSource.right;
        box.bottom = rcSource.bottom;
        box.back = 1;

        m_pD3DContext->CopySubresourceRegion(
            pDestSurface->ID3DTexture(),
            pDestSurface->Subresource(),
            rcDest.left,
            rcDest.top,
            0,
            m_pMSAAResolveScratch,
            0,
            &box
            );
    }

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::SetRenderTarget
//
//  Synopsis:
//      1. Set the render target
//      2. Set the viewport, scissor and the view and projection matrices.
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::SetRenderTarget(
    __in_ecount(1) CD3DSurface *pD3DSurface
    )
{
    AssertDeviceEntry(*this);

    HRESULT hr = S_OK;

    if (pD3DSurface == m_pCurrentRenderTargetNoRef)
    {
        goto Cleanup;
    }

    m_fInScene = false;

    m_desc = pD3DSurface->Desc();

    FreAssert(pD3DSurface->ID3DTexture() != NULL);

    DropIncompatibleDepthStencil();

    IFC(SetRenderTargetInternal(pD3DSurface));
    m_pCurrentRenderTargetNoRef = pD3DSurface;

    SetClipSet(false);

    {
        MilPointAndSizeL rcViewport;

        rcViewport.X = 0;
        rcViewport.Y = 0;
        rcViewport.Width = m_desc.Width;
        rcViewport.Height = m_desc.Height;

        //
        // Reset the scissor rect to the whole target.
        //
        if (SupportsScissorRect())
        {
            ScissorRectChanged(&rcViewport);
        }

        //
        // Set the viewport since it has inherently changed.
        //
        IFC(SetViewport(&rcViewport));

        //
        // Set the surface to clipping matrix
        //

        IFC(SetSurfaceToClippingMatrix(
            &rcViewport
            ));
    }

    m_fInScene = true;

Cleanup:

    if (FAILED(hr) && m_pCurrentRenderTargetNoRef)
    {
        ReleaseUseOfRenderTarget(m_pCurrentRenderTargetNoRef);
    }

    RRETURN(HandleDIE(hr));
}


//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::ReleaseUseOfRenderTarget
//
//------------------------------------------------------------------------------
void
CD3DDeviceLevel1::ReleaseUseOfRenderTarget(
    __in_ecount(1) const CD3DSurface *pD3DSurface
    )
{
    AssertDeviceEntry(*this);

    if (pD3DSurface == m_pCurrentRenderTargetNoRef)
    {
        m_pCurrentRenderTargetNoRef = NULL;

        m_fInScene = false;

        IGNORE_HR(SetRenderTargetInternal(NULL));

        ReleaseUseOfDepthStencilSurface(m_pDepthStencilBufferForCurrentRTNoRef);
    }
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::ClearRenderTargetRects
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::ClearRenderTargetRects(
    DWORD dwCount,
    __in_ecount_opt(dwCount) const D3DRECT* pRects,
    D3DCOLOR d3dColor
    )
{
    HRESULT hr = S_OK;

    ID3D11RenderTargetView *pRTVNoRef = NULL;

    if (!m_pCurrentRenderTargetNoRef)
    {
        IFC(E_UNEXPECTED);
    }

    IFC(m_pCurrentRenderTargetNoRef->GetRenderTargetViewNoRef(&pRTVNoRef));

    {
        float rgColor[4];
        rgColor[0] = MIL_COLOR_GET_RED(d3dColor) / 255.0f;
        rgColor[1] = MIL_COLOR_GET_GREEN(d3dColor) / 255.0f;
        rgColor[2] = MIL_COLOR_GET_BLUE(d3dColor) / 255.0f;
        rgColor[3] = MIL_COLOR_GET_ALPHA(d3dColor) / 255.0f;

        if (dwCount == 0 || pRects == NULL)
        {
            //
            // D3D9 Clear was limited to the viewport and the scissor rectangle
            // (CHwSurfaceRenderTarget::Clear sets the clip and clears with no
            // rectangles). ClearRenderTargetView ignores both, so clear only
            // the clip rectangle when one is set.
            //
            if (IsClipSet())
            {
                const MilPointAndSizeL &rcClip = GetClip();
                D3D11_RECT rc = { rcClip.X, rcClip.Y, rcClip.X + rcClip.Width, rcClip.Y + rcClip.Height };
                m_pD3DContext1->ClearView(pRTVNoRef, rgColor, &rc, 1);
            }
            else
            {
                m_pD3DContext->ClearRenderTargetView(pRTVNoRef, rgColor);
            }
        }
        else
        {
            C_ASSERT(sizeof(D3DRECT) == sizeof(D3D11_RECT));
            m_pD3DContext1->ClearView(pRTVNoRef, rgColor, reinterpret_cast<const D3D11_RECT *>(pRects), dwCount);
        }
    }

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::Clear
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::Clear(
    DWORD dwCount,
    __in_ecount_opt(dwCount) const D3DRECT* pRects,
    DWORD dwFlags,
    D3DCOLOR d3dColor,
    FLOAT rZValue,
    int iStencilValue
    )
{
    AssertDeviceEntry(*this);

    HRESULT hr = S_OK;

    Assert((dwCount > 0) == (pRects != NULL));
    Assert(rZValue >= 0.0f && rZValue <= 1.0f);
    Assert(iStencilValue >= 0);

    if (IsDepthStencilSurfaceSmallerThan(m_desc.Width, m_desc.Height))
    {
        Assert((dwFlags & (D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL) ) == 0);

        IFC(SetDepthStencilSurfaceInternal(NULL));
    }

    if (dwFlags & D3DCLEAR_TARGET)
    {
        IFC(ClearRenderTargetRects(dwCount, pRects, d3dColor));
    }

    if (dwFlags & (D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL))
    {
        // Rect-limited depth/stencil clears are not used by callers.
        Assert(pRects == NULL);

        ID3D11DepthStencilView *pDSVNoRef = NULL;

        if (m_pDepthStencilBufferForCurrentRTNoRef)
        {
            IFC(m_pDepthStencilBufferForCurrentRTNoRef->GetDepthStencilViewNoRef(&pDSVNoRef));

            UINT uClearFlags = 0;
            if (dwFlags & D3DCLEAR_ZBUFFER) uClearFlags |= D3D11_CLEAR_DEPTH;
            if (dwFlags & D3DCLEAR_STENCIL) uClearFlags |= D3D11_CLEAR_STENCIL;

            m_pD3DContext->ClearDepthStencilView(pDSVNoRef, uClearFlags, rZValue, static_cast<UINT8>(iStencilValue));
        }
    }

#if DBG
    //
    // Draw zoom mode grid (if it is enabled)
    //

    if (DbgIsPixelZoomMode())
    {
        D3DRECT d3dRect;
        MilColorB colorGrid = MIL_COLOR(255, 190, 190, 190);

        if (dwFlags == D3DCLEAR_TARGET
            && pRects == NULL
            && m_pCurrentRenderTargetNoRef != NULL)
        {
            for (unsigned x = 0; x < m_desc.Width; x += c_dbgPixelZoomModeScale)
            {
                for (unsigned y = 0; y < m_desc.Height; y += c_dbgPixelZoomModeScale)
                {
                    if (((x + y) % (2*c_dbgPixelZoomModeScale)) == 0)
                    {
                        d3dRect.x1 = x;
                        d3dRect.y1 = y;
                        d3dRect.x2 = x + c_dbgPixelZoomModeScale;
                        d3dRect.y2 = y + c_dbgPixelZoomModeScale;

                        IGNORE_HR(ClearRenderTargetRects(1, &d3dRect, colorGrid));
                    }
                }
            }
        }
    }
#endif

Cleanup:
    RRETURN(HandleDIE(hr));
}


//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::ColorFill
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::ColorFill(
    __inout_ecount(1) CD3DSurface *pSurface,
    __in_ecount_opt(1) const RECT *pRect,
    D3DCOLOR color
    )
{
    HRESULT hr = S_OK;

    MtSetDefault(Mt(D3DColorFill));

    ID3D11RenderTargetView *pRTVNoRef = NULL;

    IFC(pSurface->GetRenderTargetViewNoRef(&pRTVNoRef));

    {
        float rgColor[4];
        rgColor[0] = MIL_COLOR_GET_RED(color) / 255.0f;
        rgColor[1] = MIL_COLOR_GET_GREEN(color) / 255.0f;
        rgColor[2] = MIL_COLOR_GET_BLUE(color) / 255.0f;
        rgColor[3] = MIL_COLOR_GET_ALPHA(color) / 255.0f;

        if (pRect)
        {
            D3D11_RECT rc = { pRect->left, pRect->top, pRect->right, pRect->bottom };
            m_pD3DContext1->ClearView(pRTVNoRef, rgColor, &rc, 1);
        }
        else
        {
            m_pD3DContext->ClearRenderTargetView(pRTVNoRef, rgColor);
        }
    }

Cleanup:
    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::CleanupFreedResources
//
//------------------------------------------------------------------------------
void
CD3DDeviceLevel1::CleanupFreedResources()
{
    m_resourceManager.DestroyReleasedResourcesFromLastFrame();
    m_resourceManager.DestroyResources(CD3DResourceManager::WithoutDelay);
}

//+-----------------------------------------------------------------------------
//
//  Function:
//      FillCurrentCumulativeMax
//
//------------------------------------------------------------------------------

static void
FillCurrentCumulativeMax(
    __out_ecount(1) DWORD *pCurrent,
    __inout_ecount(1) DWORD *pCumulative,
    __inout_ecount(1) DWORD *pMax,
    DWORD newValue)
{
    InterlockedExchange(reinterpret_cast<LONG *>(pCurrent), newValue);
    InterlockedExchangeAdd(reinterpret_cast<LONG *>(pCumulative), newValue);
    DWORD maxValue = *pMax > newValue ? *pMax : newValue;
    InterlockedExchange(reinterpret_cast<LONG *>(pMax), maxValue);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::Present
//
//  Synopsis:
//      Delegate to CD3DSwapChain::Present (DXGI or GDI)
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::Present(
    __in_ecount(1) CD3DSwapChain const *pD3DSwapChain,
    __in_ecount_opt(1) CMILSurfaceRect const *prcSource,
    __in_ecount_opt(1) CMILSurfaceRect const *prcDest,
    __in_ecount(1) CMILDeviceContext const *pMILDC,
    __in_ecount_opt(1) RGNDATA const * pDirtyRegion,
    DWORD dwD3DPresentFlags
    )
{
    AssertDeviceEntry(*this);

    HRESULT hr;

    if (FAILED(m_hrDisplayInvalid))
    {
        MarkUnusable(false /* This call is already entry protected. */);

        Assert(FAILED(m_hrDisplayInvalid));

        MIL_THR(m_hrDisplayInvalid);
        goto Cleanup;
    }

    Assert(pD3DSwapChain->GetDXGISwapChainNoRef());
    Assert(pD3DSwapChain->IsValid());

#if DBG==1
    if (IsTagEnabled(tagPixelZoomMode))
    {
        g_dbgHWND = pMILDC->GetHWND();
    }
#endif

    m_glyphBank.CollectGarbage();

    //
    // Update metrics
    //

    if (g_pMediaControl)
    {
        CMediaControlFile* pFile = g_pMediaControl->GetDataPtr();

        if (m_dwMetricsTrianglesPerFrame > 0)
        {
            FillCurrentCumulativeMax(&pFile->TrianglesPerFrame,
                                     &pFile->TrianglesPerFrameCumulative,
                                     &pFile->TrianglesPerFrameMax,
                                     m_dwMetricsTrianglesPerFrame);
        }

        FillCurrentCumulativeMax(&pFile->TextureUpdatesPerFrame,
                                 &pFile->TextureUpdatesPerFrameCumulative,
                                 &pFile->TextureUpdatesPerFrameMax,
                                 g_dwTextureUpdatesPerFrame);

        FillCurrentCumulativeMax(&pFile->PixelsFilledPerFrame,
                                 &pFile->PixelsFilledPerFrameCumulative,
                                 &pFile->PixelsFilledPerFrameMax,
                                 g_lPixelsFilledPerFrame);

        InterlockedExchange(reinterpret_cast<volatile LONG *>(&g_dwTextureUpdatesPerFrame), 0);
        InterlockedExchange(static_cast<LONG *>(&g_lPixelsFilledPerFrame), 0);

        MtSet(Mt(VerticesPerFrame), m_dwMetricsVerticesPerFrame, 0);
        m_dwMetricsVerticesPerFrame = 0;

        MtSet(Mt(TrianglesPerFrame), m_dwMetricsTrianglesPerFrame, 0);
        m_dwMetricsTrianglesPerFrame = 0;
    }

    bool fPresentProcessed = false;

    IF_D3DLOG(m_log.OnPresent();)

    if (pMILDC->PresentWithHAL())
    {
        MIL_THR(PresentWithD3D(
            pD3DSwapChain->GetDXGISwapChainNoRef(),
            prcSource,
            prcDest,
            pMILDC,
            pDirtyRegion,
            dwD3DPresentFlags,
            pD3DSwapChain->IsFlipModel(),
            &fPresentProcessed
            ));

        if (SUCCEEDED(hr) && pD3DSwapChain->IsFlipModel())
        {
            //
            // D29: DXGI rotated the buffers and unbound the back buffer.
            // Bring the new back buffer up to date and forget the cached
            // render target so the next frame binds it again.
            //

            pD3DSwapChain->SyncBackBufferAfterPresent(
                m_pD3DContext,
                prcSource,
                pDirtyRegion,
                fPresentProcessed
                );

            if (m_pCurrentRenderTargetNoRef)
            {
                ReleaseUseOfRenderTarget(m_pCurrentRenderTargetNoRef);
            }
        }

        IFC(hr);
    }
    else
    {
        IFC(PresentWithGDI(
            pD3DSwapChain,
            prcSource,
            prcDest,
            pMILDC,
            pDirtyRegion,
            &fPresentProcessed
            ));
    }

    if (fPresentProcessed)
    {
        if (!CCommonRegistryData::GPUThrottlingDisabled())
        {
            ULONGLONG ullPresentTime;

            m_uNumSuccessfulPresentsSinceMarkerFlush++;

            IFCW32(QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&ullPresentTime)));

            IFC(InsertGPUMarker(ullPresentTime));
        }
    }

Cleanup:

    RRETURN1(hr, S_PRESENT_OCCLUDED); // DIE already handled
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::PresentWithD3D
//
//  Synopsis:
//      Use DXGI to present from the swap chain.
//
//------------------------------------------------------------------------------

HRESULT
CD3DDeviceLevel1::PresentWithD3D(
    __inout_ecount(1) IDXGISwapChain1 *pDXGISwapChain,
    __in_ecount_opt(1) CMILSurfaceRect const *prcSource,
    __in_ecount_opt(1) CMILSurfaceRect const *prcDest,
    __in_ecount(1) CMILDeviceContext const *pMILDC,
    __in_ecount_opt(1) RGNDATA const * pDirtyRegion,
    DWORD dwD3DPresentFlags,
    bool fFlipModel,
    __out_ecount(1) bool *pfPresentProcessed
    )
{
    HRESULT hr = S_OK;

    Assert( pMILDC->PresentWithHAL() );

    UNREFERENCED_PARAMETER(prcDest);

    *pfPresentProcessed = false;

    //
    // Dirty rects (only when a source rect is given, like the D3D9 code)
    //

    DXGI_PRESENT_PARAMETERS params;
    ZeroMemory(&params, sizeof(params));

    //
    // D3D9 presented the whole source rectangle (the union of the dirty
    // rectangles) and used the dirty region only as a hint. DXGI copies exactly
    // the dirty rectangles, so present the union rectangle to keep the D3D9
    // behaviour; NULL (no source rectangle) presents the whole back buffer.
    //
    RECT rcDirty;

    UNREFERENCED_PARAMETER(pDirtyRegion);

    //
    // DXGI dirty rectangles only update the window surface inside the
    // rectangles. D3D9 recopied the whole back buffer whenever the window
    // surface was (re)created, which DXGI does not do, so a dirty-rect present
    // right after Show/Resize leaves the rest of the window black. Present the
    // whole back buffer; the rectangle is kept for the log only.
    //
    if (prcSource != NULL)
    {
        rcDirty.left = prcSource->left;
        rcDirty.top = prcSource->top;
        rcDirty.right = prcSource->right;
        rcDirty.bottom = prcSource->bottom;
    }

    params.DirtyRectsCount = 0;
    params.pDirtyRects = NULL;

    RECT rgDirtyRects[CD3DSwapChain::sc_cMaxDirtyRects];

    if (fFlipModel)
    {
        //
        // D29: flip model presents report what changed so DWM composes only
        // that. Rects must lie inside the buffer or DXGI rejects the call.
        //

        DXGI_SWAP_CHAIN_DESC1 descChain;

        if (SUCCEEDED(pDXGISwapChain->GetDesc1(&descChain)))
        {
            UINT cCollected = CD3DSwapChain::CollectDirtyRects(
                prcSource,
                pDirtyRegion,
                rgDirtyRects,
                ARRAYSIZE(rgDirtyRects)
                );

            for (UINT i = 0; i < cCollected; i++)
            {
                RECT rc = rgDirtyRects[i];

                rc.left = max(rc.left, 0L);
                rc.top = max(rc.top, 0L);
                rc.right = min(rc.right, static_cast<LONG>(descChain.Width));
                rc.bottom = min(rc.bottom, static_cast<LONG>(descChain.Height));

                if (rc.right > rc.left && rc.bottom > rc.top)
                {
                    rgDirtyRects[params.DirtyRectsCount++] = rc;
                }
            }

            params.pDirtyRects = (params.DirtyRectsCount > 0) ? rgDirtyRects : NULL;
        }
    }

    UINT uSyncInterval =
        (pMILDC->GetRTInitializationFlags() & MilRTInitialization::PresentImmediately) ? 0 : 1;

    UINT uFlags = 0;

    if (dwD3DPresentFlags & D3DPRESENT_DONOTWAIT)
    {
        uFlags |= DXGI_PRESENT_DO_NOT_WAIT;
    }

    //
    // Call present and check for mode change
    //

    BEGIN_DEVICE_ALLOCATION;

    hr = MilTranslateD3D11Error(pDXGISwapChain->Present1(
        uSyncInterval,
        uFlags,
        &params
        ));

    END_DEVICE_ALLOCATION;

    if (SUCCEEDED(hr) && MilDx11TestInjectDeviceLoss())
    {
        // Test hook (WPFGFX_DX11_TEST_DEVICE_LOSS): behave as if DXGI had
        // reported DXGI_ERROR_DEVICE_REMOVED on this present.
        MIL_DX11_LOG("test hook: injecting device loss on this Present");
        m_fTestDeviceLost = true;
        hr = D3DERR_DEVICELOST;
    }

    m_uPresentCount++;
    TrimTexturePool(false);

    static LONG s_cPresentLogs = 0;

    if (InterlockedIncrement(&s_cPresentLogs) <= 20)
    MIL_DX11_LOG("Present1 hr=0x%08x rect=%ld,%ld,%ld,%ld sync=%u",
                 hr,
                 prcSource ? rcDirty.left : -1, prcSource ? rcDirty.top : -1,
                 prcSource ? rcDirty.right : -1, prcSource ? rcDirty.bottom : -1,
                 uSyncInterval);

    MilDx11LogD3DMessages(m_pD3DDevice, "Present");

    if (MilDx11LogEnabled())
    {
        // Leak tracking: live object counts every 300 presents.
        static LONG s_cPresentsForLiveObjects = 0;
        LONG cPresents = InterlockedIncrement(&s_cPresentsForLiveObjects);
        if (cPresents % 300 == 0)
        {
            char szContext[48];
            StringCchPrintfA(szContext, ARRAYSIZE(szContext), "after %ld presents", cPresents);
            MilDx11LogLiveObjects(m_pD3DDevice, szContext);
            MilDx11StatsReport(300);
        }
    }

    if (MilDx11LogEnabled())
    {
        static LONG s_cPresentDumps = 0;
        LONG cDump = InterlockedIncrement(&s_cPresentDumps);

        if (cDump <= 4)
        {
            ID3D11Texture2D *pBackBuffer = NULL;

            if (SUCCEEDED(pDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&pBackBuffer))))
            {
                char szName[32];
                StringCchPrintfA(szName, ARRAYSIZE(szName), "present%d", cDump);
                MilDx11DumpTexture(m_pD3DDevice, m_pD3DContext, pBackBuffer, szName);
                pBackBuffer->Release();
            }
        }
    }

    DbgInjectDIE(&hr);

    if (hr == S_OK)
    {
        *pfPresentProcessed = true;
    }
    else if (hr == S_PRESENT_MODE_CHANGED)
    {
        //
        // Desktop Display Mode has changed.  Handle identically to device lost.
        //
        hr = D3DERR_DEVICELOST;
    }
    else if (hr == S_PRESENT_OCCLUDED)
    {
        //
        // Device is in a normal state but isn't visible.  Keep rendering as if
        // nothing has happened, but don't spin.
        //
        Sleep(100);
        PostMessage(pMILDC->GetHWND(), m_presentFailureWindowMessage, NULL, NULL);

        hr = S_OK;
    }

    MIL_THR(hr);

    if (FAILED(hr))
    {
        MIL_DX11_LOG("Present1 failed hr=0x%08x sync=%u flags=0x%x dirty=%u", hr, uSyncInterval, uFlags, params.DirtyRectsCount);
        hr = HandlePresentFailure(pMILDC, hr);
    }

    RRETURN1(hr, S_PRESENT_OCCLUDED);
}


//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::HandlePresentFailure
//
//------------------------------------------------------------------------------

HRESULT
CD3DDeviceLevel1::HandlePresentFailure(
    __in_ecount(1) CMILDeviceContext const *pMILDC,
    HRESULT hr
    )
{
    Assert(FAILED(hr));

    if (m_pCurrentRenderTargetNoRef)
    {
        m_pCurrentRenderTargetNoRef = NULL;
        m_fInScene = false;

        IGNORE_HR(SetRenderTargetInternal(NULL));

        ReleaseUseOfDepthStencilSurface(m_pDepthStencilBufferForCurrentRTNoRef);
    }

    if (!IsWindow(pMILDC->GetHWND()))
    {
        MIL_THR(HRESULT_FROM_WIN32(ERROR_INVALID_WINDOW_HANDLE));
    }
    else if (hr == E_FAIL || hr == D3DERR_DRIVERINTERNALERROR)
    {
        hr = D3DERR_DEVICELOST;
        IGNORE_HR(CD3DRegistryDatabase::HandleAdapterUnexpectedError(m_uAdapterOrdinal));
    }
    else if (hr == E_INVALIDARG)
    {
        RIPW(L"Present returned E_INVALIDARG");
        MIL_THR(WGXERR_NEED_RECREATE_AND_PRESENT);
    }

    if (hr == D3DERR_DEVICELOST ||
        hr == D3DERR_DEVICEHUNG ||
        hr == D3DERR_DEVICEREMOVED
        )
    {
        hr = WGXERR_DISPLAYSTATEINVALID;
        MarkUnusable(false /* This call is already entry protected. */);
    }

    return hr;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::PresentWithGDI
//
//  Synopsis:
//      Presents the backbuffer using a gdi bit blt or UpdateLayeredWindow.
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::PresentWithGDI(
    __in_ecount(1) CD3DSwapChain const *pD3DSwapChain,
    __in_ecount_opt(1) CMILSurfaceRect const * prcSource,
    __in_ecount_opt(1) CMILSurfaceRect const * prcDest,
    __in_ecount(1) CMILDeviceContext const * pMILDC,
    __in_ecount_opt(1) RGNDATA const * pDirtyRegion,
    __out_ecount(1) bool *pfPresentProcessed
    )
{
    HRESULT hr = S_OK;
    HDC hdcFrontBuffer = NULL;
    HDC hdcBackBuffer = NULL;

    CD3DSurface *pBackBufferSurface = NULL;

    CMilRectU rcSource;
    RECT rcDest;

    UINT uBufferWidth;
    UINT uBufferHeight;

    UNREFERENCED_PARAMETER(pDirtyRegion);

    Assert( !pMILDC->PresentWithHAL() );

#if DBG
    Assert(pD3DSwapChain->DbgGetNumBackBuffers() == 1);
#endif

    *pfPresentProcessed = false;

    IFC(pD3DSwapChain->GetBackBuffer(
        0,
        &pBackBufferSurface
        ));

    pBackBufferSurface->GetSurfaceSize(
        &uBufferWidth,
        &uBufferHeight
        );

    if (prcSource)
    {
        Assert(prcDest);

        Assert(prcSource->Width() == prcDest->Width());
        Assert(prcSource->Height() == prcDest->Height());

        rcDest = *prcDest;
        rcSource.left = prcSource->left;
        rcSource.top = prcSource->top;
        rcSource.right = prcSource->right;
        rcSource.bottom = prcSource->bottom;
    }
    else
    {
        Assert(!prcDest);

        rcSource.left = 0;
        rcSource.top = 0;
        rcSource.right = uBufferWidth;
        rcSource.bottom = uBufferHeight;

        rcDest.left   = 0;
        rcDest.top    = 0;
        rcDest.right  = uBufferWidth;
        rcDest.bottom = uBufferHeight;
    }

    Assert(rcDest.right > rcDest.left);
    Assert(rcDest.bottom > rcDest.top);

    IFC(pD3DSwapChain->GetDC(
        0,
        rcSource,
        OUT &hdcBackBuffer
        ));

    if (MilDx11LogEnabled())
    {
        // Diagnostics: the first two GDI-presented back buffers (layered
        // windows) go to the log folder as gdipresent1..2.bmp so the
        // premultiplied alpha handed to UpdateLayeredWindow can be inspected.
        static LONG s_cGdiPresentDumps = 0;
        LONG cDump = InterlockedIncrement(&s_cGdiPresentDumps);
        if (cDump <= 2)
        {
            char szName[32];
            StringCchPrintfA(szName, ARRAYSIZE(szName), "gdipresent%d", cDump);
            MilDx11DumpTexture(m_pD3DDevice, m_pD3DContext, pBackBufferSurface->ID3DTexture(), szName);
        }
    }

    *pfPresentProcessed = true;

    if ((pMILDC->GetRTInitializationFlags() & MilRTInitialization::PresentUsingMask) == MilRTInitialization::PresentUsingUpdateLayeredWindow)
    {
        SIZE sz = { static_cast<LONG>(uBufferWidth), static_cast<LONG>(uBufferHeight) };
        POINT ptSrc = { 0, 0 };
        HWND hWnd = pMILDC->GetHWND();
        double const msUlwStart = MilDx11QpcMs();
        hr = UpdateLayeredWindowEx(
            hWnd,
            NULL, // front buffer
            &pMILDC->GetPosition(),
            &sz,
            hdcBackBuffer,
            &ptSrc,
            pMILDC->GetColorKey(), // colorkey
            &pMILDC->GetBlendFunction(), // blendfunction
            pMILDC->GetULWFlags(), // flags
            prcSource
            );
        MilDx11TimingAdd(MilDx11Timing_Ulw, MilDx11QpcMs() - msUlwStart);
        if (hr == HRESULT_FROM_WIN32(ERROR_GEN_FAILURE))
        {
            hr = S_OK;
        }
        IFC(hr);
    }
    else if ((pMILDC->GetRTInitializationFlags() & MilRTInitialization::PresentUsingMask) == MilRTInitialization::PresentUsingBitBlt)
    {
        IFC(pMILDC->BeginRendering(
            &hdcFrontBuffer
            ));

        IFCW32_CHECKSAD(BitBlt(
            hdcFrontBuffer,
            rcDest.left,
            rcDest.top,
            rcDest.right - rcDest.left,
            rcDest.bottom - rcDest.top,
            hdcBackBuffer,
            rcSource.left,
            rcSource.top,
            SRCCOPY
            ));
    }
    else
    {
        // No support for AlphaBlend yet.
        IFC(E_NOTIMPL);
    }

Cleanup:

    if (FAILED(hr))
    {
        hr = HandlePresentFailure(pMILDC, hr);
    }

    if (hdcBackBuffer)
    {
        MIL_THR_SECONDARY(pD3DSwapChain->ReleaseDC(
            0,
            hdcBackBuffer
            ));
    }

    ReleaseInterfaceNoNULL(pBackBufferSurface);

    if (hdcFrontBuffer != NULL)
    {
        pMILDC->EndRendering(hdcFrontBuffer);
    }

    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::SetTexture
//
//  Synopsis:
//      Sets the texture for a particular stage
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::SetTexture(
    DWORD dwTextureStage,
    __inout_ecount_opt(1) CD3DTexture *pD3DTexture
    )
{
    AssertDeviceEntry(*this);

    HRESULT hr = S_OK;

    ID3D11ShaderResourceView *pSRVNoRef = NULL;

    if (pD3DTexture != NULL)
    {
        Assert(pD3DTexture->IsValid());

        Use(*pD3DTexture);

        IFC(pD3DTexture->EnsureGpuContents());

        IFC(pD3DTexture->GetShaderResourceViewNoRef(&pSRVNoRef));
    }

    if (MilDx11LogEnabled())
    {
        static LONG s_cTextureLogs = 0;

        if (InterlockedIncrement(&s_cTextureLogs) <= 300)
        {
            if (pD3DTexture)
            {
                const D3D11_TEXTURE2D_DESC &d = pD3DTexture->D3DSurface0Desc();
                MIL_DX11_LOG("SetTexture stage=%u %ux%u fmt=%d mips=%u srv=%p", dwTextureStage, d.Width, d.Height, d.Format, d.MipLevels, pSRVNoRef);
            }
            else
            {
                MIL_DX11_LOG("SetTexture stage=%u NULL", dwTextureStage);
            }
        }
    }

    IFC(CD3DRenderState::SetTexture(dwTextureStage, pSRVNoRef));

Cleanup:
    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::SetD3DTexture
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::SetD3DTexture(
    DWORD dwTextureStage,
    __in_ecount_opt(1) ID3D11ShaderResourceView *pSRV
    )
{
    AssertDeviceEntry(*this);

    HRESULT hr = S_OK;

    IFC(CD3DRenderState::SetTexture(dwTextureStage, pSRV));

Cleanup:
    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::FlushBufferFan
//
//  Synopsis:
//      Draws the vertexbuffer assuming the primitive type is a fan.
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::FlushBufferFan(
    __in_ecount(1) CD3DVertexBuffer *pBuffer
    )
{
    HRESULT hr = S_OK;
    UINT cVertices = pBuffer->GetNumVertices();

    Assert(m_fInScene);

    if (cVertices > 2)
    {
        IFC(DrawTriangleFanUP(
            cVertices,
            pBuffer->GetVertices(),
            pBuffer->GetVertexStride()
            ));
    }

Cleanup:
    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::Set3DTransforms
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::Set3DTransforms(
    __in_ecount(1) CMILMatrix const *pWorldTransform3D,
    __in_ecount(1) CMILMatrix const *pViewTransform3D,
    __in_ecount(1) CMILMatrix const *pProjectionTransform3D,
    __in_ecount(1) CMatrix<CoordinateSpace::Projection3D,CoordinateSpace::Device> const &matHomogeneousTo2DDevice
    )
{
    HRESULT hr = S_OK;
    CMILMatrix matProjectionModifier;
    CMILMatrix mat3DViewportProjection;

    IFC(SetWorldTransform(
        pWorldTransform3D
        ));

    IFC(SetNonWorldTransform(
        MilTransform_View,
        pViewTransform3D
        ));

    matProjectionModifier = matHomogeneousTo2DDevice * m_matSurfaceToClip;

    mat3DViewportProjection = *pProjectionTransform3D * matProjectionModifier;

    IFC(SetNonWorldTransform(
        MilTransform_Projection,
        &mat3DViewportProjection
        ));

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::SetSurfaceToClippingMatrix
//
//  Synopsis:
//      Calculate surface space to homogeneous clipping (~viewport) matrix for
//      2D and 3D rendering.  See the D3D9 derivation: with D3D11's half
//      integer pixel centers the +1/V.W terms disappear.
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::SetSurfaceToClippingMatrix(
    __in_ecount(1) const MilPointAndSizeL *prcViewport
    )
{
    HRESULT hr = S_OK;

    m_matSurfaceToClip.reset_to_identity();

    //
    // D3D11 (unlike D3D9) places pixel centers at half integers, exactly like
    // our surface space.  So the transform is a plain scale and translate:
    //
    //      x' = x * 2/V.W - 1 - V.L*2/V.W
    //      y' = -(y * 2/V.H - 1 - V.T*2/V.H)
    //

    FLOAT flRecipViewWidth = 1.0f / static_cast<FLOAT>(prcViewport->Width);

    m_matSurfaceToClip._11 = 2.0f * flRecipViewWidth;

    m_matSurfaceToClip._41 =
        - (  static_cast<FLOAT>(prcViewport->X) * m_matSurfaceToClip._11
           + 1);

    FLOAT flRecipViewHeight = 1.0f / static_cast<FLOAT>(prcViewport->Height);

    m_matSurfaceToClip._22 = -2.0f * flRecipViewHeight;

    m_matSurfaceToClip._42 =
        - static_cast<FLOAT>(prcViewport->Y) * m_matSurfaceToClip._22
        + 1;

    IFC(Define2DTransforms(
        &m_matSurfaceToClip
        ));

    IFC(Set2DTransformForFixedFunction());

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::CreatePixelShaderFromResource
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::CreatePixelShaderFromResource(
    UINT uResourceId,
    __deref_out_ecount(1) ID3D11PixelShader ** const ppPixelShader
    )
{
    HRESULT hr = S_OK;
    HGLOBAL hResource = NULL;
    HRSRC hResourceInfo = NULL;
    const void *pData = NULL;
    DWORD cbData = 0;

    Assert(*ppPixelShader == NULL);

    IFCW32(hResourceInfo = FindResource(
        g_DllInstance,
        MAKEINTRESOURCE(uResourceId),
        RT_RCDATA
        ));

    IFCW32(hResource = LoadResource(
        g_DllInstance,
        hResourceInfo
        ));

    cbData = SizeofResource(g_DllInstance, hResourceInfo);

    pData = LockResource(hResource);

    if (pData == NULL || cbData == 0)
    {
        AssertMsg(0, "Error-couldn't load shader resource");
        IFC(E_FAIL);
    }

    IFC(CreatePixelShader(pData, cbData, ppPixelShader));

Cleanup:
    return hr;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::CreateVertexShaderFromResource
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::CreateVertexShaderFromResource(
    UINT uResourceId,
    __deref_out_ecount(1) ID3D11VertexShader ** const ppVertexShader
    )
{
    HRESULT hr = S_OK;
    HGLOBAL hResource = NULL;
    HRSRC hResourceInfo = NULL;
    const void *pData = NULL;
    DWORD cbData = 0;

    Assert(*ppVertexShader == NULL);

    IFCW32(hResourceInfo = FindResource(g_DllInstance, MAKEINTRESOURCE(uResourceId), RT_RCDATA));
    IFCW32(hResource = LoadResource(g_DllInstance, hResourceInfo));

    cbData = SizeofResource(g_DllInstance, hResourceInfo);

    pData = LockResource(hResource);

    if (pData == NULL || cbData == 0)
    {
        AssertMsg(0, "Error-couldn't load shader resource");
        IFC(E_FAIL);
    }

    IFC(CreateVertexShader(pData, cbData, ppVertexShader));

Cleanup:
    RRETURN(hr);
}


//+-----------------------------------------------------------------------------
//
//  Function:  MilDx11LogShaderBindings
//
//  Synopsis:  진단용. 컴파일된 셰이더의 리소스 바인딩(텍스처/샘플러/상수 버퍼 슬롯)과.
//             $Globals 변수 오프셋을 로그에 쓴다.
//
//------------------------------------------------------------------------------
static void
MilDx11LogShaderBindings(
    __in_z const char *pszKind,
    __in_bcount(cb) const void *pvBytecode,
    SIZE_T cb
    )
{
    if (!MilDx11LogEnabled())
    {
        return;
    }

    ID3D11ShaderReflection *pReflection = NULL;

    if (SUCCEEDED(D3DReflect(pvBytecode, cb, __uuidof(ID3D11ShaderReflection), reinterpret_cast<void **>(&pReflection))))
    {
        D3D11_SHADER_DESC desc;

        if (SUCCEEDED(pReflection->GetDesc(&desc)))
        {
            for (UINT i = 0; i < desc.BoundResources; i++)
            {
                D3D11_SHADER_INPUT_BIND_DESC bind;

                if (SUCCEEDED(pReflection->GetResourceBindingDesc(i, &bind)))
                {
                    MIL_DX11_LOG("%s bind: %s type=%d point=%u dim=%d", pszKind, bind.Name, bind.Type, bind.BindPoint, bind.Dimension);
                }
            }

            for (UINT i = 0; i < desc.ConstantBuffers; i++)
            {
                ID3D11ShaderReflectionConstantBuffer *pCB = pReflection->GetConstantBufferByIndex(i);
                D3D11_SHADER_BUFFER_DESC cbDesc;

                if (pCB && SUCCEEDED(pCB->GetDesc(&cbDesc)))
                {
                    for (UINT v = 0; v < cbDesc.Variables; v++)
                    {
                        ID3D11ShaderReflectionVariable *pVar = pCB->GetVariableByIndex(v);
                        D3D11_SHADER_VARIABLE_DESC varDesc;

                        if (pVar && SUCCEEDED(pVar->GetDesc(&varDesc)))
                        {
                            MIL_DX11_LOG("%s cbuffer %s: %s offset=%u size=%u", pszKind, cbDesc.Name, varDesc.Name, varDesc.StartOffset, varDesc.Size);
                        }
                    }
                }
            }
        }

        pReflection->Release();
    }
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::CompilePipelineVertexShader
//
//------------------------------------------------------------------------------

HRESULT
CD3DDeviceLevel1::CompilePipelineVertexShader(
    __in_bcount(cbHLSLSource) PCSTR pHLSLSource,
    UINT cbHLSLSource,
    __deref_out_ecount(1) ID3D11VertexShader ** const ppVertexShader
)
{
    MtSetDefault(Mt(D3DCompiler));

    std::shared_ptr<buffer> pShader;
    std::shared_ptr<buffer> pErr;

    std::string profile_name = shader::get_vertex_shader_profile_name();

    if (MilDx11LogEnabled())
    {
        MilDx11LogText("pipeline HLSL", pHLSLSource, cbHLSLSource);
    }

    // 파이프라인 HLSL 은 D3D9 문법(POSITION/COLOR 시맨틱, tex2D)을 쓴다.
    // 변환기를 SM4 전용으로 바꿀 때까지(M3) 호환 모드로 컴파일한다.
    // MIL_VS: the converter guards the pixel shader half of the source so
    // the pixel constants do not take space in this stage's $Globals (D11).
    HRESULT hr =
        shader::compile(
            std::string("#define MIL_VS 1\n") + std::string(pHLSLSource, pHLSLSource + cbHLSLSource),
            "VertexShaderImpl",
            profile_name,
            D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY | D3DCOMPILE_OPTIMIZATION_LEVEL3, 0,
            pShader, pErr);

    if (SUCCEEDED(hr))
    {
        dxlayer::data blob = pShader->get_buffer_data();
        MilDx11LogShaderBindings("pipeline VS", blob.buffer, static_cast<SIZE_T>(blob.buffer_size));
        hr = CreateVertexShader(blob.buffer, static_cast<SIZE_T>(blob.buffer_size), ppVertexShader);
    }

    if (FAILED(hr))
    {
        MIL_DX11_LOG("pipeline VS compile failed hr=0x%08x: %s", hr,
                     pErr ? static_cast<const char *>(pErr->get_buffer_data().buffer) : "");
        hr = shader::handle_errors_and_transform_hresult(hr, pErr);
    }

    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::CompilePipelinePixelShader
//
//------------------------------------------------------------------------------

HRESULT
CD3DDeviceLevel1::CompilePipelinePixelShader(
    __in_bcount(cbHLSLSource) PCSTR pHLSLSource,
    UINT cbHLSLSource,
    __deref_out_ecount(1) ID3D11PixelShader ** const ppPixelShader
    )
{
    MtSetDefault(Mt(D3DCompiler));

    std::shared_ptr<buffer> pShader;
    std::shared_ptr<buffer> pErr;

    std::string profile_name = shader::get_pixel_shader_profile_name();

    HRESULT hr =
        shader::compile(
            std::string("#define MIL_PS 1\n") + std::string(pHLSLSource, pHLSLSource + cbHLSLSource),
            "PixelShaderImpl",
            profile_name,
            D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY | D3DCOMPILE_OPTIMIZATION_LEVEL3, 0,
            pShader, pErr);

    if (SUCCEEDED(hr))
    {
        dxlayer::data blob = pShader->get_buffer_data();
        MilDx11LogShaderBindings("pipeline PS", blob.buffer, static_cast<SIZE_T>(blob.buffer_size));
        hr = CreatePixelShader(blob.buffer, static_cast<SIZE_T>(blob.buffer_size), ppPixelShader);
    }

    if (!SUCCEEDED(hr))
    {
        MIL_DX11_LOG("pipeline PS compile failed hr=0x%08x: %s", hr,
                     pErr ? static_cast<const char *>(pErr->get_buffer_data().buffer) : "");
        hr = shader::handle_errors_and_transform_hresult(hr, pErr);
    }

    RRETURN(HandleDIE(hr));
}


//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::CompileShaderEffectPixelShader
//
//  Synopsis:
//      Compiles the HLSL produced by the ShaderEffect translator (entry
//      "main", ps_4_0). Errors go to the diagnostic log; the caller falls back
//      to the pass-through shader.
//
//------------------------------------------------------------------------------

HRESULT
CD3DDeviceLevel1::CompileShaderEffectPixelShader(
    __in_bcount(cbHLSLSource) PCSTR pHLSLSource,
    UINT cbHLSLSource,
    __deref_out_ecount(1) ID3D11PixelShader ** const ppPixelShader
    )
{
    MtSetDefault(Mt(D3DCompiler));

    std::shared_ptr<buffer> pShader;
    std::shared_ptr<buffer> pErr;

    if (MilDx11LogEnabled())
    {
        MilDx11LogText("ShaderEffect HLSL", pHLSLSource, cbHLSLSource);
    }

    HRESULT hr =
        shader::compile(
            std::string(pHLSLSource, pHLSLSource + cbHLSLSource),
            "main",
            "ps_4_0",
            D3DCOMPILE_OPTIMIZATION_LEVEL3, 0,
            pShader, pErr);

    if (SUCCEEDED(hr))
    {
        dxlayer::data blob = pShader->get_buffer_data();
        MilDx11LogShaderBindings("ShaderEffect PS", blob.buffer, static_cast<SIZE_T>(blob.buffer_size));
        hr = CreatePixelShader(blob.buffer, static_cast<SIZE_T>(blob.buffer_size), ppPixelShader);
    }

    if (FAILED(hr))
    {
        MIL_DX11_LOG("ShaderEffect PS compile failed hr=0x%08x: %s", hr,
                     pErr ? static_cast<const char *>(pErr->get_buffer_data().buffer) : "");
        hr = shader::handle_errors_and_transform_hresult(hr, pErr);
    }

    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::CreateVertexShader / CreatePixelShader
//
//------------------------------------------------------------------------------

HRESULT
CD3DDeviceLevel1::CreateVertexShader(
    __in_bcount(cbBytecode) const void *pvBytecode,
    SIZE_T cbBytecode,
    __deref_out_ecount(1) ID3D11VertexShader ** const ppOutShader
    )
{
    HRESULT hr = S_OK;

    BEGIN_DEVICE_ALLOCATION;

    hr = MilTranslateD3D11Error(m_pD3DDevice->CreateVertexShader(
        pvBytecode,
        cbBytecode,
        NULL,
        ppOutShader
        ));

    END_DEVICE_ALLOCATION;

    MIL_THR(hr);

    RRETURN(HandleDIE(hr));
}

HRESULT
CD3DDeviceLevel1::CreatePixelShader(
    __in_bcount(cbBytecode) const void *pvBytecode,
    SIZE_T cbBytecode,
    __deref_out_ecount(1) ID3D11PixelShader ** const ppOutShader
    )
{
    HRESULT hr = S_OK;

    BEGIN_DEVICE_ALLOCATION;

    hr = MilTranslateD3D11Error(m_pD3DDevice->CreatePixelShader(
        pvBytecode,
        cbBytecode,
        NULL,
        ppOutShader
        ));

    END_DEVICE_ALLOCATION;

    MIL_THR(hr);

    RRETURN(HandleDIE(hr));
}


//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::RenderTexture
//
//  Synopsis:
//      Renders the upper-left portion of the given texture 1:1 on the current
//      render target
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::RenderTexture(
    __in_ecount(1) CD3DTexture *pD3DTexture,
    __in_ecount(1) const MilPointAndSizeL &rcDestination,
    TextureBlendMode blendMode
    )
{
    AssertDeviceEntry(*this);

    HRESULT hr = S_OK;

    CD3DVertexXYZDUV2 *pVertex;
    FLOAT rLeft, rTop, rRight, rBottom;
    FLOAT ruRight, rvBottom;

    Assert(rcDestination.X >= 0);
    Assert(rcDestination.Y >= 0);
    Assert(rcDestination.Width > 0);
    Assert(rcDestination.Height > 0);
    Assert(static_cast<UINT>(rcDestination.X) < m_desc.Width);
    Assert(static_cast<UINT>(rcDestination.Y) < m_desc.Height);
    Assert(static_cast<UINT>(rcDestination.X) +
            static_cast<UINT>(rcDestination.Width) <= m_desc.Width);
    Assert(static_cast<UINT>(rcDestination.Y) +
            static_cast<UINT>(rcDestination.Height) <= m_desc.Height);

    UINT uTexWidth, uTexHeight;
    pD3DTexture->GetTextureSize(&uTexWidth, &uTexHeight);

    Assert(uTexWidth > 0);
    Assert(uTexHeight > 0);

    Assert(static_cast<UINT>(rcDestination.Width) <= uTexWidth);
    Assert(static_cast<UINT>(rcDestination.Height) <= uTexHeight);

    rLeft   = static_cast<FLOAT>(rcDestination.X);
    rTop    = static_cast<FLOAT>(rcDestination.Y);
    rRight  = rLeft + static_cast<FLOAT>(rcDestination.Width);
    rBottom = rTop + static_cast<FLOAT>(rcDestination.Height);

    ruRight = static_cast<FLOAT>(rcDestination.Width) / static_cast<FLOAT>(uTexWidth);
    rvBottom = static_cast<FLOAT>(rcDestination.Height) / static_cast<FLOAT>(uTexHeight);

    Assert(m_fInScene);

    IFC(SetTexture(0, pD3DTexture));

    IFC(SetRenderState_Texture(
        blendMode,
        TBA_Diffuse,
        MilBitmapInterpolationMode::NearestNeighbor,
        0
        ));

    CD3DVertexBufferDUV2 *pBuffer;

    IFC(StartPrimitive(&pBuffer));

    IFC(pBuffer->GetNewVertices(
        4,
        &pVertex
        ));

    pVertex[0].SetXYDUV0(rLeft, rTop, 0xffffffff, 0.0f, 0.0f);
    pVertex[1].SetXYDUV0(rLeft, rBottom, 0xffffffff, 0.0f, rvBottom);
    pVertex[2].SetXYDUV0(rRight, rBottom, 0xffffffff, ruRight, rvBottom);
    pVertex[3].SetXYDUV0(rRight, rTop, 0xffffffff, ruRight, 0.0f);

    IFC(EndPrimitiveFan(&m_vbBufferDUV2));

Cleanup:
    RRETURN(HandleDIE(hr));
}


//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::GetSupportedTextureFormat
//
//------------------------------------------------------------------------------

HRESULT
CD3DDeviceLevel1::GetSupportedTextureFormat(
    MilPixelFormat::Enum fmtBitmapSource,
    MilPixelFormat::Enum fmtDestinationSurface,
    bool fForceAlpha,
    __out_ecount(1)
    MilPixelFormat::Enum &fmtTextureSource
    ) const
{
    bool fUseAlpha = fForceAlpha || HasAlphaChannel(fmtBitmapSource);

    if (fmtDestinationSurface == MilPixelFormat::BGR32bpp101010)
    {
        if (fmtBitmapSource == MilPixelFormat::RGB128bppFloat)
        {
            fmtTextureSource = fUseAlpha
                               ? m_fmtSupportFor128bppPRGBAFloat
                               : m_fmtSupportFor128bppRGBFloat;
        }
        else
        {
            Assert(GetPixelFormatSize(MilPixelFormat::BGR32bpp101010) == 32);
            if ((GetPixelFormatSize(fmtBitmapSource) <= 32) && !fUseAlpha)
            {
                fmtTextureSource = m_fmtSupportFor32bppBGR101010;
            }
            else
            {
                fmtTextureSource = m_fmtSupportFor128bppPRGBAFloat;
            }
        }

        Assert(   (fmtTextureSource == MilPixelFormat::BGR32bpp101010)
               || (fmtTextureSource == MilPixelFormat::RGB128bppFloat)
               || (fmtTextureSource == MilPixelFormat::PRGBA128bppFloat)
               || (fmtTextureSource == MilPixelFormat::Undefined));
    }
    else
    {
        if (!fUseAlpha)
        {
            fmtTextureSource = m_fmtSupportFor32bppBGR;
        }
        else
        {
            fmtTextureSource = m_fmtSupportFor32bppPBGRA;
        }

        Assert(   (fmtTextureSource == MilPixelFormat::BGR32bpp)
               || (fmtTextureSource == MilPixelFormat::PBGRA32bpp)
               || (fmtTextureSource == MilPixelFormat::BGR32bpp101010)
               || (fmtTextureSource == MilPixelFormat::RGB128bppFloat)
               || (fmtTextureSource == MilPixelFormat::PRGBA128bppFloat)
               || (fmtTextureSource == MilPixelFormat::Undefined));
    }

    HRESULT hr = S_OK;

    if (fmtTextureSource == MilPixelFormat::Undefined)
    {
        MIL_THR(WGXERR_UNSUPPORTEDPIXELFORMAT);
    }

    RRETURN(hr);
}


//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::GetSupportedMultisampleType
//
//------------------------------------------------------------------------------

DXGI_SAMPLE_DESC
CD3DDeviceLevel1::GetSupportedMultisampleType(
    MilPixelFormat::Enum fmtDestinationSurface
    ) const
{
    DXGI_SAMPLE_DESC sd;

    if (fmtDestinationSurface == MilPixelFormat::BGR32bpp)
    {
        sd = m_multisampleTypeFor32bppBGR;
    }
    else if (fmtDestinationSurface == MilPixelFormat::PBGRA32bpp)
    {
        sd = m_multisampleTypeFor32bppPBGRA;
    }
    else
    {
        Assert(fmtDestinationSurface == MilPixelFormat::BGR32bpp101010);
        sd = m_multisampleTypeFor32bppBGR101010;
    }

    return sd;
}


//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::SetClipRect
//
//  Synopsis:
//      Provide access aligned clipping using the scissor rect (or viewport).
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::SetClipRect(
    __in_ecount_opt(1) const CMILSurfaceRect *prcClip
    )
{
    HRESULT hr = S_OK;
    MilPointAndSizeL rcTargetSurface = {0, 0, static_cast<INT>(m_desc.Width), static_cast<INT>(m_desc.Height)};
    MilPointAndSizeL rcSurfaceIntersectClip;
    const MilPointAndSizeL *prcNewClip = NULL;

    if (prcClip)
    {
        MilPointAndSizeL rcClip = {
            prcClip->left,
            prcClip->top,
            prcClip->right - prcClip->left,
            prcClip->bottom - prcClip->top
        };

        if (!IntersectRect(OUT &rcSurfaceIntersectClip, &rcTargetSurface, &rcClip))
        {
            hr = WGXHR_CLIPPEDTOEMPTY;
            goto Cleanup;
        }

        if (!IsClipSet(&rcSurfaceIntersectClip))
        {
            prcNewClip = &rcSurfaceIntersectClip;
        }
    }
    else
    {
        if (IsClipSet())
        {
            prcNewClip = &rcTargetSurface;
        }
    }

    if (prcNewClip)
    {
        if (SupportsScissorRect())
        {
            if (prcNewClip == &rcTargetSurface ||
                RtlEqualMemory(prcNewClip, &rcTargetSurface, sizeof(*prcNewClip)))
            {
                IFC(SetScissorRect(NULL));
            }
            else
            {
                IFC(SetScissorRect(prcNewClip));
            }
        }
        else
        {
            IFC(SetViewport(prcNewClip));

            IFC(SetSurfaceToClippingMatrix(
                prcNewClip
                ));
        }

        SetClipSet(prcClip != NULL);
        SetClip(*prcNewClip);

        EventWriteSetClipInfo(prcNewClip->X,
                              prcNewClip ->Y,
                              prcNewClip->Width,
                              prcNewClip->Height);
    }

Cleanup:
    RRETURN1(hr, WGXHR_CLIPPEDTOEMPTY);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::GetClipRect
//
//------------------------------------------------------------------------------
void
CD3DDeviceLevel1::GetClipRect(
    __out_ecount(1) MilPointAndSizeL * const prcClipRect
    ) const
{
    if (IsClipSet())
    {
        *prcClipRect = GetClip();
    }
    else
    {
        prcClipRect->X = 0;
        prcClipRect->Y = 0;
        prcClipRect->Width = m_desc.Width;
        prcClipRect->Height = m_desc.Height;
    }
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::DbgTraceDeviceCreationFailure
//
//------------------------------------------------------------------------------
#if DBG==1
void
CD3DDeviceLevel1::DbgTraceDeviceCreationFailure(
    UINT uAdapter,
    __in PCSTR szMessage,
    HRESULT hrError
    )
{
    TraceTag((tagError, "MIL-HW(adapter=%d): Can't create d3d rendering device.", uAdapter));
    TraceTag((tagError, "MIL-HW(adapter=%d): %s (hr = 0x%x).", uAdapter, szMessage, hrError));
}
#endif

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::CreateDepthBuffer
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::CreateDepthBuffer(
    UINT uWidth,
    UINT uHeight,
    __in_ecount(1) const DXGI_SAMPLE_DESC &sampleDesc,
    __deref_out_ecount(1) CD3DSurface ** const ppSurface
    )
{
    HRESULT hr = S_OK;
    ID3D11Texture2D *pD3DTexture = NULL;

    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = uWidth;
    desc.Height = uHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = kD3DDepthFormat;
    desc.SampleDesc = sampleDesc;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    {
        MtSetDefault(Mt(D3DStencilSurface));

        BEGIN_DEVICE_ALLOCATION;

        MilDx11CountCreate(MilDx11Site_DepthStencil);
        hr = MilTranslateD3D11Error(m_pD3DDevice->CreateTexture2D(&desc, NULL, &pD3DTexture));

        //
        // Out of video memory while multisampling: let the caller reduce the
        // sample count instead of freeing more resources.
        //
        if ((hr == D3DERR_OUTOFVIDEOMEMORY) && (sampleDesc.Count > 1))
        {
            break;
        }

        END_DEVICE_ALLOCATION;

        IFC(hr);
    }

    IFC(CD3DSurface::Create(&m_resourceManager, pD3DTexture, 0, ppSurface));

Cleanup:
    ReleaseInterfaceNoNULL(pD3DTexture);
    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::SetDepthStencilSurface
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::SetDepthStencilSurface(
    __in_ecount_opt(1) CD3DSurface *pSurface
    )
{
    HRESULT hr = S_OK;

    if (pSurface)
    {
        IFC(SetDepthEnable(TRUE));
    }
    else
    {
        IFC(SetDepthEnable(FALSE));
        IFC(SetStencilEnable(FALSE));
    }

    Assert(m_pCurrentRenderTargetNoRef);
    m_pDepthStencilBufferForCurrentRTNoRef = pSurface;

    IFC(SetDepthStencilSurfaceInternal(pSurface));

Cleanup:
    if (FAILED(hr))
    {
        IGNORE_HR(SetDepthEnable(FALSE));
        IGNORE_HR(SetStencilEnable(FALSE));
        IGNORE_HR(SetDepthStencilSurfaceInternal(NULL));
    }
    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::ReleaseUseOfDepthStencilSurface
//
//------------------------------------------------------------------------------
void
CD3DDeviceLevel1::ReleaseUseOfDepthStencilSurface(
    __in_ecount_opt(1) CD3DSurface *pSurface
    )
{
    AssertDeviceEntry(*this);

    if (pSurface)
    {
        IGNORE_HR(ReleaseUseOfDepthStencilSurfaceInternal(pSurface));

        if (m_pDepthStencilBufferForCurrentRTNoRef == pSurface)
        {
            m_pDepthStencilBufferForCurrentRTNoRef = NULL;
        }
    }
}

#if DBG_STEP_RENDERING
HRESULT
CD3DDeviceLevel1::DbgSaveSurface(
    __in_ecount(1) CD3DSurface *pD3DSurface,
    __in_ecount(1) const MilPointAndSizeL &rcSave
    )
{
    HRESULT hr;

    Assert(!m_pDbgSaveSurface);

    D3D11_TEXTURE2D_DESC const &desc = pD3DSurface->Desc();
    DXGI_SAMPLE_DESC sd = { 1, 0 };

    MIL_THR(CreateRenderTarget(
        rcSave.Width,
        rcSave.Height,
        desc.Format,
        sd,
        false,
        &m_pDbgSaveSurface
        ));

    if (SUCCEEDED(hr))
    {
        RECT rcSrc = {
            rcSave.X, rcSave.Y,
            rcSave.X + rcSave.Width, rcSave.Y + rcSave.Height
            };

        RECT rcDst = { 0, 0, rcSave.Width, rcSave.Height };

        MIL_THR(StretchRect(
            pD3DSurface,
            &rcSrc,
            m_pDbgSaveSurface,
            &rcDst,
            MilStretchFilter_None
            ));
    }

    if (FAILED(hr))
    {
        ReleaseInterface(m_pDbgSaveSurface);
    }

    RRETURN(HandleDIE(hr));
}


HRESULT
CD3DDeviceLevel1::DbgRestoreSurface(
    __inout_ecount(1) CD3DSurface *pD3DSurface,
    __in_ecount(1) const MilPointAndSizeL &rcRestore
    )
{
    HRESULT hr;

    Assert(m_pDbgSaveSurface);

    RECT rcSrc = { 0, 0, rcRestore.Width, rcRestore.Height };
    RECT rcDst = {
        rcRestore.X, rcRestore.Y,
        rcRestore.X + rcRestore.Width, rcRestore.Y + rcRestore.Height
        };

    hr = THR(StretchRect(
        m_pDbgSaveSurface,
        &rcSrc,
        pD3DSurface,
        &rcDst,
        MilStretchFilter_None
        ));

    m_pDbgSaveSurface->Release();
    m_pDbgSaveSurface = NULL;

    RRETURN(HandleDIE(hr));
}
#endif DBG_STEP_RENDERING


//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::DrawBox
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::DrawBox(
    __in_ecount(1) const MilPointAndSize3F *pbox,
    const D3D11_FILL_MODE fillMode,
    const DWORD dwColor
    )
{
    HRESULT hr = S_OK;
    CD3DVertexXYZDUV2 vecMeshBounds[8];
    WORD wMeshTriangles[36];
    D3D11_FILL_MODE origFillMode = D3D11_FILL_SOLID;
    D3D11_COMPARISON_FUNC origDepthTest = D3D11_COMPARISON_LESS_EQUAL;
    bool fFillModeRetrieved = false;

    IFC(GetFillMode(&origFillMode));
    IFC(GetDepthTestFunction(&origDepthTest));

    fFillModeRetrieved = true;

    vecMeshBounds[0].X = pbox->X;
    vecMeshBounds[0].Y = pbox->Y;
    vecMeshBounds[0].Z = pbox->Z;

    vecMeshBounds[1].X = pbox->X + pbox->LengthX;
    vecMeshBounds[1].Y = pbox->Y;
    vecMeshBounds[1].Z = pbox->Z;

    vecMeshBounds[2].X = pbox->X + pbox->LengthX;
    vecMeshBounds[2].Y = pbox->Y + pbox->LengthY;
    vecMeshBounds[2].Z = pbox->Z;

    vecMeshBounds[3].X = pbox->X;
    vecMeshBounds[3].Y = pbox->Y + pbox->LengthY;
    vecMeshBounds[3].Z = pbox->Z;

    for (int i = 4; i < 8; i++)
    {
        vecMeshBounds[i].X = vecMeshBounds[i-4].X;
        vecMeshBounds[i].Y = vecMeshBounds[i-4].Y;
        vecMeshBounds[i].Z = vecMeshBounds[i-4].Z + pbox->LengthZ;
    }

    for (int i = 0; i < 8; i++)
    {
        vecMeshBounds[i].Diffuse = dwColor;
        vecMeshBounds[i].U0 = 0.0f;
        vecMeshBounds[i].V0 = 0.0f;
        vecMeshBounds[i].U1 = 0.0f;
        vecMeshBounds[i].V1 = 0.0f;
    }

    static const WORD rgIndices[36] =
    {
        0, 2, 1,   0, 3, 2,     // Bottom Face
        4, 5, 6,   4, 6, 7,     // Top Face
        0, 4, 3,   3, 4, 7,     // Left Face
        1, 2, 5,   2, 6, 5,     // Right Face
        2, 3, 6,   3, 7, 6,     // Upper Face
        0, 1, 4,   1, 5, 4,     // Lower Face
    };

    CopyMemory(wMeshTriangles, rgIndices, sizeof(wMeshTriangles));

    IFC(SetFillMode(fillMode));
    IFC(SetDepthFunc(D3D11_COMPARISON_ALWAYS));

    IFC(SetVertexFormat(static_cast<MilVertexFormatId>(vecMeshBounds[0].Format)));

    IFC(SetRenderState_AlphaSolidBrush());

    IFC(DrawIndexedTriangleListUP(
        8,
        12,
        wMeshTriangles,
        vecMeshBounds,
        sizeof(vecMeshBounds[0])
        ));

    IFC(SetFillMode(origFillMode));
    IFC(SetDepthFunc(origDepthTest));

Cleanup:
    if (FAILED(hr) && fFillModeRetrieved)
    {
        IGNORE_HR(SetFillMode(origFillMode));
        IGNORE_HR(SetDepthFunc(origDepthTest));
    }

    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::DrawIndexedTriangleListUP
//
//  Synopsis:
//      Draws user supplied vertices and indices through our shared dynamic
//      VB/IB.  Data that does not fit goes through a temporary buffer.
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::DrawIndexedTriangleListUP(
    __range(1, UINT_MAX) UINT uNumVertices,
    __range(1, UINT_MAX) UINT uPrimitiveCount,
    __in_xcount(sizeof(WORD) * uPrimitiveCount * 3) const WORD* pIndexData,
    __in const void* pVertexStreamZeroData,
    __range(1, UINT_MAX) UINT uVertexStreamZeroStride
    )
{
    HRESULT hr = S_OK;
    UINT uiNumIndices = uPrimitiveCount*3;
    void *pvDataVertices = NULL;
    void *pvDataIndices = NULL;
    bool fVBLockAcquired = false;
    bool fIBLockAcquired = false;
    UINT uiCurrentVertex = 0;
    UINT uiCurrentIndex = 0;

    ID3D11Buffer *pTempVB = NULL;
    ID3D11Buffer *pTempIB = NULL;

    Assert(uNumVertices > 0);
    Assert(uPrimitiveCount > 0);
    Assert(uVertexStreamZeroStride > 0);

    if (g_pMediaControl)
    {
        const WORD *pIndex = static_cast<const WORD *>(pIndexData);
        const CD3DVertexXYZDUV2 *pVertices = static_cast<const CD3DVertexXYZDUV2 *>(pVertexStreamZeroData);

        for (UINT i = 0; i < uPrimitiveCount; i++)
        {
            CD3DVertexXYZDUV2 v1 = pVertices[pIndex[0]];
            CD3DVertexXYZDUV2 v2 = pVertices[pIndex[1]];
            CD3DVertexXYZDUV2 v3 = pVertices[pIndex[2]];

            float rArea = fabs((v2.X*v1.Y-v1.X*v2.Y) + (v3.X*v2.Y-v2.X*v3.Y) + (v1.X*v3.Y-v3.X*v1.Y))/2.0f;
            g_lPixelsFilledPerFrame += CFloatFPU::Ceiling(rArea);

            pIndex += 3;
        }
    }

    //
    // Try to lock both the IB/VB for the fast path case
    //

    hr = THR(m_pHwVertexBuffer->Lock(
        uNumVertices,
        uVertexStreamZeroStride,
        &pvDataVertices,
        &uiCurrentVertex
        ));

    if (SUCCEEDED(hr))
    {
        fVBLockAcquired = true;

        hr = THR(m_pHwIndexBuffer->Lock(
            uiNumIndices,
            reinterpret_cast<WORD **>(&pvDataIndices),
            &uiCurrentIndex
            ));

        fIBLockAcquired = SUCCEEDED(hr);
    }

    if (!fVBLockAcquired || !fIBLockAcquired)
    {
        MtSetDefault(Mt(D3DDrawIndexedPrimitiveUP));
        MilDx11StatAdd(MilDx11Stat_TempBufferDraws);

        //
        // Too large for the shared buffers: use temporary buffers
        //

        if (fVBLockAcquired)
        {
            IGNORE_HR(m_pHwVertexBuffer->Unlock(0));
            fVBLockAcquired = false;
        }

        D3D11_MAPPED_SUBRESOURCE mapped;

        IFC(CreateVertexBuffer(uNumVertices * uVertexStreamZeroStride, true, &pTempVB));
        IFC(CreateIndexBuffer(uiNumIndices * sizeof(WORD), true, &pTempIB));

        IFC(MilTranslateD3D11Error(m_pD3DContext->Map(pTempVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)));
        RtlCopyMemory(mapped.pData, pVertexStreamZeroData, uNumVertices * uVertexStreamZeroStride);
        m_pD3DContext->Unmap(pTempVB, 0);

        IFC(MilTranslateD3D11Error(m_pD3DContext->Map(pTempIB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)));
        RtlCopyMemory(mapped.pData, pIndexData, uiNumIndices * sizeof(WORD));
        m_pD3DContext->Unmap(pTempIB, 0);

        IFC(SetStreamSource(pTempVB, uVertexStreamZeroStride));
        IFC(SetIndices(pTempIB));
        IFC(SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
        IFC(ApplyStateForDraw());

        m_pD3DContext->DrawIndexed(uiNumIndices, 0, 0);

        // Our shared buffers are no longer bound
        IFC(SetStreamSource(NULL, 0));
        IFC(SetIndices(NULL));

        UpdateMetrics(uNumVertices, uPrimitiveCount);

        goto Cleanup;
    }

    //
    // Update vertices
    //

    RtlCopyMemory(pvDataVertices, pVertexStreamZeroData, uNumVertices*uVertexStreamZeroStride);

    IFC(m_pHwVertexBuffer->Unlock(uNumVertices));
    fVBLockAcquired = false;

    //
    // Update indices
    //

    RtlCopyMemory(pvDataIndices, pIndexData, sizeof(short)*uiNumIndices);

    IFC(m_pHwIndexBuffer->Unlock());
    fIBLockAcquired = false;

    IFC(SetStreamSource(
        m_pHwVertexBuffer->GetD3DBuffer(),
        uVertexStreamZeroStride
        ));

    IFC(SetIndices(
        m_pHwIndexBuffer->GetD3DBuffer()
        ));

    IFC(SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

    {
        MtSetDefault(Mt(D3DDrawIndexedPrimitive));

        IFC(ApplyStateForDraw());

        m_pD3DContext->DrawIndexed(uiNumIndices, uiCurrentIndex, uiCurrentVertex);
    }

    UpdateMetrics(uNumVertices, uPrimitiveCount);

Cleanup:
    if (fVBLockAcquired)
    {
        IGNORE_HR(m_pHwVertexBuffer->Unlock(uNumVertices));
    }
    if (fIBLockAcquired)
    {
        IGNORE_HR(m_pHwIndexBuffer->Unlock());
    }

    ReleaseInterface(pTempVB);
    ReleaseInterface(pTempIB);

    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::DrawIndexedTriangleList
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::DrawIndexedTriangleList(
    UINT uBaseVertexIndex,
    UINT uMinIndex,
    UINT cVertices,
    UINT uStartIndex,
    UINT cPrimitives
    )
{
    HRESULT hr = S_OK;
    MtSetDefault(Mt(D3DDrawIndexedPrimitive));

    UNREFERENCED_PARAMETER(uMinIndex);

    IFC(SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
    IFC(ApplyStateForDraw());

    m_pD3DContext->DrawIndexed(cPrimitives * 3, uStartIndex, uBaseVertexIndex);

    UpdateMetrics(cVertices, cPrimitives);

Cleanup:
    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::DrawTriangleList
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::DrawTriangleList(
    UINT uStartVertex,
    UINT cPrimitives
    )
{
    HRESULT hr = S_OK;
    MtSetDefault(Mt(D3DDrawPrimitive));

    IFC(SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
    IFC(ApplyStateForDraw());

    m_pD3DContext->Draw(cPrimitives * 3, uStartVertex);

    UpdateMetrics(cPrimitives*3, cPrimitives);

Cleanup:
    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::DrawTriangleStrip
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::DrawTriangleStrip(
    UINT uStartVertex,
    UINT cPrimitives
    )
{
    HRESULT hr = S_OK;
    MtSetDefault(Mt(D3DDrawPrimitive));

    IFC(SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP));
    IFC(ApplyStateForDraw());

    m_pD3DContext->Draw(cPrimitives + 2, uStartVertex);

    UpdateMetrics(cPrimitives + 2, cPrimitives);

Cleanup:
    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::CopyD3DTexture
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::CopyD3DTexture(
    __in_ecount(1) ID3D11Texture2D *pD3DSourceTexture,
    __inout_ecount(1) ID3D11Texture2D *pD3DDestinationTexture
    )
{
    HRESULT hr = S_OK;

    m_pD3DContext->CopyResource(pD3DDestinationTexture, pD3DSourceTexture);

    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::DrawPrimitiveUP
//
//  Synopsis:
//      Draws user supplied vertices through our shared dynamic VB.
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::DrawPrimitiveUP(
    D3D11_PRIMITIVE_TOPOLOGY primitiveType,
    UINT uPrimitiveCount,
    __in_xcount(
        uVertexStreamZeroStride * ((primitiveType == D3D11_PRIMITIVE_TOPOLOGY_LINELIST) ? uPrimitiveCount*2 : uPrimitiveCount+2)
        ) const void* pVertexStreamZeroData,
    UINT uVertexStreamZeroStride
    )
{
    HRESULT hr = S_OK;
    void *pvData = NULL;
    UINT uNumVertices = 0;
    bool fLockAcquired = false;
    UINT uiCurrentVertex = 0;
    ID3D11Buffer *pTempVB = NULL;

    Assert(uPrimitiveCount > 0);
    Assert(pVertexStreamZeroData);
    Assert(uVertexStreamZeroStride > 0);

    switch (primitiveType)
    {
    case D3D11_PRIMITIVE_TOPOLOGY_LINELIST:
        uNumVertices = uPrimitiveCount*2;
        break;

    case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
        uNumVertices = uPrimitiveCount*3;
        break;

    case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
        {
            uNumVertices = uPrimitiveCount+2;

            if (g_pMediaControl)
            {
                const CD3DVertexXYZDUV2 *pVertices =
                    static_cast<const CD3DVertexXYZDUV2 *>(pVertexStreamZeroData);

                for (UINT i = 0; i < uPrimitiveCount; i++)
                {
                    CD3DVertexXYZDUV2 v1 = pVertices[0];
                    CD3DVertexXYZDUV2 v2 = pVertices[1];
                    CD3DVertexXYZDUV2 v3 = pVertices[2];

                    float rArea = fabs((v2.X*v1.Y-v1.X*v2.Y) + (v3.X*v2.Y-v2.X*v3.Y) + (v1.X*v3.Y-v3.X*v1.Y))/2.0f;
                    g_lPixelsFilledPerFrame += CFloatFPU::Ceiling(rArea);

                    pVertices += 1;
                }
            }
            break;
        }

    default:
        AssertMsg(0, "Unsupported primitive type");
        IFC(E_INVALIDARG);
    }

    //
    // Try to lock the VB for the fast path case
    //

    hr = THR(m_pHwVertexBuffer->Lock(
        uNumVertices,
        uVertexStreamZeroStride,
        &pvData,
        &uiCurrentVertex
        ));

    fLockAcquired = SUCCEEDED(hr);

    if (!fLockAcquired)
    {
        MtSetDefault(Mt(D3DDrawPrimitiveUP));
        MilDx11StatAdd(MilDx11Stat_TempBufferDraws);

        //
        // Too large for the shared buffer: use a temporary buffer
        //

        D3D11_MAPPED_SUBRESOURCE mapped;

        IFC(CreateVertexBuffer(uNumVertices * uVertexStreamZeroStride, true, &pTempVB));

        IFC(MilTranslateD3D11Error(m_pD3DContext->Map(pTempVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)));
        RtlCopyMemory(mapped.pData, pVertexStreamZeroData, uNumVertices * uVertexStreamZeroStride);
        m_pD3DContext->Unmap(pTempVB, 0);

        IFC(SetStreamSource(pTempVB, uVertexStreamZeroStride));
        IFC(SetPrimitiveTopology(primitiveType));
        IFC(ApplyStateForDraw());

        m_pD3DContext->Draw(uNumVertices, 0);

        IFC(SetStreamSource(NULL, 0));

        UpdateMetrics(uNumVertices, uPrimitiveCount);

        goto Cleanup;
    }

    RtlCopyMemory(pvData, pVertexStreamZeroData, uNumVertices*uVertexStreamZeroStride);

    IFC(m_pHwVertexBuffer->Unlock(uNumVertices));
    fLockAcquired = false;

    IFC(SetStreamSource(
        m_pHwVertexBuffer->GetD3DBuffer(),
        uVertexStreamZeroStride
        ));

    IFC(SetPrimitiveTopology(primitiveType));

    {
        MtSetDefault(Mt(D3DDrawPrimitive));

        IFC(ApplyStateForDraw());

        m_pD3DContext->Draw(uNumVertices, uiCurrentVertex);
    }

    UpdateMetrics(uNumVertices, uPrimitiveCount);

Cleanup:
    if (fLockAcquired)
    {
        IGNORE_HR(m_pHwVertexBuffer->Unlock(uNumVertices));
    }

    ReleaseInterface(pTempVB);

    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::EnsureScratchIndexBuffer
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::EnsureScratchIndexBuffer(UINT cIndices)
{
    HRESULT hr = S_OK;

    if (cIndices > m_cFanIndices)
    {
        UINT cNew = max(cIndices, 3u * 64u);

        WORD *rgNew = WPFAllocType(WORD *, ProcessHeap, Mt(CD3DDeviceLevel1), cNew * sizeof(WORD));
        IFCOOM(rgNew);

        WPFFree(ProcessHeap, m_rgwFanIndices);
        m_rgwFanIndices = rgNew;
        m_cFanIndices = cNew;
    }

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::DrawTriangleFanUP
//
//  Synopsis:
//      D3D11 has no fans: draw (0, i+1, i+2) triangles as an indexed list.
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::DrawTriangleFanUP(
    __range(3, UINT_MAX) UINT uNumVertices,
    __in_xcount(uVertexStreamZeroStride * uNumVertices) const void* pVertexStreamZeroData,
    UINT uVertexStreamZeroStride
    )
{
    HRESULT hr = S_OK;

    Assert(uNumVertices >= 3);

    if (uNumVertices > 0xFFFF)
    {
        IFC(WGXERR_INSUFFICIENTBUFFER);
    }

    {
        UINT cTriangles = uNumVertices - 2;

        if (g_pMediaControl)
        {
            const CD3DVertexXYZDUV2 *pVertices =
                static_cast<const CD3DVertexXYZDUV2 *>(pVertexStreamZeroData);

            CD3DVertexXYZDUV2 v1 = pVertices[0];

            for (UINT i = 0; i < cTriangles; i++)
            {
                CD3DVertexXYZDUV2 v2 = pVertices[i + 1];
                CD3DVertexXYZDUV2 v3 = pVertices[i + 2];

                float rArea = fabs((v2.X*v1.Y-v1.X*v2.Y) + (v3.X*v2.Y-v2.X*v3.Y) + (v1.X*v3.Y-v3.X*v1.Y))/2.0f;
                g_lPixelsFilledPerFrame += CFloatFPU::Ceiling(rArea);
            }
        }

        IFC(EnsureScratchIndexBuffer(cTriangles * 3));

        WORD *pw = m_rgwFanIndices;
        for (UINT i = 0; i < cTriangles; i++)
        {
            *pw++ = 0;
            *pw++ = static_cast<WORD>(i + 1);
            *pw++ = static_cast<WORD>(i + 2);
        }

        // Temporarily disable the metrics accounting in the callee; it was done above.
        IFC(DrawIndexedTriangleListUP(
            uNumVertices,
            cTriangles,
            m_rgwFanIndices,
            pVertexStreamZeroData,
            uVertexStreamZeroStride
            ));
    }

Cleanup:
    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::DrawVideoToSurface
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::DrawVideoToSurface(
    __in_ecount(1) IAVSurfaceRenderer *pSurfaceRenderer,
    __deref_opt_out_ecount(1) IWGXBitmapSource ** const ppBitmapSource
    )
{
    HRESULT hr = S_OK;

    Assert(m_pD3DDevice);

    AssertDeviceEntry(*this);

    IFC(pSurfaceRenderer->BeginRender(this, ppBitmapSource));

Cleanup:

    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::CheckDeviceState
//
//  Synopsis:
//      Checks whether the device has been removed or reset.
//
//------------------------------------------------------------------------------

HRESULT
CD3DDeviceLevel1::CheckDeviceState(
    __in_opt HWND hwnd
    )
{
    HRESULT hr = S_OK;

    MIL_THR(MilTranslateD3D11Error(m_pD3DDevice->GetDeviceRemovedReason()));

    if (m_fTestDeviceLost)
    {
        hr = D3DERR_DEVICELOST;
    }

    if (FAILED(hr))
    {
        MIL_DX11_LOG("CheckDeviceState: device lost hr=0x%08x", hr);
        MIL_THR(WGXERR_DISPLAYSTATEINVALID);
        MarkUnusable(false /* This call is already entry protected. */);
    }

    EventWriteWClientUceCheckDeviceStateInfo(hwnd, hr);

    RRETURN1(hr, S_PRESENT_OCCLUDED);
}


//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::WaitForVBlank
//
//  Synopsis:
//      Waits until a vblank occurs on the primary output of this device
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::WaitForVBlank(UINT uSwapChainIndex)
{
    BEGIN_MILINSTRUMENTATION_HRESULT_LIST_WITH_DEFAULTS
        WGXERR_NO_HARDWARE_DEVICE
    END_MILINSTRUMENTATION_HRESULT_LIST

    HRESULT hr = WGXERR_NO_HARDWARE_DEVICE;

    UNREFERENCED_PARAMETER(uSwapChainIndex);

    if (m_pDXGIOutput && IsHALDevice())
    {
        double const msWaitStart = MilDx11QpcMs();
        MIL_THR(MilTranslateD3D11Error(m_pDXGIOutput->WaitForVBlank()));
        MilDx11TimingAdd(MilDx11Timing_VBlankWait, MilDx11QpcMs() - msWaitStart);

        if (FAILED(hr))
        {
            hr = WGXERR_NO_HARDWARE_DEVICE;
        }
    }

    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::ResetMarkers
//
//------------------------------------------------------------------------------
void
CD3DDeviceLevel1::ResetMarkers()
{
    m_ullLastConsumedMarkerId = m_ullLastMarkerId;
    UINT uIndex;
    UINT count = m_rgpMarkerFree.GetCount();
    for (uIndex = 0; uIndex < count ; uIndex++)
    {
        delete m_rgpMarkerFree[uIndex];
    }
    m_rgpMarkerFree.Reset();

    count = m_rgpMarkerActive.GetCount();
    for (uIndex = 0; uIndex < count;  uIndex++)
    {
        delete m_rgpMarkerActive[uIndex];
    }
    m_rgpMarkerActive.Reset();
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::ConsumePresentMarkers
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::ConsumePresentMarkers(
    bool fForceFlush
    )
{
    HRESULT hr = S_OK;

    Assert(m_rgpMarkerActive.GetCount() < INT_MAX);

    for (int i = static_cast<int>(m_rgpMarkerActive.GetCount()) - 1; i >= 0; i--)
    {
        bool fMarkerConsumed = false;

        Assert(i < static_cast<int>(m_rgpMarkerActive.GetCount()));

        IFC(IsConsumedGPUMarker(
            i,
            fForceFlush,
            &fMarkerConsumed
            ));

        if (fMarkerConsumed)
        {
            IFC(FreeMarkerAndItsPredecessors(i));
            break;
        }

        if (fForceFlush)
        {
            fForceFlush = false;
            m_uNumSuccessfulPresentsSinceMarkerFlush = 0;
        }
    }

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::AdvanceFrame
//
//------------------------------------------------------------------------------
void
CD3DDeviceLevel1::AdvanceFrame(UINT uFrameNumber)
{
    if (m_uFrameNumber != uFrameNumber)
    {
        m_uFrameNumber = uFrameNumber;

        m_resourceManager.EndFrame();
        m_resourceManager.DestroyReleasedResourcesFromLastFrame();
        m_resourceManager.DestroyResources(CD3DResourceManager::WithDelay);
    }
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::GetNumQueuedPresents
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::GetNumQueuedPresents(
    __out_ecount(1) UINT *puNumQueuedPresents
    )
{
    HRESULT hr = S_OK;
    bool fForceFlush = (m_uNumSuccessfulPresentsSinceMarkerFlush >= NUM_PRESENTS_BEFORE_GPU_MARKER_FLUSH);

    *puNumQueuedPresents = 0;

    if (   !AreGPUMarkersTested()
        || !AreGPUMarkersEnabled()
            )
    {
        goto Cleanup;
    }

    IFC(ConsumePresentMarkers(
        fForceFlush
        ));

    if (   m_rgpMarkerActive.GetCount() > 2
        && fForceFlush == false
           )
    {
        IFC(ConsumePresentMarkers(
            true
            ));
    }

    if (HaveGPUMarkersBeenConsumed())
    {
        *puNumQueuedPresents = m_rgpMarkerActive.GetCount();
    }

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::InsertGPUMarker
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::InsertGPUMarker(
    ULONGLONG ullMarkerId
    )
{
    HRESULT hr = S_OK;
    CGPUMarker *pMarker = NULL;

    if (ullMarkerId < m_ullLastMarkerId)
    {
        TraceTag((tagError, "Warning: GPUMarker Received out of increasing order.  Ignoring it."));
        goto Cleanup;
    }

    if (!m_pD3DDevice)
    {
        goto Cleanup;
    }

    if (!AreGPUMarkersTested())
    {
        D3D11_QUERY_DESC qd = { D3D11_QUERY_EVENT, 0 };

        MIL_THR(m_pD3DDevice->CreateQuery(&qd, NULL));

        if (SUCCEEDED(hr) || hr == S_FALSE)
        {
            hr = S_OK;
            SetGPUMarkersAsEnabled();
        }

        SetGPUMarkersAsTested();
    }

    Assert(AreGPUMarkersTested());

    if (!AreGPUMarkersEnabled())
    {
        m_ullLastMarkerId = ullMarkerId;
        goto Cleanup;
    }

    if (m_rgpMarkerFree.GetCount() > 0)
    {
        UINT uIndex = m_rgpMarkerFree.GetCount()-1;
        pMarker = m_rgpMarkerFree[uIndex];
        m_rgpMarkerFree.SetCount(uIndex);
        pMarker->Reset(ullMarkerId);
    }
    else
    {
        ID3D11Query *pQuery = NULL;
        D3D11_QUERY_DESC qd = { D3D11_QUERY_EVENT, 0 };

        IFC(MilTranslateD3D11Error(m_pD3DDevice->CreateQuery(&qd, &pQuery)));
        pMarker = new CGPUMarker(pQuery, m_pD3DContext, ullMarkerId);
        if (!pMarker)
        {
            ReleaseInterface(pQuery);
            IFC(E_OUTOFMEMORY);
        }
    }

    IFC(pMarker->InsertIntoCommandStream());
    IFC(m_rgpMarkerActive.Add(pMarker));

    m_ullLastMarkerId = ullMarkerId;

    pMarker = NULL;

  Cleanup:
    delete pMarker;

    if (m_rgpMarkerActive.GetCount() > GPU_MARKERS_MAX_ARRAY_SIZE)
    {
        TraceTag((tagError, "Backlog of unconsumed markers in the device, turning marking checking off."));

        DisableGPUMarkers();
    }

    if (D3DERR_DEVICELOST == hr
     || D3DERR_NOTAVAILABLE == hr)
    {
        hr = S_OK;
    }

    if (FAILED(hr))
    {
        hr = S_OK;
        DisableGPUMarkers();
    }

    RRETURN(HandleDIE(hr));
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::IsConsumedGPUMarker
//
//------------------------------------------------------------------------------
HRESULT CD3DDeviceLevel1::IsConsumedGPUMarker(
    UINT uMarkerIndex,
    bool fFlushMarkers,
    __out_ecount(1) bool *pfMarkerConsumed
    )
{
    HRESULT hr = S_OK;

    Assert(m_rgpMarkerActive.GetCount() > 0);
    Assert(AreGPUMarkersEnabled());
    Assert(uMarkerIndex < m_rgpMarkerActive.GetCount());

    MIL_THR(m_rgpMarkerActive[uMarkerIndex]->CheckStatus(
        fFlushMarkers,
        pfMarkerConsumed
        ));

    if (hr == D3DERR_DEVICELOST)
    {
        *pfMarkerConsumed = true;
        hr = S_OK;
    }

    if (SUCCEEDED(hr))
    {
        if (*pfMarkerConsumed == true)
        {
            SetGPUMarkersAsConsumed();
        }
    }

    if (FAILED(hr))
    {
        hr = S_OK;
        DisableGPUMarkers();
        *pfMarkerConsumed = true;
    }

    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::FreeMarkerAndItsPredecessors
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::FreeMarkerAndItsPredecessors(
    UINT uIndex
    )
{
    HRESULT hr = S_OK;

    UINT cConsumed = uIndex+1;
    UINT cNew  = m_rgpMarkerActive.GetCount() - cConsumed;
    UINT ui;

    m_ullLastConsumedMarkerId = m_rgpMarkerActive[uIndex]->GetId();

    for (ui = 0; ui < cConsumed; ui++)
    {
        IFC(m_rgpMarkerFree.Add(m_rgpMarkerActive[ui]));
        m_rgpMarkerActive[ui] = NULL;
    }
    for (ui = 0; ui < cNew; ui++)
    {
        m_rgpMarkerActive[ui] = m_rgpMarkerActive[cConsumed + ui];
    }
    m_rgpMarkerActive.SetCount(cNew);

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::InitializeIMediaDeviceConsumer
//
//  Synopsis:
//      Hardware video is reworked in M11 (D16); until then the media code
//      renders through its software buffer and gets no device.
//
//------------------------------------------------------------------------------
void
CD3DDeviceLevel1::InitializeIMediaDeviceConsumer(
    __in_ecount(1) IMediaDeviceConsumer *pIMediaDeviceConsumer
    )
{
    UNREFERENCED_PARAMETER(pIMediaDeviceConsumer);
}

//+-----------------------------------------------------------------------------
//
//  Intermediate texture pool (D26)
//
//------------------------------------------------------------------------------

static UINT TexturePoolBytes(__in_ecount(1) const D3D11_TEXTURE2D_DESC &desc)
{
    UINT cbPixel = 4;

    if (desc.Format == DXGI_FORMAT_R32G32B32A32_FLOAT)
    {
        cbPixel = 16;
    }
    else if (desc.Format == DXGI_FORMAT_R16G16B16A16_FLOAT)
    {
        cbPixel = 8;
    }

    return desc.Width * desc.Height * cbPixel * max(1u, desc.SampleDesc.Count);
}

static bool IsTexturePoolable(__in_ecount(1) const D3D11_TEXTURE2D_DESC &desc)
{
    if (desc.MiscFlags != 0 || desc.MipLevels != 1 || desc.ArraySize != 1 || desc.Width > 4096 || desc.Height > 4096)
    {
        return false;
    }

    // GPU textures (intermediate render targets, bitmap and glyph uploads) and
    // the staging half of lockable textures. Both are created and freed per
    // frame by the engine's own code paths.
    if (desc.Usage == D3D11_USAGE_DEFAULT)
    {
        return desc.CPUAccessFlags == 0
            && (desc.BindFlags & (D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE)) != 0;
    }

    return desc.Usage == D3D11_USAGE_STAGING && desc.BindFlags == 0;
}

bool
CD3DDeviceLevel1::TryGetPooledTexture(
    __in_ecount(1) const D3D11_TEXTURE2D_DESC *pDesc,
    __deref_out_ecount(1) ID3D11Texture2D **ppTexture,
    bool fAllowLarger
    )
{
    bool fFound = false;

    if (m_fTexturePoolEnabled && IsTexturePoolable(*pDesc))
    {
        CGuard<CCriticalSection> guard(m_csTexturePool);

        UINT iBest = UINT_MAX;
        UINT uBestArea = UINT_MAX;

        for (UINT i = 0; i < m_cTexturePool; i++)
        {
            const D3D11_TEXTURE2D_DESC &d = m_rgTexturePool[i].desc;

            // A staging texture freed during this present may still be the
            // source of a copy in flight: reusing it would stall in Map. Give
            // those one present of rest; GPU-only textures are safe at once.
            if (   pDesc->Usage == D3D11_USAGE_STAGING
                && m_rgTexturePool[i].uLastUsePresent == m_uPresentCount)
            {
                continue;
            }

            if (   d.Format != pDesc->Format
                || d.Usage != pDesc->Usage
                || d.BindFlags != pDesc->BindFlags
                || d.CPUAccessFlags != pDesc->CPUAccessFlags
                || d.MiscFlags != pDesc->MiscFlags
                || d.SampleDesc.Count != pDesc->SampleDesc.Count
                || d.SampleDesc.Quality != pDesc->SampleDesc.Quality
                || d.MipLevels != pDesc->MipLevels
                || d.ArraySize != pDesc->ArraySize)
            {
                continue;
            }

            if (d.Width == pDesc->Width && d.Height == pDesc->Height)
            {
                iBest = i;
                break;
            }

            // Intermediate render targets: a texture up to 64 pixels larger in
            // each direction serves (the color source maps the bitmap rectangle
            // onto the real texture size).
            if (   fAllowLarger
                && d.Width >= pDesc->Width && d.Width <= pDesc->Width + 64
                && d.Height >= pDesc->Height && d.Height <= pDesc->Height + 64
                && d.Width * d.Height < uBestArea)
            {
                iBest = i;
                uBestArea = d.Width * d.Height;
            }
        }

        if (iBest != UINT_MAX)
        {
            *ppTexture = m_rgTexturePool[iBest].pTexture;   // the pool's reference moves to the caller
            m_cbTexturePool -= TexturePoolBytes(m_rgTexturePool[iBest].desc);
            m_rgTexturePool[iBest] = m_rgTexturePool[--m_cTexturePool];
            fFound = true;
        }
    }

    return fFound;
}

void
CD3DDeviceLevel1::ReturnTextureToPool(
    __in_ecount(1) ID3D11Texture2D *pTexture,
    ULONG cExpectedRefs
    )
{
    D3D11_TEXTURE2D_DESC desc;
    pTexture->GetDesc(&desc);

    // Only textures nobody else references (views and surfaces are already
    // gone), and only while the device is usable.
    pTexture->AddRef();
    ULONG cRefs = pTexture->Release();

    bool fPooled = false;

    if (   cRefs <= cExpectedRefs
        && m_fTexturePoolEnabled
        && !m_fDeviceLostProcessed
        && IsTexturePoolable(desc))
    {
        CGuard<CCriticalSection> guard(m_csTexturePool);

        UINT cb = TexturePoolBytes(desc);

        // Full: evict the least recently returned entry (unless everything in
        // the pool came back during this very present).
        if (m_cTexturePool >= c_cTexturePoolMax || m_cbTexturePool + cb > c_cbTexturePoolMax)
        {
            UINT iOldest = UINT_MAX;
            UINT uOldest = m_uPresentCount;

            for (UINT i = 0; i < m_cTexturePool; i++)
            {
                if (m_rgTexturePool[i].uLastUsePresent < uOldest)
                {
                    uOldest = m_rgTexturePool[i].uLastUsePresent;
                    iOldest = i;
                }
            }

            if (iOldest != UINT_MAX)
            {
                m_cbTexturePool -= TexturePoolBytes(m_rgTexturePool[iOldest].desc);
                m_rgTexturePool[iOldest].pTexture->Release();
                m_rgTexturePool[iOldest] = m_rgTexturePool[--m_cTexturePool];
            }
        }

        if (   m_cTexturePool < c_cTexturePoolMax
            && m_cbTexturePool + cb <= c_cbTexturePoolMax)
        {
            PooledTexture &entry = m_rgTexturePool[m_cTexturePool++];
            entry.pTexture = pTexture;
            entry.desc = desc;
            entry.uLastUsePresent = m_uPresentCount;
            m_cbTexturePool += cb;
            fPooled = true;
        }
    }

    if (!fPooled)
    {
        pTexture->Release();
    }
}

void
CD3DDeviceLevel1::TrimTexturePool(
    bool fAll
    )
{
    if (!m_fTexturePoolEnabled)
    {
        return;
    }

    CGuard<CCriticalSection> guard(m_csTexturePool);

    for (UINT i = 0; i < m_cTexturePool; )
    {
        if (   fAll
            || m_uPresentCount - m_rgTexturePool[i].uLastUsePresent > c_cTexturePoolIdlePresents)
        {
            m_cbTexturePool -= TexturePoolBytes(m_rgTexturePool[i].desc);
            m_rgTexturePool[i].pTexture->Release();
            m_rgTexturePool[i] = m_rgTexturePool[--m_cTexturePool];
        }
        else
        {
            i++;
        }
    }
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::SetMediaDevice9
//
//  Synopsis:
//      Keeps the media D3D9Ex device alive as long as this device (D25).
//
//------------------------------------------------------------------------------

void
CD3DDeviceLevel1::SetMediaDevice9(
    __in_opt IUnknown *pMediaDevice9
    )
{
    ReplaceInterface(m_pMediaDevice9, pMediaDevice9);
}

#if DBG
//+-----------------------------------------------------------------------------
//
//  Function:
//      DbgInjectDIE
//
//------------------------------------------------------------------------------
void
DbgInjectDIE(
    __inout_ecount(1) HRESULT * const pHR
    )
{
    if (IsTagEnabled(tagInjectDIE))
    {
        const unsigned c_uInjectionRate = 200;
        static unsigned s_count = 0;

        if (    SUCCEEDED(*pHR)
                && !(++s_count%c_uInjectionRate))
        {
            TraceTag((tagError, "MIL-HW: Injecting D3DERR_DRIVERINTERNALERROR!."));
            *pHR = D3DERR_DRIVERINTERNALERROR;
        }
    }
}
#endif


//+-----------------------------------------------------------------------------
//
//  Function:
//      PrepareShaderEffectPipeline
//
//  Synopsis:
//      Configures the device for running a pixel shader effect.
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::PrepareShaderEffectPipeline(bool useVS30)
{
    HRESULT hr = S_OK;
    CD3DVertexXYZDUV2* pVertices = NULL;

    UNREFERENCED_PARAMETER(useVS30);    // one SM4 vertex shader serves both

    if (m_pEffectPipelineVertexShader == NULL)
    {
        IFC(CreateVertexShaderFromResource(VS_ShaderEffects, &m_pEffectPipelineVertexShader));
    }

    if (m_pEffectPipelineVertexBuffer == NULL)
    {
        IFC(CreateVertexBuffer(
            4 * sizeof(CD3DVertexXYZDUV2),
            true,
            &m_pEffectPipelineVertexBuffer));
    }

    IFC(SetVertexShader(m_pEffectPipelineVertexShader));
    IFC(SetVertexFormat(static_cast<MilVertexFormatId>(CD3DVertexXYZDUV2::Format)));

    {
        D3D11_MAPPED_SUBRESOURCE mapped;

        IFC(MilTranslateD3D11Error(m_pD3DContext->Map(m_pEffectPipelineVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)));

        pVertices = static_cast<CD3DVertexXYZDUV2 *>(mapped.pData);

        //   1---3
        //   | \ |
        //   |  \|
        //   0---2
        pVertices[0].SetXYUV0(0, 1.0f, 0, 1.0f);
        pVertices[1].SetXYUV0(0, 0, 0, 0);
        pVertices[2].SetXYUV0(1.0f, 1.0f, 1.0f, 1.0f);
        pVertices[3].SetXYUV0(1.0f, 0, 1.0f, 0);

        m_pD3DContext->Unmap(m_pEffectPipelineVertexBuffer, 0);
        pVertices = NULL;
    }

    IFC(SetStreamSource(m_pEffectPipelineVertexBuffer, sizeof(CD3DVertexXYZDUV2)));

Cleanup:
    RRETURN(HandleDIE(hr));
}


//+-----------------------------------------------------------------------------
//
//  Function:
//      SetPassThroughPixelShader
//
//------------------------------------------------------------------------------
HRESULT
CD3DDeviceLevel1::SetPassThroughPixelShader()
{
    HRESULT hr = S_OK;

    if (m_pEffectPipelinePassThroughPixelShader == NULL)
    {
        IFC(CreatePixelShaderFromResource(PS_PassThroughShaderEffect, &m_pEffectPipelinePassThroughPixelShader));
    }

    IFC(SetPixelShader(m_pEffectPipelinePassThroughPixelShader));

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Function:
//      Is128BitFPTextureSupported
//
//------------------------------------------------------------------------------
bool
CD3DDeviceLevel1::Is128BitFPTextureSupported() const
{
    return (m_fmtSupportFor128bppPRGBAFloat == MilPixelFormat::PRGBA128bppFloat);
}
