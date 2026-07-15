#pragma once

#include "common.h"
#include "lib/dtb.h"

#define PAGE_SIZE 4096

void init_memory_manager(struct fdt_header *fdt);
paddr_t alloc_pages(uint32_t n);
