param([string]$NsisCompiler)
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
Set-Location -LiteralPath $root
$qtBin = Split-Path -Parent (Get-Command windeployqt.exe -ErrorAction Stop).Source
$qtPlugins = Join-Path (Split-Path -Parent $qtBin) 'share\qt6\plugins'
$stage = Join-Path $root 'build-release\package\LabBook'
$expectedParent = Join-Path $root 'build-release\package'
if (-not $stage.StartsWith($expectedParent + '\', [System.StringComparison]::OrdinalIgnoreCase)) {
  throw 'Stage path is outside build-release/package.'
}
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed.' }
cmake --build build-release -j 4
if ($LASTEXITCODE -ne 0) { throw 'Release build failed.' }
if (Test-Path -LiteralPath $stage) { Remove-Item -LiteralPath $stage -Recurse -Force }
New-Item -ItemType Directory -Path $stage -Force | Out-Null
Copy-Item -LiteralPath 'build-release\LabBooker.exe' -Destination (Join-Path $stage 'LabBook.exe')
windeployqt --release --no-plugins --no-translations --compiler-runtime --no-system-d3d-compiler --no-opengl-sw (Join-Path $stage 'LabBook.exe')
if ($LASTEXITCODE -ne 0) { throw 'Qt deployment failed.' }
python packaging/collect_runtime.py $stage $qtBin $qtPlugins
if ($LASTEXITCODE -ne 0) { throw 'Runtime collection failed.' }
Copy-Item -LiteralPath 'packaging\INSTALL.txt' -Destination (Join-Path $stage 'INSTALL.txt')
python packaging/finalize.py
if ($LASTEXITCODE -ne 0) { throw 'Portable packaging failed.' }
if (-not $NsisCompiler) { $NsisCompiler = (Get-Command makensis.exe -ErrorAction SilentlyContinue).Source }
if (-not $NsisCompiler) { throw 'Pass -NsisCompiler with the path to makensis.exe.' }
& $NsisCompiler 'packaging\LabBook.nsi'
if ($LASTEXITCODE -ne 0) { throw 'Installer build failed.' }
