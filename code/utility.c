/* ************************************************************************
 *  file: utility.c, Utility module.                       Part of DIKUMUD *
 *  Usage: Utility procedures                                              *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/param.h>
#include <time.h>

#include "area.h"
#include "comm.h"
#include "constants.h"
#include "db.h"
#include "handler.h"
#include "multiclass.h"
#include "opinion.h"
#include "race.h"
#include "spells.h"
#include "structs.h"
#include "utils.h"

int CAN_SEE_FOR_WHO(struct char_data* s, struct char_data* o) {
  if (!o || s->in_room < 0 || o->in_room < 0) {
    return 0;
  }

  if (IS_IMMORTAL(s)) {
    if (GetMaxLevel(s) < (o->invis_level)) {
      return 0;
    }
    return 1;
  }

  if ((GetMaxLevel(s) < (o->invis_level)) && (IS_IMMORTAL(o))) {
    return 0;
  }

  if (IS_AFFECTED(s, AFF_TRUE_SIGHT)) {
    return 1;
  }

  if (IS_AFFECTED(s, AFF_BLIND) || IS_AFFECTED(o, AFF_HIDE)) {
    return 0;
  }

  if (IS_AFFECTED(o, AFF_INVISIBLE)) {
    if (IS_IMMORTAL(o)) {
      return 0;
    }
    if (!IS_AFFECTED(s, AFF_DETECT_INVISIBLE)) {
      return 0;
    }
  }

  if ((IS_DARK(s->in_room) || IS_DARK(o->in_room)) &&
      (!IS_AFFECTED(s, AFF_INFRAVISION))) {
    return 0;
  }

  return 1;

#if 0
  ((IS_IMMORTAL(sub)) || /* gods can see anything */ \
   (((!IS_AFFECTED((obj),AFF_INVISIBLE)) || /* visible object */ \
     ((IS_AFFECTED((sub),AFF_DETECT_INVISIBLE)) && /* you detect I and */ \
      (!IS_IMMORTAL(obj)))) &&                  /* object is not a god */ \
    (!IS_AFFECTED((sub),AFF_BLIND)) &&      /* you are not blind */ \
    ( (IS_LIGHT(sub->in_room)) || (IS_AFFECTED((sub),AFF_INFRAVISION))) \
                /* there is enough light to see or you have infravision */ \
    ))
#endif
}

int CAN_SEE(struct char_data* s, struct char_data* o) {
  if (!o || s->in_room < 0 || o->in_room < 0) {
    return 0;
  }

  if (IS_IMMORTAL(s)) {
    if (GetMaxLevel(s) < (o->invis_level)) {
      return 0;
    }
    return 1;
  }

  if (GetMaxLevel(s) < (o->invis_level)) {
    return 0;
  }

  if (IS_AFFECTED(s, AFF_TRUE_SIGHT)) {
    return 1;
  }

  if (IS_AFFECTED(s, AFF_BLIND) || IS_AFFECTED(o, AFF_HIDE)) {
    return 0;
  }

  if (IS_AFFECTED(o, AFF_INVISIBLE)) {
    if (IS_IMMORTAL(o)) {
      return 0;
    }
    if (!IS_AFFECTED(s, AFF_DETECT_INVISIBLE)) {
      return 0;
    }
  }

  if ((IS_DARK(s->in_room) || IS_DARK(o->in_room)) &&
      (!IS_AFFECTED(s, AFF_INFRAVISION))) {
    return 0;
  }

  return 1;
}

void LearnFromMistake(struct char_data* ch, int sknum, int silent, int max) {
  if (!ch || !ch->skills) {
    return;
  }

  if ((ch->skills[sknum].learned < max) && (ch->skills[sknum].learned > 0)) {
    if (number(1, 101) > (ch->skills[sknum].learned / 2)) {
      if (!silent) {
        send_to_char("You learn from your mistake!\n\r", ch);
      }
      ch->skills[sknum].learned += 1;
      if (ch->skills[sknum].learned >= max) {
        if (!silent) {
          send_to_char("You are now learned in this skill!\n\r", ch);
        }
      }
    }
  }
}

int exit_ok(struct room_direction_data* exit, struct room_data** rpp) {
  struct room_data* rp;
  if (rpp == nullptr) {
    rpp = &rp;
  }
  if (!exit) {
    *rpp = nullptr;
    return 0;
  }
  *rpp = real_roomp(exit->to_room);
  return (*rpp != nullptr);
}

int ObjVnum(struct obj_data* o) {
  if (o->item_number >= 0) {
    return (obj_index[o->item_number].virtual);
  }
  return (-1);
}

