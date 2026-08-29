@echo off
setlocal

set TOOLS_DIR=%~dp0tools
set NASM=%TOOLS_DIR%\nasm\nasm.exe
set CLANG=%TOOLS_DIR%\llvm\bin\clang.exe
set LLD=%TOOLS_DIR%\llvm\bin\ld.lld.exe
set OBJCOPY=%TOOLS_DIR%\llvm\bin\llvm-objcopy.exe

set BUILD_DIR=%~dp0build
set BOOT_OBJ=%BUILD_DIR%\boot.o
set KERNEL_OBJ=%BUILD_DIR%\kernel.o
set IDT_OBJ=%BUILD_DIR%\idt.o
set KEYBOARD_OBJ=%BUILD_DIR%\keyboard.o
set PCI_OBJ=%BUILD_DIR%\pci.o
set KERNEL_ELF=%BUILD_DIR%\kernel.elf
set KERNEL_BIN=%BUILD_DIR%\kernel.bin

if not exist "%NASM%" (
    echo NASM not found! Please run setup.ps1 first.
    exit /b 1
)
if not exist "%CLANG%" (
    echo LLVM/Clang not found! Please run setup.ps1 first.
    exit /b 1
)

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

echo [1/4] Assembling bootloader...
"%NASM%" -f elf64 "%~dp0src\arch\x86_64\boot.asm" -o "%BOOT_OBJ%"
if %ERRORLEVEL% neq 0 exit /b 1

echo [2/4] Compiling C source files...
"%CLANG%" -target x86_64-pc-none-elf -mcmodel=kernel -mno-red-zone -ffreestanding -I "%~dp0src\include" -c "%~dp0src\kernel\kernel.c" -o "%KERNEL_OBJ%"
"%CLANG%" -target x86_64-pc-none-elf -mcmodel=kernel -mno-red-zone -ffreestanding -I "%~dp0src\include" -c "%~dp0src\arch\x86_64\idt.c" -o "%IDT_OBJ%"
"%CLANG%" -target x86_64-pc-none-elf -mcmodel=kernel -mno-red-zone -ffreestanding -I "%~dp0src\include" -c "%~dp0src\drivers\keyboard.c" -o "%KEYBOARD_OBJ%"
"%CLANG%" -target x86_64-pc-none-elf -mcmodel=kernel -mno-red-zone -ffreestanding -I "%~dp0src\include" -c "%~dp0src\drivers\pci.c" -o "%PCI_OBJ%"
if %ERRORLEVEL% neq 0 exit /b 1

echo [3/4] Linking...
"%LLD%" -m elf_x86_64 -T "%~dp0src\arch\x86_64\linker.ld" -o "%KERNEL_ELF%" "%BOOT_OBJ%" "%KERNEL_OBJ%" "%IDT_OBJ%" "%KEYBOARD_OBJ%" "%PCI_OBJ%"
if %ERRORLEVEL% neq 0 exit /b 1

echo Build successful: %KERNEL_ELF%
