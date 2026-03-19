/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * ARMv7 hook system
 * ARMv7 port by matheusc457 (MPatch/MKernelPatch)
 */

#ifndef __aarch64__

#include <hook.h>
#include <log.h>
#include <cache.h>

#define ARM32_NOP    0xE320F000  /* MOV R0, R0 */
#define ARM32_B      0xEA000000  /* B <offset> */
#define ARM32_BL     0xEB000000  /* BL <offset> */
#define ARM32_LDR_PC 0xE59FF000  /* LDR PC, [PC, #0] */

/* Check if branch is within ±32MB range */
static int can_b_rel_arm(uint32_t src, uint32_t dst)
{
    int32_t offset = (int32_t)(dst - src - 8);
    return offset >= -0x2000000 && offset <= 0x1FFFFFC;
}

/* Generate relative ARM branch */
int32_t branch_relative(uint32_t *buf, uint64_t src_addr, uint64_t dst_addr)
{
    int32_t offset = (int32_t)((uint32_t)dst_addr - (uint32_t)src_addr - 8);
    if (offset < -0x2000000 || offset > 0x1FFFFFC) return 0;
    buf[0] = ARM32_B | (((uint32_t)(offset >> 2)) & 0x00FFFFFF);
    return 1;
}

/* Generate absolute branch via LDR PC */
int32_t branch_absolute(uint32_t *buf, uint64_t addr)
{
    buf[0] = ARM32_LDR_PC; /* LDR PC, [PC, #0] */
    buf[1] = (uint32_t)addr;
    return 2;
}

/* Same as branch_absolute for ARMv7 */
int32_t ret_absolute(uint32_t *buf, uint64_t addr)
{
    return branch_absolute(buf, addr);
}

/* Generate branch from src to dst, choosing best encoding */
int32_t branch_from_to(uint32_t *tramp_buf, uint64_t src_addr, uint64_t dst_addr)
{
    if (can_b_rel_arm((uint32_t)src_addr, (uint32_t)dst_addr))
        return branch_relative(tramp_buf, src_addr, dst_addr);
    return branch_absolute(tramp_buf, dst_addr);
}

/* Resolve branch target (follow B/BL) */
uint64_t branch_func_addr(uint64_t addr)
{
    uint32_t inst = *(uint32_t *)addr;
    /* Check if it's a B instruction */
    if ((inst & 0xFF000000) == ARM32_B) {
        int32_t offset = (int32_t)(inst << 8) >> 6; /* sign extend 24-bit offset, shift left 2 */
        return addr + 8 + offset;
    }
    return addr;
}

#endif /* !__aarch64__ */

#ifndef __aarch64__

#include <hotpatch.h>

/* ARMv7 hook_prepare:
 * Writes LDR PC, [PC, #0] + target_addr at function entry
 * Saves original instructions and creates relocation buffer
 */

/* Relocate a single ARMv7 instruction */
static hook_err_t relocate_inst_arm(hook_t *hook, uint64_t inst_addr, uint32_t inst)
{
    uint32_t *buf = hook->relo_insts + hook->relo_insts_num;

    /* B instruction: 0xEA...... */
    if ((inst & 0xFF000000) == 0xEA000000) {
        int32_t offset = (int32_t)(inst << 8) >> 6;
        uint32_t target = (uint32_t)inst_addr + 8 + offset;
        /* Generate absolute branch to target */
        buf[0] = ARM32_LDR_PC;
        buf[1] = target;
        hook->relo_insts_num += 2;
        return HOOK_NO_ERR;
    }

    /* BL instruction: 0xEB...... */
    if ((inst & 0xFF000000) == 0xEB000000) {
        int32_t offset = (int32_t)(inst << 8) >> 6;
        uint32_t target = (uint32_t)inst_addr + 8 + offset;
        /* Save LR then branch */
        uint32_t next_pc = (uint32_t)hook->relo_addr + (hook->relo_insts_num + 4) * 4;
        buf[0] = 0xE59FE004; /* LDR LR, [PC, #4] - load return addr into LR */
        buf[1] = ARM32_LDR_PC; /* LDR PC, [PC, #0] - branch to target */
        buf[2] = target;
        buf[3] = next_pc; /* return address */
        hook->relo_insts_num += 4;
        return HOOK_NO_ERR;
    }

    /* LDR PC, [PC, #offset] - computed branch */
    if ((inst & 0xFFFF0000) == 0xE59FF000) {
        int32_t offset = inst & 0xFFF;
        uint32_t target_addr = (uint32_t)inst_addr + 8 + offset;
        uint32_t target = *(uint32_t *)target_addr;
        buf[0] = ARM32_LDR_PC;
        buf[1] = target;
        hook->relo_insts_num += 2;
        return HOOK_NO_ERR;
    }

    /* Default: copy instruction as-is */
    buf[0] = inst;
    hook->relo_insts_num += 1;
    return HOOK_NO_ERR;
}

