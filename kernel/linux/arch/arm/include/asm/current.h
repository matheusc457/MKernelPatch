/* SPDX-License-Identifier: GPL-2.0 */
/*
 * ARMv7 current task definitions
 * ARMv7 port by matheusc457 (MPatch/MKernelPatch)
 */
#ifndef __ASM_ARM_CURRENT_H
#define __ASM_ARM_CURRENT_H

#include <linux/compiler.h>
#include <linux/sched.h>

struct task_struct;
struct thread_info;

extern int thread_info_in_task;

static inline struct thread_info *current_thread_info(void)
{
    struct thread_info *ti;
    asm volatile("mrc p15, 0, %0, c13, c0, 4" : "=r"(ti));
    return ti;
}

static inline struct task_struct *get_current(void)
{
    struct task_struct *task;
    asm volatile("mrc p15, 0, %0, c13, c0, 4" : "=r"(task));
    return task;
}

#define current get_current()

static inline unsigned long *end_of_stack(const struct task_struct *task)
{
    /* ARMv7: task_ext is at end of stack, use stack pointer approach */
    return (unsigned long *)((char *)task + 0x1000); /* approximate, TODO: get real size */
}

static inline struct thread_info *get_task_thread_info(const struct task_struct *task)
{
    return (struct thread_info *)task;
}

#include <patch/include/taskext.h>

static inline struct task_ext *get_task_ext(const struct task_struct *task)
{
    return (struct task_ext *)(end_of_stack(task) + 1);
}

static inline struct task_ext *get_current_task_ext(void)
{
    return get_task_ext(current);
}

#define current_ext get_current_task_ext()

static inline struct task_struct *set_current_task(struct task_struct *task)
{
    return task;
}

static inline struct task_struct *swap_current_task(struct task_struct *task)
{
    return task;
}

static inline const struct task_struct *override_current(struct task_struct *task)
{
    return task;
}

static inline void revert_current(const struct task_struct *old)
{
    (void)old;
}

static inline struct thread_info *current_thread_info_sp(void)
{
    return current_thread_info();
}

#endif /* __ASM_ARM_CURRENT_H */
