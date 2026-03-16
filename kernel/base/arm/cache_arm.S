/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 bmax121. All Rights Reserved.
 * ARMv7 port by matheusc457 (MPatch/MKernelPatch)
 */

#define __ASSEMBLY__

#define ASM_NL ;
#define ALIGN .align 2
#define DMA_BIDIRECTIONAL 0
#define DMA_TO_DEVICE 1
#define DMA_FROM_DEVICE 2
#define DMA_NONE 3

#define ENTRY(name) \
    .globl name ASM_NL \
    ALIGN ASM_NL \
    name:

#define END(name) \
    .size name, .-name

#define ENDPROC(name) \
    .type name, %function ASM_NL \
    END(name)

.text
.thumb
.syntax unified

.align 2
.type memcpy8, %function
memcpy8:
    cmp r2, #0
    ble .l8end
.l8loop:
    ldrb r3, [r1], #1
    strb r3, [r0], #1
    subs r2, r2, #1
    bne .l8loop
.l8end:
    bx lr

.align 2
.type rmemcpy32, %function
rmemcpy32:
    subs r2, r2, #4
    blt .r32end
.r32loop:
    ldr r3, [r1, r2]
    str r3, [r0, r2]
    subs r2, r2, #4
    bge .r32loop
.r32end:
    bx lr

ENTRY(flush_cache_all)
    mov r12, lr
    bl __flush_dcache_all
    mov r0, #0
    mcr p15, 0, r0, c7, c1, 0  @ invalidate I+BTB cache
    bx r12
ENDPROC(flush_cache_all)

ENTRY(__flush_dcache_all)
    stmfd sp!, {r4-r11, lr}
    mrc p15, 1, r0, c0, c0, 1   @ read CLIDR
    ands r3, r0, #0x7000000
    mov r3, r3, lsr #23         @ cache level value (two bits per cache)
    beq .finished
    mov r10, #0                  @ start clean at cache level 0
.loop1:
    add r2, r10, r10, lsr #1    @ work out 3x current cache level
    mov r1, r0, lsr r2          @ extract cache type bits from CLIDR
    and r1, r1, #7              @ mask bits for current cache only
    cmp r1, #2                  @ see what cache we have at this level
    blt .skip                   @ skip if no cache, or just i-cache
    mcr p15, 2, r10, c0, c0, 0  @ select current cache level in CSSELR
    isb
    mrc p15, 1, r1, c0, c0, 0  @ read the new CCSIDR
    and r2, r1, #7              @ extract the length of the cache lines
    add r2, r2, #4              @ add 4 (line length offset)
    movw r4, #0x3ff
    and r4, r4, r1, lsr #3     @ find maximum number on the way size
    clz r5, r4                  @ find bit position of way size increment
    movw r7, #0x7fff
    and r7, r7, r1, lsr #13    @ extract max number of the index size
.loop2:
    mov r9, r4                  @ create working copy of max way size
.loop3:
    lsl r6, r9, r5
    orr r11, r10, r6            @ factor way and cache number into r11
    lsl r6, r7, r2
    orr r11, r11, r6            @ factor index number into r11
    mcr p15, 0, r11, c7, c14, 2 @ clean & invalidate by set/way
    subs r9, r9, #1             @ decrement the way
    bge .loop3
    subs r7, r7, #1             @ decrement the index
    bge .loop2
.skip:
    add r10, r10, #2            @ increment cache number
    cmp r3, r10
    bgt .loop1
.finished:
    mov r10, #0
    mcr p15, 2, r10, c0, c0, 0  @ switch back to cache level 0
    dsb
    isb
    ldmfd sp!, {r4-r11, lr}
    bx lr
ENDPROC(__flush_dcache_all)

ENTRY(flush_icache_range)
ENTRY(__flush_cache_user_range)
    stmfd sp!, {r4-r5, lr}
    mrc p15, 0, r4, c0, c0, 1  @ read CTR
    and r4, r4, #0xf            @ D cache line size
    mov r5, #4
    lsl r4, r5, r4              @ actual cache line size
    sub r5, r4, #1
    bic r2, r0, r5              @ start of D cache line
1:
    mcr p15, 0, r2, c7, c11, 1 @ clean D line to PoU
    add r2, r2, r4
    cmp r2, r1
    blo 1b
    dsb

    mrc p15, 0, r4, c0, c0, 1  @ read CTR
    and r4, r4, #0xf            @ I cache line size
    mov r5, #4
    lsl r4, r5, r4              @ actual cache line size
    sub r5, r4, #1
    bic r2, r0, r5              @ start of I cache line
2:
    mcr p15, 0, r2, c7, c5, 1  @ invalidate I line PoU
    add r2, r2, r4
    cmp r2, r1
    blo 2b
    dsb
    isb
    ldmfd sp!, {r4-r5, lr}
    bx lr
ENDPROC(flush_icache_range)
ENDPROC(__flush_cache_user_range)

ENTRY(__flush_dcache_area)
    stmfd sp!, {r4, lr}
    mrc p15, 0, r4, c0, c0, 1  @ read CTR
    lsr r4, r4, #16
    and r4, r4, #0xf            @ D cache line size
    mov r2, #4
    lsl r4, r2, r4              @ actual cache line size
    add r1, r0, r1
    sub r2, r4, #1
    bic r0, r0, r2
1:
    mcr p15, 0, r0, c7, c14, 1 @ clean & invalidate D line
    add r0, r0, r4
    cmp r0, r1
    blo 1b
    dsb
    ldmfd sp!, {r4, lr}
    bx lr
ENDPROC(__flush_dcache_area)

ENTRY(__dma_flush_range)
    stmfd sp!, {r4, lr}
    mrc p15, 0, r4, c0, c0, 1  @ read CTR
    lsr r4, r4, #16
    and r4, r4, #0xf
    mov r2, #4
    lsl r4, r2, r4              @ actual cache line size
    sub r2, r4, #1
    bic r0, r0, r2
1:
    mcr p15, 0, r0, c7, c14, 1 @ clean & invalidate D line
    add r0, r0, r4
    cmp r0, r1
    blo 1b
    dsb
    ldmfd sp!, {r4, lr}
    bx lr
ENDPROC(__dma_flush_range)

ENTRY(__dma_map_area)
    add r1, r1, r0
    cmp r2, #DMA_FROM_DEVICE
    beq __dma_inv_range
    b __dma_clean_range
ENDPROC(__dma_map_area)

ENTRY(__dma_unmap_area)
    add r1, r1, r0
    cmp r2, #DMA_TO_DEVICE
    bne __dma_inv_range
    bx lr
ENDPROC(__dma_unmap_area)
