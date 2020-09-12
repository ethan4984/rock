#include <kernel/mm/virtualPageManager.h>
#include <kernel/acpi/rsdp.h>
#include <lib/stringUtils.h>
#include <lib/output.h>

namespace kernel {
    
void acpi_t::rsdpInit(uint64_t *rsdpAddr) {
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

void *acpi_t::findSDT(const char *signature) {
    if(xsdt != NULL) {
        for(uint64_t i = 0; i < (xsdt->acpihdr.length - sizeof(acpihdr_t)); i++) {
            acpihdr_t *acpihdr = (acpihdr_t*)(xsdt->acpiptr[i] + HIGH_VMA);
            if(strncmp(acpihdr->signature, signature, 4) == 0) {
                cout + "[ACPI]" << signature << " located at " << (uint64_t)acpihdr << "\n";
                return (void*)acpihdr;
            }
        }
    } 

    if(rsdt != NULL) {
        for(uint64_t i = 0; i < (rsdt->acpihdr.length - sizeof(acpihdr_t)); i++) {
            acpihdr_t *acpihdr = (acpihdr_t*)((uint64_t)rsdt->acpiptr[i] + HIGH_VMA);
            if(strncmp(acpihdr->signature, signature, 4) == 0) {
                cout + "[ACPI]" << signature << " located at " << (uint64_t)acpihdr << "\n";
                return (void*)acpihdr;
            }
        }
    }

    cout + "[ACPI]" << signature << " cout not be found :(\n";

    return NULL;
}

}
