#ifndef VEC_H_
#define VEC_H_
 
#include <sched/smp.h>
#include <mm/slab.h>
 
#define ddl_push(type, head, new_node) ({ \
    type *tmp = (head); \
    while(tmp->next != NULL) \
        tmp = tmp->next; \
    tmp->next = (new_node); \
    (new_node)->last = tmp; \
})
 
#define ddl_remove(type, head, node) ({ \
    __label__ out; \
    int _ret = 0; \
    if(!*(head) || !(node)) {\
        _ret = -1; \
        goto out; \
    } \
    if(*(head) == node) \
        *(head) = (node)->next; \
    if((node)->next != NULL) \
        (node)->next->last = (node)->last; \
    if((node)->last != NULL) \
        (node)->last->next = node->next; \
    kfree(node); \
out: \
    _ret; \
})
 
#define ddl_search(type, head, var, identifier) ({ \
    __label__ out; \
    type *node = head; \
    do { \
        if(node->var == (identifier)) \
            goto out; \
        node = node->next; \
    } while(node != NULL); \
    node = NULL; \
out: \
    node; \
})
 
#define ddl_get_index(type, head, num) ({ \
    type *node = head; \
    for(int _i = 0; _i < (num); _i++) { \
        if(node->next) { \
            node = node->next; \
            continue; \
        } \
        node = NULL; \
        break; \
    } \
    node; \
})
 
#define create_vec_struct(type) \
    struct { \
        type *data; \
        size_t lock; \
        size_t size; \
        size_t element_cnt; \
        size_t current; \
    }
 
#define uninit_vec(type, name) \
    create_vec_struct(type) name;
 
#define vec(type, name) \
    create_vec_struct(type) name = { 0 };
 
#define extern_vec(type, name) \
    extern create_vec_struct(type) name;
 
#define global_vec(name) \
    typeof(name) name = { 0 };
 
#define static_vec(type, name) \
    static create_vec_struct(type) name = { 0 };
 
#define vec_push(type, name, element) ({ \
    spin_lock(&name.lock); \
    int _ret = 0; \
    if(name.data == NULL) { \
        name.data = kcalloc(sizeof(type) * 32); \
        name.size = 32; \
    } \
    if(name.current >= name.size) { \
        name.size += 32; \
        name.data = krecalloc(name.data, name.size * sizeof(type)); \
    } \
    _ret = name.current; \
    name.data[name.current++] = element; \
    name.element_cnt++; \
    spin_release(&name.lock); \
    _ret; \
})
 
#define vec_search(type, name, index) ({ \
    __label__ lret; \
    type *_ret = NULL; \
    spin_lock(&name.lock); \
    if(name.element_cnt <= (index)) { \
        goto lret; \
    } \
    _ret = &name.data[index]; \
lret: \
    spin_release(&name.lock); \
    _ret; \
})
 
#define vec_remove(type, name, index) ({ \
    __label__ lret; \
    int _ret = 0; \
    spin_lock(&name.lock); \
    if(name.element_cnt <= index) { \
        _ret = -1; \
        goto lret; \
    } \
    type *tmp = kmalloc(sizeof(type) * name.size); \
    size_t _origin_cnt = 0; \
    for(size_t _i = 0; _i < name.element_cnt; _i++) { \
        if(_i != index) \
            tmp[_origin_cnt++] = name.data[_i]; \
    } \
    name.element_cnt--; \
    name.current--; \
    kfree(name.data); \
    name.data = tmp; \
lret: \
    spin_release(&name.lock); \
    _ret; \
})
 
#define vec_addr_remove(type, name, addr) ({ \
    int _ret = -1; \
    for(size_t _i = 0; _i < name.element_cnt; _i++) { \
        if(&name.data[_i] == (addr)) { \
            _ret = vec_remove(type, name, _i); \
            break; \
        } \
    } \
    _ret; \
})
 
#define vec_delete(name) \
    kfree(name.data);

#define hash_delete(name) \
    kfree(name.data_map.data); \
    kfree(name.hash_map.data);
 
#define create_hash_struct(type) \
    struct { \
        size_t hash_cnt; \
        uninit_vec(type, data_map); \
        uninit_vec(size_t, hash_map) \
    }
 
#define uninit_hash_table(type, name) \
    create_hash_struct(type) name;
 
#define hash_table(type, name) \
    create_hash_struct(type) name = { 0 };
 
#define static_hash_table(type, name) \
    static create_hash_struct(type) name = { 0 };
 
#define extern_hash_table(type, name) \
    extern create_hash_struct(type) name;
 
#define global_hash_table(name) \
    typeof(name) name = { 0 };
 
#define hash_push(type, name, element) ({ \
    size_t _hash_index = name.hash_cnt; \
    vec_push(type, name.data_map, element); \
    vec_push(size_t, name.hash_map, name.hash_cnt++); \
    _hash_index; \
})
 
#define hash_search(type, name, hash_index) ({ \
    __label__ lret; \
    __label__ found; \
    type *_ret = NULL; \
    size_t _hash_offset = 0; \
    for(; _hash_offset < name.hash_map.element_cnt; _hash_offset++) { \
        size_t index = *vec_search(size_t, name.hash_map, _hash_offset); \
        if(index == (size_t)hash_index) \
            goto found; \
    } \
    goto lret; \
found: \
    _ret = vec_search(type, name.data_map, _hash_offset); \
lret: \
    _ret; \
})
 
#define hash_remove(type, name, hash_index) ({ \
    __label__ lret; \
    __label__ found; \
    int _ret = -1; \
    size_t _hash_offset = 0; \
    for(; _hash_offset < name.hash_map.element_cnt; _hash_offset++) { \
        size_t index = *vec_search(size_t, name.hash_map, _hash_offset); \
        if(index == hash_index) \
            goto found; \
    } \
    goto lret; \
found: \
    vec_remove(type, name.data_map, _hash_offset); \
    vec_remove(size_t, name.hash_map, _hash_offset); \
    name.hash_cnt--; \
    _ret = 0; \
lret: \
    _ret; \
})
 
#define hash_addr_remove(type, name, addr) ({ \
    __label__ lret; \
    __label__ found; \
    int _ret = -1; \
    size_t _hash_offset = 0; \
    for(; _hash_offset < name.data_map.element_cnt; _hash_offset++) { \
        type *index = vec_search(type, name.data_map, _hash_offset); \
        if(index == addr) \
            goto found; \
    } \
    goto lret; \
found: \
    vec_remove(type, name.data_map, _hash_offset); \
    vec_remove(size_t, name.hash_map, _hash_offset); \
    name.hash_cnt--; \
    _ret = 0; \
lret: \
    _ret; \
})
 
#endif
