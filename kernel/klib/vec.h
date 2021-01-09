#ifndef VEC_H_
#define VEC_H_

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

#endif
