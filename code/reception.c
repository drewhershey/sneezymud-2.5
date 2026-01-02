#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "accessors.h"
#include "bit_ops.h"
#include "character_flags.h"
#include "comm.h"
#include "commands.h"
#include "db.h"
#include "game_constants.h"
#include "handler.h"
#include "interpreter.h"
#include "object_flags.h"
#include "room_flags.h"
#include "structs.h"
#include "utils.h"
#include "compat_types.h"
#include "spec_procs.h"

/* ************************************************************************
 * Routines used for the "Offer"                                           *
 ************************************************************************* */

char recep_offer(struct char_data* ch, struct char_data* receptionist,
  struct obj_cost* cost) {
  char buf[MAX_INPUT_LENGTH];

  cost->total_cost = 100; /* Minimum cost */
  cost->no_carried = 0;
  cost->ok = 1; /* Use if any "-1" objects */

  if (cost->ok == 0) {
    return 0;
  }

  if (cost->no_carried > MAX_OBJ_SAVE) {
    if (receptionist != nullptr) {
      sprintf(buf, "$n tells you 'Sorry, but I can't store more than %d items.",
        MAX_OBJ_SAVE);
      act(buf, 0, receptionist, nullptr, ch, TO_VICT);
    }
    return 0;
  }
  cost->total_cost = 0;

  if (cost->total_cost > GET_GOLD(ch)) {
    return 0;
  }
  return 1;
}

/* ************************************************************************
 * General save/load routines                                              *
 ************************************************************************* */
static void write_objs(FILE* fl, struct obj_file_u* st, int save) {
  int i = 0;
  char buf[80];

  fwrite(&st->owner, sizeof(st->owner), 1, fl);
  fwrite(&st->gold_left, sizeof(st->gold_left), 1, fl);
  fwrite(&st->total_cost, sizeof(st->total_cost), 1, fl);
  fwrite(&st->last_update, sizeof(st->last_update), 1, fl);
  fwrite(&st->minimum_stay, sizeof(st->minimum_stay), 1, fl);
  fwrite(&st->number, sizeof(st->number), 1, fl);

  for (i = 0; i < st->number; i++) {
    fwrite(&st->objects[i], sizeof(struct obj_file_elem), 1, fl);
  }
  if (save == 1) {
    sprintf(buf, "%s rented out with [%d] items.", st->owner, st->number);
    vlog(buf);
  }
}

void update_file(struct char_data* ch, struct obj_file_u* st, int save) {
  FILE* fl = nullptr;
  char buf[200];

  sprintf(buf, "rent/%s", lower(ch->player.name));

  if ((fl = fopen(buf, "w")) == nullptr) {
    perror("saving PC's objects");
    exit(1);
  }

  rewind(fl);

  strcpy(st->owner, GET_NAME(ch));

  write_objs(fl, st, save);

  fclose(fl);
}

/* ************************************************************************
 * Routines used to load a characters equipment from disk                  *
 ************************************************************************* */

int read_objs(FILE* fl, struct obj_file_u* st) {
  int i = 0;

  if (feof(fl) != 0) {
    fclose(fl);
    return 0;
  }
  fread(&st->owner, sizeof(st->owner), 1, fl);
  if (feof(fl) != 0) {
    fclose(fl);
    return 0;
  }
  fread(&st->gold_left, sizeof(st->gold_left), 1, fl);
  if (feof(fl) != 0) {
    fclose(fl);
    return 0;
  }
  fread(&st->total_cost, sizeof(st->total_cost), 1, fl);
  if (feof(fl) != 0) {
    fclose(fl);
    return 0;
  }
  fread(&st->last_update, sizeof(st->last_update), 1, fl);
  if (feof(fl) != 0) {
    fclose(fl);
    return 0;
  }
  fread(&st->minimum_stay, sizeof(st->minimum_stay), 1, fl);
  if (feof(fl) != 0) {
    fclose(fl);
    return 0;
  }
  fread(&st->number, sizeof(st->number), 1, fl);
  if (feof(fl) != 0) {
    fclose(fl);
    return 0;
  }

  for (i = 0; i < st->number; i++) {
    fread(&st->objects[i], sizeof(struct obj_file_elem), 1, fl);
    /*
       printf("%d [%d %d %d %d] %d %d %d %d\r\n",
        st->objects[i].item_number,
        st->objects[i].value[0],
        st->objects[i].value[1],
        st->objects[i].value[2],
        st->objects[i].value[3],
        st->objects[i].extra_flags,
        st->objects[i].weight,
        st->objects[i].timer,
        st->objects[i].bitvector);
       printf("%s %s
       %s.\r\n",st->objects[i].name,st->objects[i].sd,st->objects[i].desc);

    */
  }
  return 1;
}

