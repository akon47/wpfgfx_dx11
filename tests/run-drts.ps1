<#
.SYNOPSIS
  dotnet/wpf-test 의 milcore DRT 들을 스톡 엔진과 교체 엔진(wpfgfx_cor3.dll)으로 각각 실행해 결과를 표로 보고한다.

.DESCRIPTION
  DRT 는 프레임워크 종속 .NET 앱이라 출력 폴더에 DLL 을 복사해도 교체되지 않는다(CLAUDE.md §5). 이 스크립트는 설치된
  공유 프레임워크를 임시 폴더로 복사해 그 안의 wpfgfx_cor3.dll·D3DCompiler_47_cor3.dll 만 바꾸고, DOTNET_ROOT 로 그 복사본을
  가리켜 DRT 를 실행한다. 머신의 공유 프레임워크는 건드리지 않는다. 교체 엔진이 실제로 로드됐는지는 WPFGFX_DX11_LOG 로그 파일이
  생겼는지로 확인한다(스톡 엔진은 그 변수를 모른다).

  준비:
    git clone --depth 1 --branch release/10.0 https://github.com/dotnet/wpf-test.git <폴더>
    cd <폴더>; .\Build.cmd -configuration Release -platform x64 -projects src\Test\mil\MIL.sln
    → <폴더>\publish\test\Release\x64\Test\DRT 가 -TestDir 이다.

.EXAMPLE
  .\tests\run-drts.ps1 -TestDir ..\wpf-test\publish\test\Release\x64\Test\DRT
  .\tests\run-drts.ps1 -TestDir <DRT 폴더> -Dll <엔진 DLL 폴더> -Drts DrtMil2D,DrtBasic3D
#>
param(
    [Parameter(Mandatory = $true)][string]$TestDir,
    [string]$Dll = "",                                  # 기본: tests\out\dll
    [string[]]$Drts = @("DrtChangeable", "DrtVisualTarget", "DrtMil2D", "DrtElement3D", "DrtBasic3D", "DRTStoryboards",
                        "DrtAnimation", "DrtTiming", "DrtColorAPI", "DrtMediaAPI", "DrtMedia"),
    [int]$TimeoutSec = 240,
    [string]$RuntimeCopy = (Join-Path $env:TEMP "wpfgfx-drt-runtime"),
    [string]$LogDir = ""                                # 기본: tests\out\drt
)

$ErrorActionPreference = "Stop"
if ($Dll -eq "") { $Dll = Join-Path $PSScriptRoot "out\dll" }
if ($LogDir -eq "") { $LogDir = Join-Path $PSScriptRoot "out\drt" }
$TestDir = (Resolve-Path $TestDir).Path
$Dll = (Resolve-Path $Dll).Path
New-Item -ItemType Directory -Force $LogDir | Out-Null

# ---- which shared framework the DRTs will use: highest installed patch of the major in the runtimeconfig
$cfg = Get-ChildItem $TestDir -Filter "*.runtimeconfig.json" | Select-Object -First 1
if (-not $cfg) { throw "no runtimeconfig.json in $TestDir (built?)" }
$major = ([regex]::Match((Get-Content $cfg.FullName -Raw), '"version":\s*"(\d+)\.')).Groups[1].Value
$dotnet = Split-Path (Get-Command dotnet).Source
function Latest([string]$dir) {
    return (Get-ChildItem $dir | Where-Object { $_.Name -like "$major.*" } | Sort-Object { [version]($_.Name -replace '-.*$', '') } | Select-Object -Last 1).Name
}
$ncore = Latest "$dotnet\shared\Microsoft.NETCore.App"
$wdesk = Latest "$dotnet\shared\Microsoft.WindowsDesktop.App"
$fxr = Latest "$dotnet\host\fxr"
if (-not $ncore -or -not $wdesk -or -not $fxr) { throw "no installed .NET $major runtime (NETCore=$ncore WindowsDesktop=$wdesk fxr=$fxr)" }

# ---- private runtime copy with the replacement engine
$priv = Join-Path $RuntimeCopy $wdesk
foreach ($pair in @(@("shared\Microsoft.NETCore.App\$ncore"), @("shared\Microsoft.WindowsDesktop.App\$wdesk"), @("host\fxr\$fxr"))) {
    $rel = $pair[0]
    if (-not (Test-Path (Join-Path $priv $rel))) {
        New-Item -ItemType Directory -Force (Join-Path $priv $rel) | Out-Null
        & robocopy (Join-Path $dotnet $rel) (Join-Path $priv $rel) /E /NFL /NDL /NJH /NJS /NP | Out-Null
    }
}
Copy-Item "$dotnet\dotnet.exe" (Join-Path $priv "dotnet.exe") -Force
Copy-Item (Join-Path $Dll "wpfgfx_cor3.dll") (Join-Path $priv "shared\Microsoft.WindowsDesktop.App\$wdesk\wpfgfx_cor3.dll") -Force
Copy-Item (Join-Path $Dll "D3DCompiler_47_cor3.dll") (Join-Path $priv "shared\Microsoft.WindowsDesktop.App\$wdesk\D3DCompiler_47_cor3.dll") -Force
$ver = (Get-Item (Join-Path $priv "shared\Microsoft.WindowsDesktop.App\$wdesk\wpfgfx_cor3.dll")).VersionInfo.FileVersion
Write-Host "runtime copy: $priv (WindowsDesktop $wdesk, wpfgfx $ver)"

