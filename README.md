# MyOS

A self-contained, portable x86_64 OS written in C and Assembly.

This project is designed to be built and run on Windows **without requiring Docker, MSYS2, WSL, or any global installations**. Everything needed is contained directly in this folder.

## Setup

Before you can build or run the OS, you need to download the necessary portable compilers and emulators (NASM, LLVM/Clang, and QEMU).

1. Open PowerShell in this folder.
2. Run the setup script:
   ```powershell
   .\setup.ps1
   ```
   *This will download all the necessary tools into a local `tools/` folder. It will not touch your global system environment.*

## Building

To compile the C kernel and the assembly bootstub into a flat binary executable, run the build script:

```cmd
build.bat
```
This script uses the portable `clang` to compile `kernel.c` and `nasm` to assemble `boot.asm`, outputting the result to `build/kernel.bin`.

## Running

To run the OS inside the QEMU emulator, run:

```cmd
run.bat
```
QEMU will launch and immediately boot into your C kernel, rendering text directly to the VGA display buffer!
