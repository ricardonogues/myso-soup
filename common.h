#pragma once

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef uint32_t size_t;
typedef uint32_t paddr_t;
typedef uint32_t vaddr_t;

#define true 1
#define false 0
#define NULL ((void *)0)
#define align_up(value, align) ((((value) + (align) - 1)) & ~((align) - 1))
#define is_aligned(value, align) (((value) & ((align) - 1)) == 0)
/*
 * offsetof(type, member) - compute byte offset of `member` within `type`
 *
 * Trick: treat address 0 as if a `type` object lived there, then take the
 * address of `member` on it. A pointer is just an integer address under the
 * hood, so &(((type*)0)->member) evaluates to (0 + member's offset), i.e.
 * the offset itself, just wearing a pointer type. We never dereference
 * (no read of address 0 ever happens - only its address is taken), so no
 * real memory access occurs; the compiler resolves it purely from the
 * struct's known layout at compile time. The final (size_t) cast just
 * relabels that value as a plain integer instead of a pointer.
 */
#define offsetof(type, member) ((size_t)&(((type *)0)->member))
#define SYS_PUTCHAR 1

void *memset(void *buffer, char c, size_t n);
void *memcpy(void *destination, const void *source, size_t n);
void *strcpy(char *destination, const char *source);
int strlen(const char *source);
int strcmp(const char *s1, const char *s2);
void printf(const char *fmt, ...);

uint32_t bswap32(uint32_t value);
uint64_t bswap64(uint64_t value);
