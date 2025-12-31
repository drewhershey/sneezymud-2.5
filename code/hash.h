#pragma once

#define WORLD_SIZE 30000

/* Hash table structures - used for pathfinding in skills.c */
struct hash_link {
    int key;
    struct hash_link* next;
    void* data;
};

struct hash_header {
    int rec_size;
    int table_size;
    int* keylist;
    int klistsize;
    int klistlen;
    struct hash_link** buckets;
};

/* Hash table functions for pathfinding */
void init_hash_table(struct hash_header* ht, int rec_size, int table_size);
void* hash_find(struct hash_header* ht, int key);
void hash_enter_no_key(struct hash_header* ht, int key, void* data);

/* room_data array functions */
struct room_data* room_find(struct room_data* room_db[], int key);
int room_remove(struct room_data* rb[], int key);
