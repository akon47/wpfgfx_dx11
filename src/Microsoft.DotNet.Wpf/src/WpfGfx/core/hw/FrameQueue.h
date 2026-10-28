// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.


//+-----------------------------------------------------------------------------
//
//  Description:
//      프레임 큐 (D19). 앱의 렌더 스레드가 자기 D3D11 디바이스로 그린 텍스처 링을.
//      UI 스레드 개입 없이 컴포지션에 잇는다. D3DImage(CInteropDeviceBitmap)의 백버퍼.
//      자리에 이 객체를 넘기면 엔진이 QueryInterface 로 알아보고 프레임 큐 모드로 묶는다.
//
//      규약(소비자 ↔ 컴포지션):
//      - 버퍼마다 keyed mutex. 생산자는 키 0 으로 AcquireSync 한 뒤 그리고 키 1 로.
//        ReleaseSync 한 다음 Present 한다. 컴포지션은 키 1 로(짧은 타임아웃) 잡아.
//        복사하고 키 0 으로 놓는다. keyed mutex 가 GPU 순서를 보장하므로 Flush 는 필요 없다.
//      - TryAcquire/Present 는 어느 스레드에서든 블로킹 없이 돌아온다. 빈 버퍼가 없으면.
//        S_FALSE. 컴포지션이 아직 안 가져간 이전 프레임은 버려진다(최신 프레임만).
//      - Present 는 컴포지션 패스를 예약한다. 묶이지 않았으면 S_FALSE 를 돌려주고.
//        프레임은 다음 바인딩 때 표시된다.
//
//------------------------------------------------------------------------------

#pragma once

MtExtern(CMilFrameQueue);

class CHwDeviceBitmapColorSource;

// Implemented by the composition resource that shows the queue (resources
// layer); called on the producer thread when a frame was presented.
interface IMilFrameQueueNotify
{
    virtual void OnFrameQueueFrame() = 0;
};

// {6E9A2C1B-5D3F-4B8E-9C47-2F1A0B7D3E55} - 내부 식별용. 관리 코드는 IUnknown 만 본다.
DEFINE_GUID(IID_IMilFrameQueue, 0x6e9a2c1b, 0x5d3f, 0x4b8e, 0x9c, 0x47, 0x2f, 0x1a, 0x0b, 0x7d, 0x3e, 0x55);

#define WPFGFX_FRAME_QUEUE_MAX_BUFFERS      4

#define WPFGFX_FRAME_QUEUE_FLAG_USER_TEXTURES   0x1     // rgSharedHandles 는 소비자의 NT 핸들.
#define WPFGFX_FRAME_QUEUE_FLAG_ENGINE_TEXTURES 0x2     // 엔진이 링을 만들고 NT 핸들을 돌려준다.
#define WPFGFX_FRAME_QUEUE_FLAG_OPAQUE          0x4     // 알파를 무시해도 된다는 힌트.

// ABI: cbSize 로 시작, 필드는 끝에만 덧붙인다 (WpfGfx.Dx11.Interop 과 같은 배치).
struct WpfGfxFrameQueueDesc
{
    UINT cbSize;
    UINT uWidth;
    UINT uHeight;
    UINT uFormat;                                       // DXGI_FORMAT; B8G8R8A8_UNORM 만 받는다.
    UINT cBuffers;                                      // 2..WPFGFX_FRAME_QUEUE_MAX_BUFFERS
    UINT uFlags;                                        // WPFGFX_FRAME_QUEUE_FLAG_*
    HANDLE rgSharedHandles[WPFGFX_FRAME_QUEUE_MAX_BUFFERS];
};

struct WpfGfxFrameQueueStats
{
    UINT cbSize;
    UINT uReserved;
    UINT64 uPresented;              // Present 성공 횟수.
    UINT64 uDroppedByProducer;      // 컴포지션이 가져가기 전에 다음 프레임으로 대체된 횟수.
    UINT64 uAcquireTimeouts;        // 컴포지션 쪽 keyed mutex 타임아웃.
    UINT64 uDisplayed;              // 엔진 프런트 버퍼로 복사된 프레임 수.
    UINT64 uAcquireFailures;        // TryAcquire 가 S_FALSE 를 돌려준 횟수.
};

