/**
 * Hash Table Map Implementation
 *
 * Uses separate chaining (linked list of entries per bucket).
 * Average O(1) for insert/get/remove with a good hash function.
 * Automatically resizes (doubles) when load factor exceeds 0.75.
 */

#include "map.h"
#include "defs.h"

#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY  64
#define MAX_LOAD_FACTOR   0.75

typedef struct entry entry_t;
struct entry {
    char    *key;       /* internal copy of key */
    void    *value;
    entry_t *next;      /* chaining for collision handling */
};

struct map {
    entry_t    **buckets;
    size_t       capacity;
    size_t       length;
    cmp_fn       cmpfn;
    hash64_fn    hashfn;
};

static entry_t *entry_create(const void *key, size_t key_size, void *value) {
    entry_t *e = malloc(sizeof(entry_t));
    if (!e) return NULL;
    e->key = malloc(key_size);
    if (!e->key) { free(e); return NULL; }
    memcpy(e->key, key, key_size);
    e->value = value;
    e->next  = NULL;
    return e;
}

map_t *map_create(cmp_fn cmpfn, hash64_fn hashfn) {
    map_t *map = malloc(sizeof(map_t));
    if (!map) return NULL;

    map->buckets  = calloc(INITIAL_CAPACITY, sizeof(entry_t *));
    if (!map->buckets) { free(map); return NULL; }

    map->capacity = INITIAL_CAPACITY;
    map->length   = 0;
    map->cmpfn    = cmpfn;
    map->hashfn   = hashfn;
    return map;
}

void map_destroy(map_t *map, free_fn val_freefn) {
    for (size_t i = 0; i < map->capacity; i++) {
        entry_t *e = map->buckets[i];
        while (e) {
            entry_t *next = e->next;
            if (val_freefn) val_freefn(e->value);
            free(e->key);
            free(e);
            e = next;
        }
    }
    free(map->buckets);
    free(map);
}

size_t map_length(map_t *map) {
    return map->length;
}

/* Compute bucket index for a key */
static size_t bucket_idx(map_t *map, const void *key) {
    uint64_t h = map->hashfn(key);
    return (size_t)(h % (uint64_t)map->capacity);
}

/* Resize: double capacity and rehash all entries */
static void map_resize(map_t *map) {
    size_t new_cap = map->capacity * 2;
    entry_t **new_buckets = calloc(new_cap, sizeof(entry_t *));
    if (!new_buckets) return; /* fail silently, map still works just slower */

    for (size_t i = 0; i < map->capacity; i++) {
        entry_t *e = map->buckets[i];
        while (e) {
            entry_t *next  = e->next;
            uint64_t h     = map->hashfn(e->key);
            size_t   idx   = (size_t)(h % (uint64_t)new_cap);
            e->next        = new_buckets[idx];
            new_buckets[idx] = e;
            e = next;
        }
    }
    free(map->buckets);
    map->buckets  = new_buckets;
    map->capacity = new_cap;
}

void *map_insert(map_t *map, void *key, size_t key_size, void *value) {
    /* Resize if load factor exceeded */
    if ((double)(map->length + 1) / (double)map->capacity > MAX_LOAD_FACTOR) {
        map_resize(map);
    }

    size_t idx  = bucket_idx(map, key);
    entry_t *e  = map->buckets[idx];

    /* Check if key already exists */
    while (e) {
        if (map->cmpfn(e->key, key) == 0) {
            void *old = e->value;
            e->value  = value;
            return old;
        }
        e = e->next;
    }

    /* New key: create entry and prepend to chain */
    entry_t *ne = entry_create(key, key_size, value);
    if (!ne) return NULL;

    ne->next         = map->buckets[idx];
    map->buckets[idx] = ne;
    map->length++;
    return NULL;
}

void *map_remove(map_t *map, void *key) {
    size_t   idx  = bucket_idx(map, key);
    entry_t *e    = map->buckets[idx];
    entry_t *prev = NULL;

    while (e) {
        if (map->cmpfn(e->key, key) == 0) {
            if (prev) prev->next       = e->next;
            else      map->buckets[idx] = e->next;

            void *val = e->value;
            free(e->key);
            free(e);
            map->length--;
            return val;
        }
        prev = e;
        e    = e->next;
    }
    return NULL;
}

void *map_get(map_t *map, void *key) {
    size_t   idx = bucket_idx(map, key);
    entry_t *e   = map->buckets[idx];

    while (e) {
        if (map->cmpfn(e->key, key) == 0) return e->value;
        e = e->next;
    }
    return NULL;
}