static void zero_rent_by_name(char* n) {
  FILE* fl = nullptr;
  char buf[200];

  sprintf(buf, "rent/%s", lower(n));

  if ((fl = fopen(buf, "w")) == nullptr) {
    perror("saving PC's objects");
    exit(1);
  }

  fclose(fl);
}

/* ************************************************************************
 * Routines used to save a characters equipment from disk                  *
 ************************************************************************* */

/* Puts object in store, at first item which has no -1 */
static void put_obj_in_store(struct obj_data* obj, struct obj_file_u* st) {
  int j = 0;
  struct obj_file_elem* oe = nullptr;
  char buf[256];

  if (st->number >= MAX_OBJ_SAVE) {
    printf("holy shit, you want to rent more than %d items?!\n", st->number);
    return;
  }

  oe = st->objects + st->number;

  oe->item_number = obj_index[obj->item_number].vnum;
  oe->value[0] = obj->obj_flags.value[0];
  oe->value[1] = obj->obj_flags.value[1];
  oe->value[2] = obj->obj_flags.value[2];
  oe->value[3] = obj->obj_flags.value[3];

  oe->extra_flags = obj->obj_flags.extra_flags;
  oe->weight = obj->obj_flags.weight;
  oe->volume = obj->obj_flags.volume;
  oe->timer = obj->obj_flags.timer;
  oe->bitvector = obj->obj_flags.bitvector;
  oe->struct_points = obj->obj_flags.struct_points;
  oe->max_struct_points = obj->obj_flags.max_struct_points;
  oe->decay_time = obj->obj_flags.decay_time;
  oe->material_points = obj->obj_flags.material_points;

  /*  new, saving names and descrips stuff */
  if (obj->name != nullptr) {
    strcpy(oe->name, obj->name);
  } else {
    sprintf(buf, "object %d has no name!", obj_index[obj->item_number].vnum);
    vlog(buf);
  }

  if (obj->short_description != nullptr) {
    strcpy(oe->sd, obj->short_description);
  } else {
    *oe->sd = '\0';
  }
  if (obj->description != nullptr) {
    strcpy(oe->desc, obj->description);
  } else {
    *oe->desc = '\0';
  }

  /* end of new, possibly buggy stuff */

  for (j = 0; j < MAX_OBJ_AFFECT; j++) {
    oe->affected[j].location = obj->affected[j].location;
    oe->affected[j].modifier = LONG_TO_COMPAT(obj->affected[j].modifier);
  }

  st->number++;
}

static int contained_weight(struct obj_data* container) {
  struct obj_data* tmp = nullptr;
  int rval = 0;

  for (tmp = container->contains; tmp != nullptr; tmp = tmp->next_content) {
    rval += GET_OBJ_WEIGHT(tmp);
  }
  return rval;
}

