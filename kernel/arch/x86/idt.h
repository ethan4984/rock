#ifndef IDT_H_ 
#define IDT_H_

#include <arch/x86/cpu.h>

struct idtr {
	uint16_t limit;
	uint64_t offset;
} __attribute__((packed));

int idt_alloc_vector(void (*handler)(struct registers*, void*), void *ptr);
void idt_init(void);

#endif
