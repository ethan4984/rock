#include <fayt/syscall.hpp>
#include <fayt/stream.hpp>
#include <fayt/lock.hpp>
#include <fayt/portal.hpp>
#include <fayt/slab.hpp> 
#include <fayt/allocator.hpp>

#include <schedule.hpp>

using namespace fayt;

int main(void) {
	fayt::print("DUFAY: Scheduling server booting...\n"); 

	fayt::PageFrameAllocator allocator(0xA0000000, 0x0000FFFFFFFFF0FF);
	fayt::CacheDirectory<fayt::PageFrameAllocator> cacheDirectory(&allocator);

	fayt::print("DUFAY: Slab cache dircetory initialised\n");

	uintptr_t addr = allocator.AllocAddress(0x10000);

	PortalReq portalReq = {
		.type = PORTAL_REQ_SHARE | PORTAL_REQ_ANON,
		.prot = PORTAL_PROT_READ | PORTAL_PROT_WRITE,
		.length = sizeof(PortalReq),
		.share = {
			.identifier = "SCHEDULER CORE0",
			.type = PORTAL_SHARE_TYPE_CIRCULAR,
			.create = 0
		},
		.morphology = {
			.addr = addr,
			.length = 0x10000 
		}
	};
	
	int ret = -1;
	PortalResp portalResp;
	SyscallRet portalRet = syscall(SYSCALL_PORTAL, &portalReq, &portalResp);

	if(portalRet.ret == -1) { 
		fayt::print("DUFAY: Scheduler: Failed to establish link with kernel\n");
		goto failure;
	}

	fayt::print("DUFAY: Scheduler: Has establish link with kernel successfully\n");

	ret = schedule(&portalResp);
	if(ret == -1) {
		fayt::print("DUFAY: Scheduler: Internal critical failure\n");
		goto failure;
	}

failure:
	for(;;);
}
