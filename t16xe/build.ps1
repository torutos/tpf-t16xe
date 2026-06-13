# build_t16xe.ps1 — сборка t16xe + tasm
$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

# Очистка
if (Test-Path build) { Remove-Item -Recurse -Force build }
New-Item -ItemType Directory build | Out-Null
Set-Location build

# Сборка
cmake .. -G "Ninja"
ninja

Write-Host ""
Write-Host "t16xe build complete!"
Write-Host ""