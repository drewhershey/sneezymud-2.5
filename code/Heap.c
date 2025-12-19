/*
**  Generic text heaping code -- all purpose
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comm.h"
#include "heap.h"
#include "structs.h"
#include "utils.h"

#define TRUE 1
#define FALSE 0

/* ignore trailing spaces and \n */
static void smart_str_cpy(char* s1, const char* s2) {
  int i;

  i = strlen(s2);
  while (s2[i] <= ' ') {
    i--;
  }

  /* null terminate s1 */
  s1[i + 1] = '\0';

  while (i >= 0) {
    s1[i] = s2[i];
    i--;
  }
}

void StringHeap(char* string, struct StrHeap* Heap) {
  unsigned char found = FALSE;
  int i;

  if (!string || !*string) {
    return; /* don't bother adding if null string */
  }

  for (i = 0; i < Heap->uniq && !found; i++) {
    if (!strcmp(string, Heap->str[i].string)) {
      Heap->str[i].total++;
      found = TRUE;
    }
  }
  if (!found) {
    if (Heap->str) {
      /* increase size by 1 */
      RECREATE(Heap->str, struct StrHeapList, (size_t)(Heap->uniq + 1));
    } else {
      CREATE(Heap->str, struct StrHeapList, 1);
    }
    Heap->str[Heap->uniq].string = (char*)malloc(strlen(string) + 1);
    smart_str_cpy(Heap->str[Heap->uniq].string, string);
    Heap->str[Heap->uniq].total = 1;
    Heap->uniq++;
  }
}

void DisplayStringHeap(struct StrHeap* Heap, struct char_data* ch, int type,
  int destroy) {
  char buf[256];
  int i;

  for (i = 0; i < Heap->uniq; i++) {
    if (type != TO_CHAR) {
      if (Heap->str[i].total > 1) {
        sprintf(buf, "%s [%d]", Heap->str[i].string, Heap->str[i].total);
      } else {
        sprintf(buf, "%s", Heap->str[i].string);
      }
    } else {
      if (Heap->str[i].total > 1) {
        sprintf(buf, "%s [%d]\n\r", Heap->str[i].string, Heap->str[i].total);
      } else {
        sprintf(buf, "%s\n\r", Heap->str[i].string);
      }
    }
    if (type == TO_CHAR) {
      send_to_char(buf, ch);
    } else {
      if (ch->in_room > -1) {
        act(buf, FALSE, ch, 0, 0, TO_ROOM);
      }
    }

    if (destroy) {
      /* free everything */
      free(Heap->str[i].string);
    }
  }
  if (destroy) {
    free(Heap->str);
    free(Heap);
  }
}
