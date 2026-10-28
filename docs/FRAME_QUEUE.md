# 프레임 큐 (D19): 앱 렌더 스레드를 WPF 컴포지션에 잇기

D3D11 엔진에만 있는 기능이다(D8). 앱이 자기 `ID3D11Device`로 그린 텍스처 링을 **UI 스레드 개입 없이** WPF 비주얼 트리 안의
이미지 소스로 컴포지팅한다. 첫 소비자는 수십 개의 프리뷰를 HwndHost + 스왑체인으로 그리던 방송용 멀티뷰 앱이다. 그 구조의
문제(에어스페이스: HWND 위에 WPF 오버레이·툴팁·어도너를 올릴 수 없음, 스크롤 클리핑 우회, 호스트 재생성)를 없애는 것이 목표다.

## 1. 요구사항 (소비자 코드의 사실에 근거)

생산자(렌더 스레드)
- **R1 논블로킹.** 어느 스레드에서든 `TryAcquire`와 `Present`를 부른다. 빈 버퍼가 없으면 `TryAcquire`는 즉시 실패하고 그 프레임은
  건너뛴다. `Present`는 UI 스레드가 바쁘거나 모달 중이거나 창이 최소화·가려짐 상태여도 막히거나 누적하지 않는다.
- **R2 텍스처 소유 두 모드.** (a) 소비자가 자기 디바이스에서 `MISC_SHARED_NTHANDLE | MISC_SHARED_KEYEDMUTEX`로 만든 링을 NT 핸들로
  등록(필수). (b) 엔진이 링을 만들고 NT 핸들을 돌려주면 소비자가 `OpenSharedResource1`로 연다(선택). 레거시 핸들은 쓰지 않는다.
- **R3 동기화 규약.** keyed mutex 키: 생산자는 0으로 `AcquireSync`, 1로 `ReleaseSync`. 컴포지션은 1로(타임아웃 2 ms) 잡아 복사하고
  0으로 놓는다. 실패하면 직전 프레임을 유지한다. keyed mutex가 GPU 순서를 보장하므로 생산자는 `Flush`를 부를 필요가 없다.
- **R4 최신 프레임만.** 컴포지션은 마지막 `Present`된 버퍼 하나만 그린다. 생산자가 빠르면 중간 프레임은 버리고, 느리거나 멈추면
  마지막 프레임을 계속 보인다. 큐 깊이 2~4, 추가 지연은 컴포지션 1패스 이내.
- **R5 컴포지션 깨우기.** `Present`가 컴포지션 패스를 예약한다. 프레임마다 `Lock`/`AddDirtyRect`/`Unlock`이 필요 없다(최초 바인딩 때 한 번).

크기·형식·표시
- **R6 크기 변경.** 소비자가 새 링을 만들어 교체한다. 이미지 소스는 텍스처 픽셀 크기를 보고하고 표시 크기는 WPF `Stretch`가 맡는다.
- **R7 알파.** 프리멀티플라이드 BGRA 알파를 존중한다(레터박스를 WPF 배경으로 처리). 불투명 힌트는 선택.
- **R8 1:1 픽셀.** Per-monitor DPI에서 텍스처 픽셀을 화면 픽셀에 1:1로 놓을 수단(`D3DImage(dpiX, dpiY)` 생성자).

수명·바인딩
- **R9 스레드 안전한 수명.** UI 스레드에서 만들고 파기하며, 그 순간 렌더 스레드가 `Present` 중이어도 안전하다. 파기 뒤 호출은 실패 코드.
- **R10 분리·재부착.** 같은 큐를 다른 이미지 소스에 다시 붙일 수 있다(탭 전환, 창 재생성).
- **R11 바인딩 없는 Present.** 붙기 전·떼어진 뒤의 `Present`는 "미표시" 코드로 즉시 돌아오고 마지막 프레임은 다음 바인딩 때 보인다.

