#include <mm/slab.h>
#include <mm/vmm.h>
#include <mm/pmm.h>

static struct kmm_slab_cache slab_cache_static[] = {
    {   .next = &slab_cache_static[1], .last = NULL,
        .object_size = 32, .objects_per_slab = OBJECTS_PER_SLAB,
        .pages_per_slab = DIV_ROUNDUP(32 * OBJECTS_PER_SLAB, PAGE_SIZE)
    },

    {   .next = &slab_cache_static[2], .last = &slab_cache_static[0],
        .object_size = 64, .objects_per_slab = OBJECTS_PER_SLAB,
        .pages_per_slab = DIV_ROUNDUP(64 * OBJECTS_PER_SLAB, PAGE_SIZE)
    },

    {   .next = &slab_cache_static[3], .last = &slab_cache_static[1],
        .object_size = 128, .objects_per_slab = OBJECTS_PER_SLAB,
        .pages_per_slab = DIV_ROUNDUP(128 * OBJECTS_PER_SLAB, PAGE_SIZE)
    },

    {   .next = &slab_cache_static[4], .last = &slab_cache_static[2],
        .object_size = 256, .objects_per_slab = OBJECTS_PER_SLAB,
        .pages_per_slab = DIV_ROUNDUP(256 * OBJECTS_PER_SLAB, PAGE_SIZE)
    },

    {   .next = &slab_cache_static[5], .last = &slab_cache_static[3],
        .object_size = 512, .objects_per_slab = OBJECTS_PER_SLAB,
        .pages_per_slab = DIV_ROUNDUP(512 * OBJECTS_PER_SLAB, PAGE_SIZE)
    },

    {   .next = &slab_cache_static[6], .last = &slab_cache_static[4],
        .object_size = 1024, .objects_per_slab = OBJECTS_PER_SLAB,
        .pages_per_slab = DIV_ROUNDUP(1024 * OBJECTS_PER_SLAB, PAGE_SIZE)
    },

    {   .next = &slab_cache_static[7], .last = &slab_cache_static[5],
        .object_size = 2048,  .objects_per_slab = OBJECTS_PER_SLAB,
        .pages_per_slab = DIV_ROUNDUP(2048 * OBJECTS_PER_SLAB, PAGE_SIZE)
    },

    {   .next = &slab_cache_static[8], .last = &slab_cache_static[6],
        .object_size = 4096, .objects_per_slab = OBJECTS_PER_SLAB,
        .pages_per_slab = DIV_ROUNDUP(4096 * OBJECTS_PER_SLAB, PAGE_SIZE)
    },

    {   .next = &slab_cache_static[9], .last = &slab_cache_static[7],
        .object_size =  8192, .objects_per_slab = OBJECTS_PER_SLAB,
        .pages_per_slab = DIV_ROUNDUP(8192 * OBJECTS_PER_SLAB, PAGE_SIZE)
    },

    {   .next = &slab_cache_static[10], .last = &slab_cache_static[8],
        .object_size = 16384, .objects_per_slab = OBJECTS_PER_SLAB,
        .pages_per_slab = DIV_ROUNDUP(16384 * OBJECTS_PER_SLAB, PAGE_SIZE)
    },

    {   .next = NULL, .last = &slab_cache_static[9],
        .object_size = 32768, .objects_per_slab = OBJECTS_PER_SLAB,
        .pages_per_slab = DIV_ROUNDUP(32768 * OBJECTS_PER_SLAB, PAGE_SIZE)
    }
};

static struct kmm_slab_cache *cache_root = &slab_cache_static[0];
static size_t kmm_slab_biggest_object = 32768;

static struct kmm_slab *allocate_slab(struct kmm_slab_cache *cache, size_t flags) {
    struct kmm_slab *new_slab = (struct kmm_slab*)(pmm_alloc(cache->pages_per_slab) + HIGH_VMA);

    *new_slab = (struct kmm_slab) { .cache_parent = cache,
                                    .flags = flags,
                                    .bitmap = (void*)new_slab + sizeof(struct kmm_slab),
                                    .buf = (void*)ALIGN_UP((size_t)new_slab + sizeof(struct kmm_slab) + cache->objects_per_slab / sizeof(uint8_t), 16),
                                    .next = NULL,
                                    .last = NULL
                                  };

    memset8(new_slab->bitmap, 0, cache->objects_per_slab / sizeof(uint8_t));
    
    if(cache->slab_empty == NULL) { 
        cache->slab_empty = new_slab; 
    } else {
        struct kmm_slab *node = cache->slab_empty;
        while(node->next != NULL)
            node = node->next;
        node->next = new_slab; 
        new_slab->last = node;
    }
 
    return new_slab; 
}

struct kmm_slab_cache *kmm_create_cache(const char *name, size_t object_size, size_t flags) {
    struct kmm_slab_cache new_cache = { .name = name,
                                        .object_size = object_size,
                                        .flags = flags,
                                        .objects_per_slab = OBJECTS_PER_SLAB,
                                        .pages_per_slab = DIV_ROUNDUP(object_size * OBJECTS_PER_SLAB, PAGE_SIZE)
                                      };

    struct kmm_slab *root_slab = allocate_slab(&new_cache, SLAB_CACHE);

    *(struct kmm_slab_cache*)root_slab->buf = new_cache;
    struct kmm_slab_cache *cache_desc = (struct kmm_slab_cache*)root_slab->buf;
    root_slab->buf += sizeof(struct kmm_slab_cache);

    struct kmm_slab_cache *node = cache_root;
    while(node->next != NULL)
        node = node->next;
    node->next = cache_desc;
    cache_desc->last = node;

