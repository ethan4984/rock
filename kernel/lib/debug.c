#include <sched/smp.h>
#include <debug.h>

void kprintf(const char *str, ...) {
    va_list arg;
    va_start(arg, str);

    print_args(str, arg, serial_write);
}

void print_args(const char *str, va_list arg, void (*handler)(uint8_t)) {
    size_t hold = 0;
    char *string;
    char character;
	
    for(uint64_t i = 0; i < strlen(str); i++) {
        if(str[i] != '%') {
            handler(str[i]);
        } else {
            switch(str[++i]) {
                case 'd':
                    hold = va_arg(arg, long);
                    string = itob(hold, 10);
                    for(size_t i = 0; i < strlen(string); i++) {
                        handler(string[i]);
                    }
                    break;
                case 's':
                    string = va_arg(arg, char*);
                    for(size_t i = 0; i < strlen(string); i++) {
                        handler(string[i]);
                    }
                    break;
                case 'c':
                    character = va_arg(arg, int);
                    handler(character);
                    break; 
                case 'x':
                    hold = va_arg(arg, size_t);
                    string = itob(hold, 16);
                    for(size_t i = 0; i < strlen(string); i++) {
                        handler(string[i]);
                    }
                    break;
                case 'b':
                    hold = va_arg(arg, size_t);
                    string = itob(hold, 2);
                    for(size_t i = 0; i < strlen(string); i++) {
                        handler(string[i]);
                    }
                    break;
            }
        }
    }
    va_end(arg);
}

static char *sprintf_buf;
static size_t sprintf_loc = 0;

static void sprintf_handler(uint8_t c) {
    sprintf_buf[sprintf_loc++] = c;
}

void sprintf(char *buf, const char *str, int null_term, ...) {
    static char lock = 0;
    spin_lock(&lock);

    va_list arg;
    va_start(arg, null_term);

    sprintf_buf = buf;
    sprintf_loc = 0;

    print_args(str, arg, sprintf_handler);

    if(null_term)
        sprintf_handler('\0');

    spin_release(&lock);
}

uint8_t serial_read() {
    while((inb(COM1 + 5) & 1) == 0);
    return inb(COM1);
}

void serial_write(uint8_t data) {
    while((inb(COM1 + 5) & 0x20) == 0);
    outb(COM1, data);
}

void serial_write_str(const char *str) {
    for(; *str != '\0'; str++)
        serial_write(*str);
}

void syscall_log(struct regs *regs) {
    kprintf("%s\n", (void*)regs->rdi);
}


void stacktrace(uint64_t *rbp) {
    for(;;) {
        uint64_t old_frame = *rbp;
        uint64_t ret_addr = rbp[1];

        if(!ret_addr)
            break;

        kprintf("[stacktrace] %x\n", ret_addr);

        if(!old_frame)
            break;

        rbp = (uint64_t*)old_frame;
    }
}