디스플레이·디바이스·폴백
- **R12 다중 디스플레이·어댑터.** 창이 디스플레이 사이를 옮기거나 걸쳐도(D23) 동작. 다른 LUID면 정의된 실패(소비자 폴백).
- **R13 엔진 디바이스 손실.** M10 복구 뒤 `IsFrontBufferAvailable` 알림으로 소비자가 다시 붙인다. 복구 중 `Present`는 "미표시".
- **R14 소프트웨어 컴포지션.** 하드웨어 큐 가능 여부를 묻는 능력 질의가 있다. 불가능하면 소비자는 HwndHost 경로로 폴백한다.
  스톡 엔진에서는 `IsAvailable == false`로 같은 폴백.

관리 API
- **R15 WPF 무의존.** 생성·`TryAcquire`·`Present`·통계·파기는 WPF 타입 없이 쓴다(`WpfGfx.Dx11.Interop`). 소비자 쪽은 큐를
  이미지에 넘기는 한 줄만 WPF다.
- **R16 핫패스 무할당.** 프레임당 관리 힙 할당 0, P/Invoke 두 번(`TryAcquire`, `Present`).
- **R17 진단.** 큐별 카운터(Present, 생산자 드롭, 컴포지션 타임아웃, 표시 수) 조회. 실패 HRESULT는 `WPFGFX_DX11_LOG`에 남는다.

## 2. 설계

```
앱 렌더 스레드                       엔진(렌더 스레드)                   컴포지션 패스
TryAcquire → i (Free→Producing)
AcquireSync(0) · 그리기 · ReleaseSync(1)
Present(i)  (Producing→Presented,      OnFrameQueueFrame →
             이전 Presented 는 Free)    ScheduleCompositionPass ─────→  ProcessPendingFrameQueueImages
                                                                        CMilD3DImageDuce::PresentPendingFrame
                                                                        → CInteropDeviceBitmap::PresentFromFrameQueue
                                                                        → CMilFrameQueue::CopyPendingFrame
                                                                          AcquireSync(1, 2ms) · CopySubresourceRegion
                                                                          → 프런트 버퍼(색 소스) · ReleaseSync(0) · Free
                                                                        → NotifyOnChanged(이미지) → 비주얼 재렌더
```

- 네이티브 객체 `CMilFrameQueue`(`core/hw/FrameQueue.cpp`): `IUnknown` + 내부 IID. `D3DImage.SetBackBuffer`에 이 포인터를 넘기면
  `InteropDeviceBitmap_Create`가 `QueryInterface`로 알아보고 `CInteropDeviceBitmap`을 **프레임 큐 모드**(갱신 방법 `FrameQueue`)로
  만든다. 관리 코드와 DUCE 프로토콜은 그대로다(D1).
- 엔진은 링을 `ID3D11Device1::OpenSharedResource1`로 자기 디바이스에서 열고, 프런트 버퍼(기존 `CHwDeviceBitmapColorSource`)로
  복사한다. 복사 한 번은 다른 디스플레이·의존 색 소스·소프트웨어 리드백 같은 기존 경로가 그대로 맞물리기 위한 대가다.
- 컴포지션 깨우기는 미디어와 같은 경로다: 리소스(`CMilD3DImageDuce`)가 `IMilFrameQueueNotify`를 구현해 생산자 스레드에서
  `CComposition::ScheduleCompositionPass()`를 부르고, 패스 시작(`ProcessPendingFrameQueueImages`)에서 대기 프레임을 올린다.
- 버퍼 상태 머신은 원자 연산만 쓴다: Free → Producing(TryAcquire) → Presented(Present) → Reading(컴포지션) → Free.
  `Discard`는 Producing → Free. 새 `Present`가 아직 안 읽힌 Presented 버퍼를 Free로 돌리면 "생산자 드롭"으로 센다.
- 디바이스 손실: `CInteropDeviceBitmap`의 어댑터 알림이 큐를 `Unbind`하고 `IsFrontBufferAvailable`을 내린다. 소비자가 다시
  `SetFrameQueue`하면 새 디바이스에서 다시 연다. 큐 객체 자체는 살아 있어 마지막 프레임이 유지된다.

## 3. API

