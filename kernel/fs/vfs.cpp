#include <fs/vfs.hpp>
#include <fs/dev.hpp>
#include <debug.hpp>
#include <fs/ramfs.hpp>
 
namespace vfs {

int fs::read(node *vfs_node, off_t off, off_t cnt, void *buf) {
    if(vfs_node->is_directory()) {
        set_errno(eisdir);
        return -1;
    }

    if(!(vfs_node->stat_cur->st_mode & s_irusr)) {
        set_errno(ebadf); 
        return -1;
    }

    return raw_read(vfs_node, off, cnt, buf);
}

int fs::write(node *vfs_node, off_t off, off_t cnt, void *buf) {
    if(vfs_node->is_directory()) {
        set_errno(eisdir);
        return -1;
    }

    if(!(vfs_node->stat_cur->st_mode & s_iwusr)) {
        set_errno(ebadf); 
        return -1;
    }

    return raw_write(vfs_node, off, cnt, buf);
}

int fs::mkdir(node *vfs_node) {
    return raw_mkdir(vfs_node);
}

int fs::open(node *vfs_node, uint16_t status) {
    if(status & o_creat && status & o_excl) {
        set_errno(eexist);
        return -1;
    }

    return raw_open(vfs_node, status);
}

int fs::unlink(node *vfs_node) {
    return raw_unlink(vfs_node);
}

cluster::cluster(fs *filesystem) : filesystem(filesystem) {
    root_node = (node*)kmm::calloc(sizeof(node));

    root_node->name = "/";
    root_node->parent = root_node;
    root_node->stat_cur = (stat*)kmm::calloc(sizeof(stat));
}

cluster::cluster(node *vfs_node, fs *filesystem) : filesystem(filesystem) {
    root_node = (node*)kmm::calloc(sizeof(node));

    root_node->name = "/";
    root_node->parent = vfs_node;
    root_node->stat_cur = (stat*)kmm::calloc(sizeof(stat));
}

void cluster::generate_node(lib::string absolute_path, fs *override_filesystem, uint16_t mode, default_ioctl *ioctl_device) {
    if(override_filesystem == NULL) {
        node(this, filesystem, mode, absolute_path, ioctl_device);
    } else {
        node(this, override_filesystem, mode, absolute_path, ioctl_device);
    }
}

node *node::search_relative(lib::string name) {
    node *cur = (child == NULL) ? parent->child : child;

    while(cur != NULL) {
        if(cur->name == name)
            return cur;
        cur = cur->next;
    }

    return NULL;
}

node *cluster::search_absolute(lib::string absolute_path, node *vfs_node) {
    lib::vector<lib::string> sub_paths = [&]() {
        size_t start = 0;
        size_t end = absolute_path.find_first('/');
 
        lib::vector<lib::string> ret;
 
        while(end != lib::string::npos) {
            lib::string token = absolute_path.substr(start, end - start);
            if(!token.empty())
                ret.push(token);
            start = end + 1;
            end = absolute_path.find_first('/', start);
        }

        lib::string token = absolute_path.substr(start, end);
        if(!token.empty())
            ret.push(absolute_path.substr(start, end));
 
        return ret;
    } ();

    if(sub_paths.size() == 0) {
        return root_node;
    }

    node *cur = root_node;
    if(vfs_node == NULL)
        cur = root_node;

    for(size_t i = 0; i < sub_paths.size(); i++) {
        if(cur->is_directory() && cur->child_cluster != NULL) {
            cur = cur->child_cluster->root_node;
        }

        cur = cur->search_relative(sub_paths[i]);

        if(cur == NULL) 
            return NULL;
    }

    return cur;
}

node::node(cluster *parent_cluster, fs *filesystem, uint16_t mode, lib::string absolute_path, default_ioctl *ioctl_device) : filesystem(filesystem), ioctl_device(ioctl_device), parent_cluster(parent_cluster), child_cluster(NULL) {
    lib::vector<lib::string> sub_paths = [&]() {
        size_t start = 0;
        size_t end = absolute_path.find_first('/');
 
        lib::vector<lib::string> ret;
 
        while(end != lib::string::npos) {
            lib::string token = absolute_path.substr(start, end - start);
            if(!token.empty())
                ret.push(token);
            start = end + 1;
            end = absolute_path.find_first('/', start);
        }

        lib::string token = absolute_path.substr(start, end);
        if(!token.empty())
            ret.push(absolute_path.substr(start, end));
 
        return ret;
    } ();

    node *node_cur = parent_cluster->root_node;
    node *parent_cur;

    auto create_node = [](cluster *parent_cluster, fs *filesystem, node *parent, lib::string name, uint16_t st_mode) -> node* {
        if(parent == NULL)
            return parent_cluster->root_node;

        node *new_node = (node*)kmm::calloc(sizeof(node));

        new_node->name = name;
        new_node->parent_cluster = parent_cluster; 
        new_node->stat_cur = (stat*)kmm::calloc(sizeof(stat));
        new_node->filesystem = filesystem;

        new_node->stat_cur->st_mode = st_mode;

        new_node->parent = parent;
        node *cur = parent->child;

        if(parent->child == NULL) {
            parent->child = new_node;
            return new_node;
        }

        while(cur) {
            if(cur->next == NULL) {
                cur->next = new_node;
                new_node->last = cur;
                return new_node;
            }
            cur = cur->next;
        }

        return NULL;
    };

    cluster *current_cluster = parent_cluster;

    for(size_t i = 0; i < sub_paths.size(); i++) {
        parent_cur = node_cur;
        node_cur = parent_cur->search_relative(sub_paths[i]);

        if(node_cur != NULL) {
            if(node_cur->is_directory() && node_cur->child_cluster != NULL) {
                parent_cur = NULL;
                current_cluster = node_cur->child_cluster;
                node_cur = node_cur->child_cluster->root_node;
            }
        }

        if(node_cur == NULL) {
            for(; i < (sub_paths.size() - 1); i++) {
                parent_cur = create_node(current_cluster, current_cluster->filesystem, parent_cur, sub_paths[i], mode | s_ifdir);
            }
            parent_cur = create_node(current_cluster, filesystem, parent_cur, sub_paths[i], mode | s_ifreg);
            return;
        }
    }
}

lib::string get_absolute_path(node *vfs_node) {
    lib::vector<node*> node_list = [](node *vfs_node) -> lib::vector<node*> {
        lib::vector<node*> ret;
        
        node *last_node = NULL; 

        while(vfs_node) {
            if(last_node == vfs_node) 
                break;

            ret.push(vfs_node);

            last_node = vfs_node;
            vfs_node = vfs_node->parent;
        }

        return ret;
    } (vfs_node);

    lib::string ret = "";

    for(size_t i = node_list.size(); i-- > 0;) {
        if(node_list[i]->is_directory()) {
            ret += node_list[i]->name + "/";
        } else {
            ret += node_list[i]->name;
        }
    }

    return ret;
}

lib::string get_relative_path(node *vfs_node) {
    lib::vector<node*> node_list = [](node *vfs_node) -> lib::vector<node*> {
        lib::vector<node*> ret;
        
        node *last_node = NULL; 

        while(vfs_node) {
            if(last_node == vfs_node) 
                break;

            ret.push(vfs_node);

            last_node = vfs_node;
            vfs_node = vfs_node->parent;

            if(vfs_node->child_cluster)
                break;
        }

        return ret;
    } (vfs_node);

    lib::string ret = "";

    for(size_t i = node_list.size(); i-- > 0;) {
        if(node_list[i]->is_directory()) {
            ret += node_list[i]->name + "/";
        } else {
            ret += node_list[i]->name;
        }
    }

    return ret;
}

int mount(lib::string source, lib::string target) {
    node *source_node = dev::root_cluster.search_absolute(source);

    if(source_node == NULL) 
        return -1; 

    if(source_node->filesystem == NULL)
        return -1;

    dev::node source_devfs_node(source_node);
    if(source_devfs_node.vfs_node == NULL) 
        return -1;

    if(root_cluster == NULL) {
        root_cluster = source_devfs_node.filesystem->root_cluster;

        root_cluster->generate_node("/dev/", NULL, s_ifdir);
        node *dev_node = root_cluster->search_absolute(lib::string("/dev/"));

        dev::root_cluster.root_node->parent = dev_node;
        dev_node->set_cluster(&dev::root_cluster);

        root_cluster->root_node->filesystem = source_devfs_node.filesystem;
    } else {
        node *target_node = root_cluster->search_absolute(target);
        if(target_node == NULL) 
            return -1;

        target_node->set_cluster(source_devfs_node.filesystem->root_cluster);
    }

    source_devfs_node.filesystem->refresh(source_node);

    return 0;
}

struct [[gnu::packed]] winsize {
    uint16_t ws_row;
    uint16_t ws_col;
    uint16_t ws_xpixel;
    uint16_t ws_ypixel;
};

constexpr size_t tiocginsz = 0x5413;

int node::ioctl(regs *regs_cur) {
    if(ioctl_device == NULL) {
        return -1;
    }

    return ioctl_device->call(regs_cur);
}

}
