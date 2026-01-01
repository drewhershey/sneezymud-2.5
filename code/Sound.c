#include <stdio.h>

#include "accessors.h"
#include "bit_ops.h"
#include "character_flags.h"
#include "comm.h"
#include "db.h"
#include "object_flags.h"
#include "room_flags.h"
#include "structs.h"
#include "utils.h"

int RecGetObjRoom(struct obj_data* obj) {
  if (obj->in_room != NOWHERE) {
    return (obj->in_room);
  }
  if (obj->carried_by != nullptr) {
    return (obj->carried_by->in_room);
  }
  if (obj->equipped_by != nullptr) {
    return (obj->equipped_by->in_room);
  }
  if (obj->in_obj != nullptr) {
    return (RecGetObjRoom(obj->in_obj));
  }
  return NOWHERE;
}

void MakeNoise(int room, const char* local_snd, const char* distant_snd) {
  int door = 0;
  struct char_data* ch = nullptr;
  struct room_data* rp = nullptr;
  struct room_data* orp = nullptr;

  rp = real_roomp(room);

  if (rp == nullptr) {
    return;
  }

  for (ch = rp->people; ch != nullptr; ch = ch->next_in_room) {
    send_to_char(local_snd, ch);
  }

  for (door = 0; door <= 5; door++) {
    if ((rp->dir_option[door] != nullptr) &&
        ((orp = real_roomp(rp->dir_option[door]->to_room)) != nullptr)) {
      for (ch = orp->people; ch != nullptr; ch = ch->next_in_room) {
        if (!IS_NPC(ch) && (!IS_SET(ch->specials.act, PLR_NOSHOUT))) {
          send_to_char(distant_snd, ch);
        }
      }
    }
  }
}

void MakeSound(int pulse) {
  int room = 0;
  char buffer[128];
  struct obj_data* obj = nullptr;
  struct char_data* ch = nullptr;

  /*
   *  objects
   */

  for (obj = object_list; obj != nullptr; obj = obj->next) {
    if (ITEM_TYPE(obj) == ITEM_AUDIO) {
      if ((((obj->obj_flags.value[0]) != 0) &&
            (pulse % obj->obj_flags.value[0]) == 0) ||
          (number(0, 5) == 0)) {
        if (obj->carried_by != nullptr) {
          room = obj->carried_by->in_room;
        } else if (obj->equipped_by != nullptr) {
          room = obj->equipped_by->in_room;
        } else if (obj->in_room != NOWHERE) {
          room = obj->in_room;
        } else {
          room = RecGetObjRoom(obj);
        }
        /*
         *  broadcast to room
         */

        if (obj->action_description != nullptr) {
          MakeNoise(room, obj->action_description, obj->action_description);
        }
      }
    }
  }

  /*
   *   mobiles
   */

  for (ch = character_list; ch != nullptr; ch = ch->next) {
    if (IS_NPC(ch) && ((ch->player.sounds) != nullptr) && (number(0, 5) == 0)) {
      if (ch->specials.default_pos > POSITION_SLEEPING) {
        if (GET_POS(ch) > POSITION_SLEEPING) {
          /*
           *  Make the sound;
           */
          MakeNoise(ch->in_room, ch->player.sounds, ch->player.distant_snds);
        } else if (GET_POS(ch) == POSITION_SLEEPING) {
          /*
           * snore
           */
          sprintf(buffer, "%s snores loudly.\n\r", ch->player.short_descr);
          MakeNoise(ch->in_room, buffer, "You hear a loud snore nearby.\n\r");
        }
      } else if (GET_POS(ch) == ch->specials.default_pos) {
        /*
         * Make the sound
         */
        MakeNoise(ch->in_room, ch->player.sounds, ch->player.distant_snds);
      }
    }
  }
}
