<#
.SYNOPSIS
  Drop-in gate for wpfgfx_cor3.dll: runs tests/SmokeApp on the stock DX9 engine and on the given
  D3D11 engine for each target framework, captures every scenario, compares them per cell and
  prints a table (also written to tests/out/report.md). Exit code 1 when any scenario fails.

.EXAMPLE
  .\tests\run-smoke.ps1                              # artifacts build, .NET 8 and 10
  .\tests\run-smoke.ps1 -Dll C:\somewhere -Tfm net8.0-windows -Bench -StraddleLeft 2110

.NOTES
  Expected differences are excluded: cell 4 (rotating animation phase) and cell 16 (the D3D11
  texture cell shows a notice on the stock engine). Device loss compares the D3D11 engine against
  its own normal run (WPFGFX_DX11_TEST_DEVICE_LOSS hook, D24).
#>
param(
    [string]$Dll = "",
    [string[]]$Tfm = @("net8.0-windows", "net10.0-windows"),
    [switch]$SkipBuild,
    [switch]$Bench,
    [int]$BenchSeconds = 8,
    [int]$StraddleLeft = -1,
    [string]$Rid = "win-x64"          # win-x86 runs the 32-bit app against a 32-bit engine (-Dll artifacts\bin\wpfgfx\x86\Release)
)

$ErrorActionPreference = "Stop"
$tests = $PSScriptRoot
$repo = Split-Path $tests -Parent
$app = Join-Path $tests "SmokeApp"
$out = Join-Path $tests "out"
$dllDir = Join-Path $out "dll"
New-Item -ItemType Directory -Force $out, $dllDir | Out-Null

$arch = if ($Rid -eq "win-x86") { "x86" } else { "x64" }
if ($Dll -eq "") { $Dll = Join-Path $repo "artifacts\bin\wpfgfx\$arch\Release" }
Copy-Item (Join-Path $Dll "wpfgfx_cor3.dll") $dllDir -Force
$compiler = Join-Path $Dll "D3DCompiler_47_cor3.dll"
if (-not (Test-Path $compiler)) { $compiler = Join-Path $repo "artifacts\bin\D3DCompiler\$arch\Release\D3DCompiler_47_cor3.dll" }
Copy-Item $compiler $dllDir -Force

Add-Type -AssemblyName System.Drawing

function Compare-Capture([string]$a, [string]$b, [int[]]$excludeCells) {
    $ia = [System.Drawing.Bitmap]::FromFile($a)
    $ib = [System.Drawing.Bitmap]::FromFile($b)
    try {
        $w = [Math]::Min($ia.Width, $ib.Width); $h = [Math]::Min($ia.Height, $ib.Height)
        $n8 = 0; $n64 = 0; $counted = 0
        for ($y = 0; $y -lt $h; $y++) {
            for ($x = 0; $x -lt $w; $x++) {
                if ($excludeCells) {
                    $cell = [int][Math]::Floor($y * 4 / $h) * 4 + [int][Math]::Floor($x * 4 / $w) + 1
                    if ($excludeCells -contains $cell) { continue }
                }
                $counted++
                $ca = $ia.GetPixel($x, $y); $cb = $ib.GetPixel($x, $y)
                $d = [Math]::Max([Math]::Abs($ca.R - $cb.R), [Math]::Max([Math]::Abs($ca.G - $cb.G), [Math]::Abs($ca.B - $cb.B)))
                if ($d -gt 8) { $n8++ }
                if ($d -gt 64) { $n64++ }
            }
        }
        if ($counted -eq 0) { $counted = 1 }
        return @{ Diff8 = $n8 / $counted; Diff64 = $n64 / $counted; Size = "$($ia.Width)x$($ia.Height)" }
    } finally { $ia.Dispose(); $ib.Dispose() }
}

function Run-Smoke([string]$tfm, [bool]$hw, [string[]]$smokeArgs) {
    $exe = Join-Path $app "bin\Release\$tfm\$Rid\SmokeApp.exe"
    $all = @()
    if ($hw) { $all += @("--wpfgfx", $dllDir) }
    $all += $smokeArgs
    $all += "--exit"
    $text = (& $exe @all 2>&1 | Out-String)
    return $text
}

