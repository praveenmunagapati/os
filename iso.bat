@echo off
setlocal

set TOOLS_DIR=%~dp0tools
set XORRISO=%TOOLS_DIR%\xorriso\xorriso.exe
set LIMINE_DIR=%TOOLS_DIR%\limine
set BUILD_DIR=%~dp0build
set ISO_ROOT=%BUILD_DIR%\iso_root
set ISO_FILE=%~dp0MyOS.iso
set KERNEL=%BUILD_DIR%\kernel.elf

if not exist "%XORRISO%" (
    echo xorriso not found! Please run setup.ps1 first.
    exit /b 1
)

if not exist "%KERNEL%" (
    echo Kernel binary not found! Please run build.bat first.
    exit /b 1
)

echo [1/3] Preparing ISO root directory...
if exist "%ISO_ROOT%" rmdir /s /q "%ISO_ROOT%"
mkdir "%ISO_ROOT%"

echo [2/3] Copying files...
mkdir "%ISO_ROOT%\EFI\BOOT"
copy /y "%KERNEL%" "%ISO_ROOT%\" >nul
copy /y "%~dp0limine.conf" "%ISO_ROOT%\" >nul
copy /y "%LIMINE_DIR%\limine-bios-cd.bin" "%ISO_ROOT%\" >nul
copy /y "%LIMINE_DIR%\limine-uefi-cd.bin" "%ISO_ROOT%\" >nul
copy /y "%LIMINE_DIR%\limine-bios.sys" "%ISO_ROOT%\" >nul
copy /y "%LIMINE_DIR%\BOOTX64.EFI" "%ISO_ROOT%\EFI\BOOT\" >nul

echo [3/3] Generating Bootable ISO...
"%XORRISO%" -as mkisofs -b limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label build\iso_root -o MyOS.iso

if %ERRORLEVEL% equ 0 (
    echo.
    echo Successfully created: %ISO_FILE%
    echo You can now boot this file in VirtualBox!
) else (
    echo.
    echo Failed to create ISO!
)
