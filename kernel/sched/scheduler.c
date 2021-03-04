#include <sched/scheduler.h>
#include <mm/mmap.h>
#include <acpi/madt.h>
#include <sched/smp.h>
#include <fs/fd.h>
#include <vec.h>
#include <elf.h>
#include <debug.h>
#include <int/apic.h>

global_hash_table(tasks);
char sched_lock = 0;

static struct thread *find_next_thread(struct task *task) {
    struct thread *ret = NULL; 

    for(size_t i = 0, cnt = 0; i < task->threads.hash_cnt; i++) {
        struct thread *thread = vec_search(struct thread, task->threads.data_map, i);
        thread->idle_cnt++;

        if(thread->status == SCHED_WAITING && cnt < thread->idle_cnt) {
            cnt = thread->idle_cnt;
            ret = thread;
        }
    }

    return ret;
}

static struct task *find_next_task() {
    struct task *ret = NULL;

    if(tasks.hash_cnt == 1) {
        return vec_search(struct task, tasks.data_map, 0);
    }

    for(size_t i = 0, cnt = 0; i < tasks.hash_cnt; i++) {
        struct task *task = vec_search(struct task, tasks.data_map, i);
        task->idle_cnt++;

        if(task->status == SCHED_WAITING && cnt < task->idle_cnt) {
            cnt = task->idle_cnt;
            ret = task;
        }
    }

    return ret;
}

static void reschedule(struct regs *regs) {
    if(regs->cs & 0x3)
        swapgs();

    struct core_local *local = get_core_local(CURRENT_CORE);
    
    struct task *next_task = find_next_task();
    if(next_task == NULL) {
        if(regs->cs & 0x3)
            swapgs();
        return;
    }

    struct thread *next_thread = find_next_thread(next_task);
    if(next_thread == NULL) {
        if(regs->cs & 0x3)
            swapgs();
        return;
    }

    int tid = next_thread->tid;
    int pid = next_task->pid;

    if(local->tid != -1 && local->pid != -1) {
        struct task *old_task = hash_search(struct task, tasks, local->pid);
        if(old_task == NULL) {
            if(regs->cs & 0x3)
                swapgs();
            return;
        }

        struct thread *old_thread = hash_search(struct thread, old_task->threads, local->tid);
        if(old_thread == NULL) {
            if(regs->cs & 0x3)
                swapgs();
            return;
        }

        old_thread->regs = *regs;
        old_thread->status = SCHED_WAITING;
        old_task->status = SCHED_WAITING;

        old_thread->user_gs_base = get_user_gs(); 
        old_thread->user_fs_base = get_user_fs();
    }

    local->pid = pid;
    local->tid = tid;
    local->page_map = next_task->page_map;

    next_thread->idle_cnt = 0;
    next_task->idle_cnt = 0;
    
    next_thread->status = SCHED_RUNNING;
    next_task->status = SCHED_RUNNING;

    set_user_fs(next_thread->user_fs_base);
    set_user_gs(next_thread->user_gs_base);

    vmm_page_map_init(next_task->page_map);

    if(next_thread->regs.cs & 0x3) {
        swapgs();
    }
    
    lapic_write(LAPIC_EOI, 0);
    spin_release(&sched_lock);

    switch_task((uint64_t)&next_thread->regs);
}

struct task *sched_create_task(struct task *parent, struct page_map *page_map) {
    struct task task = { .status = SCHED_WAITING };

    if(parent != NULL) {
        task.ppid = parent->pid;
    }

    if(page_map != NULL) {
        task.page_map = page_map;
    } else {
        task.page_map = &kernel_mapping;
    }

    pid_t new_pid = hash_push(struct task, tasks, task);
    struct task *new_task = hash_search(struct task, tasks, new_pid);
    new_task->pid = new_pid;

    char *stdout_name = kcalloc(256);
    char *stderr_name = kcalloc(256);
    char *stdin_name = kcalloc(256);

    sprintf(stdout_name, "/dev/stdout%x", 1,  new_pid);
    sprintf(stderr_name, "/dev/stderr%x", 1, new_pid);
    sprintf(stdin_name, "/dev/stdin%x", 1, new_pid);

    fd_open_task(new_task, stdin_name, O_CREAT);
    fd_open_task(new_task, stdout_name, O_CREAT);
    fd_open_task(new_task, stderr_name, O_CREAT);

    return new_task;
}

int sched_delete_thread(struct task *parent, int tid) {
    struct thread *thread = hash_search(struct thread, parent->threads, tid);
    if(thread == NULL)
        return -1;

    struct core_local *local = get_core_local(CURRENT_CORE);

    pmm_free(thread->kernel_stack - THREAD_STACK_SIZE, THREAD_STACK_SIZE / PAGE_SIZE);
    munmap(local->page_map, (void*)thread->user_stack - THREAD_STACK_SIZE, THREAD_STACK_SIZE);

    return hash_remove(struct thread, parent->threads, (size_t)tid);
}

struct thread *sched_create_thread(pid_t pid, struct aux *aux, const char **argv, const char **envp, uint64_t starting_addr, uint16_t cs) {
    struct task *task = hash_search(struct task, tasks, pid);
    if(task == NULL)
        return NULL;

    struct core_local *local = get_core_local(CURRENT_CORE);

    struct thread thread = {    .status = SCHED_WAITING,
                                .kernel_stack = (size_t)pmm_alloc(THREAD_STACK_SIZE / PAGE_SIZE) + THREAD_STACK_SIZE,
                           };

    tid_t new_tid = hash_push(struct thread, task->threads, thread);
    struct thread *new_thread = hash_search(struct thread, task->threads, new_tid);
    new_thread->tid = new_tid;

