#include <acpi.h>
#include <memory.h>
#include <string.h>
#include <shitio.h>
#include <alloc.h>
#include <interrupt.h>

using namespace standardout;

rsdp_t* rsdp;
rsdt_t* rsdt;
xsdt_t* xsdt;

uint8_t checksum()
{
    uint8_t sum = 0;
    for(uint32_t i = 0; i < rsdp->length; i++)
        sum += ((char*)rsdp)[i];
    return sum;
}

void init_acpi() {
    k_print("ACPI init:\n");
    for(uint64_t i = kernel_high + 0xe0000; i < kernel_high + 0x100000; i += 16) {
        if(!strncmp((char*)i, "RSD PTR ", 8)) {
            rsdp = (rsdp_t*)i; /* fill the table */
            k_print("\tRSDP filled at: %x\n", rsdp);

            uint8_t sum = checksum();

            if(rsdp->revision == 2 && rsdp->xsdtAddr) { /* no xsdt on acpi 1.0 */
                xsdt = (xsdt_t*)((size_t)rsdp->xsdtAddr + kernel_high);
                k_print("\tXSDT filled at: %x\n", xsdt);
            }
            else {
                rsdt = (rsdt_t*)((size_t)rsdp->rsdtAddr + kernel_high);
                k_print("\tRSDT filled at: %x\n", rsdt);
            }
            return;
        }
    }
}