$rows = @()
$failed = $false
function Add-Row([string]$scenario, [string]$tfm, [string]$detail, [bool]$pass) {
    $script:rows += ("| {0} | {1} | {2} | {3} |" -f $scenario, $tfm, $detail, $(if ($pass) { "PASS" } else { "FAIL" }))
    if (-not $pass) { $script:failed = $true }
}

foreach ($t in $Tfm) {
    if (-not $SkipBuild) {
        Push-Location $app
        try { & dotnet build -c Release -p:SmokeTfm=$t -p:SmokeRid=$Rid -nologo -v q | Out-Null } finally { Pop-Location }
    }
    $short = $t -replace "\.0-windows", ""

    # ---- standard scene: engine identity + cells
    $stock = Run-Smoke $t $false @("--screenshot", "$out\$short-stock.png")
    $hwText = Run-Smoke $t $true @("--screenshot", "$out\$short-hw.png")
    $tierOk = ($hwText -match "Tier=2") -and ($stock -match "Tier=2")
    $dllLine = ($hwText -split "`n" | Where-Object { $_ -match "wpfgfx_cor3.dll" } | Select-Object -First 1)
    $dllOk = ($dllLine -match "42,42,42,42424") -and ($dllLine -match [regex]::Escape($dllDir))
    Add-Row "engine" $short ("Tier 2 both: {0}; loaded {1}" -f $tierOk, $(if ($dllOk) { "repo DLL 42.42.42.42424" } else { "UNEXPECTED: $dllLine".Trim() })) ($tierOk -and $dllOk)
    $c = Compare-Capture "$out\$short-stock.png" "$out\$short-hw.png" @(4, 16)
    Add-Row "standard (cells 1-15 except 4)" $short ("diff>8 {0:P2}, diff>64 {1:P2}" -f $c.Diff8, $c.Diff64) (($c.Diff64 -lt 0.005) -and ($c.Diff8 -lt 0.015))

    # ---- spin jitter: the rotating cell's body centroid must stay put frame to frame (regression guard for D26)
    $js = Run-Smoke $t $false @("--spin-jitter", "24")
    $jh = Run-Smoke $t $true @("--spin-jitter", "24")
    $devS = 99.0; $devH = 99.0
    if ($js -match "bodyCentroidMaxDev=([0-9.]+)px") { $devS = [double]$matches[1] }
    if ($jh -match "bodyCentroidMaxDev=([0-9.]+)px") { $devH = [double]$matches[1] }
    Add-Row "spin jitter (body centroid max dev)" $short ("stock {0:F2}px, D3D11 {1:F2}px" -f $devS, $devH) (($devH -lt 1.5) -and ($devS -lt 1.5))

    # ---- resize: the swap chain is resized in place; a recreated flip chain would flash the unpainted redirection surface (D29)
    $rs = Run-Smoke $t $false @("--resize", "16")
    $rh = Run-Smoke $t $true @("--resize", "16")
    $bs = 99.0; $bh = 99.0
    if ($rs -match "maxBlack=([0-9.]+)%") { $bs = [double]$matches[1] }
    if ($rh -match "maxBlack=([0-9.]+)%") { $bh = [double]$matches[1] }
    Add-Row "resize (16 steps, black pixels)" $short ("stock max {0:F2}%, D3D11 max {1:F2}%" -f $bs, $bh) (($bh -le ($bs + 0.5)) -and ($bh -lt 5))

    # ---- software rendering mode: RenderMode.SoftwareOnly, 3D drawn through the software 3D fallback (DrtBasic3D regression guard)
    [void](Run-Smoke $t $false @("--software", "--screenshot", "$out\$short-stock-sw.png"))
    $swText = Run-Smoke $t $true @("--software", "--screenshot", "$out\$short-hw-sw.png")
    $c = Compare-Capture "$out\$short-stock-sw.png" "$out\$short-hw-sw.png" @(4, 16)
    Add-Row "software mode (ProcessRenderMode)" $short ("diff>8 {0:P2}, diff>64 {1:P2}" -f $c.Diff8, $c.Diff64) (($swText -match "Tier=") -and ($c.Diff64 -lt 0.01))

    # ---- transparent (layered) window
    [void](Run-Smoke $t $false @("--screenshot", "$out\$short-stock-t.png", "--transparent", "$out\$short-stock-transparent.png"))
    [void](Run-Smoke $t $true @("--screenshot", "$out\$short-hw-t.png", "--transparent", "$out\$short-hw-transparent.png"))
    $c = Compare-Capture "$out\$short-stock-transparent.png" "$out\$short-hw-transparent.png" $null
    Add-Row "transparent window" $short ("diff>8 {0:P2}, diff>64 {1:P2}" -f $c.Diff8, $c.Diff64) ($c.Diff8 -lt 0.001)

    # ---- media
    $media = Join-Path $tests "assets\smptebars.mp4"
    $ms = Run-Smoke $t $false @("--media", $media, "--media-shot", "$out\$short-stock-media.png", "--delay", "3500")
    $mh = Run-Smoke $t $true @("--media", $media, "--media-shot", "$out\$short-hw-media.png", "--delay", "3500")
    $opened = ($ms -match "media status: opened") -and ($mh -match "media status: opened")
    $c = Compare-Capture "$out\$short-stock-media.png" "$out\$short-hw-media.png" $null
    Add-Row "media (MediaElement)" $short ("opened both: {0}; diff>8 {1:P2}, diff>64 {2:P2}" -f $opened, $c.Diff8, $c.Diff64) ($opened -and ($c.Diff64 -lt 0.01))

    # ---- device loss (D3D11 engine only, compared with its own normal run)
    $env:WPFGFX_DX11_TEST_DEVICE_LOSS = "5"
    try { $dl = Run-Smoke $t $true @("--screenshot", "$out\$short-hw-devloss.png", "--delay", "4000") }
    finally { Remove-Item Env:\WPFGFX_DX11_TEST_DEVICE_LOSS -ErrorAction SilentlyContinue }
    $recovered = ($dl -match "front buffer available=True") -and ($dl -match "Tier=2")
    $c = Compare-Capture "$out\$short-hw.png" "$out\$short-hw-devloss.png" @(4)
    Add-Row "device loss recovery" $short ("front buffer back: {0}; diff>8 vs normal {1:P2}" -f $recovered, $c.Diff8) ($recovered -and ($c.Diff8 -lt 0.005))

    # ---- RenderTargetBitmap: software path must match; the opt-in GPU path is reported
    $rs = Run-Smoke $t $false @("--rtb", "$out\$short-stock-rtb.png")
    $rw = Run-Smoke $t $true @("--rtb", "$out\$short-hw-rtb.png")
    $rh = Run-Smoke $t $true @("--rtb", "$out\$short-hw-rtb-gpu.png", "--rtb-hw")
    $swOk = ($rs -match "rtb: 320x240") -and ($rw -match "rtb: 320x240")
    $c = Compare-Capture "$out\$short-stock-rtb.png" "$out\$short-hw-rtb.png" $null
    Add-Row "RenderTargetBitmap (software)" $short ("rendered both: {0}; diff>8 {1:P2}, diff>64 {2:P2}" -f $swOk, $c.Diff8, $c.Diff64) ($swOk -and ($c.Diff8 -lt 0.001))
    $gpuOk = ($rh -match "rtb: 320x240 hardware=True")
    if ($gpuOk) {
        $c = Compare-Capture "$out\$short-stock-rtb.png" "$out\$short-hw-rtb-gpu.png" $null
        Add-Row "RenderTargetBitmap (GPU opt-in)" $short ("rendered; vs software diff>8 {0:P2}, diff>64 {1:P2} (ps_3_0 effect only on GPU)" -f $c.Diff8, $c.Diff64) $true
    } else {
        Add-Row "RenderTargetBitmap (GPU opt-in)" $short ("not rendered: " + (($rh -split "`n" | Where-Object { $_ -match "^rtb:" } | Select-Object -First 1)).Trim()) $false
    }

    # ---- frame queues (D19): producer thread + blocked UI thread; stock engine must report the fallback
    $fs = Run-Smoke $t $false @("--fq", "$out\$short-stock-fq.png", "--fq-queues", "8", "--fq-seconds", "4")
    $fh = Run-Smoke $t $true @("--fq", "$out\$short-hw-fq.png", "--fq-queues", "8", "--fq-seconds", "4", "--fq-block")
    $stockLine = (($fs -split "`n" | Where-Object { $_ -match "^fq:" } | Select-Object -First 1)).Trim()
    $hwLine = (($fh -split "`n" | Where-Object { $_ -match "^fq:" } | Select-Object -First 1)).Trim()
    $stockOk = $stockLine -match "not available"
    $changed = 0.0; $us = 999.0
    if ($hwLine -match "changedBetweenCaptures=([0-9.]+)") { $changed = [double]$matches[1] }
    if ($hwLine -match "producerUsPerQueueFrame=([0-9.]+)") { $us = [double]$matches[1] }
    $hwOk = ($hwLine -match "displayed=([1-9][0-9]*)") -and ($changed -gt 30) -and ($us -lt 200)
    Add-Row "frame queues (8, UI blocked)" $short ($hwLine -replace "^fq: ", "") $hwOk
    Add-Row "frame queues on stock" $short ($stockLine -replace "^fq: ", "") $stockOk

    # ---- optional: window straddling two monitors
    if ($StraddleLeft -ge 0) {
        [void](Run-Smoke $t $false @("--left", "$StraddleLeft", "--top", "40", "--screenshot", "$out\$short-stock-straddle.png"))
        [void](Run-Smoke $t $true @("--left", "$StraddleLeft", "--top", "40", "--screenshot", "$out\$short-hw-straddle.png"))
        $c = Compare-Capture "$out\$short-stock-straddle.png" "$out\$short-hw-straddle.png" @(4, 16)
        Add-Row "straddling two monitors" $short ("diff>8 {0:P2}, diff>64 {1:P2}" -f $c.Diff8, $c.Diff64) (($c.Diff64 -lt 0.005) -and ($c.Diff8 -lt 0.015))
    }

    # ---- optional: throughput benchmark
    if ($Bench) {
        $bs = Run-Smoke $t $false @("--bench", "$BenchSeconds")
        $bh = Run-Smoke $t $true @("--bench", "$BenchSeconds")
        $ls = ($bs -split "`n" | Where-Object { $_ -match "^bench:" } | Select-Object -First 1) -replace "^bench: ", ""
        $lh = ($bh -split "`n" | Where-Object { $_ -match "^bench:" } | Select-Object -First 1) -replace "^bench: ", ""
        Add-Row "bench stock DX9" $short $ls.Trim() $true
        Add-Row "bench D3D11" $short $lh.Trim() $true

        # layered (AllowsTransparency) window: readback + UpdateLayeredWindow on both engines (D30)
        $ts = Run-Smoke $t $false @("--bench", "$BenchSeconds", "--bench-transparent")
        $th = Run-Smoke $t $true @("--bench", "$BenchSeconds", "--bench-transparent")
        $lts = ($ts -split "`n" | Where-Object { $_ -match "^bench:" } | Select-Object -First 1) -replace "^bench: ", ""
        $lth = ($th -split "`n" | Where-Object { $_ -match "^bench:" } | Select-Object -First 1) -replace "^bench: ", ""
        Add-Row "bench stock DX9 (transparent window)" $short $lts.Trim() $true
        Add-Row "bench D3D11 (transparent window)" $short $lth.Trim() $true

        # layered window, small dirty region each frame (typical UI): readback cost and GPU back-pressure (D31)
        $ps = Run-Smoke $t $false @("--bench", "$BenchSeconds", "--bench-transparent", "--bench-partial")
        $ph = Run-Smoke $t $true @("--bench", "$BenchSeconds", "--bench-transparent", "--bench-partial")
        $lps = ($ps -split "`n" | Where-Object { $_ -match "^bench:" } | Select-Object -First 1) -replace "^bench: ", ""
        $lph = ($ph -split "`n" | Where-Object { $_ -match "^bench:" } | Select-Object -First 1) -replace "^bench: ", ""
        Add-Row "bench stock DX9 (transparent, partial update)" $short $lps.Trim() $true
        Add-Row "bench D3D11 (transparent, partial update)" $short $lph.Trim() $true
    }
}

$header = @("| scenario | tfm | result | status |", "|---|---|---|---|")
$report = ($header + $rows) -join "`n"
$report | Out-File -Encoding utf8 (Join-Path $out "report.md")
Write-Output $report
if ($failed) { Write-Output "GATE FAILED"; exit 1 } else { Write-Output "GATE PASSED"; exit 0 }
