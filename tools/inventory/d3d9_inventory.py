#!/usr/bin/env python3
"""WpfGfx 소스에서 Direct3D 9 사용처를 추출해 docs/inventory/ 아래에 마크다운으로 쓴다.

사용법:  python tools/inventory/d3d9_inventory.py [저장소 루트]

산출물:
  docs/inventory/summary.md     파일·디렉터리별 D3D9 밀도, 인터페이스·메서드·열거형 집계
  docs/inventory/call-sites.md  인터페이스 메서드 호출과 열거형 사용의 파일:줄 목록

정확한 C++ 파싱이 아니라 정규식 기반이므로 근사치다. 메서드 호출은 `->Name(` 형태만 세고,
Lock/Unlock/GetDesc처럼 다른 타입에도 있는 이름은 IDirect3D 타입을 참조하는 파일에서만 센다.
fxjit의 사설 d3d9 헤더 복사본(core/fxjit/PixelShader/d3d9types.h, d3d9caps.h)은 정의일 뿐이므로
사용 집계에서 뺀다.
"""
import collections
import os
import re
import sys

ROOT = os.path.abspath(sys.argv[1]) if len(sys.argv) > 1 else os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
GFX = os.path.join(ROOT, 'src', 'Microsoft.DotNet.Wpf', 'src', 'WpfGfx')
OUT = os.path.join(ROOT, 'docs', 'inventory')
EXTS = ('.cpp', '.h', '.hpp', '.inl', '.hxx', '.cxx', '.inc')
SKIP_DIRS = ('codegen', 'exts', 'tools', 'DbgXHelper')
DEF_HEADERS = ('core/fxjit/PixelShader/d3d9types.h', 'core/fxjit/PixelShader/d3d9caps.h')