void Zwrite(FILE* fp, char cmd, int tf, int arg1, int arg2, int arg3,
  char* desc) {
  char buf[100];

  if (*desc) {
    sprintf(buf, "%c %d %d %d %d   ; %s\n", cmd, tf, arg1, arg2, arg3, desc);
    fputs(buf, fp);
  } else {
    sprintf(buf, "%c %d %d %d %d\n", cmd, tf, arg1, arg2, arg3);
    fputs(buf, fp);
  }
}

int WeaponImmune(struct char_data* ch) {
  if (IS_SET(IMM_NONMAG, ch->M_immune) || IS_SET(IMM_PLUS1, ch->M_immune) ||
      IS_SET(IMM_PLUS2, ch->M_immune) || IS_SET(IMM_PLUS3, ch->M_immune) ||
      IS_SET(IMM_PLUS4, ch->M_immune)) {
    return 1;
  }
  return 0;
}

int IsImmune(struct char_data* ch, int bit) {
  return (IS_SET(bit, ch->M_immune));
}

int IsResist(struct char_data* ch, int bit) {
  return (IS_SET(bit, ch->immune));
}

int IsSusc(struct char_data* ch, int bit) { return (IS_SET(bit, ch->susc)); }

/* creates a random number in interval [from;to] */
int number(int from, int to) {
  if (to - from + 1) {
    return ((rand() % (to - from + 1)) + from);
  }
  return (from);
}

/* simulates dice roll */
int dice(int number, int size) {
  int r;
  int sum = 0;

  assert(size >= 0);

  if (size == 0) {
    return (0);
  }

  for (r = 1; r <= number; r++) {
    sum += ((rand() % size) + 1);
  }
  return (sum);
}

/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different or end of both                 */
int str_cmp(const char* arg1, const char* arg2) {
  int chk;
  int i;

  if ((!arg2) || (!arg1)) {
    return (1);
  }

  for (i = 0; *(arg1 + i) || *(arg2 + i); i++) {
    if (chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i))) {
      if (chk < 0) {
        return (-1);
      }
      return (1);
    }
  }
  return (0);
}

/* writes a string to the log */
void vlog(const char* str) {
  long ct;
  char* tmstr;
  static char buf[500];
  struct descriptor_data* i;

  ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  fprintf(stderr, "%s :: %s\n", tmstr, str);

  if (str) {
    sprintf(buf, "/* %s */\n\r", str);
  }
  for (i = descriptor_list; i; i = i->next) {
    if ((!i->connected) && (GetMaxLevel(i->character) >= LOW_IMMORTAL) &&
        (!IS_SET(i->character->specials.act, PLR_NOSHOUT))) {
      write_to_q(buf, &i->output);
    }
  }
}

void slog(const char* str) {
  long ct;
  char* tmstr;

  ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  fprintf(stderr, "%s :: %s\n", tmstr, str);
}

void sprintbit(unsigned long vektor, const char* const* names, char* result) {
  long nr;

  *result = '\0';

  for (nr = 0; vektor; vektor >>= 1) {
    if (IS_SET(1, vektor)) {
      if (*names[nr] == '\n') {
        strcat(result, "UNDEFINED");
        strcat(result, " ");
      } else if (*names[nr]) {
        strcat(result, names[nr]);
        strcat(result, " ");
      }
    }

    if (*names[nr] != '\n') {
      nr++;
    }
  }

  if (!*result) {
    strcat(result, "NOBITS");
  }
}

void sprinttype(int type, const char* const* names, char* result) {
  int nr;

  for (nr = 0; (*names[nr] != '\n'); nr++) {
    ;
  }
  if (type < nr) {
    strcpy(result, names[type]);
  } else {
    strcpy(result, "UNDEFINED");
  }
}

/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1) {
  long secs;
  struct time_info_data now;

  secs = (long)(t2 - t1);

  now.hours = (secs / SECS_PER_MUD_HOUR) % 24; /* 0..23 hours */
  secs -= SECS_PER_MUD_HOUR * now.hours;

  now.day = (secs / SECS_PER_MUD_DAY) % 35; /* 0..34 days  */
  secs -= SECS_PER_MUD_DAY * now.day;

  now.month = (secs / SECS_PER_MUD_MONTH) % 17; /* 0..16 months */
  secs -= SECS_PER_MUD_MONTH * now.month;

  now.year = (secs / SECS_PER_MUD_YEAR); /* 0..XX? years */

  return now;
}

struct time_info_data age(struct char_data* ch) {
  struct time_info_data player_age;

  player_age = mud_time_passed(time(0), ch->player.time.birth);

  player_age.year += 17; /* All players start at 17 */

  return (player_age);
}

