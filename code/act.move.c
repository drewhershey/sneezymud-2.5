#include <stdio.h>
#include <string.h>

#include "accessors.h"
#include "bit_ops.h"
#include "character_flags.h"
#include "comm.h"
#include "constants.h"
#include "db.h"
#include "game_constants.h"
#include "games.h"
#include "handler.h"
#include "interpreter.h"
#include "multiclass.h"
#include "object_flags.h"
#include "room_flags.h"
#include "spell_ids.h"
#include "structs.h"
#include "trap.h"
#include "utils.h"

void open_door(struct char_data* ch, int dir) {
  struct room_direction_data* exitp = nullptr;
  struct room_direction_data* back = nullptr;
  struct room_data* rp = nullptr;
  char buf[MAX_INPUT_LENGTH];

  rp = real_roomp(ch->in_room);
  if (rp == nullptr) {
    sprintf(buf, "nullptr rp in open_door() for %s.", PERS(ch, ch));
    vlog(buf);
    return;
  }

  exitp = rp->dir_option[dir];
  if (exitp == nullptr) {
    vlog("nullptr exit in open_door()");
    return;
  }

  REMOVE_BIT(exitp->exit_info, EX_CLOSED);
  if (exitp->keyword != nullptr) {
    if (!IS_SET(exitp->exit_info, EX_SECRET)) {
      sprintf(buf, "$n opens the %s", fname(exitp->keyword));
      act(buf, 0, ch, nullptr, nullptr, TO_ROOM);
    } else {
      act("$n reveals a hidden passage!", 0, ch, nullptr, nullptr, TO_ROOM);
    }
  } else {
    act("$n opens the door.", 0, ch, nullptr, nullptr, TO_ROOM);
  }

  /* now for opening the OTHER side of the door! */
  if ((exit_ok(exitp, &rp) != 0) &&
      ((back = rp->dir_option[rev_dir[dir]]) != nullptr) &&
      (back->to_room == ch->in_room)) {
    REMOVE_BIT(back->exit_info, EX_CLOSED);
    if (back->keyword != nullptr) {
      sprintf(buf, "The %s is opened from the other side.\n\r",
        fname(back->keyword));
      send_to_room(buf, exitp->to_room);
    } else {
      send_to_room("The door is opened from the other side.\n\r",
        exitp->to_room);
    }
  }
}

/*
  Some new movement commands for diku-mud.. a bit more object oriented.
  remove all necessary bits and send messages
*/
void raw_open_door(struct char_data* ch, int dir) {
  struct room_direction_data* exitp = nullptr;
  struct room_direction_data* back = nullptr;
  struct room_data* rp = nullptr;
  char buf[MAX_INPUT_LENGTH];

  rp = real_roomp(ch->in_room);
  if (rp == nullptr) {
    sprintf(buf, "nullptr rp in raw_open_door() for %s.", PERS(ch, ch));
    vlog(buf);
    return;
  }

  exitp = rp->dir_option[dir];
  if (exitp == nullptr) {
    vlog("nullptr exit in raw_open_door()");
    return;
  }

  REMOVE_BIT(exitp->exit_info, EX_CLOSED);
  /* now for opening the OTHER side of the door! */
  if ((exit_ok(exitp, &rp) != 0) &&
      ((back = rp->dir_option[rev_dir[dir]]) != nullptr) &&
      (back->to_room == ch->in_room)) {
    REMOVE_BIT(back->exit_info, EX_CLOSED);
    if ((back->keyword != nullptr) &&
        (strcmp("secret", fname(back->keyword)) != 0)) {
      sprintf(buf, "The %s is opened from the other side.\n\r",
        fname(back->keyword));
      send_to_room(buf, exitp->to_room);
    } else {
      send_to_room("The door is opened from the other side.\n\r",
        exitp->to_room);
    }
  }
}

static void not_legal_move(struct char_data* ch) {
  send_to_char("Alas, you cannot go that way...\n\r", ch);
}

int ValidMove(struct char_data* ch, int cmd) {
  char tmp[256];
  struct room_direction_data* exitp = nullptr;

  exitp = EXIT(ch, cmd);

  if (exit_ok(exitp, nullptr) == 0) {
    not_legal_move(ch);
    return 0;
  }
  if (IS_SET(exitp->exit_info, EX_CLOSED)) {
    if (IS_IMMORTAL(ch)) {
      if (!IS_SET(ch->specials.act, PLR_STEALTH)) {
        act("$n's body splits into a cloud of atoms before before your eyes!",
          0, ch, nullptr, nullptr, TO_ROOM);
      }
      send_to_char("You make yourself ethreal to pass through the barrier.\n\r",
        ch);
      return 1;
    }
    if (exitp->keyword != nullptr) {
      if (!IS_SET(exitp->exit_info, EX_SECRET)) {
        sprintf(tmp, "The %s seems to be closed.\n\r", fname(exitp->keyword));
        send_to_char(tmp, ch);
        return 0;
      }
      not_legal_move(ch);
      return 0;
    }
    not_legal_move(ch);
    return 0;
  }
  struct room_data* rp = nullptr;
  rp = real_roomp(exitp->to_room);
  if (IS_SET(rp->room_flags, TUNNEL)) {
    if ((MobCountInRoom(rp->people) > rp->moblim) && (!IS_IMMORTAL(ch))) {
      send_to_char("Sorry, there is no room to get in there.\n\r", ch);
      return 0;
    }
  }
  return 1;
}

static const int movement_loss[] = {
  1,  /* Inside     */
  2,  /* City       */
  2,  /* Field      */
  3,  /* Forest     */
  4,  /* Hills      */
  6,  /* Mountains  */
  8,  /* Swimming   */
  10, /* Unswimable */
  2,  /* Flying     */
  20, /* Submarine  */
  4   /* Desert     */
};