hook_err_t hook_prepare(hook_t *hook)
{
    if (is_bad_address((void *)hook->func_addr)) return -HOOK_BAD_ADDRESS;
    if (is_bad_address((void *)hook->origin_addr)) return -HOOK_BAD_ADDRESS;
    if (is_bad_address((void *)hook->replace_addr)) return -HOOK_BAD_ADDRESS;
    if (is_bad_address((void *)hook->relo_addr)) return -HOOK_BAD_ADDRESS;

    /* Backup original instructions */
    for (int i = 0; i < TRAMPOLINE_MAX_NUM; i++) {
        hook->origin_insts[i] = *((uint32_t *)hook->origin_addr + i);
    }

    /* ARMv7 trampoline: LDR PC, [PC, #0] + target (2 instructions = 8 bytes) */
    hook->tramp_insts[0] = ARM32_LDR_PC;
    hook->tramp_insts[1] = (uint32_t)hook->replace_addr;
    hook->tramp_insts_num = 2;

    /* Fill relo buffer with NOPs */
    for (int i = 0; i < RELOCATE_INST_NUM; i++) {
        hook->relo_insts[i] = ARM32_NOP;
    }

    /* Relocate the 2 instructions we replaced */
    hook->relo_insts_num = 0;
    for (int i = 0; i < hook->tramp_insts_num; i++) {
        uint64_t inst_addr = hook->origin_addr + i * 4;
        uint32_t inst = hook->origin_insts[i];
        hook_err_t err = relocate_inst_arm(hook, inst_addr, inst);
        if (err) return -HOOK_BAD_RELO;
    }

    /* Jump back to original after relocated instructions */
    uint64_t back_src = hook->relo_addr + hook->relo_insts_num * 4;
    uint64_t back_dst = hook->origin_addr + hook->tramp_insts_num * 4;
    uint32_t *buf = hook->relo_insts + hook->relo_insts_num;
    hook->relo_insts_num += branch_from_to(buf, back_src, back_dst);

    return HOOK_NO_ERR;
}

#endif /* !__aarch64__ */

/* ARMv7 transit functions */

typedef uint32_t (*transit0_func_t)();
typedef uint32_t (*transit4_func_t)(uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (*transit8_func_t)(uint32_t, uint32_t, uint32_t, uint32_t,
                                     uint32_t, uint32_t, uint32_t, uint32_t);

uint32_t __attribute__((section(".transit0.text"))) __attribute__((__noinline__)) _transit0()
{
    uint32_t this_va;
    asm volatile("adr %0, ." : "=r"(this_va));
    uint32_t *vptr = (uint32_t *)this_va;
    while (*--vptr != ARM32_NOP);
    vptr--;
    hook_chain_t *hook_chain = local_container_of((uint64_t)vptr, hook_chain_t, transit);
    hook_fargs0_t fargs;
    fargs.skip_origin = 0;
    fargs.chain = hook_chain;
    for (int32_t i = 0; i < hook_chain->chain_items_max; i++) {
        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY) continue;
        hook_chain0_callback func = hook_chain->befores[i];
        if (func) func(&fargs, hook_chain->udata[i]);
    }
    if (!fargs.skip_origin) {
        transit0_func_t origin_func = (transit0_func_t)hook_chain->hook.relo_addr;
        fargs.ret = origin_func();
    }
    for (int32_t i = hook_chain->chain_items_max - 1; i >= 0; i--) {
        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY) continue;
        hook_chain0_callback func = hook_chain->afters[i];
        if (func) func(&fargs, hook_chain->udata[i]);
    }
    return fargs.ret;
}