# d3d9.h 인터페이스별 메서드. 이름이 겹치는 것은 집계에서 하나로 합친다.
METHODS = {
    'IDirect3D9': 'RegisterSoftwareDevice GetAdapterCount GetAdapterIdentifier GetAdapterModeCount EnumAdapterModes GetAdapterDisplayMode CheckDeviceType CheckDeviceFormat CheckDeviceMultiSampleType CheckDepthStencilMatch CheckDeviceFormatConversion GetDeviceCaps GetAdapterMonitor CreateDevice',
    'IDirect3D9Ex': 'GetAdapterModeCountEx EnumAdapterModesEx GetAdapterDisplayModeEx CreateDeviceEx GetAdapterLUID',
    'IDirect3DDevice9': 'TestCooperativeLevel GetAvailableTextureMem EvictManagedResources GetDirect3D GetDeviceCaps GetDisplayMode GetCreationParameters SetCursorProperties SetCursorPosition ShowCursor CreateAdditionalSwapChain GetSwapChain GetNumberOfSwapChains Reset Present GetBackBuffer GetRasterStatus SetDialogBoxMode SetGammaRamp GetGammaRamp CreateTexture CreateVolumeTexture CreateCubeTexture CreateVertexBuffer CreateIndexBuffer CreateRenderTarget CreateDepthStencilSurface UpdateSurface UpdateTexture GetRenderTargetData GetFrontBufferData StretchRect ColorFill CreateOffscreenPlainSurface SetRenderTarget GetRenderTarget SetDepthStencilSurface GetDepthStencilSurface BeginScene EndScene Clear SetTransform GetTransform MultiplyTransform SetViewport GetViewport SetMaterial GetMaterial SetLight GetLight LightEnable GetLightEnable SetClipPlane GetClipPlane SetRenderState GetRenderState CreateStateBlock BeginStateBlock EndStateBlock SetClipStatus GetClipStatus GetTexture SetTexture GetTextureStageState SetTextureStageState GetSamplerState SetSamplerState ValidateDevice SetPaletteEntries GetPaletteEntries SetCurrentTexturePalette GetCurrentTexturePalette SetScissorRect GetScissorRect SetSoftwareVertexProcessing GetSoftwareVertexProcessing SetNPatchMode GetNPatchMode DrawPrimitive DrawIndexedPrimitive DrawPrimitiveUP DrawIndexedPrimitiveUP ProcessVertices CreateVertexDeclaration SetVertexDeclaration GetVertexDeclaration SetFVF GetFVF CreateVertexShader SetVertexShader GetVertexShader SetVertexShaderConstantF GetVertexShaderConstantF SetVertexShaderConstantI GetVertexShaderConstantI SetVertexShaderConstantB GetVertexShaderConstantB SetStreamSource GetStreamSource SetStreamSourceFreq GetStreamSourceFreq SetIndices GetIndices CreatePixelShader SetPixelShader GetPixelShader SetPixelShaderConstantF GetPixelShaderConstantF SetPixelShaderConstantI GetPixelShaderConstantI SetPixelShaderConstantB GetPixelShaderConstantB DrawRectPatch DrawTriPatch DeletePatch CreateQuery',
    'IDirect3DDevice9Ex': 'SetConvolutionMonoKernel ComposeRects PresentEx GetGPUThreadPriority SetGPUThreadPriority WaitForVBlank CheckResourceResidency SetMaximumFrameLatency GetMaximumFrameLatency CheckDeviceState CreateRenderTargetEx CreateOffscreenPlainSurfaceEx CreateDepthStencilSurfaceEx ResetEx GetDisplayModeEx',
    'IDirect3DSwapChain9': 'Present GetFrontBufferData GetBackBuffer GetRasterStatus GetDisplayMode GetDevice GetPresentParameters',
    'IDirect3DSwapChain9Ex': 'GetLastPresentCount GetPresentStats GetDisplayModeEx',
    'IDirect3DResource9': 'GetDevice SetPrivateData GetPrivateData FreePrivateData SetPriority GetPriority PreLoad GetType',
    'IDirect3DBaseTexture9': 'SetLOD GetLOD GetLevelCount SetAutoGenFilterType GetAutoGenFilterType GenerateMipSubLevels',
    'IDirect3DTexture9': 'GetLevelDesc GetSurfaceLevel LockRect UnlockRect AddDirtyRect',
    'IDirect3DSurface9': 'GetContainer GetDesc LockRect UnlockRect GetDC ReleaseDC',
    'IDirect3DVertexBuffer9': 'Lock Unlock GetDesc',
    'IDirect3DIndexBuffer9': 'Lock Unlock GetDesc',
    'IDirect3DQuery9': 'GetType GetDataSize Issue GetData',
    'IDirect3DStateBlock9': 'Capture Apply',
    'IDirect3DVertexDeclaration9': 'GetDeclaration',
    'IDirect3DPixelShader9': 'GetFunction',
    'IDirect3DVertexShader9': 'GetFunction',
    'IDirect3DDeviceManager9': 'ResetDevice OpenDeviceHandle CloseDeviceHandle TestDevice LockDevice UnlockDevice GetVideoService',
}
AMBIGUOUS = {'Lock', 'Unlock', 'GetDesc', 'GetType', 'GetDevice', 'Present', 'Clear', 'Reset', 'Apply', 'Capture', 'Issue', 'GetData', 'GetContainer', 'GetDC', 'ReleaseDC', 'GetFunction', 'GetDeclaration', 'GetBackBuffer', 'GetDisplayMode', 'PreLoad', 'GetPriority', 'SetPriority'}
METHOD_OWNER = {}
for iface, names in METHODS.items():
    for n in names.split():
        METHOD_OWNER.setdefault(n, []).append(iface)
METHOD_RE = re.compile(r'->\s*(' + '|'.join(sorted(METHOD_OWNER, key=len, reverse=True)) + r')\s*\(')

