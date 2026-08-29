# Minimal x86_64 Bare Metal Hello World Kernel

This is a minimal x86_64 bare metal kernel that demonstrates:
1. Booting via the Multiboot1 specification using QEMU's `-kernel` flag.
2. Setting up basic Identity Paging (first 1GB) and the GDT.
3. Transitioning from the bootloader's 32-bit Protected Mode environment to 64-bit Long Mode.
4. Writing to the hardware Serial Port (COM1) for debugging output.
5. Writing to the hardware VGA text mode buffer (`0xB8000`) for visual output.

Because QEMU's `-kernel` flag struggles to load true 64-bit ELF files natively, this kernel is built entirely in assembly as a **Flat Binary** (`-f bin`) using the Multiboot "AOUT Kludge" fields to tell the bootloader exactly how to load it.

## Prerequisites
* **NASM**: Used to compile the assembly file.
* **QEMU**: Specifically `qemu-system-x86_64` to run the kernel.

## Building and Running

You can use the provided Makefile:

```bash
# Build the kernel
make

# Run the kernel in QEMU
make run
```

If you want to run it manually with serial output shown in your console:
```bash
qemu-system-x86_64 -kernel build/kernel.bin -serial stdio -no-reboot
```

## Structure
* **`boot.asm`**: The entire kernel. Contains the multiboot header, the 32-bit entry code, paging setup, transition to long mode, and the 64-bit logic to display colored text to the screen and characters to the serial port.
