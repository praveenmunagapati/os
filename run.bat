@echo off
setlocal

set TOOLS_DIR=%~dp0tools
set ISO=%~dp0MyOS.iso

:: Search for QEMU inside tools\qemu (it might be in a subfolder)
for /f "delims=" %%i in ('dir /s /b "%TOOLS_DIR%\qemu\qemu-system-x86_64.exe" 2^>nul') do set QEMU=%%i

if "%QEMU%"=="" (
    echo QEMU not found! Please run setup.ps1 first.
    exit /b 1
)

if not exist "%ISO%" (
    echo ISO not found! Please run iso.bat first.
    exit /b 1
)

echo Starting QEMU...
"%QEMU%" -cdrom "%ISO%" -serial stdio -no-reboot -device qemu-xhci -d int -D qemu.log