static int raw_move(struct char_data* ch, int dir) {
  int need_movement = 0;
  struct obj_data* obj = nullptr;
  char has_boat = 0;
  struct room_data* from_here = nullptr;
  struct room_data* to_here = nullptr;
  struct char_data* pers = nullptr;

  if (special(ch, dir + 1, "") !=
      0) { /* Check for special routines(North is 1)*/
    return 0;
  }

  if (ValidMove(ch, dir) == 0) {
    return 0;
  }

  if (IS_AFFECTED(ch, AFF_CHARM) && ((ch->master) != nullptr) &&
      (ch->in_room == ch->master->in_room)) {
    act("$n bursts into tears.", 0, ch, nullptr, nullptr, TO_ROOM);
    act("You burst into tears at the thought of leaving $N", 0, ch, nullptr,
      ch->master, TO_CHAR);

    return 0;
  }

  from_here = real_roomp(ch->in_room);
  to_here = real_roomp(from_here->dir_option[dir]->to_room);

  if (to_here == nullptr) {
    char_from_room(ch);
    char_to_room(ch, 0);

    send_to_char(
      "Uh-oh.  The ground melts beneath you as you fall into the swirling "
      "chaos.\n\r",
      ch);
    do_look(ch, "\0", 15);

    return 1;
  }

  if (IS_AFFECTED(ch, AFF_FLYING)) {
    need_movement = 1;
  } else {
    /* Clamp sector types to valid range to prevent buffer overflow */
    int from_sector = from_here->sector_type;
    int to_sector = to_here->sector_type;
    if (from_sector < 0 || from_sector > SECT_DESERT) {
      from_sector = SECT_FIELD; /* Default to field if invalid */
    }
    if (to_sector < 0 || to_sector > SECT_DESERT) {
      to_sector = SECT_FIELD; /* Default to field if invalid */
    }
    need_movement = (movement_loss[from_sector] + movement_loss[to_sector]) / 2;
  }

  /*
   **   Movement in water_nowswim
   */

  if ((from_here->sector_type == SECT_WATER_NOSWIM) ||
      (to_here->sector_type == SECT_WATER_NOSWIM)) {
    if ((!IS_AFFECTED(ch, AFF_WATERBREATH)) && (!IS_AFFECTED(ch, AFF_FLYING))) {
      has_boat = 0;
      /* See if char is carrying a boat */
      for (obj = ch->carrying; obj != nullptr; obj = obj->next_content) {
        if (obj->obj_flags.type_flag == ITEM_BOAT) {
          has_boat = 1;
        }
      }
      if (has_boat == 0) {
        send_to_char("You need a boat to go there.\n\r", ch);
        return 0;
      }
      if (has_boat != 0) {
        need_movement = 1;
      }
    }
  }

  /*
    Movement in SECT_AIR
    */
  if ((from_here->sector_type == SECT_AIR) ||
      (to_here->sector_type == SECT_AIR)) {
    if (!IS_AFFECTED(ch, AFF_FLYING)) {
      send_to_char("You would have to Fly to go there!\n\r", ch);
      return 0;
    }
  }

  /*
    Movement in SECT_UNDERWATER
    */
  if (to_here->sector_type == SECT_UNDERWATER) {
    if (IS_NPC(ch)) {
      if (!IS_AFFECTED(ch, AFF_WATERBREATH)) {
        return 0;
      }
    }
  }
  if (from_here->sector_type == SECT_UNDERWATER) {
    if (!IS_AFFECTED(ch, AFF_WATERBREATH)) {
      send_to_char("You would need gills to go there!\n\r", ch);
      return 0;
    }
  }

  if (GET_MOVE(ch) < need_movement) {
    send_to_char("You are too exhausted.\n\r", ch);
    return 0;
  }

  if (!IS_IMMORTAL(ch)) {
    if (IS_NPC(ch)) {
      GET_MOVE(ch) -= 1;
    } else {
      GET_MOVE(ch) -= need_movement;
    }
  }

  /*
   *  nail the unlucky with traps.
   */
  if (CheckForMoveTrap(ch, dir) != 0) {
    return 0;
  }

  char_from_room(ch);
  char_to_room(ch, from_here->dir_option[dir]->to_room);

  do_look(ch, "test", 15);

  if (IS_SET(to_here->room_flags, DEATH) && !IS_IMMORTAL(ch)) {
    death_cry(ch);

    if (IS_NPC(ch) && (IS_SET(ch->specials.act, ACT_POLYSELF))) {
      /*
       *   take char from storage, to room
       */
      pers = ch->desc->original;
      char_from_room(pers);
      char_to_room(pers, ch->in_room);
      SwitchStuff(ch, pers);
      zero_rent(ch);
      extract_char(ch);
      ch = pers;
    }
    zero_rent(ch);
    extract_char(ch);
    return 0;
  }

  /*
  **  do something with track
  */

  if (IS_NPC(ch)) {
    if (ch->specials.hunting != nullptr) {
      if (IS_SET(ch->specials.act, ACT_HUNTING) && (ch->desc != nullptr)) {
        WAIT_STATE(ch, PULSE_VIOLENCE);
      }
    }
  } else {
    if (ch->specials.hunting != nullptr) {
      if (IS_SET(ch->specials.act, PLR_HUNTING)) {
        send_to_char("You search for a trail\n\r", ch);
        WAIT_STATE(ch, PULSE_VIOLENCE);
      }
    }
  }

  return 1;
}