function Run-Drt([string]$name, [bool]$replacement) {
    $exe = Join-Path $TestDir "$name.exe"
    if (-not (Test-Path $exe)) { return @{ Exit = "missing"; Seconds = 0; Loaded = $false; Tail = "" } }
    $tag = $(if ($replacement) { "d3d11" } else { "stock" })
    $out = Join-Path $LogDir "$name-$tag.txt"
    $engineLog = Join-Path $LogDir "$name-$tag-engine.log"
    Remove-Item $out, $engineLog -ErrorAction SilentlyContinue
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $exe
    $psi.WorkingDirectory = $TestDir
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    if ($replacement) {
        $psi.Environment["DOTNET_ROOT"] = $priv
        $psi.Environment["DOTNET_ROOT_X64"] = $priv
        $psi.Environment["WPFGFX_DX11_LOG"] = $engineLog
    }
    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    $p = [System.Diagnostics.Process]::Start($psi)
    $stdout = $p.StandardOutput.ReadToEndAsync()
    $stderr = $p.StandardError.ReadToEndAsync()
    # which wpfgfx_cor3.dll the process maps (polled; a short API-only DRT may exit before the engine loads)
    $modulePath = ""
    $deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSec)
    while (-not $p.HasExited -and [DateTime]::UtcNow -lt $deadline) {
        if ($modulePath -eq "") {
            try {
                $m = (Get-Process -Id $p.Id -ErrorAction Stop).Modules | Where-Object { $_.ModuleName -ieq "wpfgfx_cor3.dll" } | Select-Object -First 1
                if ($m) { $modulePath = $m.FileName }
            } catch {}
        }
        Start-Sleep -Milliseconds 50
    }
    $exit = "timeout"
    if ($p.WaitForExit(1000)) { $exit = $p.ExitCode } else { try { $p.Kill() } catch {} }
    $sw.Stop()
    ($stdout.Result + $stderr.Result) | Out-File -Encoding utf8 $out
    $tail = ((Get-Content $out -ErrorAction SilentlyContinue | Where-Object { $_ -match "\S" } | Select-Object -Last 2) -join " / ")
    $log = (Test-Path $engineLog) -and ((Get-Item $engineLog).Length -gt 0)
    $fromCopy = ($modulePath -ne "") -and $modulePath.StartsWith($priv, [StringComparison]::OrdinalIgnoreCase)
    $evidence = $(if ($log) { "engine log" } elseif ($fromCopy) { "module from runtime copy" } elseif ($modulePath -ne "") { "UNEXPECTED module: " + $modulePath } else { "- (no rendering)" })
    return @{ Exit = $exit; Seconds = [Math]::Round($sw.Elapsed.TotalSeconds, 1); Loaded = ($log -or $fromCopy); Evidence = $evidence; Tail = $tail }
}

$rows = @("| DRT | stock exit | D3D11 exit | D3D11 engine evidence | seconds (stock / D3D11) | status |", "|---|---|---|---|---|---|")
$failed = $false
$anyLoaded = $false
foreach ($d in $Drts) {
    $s = Run-Drt $d $false
    $h = Run-Drt $d $true
    if ($h.Loaded) { $anyLoaded = $true }
    # pass: the replacement engine ends with 0, or with the same non-zero code as stock
    $pass = ($s.Exit -eq "missing") -or ($h.Exit -eq 0) -or ($h.Exit -eq $s.Exit)
    if (-not $pass) { $failed = $true }
    $status = $(if ($s.Exit -eq "missing") { "SKIP (not built)" }
                elseif (-not $pass) { "FAIL" }
                elseif ($s.Exit -eq 0 -and $h.Exit -eq 0) { "PASS" }
                elseif ($h.Exit -eq 0) { "PASS (stock fails: $($s.Exit))" }
                else { "SAME AS STOCK" })
    $rows += ("| {0} | {1} | {2} | {3} | {4} / {5} | {6} |" -f $d, $s.Exit, $h.Exit, $h.Evidence, $s.Seconds, $h.Seconds, $status)
    if (-not $pass) { Write-Host ("  {0}: stock tail: {1}" -f $d, $s.Tail); Write-Host ("  {0}: D3D11 tail: {1}" -f $d, $h.Tail) }
}
if (-not $anyLoaded) { $failed = $true; Write-Host "no DRT loaded the replacement engine from the runtime copy (DOTNET_ROOT not honoured?)" }
$rows | ForEach-Object { Write-Output $_ }
if ($failed) { Write-Output "DRTS FAILED"; exit 1 } else { Write-Output "DRTS PASSED"; exit 0 }
