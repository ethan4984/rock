#pragma once

void *memset(void *src, int val, unsigned int how_many);

void grab_reg();

struct reg_dumps {
	unsigned int eax;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;

	unsigned int esp;
	unsigned int ebp;

	unsigned int edi;
	unsigned int esi;

	unsigned int cr0;
	unsigned int cr1;
	unsigned int cr2;
};