// MoveOne and MoveGroup print messages.  Raw move sends success or failure.
int DisplayMove(struct char_data* ch, int dir, int was_in, int total) {
  struct char_data* tmp_ch = nullptr;
  char tmp[256];

  for (tmp_ch = real_roomp(was_in)->people; tmp_ch != nullptr;
    tmp_ch = tmp_ch->next_in_room) {
    if ((!IS_AFFECTED(ch, AFF_SNEAK)) || (IS_IMMORTAL(tmp_ch))) {
      if ((ch != tmp_ch) && (AWAKE(tmp_ch)) && ((CAN_SEE(tmp_ch, ch)) != 0)) {
        if (total > 1) {
          if (IS_NPC(ch)) {
            sprintf(tmp, "%s leaves %s. [%d]\n\r", ch->player.short_descr,
              dirs[dir], total);
          } else {
            sprintf(tmp, "%s leaves %s. [%d]\n\r", GET_NAME(ch), dirs[dir],
              total);
          }
        } else {
          if (IS_NPC(ch)) {
            sprintf(tmp, "%s leaves %s.\n\r", ch->player.short_descr,
              dirs[dir]);
          } else {
            sprintf(tmp, "%s leaves %s\n\r", GET_NAME(ch), dirs[dir]);
          }
        }
        send_to_char(tmp, tmp_ch);
      }
    }
  }

  for (tmp_ch = real_roomp(ch->in_room)->people; tmp_ch != nullptr;
    tmp_ch = tmp_ch->next_in_room) {
    if (((!IS_AFFECTED(ch, AFF_SNEAK)) || (IS_IMMORTAL(tmp_ch))) &&
        ((CAN_SEE(tmp_ch, ch)) != 0) && (AWAKE(tmp_ch))) {
      if (tmp_ch != ch) {
        if (dir < 4) {
          sprintf(tmp, "%s has arrived from the %s.", PERS(ch, tmp_ch),
            dirs[rev_dir[dir]], total);
        } else if (dir == 4) {
          sprintf(tmp, "%s has arrived from below.", PERS(ch, tmp_ch), total);
        } else if (dir == 5) {
          sprintf(tmp, "%s has arrived from above.", PERS(ch, tmp_ch), total);
        } else {
          sprintf(tmp, "%s has arrived from somewhere.", PERS(ch, tmp_ch),
            total);
        }
        if (total > 1) {
          sprintf(tmp + strlen(tmp), " [%d]", total);
        }
        strcat(tmp, "\n\r");
        send_to_char(tmp, tmp_ch);
      }
    }
  }
  return 1;
}

static int display_one_move(struct char_data* ch, int dir, int was_in) {
  return DisplayMove(ch, dir, was_in, 1);
}

static int add_to_char_heap(struct char_data* heap[50], int* top, int total[50],
  struct char_data* k) {
  int found = 0;
  int i = 0;

  if (*top > 50) {
    return 0;
  }
  found = 0;
  for (i = 0; (i < *top && (found == 0)); i++) {
    if (*top > 0) {
      if ((IS_NPC(k)) && (k->nr == heap[i]->nr) &&
          ((heap[i]->player.short_descr) != nullptr) &&
          (strcmp(k->player.short_descr, heap[i]->player.short_descr) == 0)) {
        total[i] += 1;
        found = 1;
      }
    }
  }
  if (found == 0) {
    heap[*top] = k;
    total[*top] = 1;
    *top += 1;
  }

  return 1;
}

int MoveOne(struct char_data* ch, int dir) {
  int was_in = 0;

  was_in = ch->in_room;
  if (raw_move(ch, dir) != 0) { /* no error */
    display_one_move(ch, dir, was_in);
    return 1;
  }
  return 0;
}

static int display_group_move(struct char_data* ch, int dir, int was_in,
  int total) {
  return DisplayMove(ch, dir, was_in, total);
}

static void move_group(struct char_data* ch, int dir) {
  struct char_data* heap_ptr[50];
  int was_in = 0;
  int i = 0;
  int heap_top = 0;
  int heap_tot[50];
  struct follow_type* k = nullptr;
  struct follow_type* next_dude = nullptr;

  /*
   *   move the leader. (leader never duplicates)
   */

  was_in = ch->in_room;
  if (raw_move(ch, dir) != 0) { /* no error */
    display_one_move(ch, dir, was_in);
    if (ch->followers != nullptr) {
      heap_top = 0;
      for (k = ch->followers; k != nullptr; k = next_dude) {
        next_dude = k->next;
        /*
         *  compose a list of followers, w/heaping
         */
        if ((was_in == k->follower->in_room) &&
            (GET_POS(k->follower) >= POSITION_STANDING)) {
          act("You follow $N.", 0, k->follower, nullptr, ch, TO_CHAR);
          if (k->follower->followers != nullptr) {
            move_group(k->follower, dir);
          } else {
            if (raw_move(k->follower, dir) != 0) {
              if (add_to_char_heap(heap_ptr, &heap_top, heap_tot,
                    k->follower) == 0) {
                display_one_move(k->follower, dir, was_in);
              }
            }
          }
        }
      }
      /*
       *  now, print out the heaped display message
       */
      for (i = 0; i < heap_top; i++) {
        if (heap_tot[i] > 1) {
          display_group_move(heap_ptr[i], dir, was_in, heap_tot[i]);
        } else {
          display_one_move(heap_ptr[i], dir, was_in);
        }
      }
    }
  }
}

void do_move(struct char_data* ch, const char* argument, int cmd) {
  cmd -= 1;

  /*
   ** the move is valid, check for follower/master conflicts.
   */

  if (ch->attackers > 1) {
    send_to_char("There's too many people around, no place to flee!\n\r", ch);
    return;
  }

  if ((ch->followers == nullptr) && (ch->master == nullptr)) {
    MoveOne(ch, cmd);
  } else {
    if (ch->followers == nullptr) {
      MoveOne(ch, cmd);
    } else {
      move_group(ch, cmd);
    }
  }
}