FAMILIES = ['D3DRS_', 'D3DFMT_', 'D3DTSS_', 'D3DTOP_', 'D3DTA_', 'D3DSAMP_', 'D3DTEXF_', 'D3DTADDRESS_', 'D3DUSAGE_', 'D3DPOOL_', 'D3DFVF_',
            'D3DDECLUSAGE_', 'D3DDECLTYPE_', 'D3DDECLMETHOD_', 'D3DPT_', 'D3DBLEND_', 'D3DBLENDOP_', 'D3DCMP_', 'D3DSTENCILOP_', 'D3DCULL_',
            'D3DFILL_', 'D3DZB_', 'D3DMULTISAMPLE_', 'D3DSWAPEFFECT_', 'D3DPRESENT_', 'D3DPRESENTFLAG_', 'D3DCREATE_', 'D3DDEVTYPE_',
            'D3DLOCK_', 'D3DCLEAR_', 'D3DERR_', 'D3DRTYPE_', 'D3DQUERYTYPE_', 'D3DISSUE_', 'D3DTTFF_', 'D3DBACKBUFFER_', 'D3DSBT_',
            'D3DCAPS2_', 'D3DCAPS3_', 'D3DDEVCAPS_', 'D3DDEVCAPS2_', 'D3DPMISCCAPS_', 'D3DPRASTERCAPS_', 'D3DPTEXTURECAPS_',
            'D3DPTFILTERCAPS_', 'D3DPTADDRESSCAPS_', 'D3DPBLENDCAPS_', 'D3DPCMPCAPS_', 'D3DSTENCILCAPS_', 'D3DTEXOPCAPS_', 'D3DVTXPCAPS_',
            'D3DDTCAPS_', 'D3DPS20CAPS_', 'D3DLINECAPS_', 'D3DCURSORCAPS_',
            'D3DSIO_', 'D3DSPR_', 'D3DSPSM_', 'D3DSPDM_', 'D3DPS_', 'D3DVS_', 'D3DSHADER_', 'D3DSTT_', 'D3DSP_', 'D3DSI_']
FAMILY_RE = re.compile(r'\b(' + '|'.join(FAMILIES) + r')([A-Z0-9_]+)\b')
IFACE_RE = re.compile(r'\bIDirect3D[A-Za-z0-9]+\b')
STRUCT_RE = re.compile(r'\b(D3DPRESENT_PARAMETERS|D3DCAPS9|D3DSURFACE_DESC|D3DVERTEXELEMENT9|D3DLOCKED_RECT|D3DVIEWPORT9|D3DMATRIX|D3DCOLOR|D3DDISPLAYMODE|D3DDISPLAYMODEEX|D3DADAPTER_IDENTIFIER9|D3DRASTER_STATUS|D3DDEVICE_CREATION_PARAMETERS|D3DVERTEXBUFFER_DESC|D3DINDEXBUFFER_DESC|D3DMATERIAL9|D3DLIGHT9|D3DRECT|D3DBOX|D3DLOCKED_BOX|D3DVOLUME_DESC|D3DVECTOR|D3DCOLORVALUE|D3DXMATRIX|D3DXVECTOR[234]|D3DXCOLOR|D3DXFLOAT16|D3DXMACRO)\b')
INCLUDE_RE = re.compile(r'^\s*#\s*include\s*[<"]\s*(d3d9[a-z]*\.h|d3dx9[a-z]*\.h|dxva2api\.h|evr9?\.h|d3d9types\.h|d3d9caps\.h)\s*[>"]', re.M)


def zone_of(rel):
    parts = rel.split('/')
    if parts[0] in ('core', 'common', 'shared') and len(parts) > 2:
        return parts[0] + '/' + parts[1]
    return parts[0]


