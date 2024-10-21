#ifndef PORTAL_H_
#define PORTAL_H_

#include <fayt/hash.h>
#include <fayt/bst.h>

#include <stdint.h>
#include <stddef.h>

constexpr uint32_t PORTAL_REQ_SHARE = 1u << 0;
constexpr uint32_t PORTAL_REQ_DIRECT = 1u << 1;
constexpr uint32_t PORTAL_REQ_ANON = 1u << 2;
constexpr uint32_t PORTAL_REQ_COW = 1u << 3;
constexpr uint32_t PORTAL_REQ_SP = 1u << 4;

constexpr uint32_t PORTAL_RESP_FAILURE = 1u << 0;
constexpr uint32_t PORTAL_RESP_SUCCESS = 1u << 1;

constexpr uint32_t PORTAL_PROT_READ = 1u << 0;
constexpr uint32_t PORTAL_PROT_WRITE = 1u << 1;
constexpr uint32_t PORTAL_PROT_EXEC = 1u << 2;

struct [[gnu::packed]] portal_link {
	char lock;

	int length;
	int header_offset;
	int header_limit;
	int data_offset;
	int data_limit;

	char data[];
};

constexpr uint32_t LINK_CIRCULAR = 1u << 0;
constexpr uint32_t LINK_VECTOR = 1u << 1;
constexpr uint32_t LINK_RAW = 1u << 2;

constexpr uint32_t LINK_CIRCULAR_MAGIC = 0x9E810F7F;
constexpr uint32_t LINK_VECTOR_MAGIC = 0xEF8647C0;
constexpr uint32_t LINK_RAW_MAGIC = 0xFF6C7D34;

#define SHARE_MAX_NAME_LENGTH 256

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
