#include <arch/x86/paging.h>
#include <arch/x86/cpu.h>
#include <arch/x86/apic.h> 
#include <arch/x86/smp.h>

#include <core/scheduler.h>
#include <core/virtual.h>
#include <core/physical.h>
#include <core/server.h>
#include <core/syscall.h>

#include <core/debug.h>
#include <fayt/lock.h>
#include <fayt/string.h>
#include <fayt/compiler.h>

static struct spinlock reschedule_lock;

int create_blank_context(struct context *context) { 
	if(unlikely(context == NULL)) return -1;

	context->page_table = alloc(sizeof(struct page_table));
	vmm_default_table(context->page_table);

	context->sysperm = 0;

	context->kernel_stack.sp = pmm_alloc(DIV_ROUNDUP(CONTEXT_DEFAULT_STACK_SIZE, PAGE_SIZE), 1)
		+ CONTEXT_DEFAULT_STACK_SIZE + HIGH_VMA;
	context->kernel_stack.size = CONTEXT_DEFAULT_STACK_SIZE;

	context->signal_kernel_stack.sp = pmm_alloc(DIV_ROUNDUP(CONTEXT_DEFAULT_STACK_SIZE, PAGE_SIZE), 1)
		+ CONTEXT_DEFAULT_STACK_SIZE + HIGH_VMA;
	context->signal_kernel_stack.size = CONTEXT_DEFAULT_STACK_SIZE;

	context->fpu_context = alloc(CORE_LOCAL->fpu_context_size);

	return 0;
}

int sched_establish_shared_link(struct context *scheduler_context, const char *identifier)  {
	size_t page_cnt = DIV_ROUNDUP(SCHEDULER_DEFAULT_QUEUE_SIZE, PAGE_SIZE);
	uint64_t physical_base = pmm_alloc(page_cnt, 1);
	uint64_t virtual_base = physical_base + HIGH_VMA;

	struct portal_resp resp;
	struct portal_req *req = alloc(sizeof(struct portal_req) + sizeof(uint64_t) * page_cnt);

	*req = (struct portal_req) {
		.type = PORTAL_REQ_SHARE | PORTAL_REQ_DIRECT, 
		.prot = PORTAL_PROT_READ | PORTAL_PROT_WRITE,
		.length = sizeof(struct portal_req) + sizeof(uint64_t) * page_cnt,
		.share = {
			.identifier = identifier,
			.type = PORTAL_SHARE_TYPE_CIRCULAR, 
			.create = 1
		}
	};

	req->morphology.addr = virtual_base;
	req->morphology.length = page_cnt * PAGE_SIZE;
	for(int i = 0; i < page_cnt; i++, physical_base += PAGE_SIZE) req->morphology.paddr[i] = physical_base;

	int ret = portal(req, &resp);
	if(ret == -1) return -1;

	free(req);

	return 0;
}

SYSCALL_DEFINE0(yield, {
	asm volatile ("int $32");
})

void reschedule(struct registers *regs, void *) {
	spinlock(&reschedule_lock);

	struct context *next_context;

	int ret = VECTOR_POP(CORE_LOCAL->thread_queue, next_context);
	if(ret == -1) {
		struct server *scheduling_server = CORE_LOCAL->scheduling_server;
		if(scheduling_server == NULL) {
			spinrelease(&reschedule_lock);
			return;
		}

		next_context = scheduling_server->context;

		if(next_context == NULL) {
			spinrelease(&reschedule_lock);
			return;
		}
	}

	struct context *current_context = CORE_LOCAL->current_context;
	if(likely(current_context)) {
		current_context->regs = *regs;
		CORE_LOCAL->fpu_save(current_context->fpu_context);
		current_context->user_fs_base = get_user_fs();
		current_context->user_gs_base = get_user_gs();
		current_context->user_stack.sp = CORE_LOCAL->user_stack;
	}

	x86_swap_tables(next_context->page_table);

	CORE_LOCAL->fpu_rstor(next_context->fpu_context);

	set_user_fs(next_context->user_fs_base);
	set_user_gs(next_context->user_gs_base);

	CORE_LOCAL->current_context = next_context;

	xapic_write(XAPIC_EOI_OFF, 0);
	spinrelease(&reschedule_lock);

	SWAP_TLS(&next_context->regs);

	asm volatile (
		"mov %0, %%rsp\n\t"
		"pop %%r15\n\t"
		"pop %%r14\n\t"
		"pop %%r13\n\t"
		"pop %%r12\n\t"
		"pop %%r11\n\t"
		"pop %%r10\n\t"
		"pop %%r9\n\t"
		"pop %%r8\n\t"
		"pop %%rsi\n\t"
		"pop %%rdi\n\t"
		"pop %%rbp\n\t"
		"pop %%rdx\n\t"
		"pop %%rcx\n\t"
		"pop %%rbx\n\t"
		"pop %%rax\n\t"
		"addq $16, %%rsp\n\t"
		"iretq\n\t"
		:: "r" (&next_context->regs)
	);
}