char in_group(struct char_data* ch1, struct char_data* ch2) {
  /*
     three possibilities ->
     1.  char is char2's master
     2.  char2 is char's master
     3.  char and char2 follow same.


      otherwise not true.

  */
  if (ch1 == ch2) {
    return 1;
  }

  if ((!ch1) || (!ch2)) {
    return (0);
  }

  if ((!ch1->master) && (!ch2->master)) {
    return (0);
  }

  if (ch2->master) {
    if (!strcmp(GET_NAME(ch1), GET_NAME(ch2->master))) {
      return (1);
    }
  }

  if (ch1->master) {
    if (!strcmp(GET_NAME(ch1->master), GET_NAME(ch2))) {
      return (1);
    }
  }

  if ((ch2->master) && (ch1->master)) {
    if (!strcmp(GET_NAME(ch1->master), GET_NAME(ch2->master))) {
      return (1);
    }
  }

  return (0);
}

/*
   these two procedures give the player the ability to buy 2*bread
   or put all.bread in bag, or put 2*bread in bag...
*/
char getall(char* name, char* newname) {
  char arg[40] = "\0\0\0";
  char tmpname[80] = "\0\0\0\0\0";
  char otname[80] = "\0";
  char prd;

  sscanf(name, "%s ", otname); /* reads up to first space */
  if (strlen(otname) < 5) {
    return 0;
  }

  sscanf(otname, "%3s%c%s", arg, &prd, tmpname);

  if (prd != '.') {
    return 0;
  }
  if (tmpname == nullptr) {
    return 0;
  }
  if (strcmp(arg, "all") != 0) {
    return 0;
  }

  while (*name != '.') {
    name++;
  }

  name++;

  for (; *newname = *name; name++, newname++) {
    ;
  }

  return 1;
}

int getabunch(const char* name, char* newname) {
  if (name == nullptr || newname == nullptr) {
    return 0;
  }

  char* endptr;
  size_t num = strtoul(name, &endptr, 10);

  if (endptr == name || *endptr != '*' || *(endptr + 1) == '\0' ||
      strcpy(newname, endptr + 1) == nullptr) {
    return 0;
  }

  return MIN((int)num, 9);
}

int DetermineExp(struct char_data* mob, int exp_flags) {
  int base;
  int phit;
  int sab;
  char buf[200];

  /*
  reads in the monster, and adds the flags together
  for simplicity, 1 exceptional ability is 2 special abilities
  */

  if (GetMaxLevel(mob) < 0) {
    return (1);
  }

  switch (GetMaxLevel(mob)) {
    case 0:
      base = 5;
      phit = 1;
      sab = 10;
      break;

    case 1:
      base = 10;
      phit = 1;
      sab = 15;
      break;

    case 2:
      base = 20;
      phit = 2;
      sab = 20;
      break;

    case 3:
      base = 35;
      phit = 3;
      sab = 25;
      break;

    case 4:
      base = 60;
      phit = 4;
      sab = 30;
      break;

    case 5:
      base = 90;
      phit = 5;
      sab = 40;
      break;

    case 6:
      base = 150;
      phit = 6;
      sab = 75;
      break;

    case 7:
      base = 225;
      phit = 8;
      sab = 125;
      break;

    case 8:
      base = 600;
      phit = 12;
      sab = 175;
      break;

    case 9:
      base = 900;
      phit = 14;
      sab = 300;
      break;

    case 10:
      base = 1100;
      phit = 15;
      sab = 450;
      break;

    case 11:
      base = 1300;
      phit = 16;
      sab = 700;
      break;

    case 12:
      base = 1550;
      phit = 17;
      sab = 700;
      break;

    case 13:
      base = 1800;
      phit = 18;
      sab = 950;
      break;

    case 14:
      base = 2100;
      phit = 19;
      sab = 950;
      break;

    case 15:
      base = 2400;
      phit = 20;
      sab = 1250;
      break;

    case 16:
      base = 2700;
      phit = 23;
      sab = 1250;
      break;

    case 17:
      base = 3000;
      phit = 25;
      sab = 1550;
      break;

    case 18:
      base = 3500;
      phit = 28;
      sab = 1550;
      break;

    case 19:
      base = 4000;
      phit = 30;
      sab = 2100;
      break;

    case 20:
      base = 4500;
      phit = 33;
      sab = 2100;
      break;

    case 21:
      base = 5000;
      phit = 35;
      sab = 2600;
      break;

    case 22:
      base = 6000;
      phit = 40;
      sab = 3000;
      break;

    case 23:
      base = 7000;
      phit = 45;
      sab = 3500;
      break;

    case 24:
      base = 8000;
      phit = 50;
      sab = 4000;
      break;

    case 25:
      base = 9000;
      phit = 55;
      sab = 4500;
      break;

    case 26:
      base = 10000;
      phit = 60;
      sab = 5000;
      break;

    case 27:
      base = 12000;
      phit = 70;
      sab = 6000;
      break;

    case 28:
      base = 14000;
      phit = 80;
      sab = 7000;
      break;

    case 29:
      base = 16000;
      phit = 90;
      sab = 8000;
      break;

    case 30:
      base = 20000;
      phit = 100;
      sab = 10000;
      break;

    default:
      base = 25000;
      phit = 150;
      sab = 20000;
      break;
  }

  return (base + (phit * (GET_HIT(mob) / 2)) + ((sab * exp_flags) / 2));
}