int find_door(struct char_data* ch, char* type, char* dir) {
  char buf[MAX_STRING_LENGTH];
  int door = 0;
  const char* const dirs[] = {"north", "east", "south", "west", "up", "down",
    "\n"};
  struct room_direction_data* exitp = nullptr;

  if (*dir != 0) { /* a direction was specified */
    if ((door = search_block(dir, dirs, 0)) == -1) { /* Partial Match */
      send_to_char("That's not a direction.\n\r", ch);
      return (-1);
    }
    exitp = EXIT(ch, door);
    if (exitp != nullptr) {
      if (exitp->keyword == nullptr) {
        return (door);
      }
      if (((isname(type, exitp->keyword)) != 0) &&
          (strcmp(type, "secret") != 0)) {
        return (door);
      }
      sprintf(buf, "I see no %s there.\n\r", type);
      send_to_char(buf, ch);
      return (-1);
    }
    sprintf(buf, "I see no %s there.\n\r", type);
    send_to_char(buf, ch);
    return (-1);

  } /* try to locate the keyword */
  for (door = 0; door <= 5; door++) {
    if (((exitp = EXIT(ch, door)) != nullptr) && (exitp->keyword != nullptr) &&
        (isname(type, exitp->keyword) != 0)) {
      return (door);
    }
  }

  sprintf(buf, "I see no %s here.\n\r", type);
  send_to_char(buf, ch);
  return (-1);
}

void do_open(struct char_data* ch, const char* argument, int cmd) {
  int door = 0;
  char type[MAX_INPUT_LENGTH];
  char dir[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  struct room_direction_data* back = nullptr;
  struct obj_data* obj = nullptr;
  struct char_data* victim = nullptr;
  struct room_direction_data* exitp = nullptr;

  argument_interpreter(argument, type, dir);

  if (*type == 0) {
    send_to_char("Open what?\n\r", ch);
  } else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim,
               &obj) != 0) {
    /* this is an object */

    if (obj->obj_flags.type_flag != ITEM_CONTAINER) {
      send_to_char("That's not a container.\n\r", ch);
    } else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) {
      send_to_char("But it's already open!\n\r", ch);
    } else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE)) {
      send_to_char("You can't do that.\n\r", ch);
    } else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED)) {
      send_to_char("It seems to be locked.\n\r", ch);
    } else {
      REMOVE_BIT(obj->obj_flags.value[1], CONT_CLOSED);
      send_to_char("Ok.\n\r", ch);
      act("$n opens $p.", 0, ch, obj, nullptr, TO_ROOM);
    }
  } else if ((door = find_door(ch, type, dir)) >= 0) {
    /* perhaps it is a door */
    exitp = EXIT(ch, door);
    if (!IS_SET(exitp->exit_info, EX_ISDOOR)) {
      send_to_char("That's impossible, I'm afraid.\n\r", ch);
    } else if (!IS_SET(exitp->exit_info, EX_CLOSED)) {
      send_to_char("It's already open!\n\r", ch);
    } else if (IS_SET(exitp->exit_info, EX_LOCKED)) {
      send_to_char("It seems to be locked.\n\r", ch);
    } else {
      struct room_data* rp = nullptr;

      open_door(ch, door);
      send_to_char("Ok.\n\r", ch);
    }
  }
}

void do_close(struct char_data* ch, const char* argument, int cmd) {
  int door = 0;
  char type[MAX_INPUT_LENGTH];
  char dir[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  struct room_direction_data* back = nullptr;
  struct room_direction_data* exitp = nullptr;
  struct obj_data* obj = nullptr;
  struct char_data* victim = nullptr;
  struct room_data* rp = nullptr;

  argument_interpreter(argument, type, dir);

  if (*type == 0) {
    send_to_char("Close what?\n\r", ch);
  } else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim,
               &obj) != 0) {
    /* this is an object */

    if (obj->obj_flags.type_flag != ITEM_CONTAINER) {
      send_to_char("That's not a container.\n\r", ch);
    } else if (IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) {
      send_to_char("But it's already closed!\n\r", ch);
    } else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE)) {
      send_to_char("That's impossible.\n\r", ch);
    } else {
      SET_BIT(obj->obj_flags.value[1], CONT_CLOSED);
      send_to_char("Ok.\n\r", ch);
      act("$n closes $p.", 0, ch, obj, nullptr, TO_ROOM);
    }
  } else if ((door = find_door(ch, type, dir)) >= 0) {
    /* Or a door */
    exitp = EXIT(ch, door);
    if (!IS_SET(exitp->exit_info, EX_ISDOOR)) {
      send_to_char("That's absurd.\n\r", ch);
    } else if (IS_SET(exitp->exit_info, EX_CLOSED)) {
      send_to_char("It's already closed!\n\r", ch);
    } else {
      SET_BIT(exitp->exit_info, EX_CLOSED);
      if (exitp->keyword != nullptr) {
        act("$n closes the $F.", 0, ch, nullptr, exitp->keyword, TO_ROOM);
      } else {
        act("$n closes the door.", 0, ch, nullptr, nullptr, TO_ROOM);
      }
      send_to_char("Ok.\n\r", ch);
      /* now for closing the other side, too */
      if ((exit_ok(exitp, &rp) != 0) &&
          ((back = rp->dir_option[rev_dir[door]]) != nullptr) &&
          (back->to_room == ch->in_room)) {
        SET_BIT(back->exit_info, EX_CLOSED);
        if (back->keyword != nullptr) {
          sprintf(buf, "The %s closes quietly.\n\r", back->keyword);
          send_to_room(buf, exitp->to_room);
        } else {
          send_to_room("The door closes quietly.\n\r", exitp->to_room);
        }
      }
    }
  }
}

