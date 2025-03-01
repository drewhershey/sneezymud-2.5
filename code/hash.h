#ifndef HASH_H
#define HASH_H

struct hash_link {
    int key;
    struct hash_link* next;
    void* data;
};

struct hash_header {
    int rec_size;
    int table_size;
    int *keylist, klistsize, klistlen; /* this is really lame,
              AMAZINGLY lame */
    struct hash_link** buckets;
};

void* hash_find(struct hash_header* ht, int key);
void* hash_find_or_create(struct hash_header* ht, int key);
void* hash_remove(struct hash_header* ht, int key);
void init_hash_table(struct hash_header* ht, int rec_size, int table_size);

struct room_data* room_find(struct room_data* room_db[], int key);
int room_remove(struct room_data* rb[], int key);

void hash_enter_no_key(struct hash_header* ht, int key, void* data);

#define WORLD_SIZE 30000

#endif