def scan():
    files = []
    for dp, dn, fn in os.walk(GFX):
        rel = os.path.relpath(dp, GFX).replace('\\', '/')
        if rel.split('/')[0] in SKIP_DIRS:
            continue
        for f in fn:
            if f.lower().endswith(EXTS):
                files.append(os.path.join(dp, f))
    records = []
    for path in sorted(files):
        rel = os.path.relpath(path, GFX).replace('\\', '/')
        try:
            text = open(path, encoding='utf-8', errors='ignore').read()
        except OSError:
            continue
        lines = text.split('\n')
        rec = {'rel': rel, 'zone': zone_of(rel), 'is_def': rel in DEF_HEADERS, 'lines': len(lines),
               'ifaces': collections.Counter(), 'methods': collections.Counter(), 'enums': collections.defaultdict(collections.Counter),
               'structs': collections.Counter(), 'includes': set(), 'sites': []}
        rec['includes'] = set(m.group(1) for m in INCLUDE_RE.finditer(text))
        has_iface = False
        for i, line in enumerate(lines, 1):
            code = line.split('//')[0]
            for m in IFACE_RE.finditer(code):
                rec['ifaces'][m.group(0)] += 1
                has_iface = True
            for m in FAMILY_RE.finditer(code):
                fam, val = m.group(1), m.group(1) + m.group(2)
                if val.endswith('FORCE_DWORD'):
                    continue
                rec['enums'][fam][val] += 1
                rec['sites'].append((i, 'enum', val, code.strip()))
            for m in STRUCT_RE.finditer(code):
                rec['structs'][m.group(1)] += 1
        for i, line in enumerate(lines, 1):
            code = line.split('//')[0]
            for m in METHOD_RE.finditer(code):
                name = m.group(1)
                if name in AMBIGUOUS and not (has_iface or rec['includes']):
                    continue
                rec['methods'][name] += 1
                rec['sites'].append((i, 'call', name, code.strip()))
        rec['sites'].sort()
        records.append(rec)
    return records


def total(rec):
    return sum(rec['ifaces'].values()) + sum(rec['methods'].values()) + sum(sum(c.values()) for c in rec['enums'].values()) + sum(rec['structs'].values())


