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
    [int]$StraddleLeft = -1
)

$ErrorActionPreference = "Stop"
$tests = $PSScriptRoot
$repo = Split-Path $tests -Parent
$app = Join-Path $tests "SmokeApp"
$out = Join-Path $tests "out"
$dllDir = Join-Path $out "dll"
New-Item -ItemType Directory -Force $out, $dllDir | Out-Null

if ($Dll -eq "") { $Dll = Join-Path $repo "artifacts\bin\wpfgfx\x64\Release" }
Copy-Item (Join-Path $Dll "wpfgfx_cor3.dll") $dllDir -Force
$compiler = Join-Path $Dll "D3DCompiler_47_cor3.dll"
if (-not (Test-Path $compiler)) { $compiler = Join-Path $repo "artifacts\bin\D3DCompiler\x64\Release\D3DCompiler_47_cor3.dll" }
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
    $exe = Join-Path $app "bin\Release\$tfm\win-x64\SmokeApp.exe"
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
        try { & dotnet build -c Release -p:SmokeTfm=$t -nologo -v q | Out-Null } finally { Pop-Location }
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
    }
}

$header = @("| scenario | tfm | result | status |", "|---|---|---|---|")
$report = ($header + $rows) -join "`n"
$report | Out-File -Encoding utf8 (Join-Path $out "report.md")
Write-Output $report
if ($failed) { Write-Output "GATE FAILED"; exit 1 } else { Write-Output "GATE PASSED"; exit 0 }