    if(kmm_slab_biggest_object < object_size)
        kmm_slab_biggest_object = object_size;

    return cache_desc;
}

static void free_slabs(struct kmm_slab *slabs) { 
    struct kmm_slab *slab = slabs;
    struct kmm_slab_cache *cache = slabs->cache_parent; 

    do { 
        if((slab->buf - HIGH_VMA) != NULL)
            pmm_free((size_t)slab - HIGH_VMA, cache->pages_per_slab);
        slab = slab->next;
    } while(slab != NULL);
}

static int cache_remove_slab(struct kmm_slab **head, struct kmm_slab *slab) {
    struct kmm_slab_cache *cache = (*head)->cache_parent;
    if(cache->flags & CACHE_STATIC)
        return -1;

    if(!head || !slab || slab->flags & SLAB_CACHE) 
        return -1;

    if(*head == slab)
        *head = slab->next;
    if(slab->next != NULL)
        slab->next->last = slab->last;
    if(slab->last != NULL)
        slab->last->next = slab->next; 

    pmm_free((size_t)slab - HIGH_VMA, cache->pages_per_slab);

    return 0;
}

static int cache_move_slab(struct kmm_slab **dest_head, struct kmm_slab **src_head, struct kmm_slab *src) {
    if(!src || !*src_head)
        return -1; 

    if(src->next != NULL)
        src->next->last = src->last;
    if(src->last != NULL)
        src->last->next = src->next;
    if(*src_head == src)
        *src_head = src->next;

    if(!*dest_head) {
        src->last = NULL;
        src->next = NULL;
        *dest_head = src; 
        return 0;
    }

    struct kmm_slab *node = *dest_head;
    while(node->next != NULL)
        node = node->next;
    node->next = src;
    src->last = node;

    return 0;
}

void kmm_cache_destory(struct kmm_slab_cache *cache) {
    free_slabs(cache->slab_empty);
    free_slabs(cache->slab_partial);
    free_slabs(cache->slab_full);
}

void *kmm_slab_alloc(struct kmm_slab_cache *cache) {
    struct kmm_slab *slab = NULL;
    if(cache->slab_partial) {
        slab = cache->slab_partial;
    } else {
        slab = cache->slab_empty;
    }

    if(slab == NULL) {
        slab = allocate_slab(cache, 0);          
    }

    size_t index = 0;
    for(; index < cache->objects_per_slab / sizeof(uint8_t); index++) {
        if(!BM_TEST(slab->bitmap, index))
            break; 
    }

    BM_SET(slab->bitmap, index);

    if(!slab->active_objects++) {
        cache_move_slab(&cache->slab_partial, &cache->slab_empty, slab); 
    } else if(slab->active_objects >= cache->objects_per_slab) {
        cache_move_slab(&cache->slab_full, &cache->slab_partial, slab); 
    }

    return slab->buf + (index * cache->object_size);
}

static int kmm_slab_obj_free(struct kmm_slab *slab, const void *obj) {
    if(!slab)
        return -1;

    struct kmm_slab_cache *cache = slab->cache_parent;
    if(slab->buf <= obj && (slab->buf + cache->object_size * cache->objects_per_slab) > obj) {
        size_t index = (size_t)(obj - slab->buf) / cache->object_size;
        BM_CLEAR(slab->bitmap, index);
        return 0;
    }

    return -1;
}

static size_t kmm_object_size(const void *addr) {
    if(!addr)
        return 0;

    struct kmm_slab_cache *cache = cache_root;

    do {
        struct kmm_slab *slab = cache->slab_partial;
        if(slab && slab->buf <= addr && (slab->buf + cache->object_size * cache->objects_per_slab) > addr) {
            return cache->object_size;
        }

        slab = cache->slab_full;
        if(slab && slab->buf <= addr && (slab->buf + cache->object_size * cache->objects_per_slab) > addr) {
            return cache->object_size;
        }

        cache = cache->next;
    } while(cache != NULL);

    return 0;
}

void *kmalloc(size_t size) {
    if(!size)
        return NULL;

    struct kmm_slab_cache *cache = cache_root;
    struct kmm_slab_cache *closest = cache_root;
    size_t object_abs = kmm_slab_biggest_object;

    do {
        if(cache->object_size == size)
            goto found;

        size_t abs = ABS(cache->object_size, size);
        if(abs < object_abs && cache->object_size > size) {
            object_abs = abs;
            closest = cache;
        }

        cache = cache->next;
    } while(cache != NULL);
    cache = closest;
found:
    return kmm_slab_alloc(cache);
}

size_t kfree(const void *obj) {
    if(!obj) 
        return 0;

    struct kmm_slab_cache *cache = cache_root;
    do {
        if(kmm_slab_obj_free(cache->slab_partial, obj) == 0)
            return cache->object_size;

        if(kmm_slab_obj_free(cache->slab_full, obj) == 0)
            return cache->object_size;

        cache = cache->next; 
    } while(cache != NULL);

    return 0;
}

void *krealloc(void *addr, size_t cnt) {
    size_t alloc_size = kmm_object_size(addr);
    void *new_addr = kmalloc(cnt);
    memcpy8(new_addr, addr, alloc_size);
    kfree(addr);
    return new_addr;
}

void *krecalloc(void *addr, size_t cnt) {
    size_t alloc_size = kmm_object_size(addr);
    void *new_addr = kmalloc(cnt);
    memcpy8(new_addr, addr, alloc_size);
    kfree(addr);
    return new_addr;
}

void *kcalloc(size_t cnt) {
    void *addr = kmalloc(cnt);
    if(addr == NULL)
        return NULL;

    memset8(addr, 0, cnt);

    return addr;
}
