@echo off
setlocal

set TOOLS_DIR=%~dp0tools
set KERNEL=%~dp0build\kernel.bin

:: Search for QEMU inside tools\qemu (it might be in a subfolder)
for /f "delims=" %%i in ('dir /s /b "%TOOLS_DIR%\qemu\qemu-system-x86_64.exe" 2^>nul') do set QEMU=%%i

if "%QEMU%"=="" (
    echo QEMU not found! Please run setup.ps1 first.
    exit /b 1
)

if not exist "%KERNEL%" (
    echo Kernel binary not found! Please run build.bat first.
    exit /b 1
)

echo Starting QEMU...
"%QEMU%" -kernel "%KERNEL%" -serial stdio -no-reboot