    new_thread->regs.rip = starting_addr;
    new_thread->regs.cs = cs;
    new_thread->regs.rflags = 0x202;

    if(cs & 0x3) {
        new_thread->regs.ss = cs - 8;

        new_thread->user_stack = (size_t)mmap(task->page_map, NULL, THREAD_STACK_SIZE, PROT_READ | PROT_WRITE | (1 << 2), MAP_ANONYMOUS, 0, 0) + THREAD_STACK_SIZE;
        new_thread->regs.rsp = new_thread->user_stack;
        
        uint64_t *stack = (uint64_t*)new_thread->regs.rsp; 

        size_t envp_cnt = 0;
        size_t argv_cnt = 0;

        while(*envp) {
            const char *element = *envp;
            stack = (void*)stack - (strlen(element) + 1);
            strcpy((void*)stack, element);
            envp++; envp_cnt++;
        }

        while(*argv) {
            const char *element = *argv;
            stack = (void*)stack - (strlen(element) + 1);
            strcpy((void*)stack, element);
            argv++; argv_cnt++;
        }

        argv -= argv_cnt; envp -= envp_cnt;

        stack = (void*)stack - ((uintptr_t)stack & 0xf);

        if((argv_cnt + envp_cnt + 1) & 1)
            stack--;

        stack -= 10;

        stack[0] = AT_PHNUM; stack[1] = aux->at_phnum;
        stack[2] = AT_PHENT; stack[3] = aux->at_phent;
        stack[4] = AT_PHDR;  stack[5] = aux->at_phdr;
        stack[6] = AT_ENTRY; stack[7] = aux->at_entry;
        stack[8] = 0; stack[9] = 0;

        uint64_t save = new_thread->regs.rsp;

        *(--stack) = 0;
        stack -= envp_cnt;

        for(size_t i = 0; i < envp_cnt; i++) {
            save -= strlen(envp[i]) + 1;
            stack[i] = save;
        }

        *(--stack) = 0;
        stack -= argv_cnt;

        for(size_t i = 0; i < argv_cnt; i++) {
            save -= strlen(argv[i]) + 1;
            stack[i] = save;
        }

        *(--stack) = argv_cnt; // argc

        new_thread->regs.rsp = (uint64_t)stack;
    } else {
        new_thread->regs.ss = cs + 8;
        new_thread->regs.rsp = new_thread->kernel_stack + HIGH_VMA;
    }

    return new_thread;
}

void scheduler_main(struct regs *regs) {
    spin_lock(&sched_lock);

    reschedule(regs);

    spin_release(&sched_lock);
}

void syscall_getpid(struct regs *regs) {
    struct core_local *local = get_core_local(CURRENT_CORE);
    regs->rax = local->pid;
}

void syscall_gettid(struct regs *regs) {
    struct core_local *local = get_core_local(CURRENT_CORE);
    regs->rax = local->tid;
}

void syscall_getppid(struct regs *regs) {
    struct core_local *local = get_core_local(CURRENT_CORE);
    struct task *task = hash_search(struct task, tasks, local->pid);
    regs->rax = task->ppid;
}

int sched_exec(char *path, const char **argv, const char **envp, int mode) {
     int fd = open(path, 0);
     if(fd == -1)
        return -1;

    struct task *parent;

    struct core_local *local = get_core_local(CURRENT_CORE);
    if(local->pid == -1)
        parent = NULL;
    else
        parent = hash_search(struct task, tasks, local->pid);
    
    uint16_t cs;

    if(mode & SCHED_USER)
        cs = 0x23;
    else if(mode & SCHED_KERNEL)
        cs = 0x8;
    else 
        return -1;

    if(!(mode & SCHED_ELF))
       return -1; 

    asm ("cli");

    struct page_map *page_map = vmm_generic_page_map();
    vmm_page_map_init(page_map);

    char *ld_path = NULL;

    struct aux aux;
    if(elf64_load(page_map, &aux, fd, 0, &ld_path) == -1)
        return -1;

    uint64_t entry_point = aux.at_entry;

    if(ld_path != NULL) {
        int ld_fd = open(ld_path, 0);
        if(ld_fd == -1)
            return -1;
    
        struct aux ld_aux;
        if(elf64_load(page_map, &ld_aux, ld_fd, 0x40000000, NULL) == -1)
            return -1;

        entry_point = ld_aux.at_entry;
    }

    struct task *new_task = sched_create_task(parent, page_map);
    struct thread *new_thread = sched_create_thread(new_task->pid, &aux, argv, envp, entry_point, cs);

    vmm_page_map_init(local->page_map);

    asm ("sti");

    return 0;
}

void syscall_execve(struct regs *regs) {
    regs->rax = sched_exec((void*)regs->rdi, (void*)regs->rsi, (void*)regs->rdx, SCHED_USER | SCHED_ELF);
}

void yeild(struct regs *regs) {
    swapgs();
    reschedule(regs);    
}

void syscall_yeild(struct regs *regs) {
    yeild(regs); 
} 

int sched_exit(struct regs *regs) {
    struct core_local *local = get_core_local(CURRENT_CORE);
    struct task *current_task = hash_search(struct task, tasks, local->pid);

    for(size_t i = 0; i < current_task->fd_list.hash_cnt; i++) {
        int fd = *vec_search(int, current_task->fd_list.data_map, i);
        close(fd);
    }

    hash_delete(current_task->fd_list);

    for(size_t i = 0; i < current_task->threads.hash_cnt; i++) {
        struct thread *thread = vec_search(struct thread, current_task->threads.data_map, i);
        sched_delete_thread(current_task, thread->tid); 
    }

    local->pid = -1;
    local->tid = -1;

    yeild(regs);

    return -1;
}

void syscall_exit(struct regs *regs) {
    regs->rax = sched_exit(regs);
}
