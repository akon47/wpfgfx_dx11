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
- 미디어: D16, DESIGN §10 ✅
- 작업 방식: D12(브랜치), D13(첫 마일스톤) ✅

## Phase 3 — 마일스톤 구현 🔄

`dx11-port` 브랜치에서 작업하고 마일스톤마다 `main`에 병합한다(D12). 마일스톤 표와 합격 기준은 `docs/DESIGN.md` §12.

| M | 내용 | 상태 |
|---|---|---|
| M1 | D3D9 없이 컴파일, 소프트웨어 렌더링(Tier 0) | ⏳ |
| M2 | 디바이스·스왑체인·클리어·프레젠트 | ⏳ |
| M3 | 셰이더 파이프라인: 단색·그라데이션·경로 | ⏳ |
| M4 | 텍스처·비트맵·타일 브러시 | ⏳ |
| M5 | 텍스트 | ⏳ |
| M6 | 중간 렌더타깃·효과·클립 | ⏳ |
| M7 | 3D | ⏳ |
| M8 | ShaderEffect 변환기 | ⏳ |
| M9 | D3DImage, 새 export, WpfGfx.Dx11 | ⏳ |
| M10 | 투명 창, 멀티 어댑터, 디바이스 제거 복구 | ⏳ |
| M11 | 미디어 | ⏳ |

## Phase 4 — 검증과 정리 ⏳

- 스모크 앱 확장(각 단계 전용 케이스), 스톡 대비 스크린샷 자동 비교.
- dotnet/wpf-test의 DRT 중 렌더링 관련 테스트를 교체 DLL로 실행.
- x86·arm64 빌드.
- D3D9 흔적 제거 확인(프로세스에 `d3d9.dll`이 D3DImage interop 외에는 로드되지 않음).
