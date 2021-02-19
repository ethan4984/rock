#include <acpi/rsdp.h>
#include <debug.h>

static struct rsdp *rsdp = NULL;
static struct rsdt *rsdt = NULL;
static struct xsdt *xsdt = NULL;

void rsdp_init(struct rsdp *rsdp_addr) {
    rsdp = rsdp_addr;

    if(rsdp->xsdt_addr) {
        xsdt = (struct xsdt*)((uint64_t)rsdp->xsdt_addr + HIGH_VMA);
        kprintf("[ACPI] xsdt found at %x\n", (uint64_t)xsdt);
    } else if(rsdp->rsdt_addr) {
        rsdt = (struct rsdt*)((uint64_t)rsdp->rsdt_addr + HIGH_VMA);
        kprintf("[ACPI] rsdt found at %x\n", (uint64_t)rsdt);
    }
}

void *find_SDT(const char *signature) {
    if(xsdt != NULL) {
        for(uint64_t i = 0; i < (xsdt->acpihdr.length - sizeof(struct acpihdr)); i++) {
            struct acpihdr *acpihdr = (struct acpihdr*)(xsdt->acpiptr[i] + HIGH_VMA);
            if(strncmp(acpihdr->signature, signature, 4) == 0) {
                kprintf("[ACPI] %s found\n", signature);
                return acpihdr;
            }
        }
    } 

    if(rsdt != NULL) {
        for(uint64_t i = 0; i < (rsdt->acpihdr.length - sizeof(struct acpihdr)); i++) {
            struct acpihdr *acpihdr = (struct acpihdr*)(rsdt->acpiptr[i] + HIGH_VMA);
            if(strncmp(acpihdr->signature, signature, 4) == 0) {
                kprintf("[ACPI] %s found\n", signature);
                return acpihdr;
            }
        }
    } 

    kprintf("[ACPI] %s not found\n", signature);

    return NULL;
}
