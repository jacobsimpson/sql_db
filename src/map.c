
#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_SIZE 100

typedef enum {Empty, Full, Deleted} Usage;

typedef struct Entry {
    Usage usage;
    char *key;
    void *value;
} Entry;

static void entry_init(Entry *entry, const char *key, void *value) {
    entry->usage = Full;
    int len = strlen(key) + 1;
    entry->key = malloc(len * sizeof(char));
    strlcpy(entry->key, key, len);
    entry->value = value;
}

static void entry_free(Entry *entry) {
    free(entry->key);
}

struct Map {
    // The number of buckets in the map.
    int capacity;
    // The number of elements currently in the map.
    int size;
    Entry *entries;
};

static unsigned int m_calculate_hash(const char *key) {
    unsigned int result = 0;
    while (*key != 0) {
        result += *key;
        key++;
    }
    return result;
}

static void m_new_entries(Map *map) {
    map->entries = malloc(map->capacity * sizeof(Entry));
    for (int i = 0; i < map->capacity; i++) {
        map->entries[i].usage = Empty;
    }
}

static void *m_put(Map *map, const char *key, void *value);

static void m_resize(Map *map) {
    int old_capacity = map->capacity;
    if ((map->size / (float)map->capacity) > 0.75) {
        // Size up.
        map->capacity = map->capacity * 2;
    } else if ((map->size / (float)map->capacity) < 0.25
               && map->capacity > MIN_SIZE) {
        // Size down.
        map->capacity = map->capacity / 2;
    } else {
        return;
    }
    Entry *old_entries = map->entries;
    map->size = 0;
    m_new_entries(map);
    for (int i = 0; i < old_capacity; i++) {
        if (old_entries[i].usage == Full) {
            m_put(map, old_entries[i].key, old_entries[i].value);
            entry_free(&(old_entries[i]));
        }
    }
    free(old_entries);
}

static void *m_put(Map *map, const char *key, void *value) {
    unsigned int hash = m_calculate_hash(key);
    int bucket = hash % map->capacity;
    int *v = value;

    for (int i = 0; i < map->capacity; i++) {
        int loc = (i + bucket) % map->capacity;
        if (map->entries[loc].usage == Full && strcmp(key, map->entries[loc].key) == 0) {
            void *previous = map->entries[loc].value;
            map->entries[loc].value = value;
            return previous;
        } else if (map->entries[loc].usage == Empty || map->entries[loc].usage == Deleted) {
            entry_init(&(map->entries[loc]), key, value);
            map->size++;
            return NULL;
        }
    }
    return NULL;
}

static void *m_remove(Map *map, const char *key) {
    if (key == NULL) return NULL;
    unsigned int hash = m_calculate_hash(key);
    int bucket = hash % map->capacity;
    for (int i = 0; i < map->capacity; i++) {
        int loc = (i + bucket) % map->capacity;
        if (map->entries[loc].usage == Full && strcmp(key, map->entries[loc].key) == 0) {
            map->entries[loc].usage = Deleted;
            map->size--;
            void *previous = map->entries[loc].value;
            entry_free(&(map->entries[loc]));
            return previous;
        } else if (map->entries[loc].usage == Empty) {
            break;
        }
    }
    return NULL;
}

Map *map_new() {
    return map_new_capacity(MIN_SIZE);
}

Map *map_new_capacity(int capacity) {
    Map *map = malloc(sizeof(Map));
    map->capacity = capacity;
    map->size = 0;
    m_new_entries(map);
    return map;
}

void map_free(Map *map) {
    for (int i = 0; i < map->capacity; i++) {
        if (map->entries[i].usage == Full) {
            entry_free(&map->entries[i]);
        }
    }
    free(map->entries);
    free(map);
}

void *map_put(Map *map, const char *key, void *value) {
    void *result = m_put(map, key, value);
    m_resize(map);
    return result;
}

void *map_remove(Map *map, const char *key) {
    void *result = m_remove(map, key);
    m_resize(map);
    return result;
}

void *map_get(Map *map, const char *key) {
    unsigned int hash = m_calculate_hash(key);
    int bucket = hash % map->capacity;
    for (int i = 0; i < map->capacity; i++) {
        int loc = (i + bucket) % map->capacity;
        if (map->entries[loc].usage == Full && strcmp(key, map->entries[loc].key) == 0) {
            return map->entries[loc].value;
        } else if (map->entries[loc].usage == Empty) {
            break;
        }
    }
    return NULL;
}

int map_size(Map *map) {
    return map->size;
}

void map_print(Map *map) {
    printf("Map [capacity=%d, size=%d] {\n", map->capacity, map->size);
    for (int i = 0; i < map->capacity; i++) {
        if (map->entries[i].usage == Full) {
            int *value = map->entries[i].value;
            printf("    [%2d] %s = %d\n", i, map->entries[i].key, *value);
        }
    }
    printf("}\n");
}

char **map_keys(Map *map) {
    char **keys = malloc(map->size * sizeof(char*));
    for (int i = 0, j = 0; i < map->capacity; i++) {
        if (map->entries[i].usage == Full) {
            keys[j++] = map->entries[i].key;
        }
    }
    return keys;
}

