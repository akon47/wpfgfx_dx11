# 로드맵

각 Phase는 합격 기준을 모두 충족해야 끝난다. 상태: ✅ 완료, 🔄 진행 중, ⏳ 예정.

## Phase 0 — 원본 빌드와 교체 테스트 루프 ✅ (2026-10-28)

목표: 업스트림 WpfGfx를 수정 없이 빌드하고, 빌드한 DLL을 실제 WPF 앱에 꽂아 렌더링을 확인하는 루프를 만든다.

합격 기준:
- `build.cmd`가 경고·오류 없이 `wpfgfx_cor3.dll`을 만든다. ✅
- 스모크 앱이 그 DLL을 로드해 .NET 8과 .NET 10에서 Tier 2로 렌더링한다. ✅
- 저장소가 WpfGfx와 빌드에 필요한 파일만 담는다. ✅

## Phase 1 — D3D9 사용 인벤토리 ✅ (2026-10-28)

목표: `core/hw`(와 `core/av`, `core/common`, `core/resources`)가 쓰는 D3D9 API 표면을 자동 추출해
`docs/INVENTORY.md`에 D3D11 매핑표로 정리한다.

합격 기준:
- IDirect3D* 인터페이스별 메서드 호출 목록과 호출 지점(파일:줄). ✅ `docs/inventory/call-sites.md`
- 열거형 가족별 사용 값과 D3D11 대응. ✅ `docs/INVENTORY.md` §4
- 고정 기능 파이프라인과 ps_1_1 셰이더 사용처 목록. ✅ §3, §5
- "렌더링 외" D3D9 의존처 목록. ✅ §2

도구: `tools/inventory/d3d9_inventory.py` (재생성 가능).

## Phase 2 — 설계 ✅ (2026-10-28)

목표: 인벤토리를 바탕으로 치환 설계를 확정한다. 산출물: `docs/DESIGN.md`, `docs/DECISIONS.md` D9~D16.

- 디바이스·컨텍스트·스레딩·오류 매핑·caps: DESIGN §8 ✅
- 스왑체인과 프레젠테이션: D10, DESIGN §5 ✅
- 상태 객체 캐시, 입력 레이아웃, 타입 치환: DESIGN §3·§4·§6 ✅
- 셰이더(조립기 SM4, 텍스트, 효과, 고정 기능 제거): D11, D15, DESIGN §6 ✅
- ShaderEffect(D6 세부): DESIGN §7 ✅
- D3DImage(D7 세부): D14, DESIGN §9 ✅
- 미디어: D16→D25, DESIGN §10 ✅
- 작업 방식: D12(브랜치), D13(첫 마일스톤) ✅

## Phase 3 — 마일스톤 구현 ✅ (2026-10-28, M1~M11)

`dx11-port` 브랜치에서 작업하고 마일스톤마다 `main`에 병합한다(D12). 마일스톤 표와 합격 기준은 `docs/DESIGN.md` §12.

| M | 내용 | 상태 |
|---|---|---|
| M1 | D3D9 없이 컴파일, 소프트웨어 렌더링(Tier 0) | ✅ 2026-10-28. 스모크 .NET 8/10 Tier 0, 3D 셀(11번)만 빈 칸(소프트웨어 3D도 WARP 디바이스가 필요, M7) |
| M2 | 디바이스·스왑체인·클리어·프레젠트 | ✅ 2026-10-28 |
| M3 | 셰이더 파이프라인: 단색·그라데이션·경로 | ✅ 2026-10-28 |
| M4 | 텍스처·비트맵·타일 브러시 | ✅ 2026-10-28 |
| M5 | 텍스트 | ✅ 2026-10-28 |
| M6 | 중간 렌더타깃·효과·클립 | ✅ 2026-10-28 |
| M7 | 3D | ✅ 2026-10-28 |
| M8 | ShaderEffect 변환기 | ✅ 2026-10-28. 스모크 13·14번 셀(ps_2_0, ps_3_0 loop/branch)이 스톡과 일치 |
| M9 | D3DImage, 새 export, WpfGfx.Dx11 | ✅ 2026-10-28. 스모크 15번(D3D9Ex 서피스)이 스톡과 픽셀 일치, 16번(D3D11 텍스처)은 `WpfGfx.Dx11` 로 표시 |
| M10 | 투명 창, 멀티 어댑터, 디바이스 제거 복구 | ✅ 2026-10-28. 투명 창 스톡과 픽셀 일치, 두 모니터에 걸친 창 정상, 모의 디바이스 손실 후 복구 |
| M11 | 미디어 | ✅ 2026-10-28. MediaElement(H.264 MP4) 재생, 프레임이 스톡과 일치 |

