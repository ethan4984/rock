#include <fs/vfs.h>
#include <output.h>
#include <bitmap.h>

vfs_node_t vfs_root_node = { .absolute_path = "/",
                             .name = "/",
                           };

vfs_node_t *vfs_create_node(vfs_node_t *parent, char *name) {
    char *absolute_path;
    if(parent != &vfs_root_node)  {
        char *dir_path = str_congregate(parent->absolute_path, "/");
        absolute_path = str_congregate(dir_path, name);
        kfree(dir_path);
    } else {
        absolute_path = str_congregate(parent->absolute_path, name);
    }

    char *relative_path = NULL;
    if(parent->fs != NULL && parent->fs->mount_gate != NULL)
        relative_path = absolute_path + strlen(parent->fs->mount_gate);

    vfs_node_t new_node = { .parent = parent,
                            .absolute_path = absolute_path,
                            .relative_path = relative_path,
                            .name = name,
                            .fs = parent->fs
                          };

    vec_push(vfs_node_t, parent->child_nodes, new_node);
    
    return vec_search(vfs_node_t, parent->child_nodes, parent->child_nodes.element_cnt - 1);
}

vfs_node_t *vfs_create_node_deep(vfs_node_t *parent, char *path) {
    char *buffer = kmalloc(strlen(path));
    strcpy(buffer, path);

    vfs_node_t *node = &vfs_root_node;
    vfs_node_t *node_parent;

    char *sub_path, *save = buffer;
    while((sub_path = strtok_r(save, "/", &save))) {
        node_parent = node;
        node = vfs_relative_path(parent, sub_path);
        if(node == NULL) {
            char *name = kmalloc(strlen(sub_path));
            strcpy(name, sub_path);

            if(strtok_r(save, "/", &save) == NULL) {
                node = vfs_create_node(node_parent, name);
            } else {
                node = NULL;
            }

            kfree(name);
            kfree(buffer);

            return node;
        }
    }
    return NULL;
}

vfs_node_t *vfs_find_parent(char *path) {
    if(strcmp(path, "/") == 0)
        return &vfs_root_node;

    char *buffer = kmalloc(strlen(path));
    strcpy(buffer, path);

    vfs_node_t *node = &vfs_root_node;
    vfs_node_t *parent = NULL;

    char *sub_path, *save = buffer;
    while((sub_path = strtok_r(save, "/", &save))) {
        parent = node;
        node = vfs_relative_path(node, sub_path);
        if(node == NULL) {
            kfree(buffer);
            return NULL;
        }
    }
   
    kfree(buffer);
    return parent;
}

vfs_node_t *vfs_absolute_path(char *path) {
    if(strcmp(path, "/") == 0)
        return &vfs_root_node;

    if(*path == '/')
        path++;

    char *buffer = kmalloc(strlen(path));
    strcpy(buffer, path);

    vfs_node_t *node = &vfs_root_node;

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

vfs_node_t *vfs_relative_path(vfs_node_t *parent, char *name) {
    for(size_t i = 0; i < parent->child_nodes.element_cnt; i++) {
        vfs_node_t *node = vec_search(vfs_node_t, parent->child_nodes, i);
        if(strcmp(node->name, name) == 0) {
            return node;
        }
    }
    return NULL;
}

vfs_node_t *vfs_check_node(vfs_node_t *node) {
    return vfs_absolute_path(node->absolute_path);
}

vfs_node_t *vfs_mkdir(vfs_node_t *parent, char *name) {
    vfs_node_t *dir_node = vfs_create_node(parent, name);

    vfs_create_node(dir_node, ".");
    vfs_node_t *dotdot = vfs_create_node(dir_node, "..");
    dotdot->parent = parent;

    return dir_node;
}

static void vfs_remove_cluster(vfs_node_t *node) {
    if(node->child_nodes.element_cnt != 0) {
        for(size_t i = 0; i < node->child_nodes.element_cnt; i++) {
            vfs_node_t *tmp = vec_search(vfs_node_t, node->child_nodes, i);
            tmp->fs->unlink(tmp);
            vfs_remove_cluster(tmp);
        }
        vec_delete(node->child_nodes);
    } else {
        node->fs->unlink(node);
        vec_delete(node->child_nodes);
    }
}

int vfs_remove_node(vfs_node_t *node) {
    if(vfs_check_node(node) == NULL)
        return -1;

    vfs_remove_cluster(node);

    return 0; 
}

int vfs_mount_dev(char *dev, char *mount_gate) {
    vfs_node_t *mount_node = vfs_absolute_path(mount_gate);
    if(mount_node == NULL) 
        return -1;

    devfs_node_t *devfs_node = path2devfs(dev);
    if(devfs_node == NULL)
        return -1;

    if(devfs_node->device == NULL) 
        return -1;

    if(devfs_node->device->fs == NULL)
        return -1;

    kprintf("[FS]", "Mounting [%s] to [%s]", dev, mount_gate);

    devfs_node->device->fs->mount_gate = mount_gate;
    mount_node->fs = devfs_node->device->fs;

    devfs_node->device->fs->refresh(mount_node);

    return 0;
}

int vfs_mount_fs(filesystem_t *fs) {
    vfs_node_t *vfs_node = vfs_absolute_path(fs->mount_gate);
    if(vfs_node == NULL)
        return -1;

    vfs_node->fs = fs;

    return 0;
}

int vfs_write(vfs_node_t *node, off_t off, off_t cnt, void *buf) {
    if(node == NULL) {
        return -1;
    }

    return node->fs->write(node, off, cnt, buf);
}

int vfs_read(vfs_node_t *node, off_t off, off_t cnt, void *buf) {
    if(node == NULL) {
        return -1;
    }

    return node->fs->read(node, off, cnt, buf);
}

int vfs_open(char *path, int flags) {
    vfs_node_t *node = vfs_absolute_path(path);
    if(node == NULL && flags & O_CREAT) {
        node = vfs_create_node_deep(&vfs_root_node, path);
        if(node == NULL) 
            return -1;
        int ret = node->fs->open(node, flags);
        if(ret == -1)
            vfs_remove_node(node);
        return ret;
    } else if(node == NULL) {
        return -1;
    }

    int ret = node->fs->open(node, flags);
    if(ret == -1)
        vfs_remove_node(node);

    return ret;
}

int vfs_unlink(char *path) {
    vfs_node_t *node = vfs_absolute_path(path);
    if(node == NULL)
        return -1;

    vfs_remove_node(node);

    return 0;
}
