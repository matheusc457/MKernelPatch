/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ARMv7 ELF definitions
 * ARMv7 port by matheusc457 (MPatch/MKernelPatch)
 */
#ifndef __ASM_ARM_ELF_H
#define __ASM_ARM_ELF_H

#include <asm/hwcap.h>

typedef unsigned long elf_greg_t;
typedef unsigned long elf_gregset_t[18];
typedef unsigned long elf_fpregset_t[64];

#define EM_ARM 40
#define ELF_CLASS ELFCLASS32
#define ELF_DATA ELFDATA2LSB
#define ELF_ARCH EM_ARM

/* ARM relocation types */
#define R_ARM_NONE          0
#define R_ARM_ABS32         2
#define R_ARM_CALL          28
#define R_ARM_JUMP24        29
#define R_ARM_THM_CALL      10
#define R_ARM_THM_JUMP24    30
#define R_ARM_MOVW_ABS_NC   43
#define R_ARM_MOVT_ABS      44
#define R_ARM_PREL31        42
#define R_ARM_REL32         3

#endif /* __ASM_ARM_ELF_H */
