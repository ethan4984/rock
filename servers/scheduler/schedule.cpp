#include <fayt/stream.hpp>
#include <fayt/lock.hpp>
#include <fayt/circular_queue.hpp> 

#include <schedule.hpp>

int schedule(struct fayt::PortalResp *portal) {
	if(portal == nullptr) return -1;

	fayt::PortalShareMeta *portalShareMeta = reinterpret_cast<struct fayt::PortalShareMeta*>(portal->base);
	fayt::CircularQueue *circularQueue = reinterpret_cast<struct fayt::CircularQueue*>(portal->base +
		sizeof(fayt::PortalShareMeta));

	fayt::Spinlock queueLock(&portalShareMeta->lock);

	for(;;) {
		fayt::SpinlockGuard guard(queueLock);
		fayt::syscall(fayt::SYSCALL_YIELD);
	}

	return 0;
}