static int has_key(struct char_data* ch, int key) {
  struct obj_data* o = nullptr;

  for (o = ch->carrying; o != nullptr; o = o->next_content) {
    if (obj_index[o->item_number].vnum == key) {
      return (1);
    }
  }

  if (ch->equipment[HOLD] != nullptr) {
    if (obj_index[ch->equipment[HOLD]->item_number].vnum == key) {
      return (1);
    }
  }

  return (0);
}

void do_lock(struct char_data* ch, const char* argument, int cmd) {
  int door = 0;
  char type[MAX_INPUT_LENGTH];
  char dir[MAX_INPUT_LENGTH];
  struct room_direction_data* back = nullptr;
  struct room_direction_data* exitp = nullptr;
  struct obj_data* obj = nullptr;
  struct char_data* victim = nullptr;
  struct room_data* rp = nullptr;

  argument_interpreter(argument, type, dir);

  if (*type == 0) {
    send_to_char("Lock what?\n\r", ch);
  } else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim,
               &obj) != 0) {
    /* this is an object */

    if (obj->obj_flags.type_flag != ITEM_CONTAINER) {
      send_to_char("That's not a container.\n\r", ch);
    } else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) {
      send_to_char("Maybe you should close it first...\n\r", ch);
    } else if (obj->obj_flags.value[2] < 0) {
      send_to_char("That thing can't be locked.\n\r", ch);
    } else if (has_key(ch, obj->obj_flags.value[2]) == 0) {
      send_to_char("You don't seem to have the proper key.\n\r", ch);
    } else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED)) {
      send_to_char("It is locked already.\n\r", ch);
    } else {
      SET_BIT(obj->obj_flags.value[1], CONT_LOCKED);
      send_to_char("*Cluck*\n\r", ch);
      act("$n locks $p - 'cluck', it says.", 0, ch, obj, nullptr, TO_ROOM);
    }
  } else if ((door = find_door(ch, type, dir)) >= 0) {
    /* a door, perhaps */
    exitp = EXIT(ch, door);

    if (!IS_SET(exitp->exit_info, EX_ISDOOR)) {
      send_to_char("That's absurd.\n\r", ch);
    } else if (!IS_SET(exitp->exit_info, EX_CLOSED)) {
      send_to_char("You have to close it first, I'm afraid.\n\r", ch);
    } else if (exitp->key < 0) {
      send_to_char("There does not seem to be any keyholes.\n\r", ch);
    } else if (has_key(ch, exitp->key) == 0) {
      send_to_char("You don't have the proper key.\n\r", ch);
    } else if (IS_SET(exitp->exit_info, EX_LOCKED)) {
      send_to_char("It's already locked!\n\r", ch);
    } else {
      SET_BIT(exitp->exit_info, EX_LOCKED);
      if (exitp->keyword != nullptr) {
        act("$n locks the $F.", 0, ch, nullptr, exitp->keyword, TO_ROOM);
      } else {
        act("$n locks the door.", 0, ch, nullptr, nullptr, TO_ROOM);
      }
      send_to_char("*Click*\n\r", ch);
      /* now for locking the other side, too */
      rp = real_roomp(exitp->to_room);
      if ((rp != nullptr) &&
          ((back = rp->dir_option[rev_dir[door]]) != nullptr) &&
          back->to_room == ch->in_room) {
        SET_BIT(back->exit_info, EX_LOCKED);
      }
    }
  }
}

void do_unlock(struct char_data* ch, const char* argument, int cmd) {
  int door = 0;
  char type[MAX_INPUT_LENGTH];
  char dir[MAX_INPUT_LENGTH];
  struct room_direction_data* back = nullptr;
  struct room_direction_data* exitp = nullptr;
  struct obj_data* obj = nullptr;
  struct char_data* victim = nullptr;
  struct room_data* rp = nullptr;

  argument_interpreter(argument, type, dir);

  if (*type == 0) {
    send_to_char("Unlock what?\n\r", ch);
  } else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim,
               &obj) != 0) {
    /* this is an object */

    if (obj->obj_flags.type_flag != ITEM_CONTAINER) {
      send_to_char("That's not a container.\n\r", ch);
    } else if (obj->obj_flags.value[2] < 0) {
      send_to_char("Odd - you can't seem to find a keyhole.\n\r", ch);
    } else if (has_key(ch, obj->obj_flags.value[2]) == 0) {
      send_to_char("You don't seem to have the proper key.\n\r", ch);
    } else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED)) {
      send_to_char("Oh.. it wasn't locked, after all.\n\r", ch);
    } else {
      REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
      send_to_char("*Click*\n\r", ch);
      act("$n unlocks $p.", 0, ch, obj, nullptr, TO_ROOM);
    }
  } else if ((door = find_door(ch, type, dir)) >= 0) {
    /* it is a door */
    exitp = EXIT(ch, door);

    if (!IS_SET(exitp->exit_info, EX_ISDOOR)) {
      send_to_char("That's absurd.\n\r", ch);
    } else if (!IS_SET(exitp->exit_info, EX_CLOSED)) {
      send_to_char("Heck.. it ain't even closed!\n\r", ch);
    } else if (exitp->key < 0) {
      send_to_char("You can't seem to spot any keyholes.\n\r", ch);
    } else if (has_key(ch, exitp->key) == 0) {
      send_to_char("You do not have the proper key for that.\n\r", ch);
    } else if (!IS_SET(exitp->exit_info, EX_LOCKED)) {
      send_to_char("It's already unlocked, it seems.\n\r", ch);
    } else {
      REMOVE_BIT(exitp->exit_info, EX_LOCKED);
      if (exitp->keyword != nullptr) {
        act("$n unlocks the $F.", 0, ch, nullptr, exitp->keyword, TO_ROOM);
      } else {
        act("$n unlocks the door.", 0, ch, nullptr, nullptr, TO_ROOM);
      }
      send_to_char("*click*\n\r", ch);
      /* now for unlocking the other side, too */
      rp = real_roomp(exitp->to_room);
      if ((rp != nullptr) &&
          ((back = rp->dir_option[rev_dir[door]]) != nullptr) &&
          back->to_room == ch->in_room) {
        REMOVE_BIT(back->exit_info, EX_LOCKED);
      }
    }
  }
}

