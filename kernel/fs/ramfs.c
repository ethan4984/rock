#include <fs/ramfs.h>

static_vec(ramfs_node_t, ramfs_nodes);

static ramfs_node_t *vfs2ramfs(vfs_node_t *vfs_node) {
    for(size_t i = 0; i < ramfs_nodes.element_cnt; i++) {
        ramfs_node_t *ramfs_node = vec_search(ramfs_node_t, ramfs_nodes, i);
        if(strcmp(ramfs_node->vfs_node.absolute_path, vfs_node->absolute_path) == 0)
            return ramfs_node;
    }
    return NULL;
}

int ramfs_write(vfs_node_t *vfs_node, off_t off, off_t cnt, void *buf) {
    ramfs_node_t *ramfs_node = vfs2ramfs(vfs_node);
    if(ramfs_node == NULL)
        return -1;

    if(ramfs_node->buf == NULL) {
        vfs_node->stat.st_size = off + cnt;
        ramfs_node->buf = kmalloc(off + cnt);
    }

    if((off + cnt) > vfs_node->stat.st_size) {
        vfs_node->stat.st_size += off + cnt - vfs_node->stat.st_size;
        ramfs_node->buf = krealloc(ramfs_node->buf, vfs_node->stat.st_size);
    }

    memcpy8(ramfs_node->buf + off, buf, cnt);

    return 0;
}

int ramfs_read(vfs_node_t *vfs_node, off_t off, off_t cnt, void *buf) {
    ramfs_node_t *ramfs_node = vfs2ramfs(vfs_node);
    if(ramfs_node == NULL)
        return -1;

    if(off > vfs_node->stat.st_size)
        return -1;

    if((off + cnt) > vfs_node->stat.st_size) 
        cnt = vfs_node->stat.st_size - off;

    memcpy8(buf, ramfs_node->buf + off, cnt);

    return 0;
}

int ramfs_delete(vfs_node_t *vfs_node) {
    ramfs_node_t *ramfs_node = vfs2ramfs(vfs_node);
    if(ramfs_node == NULL)
        return -1;

    kfree(ramfs_node->buf);
    vec_addr_remove(ramfs_node_t, ramfs_nodes, ramfs_node);

    return 0;
}

int ramfs_open(vfs_node_t *vfs_node, int flags) {
    if(flags & O_CREAT) {
        ramfs_node_t ramfs_node = { .vfs_node = *vfs_node, .perms = flags };
        vec_push(ramfs_node_t, ramfs_nodes, ramfs_node);
        return 0;
    }

    if(vfs2ramfs(vfs_node) == NULL)
        return -1;
    return 0;
}
