# build.ps1 — сборка всего проекта
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
Write-Host "=== Build complete! ==="
Write-Host "Executables:"
Write-Host "  .\t16xe\t16xe.exe"
Write-Host "  .\tcc\tcc.exe"
Write-Host "  .\gui\gui-test.exe"
Write-Host ""
Write-Host "Run tests:"
Write-Host "  .\t16xe\t16xe.exe ..\tests\hello.tsm"