/* Destroy inventory after transferring it to "store inventory" */
void obj_to_store(struct obj_data* obj, struct obj_file_u* st,
  struct char_data* ch, int do_delete) {
  if (obj == nullptr) {
    return;
  }

  obj_to_store(obj->contains, st, ch, do_delete);
  obj_to_store(obj->next_content, st, ch, do_delete);

  if ((obj->obj_flags.timer < 0) && (obj->obj_flags.timer != OBJ_NOTIMER)) {
#if NODUPLICATES
#else
    sprintf(buf,
      "You're told: '%s is just old junk, I'll throw it away for you.'\n\r",
      obj->short_description);
    send_to_char(buf, ch);
#endif
  } else if (obj->obj_flags.cost_per_day < 0) {
#if NODUPLICATES
#else
    sprintf(buf,
      "You're told: '%s is just old junk, I'll throw it away for you.'\n\r",
      obj->short_description);
    send_to_char(buf, ch);
#endif
    if (do_delete != 0) {
      if (obj->in_obj != nullptr) {
        obj_from_obj(obj);
      }
      extract_obj(obj);
    }
  } else if (obj->item_number == -1) {
    if (do_delete != 0) {
      if (obj->in_obj != nullptr) {
        obj_from_obj(obj);
      }
      extract_obj(obj);
    }
  } else {
    const int weight = contained_weight(obj);
    GET_OBJ_WEIGHT(obj) -= weight;
    put_obj_in_store(obj, st);
    GET_OBJ_WEIGHT(obj) += weight;
    if (do_delete != 0) {
      if (obj->in_obj != nullptr) {
        obj_from_obj(obj);
      }
      extract_obj(obj);
    }
  }
}

/* write the vital data of a player to the player file */
void save_obj(struct char_data* ch, struct obj_cost* cost, int do_delete) {
  static struct obj_file_u st;
  int i = 0;

  st.number = 0;
  st.gold_left = GET_GOLD(ch);
  st.total_cost = cost->total_cost;
  st.last_update = time(nullptr);
  st.minimum_stay = 0; /* XXX where does this belong? */

  for (i = 0; i < MAX_WEAR; i++) {
    if (ch->equipment[i] != nullptr) {
      if (do_delete != 0) {
        obj_to_store(unequip_char(ch, i), &st, ch, do_delete);
      } else {
        obj_to_store(ch->equipment[i], &st, ch, do_delete);
      }
    }
  }

  obj_to_store(ch->carrying, &st, ch, do_delete);
  if (do_delete != 0) {
    ch->carrying = nullptr;
  }

  update_file(ch, &st, 1);
}

/* ************************************************************************
 * Routines used to update object file, upon boot time                     *
 ************************************************************************* */

static void count_limited_items(struct obj_file_u* st) {
  int i = 0;
  int cost_per_day = 0;
  struct obj_data* obj = nullptr;

  if (st->owner[0] == 0) {
    return; /* don't count empty rent units */
  }

  for (i = 0; i < st->number; i++) {
    if (st->objects[i].item_number > -1 &&
        real_object(st->objects[i].item_number) > -1) {
      /*
       ** eek.. read in the object, and then extract it.
       ** (all this just to find rent cost.)  *sigh*
       */
      obj = read_object(st->objects[i].item_number, VIRTUAL);
      cost_per_day = obj->obj_flags.cost_per_day;
      /*
       **  if the cost is > LIM_ITEM_COST_MIN, then mark before extractin
       */
      if (cost_per_day > LIM_ITEM_COST_MIN) {
        obj_index[obj->item_number].number++;
      }
      extract_obj(obj);
    }
  }
}

