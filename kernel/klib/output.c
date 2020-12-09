#include <output.h>
#include <graphics.h>
#include <memutils.h>

static uint32_t x_pos = 0, y_pos = 0;

typedef struct {
    const char *prefix;
    uint8_t prefix_colour;
    uint8_t text_colour;
} prefix_t;

static prefix_t prefix_list[] = { { "[KDEBUG]", GREEN, YELLOW },
                                  { "[KMM]", GREEN, LIGHTRED },
                                  { "[ACPI]", MAGENTA, CYAN },
                                  { "[APIC]", RED, GREEN },
                                  { "[SMP]", YELLOW, LIGHTBLUE },
                                  { "[PCI]", BLUE, LIGHTGREEN },
                                  { "[AHCI]", LIGHTRED, LIGHTYELLOW },
                                  { "[FS]", RED, LIGHTGREEN },
                                  { "[NET]", GREEN, YELLOW }
                                };

const char *bash_colours[] = {  "\e[39m", "\e[30m", "\e[31m", "\e[32m",
                                "\e[33m", "\e[34m", "\e[35m", "\e[36m",
                                "\e[37m", "\e[90m", "\e[91m", "\e[92m",
                                "\e[93m", "\e[94m", "\e[95m", "\e[96m",
                                "\e[97m"
                            };

void kprintf(const char *prefix, const char *str, ...) {
    va_list arg;
    va_start(arg, str);

    for(uint64_t i = 0; i < sizeof(prefix_list) / sizeof(prefix_t); i++) {
        if(strcmp(prefix_list[i].prefix, prefix) == 0) {
            serial_write_str(bash_colours[prefix_list[i].prefix_colour]);
            serial_write_str(prefix);
            serial_write_str(bash_colours[prefix_list[i].text_colour]);
            serial_write(' ');
            break;
        }

        if(i == sizeof(prefix_list) / sizeof(prefix_t) - 1) {
            serial_write_str(bash_colours[DEFAULT]);
        }
    }

    print_args(str, arg, serial_write);

    serial_write('\n'); 
}

void kvprintf(const char *str, ...) {
    va_list arg;
    va_start(arg, str);

    print_args(str, arg, g_putchar);
}

void kpanic(const char *message, ...) {
    serial_write_str("kowalski analysis:\nPANIC: ");
    
    va_list arg;
    va_start(arg, message);

    print_args(message, arg, serial_write);

    for(;;);
}

void print_args(const char *str, va_list arg, void (*handler)(uint8_t)) {
    uint64_t hold = 0;
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
                    for(uint64_t i = 0; i < strlen(string); i++) {
                        handler(string[i]);
                    }
                    break;
                case 's':
                    string = va_arg(arg, char*);
                    for(uint64_t i = 0; i < strlen(string); i++) {
                        handler(string[i]);
                    }
                    break;
                case 'c':
                    character = va_arg(arg, int);
                    handler(character);
                    break; 
                case 'x':
                    hold = va_arg(arg, uint64_t);
                    string = itob(hold, 16);
                    for(uint64_t i = 0; i < strlen(string); i++) {
                        handler(string[i]);
                    }
                    break;
                case 'b':
                    hold = va_arg(arg, uint64_t);
                    string = itob(hold, 2);
                    for(uint64_t i = 0; i < strlen(string); i++) {
                        handler(string[i]);
                    }
                    break;
                case 'a':
                    hold = va_arg(arg, uint64_t);
                    string = itob(hold, 16);
                    int offset_zeros = 16 - strlen(string);
                    for(int i = 0; i < offset_zeros; i++) {
                        handler('0');
                    }
                    for(uint64_t i = 0; i < strlen(string); i++) {
                        handler(string[i]);
                    }
                    break;
            }
        }
    }
    va_end(arg);
}

void g_putchar(uint8_t c) {
    switch(c) {
        case '\n':
            if(y_pos + 16 == fb_height) {
                scroll(BACKGROUND_COLOUR, 16);
            } else {
                y_pos += 16;
            }

            x_pos = 0;
            break;
        case '\t':
            for(int i = 0; i < TAB_SIZE; i++) 
                g_putchar(' ');
            break;
        default:
            render_char(x_pos, y_pos, FOREGROUND_COLOUR, c);
            if(x_pos + 8 > fb_width)
                g_putchar('\n');
            else
                x_pos += 8; 
    }
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

void stacktrace(uint64_t *rbp) {
    for(;;) {
        uint64_t old_frame = *rbp;
        uint64_t ret_addr = rbp[1];

        if(!ret_addr)
            break;

        kvprintf("Stacktrace: %x\n", ret_addr);

        if(!old_frame)
            break;

        rbp = (uint64_t*)old_frame;
    }
}
