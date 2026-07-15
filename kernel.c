#include "kernel.h"
#include "bios.h"
#include "common.h"
#include "exceptions.h"
// #include "lib/dtb.h"
#include "memory.h"

extern char __bss[], __bss_end[], __stack_top[], __kernel_base[],
    __free_ram_end[];
extern char _binary_shell_bin_start[], _binary_shell_bin_size[];

void putchar(char ch) { sbi_call(ch, 0, 0, 0, 0, 0, 0, 1); }

__attribute__((naked)) void switch_context(uint32_t *prev_sp,
                                           uint32_t *next_sp) {
  __asm__ __volatile__("addi sp, sp, -13 * 4\n"
                       "sw ra, 0 * 4(sp)\n"
                       "sw s0, 1 * 4(sp)\n"
                       "sw s1, 2 * 4(sp)\n"
                       "sw s2, 3 * 4(sp)\n"
                       "sw s3, 4 * 4(sp)\n"
                       "sw s4, 5 * 4(sp)\n"
                       "sw s5, 6 * 4(sp)\n"
                       "sw s6, 7 * 4(sp)\n"
                       "sw s7, 8 * 4(sp)\n"
                       "sw s8, 9 * 4(sp)\n"
                       "sw s9, 10 * 4(sp)\n"
                       "sw s10, 11 * 4(sp)\n"
                       "sw s11, 12 * 4(sp)\n"

                       "sw sp, (a0)\n"
                       "lw sp, (a1)\n" // Switch stack pointer here

                       "lw ra, 0 * 4(sp)\n"
                       "lw s0, 1 * 4(sp)\n"
                       "lw s1, 2 * 4(sp)\n"
                       "lw s2, 3 * 4(sp)\n"
                       "lw s3, 4 * 4(sp)\n"
                       "lw s4, 5 * 4(sp)\n"
                       "lw s5, 6 * 4(sp)\n"
                       "lw s6, 7 * 4(sp)\n"
                       "lw s7, 8 * 4(sp)\n"
                       "lw s8, 9 * 4(sp)\n"
                       "lw s9, 10 * 4(sp)\n"
                       "lw s10, 11 * 4(sp)\n"
                       "lw s11, 12 * 4(sp)\n"
                       "addi sp, sp, 13 * 4\n"
                       "ret\n");
}
__attribute__((naked)) void user_entry(void) {
  __asm__ __volatile__("csrw sepc, %[sepc]\n"
                       "csrw sstatus, %[sstatus]\n"
                       "sret\n"
                       :
                       : [sepc] "r"(USER_BASE), [sstatus] "r"(SSTATUS_SPIE));
}

struct process procs[MAX_PROCS];

struct process *create_process(const void *image, size_t image_size) {
  struct process *proc = NULL;
  int i;

  for (i = 0; i < MAX_PROCS; i++) {
    if (procs[i].state == PROC_UNUSED) {
      proc = &procs[i];
      break;
    }
  }

  if (!proc) {
    PANIC("No free process slots");
  }

  uint32_t *sp = (uint32_t *)&proc->stack[sizeof(proc->stack)];
  *--sp = 0;
  *--sp = 0;
  *--sp = 0;
  *--sp = 0;
  *--sp = 0;
  *--sp = 0;
  *--sp = 0;
  *--sp = 0;
  *--sp = 0;
  *--sp = 0;
  *--sp = 0;
  *--sp = 0;
  *--sp = (uint32_t)user_entry;

  uint32_t *page_table = (uint32_t *)alloc_pages(1);
  for (paddr_t physical_address = (paddr_t)__kernel_base;
       physical_address < (paddr_t)__free_ram_end;
       physical_address += PAGE_SIZE) {
    map_page(page_table, physical_address, physical_address,
             PAGE_R | PAGE_W | PAGE_X);
  }

  for (uint32_t offset = 0; offset < image_size; offset += PAGE_SIZE) {
    paddr_t page = alloc_pages(1);

    size_t remaining = image_size - offset;
    size_t copy_size = PAGE_SIZE <= remaining ? PAGE_SIZE : remaining;

    memcpy((void *)page, image + offset, copy_size);
    map_page(page_table, USER_BASE + offset, page,
             PAGE_U | PAGE_R | PAGE_W | PAGE_X);
  }

