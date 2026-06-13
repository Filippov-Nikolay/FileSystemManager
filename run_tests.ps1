# Local CI: build tests and run them.
# Usage: .\run_tests.ps1

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# -- Find MSBuild ----------------------------------------------------------------

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

if (-not (Test-Path $vswhere)) {
    Write-Error "vswhere.exe not found. Install Visual Studio Build Tools."
}

$msbuild = & $vswhere -latest -requires Microsoft.Component.MSBuild `
    -find "MSBuild\**\Bin\MSBuild.exe" | Select-Object -First 1

if (-not $msbuild) {
    Write-Error "MSBuild not found. Make sure C++ Build Tools are installed."
}

Write-Host "MSBuild: $msbuild" -ForegroundColor Cyan

# -- Build -----------------------------------------------------------------------

Write-Host "`nBuilding tests..." -ForegroundColor Cyan

& $msbuild FileSystemManagerTests\FileSystemManagerTests.vcxproj `
    /p:Configuration=Debug `
    /p:Platform=x64 `
    /m `
    /nologo `
    /verbosity:minimal

if ($LASTEXITCODE -ne 0) {
    Write-Host "`nBuild failed." -ForegroundColor Red
    exit 1
}

# -- Run -------------------------------------------------------------------------

Write-Host "`nRunning tests..." -ForegroundColor Cyan

$exe = "FileSystemManagerTests\x64\Debug\FileSystemManagerTests.exe"

if (-not (Test-Path $exe)) {
    Write-Error "Executable not found: $exe"
}

& $exe

if ($LASTEXITCODE -ne 0) {
    Write-Host "`nTests failed." -ForegroundColor Red
    exit 1
}

Write-Host "`nAll good - safe to push." -ForegroundColor Green
