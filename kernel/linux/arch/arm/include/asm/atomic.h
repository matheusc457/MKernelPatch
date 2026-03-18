/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ARMv7 atomic operations
 * Based on arch/arm/include/asm/atomic.h
 * ARMv7 port by matheusc457 (MPatch/MKernelPatch)
 */
#ifndef __ASM_ARM_ATOMIC_H
#define __ASM_ARM_ATOMIC_H

#include <asm/barrier.h>
#include <linux/compiler.h>

#define ATOMIC_INIT(i) { (i) }

#define atomic_read(v) ACCESS_ONCE((v)->counter)
#define atomic_set(v, i) (((v)->counter) = (i))

#define ATOMIC_OP(op, asm_op)                                      \
    static inline void atomic_##op(int i, atomic_t *v)             \
    {                                                              \
        unsigned long tmp;                                         \
        int result;                                                \
        asm volatile("1: ldrex  %0, [%3]\n"                        \
                     "   " #asm_op " %0, %0, %4\n"                 \
                     "   strex  %1, %0, [%3]\n"                    \
                     "   teq    %1, #0\n"                          \
                     "   bne    1b"                                 \
                     : "=&r"(result), "=&r"(tmp), "+Qo"(v->counter)\
                     : "r"(&v->counter), "Ir"(i)                   \
                     : "cc");                                       \
    }

#define ATOMIC_OP_RETURN(op, asm_op)                               \
    static inline int atomic_##op##_return(int i, atomic_t *v)     \
    {                                                              \
        unsigned long tmp;                                         \
        int result;                                                \
        smp_mb();                                                  \
        asm volatile("1: ldrex  %0, [%3]\n"                        \
                     "   " #asm_op " %0, %0, %4\n"                 \
                     "   strex  %1, %0, [%3]\n"                    \
                     "   teq    %1, #0\n"                          \
                     "   bne    1b"                                 \
                     : "=&r"(result), "=&r"(tmp), "+Qo"(v->counter)\
                     : "r"(&v->counter), "Ir"(i)                   \
                     : "cc");                                       \
        smp_mb();                                                  \
        return result;                                             \
    }

#define ATOMIC_OPS(op, asm_op) \
    ATOMIC_OP(op, asm_op)      \
    ATOMIC_OP_RETURN(op, asm_op)

ATOMIC_OPS(add, add)
ATOMIC_OPS(sub, sub)

#undef ATOMIC_OPS
#undef ATOMIC_OP_RETURN
#undef ATOMIC_OP

static inline int atomic_cmpxchg(atomic_t *ptr, int old, int new)
{
    int oldval;
    unsigned long res;

    smp_mb();
    do {
        asm volatile("@ atomic_cmpxchg\n"
                     "ldrex  %1, [%3]\n"
                     "mov    %0, #0\n"
                     "teq    %1, %4\n"
                     "strexeq %0, %5, [%3]\n"
                     : "=&r"(res), "=&r"(oldval), "+Qo"(ptr->counter)
                     : "r"(&ptr->counter), "Ir"(old), "r"(new)
                     : "cc");
    } while (res);
    smp_mb();
    return oldval;
}

#define atomic_xchg(v, new) (xchg(&((v)->counter), new))

static inline int __atomic_add_unless(atomic_t *v, int a, int u)
{
    int c, old;
    c = atomic_read(v);
    while (c != u && (old = atomic_cmpxchg((v), c, c + a)) != c)
        c = old;
    return c;
}

#define atomic_inc(v) atomic_add(1, v)
#define atomic_dec(v) atomic_sub(1, v)
#define atomic_inc_and_test(v) (atomic_add_return(1, v) == 0)
#define atomic_dec_and_test(v) (atomic_sub_return(1, v) == 0)
#define atomic_inc_return(v) (atomic_add_return(1, v))
#define atomic_dec_return(v) (atomic_sub_return(1, v))
#define atomic_sub_and_test(i, v) (atomic_sub_return(i, v) == 0)
#define atomic_add_negative(i, v) (atomic_add_return(i, v) < 0)

/* ARMv7 does not have native 64-bit atomics - use 32-bit for now */
#define atomic64_t atomic_t
#define atomic64_read atomic_read
#define atomic64_set atomic_set
#define atomic64_add atomic_add
#define atomic64_sub atomic_sub
#define atomic64_inc atomic_inc
#define atomic64_dec atomic_dec

#endif /* __ASM_ARM_ATOMIC_H */
