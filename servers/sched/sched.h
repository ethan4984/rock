#ifndef SCHEDULE_H_
#define SCHEDULE_H_

#include <fayt/rb_tree.h>

#include <portal.h>

struct thread {
	int cid;
	int cgroup;

	int weight;
	int vruntime;
	int runtime;

	RB_META(struct thread);
};

struct sched_descriptor {
	int processor_id; 
	int queue_default_refill;
};

int sched(struct portal_link*, struct sched_descriptor*);

#endif
