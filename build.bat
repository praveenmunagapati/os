@echo off
setlocal

set TOOLS_DIR=%~dp0tools
set NASM=%TOOLS_DIR%\nasm\nasm.exe
set BUILD_DIR=%~dp0build
set KERNEL=%BUILD_DIR%\kernel.bin

if not exist "%NASM%" (
    echo NASM not found! Please run setup.ps1 first.
    exit /b 1
)

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

echo Building kernel...
"%NASM%" -f bin boot.asm -o "%KERNEL%"

if %ERRORLEVEL% equ 0 (
    echo Build successful: %KERNEL%
) else (
    echo Build failed!
)