class CMilFrameQueue :
    public CMILCOMBase
{
public:
    static HRESULT Create(
        __in_ecount(1) const WpfGfxFrameQueueDesc *pDesc,
        __deref_out_ecount(1) CMilFrameQueue **ppQueue
        );

    DECLARE_COM_BASE;

    STDMETHOD(HrFindInterface)(
        __in_ecount(1) REFIID riid,
        __deref_out void **ppvObject
        );

    //
    // Producer side (any thread, never blocks)
    //

    HRESULT TryAcquire(__out_ecount(1) UINT *puIndex);
    HRESULT Present(UINT uIndex);
    HRESULT Discard(UINT uIndex);       // acquired but not rendered: back to free
    void GetStats(__out_ecount(1) WpfGfxFrameQueueStats *pStats) const;
    HRESULT GetSharedHandles(__out_ecount(cHandles) HANDLE *pHandles, UINT cHandles) const;

    //
    // Composition side (render thread)
    //

    UINT GetWidth() const { return m_uWidth; }
    UINT GetHeight() const { return m_uHeight; }
    bool IsOpaque() const { return (m_uFlags & WPFGFX_FRAME_QUEUE_FLAG_OPAQUE) != 0; }
    bool HasPendingFrame() const { return m_iPending >= 0; }

    // Opens the ring on the engine device the front buffer lives on. Re-opens
    // when the device changes (device loss recovery).
    HRESULT BindToDevice(__in_ecount(1) CD3DDeviceLevel1 *pDevice);
    void Unbind();
    bool IsBound() const { return m_pDevice != NULL; }

    // Copies the pending frame into the front buffer color source. S_FALSE when
    // there is nothing pending or the mutex could not be taken in time (the
    // previous frame stays).
    HRESULT CopyPendingFrame(
        __inout_ecount(1) CHwDeviceBitmapColorSource *pColorSource
        );

    // The composition resource showing this queue; Present notifies it so it
    // can schedule a composition pass. Set and cleared on the render thread.
    void SetNotifyTarget(
        __in_opt IMilFrameQueueNotify *pNotify
        );

protected:
    DECLARE_METERHEAP_ALLOC(ProcessHeap, Mt(CMilFrameQueue));

    CMilFrameQueue();
    virtual ~CMilFrameQueue();

    HRESULT Init(__in_ecount(1) const WpfGfxFrameQueueDesc *pDesc);
    HRESULT CreateEngineTextures();

    // A presented buffer that is dropped unread still has its keyed mutex at
    // key 1 (the producer released it that way). Put it back to 0 so the
    // producer's next AcquireSync(0) does not wait for a composition pass.
    void ResetDroppedBuffer(UINT uIndex);

private:
    enum BufferState
    {
        Free = 0,
        Producing = 1,
        Presented = 2,
        Reading = 3
    };

    UINT m_uWidth;
    UINT m_uHeight;
    DXGI_FORMAT m_format;
    UINT m_cBuffers;
    UINT m_uFlags;

    HANDLE m_rgHandles[WPFGFX_FRAME_QUEUE_MAX_BUFFERS];         // duplicated into this process; closed in dtor
    ID3D11Texture2D *m_rgpTextures[WPFGFX_FRAME_QUEUE_MAX_BUFFERS];
    IDXGIKeyedMutex *m_rgpMutexes[WPFGFX_FRAME_QUEUE_MAX_BUFFERS];
    CD3DDeviceLevel1 *m_pDevice;                                // device the ring is opened on (ref)
    ID3D11Texture2D *m_rgpOwnedTextures[WPFGFX_FRAME_QUEUE_MAX_BUFFERS];   // engine-textures mode: keeps the ring alive
    bool m_fEngineTextures;                                     // ring created by the engine (never re-opened)

    volatile LONG m_rgState[WPFGFX_FRAME_QUEUE_MAX_BUFFERS];
    volatile LONG m_rgfKeyLeftAtOne[WPFGFX_FRAME_QUEUE_MAX_BUFFERS];  // dropped while unbound: mutex still at key 1
    volatile LONG m_iPending;                                   // -1 = none
    volatile LONG m_uNextAcquire;

    mutable CCriticalSection m_cs;                              // notify target
    bool m_fCsValid;
    IMilFrameQueueNotify *m_pNotifyNoRef;

    volatile LONG64 m_cPresented;
    volatile LONG64 m_cDroppedByProducer;
    volatile LONG64 m_cAcquireTimeouts;
    volatile LONG64 m_cDisplayed;
    volatile LONG64 m_cAcquireFailures;
};