void down_river(int pulse) {
  struct char_data* ch;
  struct char_data* tmp;
  struct obj_data* obj_object;
  struct obj_data* next_obj;
  int rd;
  int or;
  char buf[80];
  struct room_data* rp;

  if (pulse < 0) {
    return;
  }

  for (ch = character_list; ch; ch = tmp) {
    tmp = ch->next;
    if (!IS_NPC(ch)) {
      if (ch->in_room != NOWHERE) {
        if (real_roomp(ch->in_room)->sector_type == SECT_WATER_NOSWIM) {
          if ((real_roomp(ch->in_room))->river_speed > 0) {
            if ((pulse % (real_roomp(ch->in_room))->river_speed) == 0) {
              if (((real_roomp(ch->in_room))->river_dir <= 5) &&
                  ((real_roomp(ch->in_room))->river_dir >= 0)) {
                rd = (real_roomp(ch->in_room))->river_dir;
                for (obj_object = (real_roomp(ch->in_room))->contents;
                  obj_object; obj_object = next_obj) {
                  next_obj = obj_object->next_content;
                  if ((real_roomp(ch->in_room))->dir_option[rd]) {
                    obj_from_room(obj_object);
                    obj_to_room(obj_object,
                      (real_roomp(ch->in_room))->dir_option[rd]->to_room);
                  }
                }
                /*
                   flyers don't get moved
                */
                if (!IS_AFFECTED(ch, AFF_FLYING)) {
                  rp = real_roomp(ch->in_room);
                  if (rp && rp->dir_option[rd] && rp->dir_option[rd]->to_room &&
                      (EXIT(ch, rd)->to_room != NOWHERE)) {
                    if (ch->specials.fighting) {
                      stop_fighting(ch);
                    }
                    sprintf(buf, "You drift %s...\n\r", dirs[rd]);
                    send_to_char(buf, ch);
                    or = ch->in_room;
                    char_from_room(ch);
                    char_to_room(ch, (real_roomp(or))->dir_option[rd]->to_room);
                    do_look(ch, "\0", 15);

                    if (IS_SET(RM_FLAGS(ch->in_room), DEATH) &&
                        GetMaxLevel(ch) < LOW_IMMORTAL) {
                      death_cry(ch);
                      zero_rent(ch);
                      extract_char(ch);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

int IsHumanoid(struct char_data* ch) {
  /* these are all very arbitrary */

  switch (GET_RACE(ch)) {
    case RACE_HUMAN:
    case RACE_GNOME:
    case RACE_OGRE:
    case RACE_ELVEN:
    case RACE_DWARF:
    case RACE_HOBBIT:
    case RACE_ORC:
    case RACE_LYCANTH:
    case RACE_UNDEAD:
    case RACE_GIANT:
    case RACE_GOBLIN:
    case RACE_DEVIL:
    case RACE_TROLL:
    case RACE_VEGMAN:
    case RACE_MFLAYER:
    case RACE_ENFAN:
    case RACE_PATRYN:
    case RACE_SARTAN:
      return 1;
      break;

    default:
      return 0;
      break;
  }
}

int IsAnimal(struct char_data* ch) {
  switch (GET_RACE(ch)) {
    case RACE_PREDATOR:
    case RACE_FISH:
    case RACE_BIRD:
    case RACE_HERBIV:
    case RACE_LABRAT:
      return 1;
      break;
    default:
      return 0;
      break;
  }
}

int IsUndead(struct char_data* ch) {
  switch (GET_RACE(ch)) {
    case RACE_UNDEAD:
    case RACE_GHOST:
      return 1;
      break;
    default:
      return 0;
      break;
  }
}

int IsVeggie(struct char_data* ch) {
  switch (GET_RACE(ch)) {
    case RACE_PARASITE:
    case RACE_SLIME:
    case RACE_TREE:
    case RACE_VEGGIE:
    case RACE_VEGMAN:
      return 1;
      break;
    default:
      return 0;
      break;
  }
}

int IsOther(struct char_data* ch) {
  switch (GET_RACE(ch)) {
    case RACE_MFLAYER:
    case RACE_SPECIAL:
    case RACE_GOLEM:
    case RACE_ELEMENT:
    case RACE_PLANAR:
    case RACE_LYCANTH:
      return 1;
    default:
      return 0;
      break;
  }
}

int IsGiantish(struct char_data* ch) {
  switch (GET_RACE(ch)) {
    case RACE_ENFAN:
    case RACE_GOBLIN:
    case RACE_ORC:
    case RACE_GIANT:
    case RACE_TYTAN:
    case RACE_TROLL:
      return 1;
    default:
      return 0;
      break;
  }
}

int IsLycanthrope(struct char_data* ch) {
  switch (GET_RACE(ch)) {
    case RACE_LYCANTH:
      return 1;
      break;
    default:
      return 0;
      break;
  }
}

int IsDiabolic(struct char_data* ch) {
  switch (GET_RACE(ch)) {
    case RACE_DEMON:
    case RACE_DEVIL:
      return 1;
      break;
    default:
      return 0;
      break;
  }
}

int IsReptile(struct char_data* ch) {
  switch (GET_RACE(ch)) {
    case RACE_DRAGON:
    case RACE_DINOSAUR:
    case RACE_SNAKE:
      return 1;
      break;
    default:
      return 0;
      break;
  }
}

int HasHands(struct char_data* ch) {
  if (IsHumanoid(ch)) {
    return 1;
  }
  if (IsUndead(ch)) {
    return 1;
  }
  if (IsLycanthrope(ch)) {
    return 1;
  }
  if (IsDiabolic(ch)) {
    return 1;
  }
  if (GET_RACE(ch) == RACE_SPECIAL) {
    return 1;
  }
  return 0;
}

int IsPerson(struct char_data* ch) {
  switch (GET_RACE(ch)) {
    case RACE_HUMAN:
    case RACE_ELVEN:
    case RACE_DWARF:
    case RACE_HOBBIT:
    case RACE_GNOME:
      return 1;
      break;

    default:
      return 0;
      break;
  }
}

int IsExtraPlanar(struct char_data* ch) {
  switch (GET_RACE(ch)) {
    case RACE_DEMON:
    case RACE_DEVIL:
    case RACE_PLANAR:
    case RACE_ELEMENT:
      return 1;
      break;
    default:
      return 0;
      break;
  }
}

void SetHunting(struct char_data* ch, struct char_data* tch) {
  int persist;
  int dist;
  char buf[256];

#if defined(NOTRACK) && NOTRACK
  return;
#endif

  persist = GetMaxLevel(ch);
  persist *= (int)GET_ALIGNMENT(ch) / 100;

  if (persist < 0) {
    persist = -persist;
  }

  dist = GET_ALIGNMENT(tch) - GET_ALIGNMENT(ch);
  dist = (dist > 0) ? dist : -dist;
  if (Hates(ch, tch)) {
    dist *= 2;
  }

  SET_BIT(ch->specials.act, ACT_HUNTING);
  ch->specials.hunting = tch;
  ch->hunt_dist = dist;
  ch->persist = persist;
  ch->old_room = ch->in_room;

  if (GetMaxLevel(tch) >= IMMORTAL) {
    sprintf(buf, ">>%s is hunting you from %s\n\r", ch->player.short_descr,
      (real_roomp(ch->in_room))->name);
    send_to_char(buf, tch);
  }
}

void CallForGuard(struct char_data* ch, struct char_data* vict, int lev,
  int area) {
  struct char_data* i;
  int type1;
  int type2;

  switch (area) {
    case MIDGAARD:
      type1 = 3060;
      type2 = 3069;
      break;
    case NEWTHALOS:
      type1 = 3661;
      type2 = 3682;
      break;
    case TROGCAVES:
      type1 = 21114;
      type2 = 21118;
      break;
    case OUTPOST:
      type1 = 21138;
      type2 = 21139;
      break;
    case BEE:
      type1 = 29899;
      type2 = 3069;
    default:
      type1 = 3060;
      type2 = 3069;
      break;
  }

  if (lev == 0) {
    lev = 3;
  }

  for (i = character_list; i && lev > 0; i = i->next) {
    if (IS_NPC(i) && (i != ch)) {
      if (!i->specials.fighting) {
        if (mob_index[i->nr].virtual == type1) {
          if (number(1, 6) == 1) {
            if (!IS_SET(i->specials.act, ACT_HUNTING)) {
              if (vict) {
                SetHunting(i, vict);
                lev--;
              }
            }
          }
        } else if (mob_index[i->nr].virtual == type2) {
          if (number(1, 6) == 1) {
            if (!IS_SET(i->specials.act, ACT_HUNTING)) {
              if (vict) {
                SetHunting(i, vict);
                lev -= 2;
              }
            }
          }
        }
      }
    }
  }
}

void Teleport(int pulse) {
  struct char_data* ch;
  struct char_data* tmp;
  struct char_data* pers;
  struct obj_data* obj_object;
  struct obj_data* temp_obj;
  char buf[20];
  char* tmp_desc = nullptr;
  int or;
  struct room_data* rp;
  struct room_data* dest;

  if (pulse < 0) {
    return;
  }

  for (ch = character_list; ch; ch = ch->next) {
    if (IS_NPC(ch)) {
      continue;
    }
    rp = real_roomp(ch->in_room);
    if (rp && (rp)->tele_targ > 0 && rp->tele_targ != rp->number &&
        (rp)->tele_time > 0 && (pulse % (rp)->tele_time) == 0) {
      dest = real_roomp(rp->tele_targ);
      if (!dest) {
        vlog("invalid tele_targ");
        continue;
      }

      obj_object = (rp)->contents;
      while (obj_object) {
        temp_obj = obj_object->next_content;
        obj_from_room(obj_object);
        obj_to_room(obj_object, (rp)->tele_targ);
        obj_object = temp_obj;
      }

      while (rp->people /* should never fail */) {
        /* find an NPC in the room */
        for (tmp = rp->people; tmp; tmp = tmp->next_in_room) {
          if (IS_NPC(tmp)) {
            break;
          }
        }

        if (tmp == nullptr) {
          break; /* we've run out of NPCs */
        }

        or = tmp->in_room;
        char_from_room(tmp); /* the list of people in the room has changed */
        char_to_room(tmp, rp->tele_targ);
        if (IS_SET(dest->room_flags, DEATH)) {
          death_cry(tmp);
          if (IS_NPC(tmp) && (IS_SET(tmp->specials.act, ACT_POLYSELF))) {
            /*
             *   take char from storage, to room
             */
            pers = tmp->desc->original;
            char_from_room(pers);
            char_to_room(pers, tmp->in_room);
            SwitchStuff(tmp, pers);
            zero_rent(pers);
            extract_char(tmp);
            tmp = pers;
          }
          zero_rent(tmp);
          extract_char(tmp);
        }
      }
      or = ch->in_room;
      char_from_room(ch);
      char_to_room(ch, rp->tele_targ);
      if (rp->tele_look) {
        do_look(ch, "\0", 15);
      }
      if (IS_SET(dest->room_flags, DEATH) && GetMaxLevel(ch) < LOW_IMMORTAL) {
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
      }
    }
  }
}

int RecCompObjNum(struct obj_data* o, int obj_num) {
  int total = 0;
  struct obj_data* i;

  if (obj_index[o->item_number].virtual == obj_num) {
    total = 1;
  }

  if (ITEM_TYPE(o) == ITEM_CONTAINER) {
    for (i = o->contains; i; i = i->next_content) {
      total += RecCompObjNum(i, obj_num);
    }
  }
  return (total);
}

struct char_data* char_holding(struct obj_data* obj) {
  if (obj->in_room != NOWHERE) {
    return nullptr;
  }
  if (obj->carried_by) {
    return obj->carried_by;
  }
  if (obj->equipped_by) {
    return obj->equipped_by;
  }
  if (obj->in_obj) {
    return char_holding(obj->in_obj);
  }
  return nullptr;
}

void RestoreChar(struct char_data* ch) {
  GET_MANA(ch) = GET_MAX_MANA(ch);
  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);
}

void RemAllAffects(struct char_data* ch) {
  spell_dispel_magic(IMPLEMENTOR, ch, ch, 0);
}

int ObjLevelCheck(struct obj_data* obj, struct char_data* ch) {
  if ((IS_OBJ_STAT(obj, ITEM_LEVEL10) && (GetMaxLevel(ch) < 10)) ||
      (IS_OBJ_STAT(obj, ITEM_LEVEL15) && (GetMaxLevel(ch) < 15)) ||
      (IS_OBJ_STAT(obj, ITEM_LEVEL20) && (GetMaxLevel(ch) < 20)) ||
      (IS_OBJ_STAT(obj, ITEM_LEVEL25) && (GetMaxLevel(ch) < 25)) ||
      (IS_OBJ_STAT(obj, ITEM_LEVEL30) && (GetMaxLevel(ch) < 30)) ||
      (IS_OBJ_STAT(obj, ITEM_LEVEL35) && (GetMaxLevel(ch) < 35)) ||
      (IS_OBJ_STAT(obj, ITEM_LEVEL40) && (GetMaxLevel(ch) < 40))) {
    return 0;
  }
  return 1;
}

/* static void check_mobile_activity(int pulse) {
  register struct char_data* ch;
  int tick, tm;

  tm = pulse % PULSE_MOBILE;  // this is dependent on P_M = 3*P_T

  if (tm == 0) {
    tick = 0;
  } else if (tm == PULSE_TELEPORT) {
    tick = 1;
  } else if (tm == PULSE_TELEPORT * 2) {
    tick = 2;
  }

  for (ch = character_list; ch; ch = ch->next) {
    if (IS_MOB(ch)) {
      if (ch->specials.tick == tick) {
        mobile_activity(ch);
      }
    }
  }
} */

void TeleportPulseStuff(int pulse) {
  /*
    check_mobile_activity(pulse);
    Teleport(pulse);
    */

  register struct char_data* ch;
  struct char_data* next;
  struct char_data* tmp;
  struct char_data* pers;
  int tick = 0;
  int tm;
  int or;
  struct room_data* rp;
  struct room_data* dest;
  struct obj_data* obj_object;
  struct obj_data* temp_obj;
  char* tmp_desc;

  tmp_desc = nullptr;

  tm = pulse % PULSE_MOBILE; /* this is dependent on P_M = 3*P_T */

  if (tm == 0) {
    tick = 0;
  } else if (tm == PULSE_TELEPORT) {
    tick = 1;
  } else if (tm == PULSE_TELEPORT * 2) {
    tick = 2;
  }

  for (ch = character_list; ch; ch = next) {
    next = ch->next;
    if (IS_MOB(ch)) {
      if (ch->specials.tick == tick) {
        mobile_activity(ch);
      }
    } else {
      rp = real_roomp(ch->in_room);
      if (rp && (rp)->tele_targ > 0 && rp->tele_targ != rp->number &&
          (rp)->tele_time > 0 && (pulse % (rp)->tele_time) == 0) {
        dest = real_roomp(rp->tele_targ);
        if (!dest) {
          vlog("invalid tele_targ");
          continue;
        }

        obj_object = (rp)->contents;
        while (obj_object) {
          temp_obj = obj_object->next_content;
          obj_from_room(obj_object);
          obj_to_room(obj_object, (rp)->tele_targ);
          obj_object = temp_obj;
        }

        while (rp->people /* should never fail */) {
          /* find an NPC in the room */
          for (tmp = rp->people; tmp; tmp = tmp->next_in_room) {
            if (IS_NPC(tmp)) {
              break;
            }
          }

          if (tmp == nullptr) {
            break; /* we've run out of NPCs */
          }

          or = tmp->in_room;
          char_from_room(tmp); /* the list of people in the room has changed */
          char_to_room(tmp, rp->tele_targ);
          if (IS_SET(dest->room_flags, DEATH)) {
            death_cry(tmp);
            if (IS_NPC(tmp) && (IS_SET(tmp->specials.act, ACT_POLYSELF))) {
              /*
               *   take char from storage, to room
               */
              pers = tmp->desc->original;
              char_from_room(pers);
              char_to_room(pers, tmp->in_room);
              SwitchStuff(tmp, pers);
              zero_rent(pers);
              extract_char(tmp);
              tmp = pers;
            }
            zero_rent(tmp);
            extract_char(tmp);
          }
        }
        or = ch->in_room;
        char_from_room(ch);
        char_to_room(ch, rp->tele_targ);
        tmp_desc =
          find_ex_description("_tele_", real_roomp(or)->ex_description);
        if (tmp_desc) {
          page_string(ch->desc, tmp_desc, 1);
        }
        if (rp->tele_look) {
          do_look(ch, "\0", 15);
        }
        if (IS_SET(dest->room_flags, DEATH) && GetMaxLevel(ch) < LOW_IMMORTAL) {
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
        }
      }
    }
  }
}

void RiverPulseStuff(int pulse) {
  /*
    down_river(pulse);
    MakeSound();
    */

  register struct char_data* ch;
  struct char_data* tmp;
  register struct obj_data* obj_object;
  struct obj_data* next_obj;
  int rd;
  int or;
  char buf[80];
  char buffer[100];
  struct room_data* rp;

  if (pulse < 0) {
    return;
  }

  for (ch = character_list; ch; ch = tmp) {
    tmp = ch->next;
    if (!IS_NPC(ch)) {
      if (ch->in_room != NOWHERE) {
        if ((real_roomp(ch->in_room)->sector_type == SECT_WATER_NOSWIM) ||
            (real_roomp(ch->in_room)->sector_type == SECT_UNDERWATER)) {
          if ((real_roomp(ch->in_room))->river_speed > 0) {
            if ((pulse % (real_roomp(ch->in_room))->river_speed) == 0) {
              if (((real_roomp(ch->in_room))->river_dir <= 5) &&
                  ((real_roomp(ch->in_room))->river_dir >= 0)) {
                rd = (real_roomp(ch->in_room))->river_dir;
                for (obj_object = (real_roomp(ch->in_room))->contents;
                  obj_object; obj_object = next_obj) {
                  next_obj = obj_object->next_content;
                  if ((real_roomp(ch->in_room))->dir_option[rd]) {
                    obj_from_room(obj_object);
                    obj_to_room(obj_object,
                      (real_roomp(ch->in_room))->dir_option[rd]->to_room);
                  }
                }
                /*
                  flyers don't get moved
                  */
                if (!IS_AFFECTED(ch, AFF_FLYING) ||
                    (real_roomp(ch->in_room)->sector_type == SECT_UNDERWATER)) {
                  rp = real_roomp(ch->in_room);
                  if (rp && rp->dir_option[rd] && rp->dir_option[rd]->to_room &&
                      (EXIT(ch, rd)->to_room != NOWHERE)) {
                    if (ch->specials.fighting) {
                      stop_fighting(ch);
                    }
                    sprintf(buf, "You drift %s...\n\r", dirs[rd]);
                    send_to_char(buf, ch);
                    or = ch->in_room;
                    char_from_room(ch);
                    char_to_room(ch, (real_roomp(or))->dir_option[rd]->to_room);
                    do_look(ch, "\0", 15);

                    if (IS_SET(RM_FLAGS(ch->in_room), DEATH) &&
                        GetMaxLevel(ch) < LOW_IMMORTAL) {
                      death_cry(ch);
                      zero_rent(ch);
                      extract_char(ch);
                    }
                  }
                }
              }
            }
          }
        }
      }
    } else {
      /*
       *   mobiles
       */
      if ((ch->player.sounds) && (number(0, 5) == 0)) {
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
}

/*
**  Apply soundproof is for ch making noise
*/
int apply_soundproof(struct char_data* ch) {
  struct room_data* rp;

  rp = real_roomp(ch->in_room);

  if (!rp) {
    return 0;
  }

  if (IS_SET(rp->room_flags, SILENCE)) {
    send_to_char("You are in a silence zone, you can't make a sound!\n\r", ch);
    return 1; /* for shouts, emotes, etc */
  }
  return 0;
}

/*
**  check_soundproof is for others making noise
*/
int check_soundproof(struct char_data* ch) {
  struct room_data* rp;

  rp = real_roomp(ch->in_room);

  if (!rp) {
    return 0;
  }

  if (IS_SET(rp->room_flags, SILENCE)) {
    return 1; /* for shouts, emotes, etc */
  }
  return 0;
}

int MobCountInRoom(struct char_data* list) {
  int i;
  struct char_data* tmp;

  for (i = 0, tmp = list; tmp; tmp = tmp->next_in_room, i++) {
    ;
  }

  return (i);
}

void* Mymalloc(long size) { return (malloc(size)); }

void SpaceForSkills(struct char_data* ch) {
  /*
    create space for the skills for some mobile or character.
  */

  ch->skills =
    (struct char_skill_data*)calloc(MAX_SKILLS, sizeof(struct char_skill_data));

  if (ch->skills == 0) {
    abort();
  }
}

static int count_lims(struct obj_data* obj) {
  int total = 0;

  if (!obj) {
    return (0);
  }

  if (obj->contains) {
    total += count_lims(obj->contains);
  }
  if (obj->next_content) {
    total += count_lims(obj->next_content);
  }
  if (obj->obj_flags.cost_per_day > LIM_ITEM_COST_MIN) {
    total += 1;
  }
  return (total);
}

char* lower(char* s) {
  static char c[1000];
  int i = 0;

  (void)snprintf(c, sizeof(c), "%s", s);

  while (i < (int)sizeof(c) - 1 && c[i]) {
    if (isupper(c[i])) {
      c[i] = tolower(c[i]);
    }
    i++;
  }
  return (c);
}

char* strstr(const char* s1, const char* s2) {
  char* cp;
  int i;
  int j = strlen(s1) - strlen(s2);
  int k = strlen(s2);

  if (j < 0) {
    return nullptr;
  }
  for (i = 0; i <= j && strncmp(s1++, s2, k) != 0; i++) {
    ;
  }
  return (i > j) ? nullptr : (char*)(s1 - 1);
}

char* strcasestr(const char* s1, const char* s2) {
  char* cp;
  int i;
  int j = strlen(s1) - strlen(s2);
  int k = strlen(s2);

  if (j < 0) {
    return nullptr;
  }
  for (i = 0; i <= j && strncasecmp(s1++, s2, k) != 0; i++) {
    ;
  }
  return (i > j) ? nullptr : (char*)(s1 - 1);
}

int GetApprox(int num, int perc) {
  /* perc = 0 - 100 */
  int adj;
  int r;
  float fnum;
  float fadj;

  adj = 100 - perc;
  if (adj < 0) {
    adj = 0;
  }
  adj *= 2; /* percentage of play (+- x%) */

  r = number(1, adj);

  perc += r;

  fnum = (float)num;
  fadj = (float)perc * 2;
  fnum *= (float)(fadj / (200.0));

  num = (int)fnum;

  return (num);
}

void vlogf(const char* errorMsg, ...) {
  char message_buffer[256];
  va_list ap;

  va_start(ap, errorMsg);
  vsprintf(message_buffer, errorMsg, ap);
  va_end(ap);
  vlog(message_buffer);
}
