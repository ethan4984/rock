#include <fs/fd.hpp>
#include <types.hpp>
#include <string.hpp>
#include <sched/smp.hpp>
#include <sched/scheduler.hpp>
#include <memutils.hpp>
#include <drivers/tty.hpp>
#include <utility>
#include <fs/ramfs.hpp>

#define SYSCALL_FD_TRANSLATE(FD_INDEX) \
    smp::cpu *core = smp::core_local(); \
    sched::task *current_task = sched::task_list[core->pid]; \
    fd &fd_back = current_task->fd_list.list[FD_INDEX]; \
    if(fd_back.backing_fd == -1 || fd_back.status == 0) { \
        set_errno(ebadf); \
        regs_cur->rax = -1; \
        return; \
    }

namespace fs {

fd &alloc_fd(lib::string path, int flags) {
    smp::cpu *core = smp::core_local();
    sched::task *current_task = sched::task_list[core->pid];

    const auto index = [](sched::task *task) {
        const auto find_index = [](sched::task *task, const auto &func) -> size_t {
            for(size_t i = 0; i < task->fd_list.bitmap_size; i++) {
                if(!bm_test(task->fd_list.bitmap, i)) {
                    bm_set(task->fd_list.bitmap, i);
                    return i;
                }
            }

            task->fd_list.bitmap_size += 0x1000;
            task->fd_list.bitmap = (uint8_t*)kmm::recalloc(task->fd_list.bitmap, task->fd_list.bitmap_size);

            return func(task, func);
        }; 

        return find_index(task, find_index);
    } (current_task);

    return (current_task->fd_list.list[index] = fd(path, flags, index));
}

void free_fd(size_t index) {
    smp::cpu *core = smp::core_local();
    sched::task *current_task = sched::task_list[core->pid];

    if(current_task->fd_list.bitmap_size <= index)
        return;

    bm_clear(current_task->fd_list.bitmap, index);
}

inline std::pair<ssize_t, fd&> translate(int index) {
    smp::cpu *core = smp::core_local();
    sched::task *current_task = sched::task_list[core->pid];

    std::pair<ssize_t, fd&> ret = { .first = -1, .second = current_task->fd_list.list[index] };

    if(ret.second.backing_fd == -1 || ret.second.status == 0)
        return ret;

    ret.first = 0;

    return ret;
}

fd::fd(lib::string path, int flags, int backing_fd) : status(0), backing_fd(backing_fd), dirent(false) {
    auto open = [&, this]() {
        vfs_node = vfs::root_cluster->search_absolute(path);
        if((vfs_node == NULL) && (flags & o_creat)) {
            vfs::root_cluster->generate_node(path, NULL, flags);
            vfs_node = vfs::root_cluster->search_absolute(path);
            if(vfs_node == NULL) {
                set_errno(enoent);
                return -1;
            }

            int ret = vfs_node->filesystem->open(vfs_node, flags);

            return ret;
        } else if(vfs_node == NULL) {
            set_errno(enoent);
            return -1; 
        }

        vfs_node->stat_cur->st_mode |= flags;

        int ret = vfs_node->filesystem->open(vfs_node, flags);

        return ret;
    } ();

    if(open == -1) {
        return;
    }

    _loc = (size_t*)kmm::calloc(sizeof(size_t)); 
    _flags = (size_t*)kmm::calloc(sizeof(size_t));

    status = 1;
}

int fd::read(void *buf, size_t cnt) {
    if(vfs_node == NULL) {
        set_errno(enoent);
        return -1;
    }

    int ret = vfs_node->filesystem->read(vfs_node, *_loc, cnt, buf); 
    if(ret == -1)
        return -1;

    *_loc += cnt; 

    return ret;
}

int fd::write(void *buf, size_t cnt) {
    if(vfs_node == NULL) {
        set_errno(enoent);
        return -1;
    }

    int ret = vfs_node->filesystem->write(vfs_node, *_loc, cnt, buf);
    if(ret == -1)
        return -1;

    *_loc += cnt; 

    return ret;
}

int fd::seek(off_t off, int whence) {
    if(vfs_node == NULL) {
        set_errno(enoent);
        return -1;
    }

    switch(whence) {
        case seek_set:
            return (*_loc = off);
        case seek_cur:
            return (*_loc += off);
        case seek_end:
            return (*_loc = vfs_node->stat_cur->st_size + off);
        default:
            set_errno(einval);
            return -1;
    }
}

extern "C" void syscall_open(regs *regs_cur) {
    smp::cpu *core = smp::core_local();
    sched::task *current_task = sched::task_list[core->pid];

    lib::string path = lib::string((char*)regs_cur->rdi);

    if(path[0] != '/') {
        vfs::node *vfs_path = vfs::root_cluster->search_absolute(lib::string((char*)regs_cur->rdi), current_task->working_directory);

        if(vfs_path == NULL) {
            set_errno(enoent);
            regs_cur->rax = -1;
            return;
        }

        path = vfs::get_absolute_path(vfs_path);
    }

    int flags = regs_cur->rsi;
    int mode = s_irusr | s_iwusr;

    if(flags & o_creat) {
        mode = regs_cur->rdx;
    }
    
    fd &new_fd = alloc_fd(path, flags | mode);

    regs_cur->rax = new_fd.backing_fd;

    if(new_fd.status == 0) {
        current_task->fd_list.list.remove(new_fd.backing_fd);
        free_fd(new_fd.backing_fd);
        regs_cur->rax = -1;
    }
}

extern "C" void syscall_close(regs *regs_cur) {
    SYSCALL_FD_TRANSLATE(regs_cur->rdi);

    current_task->fd_list.list.remove(regs_cur->rdi);
    free_fd(regs_cur->rdi);

    regs_cur->rax = 0;
}

extern "C" void syscall_read(regs *regs_cur) {
    SYSCALL_FD_TRANSLATE(regs_cur->rdi);

    regs_cur->rax = fd_back.read((void*)regs_cur->rsi, regs_cur->rdx);
}

extern "C" void syscall_write(regs *regs_cur) {
    SYSCALL_FD_TRANSLATE(regs_cur->rdi);

    regs_cur->rax = fd_back.write((void*)regs_cur->rsi, regs_cur->rdx);
}

extern "C" void syscall_seek(regs *regs_cur) {
    SYSCALL_FD_TRANSLATE(regs_cur->rdi);

    if(fd_back.vfs_node->stat_cur->st_mode & s_ififo) {
        regs_cur->rax = -1;
        set_errno(espipe);
        return;
    }

    regs_cur->rax = fd_back.seek(regs_cur->rsi, regs_cur->rdx);
}

extern "C" void syscall_dup(regs *regs_cur) {
    SYSCALL_FD_TRANSLATE(regs_cur->rdi);

    regs_cur->rax = alloc_fd(vfs::get_absolute_path(fd_back.vfs_node), *fd_back._flags).backing_fd;
}

extern "C"  void syscall_dup2(regs *regs_cur) {
    if(regs_cur->rdi == regs_cur->rsi) {
        regs_cur->rax = regs_cur->rsi;
        return;
    }

    SYSCALL_FD_TRANSLATE(regs_cur->rdi);

    auto new_fd_status = translate(regs_cur->rsi);
    if(new_fd_status.first != -1) {
        current_task->fd_list.list.remove(regs_cur->rsi);
        free_fd(regs_cur->rsi);
    }

    current_task->fd_list.list[regs_cur->rsi] = fd_back;

    regs_cur->rax = regs_cur->rsi;
}

extern "C" void syscall_isatty(regs *regs_cur) {
    SYSCALL_FD_TRANSLATE(regs_cur->rdi);

    regs args;

    args.rdi = regs_cur->rdi;
    args.rsi = tty::tiocginsz;
    args.rdx = (size_t)(new uint64_t);

    if(fd_back.vfs_node->ioctl(&args) == -1) {
        set_errno(enotty);
        regs_cur->rax = -1;
    }

    regs_cur->rax = 1;
}

extern "C" void syscall_ioctl(regs *regs_cur) {
    SYSCALL_FD_TRANSLATE(regs_cur->rdi);
    
    if(fd_back.vfs_node->ioctl(regs_cur) == -1) {
        regs_cur->rax = -1;
        return;
    }

    regs_cur->rax = 0;
}

extern "C" void syscall_fcntl(regs *regs_cur) {
    SYSCALL_FD_TRANSLATE(regs_cur->rdi);

    switch(regs_cur->rsi) {
        case f_dupfd:
            regs_cur->rax = alloc_fd(vfs::get_absolute_path(fd_back.vfs_node), *fd_back._flags).backing_fd;
            break;
        case f_getfd:
            regs_cur->rax = (uint64_t)((*fd_back._flags & o_cloexec) ? fd_cloexec : 0);
            break;
        case f_setfd:
            *fd_back._flags = (uint64_t)((*fd_back._flags & o_cloexec) ? o_cloexec : 0);
            regs_cur->rax = 0;
            break;
        case f_getfl:
            regs_cur->rax = *fd_back._flags;
            break;
        case f_setfl:
            *fd_back._flags = regs_cur->rdx;
            regs_cur->rax = 0;
            break;
        default:
            print("fnctl unknown command {}\n", regs_cur->rsi);
            set_errno(einval);
            regs_cur->rax = -1;
    }

    return;
}

extern "C" void syscall_fstat(regs *regs_cur) {
    SYSCALL_FD_TRANSLATE(regs_cur->rdi);

    stat *stat_buf = (stat*)regs_cur->rsi;
    *stat_buf = *fd_back.vfs_node->stat_cur;

    regs_cur->rax = 0;
}

extern "C" void syscall_fstatat(regs *regs_cur) {
    smp::cpu *core = smp::core_local();
    sched::task *current_task = sched::task_list[core->pid];

    lib::string path = lib::string((char*)regs_cur->rsi);

    vfs::node *vfs_node = NULL;
    vfs::node *parent = NULL;

    if(regs_cur->rdi == 0xFFFFFF9C) {
        if(path[0] != '/') {
            vfs_node = vfs::root_cluster->search_absolute(path, current_task->working_directory);
            parent = current_task->working_directory;
        } else {
            vfs_node = vfs::root_cluster->search_absolute(path);
            parent = vfs::root_cluster->root_node;
        }
    } else {
        SYSCALL_FD_TRANSLATE(regs_cur->rdi);
        vfs_node = vfs::root_cluster->search_absolute(path, fd_back.vfs_node);
        parent = fd_back.vfs_node;
    }

    if(vfs_node == NULL) {
        set_errno(ebadf);
        regs_cur->rax = -1;
        return;
    }

    if(parent->filesystem->open(vfs_node, regs_cur->r10) == -1) {
        set_errno(enoent);
        regs_cur->rax = -1;
    }

    stat *stat_buf = (stat*)regs_cur->rdx;
    *stat_buf = *vfs_node->stat_cur;

    regs_cur->rax = 0;
}

extern "C" void syscall_readdir(regs *regs_cur) {
    SYSCALL_FD_TRANSLATE(regs_cur->rdi);
    dirent *buffer = (dirent*)regs_cur->rsi;

    if(fd_back.vfs_node->is_directory()) {
        set_errno(enotdir);
        regs_cur->rax = -1;
        return;
    }

    vfs::node *current_node;

    if(fd_back.dirent) {
        current_node = vfs::root_cluster->search_absolute(buffer->d_name);
    } else {
        current_node = fd_back.vfs_node;
        fd_back.dirent = true;
    }

    vfs::node *next_node = current_node->next;

    if(next_node == NULL) {
        set_errno(0);
        regs_cur->rax = -1;
        return;
    }

    strcpy(buffer->d_name, next_node->name.data());

    buffer->d_ino = next_node->stat_cur->st_ino;
    buffer->d_off = 0; 
    buffer->d_reclen = sizeof(dirent);

    switch(next_node->stat_cur->st_mode & s_ifmt) {
        case s_ifchr:
            buffer->d_type = dt_chr;
            break;
        case s_ifblk:
            buffer->d_type = dt_blk;
            break;
        case s_ifdir:
            buffer->d_type = dt_dir;
            break;
        case s_iflnk:
            buffer->d_type = dt_lnk;
            break;
        case s_ififo:
            buffer->d_type = dt_fifo;
            break;
        case s_ifreg:
            buffer->d_type = dt_reg;
            break;
        case s_ifsock:
            buffer->d_type = dt_sock;
            break;
        default:
            buffer->d_type = dt_unknown;
    }

    regs_cur->rax = 0;
}

extern "C" void syscall_getcwd(regs *regs_cur) {
    smp::cpu *core = smp::core_local();
    sched::task *current_task = sched::task_list[core->pid];

    char *buffer = (char*)regs_cur->rdi;
    ssize_t length = regs_cur->rsi;

    lib::string cwd_path = vfs::get_absolute_path(current_task->working_directory);
    if(cwd_path.length() <= length) {
        memcpy8((uint8_t*)buffer, (uint8_t*)cwd_path.data(), cwd_path.length());
    } else {
        set_errno(erange);
        regs_cur->rax = 0;
        return;
    }

    print("cwd path {x}\n", cwd_path);

    regs_cur->rax = (size_t)buffer;
}

}
