#include "hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "structs.h"
#include "utils.h"

#define HASH_KEY(ht, key) ((((unsigned int)(key)) * 17) % (ht)->table_size)

void init_hash_table(struct hash_header* ht, int rec_size, int table_size) {
  ht->rec_size = rec_size;
  ht->table_size = table_size;
  ht->buckets = (void*)calloc(sizeof(struct hash_link**), table_size);
  ht->keylist = (void*)malloc(sizeof(*ht->keylist) * (ht->klistsize = 128));
  ht->klistlen = 0;
}

/* precondition: there is no entry for <key> yet */
void hash_enter_no_key(struct hash_header* ht, int key, void* data) {
  struct hash_link* temp = create(struct hash_link, 1);

  temp->key = key;
  temp->next = ht->buckets[HASH_KEY(ht, key)];
  temp->data = data;
  ht->buckets[HASH_KEY(ht, key)] = temp;

  // Grow keylist if needed
  if (ht->klistlen >= ht->klistsize) {
    const int new_size = ht->klistsize * 2;
    int* new_list = NULL;

    RECREATE(new_list, int, new_size);

    ht->keylist = new_list;
    ht->klistsize = new_size;
  }

  // Find insertion point
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

struct room_data* room_find(struct room_data* room_db[], int key) {
  return ((key < WORLD_SIZE && key > -1) ? room_db[key] : 0);
}

void* hash_find(struct hash_header* ht, int key) {
  struct hash_link* scan = ht->buckets[HASH_KEY(ht, key)];

  while (scan && scan->key != key)
    scan = scan->next;

  return scan ? scan->data : NULL;
}

void* hash_find_or_create(struct hash_header* ht, int key) {
  void* rval = hash_find(ht, key);

  if (rval)
    return rval;

  rval = (void*)malloc((size_t)ht->rec_size);

  hash_enter_no_key(ht, key, rval);
  return rval;
}

int room_remove(struct room_data* rb[], int key) {
  struct room_data* tmp;

  tmp = room_find(rb, key);

  if (tmp) {
    rb[key] = 0;
    free(tmp);
  }
  return (0);
}

void* hash_remove(struct hash_header* ht, int key) {
  struct hash_link** scan;

  scan = ht->buckets + HASH_KEY(ht, key);

  while (*scan && (*scan)->key != key)
    scan = &(*scan)->next;

  if (*scan) {
    int i;

    struct hash_link *temp, *aux;
    temp = (*scan)->data;
    aux = *scan;
    *scan = aux->next;
    free(aux);

    for (i = 0; i < ht->klistlen; i++)
      if (ht->keylist[i] == key)
        break;

    if (i < ht->klistlen) {
      bcopy(ht->keylist + i + 1, ht->keylist + i,
        (ht->klistlen - i) * sizeof(*ht->keylist));
      ht->klistlen--;
    }

    return temp;
  }

  return NULL;
}
