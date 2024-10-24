#ifndef SCHEDULE_H_
#define SCHEDULE_H_

#include <arch/x86/cpu.h>
#include <fayt/vector.h>
#include <fayt/lock.h>

#define CONTEXT_DEFAULT_STACK_SIZE 0x10000
#define SCHEDULER_DEFAULT_QUEUE_SIZE 0x10000

struct stack {
	uintptr_t sp;
	size_t size;
	int flags;
};

struct ustack {
	struct stack kernel_stack;
	struct stack user_stack;

	int active;

	struct ustack *next;
};

struct ucontext {
	struct ustack *stack;

	struct registers regs;
	void *fpu_context;

	int active;
};

struct context {
	struct spinlock lock;

	uintptr_t user_gs_base;
	uintptr_t user_fs_base;

	struct stack kernel_stack;
	struct stack user_stack;

	struct {
		struct ustack *stacks;

		struct notification_action *actions;
		struct notification_queue *queue;

		VECTOR(struct ucontext*) ucontexts;
		struct ucontext *ucontext;
		int ucontext_idx;

		struct spinlock lock;
	} notification;

	struct {
		const char *namespace;
		const char *server;
		int cid;
	} comms;

	uint64_t sysperm;

	struct page_table *page_table;

	struct registers regs;
	void *fpu_context;
};

#define yield() __asm__("int $32");

void reschedule(struct registers*, void*);

int create_blank_context(struct context*);
int sched_establish_shared_link(struct context*, const char*);

#endif
