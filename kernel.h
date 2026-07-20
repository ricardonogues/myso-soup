#pragma once
#include "common.h"
#include "memory.h"

#define SATP_SV32 (1u << 31)
#define PAGE_V (1 << 0) // Valid bit - entry is enabled
#define PAGE_R (1 << 1) // Readable
#define PAGE_W (1 << 2) // Writable
#define PAGE_X (1 << 3) // Executable
#define PAGE_U (1 << 4) // User (accessible in user mode)

#define USER_BASE 0x1000000
#define SSTATUS_SPIE (1 << 5)

#define SCAUSE_ECALL 8

#define PANIC(fmt, ...)                                                        \
  do {                                                                         \
    printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);      \
    while (1) {                                                                \
    }                                                                          \
  } while (0)

#define READ_CSR(reg)                                                          \
  ({                                                                           \
    unsigned long __tmp;                                                       \
    __asm__ __volatile__("csrr %0," #reg : "=r"(__tmp));                       \
    __tmp;                                                                     \
  })

#define WRITE_CSR(reg, value)                                                  \
  do {                                                                         \
    uint32_t __tmp = (value);                                                  \
    __asm__ __volatile__("csrw " #reg ", %0" ::"r"(__tmp));                    \
    __tmp;                                                                     \
  } while (0)

#define MAX_PROCS 8

#define PROC_UNUSED 0
#define PROC_RUNNABLE 1
#define PROC_EXITED 2

#define SECTOR_SIZE 512
#define VIRTQ_ENTRY_NUM 16
#define VIRTIO_DEVICE_BLK 2
#define VIRTIO_BLK_PADDR 0x10001000
#define VIRTIO_REG_MAGIC 0x00
#define VIRTIO_REG_VERSION 0x04
#define VIRTIO_REG_DEVICE_ID 0x08
#define VIRTIO_REG_PAGE_SIZE 0x28
#define VIRTIO_REG_QUEUE_SEL 0x30
#define VIRTIO_REG_QUEUE_NUM_MAX 0x34
#define VIRTIO_REG_QUEUE_NUM 0x38
#define VIRTIO_REG_QUEUE_PFN 0x40
#define VIRTIO_REG_QUEUE_READY 0x44
#define VIRTIO_REG_QUEUE_NOTIFY 0x50
#define VIRTIO_REG_DEVICE_STATUS 0x70
#define VIRTIO_REG_DEVICE_CONFIG 0x100
#define VIRTIO_STATUS_ACK 1
#define VIRTIO_STATUS_DRIVER 2
#define VIRTIO_STATUS_DRIVER_OK 4
#define VIRTQ_DESC_F_NEXT 1
#define VIRTQ_DESC_F_WRITE 2
#define VIRTQ_AVAIL_F_NO_INTERRUPT 1
#define VIRTIO_BLK_T_IN 0
#define VIRTIO_BLK_T_OUT 1

struct virtq_desc {
  uint64_t addr;
  uint32_t len;
  uint16_t flags;
  uint16_t next;
} __attribute((packed));

struct virtq_avail {
  uint16_t flags;
  uint16_t index;
  uint16_t ring[VIRTQ_ENTRY_NUM];
} __attribute__((packed));

struct virtq_used_elem {
  uint32_t id;
  uint32_t len;
} __attribute__((packed));

struct virtq_used {
  uint16_t flags;
  uint16_t index;
  struct virtq_used_elem ring[VIRTQ_ENTRY_NUM];
} __attribute__((packed));

struct virtio_virtq {
  struct virtq_desc descs[VIRTQ_ENTRY_NUM];
  struct virtq_avail avail;
  struct virtq_used used __attribute__((aligned(PAGE_SIZE)));
  int queue_index;
  volatile uint16_t *used_index;
  uint16_t last_used_index;
} __attribute__((packed));

struct virtio_blk_req {
  uint32_t type;
  uint32_t reserved;
  uint64_t sector;
  uint8_t data[512];
  uint8_t status;
} __attribute__((packed));

struct process {
  int pid;
  int state;
  vaddr_t sp;
  uint32_t *page_table;
  uint8_t stack[8192];
};

void map_page(uint32_t *table1, uint32_t virtual_address,
              paddr_t physical_address, uint32_t flags);
void putchar(char ch);
long getchar(void);
void yield(void);
