/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ARMv7 barrier operations
 * ARMv7 port by matheusc457 (MPatch/MKernelPatch)
 */
#ifndef __ASM_ARM_BARRIER_H
#define __ASM_ARM_BARRIER_H

#define smp_mb() asm volatile("dmb ish" : : : "memory")
#define smp_rmb() asm volatile("dmb ishld" : : : "memory")
#define smp_wmb() asm volatile("dmb ishst" : : : "memory")
#define mb() asm volatile("dmb sy" : : : "memory")
#define rmb() asm volatile("dmb ld" : : : "memory")
#define wmb() asm volatile("dmb st" : : : "memory")
#define isb() asm volatile("isb" : : : "memory")
#define dsb(opt) asm volatile("dsb " #opt : : : "memory")

static inline void __read_once_size(const volatile void *p, void *res, int size)
{
    switch (size) {
    case 1: *(unsigned char *)res = *(volatile unsigned char *)p; break;
    case 2: *(unsigned short *)res = *(volatile unsigned short *)p; break;
    case 4: *(unsigned int *)res = *(volatile unsigned int *)p; break;
    default: __builtin_memcpy(res, (const void *)p, size);
    }
}

static inline void __write_once_size(volatile void *p, void *res, int size)
{
    switch (size) {
    case 1: *(volatile unsigned char *)p = *(unsigned char *)res; break;
    case 2: *(volatile unsigned short *)p = *(unsigned short *)res; break;
    case 4: *(volatile unsigned int *)p = *(unsigned int *)res; break;
    default: __builtin_memcpy((void *)p, res, size);
    }
}

#endif /* __ASM_ARM_BARRIER_H */

#define smp_store_release(p, v) \
    do { smp_mb(); *(p) = (v); } while (0)

#define smp_load_acquire(p) \
    ({ typeof(*p) ___p = *(p); smp_mb(); ___p; })
