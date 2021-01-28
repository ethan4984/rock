#ifndef VEC_H_
#define VEC_H_

#include <sched/smp.h>
#include <bitmap.h>

#define ddl_push(type, head, new_node) ({ \
    type *tmp = (head); \
    while(tmp->next != NULL) \
        tmp = tmp->next; \
    tmp->next = (new_node); \
    (new_node)->last = tmp; \
})

#define ddl_remove(type, head, node) ({ \
    __label__ out; \
    int ret = 0; \
    if(!*(head) || !(node)) {\
        ret = -1; \
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
    ret; \
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
    for(int i = 0; i < (num); i++) { \
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
    int ret = 0; \
    if(name.data == NULL) { \
        name.data = kmalloc(sizeof(type) * 32); \
        name.size = 32; \
    } \
    if(name.current > name.size) { \
        type *tmp = kmalloc(sizeof(type) * (name.size + 32)); \
        for(size_t i = 0; i < name.size; i++) \
            tmp[i] = name.data[i]; \
        kfree(name.data); \
        name.size += 32; \
        name.data = tmp; \
    } \
    ret = name.current; \
    name.data[name.current++] = element; \
    name.element_cnt++; \
    spin_release(&name.lock); \
    ret; \
})

#define vec_search(type, name, index) ({ \
    __label__ lret; \
    type *ret = NULL; \
    spin_lock(&name.lock); \
    if(name.element_cnt <= (index)) { \
        goto lret; \
    } \
    ret = &name.data[index]; \
lret: \
    spin_release(&name.lock); \
    ret; \
})

#define vec_remove(type, name, index) ({ \
    __label__ ret; \
    int ret = 0; \
    spin_lock(&name.lock); \
    if(name.element_cnt <= index) { \
        ret = -1; \
        goto ret; \
    } \
    type *tmp = kmalloc(sizeof(type) * name.size); \
    size_t origin_cnt = 0; \
    for(size_t i = 0; i < name.element_cnt; i++) { \
        if(i != index) \
            tmp[origin_cnt++] = name.data[i]; \
    } \
    name.element_cnt--; \
    kfree(name.data); \
    name.data = tmp; \
ret: \
    spin_release(&name.lock); \
    ret; \
})

#define vec_addr_remove(type, name, addr) ({ \
    int ret = -1; \
    for(size_t i = 0; i < name.element_cnt; i++) { \
        if(&name.data[i] == (addr)) { \
            ret = vec_remove(type, name, i); \
            break; \
        } \
    } \
    ret; \
})

#define vec_delete(name) \
    kfree(name.data);

#endif
