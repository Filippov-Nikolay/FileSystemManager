# Локальный аналог CI: собирает тесты и запускает их.
# Использование: .\run_tests.ps1

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# -- Найти MSBuild ---------------------------------------------------------------

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

if (-not (Test-Path $vswhere)) {
    Write-Error "vswhere.exe не найден. Установите Visual Studio Build Tools."
}

$msbuild = & $vswhere -latest -requires Microsoft.Component.MSBuild `
    -find "MSBuild\**\Bin\MSBuild.exe" | Select-Object -First 1

if (-not $msbuild) {
    Write-Error "MSBuild не найден. Убедитесь, что установлены C++ Build Tools."
}

Write-Host "MSBuild: $msbuild" -ForegroundColor Cyan

# -- Сборка ----------------------------------------------------------------------

Write-Host "`nСборка тестов..." -ForegroundColor Cyan

& $msbuild FileSystemManagerTests\FileSystemManagerTests.vcxproj `
    /p:Configuration=Debug `
    /p:Platform=x64 `
    /m `
    /nologo `
    /verbosity:minimal

if ($LASTEXITCODE -ne 0) {
    Write-Host "`nСборка провалилась." -ForegroundColor Red
    exit 1
}

# -- Запуск тестов ---------------------------------------------------------------

Write-Host "`nЗапуск тестов..." -ForegroundColor Cyan

$exe = "FileSystemManagerTests\x64\Debug\FileSystemManagerTests.exe"

if (-not (Test-Path $exe)) {
    Write-Error "Исполняемый файл не найден: $exe"
}

& $exe

if ($LASTEXITCODE -ne 0) {
    Write-Host "`nТесты не прошли." -ForegroundColor Red
    exit 1
}

Write-Host "`nВсё готово - можно пушить." -ForegroundColor Green