void do_pick(struct char_data* ch, const char* argument, int cmd) {
  signed char percent = 0;
  int door = 0;
  char type[MAX_INPUT_LENGTH];
  char dir[MAX_INPUT_LENGTH];
  struct room_direction_data* back = nullptr;
  struct room_direction_data* exitp = nullptr;
  struct obj_data* obj = nullptr;
  struct char_data* victim = nullptr;
  struct room_data* rp = nullptr;

  argument_interpreter(argument, type, dir);

  percent = number(1, 101); /* 101% is a complete failure */

  if (ch->skills == nullptr) {
    send_to_char("You failed to pick the lock.\n\r", ch);
    return;
  }

  if (percent > (ch->skills[SKILL_PICK_LOCK].learned)) {
    send_to_char("You failed to pick the lock.\n\r", ch);
    return;
  }

  if ((HasClass(ch, CLASS_THIEF) == 0) && (HasClass(ch, CLASS_MONK) == 0) &&
      (!IS_IMMORTAL(ch))) {
    send_to_char("You're no thief!\n\r", ch);
    return;
  }

  if (*type == 0) {
    send_to_char("Pick what?\n\r", ch);
  } else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim,
               &obj) != 0) {
    /* this is an object */

    if (obj->obj_flags.type_flag != ITEM_CONTAINER) {
      send_to_char("That's not a container.\n\r", ch);
    } else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) {
      send_to_char("Silly - it ain't even closed!\n\r", ch);
    } else if (obj->obj_flags.value[2] < 0) {
      send_to_char("Odd - you can't seem to find a keyhole.\n\r", ch);
    } else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED)) {
      send_to_char("Oho! This thing is NOT locked!\n\r", ch);
    } else if (IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF)) {
      send_to_char("It resists your attempts at picking it.\n\r", ch);
    } else {
      REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
      send_to_char("*Click*\n\r", ch);
      act("$n fiddles with $p.", 0, ch, obj, nullptr, TO_ROOM);
    }
  } else if ((door = find_door(ch, type, dir)) >= 0) {
    exitp = EXIT(ch, door);
    if (!IS_SET(exitp->exit_info, EX_ISDOOR)) {
      send_to_char("That's absurd.\n\r", ch);
    } else if (!IS_SET(exitp->exit_info, EX_CLOSED)) {
      send_to_char("You realize that the door is already open.\n\r", ch);
    } else if (exitp->key < 0) {
      send_to_char("You can't seem to spot any lock to pick.\n\r", ch);
    } else if (!IS_SET(exitp->exit_info, EX_LOCKED)) {
      send_to_char("Oh.. it wasn't locked at all.\n\r", ch);
    } else if (IS_SET(exitp->exit_info, EX_PICKPROOF)) {
      send_to_char("You seem to be unable to pick this lock.\n\r", ch);
    } else {
      REMOVE_BIT(exitp->exit_info, EX_LOCKED);
      if (exitp->keyword != nullptr) {
        act("$n skillfully picks the lock of the $F.", 0, ch, nullptr,
          exitp->keyword, TO_ROOM);
      } else {
        act("$n picks the lock.", 1, ch, nullptr, nullptr, TO_ROOM);
      }
      send_to_char("The lock quickly yields to your skills.\n\r", ch);
      /* now for unlocking the other side, too */
      rp = real_roomp(exitp->to_room);
      if ((rp != nullptr) &&
          ((back = rp->dir_option[rev_dir[door]]) != nullptr) &&
          back->to_room == ch->in_room) {
        REMOVE_BIT(back->exit_info, EX_LOCKED);
      }
    }
  }
}

void do_enter(struct char_data* ch, const char* argument, int cmd) {
  int door = 0;
  int location = 0;
  char buf[MAX_INPUT_LENGTH];
  char tmp[MAX_STRING_LENGTH];
  struct obj_data* obj_object = nullptr;
  struct obj_data* next_obj = nullptr;

  one_argument(argument, buf);

  if ((*buf != 0) && (str_cmp(buf, "portal") != 0)) {
    for (door = 0; door <= 5; door++) {
      if (EXIT(ch, door)) {
        if (EXIT(ch, door)->keyword) {
          if (str_cmp(EXIT(ch, door)->keyword, buf) == 0) {
            do_move(ch, "", ++door);
            return;
          }
        }
      }
    }
    sprintf(tmp, "There is no %s here.\n\r", buf);
    send_to_char(tmp, ch);
  } else if (str_cmp("portal", buf) == 0) {
    obj_object = real_roomp(ch->in_room)->contents;
    ;
    if (obj_object != nullptr) {
      send_to_char(
        "You step into the portal and are thrown into another room.\n\r", ch);
      act("$n disappears as $e step into the portal.", 0, ch, nullptr, nullptr,
        TO_ROOM);
      location = obj_object->obj_flags.value[0];
      char_from_room(ch);
      char_to_room(ch, location);
      act("$n arrives in the room through a magical portal.", 0, ch, nullptr,
        nullptr, TO_ROOM);
      do_look(ch, "", 15);
    } else {
      send_to_char("There is no portal in the room.\n\r", ch);
      return;
    }
  } else if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
    send_to_char("You are already indoors.\n\r", ch);
  } else {
    /* try to locate an entrance */
    for (door = 0; door <= 5; door++) {
      if (EXIT(ch, door)) {
        if (EXIT(ch, door)->to_room != NOWHERE) {
          if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
              IS_SET(real_roomp(EXIT(ch, door)->to_room)->room_flags,
                INDOORS)) {
            do_move(ch, "", ++door);
            return;
          }
        }
      }
    }
    send_to_char("You can't seem to find anything to enter.\n\r", ch);
  }
}