  proc->pid = i + 1;
  proc->state = PROC_RUNNABLE;
  proc->sp = (uint32_t)sp;
  proc->page_table = page_table;

  return proc;
}

void delay(void) {
  for (int i = 0; i < 30000000; i++) {
    __asm__ __volatile__("nop");
  }
}

struct process *current_proc;
struct process *idle_proc;

void yield(void) {
  struct process *next = idle_proc;
  for (int i = 0; i < MAX_PROCS; i++) {
    struct process *proc = &procs[(current_proc->pid + i) % MAX_PROCS];
    if (proc->state == PROC_RUNNABLE && proc->pid > 0) {
      next = proc;
      break;
    }
  }

  if (next == current_proc) {
    return;
  }

  __asm__ __volatile__(
      "sfence.vma\n"
      "csrw satp, %[satp]\n"
      "sfence.vma\n"
      "csrw sscratch, %[sscratch]\n"
      :
      : [satp] "r"(SATP_SV32 | ((uint32_t)next->page_table / PAGE_SIZE)),
        [sscratch] "r"((uint32_t)&next->stack[sizeof(next->stack)]));

  struct process *prev = current_proc;
  current_proc = next;

  switch_context(&prev->sp, &next->sp);
}

struct process *proc_a;
struct process *proc_b;

void proc_a_entry(void) {
  printf("starting process A\n");
  while (1) {
    putchar('A');
    yield();
    // delay();
  }
}
void proc_b_entry(void) {
  printf("starting process B\n");
  while (1) {
    putchar('B');
    yield();
    // delay();
  }
}

void kernel_main(unsigned long hart_id, void *dtb) {
  // Clear BSS section
  memset(__bss, 0, (size_t)__bss_end - (size_t)__bss); // Set the bss to 0

  printf("\n\nHello %s\n", "RISC V OS!");
  printf("Heart ID:%d\n", hart_id);
  printf("DTB: %x\n", dtb);

  // Write the stvec register to hold the exception handler address
  WRITE_CSR(stvec, (uint32_t)kernel_entry);
  // __asm__ __volatile__("unimp");
  idle_proc = create_process(NULL, 0);
  idle_proc->pid = 0;
  current_proc = idle_proc;

  create_process(_binary_shell_bin_start, (size_t)_binary_shell_bin_size);
  yield();
  PANIC("switched to idle proc!");

  // printf("1 + 2 = %d, %x\n", 1 + 2, 0x1234abcd);
  // init_memory_manager((struct fdt_header *)dtb);
  // paddr_t paddr0 = alloc_pages(2);
  // paddr_t paddr1 = alloc_pages(1);

  // printf("alloc_pages test: paddr0=%x\n", paddr0);
  // printf("alloc_pages test: paddr1=%x\n", paddr1);

  for (;;) {
    __asm__ __volatile__("wfi");
  }
}

__attribute__((section(".text.boot"))) __attribute__((naked)) void boot(void) {
  __asm__ __volatile__("la sp, __stack_top\n"
                       "j kernel_main\n");
}

void map_page(uint32_t *table1, uint32_t virtual_address,
              paddr_t physical_address, uint32_t flags) {
  if (!is_aligned(virtual_address, PAGE_SIZE)) {
    PANIC("unaligned virtual_address %x", virtual_address);
  }

  if (!is_aligned(physical_address, PAGE_SIZE)) {
    PANIC("unaligned physical_address %x", physical_address);
  }

  uint32_t vpn1 = (virtual_address >> 22) & 0x3ff;

  if ((table1[vpn1] & PAGE_V) == 0) {
    uint32_t pt_address = alloc_pages(1);
    table1[vpn1] = ((pt_address / PAGE_SIZE) << 10) | PAGE_V;
  }

  uint32_t vpn0 = (virtual_address >> 12) & 0x3ff;
  uint32_t *table0 = (uint32_t *)((table1[vpn1] >> 10) * PAGE_SIZE);
  table0[vpn0] = ((physical_address / PAGE_SIZE) << 10) | flags | PAGE_V;
}
