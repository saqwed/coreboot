/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef THREAD_H_
#define THREAD_H_

#include <stdint.h>
#include <bootstate.h>
#include <arch/cpu.h>

#if ENV_RAMSTAGE && CONFIG(COOP_MULTITASKING)

struct thread {
	int id;
	uintptr_t stack_current;
	uintptr_t stack_orig;
	struct thread *next;
	void (*entry)(void *);
	void *entry_arg;
	int can_yield;
};

void threads_initialize(void);
/* Get the base of the thread stacks.
 * Returns pointer to CONFIG_NUM_THREADS*CONFIG_STACK_SIZE contiguous bytes
 * aligned to CONFIG_STACK_SIZE, or NULL.
 */
void *arch_get_thread_stackbase(void);
/* Run func(arrg) on a new thread. Return 0 on successful start of thread, < 0
 * when thread could not be started. Note that the thread will block the
 * current state in the boot state machine until it is complete. */
int thread_run(void (*func)(void *), void *arg);
/* thread_run_until is the same as thread_run() except that it blocks state
 * transitions from occurring in the (state, seq) pair of the boot state
 * machine. */
int thread_run_until(void (*func)(void *), void *arg,
		     boot_state_t state, boot_state_sequence_t seq);
/* Return 0 on successful yield for the given amount of time, < 0 when thread
 * did not yield. */
int thread_yield_microseconds(unsigned int microsecs);

/* Allow and prevent thread cooperation on current running thread. By default
 * all threads are marked to be cooperative. That means a thread can yield
 * to another thread at a pre-determined switch point. Current there is
 * only a single place where switching may occur: a call to udelay(). */
void thread_cooperate(void);
void thread_prevent_coop(void);

static inline void thread_init_cpu_info_non_bsp(struct cpu_info *ci)
{
	ci->thread = NULL;
}

/* Architecture specific thread functions. */
asmlinkage void switch_to_thread(uintptr_t new_stack, uintptr_t *saved_stack);
/* Set up the stack frame for a new thread so that a switch_to_thread() call
 * will enter the thread_entry() function with arg as a parameter. The
 * saved_stack field in the struct thread needs to be updated accordingly. */
void arch_prepare_thread(struct thread *t,
			 asmlinkage void (*thread_entry)(void *), void *arg);
#else
static inline void threads_initialize(void) {}
static inline int thread_run(void (*func)(void *), void *arg) { return -1; }
static inline int thread_run_until(void (*func)(void *), void *arg, boot_state_t state,
				   boot_state_sequence_t seq)
{
	return -1;
}
static inline int thread_yield_microseconds(unsigned int microsecs)
{
	return -1;
}
static inline void thread_cooperate(void) {}
static inline void thread_prevent_coop(void) {}
struct cpu_info;
static inline void thread_init_cpu_info_non_bsp(struct cpu_info *ci) { }
#endif

#endif /* THREAD_H_ */
