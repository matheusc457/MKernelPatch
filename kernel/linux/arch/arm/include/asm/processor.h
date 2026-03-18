/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ARMv7 processor definitions
 * ARMv7 port by matheusc457 (MPatch/MKernelPatch)
 */
#ifndef __ASM_ARM_PROCESSOR_H
#define __ASM_ARM_PROCESSOR_H

struct task_struct;
#include <asm/ptrace.h>
#include <asm/thread_info.h>

static inline void cpu_relax(void)
{
    asm volatile("yield" : : : "memory");
}

static inline struct pt_regs *task_pt_regs(struct task_struct *task)
{
    return (struct pt_regs *)((unsigned long)task + 0x1000) - 1;
}

static inline unsigned long user_stack_pointer(struct pt_regs *regs)
{
    return regs->uregs[13];
}

#endif /* __ASM_ARM_PROCESSOR_H */
