#include <kernel/sched/smp.h>
#include <lib/stringUtils.h>
#include <lib/asmUtils.h>
#include <lib/output.h>
#include <stdarg.h>

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


void kprintDS(const char *prefix, const char *str, ...) { // debug serial
    va_list arg;
    va_start(arg, str);

    for(uint64_t i = 0; i < sizeof(prefixList) / sizeof(prefixList_t); i++) {
        if(strcmp(prefixList[i].prefix, prefix) == 0) {
            serialWriteString(bashColours[prefixList[i].prefixColour]);
            serialWriteString(prefix);
            serialWriteString(bashColours[prefixList[i].textColour]);
            serialWrite(' ');
            break;
        }

        if(i == sizeof(prefixList) / sizeof(prefixList_t) - 1) {
            serialWriteString(bashColours[DEFAULT]);
        }
    }

    printArgs(str, arg, serialWrite);

    serialWrite('\n'); 
}

void printArgs(const char *str, va_list arg, function<void, uint8_t> handler) {
    uint64_t hold = 0;
    char *string;
    char character;
	
    for(uint64_t i = 0; i < strlen(str); i++) {
        if(str[i] != '%')
            handler(str[i]); 
        else {
            switch(str[++i]) {
                case 'd':
                    hold = va_arg(arg, long);
                    string = itob(hold, 10);
                    for(uint64_t i = 0; i < strlen(string); i++)
                        handler(string[i]);
                    break;
                case 's':
                    string = va_arg(arg, char*);
                    for(uint64_t i = 0; i < strlen(string); i++)
                        handler(string[i]);
                    break;
                case 'c':
                    character = va_arg(arg, int);
                    handler(character);
                    break; 
                case 'x':
                    hold = va_arg(arg, uint64_t);
                    string = itob(hold, 16);
                    for(uint64_t i = 0; i < strlen(string); i++)
                        handler(string[i]);
                    break;
                case 'a':
                    hold = va_arg(arg, uint64_t);
                    string = itob(hold, 16);
                    int offset_zeros = 16 - strlen(string);
                    for(int i = 0; i < offset_zeros; i++)
                        handler('0');
                    for(uint64_t i = 0; i < strlen(string); i++)
                        handler(string[i]);
                    break;
            }
        }
    }
    va_end(arg);
}

}
