
#ifndef __MAP_H__
#define __MAP_H__

typedef struct Map Map;

/*
 * Allocates the structure for a new map.
 */
Map *map_new();

/*
 * Allocates a new map with a given capacity.
 */
Map *map_new_capacity(int capacity);

/*
 * Releases the memory associated with a map. It will not properly deallocate
 * the memory used by the values put in the map.
 */
void map_free(Map *map);

/*
 * Put a new key, value pair in the map. If the new entry replaces an existing
 * entry the pre-existing value will be returned. The key will be copied by the
 * value will be stored as a reference.
 */
void *map_put(Map *map, const char *key, void *value);

/*
 * Removes the value associated with the given key. If a match is found, it
 * will be returned from this function. If NULL is returned, no match was found
 * in the map.
 */
void *map_remove(Map *map, const char *key);

/*
 * Look in the map for the value associated with the given key. If no match is
 * found, NULL will be returned.
 */
void *map_get(Map *map, const char *key);

/*
 * The number of values stored in the map.
 */
int map_size(Map *map);

/*
 * Print out the map.
 */
void map_print(Map *map);

/*
 * Returns an array of the keys in the map. The caller is responsible for
 * freeing the array, but not the individual strings. No order of results is
 * guaranteed.
 */
char **map_keys(Map *map);

#endif