void update_obj_file() {
  FILE* fl = nullptr;
  FILE* char_file = nullptr;
  struct obj_file_u st{};
  struct char_file_u ch_st{};
  int i = 0;
  long days_passed = 0;
  long secs_lost = 0;
  char buf[MAX_INPUT_LENGTH];

  if ((char_file = fopen(PLAYER_FILE, "r+")) == nullptr) {
    perror("Opening player file for reading. (reception.c, update_obj_file)");
    exit(1);
  }

  for (i = 0; i <= top_of_p_table; i++) {
    sprintf(buf, "rent/%s", player_table[i].name);
    /* r+b is for Binary Reading/Writing */
    if ((fl = fopen(buf, "r+b")) != nullptr) {
      if (read_objs(fl, &st) != 0) {
        if (str_cmp(st.owner, player_table[i].name) != 0) {
          vlog("Ack!  wrong person written into object file!");
          abort();
        } else {
          sprintf(buf, "   Processing %s[%d].", st.owner, i);
          vlog(buf);
          days_passed = ((time(nullptr) - st.last_update) / SECS_PER_REAL_DAY);
          secs_lost = ((time(nullptr) - st.last_update) % SECS_PER_REAL_DAY);

          fseek(char_file,
            (long)(player_table[i].nr * sizeof(struct char_file_u)), 0);
          fread(&ch_st, sizeof(struct char_file_u), 1, char_file);

          if (ch_st.load_room == AUTO_RENT) { /* this person was autorented */
            ch_st.load_room = NOWHERE;
            st.last_update = time(nullptr);

#if LIMITED_ITEMS
            count_limited_items(&st);
#endif
            fseek(char_file,
              (long)(player_table[i].nr * sizeof(struct char_file_u)), 0);
            fwrite(&ch_st, sizeof(struct char_file_u), 1, char_file);

            fclose(fl);
          } else {
            if (days_passed > 0) {
              if ((st.total_cost * days_passed) > st.gold_left) {
                sprintf(buf, "   Dumping %s from object file.", ch_st.name);
                vlog(buf);

                ch_st.points.gold = 0;
                ch_st.load_room = NOWHERE;
                fseek(char_file,
                  (long)(player_table[i].nr * sizeof(struct char_file_u)), 0);
                fwrite(&ch_st, sizeof(struct char_file_u), 1, char_file);

                fclose(fl);
                zero_rent_by_name(ch_st.name);

              } else {
                sprintf(buf, "   Updating %s", st.owner);
                vlog(buf);
                st.gold_left -= (st.total_cost * days_passed);
                st.last_update = time(nullptr) - secs_lost;
                rewind(fl);
                write_objs(fl, &st, 0);
                fclose(fl);
#if LIMITED_ITEMS
                count_limited_items(&st);
#endif
              }
            } else {
#if LIMITED_ITEMS
              count_limited_items(&st);
#endif
              fclose(fl);
            }
          }
        }
      }
    } else {
    }
  }
  fclose(char_file);
}

/* ************************************************************************
 * Routine Receptionist                                                    *
 ************************************************************************* */

int receptionist(struct char_data* ch, int cmd, const char* /*arg*/) {
  char buf[240];
  struct obj_cost cost{};
  struct char_data* recep = nullptr;
  struct char_data* temp_char = nullptr;
  short int save_room = 0;
  const short int action_tabel[9] = {23, 24, 36, 105, 106, 109, 111, 142, 147};

  if (ch->desc == nullptr) {
    return 0; /* You've forgot false - NPC couldn't leave */
  }

  for (temp_char = real_roomp(ch->in_room)->people;
    ((temp_char) != nullptr) && (recep == nullptr);
    temp_char = temp_char->next_in_room) {
    if (IS_MOB(temp_char)) {
      if (mob_index[temp_char->nr].func.mob_f == receptionist) {
        recep = temp_char;
      }
    }
  }

  if (recep == nullptr) {
    vlog("No receptionist.\n\r");
    exit(1);
  }

  if (IS_NPC(ch)) {
    return 0;
  }

  if ((cmd != 92) && (cmd != 93)) {
    if (cmd == 0) {
      if (recep->specials.fighting != nullptr) {
        return (citizen(recep, 0, ""));
      }
    }
    if (number(0, 30) == 0) {
      do_action(recep, "", action_tabel[number(0, 8)]);
    }
    return 0;
  }

  if (!AWAKE(recep)) {
    act("$e isn't able to talk to you...", 0, recep, nullptr, ch, TO_VICT);
    return 1;
  }

  if (IS_SET(ch->specials.act, PLR_KILLER) ||
      (IS_SET(ch->specials.act, PLR_OUTLAW))) {
    sprintf(buf, "$n tells you 'Sorry, but we don't harbor criminals.");
    act(buf, 0, recep, nullptr, ch, TO_VICT);
    return 1;
  }

  if (CAN_SEE(recep, ch) == 0) {
    act("$n says, 'I don't deal with people I can't see!'", 0, recep, nullptr,
      nullptr, TO_ROOM);
    return 1;
  }

  if (cmd == 92) { /* Rent  */
    if (recep_offer(ch, recep, &cost) != 0) {
      act("$n stores your stuff in the safe, and helps you into your chamber.",
        0, recep, nullptr, ch, TO_VICT);
      act("$n helps $N into $S private chamber.", 0, recep, nullptr, ch,
        TO_NOTVICT);

      save_obj(ch, &cost, 1);
      save_room = ch->in_room;
      extract_char(ch);
      ch->in_room = save_room;
      save_char(ch, ch->in_room);
    }

  } else { /* Offer */
    recep_offer(ch, recep, &cost);
    act("$N gives $n an offer.", 0, ch, nullptr, recep, TO_ROOM);
  }

  return 1;
}

