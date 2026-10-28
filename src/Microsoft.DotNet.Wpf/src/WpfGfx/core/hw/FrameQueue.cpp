// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//
//  Description:
//      프레임 큐 (D19). FrameQueue.h 참고.
//
//------------------------------------------------------------------------------

#include "precomp.hpp"
#include "mildx11log.h"

MtDefine(CMilFrameQueue, MILRender, "CMilFrameQueue");

static const DWORD sc_dwCompositionAcquireTimeoutMs = 2;

//+-----------------------------------------------------------------------------
//
//  Member:
//      CMilFrameQueue::Create
//
//------------------------------------------------------------------------------

HRESULT
CMilFrameQueue::Create(
    __in_ecount(1) const WpfGfxFrameQueueDesc *pDesc,
    __deref_out_ecount(1) CMilFrameQueue **ppQueue
    )
{
    HRESULT hr = S_OK;
    CMilFrameQueue *pQueue = NULL;

    *ppQueue = NULL;

    CHECKPTRARG(pDesc);

    if (pDesc->cbSize < sizeof(WpfGfxFrameQueueDesc))
    {
        IFC(E_INVALIDARG);
    }

    pQueue = new CMilFrameQueue();
    IFCOOM(pQueue);
    pQueue->AddRef();   // CMILCOMBase starts at zero

    IFC(pQueue->Init(pDesc));

    *ppQueue = pQueue;
    pQueue = NULL;

Cleanup:
    if (FAILED(hr))
    {
        MIL_DX11_LOG("frame queue: create failed hr=0x%08x", hr);
    }

    ReleaseInterface(pQueue);

    RRETURN(hr);
}

CMilFrameQueue::CMilFrameQueue()
{
    m_uWidth = 0;
    m_uHeight = 0;
    m_format = DXGI_FORMAT_UNKNOWN;
    m_cBuffers = 0;
    m_uFlags = 0;
    ZeroMemory(m_rgHandles, sizeof(m_rgHandles));
    ZeroMemory(m_rgpTextures, sizeof(m_rgpTextures));
    ZeroMemory(m_rgpMutexes, sizeof(m_rgpMutexes));
    ZeroMemory(m_rgpOwnedTextures, sizeof(m_rgpOwnedTextures));
    m_pDevice = NULL;
    m_fEngineTextures = false;
    ZeroMemory(const_cast<LONG *>(m_rgState), sizeof(m_rgState));
    ZeroMemory(const_cast<LONG *>(m_rgfKeyLeftAtOne), sizeof(m_rgfKeyLeftAtOne));
    m_iPending = -1;
    m_uNextAcquire = 0;
    m_fCsValid = false;
    m_pNotifyNoRef = NULL;
    m_cPresented = 0;
    m_cDroppedByProducer = 0;
    m_cAcquireTimeouts = 0;
    m_cDisplayed = 0;
    m_cAcquireFailures = 0;
}

CMilFrameQueue::~CMilFrameQueue()
{
    Unbind();

    for (UINT i = 0; i < WPFGFX_FRAME_QUEUE_MAX_BUFFERS; i++)
    {
        ReleaseInterface(m_rgpOwnedTextures[i]);

        if (m_rgHandles[i] != NULL)
        {
            CloseHandle(m_rgHandles[i]);
            m_rgHandles[i] = NULL;
        }
    }

    if (m_fCsValid)
    {
        m_cs.DeInit();
    }
}