M2~M7 합격 근거(2026-10-28): 스모크 12셀이 모두 하드웨어 경로로 그려지고(Tier 2, .NET 8/10), 스톡 DX9 스크린샷과의 픽셀 차이는
차이>8 인 픽셀 0.18%, 차이>64 인 픽셀 0.02%(안티앨리어싱 가장자리와 회전 애니메이션 위상). 넓은 커버리지 검증은 Phase 4.

M9 합격 근거(2026-10-28): 스모크 15번 셀(D3D9Ex 디바이스의 `IDirect3DSurface9`, 공개 D3DImage 계약)은 스톡 DX9 스크린샷과
픽셀 차이 0. 16번 셀(`WpfGfx.Dx11`의 `D3D11Image` + 앱 자체 D3D11 디바이스의 공유 텍스처)은 D3D11 엔진에서만 그려지고 스톡은
안내 문구를 보인다(전체 화면 차이 5.4%는 전부 이 셀). .NET 8/10 모두 Tier 2, 디버그 레이어 메시지 0. 새 export 2개
(`WpfGfxDx11_GetInfo`/`GetDevice`)가 기존 107개 뒤에 붙었고 `Dx11Engine.GetInfo()` 가 ABI 1, Feature Level 11.1을 보고한다.
미검증: 어댑터가 다른 디바이스의 텍스처(스테이징 복사 경로), 소프트웨어 폴백(`CopyToSoftwareBitmap` D3D11 분기).

M10 합격 근거(2026-10-28, GPU 1개·모니터 2개 환경):
- 투명 창(`AllowsTransparency`, 스모크 `--transparent`): `PrintWindow` 캡처가 스톡과 픽셀 차이 0(.NET 8/10, 한 모니터 안·두 모니터에 걸친 위치 모두).
  엔진이 `UpdateLayeredWindow`에 넘기는 백버퍼 덤프(`gdipresent1.bmp`)의 알파가 기대값과 일치(투명 영역 0, 60% 패널 153, 40% 띠 102, 전 픽셀 프리멀티플라이드).
- 두 모니터에 걸친 창(`--left 2110`): 디스플레이마다 스왑체인이 하나씩 같은 HWND에 만들어지고(DXGI 비트블트 모델은 허용) 스톡과 의도한 차이(16번 셀·회전 위상·AA 246픽셀)만 있다.
- 디바이스 손실(`WPFGFX_DX11_TEST_DEVICE_LOSS=5`): 5번째 Present 를 손실로 처리하면 디스플레이별 디바이스 2개와 스왑체인이 다시 만들어지고,
  이후 스크린샷이 정상 실행과 회전 위상 외 차이 0. `D3DImage.IsFrontBufferAvailable`이 false→true 로 바뀌고 앱이 백버퍼를 다시 설정하면 15·16번 셀도 복구(.NET 8/10).
  실제 GPU 제거·TDR 은 미검증(D24).

M11 합격 근거(2026-10-28): 스모크 `--media tests/assets/smptebars.mp4`(H.264, 160×120 정지 패턴)가 .NET 8/10에서 `MediaOpened`
160×120·`HasVideo`, 캡처 프레임이 스톡과 차이>8 픽셀 0.43%(.NET 8/10 모두, 막대 경계 431픽셀). 로그에 D3D9Ex 미디어 디바이스 생성과
"mixer texture shared to the engine device (zero copy)"가 남고 디버그 레이어 메시지 0. 미검증: 다른 어댑터 디스플레이의 시스템 메모리
복사 경로, WARP 렌더 디바이스에서의 소프트웨어 버퍼, 인터레이스·DXVA 디코드 포맷 다양성.

## Phase 4 — 검증과 정리 ⏳

- ✅ 스모크 앱 확장·스톡 대비 자동 비교: `tests/run-smoke.ps1`(BUILD.md). 2026-10-28 결과: 표준·투명 창·미디어·디바이스 손실·
  두 모니터 걸침 모두 PASS(.NET 8/10, 의도된 셀 제외 차이>64 픽셀 0.00%).
