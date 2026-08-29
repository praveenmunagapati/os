#!/bin/bash
# =============================================================================
# build.sh - Build and run the x86_64 Hello World kernel
# =============================================================================
# Run this script from MSYS2 MINGW64 shell, or via:
#   C:\msys64\usr\bin\bash.exe -lc "cd /c/Users/sir/Desktop/projects/os && ./build.sh"
# =============================================================================

set -e

# --- Configuration ---
ASM=nasm
CC=gcc
LD=ld
QEMU=qemu-system-x86_64

CFLAGS="-ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
        -mno-red-zone -Wall -Wextra -m64 -c"
LDFLAGS="-n -T linker.ld -nostdlib"
ASMFLAGS="-f elf64"

BUILD_DIR=build
ISO_DIR=$BUILD_DIR/isofiles

# --- Colors for output ---
RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}=== Building x86_64 Hello World Kernel ===${NC}"

# --- Create build directory ---
mkdir -p $BUILD_DIR

# --- Assemble boot.asm ---
echo -e "${GREEN}[ASM]${NC} boot.asm -> boot.o"
$ASM $ASMFLAGS boot.asm -o $BUILD_DIR/boot.o

# --- Compile kernel.c ---
echo -e "${GREEN}[CC]${NC}  kernel.c -> kernel.o"
$CC $CFLAGS kernel.c -o $BUILD_DIR/kernel.o

# --- Link ---
echo -e "${GREEN}[LD]${NC}  Linking kernel.bin"
$LD $LDFLAGS -o $BUILD_DIR/kernel.bin $BUILD_DIR/boot.o $BUILD_DIR/kernel.o

echo -e "${GREEN}[OK]${NC}  Kernel built: $BUILD_DIR/kernel.bin"

# --- Check if grub-mkrescue is available to make an ISO ---
if command -v grub-mkrescue &> /dev/null && command -v xorriso &> /dev/null; then
    echo -e "${CYAN}--- Creating bootable ISO ---${NC}"
    mkdir -p $ISO_DIR/boot/grub
    cp $BUILD_DIR/kernel.bin $ISO_DIR/boot/kernel.bin
    cp grub.cfg $ISO_DIR/boot/grub/grub.cfg
    grub-mkrescue -o $BUILD_DIR/myos.iso $ISO_DIR 2>/dev/null
    echo -e "${GREEN}[OK]${NC}  ISO created: $BUILD_DIR/myos.iso"
    RUN_CMD="$QEMU -cdrom $BUILD_DIR/myos.iso"
else
    echo -e "${RED}[!]${NC}  grub-mkrescue/xorriso not found, using QEMU -kernel directly"
    RUN_CMD="$QEMU -kernel $BUILD_DIR/kernel.bin"
fi

# --- Run with QEMU ---
echo ""
echo -e "${CYAN}=== Launching QEMU ===${NC}"
echo -e "${GREEN}[RUN]${NC} $RUN_CMD"
echo ""

$RUN_CMD