STDMETHODIMP
CMilFrameQueue::HrFindInterface(
    __in_ecount(1) REFIID riid,
    __deref_out void **ppvObject
    )
{
    HRESULT hr = E_NOINTERFACE;

    if (ppvObject)
    {
        if (riid == IID_IMilFrameQueue)
        {
            *ppvObject = this;
            hr = S_OK;
        }
        else if (riid == IID_IUnknown)
        {
            *ppvObject = static_cast<IUnknown *>(this);
            hr = S_OK;
        }
    }

    return hr;
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CMilFrameQueue::Init
//
//------------------------------------------------------------------------------

HRESULT
CMilFrameQueue::Init(
    __in_ecount(1) const WpfGfxFrameQueueDesc *pDesc
    )
{
    HRESULT hr = S_OK;

    if (   pDesc->uWidth == 0 || pDesc->uHeight == 0
        || pDesc->uWidth > 16384 || pDesc->uHeight > 16384
        || pDesc->cBuffers < 2 || pDesc->cBuffers > WPFGFX_FRAME_QUEUE_MAX_BUFFERS)
    {
        IFC(E_INVALIDARG);
    }

    if (pDesc->uFormat != DXGI_FORMAT_B8G8R8A8_UNORM)
    {
        IFC(WGXERR_UNSUPPORTEDPIXELFORMAT);
    }

    const UINT uMode = pDesc->uFlags & (WPFGFX_FRAME_QUEUE_FLAG_USER_TEXTURES | WPFGFX_FRAME_QUEUE_FLAG_ENGINE_TEXTURES);

    if (uMode != WPFGFX_FRAME_QUEUE_FLAG_USER_TEXTURES && uMode != WPFGFX_FRAME_QUEUE_FLAG_ENGINE_TEXTURES)
    {
        IFC(E_INVALIDARG);
    }

    IFC(m_cs.Init());
    m_fCsValid = true;

    m_uWidth = pDesc->uWidth;
    m_uHeight = pDesc->uHeight;
    m_format = static_cast<DXGI_FORMAT>(pDesc->uFormat);
    m_cBuffers = pDesc->cBuffers;
    m_uFlags = pDesc->uFlags;

    if (uMode == WPFGFX_FRAME_QUEUE_FLAG_USER_TEXTURES)
    {
        for (UINT i = 0; i < m_cBuffers; i++)
        {
            if (pDesc->rgSharedHandles[i] == NULL)
            {
                IFC(E_INVALIDARG);
            }

            // Our own handle, so the consumer may close its copy right away.
            IFCW32(DuplicateHandle(
                GetCurrentProcess(),
                pDesc->rgSharedHandles[i],
                GetCurrentProcess(),
                &m_rgHandles[i],
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS
                ));
        }
    }
    else
    {
        IFC(CreateEngineTextures());
    }

    MIL_DX11_LOG("frame queue %p: %ux%u buffers=%u flags=0x%x", this, m_uWidth, m_uHeight, m_cBuffers, m_uFlags);

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Member:
//      CMilFrameQueue::CreateEngineTextures
//
//  Synopsis:
//      Engine-textures mode: the ring is created on the primary display's
//      device with NT shared handles and keyed mutexes; the consumer opens
//      the handles on its own device.
//
//------------------------------------------------------------------------------

HRESULT
CMilFrameQueue::CreateEngineTextures()
{
    HRESULT hr = S_OK;
    CD3DDeviceManager *pDeviceManager = CD3DDeviceManager::Get();
    CDisplaySet const *pDisplaySet = NULL;
    CD3DDeviceLevel1 *pDevice = NULL;

    if (g_DisplayManager.HasCurrentDisplaySet())
    {
        g_DisplayManager.GetCurrentDisplaySet(&pDisplaySet);
    }
    else
    {
        IFC(g_DisplayManager.DangerousGetLatestDisplaySet(&pDisplaySet));
    }

    if (!pDisplaySet->IsD3DAvailable() || pDisplaySet->GetNumD3DRecognizedAdapters() == 0)
    {
        IFC(WGXERR_NO_HARDWARE_DEVICE);
    }

    IFC(pDeviceManager->GetD3DDeviceAndPresentParams(
        GetDesktopWindow(),
        MilRTInitialization::Default,
        pDisplaySet->Display(0),
        MilDeviceKind_Hardware,
        &pDevice,
        NULL,
        NULL));

    {
        ENTER_DEVICE_FOR_SCOPE(*pDevice);

        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width = m_uWidth;
        desc.Height = m_uHeight;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = m_format;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_NTHANDLE | D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;

        for (UINT i = 0; i < m_cBuffers; i++)
        {
            IDXGIResource1 *pResource = NULL;

            IFC(MilTranslateD3D11Error(pDevice->GetD3DDeviceNoRef()->CreateTexture2D(&desc, NULL, &m_rgpOwnedTextures[i])));

            hr = m_rgpOwnedTextures[i]->QueryInterface(__uuidof(IDXGIResource1), reinterpret_cast<void **>(&pResource));

            if (SUCCEEDED(hr))
            {
                hr = pResource->CreateSharedHandle(
                    NULL,
                    DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE,
                    NULL,
                    &m_rgHandles[i]
                    );
            }

            ReleaseInterface(pResource);
            IFC(hr);
        }
    }

    m_fEngineTextures = true;

Cleanup:
    ReleaseInterface(pDevice);
    ReleaseInterface(pDisplaySet);
    CD3DDeviceManager::Release();

    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Producer side
//
//------------------------------------------------------------------------------

HRESULT
CMilFrameQueue::TryAcquire(
    __out_ecount(1) UINT *puIndex
    )
{
    *puIndex = 0;

    if (!m_fCsValid)
    {
        RRETURN(E_FAIL);
    }

    const UINT uStart = static_cast<UINT>(m_uNextAcquire);

    for (UINT n = 0; n < m_cBuffers; n++)
    {
        const UINT i = (uStart + n) % m_cBuffers;

        if (InterlockedCompareExchange(&m_rgState[i], Producing, Free) == Free)
        {
            InterlockedExchange(&m_uNextAcquire, static_cast<LONG>((i + 1) % m_cBuffers));
            *puIndex = i;
            return S_OK;
        }
    }

    InterlockedIncrement64(&m_cAcquireFailures);

    return S_FALSE;
}

HRESULT
CMilFrameQueue::Present(
    UINT uIndex
    )
{
    if (!m_fCsValid)
    {
        RRETURN(E_FAIL);
    }

    if (   uIndex >= m_cBuffers
        || InterlockedCompareExchange(&m_rgState[uIndex], Presented, Producing) != Producing)
    {
        RRETURN(E_INVALIDARG);
    }

    //
    // Only the latest frame matters: a frame the composition has not taken
    // yet goes back to the producer.
    //
    const LONG iPrevious = InterlockedExchange(&m_iPending, static_cast<LONG>(uIndex));

    if (iPrevious >= 0 && iPrevious != static_cast<LONG>(uIndex))
    {
        if (InterlockedCompareExchange(&m_rgState[iPrevious], Free, Presented) == Presented)
        {
            InterlockedIncrement64(&m_cDroppedByProducer);
            ResetDroppedBuffer(static_cast<UINT>(iPrevious));
        }
    }

    InterlockedIncrement64(&m_cPresented);

    bool fBound = false;

    {
        CGuard<CCriticalSection> guard(m_cs);

        if (m_pNotifyNoRef != NULL)
        {
            m_pNotifyNoRef->OnFrameQueueFrame();
            fBound = true;
        }
    }

    return fBound ? S_OK : S_FALSE;
}

HRESULT
CMilFrameQueue::Discard(
    UINT uIndex
    )
{
    if (   uIndex >= m_cBuffers
        || InterlockedCompareExchange(&m_rgState[uIndex], Free, Producing) != Producing)
    {
        RRETURN(E_INVALIDARG);
    }

    return S_OK;
}

void
CMilFrameQueue::GetStats(
    __out_ecount(1) WpfGfxFrameQueueStats *pStats
    ) const
{
    pStats->uPresented = m_cPresented;
    pStats->uDroppedByProducer = m_cDroppedByProducer;
    pStats->uAcquireTimeouts = m_cAcquireTimeouts;
    pStats->uDisplayed = m_cDisplayed;
    pStats->uAcquireFailures = m_cAcquireFailures;
}

HRESULT
CMilFrameQueue::GetSharedHandles(
    __out_ecount(cHandles) HANDLE *pHandles,
    UINT cHandles
    ) const
{
    HRESULT hr = S_OK;

    if (cHandles < m_cBuffers)
    {
        IFC(E_INVALIDARG);
    }

    // Duplicates: the consumer owns and closes them.
    for (UINT i = 0; i < m_cBuffers; i++)
    {
        IFCW32(DuplicateHandle(
            GetCurrentProcess(),
            m_rgHandles[i],
            GetCurrentProcess(),
            &pHandles[i],
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS
            ));
    }

Cleanup:
    RRETURN(hr);
}

//+-----------------------------------------------------------------------------
//
//  Composition side
//
//------------------------------------------------------------------------------

HRESULT
CMilFrameQueue::BindToDevice(
    __in_ecount(1) CD3DDeviceLevel1 *pDevice
    )
{
    HRESULT hr = S_OK;
    ID3D11Device1 *pDevice1 = NULL;

    if (pDevice == m_pDevice)
    {
        goto Cleanup;
    }

    Unbind();

    IFC(pDevice->GetD3DDeviceNoRef()->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void **>(&pDevice1)));

    for (UINT i = 0; i < m_cBuffers; i++)
    {
        D3D11_TEXTURE2D_DESC desc;

        IFC(MilTranslateD3D11Error(pDevice1->OpenSharedResource1(
            m_rgHandles[i],
            __uuidof(ID3D11Texture2D),
            reinterpret_cast<void **>(&m_rgpTextures[i])
            )));

        m_rgpTextures[i]->GetDesc(&desc);

        if (desc.Width != m_uWidth || desc.Height != m_uHeight || desc.Format != m_format)
        {
            MIL_DX11_LOG("frame queue %p: buffer %u is %ux%u fmt=%d, expected %ux%u fmt=%d",
                         this, i, desc.Width, desc.Height, desc.Format, m_uWidth, m_uHeight, m_format);
            IFC(E_INVALIDARG);
        }

        // No keyed mutex: the copy is unsynchronized (the consumer was told).
        IGNORE_HR(m_rgpTextures[i]->QueryInterface(__uuidof(IDXGIKeyedMutex), reinterpret_cast<void **>(&m_rgpMutexes[i])));
    }

    m_pDevice = pDevice;
    m_pDevice->AddRef();

    // Frames dropped while nothing was bound left their mutexes at key 1.
    for (UINT i = 0; i < m_cBuffers; i++)
    {
        if (InterlockedExchange(&m_rgfKeyLeftAtOne[i], 0) != 0 && m_rgpMutexes[i] != NULL)
        {
            if (m_rgpMutexes[i]->AcquireSync(1, 0) == S_OK)
            {
                IGNORE_HR(m_rgpMutexes[i]->ReleaseSync(0));
            }
        }
    }

    MIL_DX11_LOG("frame queue %p: bound to device %p (keyed mutex: %d)", this, pDevice, m_rgpMutexes[0] != NULL ? 1 : 0);

Cleanup:
    if (FAILED(hr))
    {
        MIL_DX11_LOG("frame queue %p: bind failed hr=0x%08x", this, hr);
        Unbind();
    }

    ReleaseInterface(pDevice1);

    RRETURN(hr);
}

void
CMilFrameQueue::Unbind()
{
    for (UINT i = 0; i < WPFGFX_FRAME_QUEUE_MAX_BUFFERS; i++)
    {
        ReleaseInterface(m_rgpMutexes[i]);
        ReleaseInterface(m_rgpTextures[i]);
    }

    ReleaseInterface(m_pDevice);
}

HRESULT
CMilFrameQueue::CopyPendingFrame(
    __inout_ecount(1) CHwDeviceBitmapColorSource *pColorSource
    )
{
    HRESULT hr = S_OK;
    bool fAcquired = false;

    const LONG i = InterlockedExchange(&m_iPending, -1);

    if (i < 0)
    {
        return S_FALSE;
    }

    if (InterlockedCompareExchange(&m_rgState[i], Reading, Presented) != Presented)
    {
        return S_FALSE;
    }

    if (!IsBound() || m_rgpTextures[i] == NULL)
    {
        hr = S_FALSE;
        goto PutBack;
    }

    {
        ENTER_DEVICE_FOR_SCOPE(*m_pDevice);

        if (m_rgpMutexes[i] != NULL)
        {
            HRESULT hrAcquire = m_rgpMutexes[i]->AcquireSync(1, sc_dwCompositionAcquireTimeoutMs);

            if (hrAcquire != S_OK)
            {
                InterlockedIncrement64(&m_cAcquireTimeouts);
                hr = S_FALSE;
                goto PutBack;
            }

            fAcquired = true;
        }

        {
            const CMilRectU rcAll(0, 0, m_uWidth, m_uHeight, XYWH_Parameters);

            hr = pColorSource->UpdateSurface(1, &rcAll, m_rgpTextures[i]);
        }

        if (fAcquired)
        {
            IGNORE_HR(m_rgpMutexes[i]->ReleaseSync(0));

            // The release only reaches the GPU with the next submission; without
            // this the producer's AcquireSync(0) waits until our next Present.
            m_pDevice->GetD3DContextNoRef()->Flush();
        }
    }

    InterlockedExchange(&m_rgState[i], Free);

    if (SUCCEEDED(hr))
    {
        InterlockedIncrement64(&m_cDisplayed);
    }
    else
    {
        MIL_DX11_LOG("frame queue %p: copy failed hr=0x%08x", this, hr);
    }

    RRETURN(hr);

PutBack:
    // Keep the frame for the next pass unless a newer one arrived meanwhile.
    InterlockedExchange(&m_rgState[i], Presented);

    if (InterlockedCompareExchange(&m_iPending, i, -1) != -1)
    {
        if (InterlockedCompareExchange(&m_rgState[i], Free, Presented) == Presented)
        {
            InterlockedIncrement64(&m_cDroppedByProducer);
            ResetDroppedBuffer(static_cast<UINT>(i));
        }
    }

    return hr;
}

void
CMilFrameQueue::ResetDroppedBuffer(
    UINT uIndex
    )
{
    // Keyed mutex calls are not bound to a device context, so this is safe on
    // the producer thread while the render thread uses the other buffers.
    IDXGIKeyedMutex *pMutex = m_rgpMutexes[uIndex];

    if (pMutex != NULL)
    {
        if (pMutex->AcquireSync(1, 0) == S_OK)
        {
            IGNORE_HR(pMutex->ReleaseSync(0));
        }
    }
    else
    {
        InterlockedExchange(&m_rgfKeyLeftAtOne[uIndex], 1);
    }
}

void
CMilFrameQueue::SetNotifyTarget(
    __in_opt IMilFrameQueueNotify *pNotify
    )
{
    if (!m_fCsValid)
    {
        return;
    }

    CGuard<CCriticalSection> guard(m_cs);

    m_pNotifyNoRef = pNotify;
}
