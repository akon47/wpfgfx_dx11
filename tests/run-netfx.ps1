<#
.SYNOPSIS
  .NET Framework 4.8.x check for the D3D11 engine (D37): runs tests/NetFxHost on the stock wpfgfx_v0400.dll
  and on the D3D11 engine for each architecture, compares the captures and prints a table. Exit code 1 when
  any row fails. Rows for an architecture whose .NET Framework WPF is not installed are skipped.

.EXAMPLE
  .\tests\run-netfx.ps1                       # artifacts build, x64 and x86
  .\tests\run-netfx.ps1 -Arch x64 -Dll C:\somewhere

.NOTES
  .NET Framework always loads wpfgfx from its own install folder, so the host redirects that load inside its
  own process (see tests/NetFxHost/Program.cs). Nothing outside tests/out is changed.
#>
param(
    [string]$Dll = "",
    [string[]]$Arch = @("x64", "x86")
)

$ErrorActionPreference = "Stop"
$tests = $PSScriptRoot
$repo = Split-Path $tests -Parent
$host_ = Join-Path $tests "NetFxHost"
$out = Join-Path $tests "out"
New-Item -ItemType Directory -Force $out | Out-Null

Add-Type -AssemblyName System.Drawing

function Compare-Capture([string]$a, [string]$b) {
    $ia = [System.Drawing.Bitmap]::FromFile($a)
    $ib = [System.Drawing.Bitmap]::FromFile($b)
    try {
        $w = [Math]::Min($ia.Width, $ib.Width); $h = [Math]::Min($ia.Height, $ib.Height)
        $n8 = 0; $n64 = 0
        for ($y = 0; $y -lt $h; $y++) {
            for ($x = 0; $x -lt $w; $x++) {
                $ca = $ia.GetPixel($x, $y); $cb = $ib.GetPixel($x, $y)
                $d = [Math]::Max([Math]::Abs($ca.R - $cb.R), [Math]::Max([Math]::Abs($ca.G - $cb.G), [Math]::Abs($ca.B - $cb.B)))
                if ($d -gt 8) { $n8++ }
                if ($d -gt 64) { $n64++ }
            }
        }
        return @{ Diff8 = $n8 / ($w * $h); Diff64 = $n64 / ($w * $h) }
    } finally { $ia.Dispose(); $ib.Dispose() }
}

function Run-Host([string]$exe, [string]$engine, [string]$png) {
    $prev = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    try { $text = (& $exe $engine $png 2>&1 | ForEach-Object { "$_" } | Out-String) } finally { $ErrorActionPreference = $prev }
    return @{ Text = $text; Exit = $LASTEXITCODE }
}

$rows = @()
$failed = $false
function Add-Row([string]$scenario, [string]$arch, [string]$detail, [string]$result) {
    $script:rows += ("| {0} | {1} | {2} | {3} |" -f $scenario, $arch, $detail, $result)
    if ($result -eq "FAIL") { $script:failed = $true }
}

foreach ($a in $Arch) {
    $fxDir = if ($a -eq "x64") { "Framework64" } else { "Framework" }
    $stockDll = Join-Path $env:WINDIR "Microsoft.NET\$fxDir\v4.0.30319\WPF\wpfgfx_v0400.dll"
    if (-not (Test-Path $stockDll)) {
        Add-Row "netfx" $a ".NET Framework WPF not installed" "SKIP"
        continue
    }

    # Engine folder: the D3D11 engine under the .NET Framework module name, next to its shader compiler.
    $src = if ($Dll -ne "") { $Dll } else { Join-Path $repo "artifacts\bin\wpfgfx\$a\Release" }
    $engine = Join-Path $out "netfx-engine-$a"
    New-Item -ItemType Directory -Force $engine | Out-Null
    Copy-Item (Join-Path $src "wpfgfx_cor3.dll") (Join-Path $engine "wpfgfx_v0400.dll") -Force
    $compiler = Join-Path $src "D3DCompiler_47_cor3.dll"
    if (-not (Test-Path $compiler)) { $compiler = Join-Path $repo "artifacts\bin\D3DCompiler\$a\Release\D3DCompiler_47_cor3.dll" }
    Copy-Item $compiler $engine -Force

    Push-Location $host_
    try { & dotnet build -c Release -p:NetFxArch=$a -nologo -v q | Out-Null } finally { Pop-Location }
    $exe = Join-Path $host_ "bin\Release\$a\NetFxHost.exe"

    $fxVersion = (Get-Item $stockDll).VersionInfo.ProductVersion
    $s = Run-Host $exe "-" "$out\netfx-$a-stock.png"
    $h = Run-Host $exe $engine "$out\netfx-$a-dx11.png"

    $bits = if ($a -eq "x64") { "64-bit" } else { "32-bit" }
    $stockOk = ($s.Exit -eq 0) -and ($s.Text -match "Tier=2") -and ($s.Text -match "engine: stock") -and ($s.Text -match $bits)
    $hwOk = ($h.Exit -eq 0) -and ($h.Text -match "Tier=2") -and ($h.Text -match "engine: D3D11") -and ($h.Text -match "42,42,42,42424") -and ($h.Text -match $bits)
    $detail = "stock wpfgfx {0}: {1}; D3D11: {2}" -f $fxVersion, $(if ($stockOk) { "Tier 2" } else { "UNEXPECTED" }), $(if ($hwOk) { "Tier 2, repo DLL" } else { "FAILED" })
    Add-Row "engine" $a $detail $(if ($stockOk -and $hwOk) { "PASS" } else { "FAIL" })
    if (-not $hwOk) { Write-Host $h.Text }

    if ($stockOk -and $hwOk) {
        $c = Compare-Capture "$out\netfx-$a-stock.png" "$out\netfx-$a-dx11.png"
        Add-Row "scene (text, gradients, shadow, blur, controls, polyline)" $a ("diff>8 {0:P2}, diff>64 {1:P2}" -f $c.Diff8, $c.Diff64) $(if (($c.Diff64 -lt 0.005) -and ($c.Diff8 -lt 0.015)) { "PASS" } else { "FAIL" })
    }
}

$table = @("| scenario | arch | detail | result |", "|---|---|---|---|") + $rows
$table | ForEach-Object { Write-Host $_ }
$table | Set-Content -Encoding utf8 (Join-Path $out "report-netfx.md")
if ($failed) { exit 1 }
