#include <process.h>
#include <paging.h>
#include <shitio.h>

#include <stdint.h>
#include <stddef.h>

using namespace standardout;
using namespace MM;

process::process(size_t range) : num_of_blocks(range)
{
    process_begin = (uint32_t*)malloc(range);
    t_print("New process allocated at %x", (uint32_t)process_begin);
}

bool process::null_check()
{
    return (process_begin != NULL) ? true : false;
}

void free_process(process &ref)
{
    if(!ref.null_check()) {
        t_print("Bruh : broken process : NULL");
        return;
    }

    t_print("Process %x freed", (uint32_t)ref.process_begin);
    free(ref.process_begin);
    ref.process_begin = NULL;
}
