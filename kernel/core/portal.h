#ifndef PORTAL_H_

#define PORTAL_H_

#define PORTAL_REQ_SHARE (1 << 0)
#define PORTAL_REQ_DIRECT (1 << 1)
#define PORTAL_REQ_ANON (1 << 2)
#define PORTAL_REQ_COW (1 << 3)
#define PORTAL_REQ_SP (1 << 4)

#define PORTAL_RESP_FAILURE (1 << 0)
#define PORTAL_RESP_SUCCESS (1 << 1)

#include <fayt/hash.h>
#include <fayt/bst.h>

#include <stdint.h>
#include <stddef.h>

#define PORTAL_PROT_READ (1 << 0)
#define PORTAL_PROT_WRITE (1 << 1)
#define PORTAL_PROT_EXEC (1 << 2)

#define SHARE_MAX_NAME_LENGTH 256

#define PORTAL_SHARE_TYPE_CIRCULAR (1 << 0) 

struct portal_share_meta {
	char lock;
	int type;
	int prot;
	size_t length;
} __attribute__((packed));

struct portal_req {
	int type;
	int prot;
	int length;

	struct __attribute__((packed)) {
		const char *identifier;
		int type;
		int create;
	} share;

	struct __attribute__((packed)) {
		uintptr_t addr;
		size_t length;
		uint64_t paddr[];
	} morphology;
} __attribute__((packed));

struct portal_resp {
	uintptr_t base;
	uint64_t limit;
	uint64_t flags;
} __attribute__((packed));

struct portal {
	int type;
	int prot;

	struct context *context;
	struct page_table *page_table;

	struct {
		const char *identifier;
	} share;

	uintptr_t base;
	uintptr_t limit;
	uint64_t flags;

	struct hash_table *pages;

	struct portal *left;
	struct portal *right;
	struct portal *parent;
};

int portal(struct portal_req *req, struct portal_resp *resp);
int portal_resolve_fault(uintptr_t faulting_address, uint64_t error_code);
void portal_destroy(struct portal *portal);

#endif
