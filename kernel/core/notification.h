#ifndef NOTIFICATION_H_
#define NOTIFICATION_H_

#include <fayt/lock.h>

#include <stdbool.h>

#define NOTIFICATION_MAX 64
#define NOTIFICATION_MASK(NOT) (1ull << ((NOT) - 1))

struct notification_action {
	void (*handler)(void*, int, int);
} __attribute__((packed));

struct notification {
	
};

struct notification_queue {
	struct notification queue[NOTIFICATION_MAX];
	int mask;

	bool active;
	int pending; 
	int delivered;

	struct spinlock lock;
};

#endif
