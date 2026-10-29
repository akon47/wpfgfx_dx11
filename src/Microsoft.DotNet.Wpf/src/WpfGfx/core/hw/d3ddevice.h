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
//      Contains CD3DDeviceLevel1 implementation
//
//      ID3D11Device / ID3D11DeviceContext 를 감싼다. 래퍼 API 모양은 D3D9 시절과.
//      같게 유지하고(D9) 내부만 D3D11로 바꿨다.
//
//  $ENDTAG
//
//------------------------------------------------------------------------------

class CD3DSwapChain;
class CD3DDeviceVertexBuffer;
class CD3DDeviceIndexBuffer;
interface IMediaDeviceConsumer;

MtExtern(CD3DDeviceLevel1);

#include "uce\global.h"

#define NUM_PRESENTS_BEFORE_GPU_MARKER_FLUSH 3

#define GPUMARKER_FLAGS_MARKERS_ENABLED             0x1
#define GPUMARKER_FLAGS_MARKERS_TESTED              0x10
#define GPUMARKER_FLAGS_MARKER_CONSUMED             0x100

//
// 디바이스 동작 플래그 (D3DCREATE_* 대체)
//
#define MIL_DEVICE_BEHAVIOR_MULTITHREADED   0x00000001

//+-----------------------------------------------------------------------------
//
//  Defines:
//      Device creation failure tracking
//
//------------------------------------------------------------------------------

#if DBG==1
#define TRACE_DEVICECREATE_FAILURE(adapter, msg,  hr) \
    CD3DDeviceLevel1::DbgTraceDeviceCreationFailure(adapter, msg, hr);
#else
#define TRACE_DEVICECREATE_FAILURE(adapter, msg,  hr)
#endif

// Forward declaration for class used for monitor GPU progress
class CGPUMarker;

//+-----------------------------------------------------------------------------
//
//  Class:
//      CD3DDeviceLevel1
//
//  Synopsis:
//      Abstracts the core D3D device to provide the following functionality:
//          0. Assert on invalid input
//          1. Restrict access to methods of ID3D11Device to those we need.
//          2. Provide correct information for device caps
//          3. Centralize resource creation so that it can be tracked.
//             Tracking created resources is important for responding to mode
//             changes
//          4. Respond to mode changes on present call
//          5. Provide testing functionality for determining if a graphics
//             card meets the level1 criteria for hw acceleration
//
//------------------------------------------------------------------------------

