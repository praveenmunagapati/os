#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096

extern uint64_t pmm_total_memory;
extern uint64_t pmm_free_memory;
extern uint64_t pmm_used_memory;

void pmm_init(void);
void* pmm_alloc_page(void);
void pmm_free_page(void* ptr);

#endif
