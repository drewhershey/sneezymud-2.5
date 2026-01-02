#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comm.h"
#include "heap.h"
#include "memory_macros.h"
#include "structs.h"

/* ignore trailing spaces and \n */
static void smart_str_cpy(char* s1, const char* s2) {
  int i = 0;

  i = (int)strlen(s2);

  /* Back up from end, skipping trailing whitespace/control chars */
  while (i > 0 && s2[i - 1] <= ' ') {
    i--;
  }

  /* null terminate s1 at the trimmed length */
  s1[i] = '\0';

  /* copy characters */
  while (i > 0) {
    i--;
    s1[i] = s2[i];
  }
}

void StringHeap(char* string, struct StrHeap* Heap) {
  unsigned char found = 0U;
  int i = 0;

  if ((string == nullptr) || (*string == 0)) {
    return; /* don't bother adding if null string */
  }

  for (i = 0; i < Heap->uniq && (found == 0U); i++) {
    if (strcmp(string, Heap->str[i].string) == 0) {
      Heap->str[i].total++;
      found = 1U;
    }
  }
  if (found == 0U) {
    if (Heap->str != nullptr) {
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
  int i = 0;

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
        act(buf, 0, ch, nullptr, nullptr, TO_ROOM);
      }
    }

    if (destroy != 0) {
      /* free everything */
      free(Heap->str[i].string);
    }
  }
  if (destroy != 0) {
    free(Heap->str);
    free(Heap);
  }
}
