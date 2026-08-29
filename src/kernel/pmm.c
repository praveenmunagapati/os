#include "pmm.h"
#include "limine.h"

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

uint64_t pmm_total_memory = 0;
uint64_t pmm_free_memory = 0;
uint64_t pmm_used_memory = 0;

static uint8_t* bitmap = 0;
static uint64_t bitmap_size = 0;
static uint64_t bitmap_pages = 0;

/* Bitmap operations */
static void bitmap_set(uint64_t bit) {
    bitmap[bit / 8] |= (1 << (bit % 8));
}

static void bitmap_clear(uint64_t bit) {
    bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static int bitmap_test(uint64_t bit) {
    return (bitmap[bit / 8] & (1 << (bit % 8))) != 0;
}

void pmm_init(void) {
    if (memmap_request.response == 0) {
        return; /* Error: Bootloader didn't provide memory map */
    }

    struct limine_memmap_response *memmap = memmap_request.response;
    uint64_t highest_address = 0;

    pmm_total_memory = 0;
    
    /* 1. Find the highest physical memory address and calculate total RAM */
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        
        /* Only count actual RAM towards total memory */
        if (entry->type == LIMINE_MEMMAP_USABLE || 
            entry->type == LIMINE_MEMMAP_RESERVED ||
            entry->type == LIMINE_MEMMAP_ACPI_RECLAIMABLE ||
            entry->type == LIMINE_MEMMAP_ACPI_NVS ||
            entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE ||
            entry->type == LIMINE_MEMMAP_KERNEL_AND_MODULES) 
        {
            pmm_total_memory += entry->length;
        }

        uint64_t top = entry->base + entry->length;
        if (top > highest_address) {
            highest_address = top;
        }
    }

    bitmap_pages = highest_address / PAGE_SIZE;
    bitmap_size = bitmap_pages / 8;
    if (bitmap_size * 8 < bitmap_pages) {
        bitmap_size++; /* round up */
    }

    /* 2. Find a Usable region large enough for our Bitmap */
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= bitmap_size) {
            bitmap = (uint8_t*)entry->base;
            break;
        }
    }

    if (bitmap == 0) {
        for (;;) { __asm__ volatile ("hlt"); } /* Out of memory for bitmap! */
    }

    /* 3. Initialize everything as Used (1) to be safe */
    for (uint64_t i = 0; i < bitmap_size; i++) {
        bitmap[i] = 0xFF;
    }

    /* 4. Free the Usable regions (Set to 0) */
    pmm_free_memory = 0;
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            pmm_free_memory += entry->length;
            uint64_t start = entry->base / PAGE_SIZE;
            uint64_t count = entry->length / PAGE_SIZE;
            for (uint64_t j = 0; j < count; j++) {
                bitmap_clear(start + j);
            }
        }
    }

    /* 5. Mark the memory used by the Bitmap itself as Used (1) */
    uint64_t bitmap_start_page = (uint64_t)bitmap / PAGE_SIZE;
    uint64_t bitmap_page_count = bitmap_size / PAGE_SIZE;
    if (bitmap_size % PAGE_SIZE != 0) bitmap_page_count++;
    
    for (uint64_t j = 0; j < bitmap_page_count; j++) {
        bitmap_set(bitmap_start_page + j);
    }
    
    /* The bitmap used some usable memory, so subtract it from free memory */
    pmm_free_memory -= (bitmap_page_count * PAGE_SIZE);
    
    /* Accurate used memory is now simply Total - Free */
    pmm_used_memory = pmm_total_memory - pmm_free_memory;
}

void* pmm_alloc_page(void) {
    for (uint64_t i = 0; i < bitmap_pages; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            pmm_used_memory += PAGE_SIZE;
            pmm_free_memory -= PAGE_SIZE;
            return (void*)(i * PAGE_SIZE);
        }
    }
    return 0; /* Out of Memory */
}

void pmm_free_page(void* ptr) {
    uint64_t page = (uint64_t)ptr / PAGE_SIZE;
    if (bitmap_test(page)) {
        bitmap_clear(page);
        pmm_used_memory -= PAGE_SIZE;
        pmm_free_memory += PAGE_SIZE;
    }
}
