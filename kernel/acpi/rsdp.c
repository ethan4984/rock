#include <acpi/rsdp.h>
#include <output.h>

static rsdp_t *rsdp = NULL;
static rsdt_t *rsdt = NULL;
static xsdt_t *xsdt = NULL;

void rsdp_init(rsdp_t *rsdp_addr) {
    rsdp = rsdp_addr;

    if(rsdp->xsdt_addr) {
        xsdt = (xsdt_t*)((uint64_t)rsdp->xsdt_addr + HIGH_VMA);
        kprintf("[ACPI]", "xsdt found at %x", (uint64_t)xsdt);
        kvprintf("[ACPI] xsdt found at %x\n", (uint64_t)xsdt);
    } else if(rsdp->rsdt_addr) {
        rsdt = (rsdt_t*)((uint64_t)rsdp->rsdt_addr + HIGH_VMA);
        kprintf("[ACPI]", "rsdt found at %x", (uint64_t)rsdt);
        kvprintf("[ACPI] rsdt found at %x\n", (uint64_t)rsdt);
    }
}

void *find_SDT(const char *signature) {
    if(xsdt != NULL) {
        for(uint64_t i = 0; i < (xsdt->acpihdr.length - sizeof(acpihdr_t)); i++) {
            acpihdr_t *acpihdr = (acpihdr_t*)(xsdt->acpiptr[i] + HIGH_VMA);
            if(strncmp(acpihdr->signature, signature, 4) == 0) {
                kprintf("[ACPI]", "%s found", signature);
                kvprintf("[ACPI] %s found\n", signature);
                return acpihdr;
            }
        }
    } 

    if(rsdt != NULL) {
        for(uint64_t i = 0; i < (rsdt->acpihdr.length - sizeof(acpihdr_t)); i++) {
            acpihdr_t *acpihdr = (acpihdr_t*)(rsdt->acpiptr[i] + HIGH_VMA);
            if(strncmp(acpihdr->signature, signature, 4) == 0) {
                kprintf("[ACPI]", "%s found", signature);
                kvprintf("[ACPI] %s found\n", signature);
                return acpihdr;
            }
        }
    } 

    kprintf("[ACPI]", "%s not found", signature);

    return NULL;
}