- ✅ 성능 벤치마크 1차(`--bench`, RTX 4080 SUPER, 2560×1440 모니터 2개, 2026-10-28): 무거운 2D 장면에서 D3D11 엔진이
  스톡보다 빠르다. .NET 8: 23.5 vs 17.1 fps(+37%), .NET 10: 21.0 vs 16.2 fps(+30%); 프레임당 프로세스 CPU 시간은 같다(병목은
  UI 스레드의 요소 갱신). 프라이빗 메모리는 D3D11 쪽이 크다: 표준 장면 339 vs 145 MB, 두 모니터 걸침 520 vs 224 MB. 워킹셋 차이는
  34 MB 뿐이라 대부분 NVIDIA D3D11 사용자 모드 드라이버(`nvwgf2umx`, 81 MB 모듈)가 디바이스마다 예약하는 커밋이다. 디스플레이마다
  디바이스를 만드는 구조(D23)가 이를 두 배로 만들므로, 같은 LUID 디스플레이의 디바이스 공유가 메모리 쪽 첫 과제다.
  텍스트 전용·투명 창 벤치는 미측정.
  후속(2026-10-28): 프라이빗 메모리가 매초 수 MB 씩 늘다 390 MB 근처에서 멈추는 현상을 추적해 **프레임마다 텍스처를 만들고 버리는**
  스톡 패턴(효과 중간 렌더타깃, 글리프 업로드용 락 가능 텍스처 쌍)을 찾았다(L13). 텍스처 풀(D26)로 생성 횟수를 54% 줄여 같은 시점의
  프라이빗 메모리가 368→320 MB. 회전하는 요소의 중간 렌더타깃은 크기가 매 프레임 달라 풀로 못 줄인다. 벤치 fps 는 변화 없음.
  같은 LUID 디스플레이의 디바이스 공유(D28): 두 모니터에 걸친 창 프라이빗 536→390 MB, GPU 로컬 371→130 MB, 디바이스 2→1.
- ✅ 플립 모델 스왑체인(2026-10-28, D29): 어댑터가 같은 디스플레이들은 창마다 하드웨어 렌더타깃 하나(걸친 창 차이>8 0.04%, 프라이빗
  390→331 MB), 그 스왑체인은 `FLIP_SEQUENTIAL`·백버퍼 2개. 프레젠트 뒤 더티 영역을 버퍼 1→0 으로 복사해 부분 갱신을 유지하고 `Present1`
  에 더티 사각형을 넘긴다. 크기 변경은 `ResizeBuffers` 제자리(게이트 `resize` 행: 검은 픽셀 0.12%, 스톡과 같음).
  벤치(2026-10-28, 플립 모델): .NET 8 스톡 15.9 → D3D11 20.5 fps, .NET 10 15.0 → 22.5 fps(비트블트 때와 같은 범위, +29~50%).
  투명 창 벤치: 스톡 15.1/15.9 → D3D11 17.2/17.2 fps. 리드백 + `UpdateLayeredWindow` 가 상한이다(D30).
  후속(D31, 2026-10-28): 리드백을 창 크기 스테이징 하나로 더티 영역만 하고 플립 체인을 3버퍼로 바꿔 투명 창 전체 갱신 17.0 → 19.5 fps,
  부분 갱신 59.7 → 107.6 fps(스톡 98). 게이트 `bench ... (transparent, partial update)` 행.
- ✅ GPU 안티에일리어싱 옵트인(D32, 2026-10-29): 렌더 스레드 시간의 절반이던 CPU 커버리지 래스터라이저를 MSAA 로 대신한다.
  벤치 22.8 → 38.5 fps. 효과 중간 텍스처 크기 양자화와 풀 상한 조정(D33)으로 프레임당 텍스처 할당 93% 감소, 메모리 541 → 343 MB.
- ✅ 엔진 원인 업스트림 버그 수정(D34): dotnet/wpf#7421(NaN 클립), #8031(BitmapCache 다수). 게이트 `bug ...` 행.
- ❌ 투명 창의 DirectComposition 프레젠트는 하지 않는다(D30): 레이어드 창의 픽셀 단위 히트 테스트가 `UpdateLayeredWindow` 비트맵에 묶여
  있어 리드백을 없앨 수 없다. 비용은 게이트 `bench ... (transparent window)` 행으로 기록한다.
