#include <arch/x86/cpu.h> 
#include <arch/x86/idt.h>
#include <arch/x86/gdt.h>
#include <arch/x86/apic.h>
#include <arch/x86/hpet.h>
#include <arch/x86/smp.h>
#include <arch/x86/debug.h>

#include <core/debug.h>

uint64_t HIGH_VMA = 0xffff800000000000;

extern void syscall_main(void);

struct cpuid_state cpuid(size_t leaf, size_t subleaf) {
	struct cpuid_state ret = { .leaf = leaf, subleaf = subleaf };

	size_t max;
	__asm__ volatile ("cpuid" : "=a"(max) : "a"(leaf & 0x80000000) : "rbx", "rcx", "rdx");

	if(leaf > max) {
		return ret;
	}

	__asm__ volatile ("cpuid" : "=a"(ret.rax), "=b"(ret.rbx), "=c"(ret.rcx), "=d"(ret.rbx) : "a"(leaf), "c"(subleaf));

	return ret;
}

void x86_system_init(void) {
	wrmsr(MSR_EFER, rdmsr(MSR_EFER) | (1 << 0) | (1 << 11)); // set SCE and NX
	wrmsr(MSR_STAR, 0x33ull << 48 | 0x28ull << 32);
	wrmsr(MSR_LSTAR, (uintptr_t)syscall_main);
	wrmsr(MSR_SFMASK, ~(uint32_t)2);

	uint64_t cr0;
	__asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));

	cr0 &= ~(1 << 2); // disable x87 emulation
	cr0 |= (1 << 1); // enables sse;

	__asm__ volatile ("mov %0, %%cr0" :: "r"(cr0));

	uint64_t cr4;
	__asm__ volatile ("mov %%cr4, %0" : "=r"(cr4));

	cr4 |=	(1 << 7) | // allow for global pages
			(1 << 9) | // enables xsave/xstore
			(1 << 10); // enables XM exceptions
											
	__asm__ volatile ("mov %0, %%cr4" :: "r"(cr4));

	serial_init();
}

void x86_fpu_init(struct cpu_local *cpu_local) {
	// TODO check cpuid for fpu capabilities

	cpu_local->fpu_context_size = 512;
	cpu_local->fpu_save = fxsave;
	cpu_local->fpu_rstor = fxrstor;
}

void x86_system_tables(void) {
	gdt_init();
	idt_init();
	hpet_init();
	apic_init();
	apic_timer_init(20);
	boot_aps();
}
