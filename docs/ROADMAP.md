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

## Phase 2 — 설계 ⏳

목표: 인벤토리를 바탕으로 치환 설계를 `docs/DECISIONS.md`에 확정한다.

결정해야 할 것:
- 디바이스·컨텍스트 모델(스레딩, 멀티 어댑터 `CD3DDeviceManager` 대응), 최소 Feature Level.
- 스왑체인과 프레젠테이션(HWND별 DXGI 플립 모델, 레이어드 윈도우의 시스템 메모리 경로).
- 렌더스테이트 → 상태 객체 캐시, FVF/버텍스 선언 → 입력 레이아웃, 포맷 매핑.
- 셰이더: 런타임 HLSL 조립기의 SM4 출력, 텍스트(ClearType) 셰이더, 3D 조명 셰이더, 고정 기능 제거.
- ShaderEffect 호환(D6): DX9 바이트코드 → SM4 변환.
- D3DImage 호환(D7): D3D9Ex 공유 서피스 → D3D11.
- MediaElement(`core/av`) 처리 순서.

## Phase 3 — 수직 슬라이스 구현 ⏳

작은 단위로 빌드 → 스모크 → 스톡 출력과 비교를 반복한다. 순서:

1. 단색 사각형 하나가 DXGI 스왑체인으로 나온다.
2. 브러시(단색·선형·방사형 그라데이션), 지오메트리 래스터화.
3. 비트맵·타일 브러시·텍스처 업로드.
4. 텍스트(글리프 캐시, ClearType/그레이스케일 셰이더).
5. 중간 렌더타깃: Opacity, OpacityMask, VisualBrush, Effects(Blur/DropShadow), 클립.
6. 3D(Viewport3D).
7. ShaderEffect(D6), D3DImage(D7).
8. 레이어드 윈도우(AllowsTransparency), 멀티 모니터·어댑터.
9. MediaElement(`core/av`).

각 단계 합격 기준: 스모크 앱의 해당 셀이 스톡 DX9 출력과 눈으로 구분되지 않는다(의도한 차이 제외).

## Phase 4 — 검증과 정리 ⏳

- 스모크 앱 확장(각 단계 전용 케이스), 스톡 대비 스크린샷 자동 비교.
- dotnet/wpf-test의 DRT 중 렌더링 관련 테스트를 교체 DLL로 실행.
- x86·arm64 빌드.
- D3D9 흔적 제거 확인(프로세스에 `d3d9.dll`이 D3DImage interop 외에는 로드되지 않음).
