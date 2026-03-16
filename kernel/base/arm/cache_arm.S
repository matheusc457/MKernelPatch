/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2023 bmax121. All Rights Reserved.
 * ARMv7 port by matheusc457 (MPatch/MKernelPatch)
 */

#define __ASSEMBLY__
#include "../setup.h"
#include "../start.h"

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