def write_summary(records):
    used = [r for r in records if not r['is_def'] and total(r) > 0]
    defs = [r for r in records if r['is_def']]
    out = []
    out.append('# D3D9 사용 집계 (자동 생성)\n')
    out.append('`tools/inventory/d3d9_inventory.py`가 만든다. 손으로 고치지 말 것. 해석과 D3D11 매핑은 `docs/INVENTORY.md`.\n')
    out.append(f'대상: WpfGfx 아래 소스 {len(records)}개 파일. D3D9를 참조하는 파일 {len(used)}개. '
               f'사설 헤더 복사본 {len(defs)}개({", ".join(r["rel"] for r in defs)})는 정의이므로 제외.\n')

    out.append('\n## 디렉터리별 밀도\n')
    out.append('| 디렉터리 | 파일 수 | 줄 수 | 인터페이스 참조 | 메서드 호출 | 열거형 사용 | 구조체 |')
    out.append('|---|---:|---:|---:|---:|---:|---:|')
    zones = collections.defaultdict(lambda: [0, 0, 0, 0, 0, 0])
    for r in used:
        z = zones[r['zone']]
        z[0] += 1
        z[1] += r['lines']
        z[2] += sum(r['ifaces'].values())
        z[3] += sum(r['methods'].values())
        z[4] += sum(sum(c.values()) for c in r['enums'].values())
        z[5] += sum(r['structs'].values())
    for zone, z in sorted(zones.items(), key=lambda kv: -(kv[1][2] + kv[1][3] + kv[1][4])):
        out.append(f'| `{zone}` | {z[0]} | {z[1]} | {z[2]} | {z[3]} | {z[4]} | {z[5]} |')

    out.append('\n## 파일별 밀도 (참조 합계 내림차순)\n')
    out.append('| 파일 | 줄 수 | 인터페이스 | 메서드 | 열거형 | 구조체 | 주요 인터페이스 |')
    out.append('|---|---:|---:|---:|---:|---:|---|')
    for r in sorted(used, key=lambda r: -total(r)):
        top = ', '.join(f'{k}({v})' for k, v in r['ifaces'].most_common(3))
        out.append(f"| `{r['rel']}` | {r['lines']} | {sum(r['ifaces'].values())} | {sum(r['methods'].values())} | "
                   f"{sum(sum(c.values()) for c in r['enums'].values())} | {sum(r['structs'].values())} | {top} |")

    out.append('\n## 인터페이스별 참조 횟수와 파일\n')
    out.append('| 인터페이스 | 참조 | 파일 수 | 파일 (상위 6) |')
    out.append('|---|---:|---:|---|')
    agg = collections.Counter()
    per_file = collections.defaultdict(collections.Counter)
    for r in used:
        for k, v in r['ifaces'].items():
            agg[k] += v
            per_file[k][r['rel']] += v
    for k, v in agg.most_common():
        files = ', '.join(f'`{os.path.basename(f)}`({c})' for f, c in per_file[k].most_common(6))
        out.append(f'| {k} | {v} | {len(per_file[k])} | {files} |')

    out.append('\n## 메서드 호출 집계 (`->Name(` 기준, 근사치)\n')
    out.append('| 메서드 | 호출 | 소유 인터페이스 | 파일 (상위 5) |')
    out.append('|---|---:|---|---|')
    magg = collections.Counter()
    mfile = collections.defaultdict(collections.Counter)
    for r in used:
        for k, v in r['methods'].items():
            magg[k] += v
            mfile[k][r['rel']] += v
    for k, v in magg.most_common():
        files = ', '.join(f'`{os.path.basename(f)}`({c})' for f, c in mfile[k].most_common(5))
        out.append(f'| {k} | {v} | {", ".join(METHOD_OWNER[k])} | {files} |')

    out.append('\n## 열거형 가족별 사용 값 (정의 헤더 제외)\n')
    fagg = collections.defaultdict(collections.Counter)
    ffiles = collections.defaultdict(set)
    for r in used:
        for fam, c in r['enums'].items():
            fagg[fam].update(c)
            ffiles[fam].add(r['rel'])
    for fam in sorted(fagg, key=lambda f: -sum(fagg[f].values())):
        vals = fagg[fam]
        out.append(f'\n### {fam} — {len(vals)}종, {sum(vals.values())}회, 파일 {len(ffiles[fam])}개\n')
        out.append(', '.join(f'`{v}`({c})' for v, c in sorted(vals.items(), key=lambda kv: (-kv[1], kv[0]))))

    out.append('\n## 구조체·타입 참조\n')
    sagg = collections.Counter()
    for r in used:
        sagg.update(r['structs'])
    out.append(', '.join(f'`{k}`({v})' for k, v in sagg.most_common()))

    out.append('\n## d3d9 계열 헤더를 include하는 파일\n')
    for r in sorted(records, key=lambda r: r['rel']):
        if r['includes']:
            out.append(f"- `{r['rel']}`: {', '.join(sorted(r['includes']))}")
    out.append('')
    return '\n'.join(out)


def write_sites(records):
    out = ['# D3D9 호출 지점 (자동 생성)\n',
           '`tools/inventory/d3d9_inventory.py`가 만든다. 형식: `줄: 종류 이름 | 코드`. 정의 헤더 복사본은 제외.\n']
    for r in sorted(records, key=lambda r: r['rel']):
        if r['is_def'] or not r['sites']:
            continue
        out.append(f"\n## {r['rel']}\n")
        for line, kind, name, code in r['sites']:
            code = code.replace('|', '\\|')
            if len(code) > 140:
                code = code[:137] + '...'
            out.append(f'- {line}: {kind} `{name}` | `{code}`')
    out.append('')
    return '\n'.join(out)


def main():
    records = scan()
    os.makedirs(OUT, exist_ok=True)
    with open(os.path.join(OUT, 'summary.md'), 'w', encoding='utf-8', newline='\n') as f:
        f.write(write_summary(records))
    with open(os.path.join(OUT, 'call-sites.md'), 'w', encoding='utf-8', newline='\n') as f:
        f.write(write_sites(records))
    used = sum(1 for r in records if not r['is_def'] and total(r) > 0)
    print(f'files scanned={len(records)} with D3D9 refs={used} -> {OUT}')


if __name__ == '__main__':
    main()