uint32_t __attribute__((section(".transit4.text"))) __attribute__((__noinline__))
_transit4(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    uint32_t this_va;
    asm volatile("adr %0, ." : "=r"(this_va));
    uint32_t *vptr = (uint32_t *)this_va;
    while (*--vptr != ARM32_NOP);
    vptr--;
    hook_chain_t *hook_chain = local_container_of((uint64_t)vptr, hook_chain_t, transit);
    hook_fargs4_t fargs;
    fargs.skip_origin = 0;
    fargs.chain = hook_chain;
    fargs.args[0] = arg0; fargs.args[1] = arg1;
    fargs.args[2] = arg2; fargs.args[3] = arg3;
    for (int32_t i = 0; i < hook_chain->chain_items_max; i++) {
        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY) continue;
        hook_chain4_callback func = hook_chain->befores[i];
        if (func) func(&fargs, hook_chain->udata[i]);
    }
    if (!fargs.skip_origin) {
        transit4_func_t origin_func = (transit4_func_t)hook_chain->hook.relo_addr;
        fargs.ret = origin_func(fargs.args[0], fargs.args[1], fargs.args[2], fargs.args[3]);
    }
    for (int32_t i = hook_chain->chain_items_max - 1; i >= 0; i--) {
        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY) continue;
        hook_chain4_callback func = hook_chain->afters[i];
        if (func) func(&fargs, hook_chain->udata[i]);
    }
    return fargs.ret;
}

uint32_t __attribute__((section(".transit8.text"))) __attribute__((__noinline__))
_transit8(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3,
          uint32_t arg4, uint32_t arg5, uint32_t arg6, uint32_t arg7)
{
    uint32_t this_va;
    asm volatile("adr %0, ." : "=r"(this_va));
    uint32_t *vptr = (uint32_t *)this_va;
    while (*--vptr != ARM32_NOP);
    vptr--;
    hook_chain_t *hook_chain = local_container_of((uint64_t)vptr, hook_chain_t, transit);
    hook_fargs8_t fargs;
    fargs.skip_origin = 0;
    fargs.chain = hook_chain;
    fargs.args[0] = arg0; fargs.args[1] = arg1;
    fargs.args[2] = arg2; fargs.args[3] = arg3;
    fargs.args[4] = arg4; fargs.args[5] = arg5;
    fargs.args[6] = arg6; fargs.args[7] = arg7;
    for (int32_t i = 0; i < hook_chain->chain_items_max; i++) {
        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY) continue;
        hook_chain8_callback func = hook_chain->befores[i];
        if (func) func(&fargs, hook_chain->udata[i]);
    }
    if (!fargs.skip_origin) {
        transit8_func_t origin_func = (transit8_func_t)hook_chain->hook.relo_addr;
        fargs.ret = origin_func(fargs.args[0], fargs.args[1], fargs.args[2], fargs.args[3],
                                fargs.args[4], fargs.args[5], fargs.args[6], fargs.args[7]);
    }
    for (int32_t i = hook_chain->chain_items_max - 1; i >= 0; i--) {
        if (hook_chain->states[i] != CHAIN_ITEM_STATE_READY) continue;
        hook_chain8_callback func = hook_chain->afters[i];
        if (func) func(&fargs, hook_chain->udata[i]);
    }
    return fargs.ret;
}

extern void _transit0_end();
extern void _transit4_end();
extern void _transit8_end();

hook_err_t hook_chain_prepare(uint32_t *transit, int32_t argno)
{
    uint32_t transit_start, transit_end;
    switch (argno) {
    case 0:
        transit_start = (uint32_t)_transit0;
        transit_end = (uint32_t)_transit0_end;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
        transit_start = (uint32_t)_transit4;
        transit_end = (uint32_t)_transit4_end;
        break;
    default:
        transit_start = (uint32_t)_transit8;
        transit_end = (uint32_t)_transit8_end;
        break;
    }

    int32_t transit_num = (transit_end - transit_start) / 4;
    if (transit_num >= TRANSIT_INST_NUM) return -HOOK_TRANSIT_NO_MEM;

    /* ARMv7: use ARM32_NOP as marker (instead of ARM64_BTI_JC + ARM64_NOP) */
    transit[0] = ARM32_NOP;
    transit[1] = ARM32_NOP;
    for (int i = 0; i < transit_num; i++) {
        transit[i + 2] = ((uint32_t *)transit_start)[i];
    }
    return HOOK_NO_ERR;
}