네이티브 export(ABI 2, `core/api/api_dx11.cpp`, `WpfGfx.Dx11.Interop`가 P/Invoke):

```
HRESULT WpfGfxDx11_FrameQueue_Create(const WpfGfxFrameQueueDesc*, IUnknown** ppQueue);
   desc: cbSize, uWidth, uHeight, uFormat(B8G8R8A8_UNORM), cBuffers(2..4),
         uFlags(0x1 UserTextures | 0x2 EngineTextures | 0x4 Opaque), HANDLE rgSharedHandles[4]
HRESULT WpfGfxDx11_FrameQueue_GetSharedHandles(IUnknown*, HANDLE* pHandles, UINT c);  // EngineTextures: 복제본, 소비자가 닫는다
HRESULT WpfGfxDx11_FrameQueue_TryAcquire(IUnknown*, UINT* pIndex);   // S_OK | S_FALSE(빈 버퍼 없음)
HRESULT WpfGfxDx11_FrameQueue_Present(IUnknown*, UINT index);        // S_OK | S_FALSE(미표시: 바인딩 전)
HRESULT WpfGfxDx11_FrameQueue_Discard(IUnknown*, UINT index);        // 그리지 못한 버퍼 반납
HRESULT WpfGfxDx11_FrameQueue_GetStats(IUnknown*, WpfGfxFrameQueueStats*);
HRESULT WpfGfxDx11_FrameQueue_QueryCapability(UINT displayIndex, BOOL* pHardware);
```

관리:
- `WpfGfx.Dx11.Interop`(WPF 참조 없음): `Dx11FrameQueue` — `IsAvailable`, `IsHardwareCapable(display)`, `Create(w, h, ntHandles, opaque)`,
  `CreateWithEngineTextures(w, h, buffers)`, `TryAcquire(out index)`, `Present(index)`, `Discard(index)`, `Stats`, `NativePointer`, `Dispose`.
- `WpfGfx.Dx11`: `D3D11Image.SetFrameQueue(queue)` — `Lock`/`SetFrameQueue`/`AddDirtyRect`/`Unlock` 한 번. `IsFrontBufferAvailableChanged`에서
  같은 호출로 다시 붙인다(R13).

## 4. 합격 기준과 상태

스모크 앱 `--fq <png> [--fq-queues N] [--fq-seconds S] [--fq-block]`: 워커 스레드가 N개 큐(1280×720·640×360·320×180 혼합)에 60 fps로
그린다. `--fq-block`은 UI 스레드를 0.5초마다 100 ms 막는다. 보고: 프레임 수, Present/표시/드롭/타임아웃 합계, 큐 1개의
`TryAcquire`+렌더+`Present` 평균·최대 µs, 250 ms 간격 두 캡처의 변화 비율.

| 기준 | 상태 |
|---|---|
| A1 다중 큐 + UI 블로킹 중 계속 갱신 | ✅ 큐 8개·32개, UI 100 ms 블로킹 중 캡처 변화 80.6% / 78.1%, 뮤텍스 실패 0 (게이트 `frame queues` 행) |
| A2 생산자 비용(큐당 50 µs 미만) | ✅ 큐 8개 39 µs, 32개 30 µs(틱당 약 1 ms, 60 fps 유지) |
| A3 지연(vsync 1회 + 1패스) | 미측정 |
| A4 느린 생산자(마지막 프레임 유지) | 설계상 보장(상태 머신), 전용 테스트 없음 |
| A5 링 교체 무깜빡임 | 미검증 |
| A6 디바이스 손실 복구 | `WPFGFX_DX11_TEST_DEVICE_LOSS` + `IsFrontBufferAvailableChanged` 재바인딩 |
| A7 두 모니터 걸침·창 이동 | 기존 D3DImage 경로와 동일(D23), 전용 테스트 없음 |
| A8 스톡 엔진 폴백 | ✅ 게이트: 스톡에서 `not available` 보고, 예외 없음 |
| A9 Tier 0에서 생산자 비블로킹 | 능력 질의 false → 소비자 폴백 |
