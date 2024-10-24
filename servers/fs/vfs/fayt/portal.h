#ifndef FAYT_PORTAL_H_
#define FAYT_PORTAL_H_

#include <fayt/lock.h>

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
	uint32_t magic;

	char data[];
};

constexpr uint32_t LINK_CIRCULAR = 1u << 0;
constexpr uint32_t LINK_VECTOR = 1u << 1;
constexpr uint32_t LINK_RAW = 1u << 2;

constexpr uint32_t LINK_CIRCULAR_MAGIC = 0x9E810F7F;
constexpr uint32_t LINK_VECTOR_MAGIC = 0xEF8647C0;
constexpr uint32_t LINK_RAW_MAGIC = 0xFF6C7D34;

#define LINK_META(LINK) ({ \
	(LINK)->data + header->offset; \
})

#define OPERATE_LINK(LINK, CLASS, OPERATION) ({ \
	__label__ out_ol; \
	int _ret = -1; \
	if((LINK) == NULL) goto out_ol; \
	static_assert((CLASS) == LINK_CIRCULAR || \
		(CLASS) == LINK_VECTOR || (CLASS) == LINK_RAW, "Invalid link class"); \
	if(*(int*)((LINK)->data + (LINK)->header_offset) != ((CLASS) == LINK_CIRCULAR) ? \
		LINK_CIRCULAR_MAGIC : ((CLASS == LINK_VECTOR) ? \
		LINK_VECTOR_MAGIC : (((CLASS) == LINK_RAW) ? LINK_RAW_MAGIC : 0))) goto out_ol; \
	raw_spinlock(&(LINK)->lock); \
	_ret = OPERATION; \
	raw_spinrelease(&(LINK)->lock); \
out_ol: \
	_ret; \
})

struct [[gnu::packed]] portal_req {
	int type;
	int prot;
	int length;

	struct [[gnu::packed]] {
		const char *identifier;
		int type;
		int create;
	} share;

	struct [[gnu::packed]] {
		uintptr_t addr;
		size_t length;
		uint64_t paddr[];
	} morphology;
};

struct [[gnu::packed]] portal_resp {
	uintptr_t base;
	uint64_t limit;
	uint64_t flags;
};

#endif