class CD3DDeviceLevel1
      // costly shared data is stored here since some of this data has
      // device affinity.  It also holds the cache index, which needs
      // to be released after all resources and therefore should be the
      // last to be destroyed and making it the first base class.
    : public CHwSurfaceRenderTargetSharedData,
      public CD3DRenderState,
      public CMILPoolResource
{
private:

    struct TargetFormatTestStatus {
        HRESULT hrTest;
        HRESULT hrTestGetDC;
    };

public:
    DECLARE_METERHEAP_CLEAR(ProcessHeap, Mt(CD3DDeviceLevel1));

    static HRESULT Create(
        __inout_ecount(1) ID3D11Device *pID3DDevice,
        __inout_ecount(1) ID3D11DeviceContext *pID3DContext,
        D3D_FEATURE_LEVEL featureLevel,
        MilDeviceKind deviceKind,
        __in_ecount(1) const CDisplay *pPrimaryDisplay,
        __in_ecount(1) IMILPoolManager *pManager,
        DWORD dwBehaviorFlags,
        __deref_out_ecount(1) CD3DDeviceLevel1 ** const ppDevice
        );

    virtual ~CD3DDeviceLevel1();

    //+-------------------------------------------------------------------------
    //
    //  Members:
    //      Enter and Leave
    //
    //  Synopsis:
    //      Thread protection marking methods
    //
    //      Call Enter when device is about to used in a way that requires
    //      exclusive access and Leave when leaving that context.  This is most
    //      commonly done when handling a drawing routine, which the caller is
    //      required to provide protection for.
    //
    //--------------------------------------------------------------------------

    void Enter();
    void Leave();

    bool IsProtected(
        bool fForceEntryConfirmation    // Ignore lack of multithreaded usage
                                        // flag when checking for protection.
                                        // This is to be used by internal
                                        // callers that may violate caller
                                        // provided protection.
        ) const;

    bool IsEntered() const;

    bool IsEnsuringCorrectMultithreadedRendering() const
        { return m_csDeviceEntry.IsValid(); }

    #define ENTER_DEVICE_FOR_SCOPE(Device) \
        CGuard<CD3DDeviceLevel1> oDeviceEntry(Device)

    #define ENTER_USE_CONTEXT_FOR_SCOPE(Device) \
        CD3DUseContextGuard oUseContext(Device)

    #define AssertDeviceEntry(Device) \
        AssertEntry((Device).m_oDbgEntryCheck); \
        Assert((Device).IsProtected(false /* Do not force entry check */))

    #define AssertNoDeviceEntry(Device) \
        AssertEntry((Device).m_oDbgEntryCheck); \
        Assert(!((Device).IsEntered()))

    //
    // Raw D3D11 objects. No AddRef.
    //

    __out_ecount(1) ID3D11Device *GetD3DDeviceNoRef() const { return m_pD3DDevice; }
    __out_ecount(1) ID3D11DeviceContext *GetD3DContextNoRef() const { return m_pD3DContext; }
    __out_ecount(1) ID3D11DeviceContext1 *GetD3DContext1NoRef() const { return m_pD3DContext1; }

    const MilDeviceCaps &GetDeviceCaps() const { return m_caps; }
    D3D_FEATURE_LEVEL GetFeatureLevel() const { return m_caps.FeatureLevel; }
    MilDeviceKind GetDeviceKind() const { return m_caps.DeviceKind; }

    void InitializeIMediaDeviceConsumer(
        __in_ecount(1) IMediaDeviceConsumer *pIMediaDeviceConsumer
        );

    // Media (core/av, D25): the Direct3D 9Ex device on this adapter that the
    // EVR mixer and DXVA2 use. Created on demand by the AV layer and released
    // with this device.
    void SetMediaDevice9(
        __in_opt IUnknown *pMediaDevice9
        );

    __out_opt IUnknown *GetMediaDevice9NoRef() const { return m_pMediaDevice9; }

    //
    // Intermediate texture pool (D26). WPF creates and frees intermediate
    // render targets every frame (effects, brush realizations). D3D11 drivers
    // make that allocation churn expensive, so released textures are kept and
    // handed back for an identical description.
    //
    bool TryGetPooledTexture(
        __in_ecount(1) const D3D11_TEXTURE2D_DESC *pDesc,
        __deref_out_ecount(1) ID3D11Texture2D **ppTexture,
        bool fAllowLarger
        );

    void ReturnTextureToPool(
        __in_ecount(1) ID3D11Texture2D *pTexture,     // the caller's reference is consumed
        ULONG cExpectedRefs = 1                       // references the caller knows will go away with it
        );

    void TrimTexturePool(
        bool fAll
        );



    HRESULT CheckRenderTargetFormat(
        DXGI_FORMAT fmtRenderTarget,
        __deref_opt_out_ecount(1) HRESULT const ** const pphrTestGetDC = NULL
        );

    void AssertRenderFormatIsTestedSuccessfully(
        DXGI_FORMAT fmtRenderTarget
        );

    HRESULT Set3DTransforms(
        __in_ecount(1) CMILMatrix const *pWorldTransform3D,
        __in_ecount(1) CMILMatrix const *pViewTransform3D,
        __in_ecount(1) CMILMatrix const *pProjectionTransform3D,
        __in_ecount(1) CMatrix<CoordinateSpace::Projection3D,CoordinateSpace::Device> const &matHomogeneousTo2DDevice
        );

    HRESULT SetRenderTarget(__in_ecount(1) CD3DSurface *pD3DSurface);

    void ReleaseUseOfRenderTarget(
        __in_ecount(1) const CD3DSurface *pD3DRenderTargetSurface
        );

    void MarkUnusable(
        bool fMayBeMultithreadedCall
        );

    HRESULT CreateAdditionalSwapChain(
        __in_ecount_opt(1) CMILDeviceContext const *pMILDC,
        __in_ecount(1) const MilSwapChainParams *pParams,
        __deref_out_ecount(1) CD3DSwapChain ** const ppSwapChain
        );

    HRESULT CreateRenderTarget(
        UINT uiWidth,
        UINT uiHeight,
        DXGI_FORMAT fmtSurface,
        __in_ecount(1) const DXGI_SAMPLE_DESC &sampleDesc,
        bool fLockable,
        __deref_out_ecount(1) CD3DSurface ** const ppD3DSurface
        );

    HRESULT CreateRenderTargetUntracked(
        UINT uiWidth,
        UINT uiHeight,
        DXGI_FORMAT fmtSurface,
        __in_ecount(1) const DXGI_SAMPLE_DESC &sampleDesc,
        bool fLockable,
        __deref_out_ecount(1) ID3D11Texture2D ** const ppD3DTexture
        );

    // Copies a render target (or any default texture) into a staging texture.
    HRESULT GetRenderTargetData(
        __in_ecount(1) ID3D11Texture2D *pSourceTexture,
        UINT uSourceSubresource,
        __inout_ecount(1) ID3D11Texture2D *pDestinationStagingTexture
        );

    HRESULT CreateVertexBuffer(
        UINT cbLength,
        bool fDynamic,
        __deref_out_ecount(1) ID3D11Buffer ** const ppVertexBuffer
        );

    HRESULT CreateIndexBuffer(
        UINT cbLength,
        bool fDynamic,
        __deref_out_ecount(1) ID3D11Buffer ** const ppIndexBuffer
        );

    HRESULT CreateTexture(
        __in_ecount(1) const D3D11_TEXTURE2D_DESC *pDesc,
        __in_ecount_opt(1) const D3D11_SUBRESOURCE_DATA *pInitialData,
        __deref_out_ecount(1) ID3D11Texture2D ** const ppD3DTexture,
        __deref_opt_inout_ecount(1) HANDLE * const pSharedHandle = NULL,
        bool fAllowLargerFromPool = false   // D26: a somewhat larger pooled texture is fine (intermediate RTs)
        );

    HRESULT CreatePixelShaderFromResource(
        UINT uResourceId,
        __deref_out_ecount(1) ID3D11PixelShader ** const ppPixelShader
        );

    HRESULT CreateVertexShaderFromResource(
        UINT uResourceId,
        __deref_out_ecount(1) ID3D11VertexShader ** const ppVertexShader
        );

    HRESULT CompilePipelineVertexShader(
        __in_bcount(cbHLSLSource) PCSTR pHLSLSource,
        UINT cbHLSLSource,
        __deref_out_ecount(1) ID3D11VertexShader ** const ppVertexShader
        );

    HRESULT CompilePipelinePixelShader(
        __in_bcount(cbHLSLSource) PCSTR pHLSLSource,
        UINT cbHLSLSource,
        __deref_out_ecount(1) ID3D11PixelShader ** const ppPixelShader
        );

    // ShaderEffect: D3D9 바이트코드를 HLSL 로 변환한 결과를 ps_4_0 으로 컴파일한다 (D6).
    HRESULT CompileShaderEffectPixelShader(
        __in_bcount(cbHLSLSource) PCSTR pHLSLSource,
        UINT cbHLSLSource,
        __deref_out_ecount(1) ID3D11PixelShader ** const ppPixelShader
        );

    HRESULT CreateVertexShader(
        __in_bcount(cbBytecode) const void *pvBytecode,
        SIZE_T cbBytecode,
        __deref_out_ecount(1) ID3D11VertexShader ** const ppOutShader
        );

    HRESULT CreatePixelShader(
        __in_bcount(cbBytecode) const void *pvBytecode,
        SIZE_T cbBytecode,
        __deref_out_ecount(1) ID3D11PixelShader **const ppOutShader
        );

    HRESULT CreateLockableTexture(
        __in_ecount(1) const D3D11_TEXTURE2D_DESC *pDesc,
        __deref_out_ecount(1) CD3DLockableTexture ** const ppLockableTexture
        );

    // Staging texture, optionally initialized from pvPixels (tightly packed).
    HRESULT CreateSysMemUpdateSurface(
        UINT uWidth,
        UINT uHeight,
        DXGI_FORMAT fmtTexture,
        __in_xcount_opt(uWidth * uHeight * D3DFormatSize(fmtTexture)) const void *pvPixels,
        __deref_out_ecount(1) ID3D11Texture2D ** const ppD3DSysMemTexture
        );

    HRESULT UpdateSurface(
        __in_ecount(1) CD3DSurface *pD3DSysMemSrcSurface,
        __in_ecount_opt(1) const RECT *pSourceRect,
        __inout_ecount(1) CD3DSurface *pD3DPoolDefaultDestSurface,
        __in_ecount_opt(1) const POINT *pDestPoint
        );

    HRESULT UpdateSurface(
        __in_ecount(1) ID3D11Texture2D *pD3DSrcTexture,
        UINT uSrcSubresource,
        __in_ecount_opt(1) const RECT *pSourceRect,
        __inout_ecount(1) ID3D11Texture2D *pD3DDestTexture,
        UINT uDestSubresource,
        __in_ecount_opt(1) const POINT *pDestPoint
        );

    HRESULT UpdateTexture(
        __in_ecount(1) ID3D11Texture2D *pD3DSrcTexture,
        __inout_ecount(1) ID3D11Texture2D *pD3DDestTexture
        );

    HRESULT StretchRect(
        __in_ecount(1) CD3DSurface *pSourceSurface,
        __in_ecount_opt(1) const RECT *pSourceRect,
        __inout_ecount(1) CD3DSurface *pDestSurface,
        __in_ecount_opt(1) const RECT *pDestRect,
        MilStretchFilter Filter
        );

    HRESULT CreateDepthBuffer(
        UINT uWidth,
        UINT uHeight,
        __in_ecount(1) const DXGI_SAMPLE_DESC &sampleDesc,
        __deref_out_ecount(1) CD3DSurface ** const ppSurface
        );

    void CleanupFreedResources();

    HRESULT Present(
        __in_ecount(1) CD3DSwapChain const *pD3DSwapChain,
        __in_ecount_opt(1) CMILSurfaceRect const *prcSource,
        __in_ecount_opt(1) CMILSurfaceRect const *prcDest,
        __in_ecount(1) CMILDeviceContext const *pMILDC,
        __in_ecount_opt(1) RGNDATA const * pDirtyRegion,
        DWORD dwD3DPresentFlags
        );

    HRESULT SetTexture(
        DWORD dwTextureStage,
        __inout_ecount_opt(1) CD3DTexture *pD3DTexture
        );

    HRESULT SetD3DTexture(
        DWORD dwTextureStage,
        __in_ecount_opt(1) ID3D11ShaderResourceView *pSRV
        );

    // 에뮬레이션 고정 기능에는 텍스처 좌표 변환이 없다. 호환용 no-op.
    HRESULT DisableTextureTransform(DWORD dwTextureStage)
    {
        UNREFERENCED_PARAMETER(dwTextureStage);
        return S_OK;
    }

    HRESULT SetDepthStencilSurface(
        __in_ecount_opt(1) CD3DSurface *pD3DSurface
        );

    void ReleaseUseOfDepthStencilSurface(
        __in_ecount_opt(1) CD3DSurface *pSurface
        );

    HRESULT Clear(
        DWORD dwCount,
        __in_ecount_opt(dwCount) const D3DRECT* pRects,
        DWORD dwFlags,
        D3DCOLOR d3dColor,
        FLOAT rZValue,
        int iStencilValue
        );

    HRESULT ColorFill(
        __inout_ecount(1) CD3DSurface *pSurface,
        __in_ecount_opt(1) const RECT *pRect,
        D3DCOLOR color
        );

    CD3DResourceManager *GetResourceManager()
    {
        return &m_resourceManager;
    }

    HRESULT RenderTexture(
        __in_ecount(1) CD3DTexture *pD3DTexture,
        __in_ecount(1) const MilPointAndSizeL &rcDestination,
        TextureBlendMode blendMode
        );

    HRESULT CopyD3DTexture(
        __in_ecount(1) ID3D11Texture2D *pD3DSourceTexture,
        __inout_ecount(1) ID3D11Texture2D *pD3DDestinationTexture
        );

    HRESULT GetMinimalTextureDesc(
        __inout_ecount(1) D3D11_TEXTURE2D_DESC *pDesc,
        bool fPalUsesAlpha,
        DWORD dwFlags
        ) const
    {
        return ::GetMinimalTextureDesc(&m_caps, pDesc, fPalUsesAlpha, dwFlags);
    }

    HRESULT GetSupportedTextureFormat(
        MilPixelFormat::Enum fmtBitmapSource,          // Current format of bitmap
        MilPixelFormat::Enum fmtDestinationSurface,    // Format of surface onto
                                                 // which the bitmap will be
                                                 // drawn
        bool fForceAlpha,
        __out_ecount(1) MilPixelFormat::Enum &fmtTextureSource    // Format of the
                                                            // texture to hold
                                                            // the bitmap
        ) const;

    DXGI_SAMPLE_DESC GetSupportedMultisampleType(
        MilPixelFormat::Enum fmtDestinationSurface        // Format of target surface
        ) const;

    HRESULT SetClipRect(
        __in_ecount_opt(1) const CMILSurfaceRect *prcClip
        );

    void GetClipRect(
        __out_ecount(1) MilPointAndSizeL * const prcClipRect
        ) const;

    LUID GetD3DAdapterLUID() const { return m_luidD3DAdapter; }
    UINT GetAdapterOrdinal() const { return m_uAdapterOrdinal; }

    //
    // device capability accessors
    //

    TierType GetTier() const { return m_Tier; }
    bool IsLDDMDevice() const {return HwCaps::IsLDDMDevice(m_caps);}
    bool IsHALDevice() const {return m_caps.DeviceKind == MilDeviceKind_Hardware;}
    bool IsSWDevice() const {return HwCaps::IsSWDevice(m_caps);}
    BOOL CanAutoGenMipMap() const {return m_caps.SupportsAutoGenMipMap;}

    bool CanStretchRectGenMipMap() const { return true; }
    bool CanStretchRectFromTextures() const { return true; }
    bool CanMaskColorChannels() const { return HwCaps::CanMaskColorChannels(m_caps); }
    bool CanHandleBlendFactor() const { return HwCaps::CanHandleBlendFactor(m_caps); }
    bool SupportsD3DFMT_A8() const { return true; }
    bool SupportsBorderColor() const { return m_caps.SupportsBorderColor; }
    bool SupportsAnisotropicFiltering() const { return GetMaxDesiredAnisotropicFilterLevel() > 1; }
    bool SupportsNonPow2Conditionally() const { return false; }
    bool SupportsNonPow2Unconditionally() const { return m_caps.SupportsNonPow2Unconditionally; }
    bool SupportsSharedResources() const { return m_caps.SupportsSharedResources; }

    bool ShouldAttemptMultisample() const
    {
        // If a previous attempt to render multisampled failed, do not continue to attempt multisampling
        return !m_fMultisampleFailed &&
            (GetTier() >= MIL_TIER(2,0));
    }

    void SetMultisampleFailed() { m_fMultisampleFailed = true; }

    DWORD GetMaxStreams() const {return 16;}
    DWORD GetMaxTextureBlendStages() const {return MIL_TEXTURE_STAGE_COUNT;}
    DWORD GetMaxSimultaneousTextures() const {return m_caps.MaxSimultaneousTextures;}
    DWORD GetMaxTextureWidth() const {return m_caps.MaxTextureWidth;}
    DWORD GetMaxTextureHeight() const {return m_caps.MaxTextureHeight;}
    DWORD GetVertexShaderVersion() const {return m_caps.VertexShaderVersion;}
    DWORD GetPixelShaderVersion() const {return m_caps.PixelShaderVersion;}
    __range(1,UINT_MAX) DWORD GetMaxDesiredAnisotropicFilterLevel() const
    {
        Assert(m_caps.MaxAnisotropy > 0);

        return (m_caps.MaxAnisotropy > 4) ? 4 : m_caps.MaxAnisotropy;
    }

    __out_ecount(1) CHwD3DVertexBuffer *Get3DVertexBuffer()
        { return m_pHwVertexBuffer; }

    __out_ecount(1) CHwD3DIndexBuffer *Get3DIndexBuffer()
        { return m_pHwIndexBuffer; }

    BOOL SupportsScissorRect() const {return m_caps.SupportsScissorRect;}

    BOOL SupportsLinearTosRGBPresentation() const {return FALSE;}

    __out_ecount(1) const FilterMode *GetSupportedAnistotropicFilterMode() const { return m_pCachedAnisoFilterMode; }

    bool IsExtendedDevice() const { return true; }

    CD3DGlyphBank* GetGlyphBank()
    {
        return &m_glyphBank;
    }

    HRESULT StartPrimitive(CD3DVertexBufferDUV2 **ppOutBuffer)
    {
        *ppOutBuffer = &m_vbBufferDUV2;

        m_vbBufferDUV2.Clear();

        return SetVertexFormat(static_cast<MilVertexFormatId>(CD3DVertexXYZDUV2::Format));
    }

    HRESULT StartPrimitive(CD3DVertexBufferDUV6 **ppOutBuffer)
    {
        m_vbBufferDUV6.Clear();

        *ppOutBuffer = &m_vbBufferDUV6;
        return SetVertexFormat(static_cast<MilVertexFormatId>(CD3DVertexXYZDUV6::Format));
    }

    HRESULT StartPrimitive(CD3DVertexBufferXYZNDSUV4 **ppOutBuffer)
    {
        m_vbBufferXYZNDSUV4.Clear();

        *ppOutBuffer = &m_vbBufferXYZNDSUV4;
        return SetVertexFormat(static_cast<MilVertexFormatId>(CD3DVertexXYZNDSUV4::Format));
    }

    HRESULT EndPrimitiveFan(__in_ecount(1) CD3DVertexBuffer *pBuffer)
    {
        return FlushBufferFan(pBuffer);
    }

    CHwTVertexBuffer<CD3DVertexXYZDUV2> *GetVB_XYZDUV2()
    {
        return &m_vBufferXYZDUV2;
    }

    CHwTVertexBuffer<CD3DVertexXYZDUV8> *GetVB_XYZRHWDUV8()
    {
        return &m_vBufferXYZRHWDUV8;
    }

    HRESULT DrawBox(
        __in_ecount(1) const MilPointAndSize3F *pbox,
        const D3D11_FILL_MODE fillMode,
        const DWORD dwColor
        );

    HRESULT DrawIndexedTriangleList(
        UINT uBaseVertexIndex,
        UINT uMinIndex,
        UINT cVertices,
        UINT uStartIndex,
        UINT cPrimitives
        );

    HRESULT DrawTriangleList(
        UINT uStartVertex,
        UINT cPrimitives
        );

    HRESULT DrawTriangleStrip(
        UINT uStartVertex,
        UINT cPrimitives
        );

    HRESULT DrawIndexedTriangleListUP(
        __range(1, UINT_MAX) UINT uNumVertices,
        __range(1, UINT_MAX) UINT uPrimitiveCount,
        __in_xcount(sizeof(WORD) * uPrimitiveCount * 3) const WORD* pIndexData,
        __in const void* pVertexStreamZeroData,
        __range(1, UINT_MAX) UINT uVertexStreamZeroStride
        );

    // primitiveType: LINELIST, TRIANGLELIST or TRIANGLESTRIP
    HRESULT DrawPrimitiveUP(
        D3D11_PRIMITIVE_TOPOLOGY primitiveType,
        UINT uPrimitiveCount,
        __in_xcount(
            uVertexStreamZeroStride * ((primitiveType == D3D11_PRIMITIVE_TOPOLOGY_LINELIST) ? uPrimitiveCount*2 : uPrimitiveCount+2)
            ) const void* pVertexStreamZeroData,
        UINT uVertexStreamZeroStride
        );

    // D3D11 has no triangle fans; draws the fan as an indexed triangle list.
    HRESULT DrawTriangleFanUP(
        __range(3, UINT_MAX) UINT uNumVertices,
        __in_xcount(uVertexStreamZeroStride * uNumVertices) const void* pVertexStreamZeroData,
        UINT uVertexStreamZeroStride
        );

    HRESULT DrawVideoToSurface(
        __in_ecount(1) IAVSurfaceRenderer *pSurfaceRenderer,
        __deref_opt_out_ecount(1) IWGXBitmapSource ** const ppBitmapSource
        );

    HRESULT CheckDeviceState(
        __in_opt HWND hwnd
        );

    BOOL SupportsGetScanLine() const {return FALSE;}

    HRESULT WaitForVBlank(
        UINT uSwapChain
        );

    HRESULT GetNumQueuedPresents(
        __out_ecount(1) UINT *puNumQueuedPresents
        );

    // Advance the frame for the device.
    // IMPORTANT to call this with increasing frame numbers because
    // it handles releasing HW resources used in the last *TWO* frames.
    void AdvanceFrame(UINT uFrameNumber);

    void Use(__in_ecount(1) const CD3DResource &refResource)
    {
        m_resourceManager.Use(refResource);
    }

    UINT EnterUseContext()
    {
        return m_resourceManager.EnterUseContext();
    }

    void ExitUseContext(UINT uDepth)
    {
        m_resourceManager.ExitUseContext(uDepth);
    }

    bool IsInAUseContext() const
    {
        return m_resourceManager.IsInAUseContext();
    }

#if DBG==1
    static void DbgTraceDeviceCreationFailure(UINT uAdapter, __in PCSTR szMessage, HRESULT hrError);
    const ID3D11Device* DbgGetID3DDevice() const { return m_pD3DDevice; }
#endif

private:
    CD3DDeviceLevel1(
        __in_ecount(1) IMILPoolManager *pManager,
        DWORD dwBehaviorFlags
        );

    HRESULT Init(
        __inout_ecount(1) ID3D11Device *pID3DDevice,
        __inout_ecount(1) ID3D11DeviceContext *pID3DContext,
        D3D_FEATURE_LEVEL featureLevel,
        MilDeviceKind deviceKind,
        __in_ecount(1) CDisplay const *pDisplay
        );

    void GatherSupportedTextureFormats();

    DXGI_SAMPLE_DESC GetMaxMultisampleTypeWithDepthSupport(
        DXGI_FORMAT fmtTarget,
        DXGI_FORMAT fmtDepth,
        UINT uMaxSampleCount
        ) const;

    void GatherSupportedMultisampleTypes();

    HRESULT GetRenderTargetFormatTestEntry(
        DXGI_FORMAT fmtRenderTarget,
        __deref_out_ecount(1) TargetFormatTestStatus * &pFormatTestEntry
        );

    HRESULT TestRenderTargetFormat(
        DXGI_FORMAT fmtRenderTarget,
        __inout_ecount(1) TargetFormatTestStatus *pFormatTestEntry
        );

    HRESULT TestLevel1Device(
        );

    void TestGetDC(
        __inout_ecount(1) CD3DSurface * const pD3DSurface,
        __inout_ecount(1) TargetFormatTestStatus * const pFormatTestEntry
        );

    HRESULT CheckBadDeviceDrivers(
        __in_ecount(1) const CDisplay *pDisplay
        );

    HRESULT SetSurfaceToClippingMatrix(
        __in_ecount(1) const MilPointAndSizeL *prcViewport
        );

    HRESULT FlushBufferFan(
        __in_ecount(1) CD3DVertexBuffer *
        );

    HRESULT EnsureScratchIndexBuffer(UINT cIndices);

    // D3D11 validates the depth stencil against the render target when both are
    // bound; drop a depth buffer whose size or sample count no longer fits.
    void DropIncompatibleDepthStencil();

    HRESULT ResolveMultisampleRect(
        __in_ecount(1) CD3DSurface *pSourceSurface,
        __in_ecount(1) const RECT &rcSource,
        __inout_ecount(1) CD3DSurface *pDestSurface,
        __in_ecount(1) const RECT &rcDest
        );

    HRESULT StretchRectScaled(
        __in_ecount(1) CD3DSurface *pSourceSurface,
        __in_ecount(1) const RECT &rcSource,
        __inout_ecount(1) CD3DSurface *pDestSurface,
        __in_ecount(1) const RECT &rcDest,
        MilStretchFilter Filter
        );

    HRESULT ClearRenderTargetRects(
        DWORD dwCount,
        __in_ecount_opt(dwCount) const D3DRECT* pRects,
        D3DCOLOR d3dColor
        );

    void UpdateMetrics(
        UINT uNumVertices,
        UINT uNumPrimitives);

    // Destroys all outstanding markers.  Called on device error
    void ResetMarkers();

    HRESULT InsertGPUMarker(
        ULONGLONG ullMarkerId
        );

    HRESULT IsConsumedGPUMarker(
        UINT uMarkerIndex,
        bool fFlushMarkers,
        __out_ecount(1) bool *pfMarkerConsumed
        );

    HRESULT FreeMarkerAndItsPredecessors(UINT uIndex);

    MIL_FORCEINLINE HRESULT HandleDIE(HRESULT hr)
    {
        if (FAILED(hr))
        {
            MIL_DX11_LOG("HandleDIE hr=0x%08x", hr);
            MilDx11LogD3DMessages(m_pD3DDevice, "HandleDIE");
        }

        if (hr == D3DERR_DRIVERINTERNALERROR)
        {
            // Return WGXERR_DISPLAYSTATEINVALID to upstream callers.
            // Present will pick up the D3DERR_DRIVERINTERNALERROR and
            // tear down the device.
            m_hrDisplayInvalid = D3DERR_DRIVERINTERNALERROR;
            hr = WGXERR_DISPLAYSTATEINVALID;
        }

        return hr;
    }

    HRESULT PresentWithGDI(
        __in_ecount(1) CD3DSwapChain const *pD3DSwapChain,
        __in_ecount_opt(1) CMILSurfaceRect const *prcSource,
        __in_ecount_opt(1) CMILSurfaceRect const *prcDest,
        __in_ecount(1) CMILDeviceContext const * pMILDC,
        __in_ecount_opt(1) RGNDATA const * pDirtyRegion,
        __out_ecount(1) bool *pfPresentProcessed
        );

    HRESULT PresentWithD3D(
        __inout_ecount(1) IDXGISwapChain1 *pDXGISwapChain,
        __in_ecount_opt(1) CMILSurfaceRect const *prcSource,
        __in_ecount_opt(1) CMILSurfaceRect const *prcDest,
        __in_ecount(1) CMILDeviceContext const *pMILDC,
        __in_ecount_opt(1) RGNDATA const * pDirtyRegion,
        DWORD dwD3DPresentFlags,
        bool fUseDirtyRects,        // D29/D35: flip chain past its first full presents
        __out_ecount(1) bool *pfPresentProcessed
        );

    HRESULT HandlePresentFailure(
         __in_ecount(1) CMILDeviceContext const *pMILDC,
        HRESULT hr);

    HRESULT ConsumePresentMarkers(
        bool fForceFlush
        );

    void SetGPUMarkersAsEnabled()
    {
        m_dwGPUMarkerFlags |= GPUMARKER_FLAGS_MARKERS_ENABLED;
    }

    void DisableGPUMarkers()
    {
        m_dwGPUMarkerFlags &= (~GPUMARKER_FLAGS_MARKERS_ENABLED);
        ResetMarkers();
    }

    bool AreGPUMarkersEnabled() const
    {
        return (m_dwGPUMarkerFlags & GPUMARKER_FLAGS_MARKERS_ENABLED) != 0;
    }

    void SetGPUMarkersAsTested()
    {
        m_dwGPUMarkerFlags |= GPUMARKER_FLAGS_MARKERS_TESTED;
    }

    bool AreGPUMarkersTested() const
    {
        return (m_dwGPUMarkerFlags & GPUMARKER_FLAGS_MARKERS_TESTED) != 0;
    }

    void SetGPUMarkersAsConsumed()
    {
        m_dwGPUMarkerFlags |= GPUMARKER_FLAGS_MARKER_CONSUMED;
    }

    bool HaveGPUMarkersBeenConsumed() const
    {
        return (m_dwGPUMarkerFlags & GPUMARKER_FLAGS_MARKER_CONSUMED) != 0;
    }

private:
    UINT m_cEntry;
    DWORD m_dwThreadId;

    // D3D objects
    ID3D11Device *m_pD3DDevice;
    ID3D11DeviceContext *m_pD3DContext;
    ID3D11DeviceContext1 *m_pD3DContext1;
    IDXGIOutput *m_pDXGIOutput;             // primary display output (vblank wait)

    TargetFormatTestStatus m_RenderTargetTestStatusB8G8R8X8;
    TargetFormatTestStatus m_RenderTargetTestStatusB8G8R8A8;
    TargetFormatTestStatus m_RenderTargetTestStatusR10G10B10A2;

    // Active render target set on device
    CD3DSurface *m_pCurrentRenderTargetNoRef;
    // Hint for what depth buffer to release when releasing a RT from use.
    // It is meaningless unless m_pCurrentRenderTargetNoRef is not NULL.
    CD3DSurface *m_pDepthStencilBufferForCurrentRTNoRef;

    // Non-multisampled scratch for resolving a sub-rectangle of a multisample
    // surface (ResolveSubresource only handles whole subresources).
    ID3D11Texture2D *m_pMSAAResolveScratch;

    LUID m_luidD3DAdapter;
    UINT m_uAdapterOrdinal;
    MilDeviceCaps m_caps;
    TierType m_Tier;
    DWORD const m_dwD3DBehaviorFlags;

    const FilterMode *m_pCachedAnisoFilterMode; // What level of aniso this device supports.

    // This is false initially, and can be set on failure to prevent future attempts to multisample.
    bool m_fMultisampleFailed;

    MilPixelFormat::Enum m_fmtSupportFor128bppPRGBAFloat;
    MilPixelFormat::Enum m_fmtSupportFor128bppRGBFloat;
    MilPixelFormat::Enum m_fmtSupportFor32bppBGR101010;
    MilPixelFormat::Enum m_fmtSupportFor32bppPBGRA;
    MilPixelFormat::Enum m_fmtSupportFor32bppBGR;

    DXGI_SAMPLE_DESC m_multisampleTypeFor32bppBGR;
    DXGI_SAMPLE_DESC m_multisampleTypeFor32bppPBGRA;
    DXGI_SAMPLE_DESC m_multisampleTypeFor32bppBGR101010;

    bool m_fInScene;

    // Set to true once the device has been marked unusable and then given a
    // chance to destroy resources and notify its manager.
    bool m_fDeviceLostProcessed;
    bool m_fTestDeviceLost;             // WPFGFX_DX11_TEST_DEVICE_LOSS hook fired on this device

    IUnknown *m_pMediaDevice9;          // see SetMediaDevice9

    // Intermediate texture pool (D26), see TryGetPooledTexture.
    struct PooledTexture
    {
        ID3D11Texture2D *pTexture;
        D3D11_TEXTURE2D_DESC desc;
        UINT uLastUsePresent;
    };

    enum
    {
        c_cTexturePoolMax = 192,          // the byte limit is the real bound (D26, D32)
        c_cbTexturePoolMax = 96 * 1024 * 1024,
        c_cTexturePoolIdlePresents = 300
    };

    PooledTexture m_rgTexturePool[c_cTexturePoolMax];
    UINT m_cTexturePool;
    UINT m_cbTexturePool;
    UINT m_uPresentCount;
    CCriticalSection m_csTexturePool;
    bool m_fTexturePoolEnabled;



    // HRESULT indicating whether the display is invalid
    HRESULT m_hrDisplayInvalid;

    // Adaptor display mode
    MilDisplayMode m_displayMode;

    // Current render target desc
    D3D11_TEXTURE2D_DESC m_desc;

    // Vertex Buffers
    CD3DVertexBufferDUV2 m_vbBufferDUV2;
    CD3DVertexBufferDUV6 m_vbBufferDUV6;
    CD3DVertexBufferXYZNDSUV4 m_vbBufferXYZNDSUV4;

    // Matrix applied to surface coordinates to translate to homogeneous clip
    // coordinates.  It is always based on the viewport; so, if the viewport
    // changes this probably needs updated.
    CMatrix<CoordinateSpace::DeviceHPC,CoordinateSpace::D3DHomogeneousClipIPC>
        m_matSurfaceToClip;

    CHwTVertexBuffer<CD3DVertexXYZDUV2>     m_vBufferXYZDUV2;
    CHwTVertexBuffer<CD3DVertexXYZDUV8>     m_vBufferXYZRHWDUV8;

    //
    // Custom VB/IB management
    //

    CHwD3DIndexBuffer *m_pHwIndexBuffer;
    CHwD3DVertexBuffer *m_pHwVertexBuffer;

    // Scratch index list used to draw triangle fans
    WORD *m_rgwFanIndices;
    UINT m_cFanIndices;

    // Additional services for tracking resources and render state
    CD3DResourceManager m_resourceManager;

    //
    // Critical section in case we're using the shared WARP device.  We have to
    // protect it from being used by multiple threads simultaneously.
    //
    CCriticalSection m_csDeviceEntry;

    // Glyph rendering
    CD3DGlyphBank m_glyphBank;

    // Per frame metrics
    DWORD m_dwMetricsVerticesPerFrame;
    DWORD m_dwMetricsTrianglesPerFrame;

    // Last marker id specified
    ULONGLONG m_ullLastMarkerId;

    // last marker ID that was consumed
    ULONGLONG m_ullLastConsumedMarkerId;

    // Last frame number given to AdvanceFrame, when this changes
    // we free resources from last two frames as appropriate.
    UINT m_uFrameNumber;

    // Active and free marker lists
    DynArray<CGPUMarker *> m_rgpMarkerActive;
    DynArray<CGPUMarker *> m_rgpMarkerFree;

    UINT m_uNumSuccessfulPresentsSinceMarkerFlush;

    DWORD m_dwGPUMarkerFlags;

    UINT m_presentFailureWindowMessage;

#if DBG_STEP_RENDERING
public:
    void DbgBeginStepRenderingPresent() {
        Assert(!m_fDbgInStepRenderingPresent);
        m_fDbgInStepRenderingPresent = true;
    }
    void DbgEndStepRenderingPresent() {
        Assert(m_fDbgInStepRenderingPresent);
        m_fDbgInStepRenderingPresent = false;
    }
    bool DbgInStepRenderingPresent() const {
        return m_fDbgInStepRenderingPresent;
    }

    HRESULT DbgSaveSurface(
        __in_ecount(1) CD3DSurface *pD3DSurface,
        __in_ecount(1) const MilPointAndSizeL &rcSave
        );

    HRESULT DbgRestoreSurface(
        __inout_ecount(1) CD3DSurface *pD3DSurface,
        __in_ecount(1) const MilPointAndSizeL &rcRestore
        );

    BOOL DbgCanShrinkRectLinear() const {
        return TRUE;
    }

private:
    bool m_fDbgInStepRenderingPresent;
    CD3DSurface *m_pDbgSaveSurface;

#endif DBG_STEP_RENDERING

#if DBG
public:
    // per-frame debug info logging
    CD3DLog m_log;

    // Device thread protection checker
    mutable CAssertEntry    m_oDbgEntryCheck;
#endif DBG


public:
    // Shader effect pipeline preparation.
    HRESULT PrepareShaderEffectPipeline(bool useVS30);
    HRESULT SetPassThroughPixelShader();
    HRESULT SetRenderTargetForEffectPipeline(__in_ecount(1) CD3DSurface *pD3DSurface);
    bool Is128BitFPTextureSupported() const;
private:

    ID3D11VertexShader* m_pEffectPipelineVertexShader;  // Vertex shader for shader effects pipeline.
    ID3D11Buffer* m_pEffectPipelineVertexBuffer;        // Scratch buffer for the shader effects pipeline.
    ID3D11PixelShader* m_pEffectPipelinePassThroughPixelShader; // Pass-through pixel shader for shader effects pipelines.
};

//+-----------------------------------------------------------------------------
//
//  Member:
//      CD3DDeviceLevel1::UpdateMetrics()
//
//  Synopsis:
//      Update metrics if tracking them
//
//------------------------------------------------------------------------------
MIL_FORCEINLINE void
CD3DDeviceLevel1::UpdateMetrics(UINT uNumVertices, UINT uNumPrimitives)
{
    if (g_pMediaControl)
    {
        m_dwMetricsVerticesPerFrame += uNumVertices;
        m_dwMetricsTrianglesPerFrame += uNumPrimitives;
    }
}

//+-----------------------------------------------------------------------------
//
//  Function:
//      DbgIsPixelZoomMode()
//
//  Synopsis:
//      Is pixel zoom mode enabled
//
//------------------------------------------------------------------------------

#if DBG
extern int c_dbgPixelZoomModeScale;
extern bool DbgIsPixelZoomMode();
#endif
