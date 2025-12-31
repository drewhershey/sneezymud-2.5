#pragma once

struct char_data;

struct StrHeapList {
    char* string; /* the matching string */
    int total;    /* total # of occurences */
};

struct StrHeap {
    int uniq;                /* number of uniq items in list */
    struct StrHeapList* str; /* the list of strings and totals */
};

void StringHeap(char* string, struct StrHeap* Heap);
void DisplayStringHeap(struct StrHeap* Heap, struct char_data* ch, int type,
  int destroy);
