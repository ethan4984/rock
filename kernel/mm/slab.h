#ifndef SLAB_H_
#define SLAB_H_

#include <memutils.h>

#define OBJECTS_PER_SLAB 256

#define SLAB_CACHE 0x1000
#define CACHE_STATIC 0x10000

struct kmm_slab;

struct kmm_slab_cache {
    const char *name;
    size_t flags;

    size_t object_size;
    size_t active_objects;
    size_t objects_per_slab;
    size_t active_slabs;
    size_t pages_per_slab;
    
    struct kmm_slab *slab_empty;
    struct kmm_slab *slab_partial;
    struct kmm_slab *slab_full;

    struct kmm_slab_cache *next;
    struct kmm_slab_cache *last;
};

struct kmm_slab {
    size_t active_objects;

    struct kmm_slab_cache *cache_parent;

    size_t flags;
    size_t free;

    void *buf;
    uint8_t *bitmap;

    struct kmm_slab *next;
    struct kmm_slab *last;
};

struct kmm_slab_cache *kmm_create_cache(const char *name, size_t object_size, size_t flags);
void kmm_cache_destory(struct kmm_slab_cache *cache);

void *kmalloc(size_t size);
void *krealloc(void *addr, size_t size);
void *krecalloc(void *addr, size_t size);
void *kcalloc(size_t size);
size_t kfree(const void *obj);

#endif
