#ifndef RSDT_HPP_
#define RSDT_HPP_

#include <acpi/tables.hpp>
#include <mm/vmm.hpp>
#include <memutils.hpp>
#include <debug.hpp>

namespace acpi {

inline rsdp *rsdp_ptr = NULL;
inline rsdt *rsdt_ptr = NULL;
inline xsdt *xsdt_ptr = NULL;

template <typename T>
T *find_SDT(const char *signature) {
    if(xsdt_ptr != NULL) {
        for(size_t i = 0; i < (xsdt_ptr->hdr_ptr.length - sizeof(hdr)); i++) {
            hdr *hdr_ptr = reinterpret_cast<hdr*>(xsdt_ptr->acpiptr[i] + vmm::high_vma);
            if(strncmp(hdr_ptr->signature, signature, 4) == 0) {
                print("[ACPI] {s} found\n", signature);
                return reinterpret_cast<T*>(hdr_ptr);
            }
        }
    } 

    if(rsdt_ptr != NULL) {
        for(size_t i = 0; i < (rsdt_ptr->hdr_ptr.length - sizeof(hdr)); i++) {
            hdr *hdr_ptr = reinterpret_cast<hdr*>(rsdt_ptr->acpiptr[i] + vmm::high_vma);
            if(strncmp(hdr_ptr->signature, signature, 4) == 0) {
                print("[ACPI] {s} found\n", signature);
                return reinterpret_cast<T*>(hdr_ptr);
            }
        }
    }

    print("[ACPI] {s} not found\n", signature);

    return NULL;
}

}

#endif