void do_leave(struct char_data* ch, const char* argument, int cmd) {
  int door = 0;
  struct room_direction_data* exitp = nullptr;
  struct room_data* rp = nullptr;

  if (!IS_SET(RM_FLAGS(ch->in_room), INDOORS)) {
    send_to_char("You are outside.. where do you want to go?\n\r", ch);
  } else {
    for (door = 0; door <= 5; door++) {
      if ((exit_ok(exitp = EXIT(ch, door), &rp) != 0) &&
          !IS_SET(exitp->exit_info, EX_CLOSED) &&
          !IS_SET(rp->room_flags, INDOORS)) {
        do_move(ch, "", ++door);
        return;
      }
    }
    send_to_char("I see no obvious exits to the outside.\n\r", ch);
  }
}

void do_stand(struct char_data* ch, const char* argument, int cmd) {
  switch (GET_POS(ch)) {
    case POSITION_STANDING: {
      act("You are already standing.", 0, ch, nullptr, nullptr, TO_CHAR);
    } break;
    case POSITION_SITTING: {
      if (check_blackjack(ch) != 0) {
        do_blackjack_exit(ch);
      }
      act("You stand up.", 0, ch, nullptr, nullptr, TO_CHAR);
      act("$n clambers on $s feet.", 1, ch, nullptr, nullptr, TO_ROOM);
      GET_POS(ch) = POSITION_STANDING;
    } break;
    case POSITION_RESTING: {
      act("You stop resting, and stand up.", 0, ch, nullptr, nullptr, TO_CHAR);
      act("$n stops resting, and clambers on $s feet.", 1, ch, nullptr, nullptr,
        TO_ROOM);
      GET_POS(ch) = POSITION_STANDING;
    } break;
    case POSITION_SLEEPING: {
      act("You have to wake up first!", 0, ch, nullptr, nullptr, TO_CHAR);
    } break;
    case POSITION_FIGHTING: {
      act("Do you not consider fighting as standing?", 0, ch, nullptr, nullptr,
        TO_CHAR);
    } break;
    default: {
      act("You stop floating around, and put your feet on the ground.", 0, ch,
        nullptr, nullptr, TO_CHAR);
      act("$n stops floating around, and puts $s feet on the ground.", 1, ch,
        nullptr, nullptr, TO_ROOM);
    } break;
  }
}

