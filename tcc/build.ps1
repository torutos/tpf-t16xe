# build.ps1
Set-Location $PSScriptRoot
if (Test-Path build) { Remove-Item -Recurse -Force build }
New-Item -ItemType Directory build | Out-Null
Set-Location build
cmake .. -G "Ninja"
ninja
Write-Host ""
Write-Host "Build complete!"
Write-Host ""

# Запуск на тестовом файле
if (Test-Path ../main.c) {
    Write-Host "Running tcc on main.c..."
    .\tcc.exe ../main.c
    if (Test-Path ../main.asm) {
        Write-Host ""
        Write-Host "Generated main.asm:"
        Get-Content ../main.asm
    }
} else {
    Write-Host "Usage: .\tcc.exe <file.c>"
}