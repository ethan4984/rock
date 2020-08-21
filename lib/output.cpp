#include <kernel/sched/smp.h>
#include <lib/stringUtils.h>
#include <lib/asmUtils.h>
#include <lib/output.h>

namespace kernel {

class prefixList_t {
public:
        prefixList_t(const char *prefix, uint8_t prefixColour, uint8_t textColour) : 
            prefix(prefix),
            prefixColour(prefixColour), 
            textColour(textColour) { }
        
        const char *prefix;
        uint8_t prefixColour;
        uint8_t textColour;
};

const char *bashColours[] = {   "\e[39m", "\e[30m", "\e[31m", "\e[32m",
                                "\e[33m", "\e[34m", "\e[35m", "\e[36m",
                                "\e[37m", "\e[90m", "\e[91m", "\e[92m",
                                "\e[93m", "\e[94m", "\e[95m", "\e[96m",
                                "\e[97m"
                            };

prefixList_t bruh("lel", 1, 2);

prefixList_t prefixList[] = {   { "[KDEBUG]", GREEN, YELLOW },
                                { "[KMM]", GREEN, LIGHTRED },
                                { "[ACPI]", MAGENTA, CYAN },
                                { "[APIC]", RED, GREEN },
                                { "[SMP]", YELLOW, LIGHTBLUE },
                                { "[PCI]", BLUE, LIGHTGREEN },
                                { "[AHCI]", LIGHTRED, LIGHTYELLOW },
                                { "[FS]", RED, LIGHTGREEN },
                                { "[NET]", GREEN, YELLOW }
                            };

coutBase &coutBase::operator<<(const char *str) {
    serialWriteString(str); 
    return *this;
}

coutBase &coutBase::operator<<(uint64_t number) {
    char *string = itob(number, 16);
    serialWriteString(string);
    return *this;
}

coutBase &coutBase::operator+(const char *str) {
    static char lock = 0;
    spinLock(&lock);
    for(uint64_t i = 0; i < sizeof(prefixList) / sizeof(prefixList_t); i++) {
        if(strcmp(str, prefixList[i].prefix) == 0) {
            serialWriteString(bashColours[prefixList[i].prefixColour]);
            serialWriteString(str);
            serialWriteString(bashColours[prefixList[i].textColour]);
            serialWrite(' ');
        }
    }
    spinRelease(&lock);
    return *this;
}

}
