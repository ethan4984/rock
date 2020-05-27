#include <videoIO.h>
#include <stdarg.h>
#include <stringUtils.h>
#include <ports.h>
#include <stddef.h>

namespace out
{
    void cPrint(const char str[256], ...) /* ouputs to COM1 */
    {
        uint64_t hold = 0;
        char *string;

        va_list arg;
        va_start(arg, str);

        for(uint64_t i = 0; i < strlen(str); i++) {
            if(str[i] != '%')
                serial_write(str[i]);
            else {
                i++;
                switch(str[i]) {
                    case 'd':
                        hold = va_arg(arg, long);
                        string = itob(hold, 10);
                        for(size_t i = 0; i < strlen(string); i++)
                            serial_write(string[i]);
                        break;
                    case 's':
                        string = va_arg(arg, char*);
                        for(size_t i = 0; i < strlen(string); i++)
                            serial_write(string[i]);
                        break;
                    case 'x':
                        hold = va_arg(arg, uint64_t);
                        string = itob(hold, 16);
                        serial_write('0');
                        serial_write('x');
                        for(size_t i = 0; i < strlen(string); i++)
                            serial_write(string[i]);
                        break;
                    case 'a':
                        hold = va_arg(arg, uint64_t);
                        string = itob(hold, 16);
                        serial_write('0');
                        serial_write('x');
                        int offset_zeros = 16 - strlen(string);
                        for(int i = 0; i < offset_zeros; i++)
                            serial_write('0');
                        for(size_t i = 0; i < strlen(string); i++)
                            serial_write(string[i]);
                        break;
                }
            }
        }
        va_end(arg);
        serial_write('\n');
    }
}
