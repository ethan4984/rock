#include <kernel/acpi/madt.h>
#include <kernel/sched/smp.h>
#include <kernel/mm/kHeap.h>

namespace kernel {

void initSMP() {
    cpuInfo = new cpuInfo_t[madtInfo.madtEntry0Count * sizeof(cpuInfo_t)];
}

}
