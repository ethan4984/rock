#include <fayt/stream.hpp>
#include <fayt/lock.hpp>
#include <fayt/circular_queue.hpp> 
#include <fayt/notification.hpp>

#include <schedule.hpp>

void NOT_table_push(struct fayt::NotificationInfo*, void*, int) {
	fayt::syscall(fayt::SYSCALL_NOTIFICATION_RETURN);
}

int schedule(struct fayt::PortalResp *portal) {
	if(portal == nullptr) return -1;

	struct fayt::NotificationAction action = {
		.handler = NOT_table_push
	};

	fayt::SyscallRet ret = fayt::syscall(fayt::SYSCALL_NOTIFICATION_ACTION, 1, &action, NULL);
	if(ret.ret == -1) {
		fayt::print("DUFAY: Failed to set notification action\n");
		return -1;
	}

	ret = fayt::syscall(fayt::SYSCALL_NOTIFICATION_UNMUTE);
	if(ret.ret == -1) {
		fayt::print("DUFAY: Failed to activate notification queue\n");
		return -1;
	}

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
