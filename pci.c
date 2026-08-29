#include "pci.h"
#include "io.h"
#include <stdint.h>

extern void term_puts(const char *str);
extern void print_hex(uint32_t val);

static uint32_t pci_config_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    
    address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
    
    outl(0xCF8, address);
    return inl(0xCFC);
}

static uint16_t pci_get_vendor_id(uint8_t bus, uint8_t slot, uint8_t func) {
    uint32_t dword = pci_config_read_dword(bus, slot, func, 0);
    return (uint16_t)(dword & 0xFFFF);
}

static uint16_t pci_get_device_id(uint8_t bus, uint8_t slot, uint8_t func) {
    uint32_t dword = pci_config_read_dword(bus, slot, func, 0);
    return (uint16_t)(dword >> 16);
}

static uint8_t pci_get_class(uint8_t bus, uint8_t slot, uint8_t func) {
    uint32_t dword = pci_config_read_dword(bus, slot, func, 8);
    return (uint8_t)(dword >> 24);
}

static uint8_t pci_get_subclass(uint8_t bus, uint8_t slot, uint8_t func) {
    uint32_t dword = pci_config_read_dword(bus, slot, func, 8);
    return (uint8_t)(dword >> 16);
}

static uint8_t pci_get_progif(uint8_t bus, uint8_t slot, uint8_t func) {
    uint32_t dword = pci_config_read_dword(bus, slot, func, 8);
    return (uint8_t)(dword >> 8);
}

void pci_scan(void) {
    term_puts("Scanning PCI Bus...\n");
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint16_t vendor = pci_get_vendor_id((uint8_t)bus, slot, 0);
            if (vendor == 0xFFFF) continue; /* Device doesn't exist */
            
            uint16_t device = pci_get_device_id((uint8_t)bus, slot, 0);
            uint8_t class_id = pci_get_class((uint8_t)bus, slot, 0);
            uint8_t subclass = pci_get_subclass((uint8_t)bus, slot, 0);
            uint8_t progif = pci_get_progif((uint8_t)bus, slot, 0);
            
            term_puts("  [PCI] Vendor: 0x");
            print_hex(vendor);
            term_puts(" Device: 0x");
            print_hex(device);
            term_puts(" Class: ");
            print_hex(class_id);
            
            if (class_id == 0x0C && subclass == 0x03) {
                term_puts(" (USB Controller");
                if (progif == 0x30) term_puts(" - xHCI / USB 3.0");
                if (progif == 0x20) term_puts(" - EHCI / USB 2.0");
                if (progif == 0x10) term_puts(" - OHCI");
                if (progif == 0x00) term_puts(" - UHCI");
                term_puts(")\n");
            } else {
                term_puts("\n");
            }
        }
    }
}
