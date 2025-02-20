/*
**  Heap data structs
*/

#ifndef HEAP_H
#define HEAP_H

struct StrHeapList {
    char* string; /* the matching string */
    int total;    /* total # of occurences */
};

struct StrHeap {
    int uniq;                /* number of uniq items in list */
    struct StrHeapList* str; /* the list of strings and totals */
};

#endif
