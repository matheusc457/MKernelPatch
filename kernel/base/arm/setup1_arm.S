/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 bmax121. All Rights Reserved.
 * ARMv7 port by matheusc457 (MPatch/MKernelPatch)
 */

#define __ASSEMBLY__
#include "../setup.h"
#include "../start.h"

.text
.arm
.syntax unified

@ memcpy8 - already in cache_arm.S
@ rmemcpy32 - already in cache_arm.S

.align 2
_sp_pc_anchor:
    .word _sp_pc_anchor
.type start_prepare, %function
start_prepare:
    push {r4-r11, lr}
    @ kernel_pa
    mov r4, r0

    @ Calculate VA->PA offset (we are running before MMU is active)
    @ Use PC-relative trick: adr gives physical address, ldr= gives virtual
    adr r5, _sp_pc_anchor      @ r5 = physical address of anchor
    ldr r6, =_sp_pc_anchor     @ r6 = virtual address of anchor
    sub r6, r6, r5             @ r6 = VA - PA = voffset

    @ map_data (convert VA to PA)
    ldr r9, =map_data
    sub r9, r9, r6

    @ setup_preset (convert VA to PA)
    ldr r10, =setup_preset
    sub r10, r10, r6

    @ start_preset (convert VA to PA)
    ldr r11, =start_preset
    sub r11, r11, r6

    @ header (convert VA to PA)
    ldr r12, =header
    sub r12, r12, r6

    @ start_preset.kernel_version = setup_preset.kernel_version
    ldr r3, [r10, #setup_kernel_version_offset]
    str r3, [r11, #start_kernel_version_offset]

    @ start_preset.kallsyms_lookup_name_offset = setup_preset.kallsyms_lookup_name_offset
    ldr r3, [r10, #setup_kallsyms_lookup_name_offset_offset]
    str r3, [r11, #start_kallsyms_lookup_name_offset_offset]

    @ start_preset.kernel_size = setup_preset.kernel_size
    ldr r3, [r10, #setup_kernel_size_offset]
    str r3, [r11, #start_kernel_size_offset]

    @ start_preset.start_offset = setup_preset.start_offset
    ldr r3, [r10, #setup_start_offset_offset]
    str r3, [r11, #start_start_offset_offset]
    mov r5, r3

    @ start_preset.extra_size = setup_preset.extra_size
    ldr r3, [r10, #setup_extra_size_offset]
    str r3, [r11, #start_extra_size_offset]

    @ start_preset.kernel_pa = kernel_pa
    str r4, [r11, #start_kernel_pa_offset]

    @ start_preset.map_offset = setup_preset.map_offset
    ldr r3, [r10, #setup_map_offset_offset]
    str r3, [r11, #start_map_offset_offset]
    mov r6, r3

    @ memcpy(&start_preset.setup, &setup_header, KP_HEADER_SIZE)
    add r0, r11, #start_header_offset
    add r1, r12, #0
    mov r2, #KP_HEADER_SIZE
    bl memcpy8

    @ memcpy(start_preset.superkey, setup_preset.superkey, SUPER_KEY_LEN)
    add r0, r11, #start_superkey_offset
    add r1, r10, #setup_superkey_offset
    mov r2, #SUPER_KEY_LEN
    bl memcpy8

    @ memcpy(start_preset.root_superkey, setup_preset.root_superkey, ROOT_SUPER_KEY_HASH_LEN)
    add r0, r11, #start_root_superkey_offset
    add r1, r10, #setup_root_superkey_offset
    mov r2, #ROOT_SUPER_KEY_HASH_LEN
    bl memcpy8

    @ memcpy(&start_preset.patch_config, &setup_preset.patch_config, PATCH_CONFIG_LEN)
    add r0, r11, #start_patch_config_offset
    add r1, r10, #setup_patch_config_offset
    mov r2, #PATCH_CONFIG_LEN
    bl memcpy8

    @ backup map area
    ldr r3, =_map_end
    ldr r7, =_map_start
    sub r2, r3, r7

    @ start_preset.map_backup_len
    str r2, [r11, #start_map_backup_len_offset]
    add r0, r11, #start_map_backup_offset
    add r1, r4, r6
    bl memcpy8

    @ start_img_size = setup_preset.kpimg_size - (_kp_start - _link_base)
    ldr r8, [r10, #setup_kpimg_size_offset]
    ldr r3, =_kp_start
    ldr r7, =_link_base
    sub r3, r3, r7
    sub r8, r8, r3

    @ map_data.start_img_size = start_img_size
    str r8, [r9, #map_start_img_size_offset]

    @ memcpy(kernel_pa + start_offset, _kp_start, start_img_size + extra_size)
    add r0, r4, r5
    ldr r1, =_kp_start
    ldr r2, [r10, #setup_extra_size_offset]
    add r2, r2, r8
    bl rmemcpy32

    pop {r4-r11, lr}
    bx lr

.align 2
.type map_prepare, %function
map_prepare:
    push {r4-r8, lr}
    mov r4, r0

    @ map_data
    ldr r9, =map_data
    @ setup_preset
    ldr r10, =setup_preset

    @ map_data.kernel_pa = kernel_pa
    str r4, [r9, #map_kernel_pa_offset]

    @ map_data.map_offset = setup_preset.map_offset
    ldr r5, [r10, #setup_map_offset_offset]
    str r5, [r9, #map_map_offset_offset]

    @ map_data.paging_init_relo = setup_preset.paging_init_offset
    ldr r6, [r10, #setup_paging_init_offset_offset]
    str r6, [r9, #map_paging_init_relo_offset]

    @ map_data.map_symbol = setup_preset.map_symbol
    add r0, r9, #map_map_symbol_offset
    add r1, r10, #setup_map_symbol_offset
    mov r2, #MAP_SYMBOL_SIZE
    bl memcpy8

    @ map_data.start_offset = setup_preset.start_offset
    ldr r7, [r10, #setup_start_offset_offset]
    str r7, [r9, #map_start_offset_offset]

    @ map_data.start_size = _kp_end - _kp_start
    ldr r7, =_kp_end
    ldr r8, =_kp_start
    sub r7, r7, r8
    str r7, [r9, #map_start_size_offset]

    @ map_data.extra_size = setup_preset.extra_size
    ldr r7, [r10, #setup_extra_size_offset]
    str r7, [r9, #map_extra_size_offset]

    @ map_data.alloc_size = HOOK_ALLOC_SIZE + MEMORY_ROX_SIZE + MEMORY_RW_SIZE
    mov r7, #HOOK_ALLOC_SIZE
    add r7, r7, #MEMORY_ROX_SIZE
    add r7, r7, #MEMORY_RW_SIZE
    str r7, [r9, #map_alloc_size_offset]

    @ paging_init_pa = paging_init_offset + kernel_pa
    add r7, r6, r4
    @ map_data.paging_init_backup = *(uint32_t *)paging_init_pa
    ldr r8, [r7]
    str r8, [r9, #map_paging_init_backup_offset]
    dsb

    @ replace_offset = (_paging_init - _map_start) + map_offset
    ldr r7, =_paging_init
    ldr r8, =_map_start
    sub r7, r7, r8
    add r7, r7, r5

    @ paging_init_pa = paging_init_offset + kernel_pa
    add r8, r6, r4

    @ B_REL(paging_init_offset, replace_offset)
    @ r7 = replace_offset (dst), r6 = paging_init_offset (src), r8 = paging_init_pa
    @ branch_insn = 0xEA000000 | (((dst - src) >> 2) & 0x00FFFFFF)
    sub r7, r7, r6      @ offset = dst - src
    lsr r7, r7, #2      @ offset >>= 2
    and r7, r7, #0x00FFFFFF  @ mask to 24 bits
    movw r6, #0x0000
    movt r6, #0xEA00    @ r6 = 0xEA000000 (ARM B instruction)
    orr r7, r7, r6      @ combine opcode and offset
    str r7, [r8]        @ write branch instruction to paging_init_pa

    @ relocate map
    ldr r2, =_map_end
    ldr r1, =_map_start
    sub r2, r2, r1
    add r0, r4, r5
    bl memcpy8

    dsb
    pop {r4-r8, lr}
    bx lr

.align 2
.type setup, %function
setup:
    @ Write to ramoops
    push {r0, r1}
    ldr r0, =0x47d6f000
    mov r1, #0x02
    str r1, [r0]           @ KP02 - entered setup
    pop {r0, r1}
    push {r0-r12, lr}

    @ _link_base - use ADR for PC-relative (physical) address
    adr r9, _setup_link_base_ref
    ldr r9, [r9]              @ r9 = virtual address of _link_base
    @ Now calculate the PA->VA offset using current PC
    adr r12, _setup_pc_ref    @ r12 = current physical PC
    ldr r11, =_setup_pc_ref   @ r11 = virtual address of same location
    sub r11, r11, r12         @ r11 = VA - PA = voffset
    sub r9, r9, r11           @ r9 = physical address of _link_base

    @ setup_preset - convert VA to PA
    ldr r10, =setup_preset
    sub r10, r10, r11         @ r10 = physical address of setup_preset
    mov r8, r10

    @ kernel_pa = _link_base - setup_preset.setup_offset
    ldr r11, [r10, #setup_setup_offset_offset]
    sub r12, r9, r11
    mov r7, r12

    mov r0, r7
    bl start_prepare
    mov r0, r7
    bl map_prepare

    @ memcpy(kernel_pa, setup_preset.header_backup, HDR_BACKUP_SIZE)
    mov r0, r7
    add r1, r8, #setup_header_backup_offset
    mov r2, #HDR_BACKUP_SIZE
    bl memcpy8

    @ flush I-cache
    dsb
    mcr p15, 0, r0, c7, c5, 0  @ invalidate I-cache
    dsb
    isb

    mov r6, r7

    pop {r0-r12, lr}
    bx r6

.section .entry.text, "ax"
.global setup_entry
.type setup_entry, %function
setup_entry:
    @ Write to ramoops region - survives reboot
    ldr r12, =0x47d6f000
    mov r11, #0x01
    str r11, [r12]         @ KP01 - entered setup_entry

    @ r0 = physical address to the FDT blob
    @ Calculate physical address of stack using PC-relative addressing
    mov r9, sp
    adr r10, _setup_entry_base      @ get current physical PC
    ldr r11, =_setup_entry_base     @ get linked virtual address
    sub r10, r11, r10               @ r10 = VA - PA = virt offset
    ldr r11, =stack                 @ virtual address of stack
    sub r11, r11, r10               @ convert to physical address
    add r11, r11, #STACK_SIZE
    mov sp, r11
    push {r9, r10}
    b setup

_setup_entry_base:
.align 2
_setup_pc_ref:
    .word _setup_pc_ref
_setup_link_base_ref:
    .word _link_base
