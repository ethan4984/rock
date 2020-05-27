#include <Kernel/sched/thread.h>

thread::thread(uint8_t status_t, uint64_t rbp_t, uint64_t rsp_t, void *main) 
	: status(status_t), rbp(rbp_t), rsp(rsp_t), task(main) 
{
	
}
