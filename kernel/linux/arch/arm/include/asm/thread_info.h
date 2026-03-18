/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ARMv7 thread_info definitions
 * ARMv7 port by matheusc457 (MPatch/MKernelPatch)
 */
#ifndef __ASM_ARM_THREAD_INFO_H
#define __ASM_ARM_THREAD_INFO_H

struct thread_info {
    unsigned long flags;
    int preempt_count;
    unsigned long addr_limit;
    struct task_struct *task;
};



#define _TIF_SECCOMP (1 << 8)
#define TIF_SECCOMP 8

#define THREAD_SIZE 8192
extern int thread_size;

static inline void *task_stack_page(const struct task_struct *task)
{
    return (void *)((unsigned long)task & ~(THREAD_SIZE - 1));
}

#endif /* __ASM_ARM_THREAD_INFO_H */
