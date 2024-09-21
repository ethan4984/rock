#include <arch/x86/smp.h>

#include <core/notification.h>
#include <core/syscall.h>
#include <core/scheduler.h>
#include <core/physical.h>
#include <core/debug.h>

#include <fayt/lock.h>
#include <fayt/string.h>
#include <fayt/compiler.h>

static inline int notification_is_valid(int not) {
	if(not < 1 || not > NOTIFICATION_MAX) return -1;
	else return 0;
}

static int get_ucontext(struct context *context, struct ucontext **ucontext) {
	if(context == NULL || ucontext == NULL) return -1;

	*ucontext = NULL;

	for(int i = 0; i < context->notification.ucontexts.length; i++) {
		if(!context->notification.ucontexts.data[i]->active) {
			*ucontext = context->notification.ucontexts.data[i];
			return i;
		}
	}

	*ucontext = alloc(sizeof(struct ucontext));
	VECTOR_PUSH(context->notification.ucontexts, *ucontext);

	return context->notification.ucontexts.length - 1;
}

int notification_dispatch(struct context *context) {
	if(context == NULL) return -1;

	struct notification_queue *queue = context->notification.queue;
	if(unlikely(queue == NULL)) return -1; 

	spinlock(&queue->lock);

	if(!queue->active || !queue->pending) {
		spinrelease(&queue->lock);
		return -1;
	}

	struct ucontext *ucontext;
	context->notification.ucontext_idx = get_ucontext(context, &ucontext); 
	if(context->notification.ucontext_idx == -1 || ucontext == NULL) {
		spinlock(&queue->lock);
		return -1;
	}

	for(int i = 1; i <= NOTIFICATION_MAX; i++) {
		if(queue->pending & NOTIFICATION_MASK(i) == 0 || 
			queue->mask & NOTIFICATION_MASK(i)) continue;

		struct notification *notification = NULL;
		struct notification_action *action = &context->notification.actions[i - 1];

		if(notification == NULL || action == NULL) continue;

		struct ustack *ustack = context->notification.stacks;
		for(; ustack;) {
			if(!ustack->active) break;
			else ustack = ustack->next;
		}
		if(ustack == NULL) return 0;

		ucontext->stack = ustack;

		ucontext->regs.ss = 0x3b;
		ucontext->regs.rsp = ucontext->stack->user_stack.sp;
		ucontext->regs.rflags = 0x202;
		ucontext->regs.cs = 0x43;
		ucontext->regs.rip = (uintptr_t)action->handler;

		ucontext->active = 1;

		return 0;
	}

	return 0;
}

SYSCALL_DEFINE3(notification_action, int, not, struct notification_action *, action,
	struct notification_action *, old, {
	struct context *context = CORE_LOCAL->current_context; 

	if(unlikely(context == NULL)) return -1;
	if(unlikely(notification_is_valid(not) == -1)) return -1;

	spinlock(&context->notification.lock);

	struct notification_action *current_action = &context->notification.actions[not];
	
	if(old) {
		*old = *current_action;	
	}

	if(action) {
		*current_action = *action;
	}

	spinrelease(&context->notification.lock);
})

SYSCALL_DEFINE2(notification_define_stack, void *, sp, size_t, size, {
	struct context *context = CORE_LOCAL->current_context;
	if(unlikely(context == NULL)) return -1;

	struct ustack *new_stack = alloc(sizeof(struct ustack));

	new_stack->user_stack.sp = (uintptr_t)sp;
	new_stack->user_stack.size = size;
	new_stack->kernel_stack.sp = (uintptr_t)pmm_alloc(DIV_ROUNDUP(CONTEXT_DEFAULT_STACK_SIZE,
		PAGE_SIZE), 1) + CONTEXT_DEFAULT_STACK_SIZE + HIGH_VMA;
	new_stack->kernel_stack.size = CONTEXT_DEFAULT_STACK_SIZE;
	new_stack->active = 0;
	new_stack->next = context->notification.stacks;

	context->notification.stacks = new_stack;
})

SYSCALL_DEFINE0(notification_return, {

})
