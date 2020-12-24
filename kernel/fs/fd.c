#include <fs/fd.h>

typedef struct fd_node {
    int flags;
    int fd;
    uint64_t file_offset;
    char *path;

    struct fd_node *next;
    struct fd_node *last;
} fd_node_t;

static fd_node_t *fd_list = NULL;

static uint8_t *free_fd_nodes;
static uint64_t max_fd = 0x1000;

static void fd_push_node(fd_node_t *head, fd_node_t *new_node) {
    fd_node_t *tmp = head;
    while(tmp->next != NULL)
        tmp = tmp->next;
    tmp->next = new_node;
    new_node->last = tmp;
}

static void fd_remove_node(fd_node_t **head, fd_node_t *node) {
    if(!*head || !node)
        return;

    if(*head == node) 
        *head = node->next;

    if(node->next != NULL) 
        node->next->last = node->last;

    if(node->last != NULL)
        node->last->next = node->next;

    kfree(node);
}

static fd_node_t *find_fd_node(fd_node_t *head, int fd) {
    fd_node_t *node = head;

    do {
        if(node->fd == fd) 
            return node;
        node = node->next;
    } while(node != NULL);

    return NULL;
}

static int activate_fd() {
    for(uint64_t i = 0; i < max_fd; i++) {
        if(!BM_TEST(free_fd_nodes, i)) {
            BM_SET(free_fd_nodes, i);
            return i;
        }
    }

    max_fd += 0x1000;
    krecalloc(free_fd_nodes, max_fd);
    return activate_fd();
}

int open(char *path, int flags) {
    fd_node_t *node = kmalloc(sizeof(fd_node_t));

    int fd = activate_fd();
    
    *node = (fd_node_t) {   .path = kmalloc(strlen(path)),
                            .flags = flags,
                            .fd = fd
                        };

    strcpy(node->path, path);

    if((flags & O_CREAT) == O_CREAT) {
        fs_touch(path, 0);
    }

    fd_push_node(fd_list, node); 

    return fd;
}

int close(int fd_idx) {
    fd_node_t *fd = find_fd_node(fd_list, fd_idx);
    if(!fd)
        return -1;

    BM_CLEAR(free_fd_nodes, fd->fd);
    fd_remove_node(&fd_list, fd); 
    return 0;
}

int read(int fd_idx, void *buf, uint64_t cnt) {
    fd_node_t *fd = find_fd_node(fd_list, fd_idx);
    if(!fd)
        return -1;

    int ret = fs_read(fd->path, fd->file_offset, cnt, buf);
    fd->file_offset += cnt;
    return ret;
} 

int write(int fd_idx, void *buf, uint64_t cnt) {
    fd_node_t *fd = find_fd_node(fd_list, fd_idx);
    if(!fd)
        return -1;

    int ret = fs_write(fd->path, fd->file_offset, cnt, buf);
    fd->file_offset += cnt;
    return ret;
}

int lseek(int fd_idx, off_t offset, int whence) {
    fd_node_t *fd = find_fd_node(fd_list, fd_idx);
    if(!fd)
        return -1;

    switch(whence) {
        case SEEK_SET:
            fd->file_offset = offset;
            break;
        case SEEK_CUR:
            fd->file_offset += offset;
            break;
        case SEEK_END:
            break;
        default:
            return -1;
    }

    return fd->file_offset;
}

int dup(int fd_idx) {
    fd_node_t *fd = find_fd_node(fd_list, fd_idx);
    if(!fd)
        return -1;

    fd_node_t *node = kmalloc(sizeof(fd_node_t));
    int new_fd = activate_fd();

    *node = *fd;
    node->fd = new_fd;

    return new_fd;
}

int dup2(int old_fd, int new_fd) {
    fd_node_t *old_node = find_fd_node(fd_list, old_fd);
    if(!old_node)
        return -1;

    fd_node_t *new_node = find_fd_node(fd_list, new_fd);
    if(new_node)
        close(new_fd);
    else 
        new_node = kmalloc(sizeof(fd_node_t));

    *new_node = *old_node;
    new_node->fd = activate_fd();

    return new_fd;
}

int mkdir(char *path, uint16_t permissions) {
    return fs_mkdir(path, permissions);
}

void init_fd() {
    fd_list = kmalloc(sizeof(fd_node_t));
    free_fd_nodes = kcalloc(max_fd / 8);

    open("/stdin", O_CREAT);
    open("/stdout", O_CREAT);
}
