#include <arch/x86/paging.h>
#include <arch/x86/smp.h> 

#include <core/server.h>
#include <core/virtual.h>
#include <core/physical.h>
#include <core/scheduler.h>
#include <core/elf.h>
#include <core/debug.h>

#include <fayt/slab.h>
#include <fayt/hash.h>
#include <fayt/bitmap.h>
#include <fayt/string.h>

static struct hash_table namespace_table;
static struct bitmap nid_bitmap;

static volatile struct limine_module_request limine_module_request = {
	.id = LIMINE_MODULE_REQUEST,
	.revision = 0
};

static int launch_schedulers(struct limine_file*);

int create_server(const char *namespace_name, const char *name, struct server *server) {
	struct namespace *namespace = hash_table_search(&namespace_table,
			(void*)namespace_name, strlen(namespace_name));
	if(namespace == NULL) {
		return -1;
	}

	struct server_id id = (struct server_id) {
		.nid = namespace->nid,
		.sid = bitmap_alloc(&namespace->sid_bitmap)
	};

	*server = (struct server) {
		.name = ({void *copy = alloc(strlen(name) + 1); strcpy(copy, name); copy;}),
		.id = id
	};

	hash_table_push (
		&namespace->server_table,
		(void*)server->name,
		server, 
		strlen(server->name)
	);

	return 0;
}

int create_namespace(const char *name) {
	struct namespace *namespace = alloc(sizeof(struct namespace));

	*namespace = (struct namespace) {
		.name = ({void *copy = alloc(strlen(name) + 1); strcpy(copy, name); copy; }),
		.nid = bitmap_alloc(&nid_bitmap),
		.sid_bitmap = (struct bitmap) {
			.data = NULL,
			.size = 0,
			.resizable = true
		}
	};

	hash_table_push (
		
	&namespace_table,
			(void*)namespace->name,
			namespace,
			strlen(namespace->name)
	);

	return 0;
}

struct server *find_server(const char *namespace_name, const char *server_name) {
	struct namespace *namespace = hash_table_search(&namespace_table,
			(void*)namespace_name, strlen(namespace_name));
	if(namespace == NULL) {
		return NULL;
	}

	struct server *server = hash_table_search(&namespace->server_table,
			(void*)server_name, strlen(server_name));

	return server;
}

int launch_servers(void) {
	if(limine_module_request.response == NULL) {
		return -1;
	}

	struct limine_file **modules = limine_module_request.response->modules;
	uint64_t module_count = limine_module_request.response->module_count;

	print("dufay: booting servers {%x}\n", module_count);

	for(uint64_t i = 0; i < module_count; i++) {
		if(strcmp(modules[i]->cmdline, "scheduler") == 0) {
			launch_schedulers(modules[i]);
		}
	}

	return 0;
}

struct scheduler_meta {
	struct __attribute__((packed)) {
		uintptr_t base; 
		size_t limit;
	} file;

	struct __attribute__((packed)) {
		const char *identifier; 
		size_t length;
	} share;
} __attribute__((packed));

static int launch_server(struct server *server) {
	struct elf64_file *elf = alloc(sizeof(struct elf64_file));

	elf->data.buffer = server->file->address;
	elf->data.length = server->file->size;

	int ret = elf64_file_init(elf);
	if(ret == -1) return -1;

	ret = elf64_file_aux(elf, &elf->aux);
	if(ret == -1) return -1;

	struct context *context = alloc(sizeof(struct context));

	ret = create_blank_context(context); 
	if(ret == -1) return -1;

	elf->page_table = context->page_table;

	ret = elf64_file_load(elf);
	if(ret == -1) return -1;

	context->regs.rip = elf->aux.at_entry;
	context->regs.cs = 0x43;
	context->regs.rflags = 0x202;
	context->regs.ss = 0x3b;

	context->user_stack.sp = SERVER_DEFAULT_STACK_LOCATION + SERVER_DEFAULT_STACK_SIZE;
	context->user_stack.size = SERVER_DEFAULT_STACK_SIZE;

	uintptr_t stack_physical = pmm_alloc(DIV_ROUNDUP(context->user_stack.sp, PAGE_SIZE), 1) + 
		DIV_ROUNDUP(context->user_stack.sp, PAGE_SIZE) * PAGE_SIZE; 
	uintptr_t stack_virtual = context->user_stack.sp;

	for(size_t i = 0; i < DIV_ROUNDUP(SERVER_DEFAULT_STACK_SIZE, PAGE_SIZE); i++) {
		context->page_table->map_page(context->page_table, stack_virtual - PAGE_SIZE * i,
				stack_physical - PAGE_SIZE * i,
				X86_FLAGS_P | X86_FLAGS_RW | X86_FLAGS_US);
	}

	char *argv[] = { "scheduler", NULL };
	char *envp[] = { "server", NULL };

	int argv_cnt = 1;
	int envp_cnt = 1;

	char *location = (void*)(stack_physical + HIGH_VMA);

	for(int i = 0; i < envp_cnt; i++) {
		location -= strlen(envp[i]) + 1;
		strcpy(location, envp[i]);
	}

	for(int i = 0; i < argv_cnt; i++) {
		location -= strlen(argv[i]) + 1;
		strcpy(location, argv[i]);
	}

	location = (void*)((uint64_t)location & -16ll);

	if((argv_cnt + envp_cnt + 1) & 1) location--;

	location -= 10;

	location[0] = ELF_AT_PHNUM; location[1] = elf->aux.at_phnum;
	location[2] = ELF_AT_PHENT; location[3] = elf->aux.at_phent;
	location[4] = ELF_AT_PHDR;	location[5] = elf->aux.at_phdr;
	location[6] = ELF_AT_ENTRY; location[7] = elf->aux.at_entry;
	location[8] = 0; location[9] = 0;

	context->regs.rsp = stack_virtual - (stack_physical - ((uint64_t)location - HIGH_VMA));

	server->context = context;

	return 0;
}

static int launch_schedulers(struct limine_file *file) {
	int ret = create_namespace("SCHEDULER");
	if(ret == -1) return -1;

	struct server *servers[bootable_processor_cnt];

	for(int i = 0; i < bootable_processor_cnt; i++) {
		servers[i] = alloc(sizeof(struct server));

		char *server_name = alloc(SERVER_MAX_NAME_LENGTH);
		sprint(server_name, "SCHEDULER CORE%d", i);

		ret = create_server("SCHEDULER", server_name, servers[i]);
		if(ret == -1) return -1;

		logical_processor_locales[i].scheduling_server = servers[i];
		servers[i]->file = file;
	}

	for(int i = 0; i < bootable_processor_cnt; i++) {
		if(launch_server(servers[i]) == -1) {
			print("dufay: failed to launch server\n");
			continue;
		}

		int ret = sched_establish_shared_link(servers[i]->context, servers[i]->name);
		if(ret == -1) return -1;
	}

	return 0;
}
