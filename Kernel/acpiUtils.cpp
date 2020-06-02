#include <Kernel/acpiUtils.h>
#include <Slib/ports.h>
#include <Slib/memoryUtils.h>
#include <Slib/stringUtils.h>
#include <Slib/videoIO.h>

#include <stddef.h>

using namespace out;

rsdp_t *rsdp = NULL;
rsdt_t *rsdt = NULL;
xsdt_t *xsdt = NULL;

void initAcpi()
{
    for(uint64_t i = 0xe0000; i < 0x100000; i += 16) { /* search for the SDT PTR */
        if(!strncmp((char*)i, "RSD PTR ", 8)) {
            rsdp = (rsdp_t*)i;

            if(rsdp->xsdtAddr) { /* if xsdt exists */
                xsdt = (xsdt_t*)((uint64_t)rsdp->xsdtAddr);
                kPrint("\nXSDT found at %x\n", xsdt);
                break;
            } 
            
            if(rsdp->rsdtAddr) { /* acpi 1.0 only */
                rsdt = (rsdt_t*)((uint64_t)rsdp->rsdtAddr);
                kPrint("\nRSDT found at %x\n", rsdt);
                break;
            } else { 
                kPrint("\nWe got a problem, RSDT nor XSDT were found");
                return;
            }
        }
    }
}

void *findSDT(const char *signature)
{
    if(xsdt != NULL) { /* acpi version >1.0? */
        for(uint64_t i = 0; i < (rsdt->ACPIheader.length - sizeof(ACPIheader_t)) / 4; i++) {
            ACPIheader_t *header = (ACPIheader_t*)xsdt->ACPIptr[i];
            if(!strncmp(header->signature, signature, 4)) {
                kPrint("%s found at %x\n", signature, header);
                return (void*)header;
            }
        }
    }

    if(rsdt != NULL) { 
        for(uint64_t i = 0; i < (rsdt->ACPIheader.length - sizeof(ACPIheader_t)) / 4; i++) {
            ACPIheader_t *header = (ACPIheader_t*)rsdt->ACPIptr[i];
            if(!strncmp(header->signature, signature, 4)) {
                kPrint("%s found at %x\n", signature, header);
                return (void*)header;
            }
        }
    }

    kPrint("%s not found\n", signature);
    return NULL;
}
