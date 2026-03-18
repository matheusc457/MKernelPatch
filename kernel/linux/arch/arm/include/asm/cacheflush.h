/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ARMv7 cache flush definitions
 * ARMv7 port by matheusc457 (MPatch/MKernelPatch)
 */
#ifndef __ASM_ARM_CACHEFLUSH_H
#define __ASM_ARM_CACHEFLUSH_H

static inline void flush_icache_range(unsigned long start, unsigned long end)
{
    asm volatile("dsb" : : : "memory");
    asm volatile("mcr p15, 0, %0, c7, c5, 0" : : "r"(0) : "memory");
    asm volatile("dsb" : : : "memory");
    asm volatile("isb" : : : "memory");
}

static inline void flush_dcache_page(void *page)
{
    asm volatile("dsb" : : : "memory");
}

#endif /* __ASM_ARM_CACHEFLUSH_H */
