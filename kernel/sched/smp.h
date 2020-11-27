#ifndef SMP_H_
#define SMP_H_

#include <int/apic.h>

#include <mm/vmm.h>
#include <mm/pmm.h>

void init_smp();

void spin_lock(char *lock);

void spin_release(char *lock);

#endif
