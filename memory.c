#include "memory.h"
#ifdef DEBUG
#include "common.h"
#include "kernel.h"
#endif /* ifdef DEBUG */
#include "kernel.h"
#include "lib/dtb.h"

void init_memory_manager(struct fdt_header *fdt) {
#ifdef iDEBUG

  uint32_t magic = bswap32(fdt->magic);

  if (magic != 0xd00dfeed) {
    PANIC("FDT MAGIC NOT PRESENT!");
  }

  uint32_t reservedMemoryRegionsOffset = bswap32(fdt->off_mem_rsvmap);

  printf("RESERVED MEMORY REGIONS OFFSET:%d\n", reservedMemoryRegionsOffset);
  printf("RESERVED MEMORY REGIONS ADDRESS:%x\n",
         (void *)(((char *)fdt) + reservedMemoryRegionsOffset));

  struct fdt_reserve_entry *entry =
      (struct fdt_reserve_entry *)(((char *)fdt) + reservedMemoryRegionsOffset);

  while (entry->address != 0 || entry->size != 0) {
    printf("MEMORY REGION AT: %x, WITH SIZE: %d\n", bswap64(entry->address),
           bswap64(entry->size));
    entry += 1;
  }

  const char *value = searchDTB(fdt, "memory");
  if (value) {
    printf("MEMORY: %s", value);
  } else {
    PANIC("No memory region found in DTB");
  }
#endif /* ifdef iDEBUG */
}

extern char __free_ram[], __free_ram_end[];

paddr_t alloc_pages(uint32_t n) {
  static paddr_t next_paddr = (paddr_t)__free_ram;
  paddr_t paddr = next_paddr;
  next_paddr += n * PAGE_SIZE;

  if (next_paddr > (paddr_t)__free_ram_end) {
    PANIC("Out of memory\n");
  }

  memset((void *)paddr, 0, n * PAGE_SIZE);

  return paddr;
}