- 미디어를 `IMFMediaEngine` + `IMFDXGIDeviceManager`로 옮겨 D3D9 의존을 없앤다(WMP OCX·EVR 대체, 대규모, D25).
- ✅ dotnet/wpf-test 의 milcore DRT 실행(2026-10-28, `tests/run-drts.ps1`, 절차는 `docs/BUILD.md`, 함정은 L18). 결과(.NET 10, x64):

  | DRT | 스톡 | D3D11 | 비고 |
  |---|---|---|---|
  | DrtMil2D, DrtVisualTarget, DrtChangeable, DrtColorAPI, DrtTiming, DRTStoryboards, DrtMediaAPI, DrtMedia | 0 | 0 | 통과 |
  | DrtBasic3D | 0 | 0 | 처음엔 `RenderMode.SoftwareOnly` 케이스에서 `UCEERR_RENDERTHREADFAILURE`. 소프트웨어 3D 폴백이 렌더타깃을 직접 Lock 하던 것을 스테이징 텍스처 경유로 고쳤다(L19). |
  | DrtElement3D | 0 또는 -2 | 0 | 스톡에서 입력 관련 케이스(Reverse Inheritance, Eventing)가 간헐적으로 실패한다. 엔진과 무관. |
  | DrtAnimation | CLR 예외 | 같은 예외 | 두 엔진에서 같은 관리 예외(0xE0434352)로 끝난다. 엔진과 무관. |

  DRT 는 대부분 API·동작 검증이라 픽셀 비교는 하지 않는다. 소프트웨어 모드는 게이트에 `software mode` 행으로 추가했다
  (스톡 대비 차이>8 0.18%, 차이>64 0%; 차이는 3D 셀의 WARP 와 D3D9 소프트웨어 래스터라이저 사이 AA 뿐).
- ✅ x86 빌드·게이트(2026-10-28): `build.cmd -platform x86` 산출물이 .NET 8 x86 앱에서 게이트 전 행 PASS(표준 차이>8 0.04%, 투명 창·미디어·
  RenderTargetBitmap·프레임 큐(큐당 51 µs) 포함). 게이트는 `run-smoke.ps1 -Rid win-x86`. ✅ arm64 빌드(2026-10-28): `build.cmd -platform arm64` 가 경고·오류 0 으로 ARM64 DLL 을 만든다
  (D3DCompiler 재배포 프로젝트에 arm64 구성을 추가했다. SDK 의 arm64 d3dcompiler_47.dll 을 쓴다). arm64 실행 환경이 없어 동작은 미검증.
- D3D9 흔적 제거 확인(`d3d9.dll`은 미디어 재생(D25) 외에는 로드되지 않음. 이 PC에서는 보안 소프트웨어가 먼저 로드해 확인이 어렵다, L7).
- ✅ `RenderTargetBitmap`의 하드웨어 경로(2026-10-28, D27, 옵트인). `WpfGfxDx11_SetOption(1, 1)` /
  `Dx11Engine.HardwareRenderTargetBitmap = true` / `WPFGFX_DX11_HW_RTB=1`. 스모크 `--rtb --rtb-hw`: ps_3_0 효과가 적용된 비트맵이 나오고
  (스톡은 #7856 대로 통과 셰이더), 옵션을 끈 소프트웨어 경로는 스톡과 픽셀 일치. GPU 결과는 소프트웨어와 차이>8 10.5%(효과 셀 + AA).
  부수 효과: 즉시 컨텍스트의 런타임 멀티스레드 보호를 켰다(UI 스레드 리드백).
- ✅ 사용자 D3D11 렌더 루프용 프레임 큐(D19, 2026-10-28): `docs/FRAME_QUEUE.md`. 스모크 `--fq`(큐 8개, UI 스레드 0.5초마다 100 ms 블로킹):
  생산자 비용 큐당 39 µs, 뮤텍스 실패·타임아웃 0, 표시/Present 1893/1904, 250 ms 간격 캡처 변화 80.6%. 큐 32개도 60 fps 유지(큐당 30 µs,
  틱당 약 1 ms). 스톡 엔진에서는 능력 질의 false 로 폴백. `WpfGfx.Dx11.Interop`(WPF 무의존)와 `D3D11Image.SetFrameQueue`.
