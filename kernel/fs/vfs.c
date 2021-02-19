#include <sched/scheduler.h>
#include <fs/vfs.h>
#include <debug.h>
#include <vec.h>

struct vfs_node vfs_root_node = { .absolute_path = "/",
                             .name = "/",
                             .relative_path = "/"
                           };

struct vfs_node *vfs_create_node(struct vfs_node *parent, char *name) {
    char *absolute_path;
    if(parent != &vfs_root_node)  {
        char *dir_path = str_congregate(parent->absolute_path, "/");
        absolute_path = str_congregate(dir_path, name);
        kfree(dir_path);
    } else {
        absolute_path = str_congregate(parent->absolute_path, name);
    }

    char *relative_path = NULL;
    if((parent->fs != NULL) && (parent->fs->mount_gate != NULL))
        relative_path = absolute_path + (strlen(parent->fs->mount_gate));

    struct vfs_node new_node = {    .parent = parent,
                                    .absolute_path = absolute_path,
                                    .relative_path = relative_path,
                                    .name = name,
                                    .fs = parent->fs
                               };

    vec_push(struct vfs_node, parent->child_nodes, new_node);
    
    return vec_search(struct vfs_node, parent->child_nodes, parent->child_nodes.element_cnt - 1);
}

struct vfs_node *vfs_create_node_deep(char *path) {
    char *buffer = kmalloc(strlen(path));
    strcpy(buffer, path);

    struct vfs_node *node = &vfs_root_node;
    struct vfs_node *parent;

    char *sub_path, *save = buffer;
    while((sub_path = strtok_r(save, "/", &save))) {
        parent = node;
        node = vfs_relative_path(node, sub_path);
        if(node == NULL)
            goto found;
    }

    kfree(buffer);
    return NULL; 

found:
    do {
        char *name = kmalloc(strlen(sub_path));
        strcpy(name, sub_path);
        parent = vfs_create_node(parent, name);
    } while((sub_path = strtok_r(save, "/", &save)));

    kfree(buffer);
    return parent;
}

struct vfs_node *vfs_absolute_path(char *path) {
    if(strcmp(path, "/") == 0)
        return &vfs_root_node;

    if(*path == '/')
        path++;

    char *buffer = kmalloc(strlen(path));
    strcpy(buffer, path);

    struct vfs_node *node = &vfs_root_node;

    char *sub_path, *save = buffer;
    while((sub_path = strtok_r(save, "/", &save))) {
        node = vfs_relative_path(node, sub_path);
        if(node == NULL) {
            kfree(buffer);
            return NULL;
        }
    }

    kfree(buffer);
    return node;
}

struct vfs_node *vfs_relative_path(struct vfs_node *parent, char *name) {
    for(size_t i = 0; i < parent->child_nodes.element_cnt; i++) {
        struct vfs_node *node = vec_search(struct vfs_node, parent->child_nodes, i);
        if(strcmp(node->name, name) == 0) {
            return node;
        }
    }
    return NULL;
}

struct vfs_node *vfs_mkdir(struct vfs_node *parent, char *name) {
    struct vfs_node *dir_node = vfs_create_node(parent, name);

    vfs_create_node(dir_node, ".");
    struct vfs_node *dotdot = vfs_create_node(dir_node, "..");
    dotdot->parent = parent;

    return dir_node;
}

static void vfs_remove_cluster(struct vfs_node *node) {
    if(node->child_nodes.element_cnt != 0) {
        for(size_t i = 0; i < node->child_nodes.element_cnt; i++) {
            struct vfs_node *tmp = vec_search(struct vfs_node, node->child_nodes, i);
            tmp->fs->unlink(tmp);
            vfs_remove_cluster(tmp);
        }
        vec_delete(node->child_nodes);
    } else {
        node->fs->unlink(node);
        vec_delete(node->child_nodes);
    }
}

int vfs_remove_node(struct vfs_node *node) {
    if(vfs_absolute_path(node->absolute_path) == NULL)
        return -1;

    vfs_remove_cluster(node);

    return 0; 
}

int vfs_mount_dev(char *dev, char *mount_gate) {
    struct vfs_node *mount_node = vfs_absolute_path(mount_gate);
    if(mount_node == NULL) 
        return -1;

    struct devfs_node *devfs_node = path2devfs(dev);
    if(devfs_node == NULL || devfs_node->device == NULL || devfs_node->device->fs == NULL)
        return -1;

    devfs_node->device->fs->mount_gate = mount_gate;
    mount_node->fs = devfs_node->device->fs;
    devfs_node->device->fs->refresh(mount_node);

    kprintf("[FS] [%s] mounted to [%s]\n", dev, mount_gate);

    return 0;
}

int vfs_mount_fs(struct filesystem *fs) {
    struct vfs_node *vfs_node = vfs_absolute_path(fs->mount_gate);
    if(vfs_node == NULL)
        return -1;

    vfs_node->fs = fs;

    return 0;
}

int vfs_write(struct vfs_node *node, off_t off, off_t cnt, void *buf) {
    if(node == NULL) {
        return -1;
    }

    return node->fs->write(node, off, cnt, buf);
}

int vfs_read(struct vfs_node *node, off_t off, off_t cnt, void *buf) {
    if(node == NULL) {
        return -1;
    }

    return node->fs->read(node, off, cnt, buf);
}

int vfs_open(char *path, int flags) {
    struct vfs_node *node = vfs_absolute_path(path);
    if(node == NULL && flags & O_CREAT) {
        node = vfs_create_node_deep(path);
        if(node == NULL) 
            return -1;
        int ret = node->fs->open(node, flags);
        if(ret == -1)
            vfs_remove_node(node);
        return ret;
    } else if(node == NULL) {
        set_errno(ENOENT);
        return -1;
    }
    
    int ret = node->fs->open(node, flags);
    if(ret == -1) {
        vfs_remove_node(node);
    }

    return ret;
}

int vfs_unlink(char *path) {
    struct vfs_node *node = vfs_absolute_path(path);
    if(node == NULL)
        return -1;

    vfs_remove_node(node);

    return 0;
}

void syscall_chdir(struct regs *regs) {
    struct core_local *local = get_core_local(CURRENT_CORE);
    struct task *current_task = hash_search(struct task, tasks, local->pid);

    struct vfs_node *working_dir = vfs_absolute_path((void*)regs->rdi);
    current_task->working_dir = working_dir;

    regs->rax = 0;
}