void do_sit(struct char_data* ch, const char* argument, int cmd) {
  if (check_blackjack(ch) != 0) {
    if (do_blackjack_enter(ch) == 0) {
      return;
    }
  }
  if (check_slots(ch) != 0) {
    if (check_slot_player(ch) != 0) {
      send_to_char("Someone is already at this machine.\n\r", ch);
      return;
    }
  }
  switch (GET_POS(ch)) {
    case POSITION_STANDING: {
      act("You sit down.", 0, ch, nullptr, nullptr, TO_CHAR);
      act("$n sits down.", 0, ch, nullptr, nullptr, TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
    } break;
    case POSITION_SITTING: {
      send_to_char("You'r sitting already.\n\r", ch);
    } break;
    case POSITION_RESTING: {
      act("You stop resting, and sit up.", 0, ch, nullptr, nullptr, TO_CHAR);
      act("$n stops resting.", 1, ch, nullptr, nullptr, TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
    } break;
    case POSITION_SLEEPING: {
      act("You have to wake up first.", 0, ch, nullptr, nullptr, TO_CHAR);
    } break;
    case POSITION_FIGHTING: {
      act("Sit down while fighting? are you MAD?", 0, ch, nullptr, nullptr,
        TO_CHAR);
    } break;
    default: {
      act("You stop floating around, and sit down.", 0, ch, nullptr, nullptr,
        TO_CHAR);
      act("$n stops floating around, and sits down.", 1, ch, nullptr, nullptr,
        TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
    } break;
  }
}

void do_rest(struct char_data* ch, const char* argument, int cmd) {
  switch (GET_POS(ch)) {
    case POSITION_STANDING: {
      act("You sit down and rest your tired bones.", 0, ch, nullptr, nullptr,
        TO_CHAR);
      act("$n sits down and rests.", 1, ch, nullptr, nullptr, TO_ROOM);
      GET_POS(ch) = POSITION_RESTING;
    } break;
    case POSITION_SITTING: {
      if (check_blackjack(ch) != 0) {
        do_blackjack_exit(ch);
      }
      act("You rest your tired bones.", 0, ch, nullptr, nullptr, TO_CHAR);
      act("$n rests.", 1, ch, nullptr, nullptr, TO_ROOM);
      GET_POS(ch) = POSITION_RESTING;
    } break;
    case POSITION_RESTING: {
      act("You are already resting.", 0, ch, nullptr, nullptr, TO_CHAR);
    } break;
    case POSITION_SLEEPING: {
      act("You have to wake up first.", 0, ch, nullptr, nullptr, TO_CHAR);
    } break;
    case POSITION_FIGHTING: {
      act("Rest while fighting? are you MAD?", 0, ch, nullptr, nullptr,
        TO_CHAR);
    } break;
    default: {
      act("You stop floating around, and stop to rest your tired bones.", 0, ch,
        nullptr, nullptr, TO_CHAR);
      act("$n stops floating around, and rests.", 0, ch, nullptr, nullptr,
        TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
    } break;
  }
}

void do_sleep(struct char_data* ch, const char* argument, int cmd) {
  switch (GET_POS(ch)) {
    case POSITION_STANDING:
    case POSITION_RESTING: {
      send_to_char("You go to sleep.\n\r", ch);
      act("$n lies down and falls asleep.", 1, ch, nullptr, nullptr, TO_ROOM);
      GET_POS(ch) = POSITION_SLEEPING;
      break;
      case POSITION_SITTING:
        send_to_char("You go to sleep.\n\r", ch);
        act("$n lies down and falls asleep.", 1, ch, nullptr, nullptr, TO_ROOM);
        GET_POS(ch) = POSITION_SLEEPING;
        if (check_blackjack(ch) != 0) {
          do_blackjack_exit(ch);
        }
    } break;
    case POSITION_SLEEPING: {
      send_to_char("You are already sound asleep.\n\r", ch);
    } break;
    case POSITION_FIGHTING: {
      send_to_char("Sleep while fighting? are you MAD?\n\r", ch);
    } break;
    default: {
      act("You stop floating around, and lie down to sleep.", 0, ch, nullptr,
        nullptr, TO_CHAR);
      act("$n stops floating around, and lie down to sleep.", 1, ch, nullptr,
        nullptr, TO_ROOM);
      GET_POS(ch) = POSITION_SLEEPING;
    } break;
  }
}

void do_wake(struct char_data* ch, const char* argument, int cmd) {
  struct char_data* tmp_char = nullptr;
  char arg[MAX_STRING_LENGTH];

  one_argument(argument, arg);
  if (*arg != 0) {
    if (GET_POS(ch) == POSITION_SLEEPING) {
      act("You can't wake people up if you are asleep yourself!", 0, ch,
        nullptr, nullptr, TO_CHAR);
    } else {
      tmp_char = get_char_room_vis(ch, arg);
      if (tmp_char != nullptr) {
        if (tmp_char == ch) {
          act("If you want to wake yourself up, just type 'wake'", 0, ch,
            nullptr, nullptr, TO_CHAR);
        } else {
          if (GET_POS(tmp_char) == POSITION_SLEEPING) {
            if (IS_AFFECTED(tmp_char, AFF_SLEEP)) {
              act("You can not wake $M up!", 0, ch, nullptr, tmp_char, TO_CHAR);
            } else {
              if (check_blackjack(tmp_char) != 0) {
                act("You wake $M up and drag $m to $M feet.", 0, ch, nullptr,
                  tmp_char, TO_CHAR);
                GET_POS(tmp_char) = POSITION_STANDING;
                act("You are awakened and drug to your feet by $n.", 0, ch,
                  nullptr, tmp_char, TO_VICT);
              } else {
                act("You wake $M up.", 0, ch, nullptr, tmp_char, TO_CHAR);
                GET_POS(tmp_char) = POSITION_SITTING;
                act("You are awakened by $n.", 0, ch, nullptr, tmp_char,
                  TO_VICT);
              }
            }
          } else {
            act("$N is already awake.", 0, ch, nullptr, tmp_char, TO_CHAR);
          }
        }
      } else {
        send_to_char("You do not see that person here.\n\r", ch);
      }
    }
  } else {
    if (IS_AFFECTED(ch, AFF_SLEEP)) {
      send_to_char("You can't wake up!\n\r", ch);
    } else {
      if (GET_POS(ch) > POSITION_SLEEPING) {
        send_to_char("You are already awake...\n\r", ch);
      } else {
        if (check_blackjack(ch) != 0) {
          send_to_char("You wake and decide to stand to see the games.\n\r",
            ch);
          act("$n awakens and clambers to his feet.", 1, ch, nullptr, nullptr,
            TO_ROOM);
          GET_POS(ch) = POSITION_STANDING;
        } else {
          send_to_char("You wake, and sit up.\n\r", ch);
          act("$n awakens.", 1, ch, nullptr, nullptr, TO_ROOM);
          GET_POS(ch) = POSITION_SITTING;
        }
      }
    }
  }
}

void do_follow(struct char_data* ch, const char* argument, int cmd) {
  char name[160];
  struct char_data* leader = nullptr;

  only_argument(argument, name);

  if (*name != 0) {
    if ((leader = get_char_room_vis(ch, name)) == nullptr) {
      send_to_char("I see no person by that name here!\n\r", ch);
      return;
    }
  } else {
    send_to_char("Who do you wish to follow?\n\r", ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_CHARM) && ((ch->master) != nullptr)) {
    act("But you only feel like following $N!", 0, ch, nullptr, ch->master,
      TO_CHAR);

  } else { /* Not Charmed follow person */

    if (leader == ch) {
      if (ch->master == nullptr) {
        send_to_char("You are already following yourself.\n\r", ch);
        return;
      }
      stop_follower(ch);
    } else {
      if (circle_follow(ch, leader) != 0) {
        act("Sorry, but following in 'loops' is not allowed", 0, ch, nullptr,
          nullptr, TO_CHAR);
        return;
      }
      if (ch->master != nullptr) {
        stop_follower(ch);
      }

      add_follower(ch, leader);
    }
  }
}
