#include <kernel/mm/virtualPageManager.h>
#include <kernel/acpi/rsdp.h>
#include <lib/stringUtils.h>
#include <lib/output.h>

namespace acpi {

void rsdpInit(uint64_t *rsdpAddr) {
    rsdp = (rsdp_t*)rsdpAddr;
    if(rsdp->xsdtAddr) {
        xsdt = (xsdt_t*)((uint64_t)rsdp->xsdtAddr);
        cout + "[ACPI]" << "xsdt located at " << (uint64_t)xsdt << "\n";
        return;
    }

    if(rsdp->rsdtAddr) {
        rsdt = (rsdt_t*)((uint64_t)rsdp->rsdtAddr + HIGH_VMA);
        cout + "[ACPI]" << "rsdt located at " << (uint64_t)rsdt << "\n";
        return;
    }
}

}