int receptionist_for_outlaws(struct char_data* ch, int cmd,
  const char* /*arg*/) {
  char buf[240];
  struct obj_cost cost{};
  struct char_data* recep = nullptr;
  struct char_data* temp_char = nullptr;
  short int save_room = 0;
  const short int action_tabel[9] = {23, 24, 36, 105, 106, 109, 111, 142, 147};

  if (ch->desc == nullptr) {
    return 0; /* You've forgot false - NPC couldn't leave */
  }

  for (temp_char = real_roomp(ch->in_room)->people;
    ((temp_char) != nullptr) && (recep == nullptr);
    temp_char = temp_char->next_in_room) {
    if (IS_MOB(temp_char)) {
      if (mob_index[temp_char->nr].func.mob_f == receptionist_for_outlaws) {
        recep = temp_char;
      }
    }
  }

  if (recep == nullptr) {
    vlog("No receptionist.\n\r");
    exit(1);
  }

  if (IS_NPC(ch)) {
    return 0;
  }

  if ((cmd != 92) && (cmd != 93)) {
    if (cmd == 0) {
      if (recep->specials.fighting != nullptr) {
        return (citizen(recep, 0, ""));
      }
    }
    if (number(0, 30) == 0) {
      do_action(recep, "", action_tabel[number(0, 8)]);
    }
    return 0;
  }

  if (!AWAKE(recep)) {
    act("$e isn't able to talk to you...", 0, recep, nullptr, ch, TO_VICT);
    return 1;
  }

  if (CAN_SEE(recep, ch) == 0) {
    act("$n says, 'I don't deal with people I can't see!'", 0, recep, nullptr,
      nullptr

      ,
      TO_ROOM);
    return 1;
  }

  if (cmd == 92) { /* Rent  */
    if (recep_offer(ch, recep, &cost) != 0) {
      if (IS_SET(ch->specials.act, PLR_KILLER) ||
          (IS_SET(ch->specials.act, PLR_OUTLAW))) {
        sprintf(buf, "$n tells you 'Hurry, before the cops catch you!'");
        act(buf, 0, recep, nullptr, ch, TO_VICT);
      }
      act("$n stores your stuff in the safe, and helps you into your chamber.",
        0, recep, nullptr, ch, TO_VICT);
      act("$n helps $N into $S private chamber.", 0, recep, nullptr, ch,
        TO_NOTVICT);

      save_obj(ch, &cost, 1);
      save_room = ch->in_room;
      extract_char(ch);
      ch->in_room = save_room;
      save_char(ch, ch->in_room);
    }

  } else { /* Offer */
    recep_offer(ch, recep, &cost);
    act("$N gives $n an offer.", 0, ch, nullptr, recep, TO_ROOM);
  }

  return 1;
}

/*
    removes a player from the list of renters
*/

void zero_rent(struct char_data* ch) {
  if (IS_NPC(ch)) {
    return;
  }

  zero_rent_by_name(GET_NAME(ch));
}
