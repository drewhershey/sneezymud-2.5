#include "hash.h"

#include <stdlib.h>

#include "memory_macros.h"

#define HASH_KEY(ht, key) ((((unsigned int)(key)) * 17) % (ht)->table_size)

/* Hash table functions - used for pathfinding */

void init_hash_table(struct hash_header* ht, int rec_size, int table_size) {
  ht->rec_size = rec_size;
  ht->table_size = table_size;
  ht->buckets =
    (struct hash_link**)calloc((size_t)table_size, sizeof(struct hash_link*));
  ht->keylist =
    (int*)malloc(sizeof(*ht->keylist) * (size_t)(ht->klistsize = 128));
  ht->klistlen = 0;
}

void hash_enter_no_key(struct hash_header* ht, int key, void* data) {
  struct hash_link* temp = create(struct hash_link, 1);

  temp->key = key;
  temp->next = ht->buckets[HASH_KEY(ht, key)];
  temp->data = data;
  ht->buckets[HASH_KEY(ht, key)] = temp;

  /* Grow keylist if needed */
  if (ht->klistlen >= ht->klistsize) {
    const int new_size = ht->klistsize * 2;
    int* new_list = nullptr;

    RECREATE(new_list, int, new_size);

    ht->keylist = new_list;
    ht->klistsize = new_size;
  }

  /* Find insertion point (keep sorted) */
  int i = ht->klistlen - 1;

  for (; i >= 0; i--) {
    if (ht->keylist[i] <= key) {
      break;
    }

    ht->keylist[i + 1] = ht->keylist[i];
  }

  ht->keylist[i + 1] = key;
  ht->klistlen++;
}

void* hash_find(struct hash_header* ht, int key) {
  struct hash_link* scan = ht->buckets[HASH_KEY(ht, key)];

  while (scan && scan->key != key) {
    scan = scan->next;
  }

  return scan ? scan->data : nullptr;
}

/* room_data array functions */

struct room_data* room_find(struct room_data* room_db[], int key) {
  return ((key < WORLD_SIZE && key > -1) ? room_db[key] : nullptr);
}

int room_remove(struct room_data* rb[], int key) {
  struct room_data* tmp;

  tmp = room_find(rb, key);

  if (tmp) {
    rb[key] = nullptr;
    free(tmp);
  }
  return (0);
}
