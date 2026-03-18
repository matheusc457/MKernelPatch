/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ARMv7 ptrace definitions
 * ARMv7 port by matheusc457 (MPatch/MKernelPatch)
 */
#ifndef __ASM_ARM_PTRACE_H
#define __ASM_ARM_PTRACE_H

#include <ksyms.h>
#include <pgtable.h>
#include <stdbool.h>

/* ARM CPSR bits */
#define PSR_MODE_USR 0x00000010
#define PSR_MODE_FIQ 0x00000011
#define PSR_MODE_IRQ 0x00000012
#define PSR_MODE_SVC 0x00000013
#define PSR_MODE_ABT 0x00000017
#define PSR_MODE_UND 0x0000001B
#define PSR_MODE_SYS 0x0000001F

#define PSR_T_BIT 0x00000020
#define PSR_F_BIT 0x00000040
#define PSR_I_BIT 0x00000080
#define PSR_A_BIT 0x00000100

struct pt_regs {
    union {
        unsigned long uregs[18];
        unsigned long regs[18];
    };
    unsigned long syscallno;
};

#define ARM_cpsr uregs[16]
#define ARM_pc   uregs[15]
#define ARM_lr   uregs[14]
#define ARM_sp   uregs[13]

static inline bool in_syscall(struct pt_regs const *regs)
{
    return false;
}

static inline bool interrupts_enabled(struct pt_regs *regs)
{
    return !(regs->ARM_cpsr & PSR_I_BIT);
}

/* ARMv7 pt_regs variants for compatibility */
struct pt_regs_lt4419 {
    unsigned long uregs[18];
    unsigned long syscallno;
};

struct pt_regs_lt4140 {
    unsigned long uregs[18];
    unsigned long syscallno;
    unsigned long orig_addr_limit;
};

struct pt_regs_lt5100 {
    unsigned long uregs[18];
    unsigned long syscallno;
    unsigned long orig_addr_limit;
    unsigned long pmr_save;
};

#endif /* __ASM_ARM_PTRACE_H */
