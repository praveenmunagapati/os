# =============================================================================
# Makefile - x86_64 Hello World Kernel (Flat Binary with Multiboot AOUT)
# =============================================================================

ASM = nasm
QEMU = qemu-system-x86_64

BUILD = build
KERNEL = $(BUILD)/kernel.bin

.PHONY: all run clean

all: $(KERNEL)

$(BUILD):
	mkdir -p $(BUILD)

$(KERNEL): boot.asm | $(BUILD)
	$(ASM) -f bin $< -o $@

run: $(KERNEL)
	$(QEMU) -kernel $(KERNEL) -serial stdio -no-reboot

clean:
	rm -rf $(BUILD)
