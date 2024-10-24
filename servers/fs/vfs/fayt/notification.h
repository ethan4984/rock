#ifndef FAYT_NOTIFICATION_H_
#define FAYT_NOTIFICATION_H_

constexpr int INSTANTANEOUS = 1 << 1;

struct comm_bridge {
	int not;

	int cid;
	int cgroup;
	int weight;

	struct {
		void *ptr;
		int length;
	} data;

	const char *namespace;
	const char *destination;
};

struct notification_info {

}; 

struct notification_action {
	void (*handler)(struct notification_info*, void*, int);
};

#endif
