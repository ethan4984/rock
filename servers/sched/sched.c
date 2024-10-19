#include <fayt/syscall.h>
#include <fayt/lock.h>
#include <fayt/debug.h>
#include <fayt/notification.h>
#include <fayt/circular_queue.h>

#include <sched.h>

static struct thread *thread_tree;

static void notify_enqueue_thread(struct notification_info *, void*, int) {
	SYSCALL0(SYSCALL_NOTIFICATION_RETURN);
}

static void notify_dequeue_thread(struct notification_info *, void*, int) {
	SYSCALL0(SYSCALL_NOTIFICATION_RETURN);
}

static int traverse_and_queue(struct thread **thread) {
	if(thread == NULL) return -1;
	
	struct thread *root = thread_tree;
	struct thread *enqueue;
	for(enqueue = root; root; enqueue = root) {
		if(root->left && root->right) {
			if(root->left->vruntime > root->right->vruntime) root = root->right;
			else root = root->left;
		} else if(root->left) root = root->left;
		else if(root->right) root = root->right;
		else break;
	}

	*thread = enqueue;

	return 0;
}

int sched(struct portal_link *link, struct sched_descriptor *desc) {
	if(link == NULL) return -1;

	struct notification_action enqueue_action =
		{ .handler = notify_enqueue_thread };
	struct notification_action dequeue_action =
		{ .handler = notify_dequeue_thread };

	struct syscall_response response = SYSCALL3(SYSCALL_NOTIFICATION_ACTION, 1, &enqueue_action, NULL);
	if(response.ret == -1) { print("DUFAY: SCHEDULER: Failure to set notification\n"); return -1; }

	response = SYSCALL3(SYSCALL_NOTIFICATION_ACTION, 1, &dequeue_action, NULL);
	if(response.ret == -1) { print("DUFAY: SCHEDULER: Failure to set notification\n"); return -1; }

	print("DUFAY: SCHEDULER: Initialised enqueue and dequeue notifications\n");

	response = SYSCALL0(SYSCALL_NOTIFICATION_UNMUTE);
	if(response.ret == -1) { print("DUFAY: SCHEDULER: Failed to activate notification queue\n"); return -1; }

	for(;;) {
		for(int i = 0; i < desc->queue_default_refill; i++) {
			int ret = OPERATE_LINK(link, LINK_CIRCULAR,
				({
					struct thread *thread;
					int ret = traverse_and_queue(&thread);
					if(ret != -1) ret = circular_queue_push((void*)link->data, thread);
					ret;
				})
			);

			if(ret == -1) {
				print("DUFAY: SCHEDULER: Failued to push onto the share queue\n");
				return -1;
			}
		}

		SYSCALL0(SYSCALL_YIELD);
	}
}

