#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "accessors.h"
#include "bit_ops.h"
#include "character_flags.h"
#include "comm.h"
#include "commands.h"
#include "constants.h"
#include "db.h"
#include "game_constants.h"
#include "handler.h"
#include "interpreter.h"
#include "limits.h"
#include "memory_macros.h"
#include "multiclass.h"
#include "object_flags.h"
#include "opinion.h"
#include "room_flags.h"
#include "spell_ids.h"
#include "structs.h"
#include "ui_strings.h"
#include "utils.h"

char* fname(char* namelist) {
  static char holder[30];
  char* point = nullptr;

  for (point = holder; isalpha(*namelist) != 0; namelist++, point++) {
    *point = *namelist;
  }

  *point = '\0';

  return (holder);
}

static int split_string(char* str, const char* sep, char** argv)
/* str must be writable */
{
  char* s = nullptr;
  int argc = 0;

  s = strtok(str, sep);
  if (s != nullptr) {
    argv[argc++] = s;
  } else {
    *argv = str;
    return 1;
  }

  while ((s = strtok(nullptr, sep)) != nullptr) {
    argv[argc++] = s;
  }
  return argc;
}

int isname(const char* str, const char* namelist) {
  char* argv[100];
  char* xargv[100];
  int argc = 0;
  int xargc = 0;
  int i = 0;
  int j = 0;
  int exact = 0;
  char buf[MAX_INPUT_LENGTH];
  char names[MAX_INPUT_LENGTH];
  char* s = nullptr;

  if ((str == nullptr) || (namelist == nullptr)) {
    return 0;
  }

  strcpy(buf, str);
  argc = split_string(buf, "- \t\n\r,", argv);

  strcpy(names, namelist);
  xargc = split_string(names, "- \t\n\r,", xargv);

  if (argc > 0) {
    s = argv[argc - 1];
    const size_t len = strlen(s);
    if (len > 0) {
      if (s[len - 1] == '.') {
        exact = 1;
        s[len - 1] = '\0';
      }
    }
  }

  if ((exact != 0) && argc != xargc) {
    return 0;
  }

  for (i = 0; i < argc; i++) {
    for (j = 0; j < xargc; j++) {
      if ((xargv[j] != nullptr) && (is_abbrev(argv[i], xargv[j]) != 0)) {
        xargv[j] = nullptr;
        break;
      }
    }
    if (j >= xargc) {
      return 0;
    }
  }
  return 1;
}

void init_string_block(struct string_block* sb) {
  sb->data = (char*)malloc(sb->size = 128);
  *sb->data = '\0';
}

void append_to_string_block(struct string_block* sb, const char* str) {
  int len = 0;
  len = strlen(sb->data) + strlen(str) + 1;
  if (len > sb->size) {
    if (len > (sb->size *= 2)) {
      sb->size = len;
    }
    sb->data = (char*)realloc(sb->data, sb->size);
  }
  strcat(sb->data, str);
}

void page_string_block(struct string_block* sb, struct char_data* ch) {
  page_string(ch->desc, sb->data, 1);
}

void destroy_string_block(struct string_block* sb) {
  free(sb->data);
  sb->data = nullptr;
}

void affect_modify(struct char_data* ch, signed char loc, long mod, long bitv,
  char add) {
  int i = 0;

  if (loc == APPLY_IMMUNE) {
    if (add != 0) {
      SET_BIT(ch->immune, mod);
    } else {
      REMOVE_BIT(ch->immune, mod);
    }
  } else if (loc == APPLY_SUSC) {
    if (add != 0) {
      SET_BIT(ch->susc, mod);
    } else {
      REMOVE_BIT(ch->susc, mod);
    }

  } else if (loc == APPLY_M_IMMUNE) {
    if (add != 0) {
      SET_BIT(ch->M_immune, mod);
    } else {
      REMOVE_BIT(ch->M_immune, mod);
    }
  } else if (loc == APPLY_SPELL) {
    if (add != 0) {
      SET_BIT(ch->specials.affected_by, mod);
    } else {
      REMOVE_BIT(ch->specials.affected_by, mod);
    }
  } else if (loc == APPLY_WEAPON_SPELL) {
    return;
  } else {
    if (add != 0) {
      SET_BIT(ch->specials.affected_by, bitv);
    } else {
      REMOVE_BIT(ch->specials.affected_by, bitv);
      mod = -mod;
    }
  }

  int maxabil = (IS_NPC(ch) ? 25 : 18);

  switch (loc) {
    case APPLY_NONE:
      break;

    case APPLY_STR:
      GET_STR(ch) += mod;
      break;

    case APPLY_DEX:
      GET_DEX(ch) += mod;
      break;

    case APPLY_INT:
      GET_INT(ch) += mod;
      break;

    case APPLY_WIS:
      GET_WIS(ch) += mod;
      break;

    case APPLY_CON:
      GET_CON(ch) += mod;
      break;

    case APPLY_SEX:
      GET_SEX(ch) = static_cast<int>((ch->player.sex - 1) == 0) + 1;
      break;

    case APPLY_CLASS:
      break;

    case APPLY_LEVEL:
      break;

    case APPLY_AGE:
      ch->player.time.birth -= SECS_PER_MUD_YEAR * mod;
      break;

    case APPLY_CHAR_WEIGHT:
      GET_WEIGHT(ch) += mod;
      break;

    case APPLY_CHAR_HEIGHT:
      GET_HEIGHT(ch) += mod;
      break;

    case APPLY_MANA:
      ch->points.max_mana += mod;
      break;

    case APPLY_HIT:
      ch->points.max_hit += mod;
      break;

    case APPLY_MOVE:
      ch->points.max_move += mod;
      break;

    case APPLY_GOLD:
      break;

    case APPLY_EXP:
      break;

    case APPLY_AC:
      GET_AC(ch) += mod;
      break;

    case APPLY_HITROLL:
      GET_HITROLL(ch) += mod;
      break;

    case APPLY_DAMROLL:
      GET_DAMROLL(ch) += mod;
      break;

    case APPLY_SAVING_PARA:
      ch->specials.apply_saving_throw[0] += mod;
      break;

    case APPLY_SAVING_ROD:
      ch->specials.apply_saving_throw[1] += mod;
      break;

    case APPLY_SAVING_PETRI:
      ch->specials.apply_saving_throw[2] += mod;
      break;

    case APPLY_SAVING_BREATH:
      ch->specials.apply_saving_throw[3] += mod;
      break;

    case APPLY_SAVING_SPELL:
      ch->specials.apply_saving_throw[4] += mod;
      break;

    case APPLY_SAVE_ALL: {
      for (i = 0; i <= 4; i++) {
        ch->specials.apply_saving_throw[i] += mod;
      }
    } break;
    case APPLY_IMMUNE:
      break;
    case APPLY_SUSC:
      break;
    case APPLY_M_IMMUNE:
      break;
    case APPLY_SPELL:
      break;
    case APPLY_HITNDAM:
      GET_HITROLL(ch) += mod;
      GET_DAMROLL(ch) += mod;
      break;
    case APPLY_WEAPON_SPELL:
    case APPLY_EAT_SPELL:
      break;
    case APPLY_BACKSTAB:
      if (ch->skills == nullptr) {
        return;
      }
      ch->skills[SKILL_BACKSTAB].learned += mod;
      break;
    case APPLY_KICK:
      if (ch->skills == nullptr) {
        return;
      }
      ch->skills[SKILL_KICK].learned += mod;
      break;
    case APPLY_SNEAK:
      if (ch->skills == nullptr) {
        return;
      }
      ch->skills[SKILL_SNEAK].learned += mod;
      break;
    case APPLY_HIDE:
      if (ch->skills == nullptr) {
        return;
      }
      ch->skills[SKILL_HIDE].learned += mod;
      break;
    case APPLY_BASH:
      if (ch->skills == nullptr) {
        return;
      }
      ch->skills[SKILL_BASH].learned += mod;
      break;
    case APPLY_PICK:
      if (ch->skills == nullptr) {
        return;
      }
      ch->skills[SKILL_PICK_LOCK].learned += mod;
      break;
    case APPLY_STEAL:
      if (ch->skills == nullptr) {
        return;
      }
      ch->skills[SKILL_STEAL].learned += mod;
      break;
    case APPLY_TRACK:
      if (ch->skills == nullptr) {
        return;
      }
      ch->skills[SKILL_HUNT].learned += mod;
      break;
    case APPLY_DEATHSTROKE:
      if (ch->skills == nullptr) {
        return;
      }
      ch->skills[SKILL_DEATHSTROKE].learned += mod;
      break;
    case APPLY_DOUBLE_ATTACK:
      if (ch->skills == nullptr) {
        return;
      }
      ch->skills[SKILL_DOUBLE_ATTACK].learned += mod;
      break;
    case APPLY_GRAPPLE:
      if (ch->skills == nullptr) {
        return;
      }
      ch->skills[SKILL_GRAPPLE].learned += mod;
      break;
    case APPLY_THROW:
      if (ch->skills == nullptr) {
        return;
      }
      ch->skills[SKILL_THROW].learned += mod;
      break;
    default:
      vlog("Unknown apply adjust attempt (handler.c, affect_modify).");
      vlog(ch->player.name);

      break;

  } /* switch */
}

/* This updates a character by subtracting everything he is affected by */
/* restoring original abilities, and then affecting all again           */
void affect_total(struct char_data* ch) {
  struct affected_type* af = nullptr;
  int i = 0;
  int j = 0;

  for (i = 0; i < MAX_WEAR; i++) {
    if (ch->equipment[i] != nullptr) {
      for (j = 0; j < MAX_OBJ_AFFECT; j++) {
        affect_modify(ch, ch->equipment[i]->affected[j].location,
          ch->equipment[i]->affected[j].modifier,
          ch->equipment[i]->obj_flags.bitvector, 0);
      }
    }
  }

  for (af = ch->affected; af != nullptr; af = af->next) {
    affect_modify(ch, af->location, af->modifier, af->bitvector, 0);
  }

  ch->tmpabilities = ch->abilities;

  for (i = 0; i < MAX_WEAR; i++) {
    if (ch->equipment[i] != nullptr) {
      for (j = 0; j < MAX_OBJ_AFFECT; j++) {
        affect_modify(ch, ch->equipment[i]->affected[j].location,
          ch->equipment[i]->affected[j].modifier,
          ch->equipment[i]->obj_flags.bitvector, 1);
      }
    }
  }

  for (af = ch->affected; af != nullptr; af = af->next) {
    affect_modify(ch, af->location, af->modifier, af->bitvector, 1);
  }

  /* Make certain values are between 0..25, not < 0 and not > 25! */

  i = (IS_NPC(ch) ? 25 : 18);

  if (ch->abilities.dex == 19) {
    GET_DEX(ch) = MAX(1, MIN(GET_DEX(ch), 19));
  } else {
    GET_DEX(ch) = MAX(1, MIN(GET_DEX(ch), 18));
  }

  if (ch->abilities.intel == 19) {
    GET_INT(ch) = MAX(1, MIN(GET_INT(ch), 19));
  } else {
    GET_INT(ch) = MAX(1, MIN(GET_INT(ch), 18));
  }

  if (ch->abilities.wis == 19) {
    GET_WIS(ch) = MAX(1, MIN(GET_WIS(ch), 19));
  } else {
    GET_WIS(ch) = MAX(1, MIN(GET_WIS(ch), 18));
  }

  if (ch->abilities.con == 19) {
    GET_CON(ch) = MAX(1, MIN(GET_CON(ch), 19));
  } else {
    GET_CON(ch) = MAX(1, MIN(GET_CON(ch), 18));
  }

  if (ch->abilities.str == 19) {
    GET_STR(ch) = MAX(1, MIN(GET_STR(ch), 19));
  } else {
    GET_STR(ch) = MAX(1, MIN(GET_STR(ch), 18));
  }

  if (IS_NPC(ch)) {
    GET_STR(ch) = MIN(GET_STR(ch), i);
  } else {
    if (GET_STR(ch) > 18) {
      i = GET_ADD(ch) + ((GET_STR(ch) - 18) * 10);
      GET_ADD(ch) = MIN(i, 100);
    }
  }
}

/* Insert an affect_type in a char_data structure
   Automatically sets apropriate bits and apply's */
void affect_to_char(struct char_data* ch, struct affected_type* af) {
  struct affected_type* affected_alloc = nullptr;

  CREATE(affected_alloc, struct affected_type, 1);

  *affected_alloc = *af;
  affected_alloc->next = ch->affected;
  ch->affected = affected_alloc;

  affect_modify(ch, af->location, af->modifier, af->bitvector, 1);
  affect_total(ch);
}

/* Remove an affected_type structure from a char (called when duration
   reaches zero). Pointer *af must never be NIL! Frees mem and calls
   affect_location_apply                                                */
void affect_remove(struct char_data* ch, struct affected_type* af) {
  struct affected_type* hjp = nullptr;

  if (ch->affected == nullptr) {
    vlog("affect removed from char without affect");
    vlog(GET_NAME(ch));
    return;
  }

  affect_modify(ch, af->location, af->modifier, af->bitvector, 0);

  /* remove structure *af from linked list */

  if (ch->affected == af) {
    /* remove head of list */
    ch->affected = af->next;
  } else {
    for (hjp = ch->affected; ((hjp->next) != nullptr) && (hjp->next != af);
      hjp = hjp->next) {
      ;
    }

    if (hjp->next != af) {
      vlog(
        "Could not locate affected_type in ch->affected. (handler.c, "
        "affect_remove)");
      return;
    }
    hjp->next = af->next; /* skip the af element */
  }

  free(af);

  affect_total(ch);
}

/* Call affect_remove with every spell of spelltype "skill" */
void affect_from_char(struct char_data* ch, short skill) {
  struct affected_type* next = nullptr;

  for (struct affected_type* hjp = ch->affected; hjp != nullptr; hjp = next) {
    next = hjp->next;
    if (hjp->type == skill) {
      affect_remove(ch, hjp);
    }
  }
}

/* Return if a char is affected by a spell (SPELL_XXX), nullptr indicates
   not affected                                                        */
char affected_by_spell(struct char_data* ch, short skill) {
  struct affected_type* hjp = nullptr;

  for (hjp = ch->affected; hjp != nullptr; hjp = hjp->next) {
    if (hjp->type == skill) {
      return 1;
    }
  }

  return 0;
}

void affect_join(struct char_data* ch, struct affected_type* af, char avg_dur,
  char avg_mod) {
  struct affected_type* hjp = nullptr;
  char found = 0;

  for (hjp = ch->affected; (found == 0) && (hjp != nullptr); hjp = hjp->next) {
    if (hjp->type == af->type) {
      af->duration += hjp->duration;
      if (avg_dur != 0) {
        af->duration /= 2;
      }

      af->modifier += hjp->modifier;
      if (avg_mod != 0) {
        af->modifier /= 2;
      }

      affect_remove(ch, hjp);
      affect_to_char(ch, af);
      found = 1;
      break; /* Exit loop immediately to avoid use-after-free on hjp */
    }
  }
  if (found == 0) {
    affect_to_char(ch, af);
  }
}

/* move a player out of a room */
void char_from_room(struct char_data* ch) {
  char buf[MAX_INPUT_LENGTH];
  struct char_data* i = nullptr;
  struct room_data* rp = nullptr;

  if (ch->in_room == NOWHERE) {
    vlog("NOWHERE extracting char from room (handler.c, char_from_room)");
    return;
  }

  if (ch->equipment[WEAR_LIGHT] != nullptr) {
    if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT) {
      if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2] !=
          0) { /* Light is ON */
        real_roomp(ch->in_room)->light--;
      }
    }
  }

  rp = real_roomp(ch->in_room);
  if (rp == nullptr) {
    sprintf(buf, "ERROR: char_from_room: %s was not in a valid room (%d)",
      (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr),
      ch->in_room);
    vlog(buf);
    return;
  }

  if (ch == rp->people) { /* head of list */
    rp->people = ch->next_in_room;

  } else { /* locate the previous element */
    for (i = rp->people; (i != nullptr) && i->next_in_room != ch;
      i = i->next_in_room) {
      ;
    }
    if (i != nullptr) {
      i->next_in_room = ch->next_in_room;
    } else {
      sprintf(buf, "SHIT, %s was not in people list of his room %d!",
        (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr),
        ch->in_room);
      vlog(buf);
    }
  }

  ch->in_room = NOWHERE;
  ch->next_in_room = nullptr;
}

/* place a character in a room */
void char_to_room(struct char_data* ch, int room) {
  struct room_data* rp = nullptr;

  rp = real_roomp(room);
  if (rp == nullptr) {
    room = 0;
    rp = real_roomp(room);
    if (rp == nullptr) {
      exit(0);
    }
  }
  ch->next_in_room = rp->people;
  rp->people = ch;
  ch->in_room = room;

  if (ch->equipment[WEAR_LIGHT] != nullptr) {
    if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT) {
      if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2] !=
          0) { /* Light is ON */
        rp->light++;
      }
    }
  }
}

/* give an object to a char   */
void obj_to_char(struct obj_data* object, struct char_data* ch) {
  assert(!object->in_obj && !object->carried_by && !object->equipped_by &&
         object->in_room == NOWHERE);

  if (ch->carrying != nullptr) {
    object->next_content = ch->carrying;
  } else {
    object->next_content = nullptr;
  }

  ch->carrying = object;
  object->carried_by = ch;
  object->in_room = NOWHERE;
  object->equipped_by = nullptr;
  object->in_obj = nullptr;
  IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(object);
  IS_CARRYING_N(ch) += GET_OBJ_VOLUME(object);
}

/* take an object from a char */
void obj_from_char(struct obj_data* object) {
  struct obj_data* tmp = nullptr;

  if (object == nullptr) {
    vlog("No object to be take from char.");
    abort();
  }

  if (object->carried_by == nullptr) {
    vlog("this object is not carried by anyone");
    abort();
  }

  if (object->carried_by->carrying == nullptr) {
    vlog("No one is carrying this object");
    abort();
  }

  if (object->in_obj != nullptr) {
    vlog("obj_data in more than one place.");
    abort();
  }

  if (object->equipped_by != nullptr) {
    vlog("obj_data in more than one place.");
    abort();
  }

  if (object->carried_by->carrying == object) { /* head of list */
    object->carried_by->carrying = object->next_content;

  } else {
    for (tmp = object->carried_by->carrying;
      (tmp != nullptr) && (tmp->next_content != object);
      tmp = tmp->next_content) {
      ; /* locate previous */
    }

    if (tmp == nullptr) {
      vlog("Couldn't find object on character");
      abort();
    }

    tmp->next_content = object->next_content;
  }

  IS_CARRYING_W(object->carried_by) -= GET_OBJ_WEIGHT(object);
  IS_CARRYING_N(object->carried_by) -= GET_OBJ_VOLUME(object);
  object->carried_by = nullptr;
  object->equipped_by = nullptr; /* should be unnecessary, but, why risk it */
  object->next_content = nullptr;
  object->in_obj = nullptr;
}

/* Return the effect of a piece of armor in position eq_pos */
static int apply_ac(struct char_data* ch, int eq_pos) {
  assert(ch->equipment[eq_pos]);

  if (!(GET_ITEM_TYPE(ch->equipment[eq_pos]) == ITEM_ARMOR)) {
    return 0;
  }

  switch (eq_pos) {
    case WEAR_BODY:
      return (2 * ch->equipment[eq_pos]->obj_flags.value[0]); /* 20% */
    case WEAR_HEAD:
      return (2 * ch->equipment[eq_pos]->obj_flags.value[0]); /* 20% */
    case WEAR_LEGS:
      return (ch->equipment[eq_pos]->obj_flags.value[0]); /* 10% */
    case WEAR_FEET:
      return (ch->equipment[eq_pos]->obj_flags.value[0]); /* 10% */
    case WEAR_HANDS:
      return (ch->equipment[eq_pos]->obj_flags.value[0]); /* 10% */
    case WEAR_ARMS:
      return (ch->equipment[eq_pos]->obj_flags.value[0]); /* 10% */
    case WEAR_SHIELD:
      return (ch->equipment[eq_pos]->obj_flags.value[0]); /* 10% */
  }
  return 0;
}

static int give_min_str_to_wield(struct obj_data* obj, struct char_data* ch) {
  const int str = 0;

  GET_STR(ch) = 16; /* nice, semi-reasonable start */
  /*
    will have a problem with except. str, that i do not care to solve
  */

  while (GET_OBJ_WEIGHT(obj) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) {
    GET_STR(ch)++;
  }

  return (str);
}

void equip_char(struct char_data* ch, struct obj_data* obj, int pos) {
  int j = 0;

  assert(pos >= 0 && pos < MAX_WEAR);
  assert(!(ch->equipment[pos]));

  if (obj->carried_by != nullptr) {
    vlog("EQUIP: obj_data is carried_by when equip.");
    abort();
  }

  if (obj->in_room != NOWHERE) {
    vlog("EQUIP: obj_data is in_room when equip.");
    abort();
    return;
  }

  if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
      (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
    if (ch->in_room != NOWHERE) {
      act("You are zapped by $p and instantly drop it.", 0, ch, obj, nullptr,
        TO_CHAR);
      act("$n is zapped by $p and instantly drops it.", 0, ch, obj, nullptr,
        TO_ROOM);
      obj_to_room(obj, ch->in_room);
      return;
    }
  }
  if ((GET_ITEM_TYPE(obj) == ITEM_ARMOR) && ((HasClass(ch, CLASS_MONK)) != 0)) {
    send_to_char("Your monk vows wont let you wear such items.\n\r", ch);
    obj_to_char(obj, ch);
    return;
  }
  if ((IS_OBJ_STAT(obj, ITEM_LEVEL10) && (GetMaxLevel(ch) < 10)) ||
      (IS_OBJ_STAT(obj, ITEM_LEVEL15) && (GetMaxLevel(ch) < 15)) ||
      (IS_OBJ_STAT(obj, ITEM_LEVEL20) && (GetMaxLevel(ch) < 20)) ||
      (IS_OBJ_STAT(obj, ITEM_LEVEL25) && (GetMaxLevel(ch) < 25)) ||
      (IS_OBJ_STAT(obj, ITEM_LEVEL30) && (GetMaxLevel(ch) < 30)) ||
      (IS_OBJ_STAT(obj, ITEM_LEVEL35) && (GetMaxLevel(ch) < 35)) ||
      (IS_OBJ_STAT(obj, ITEM_LEVEL40) && (GetMaxLevel(ch) < 40))) {
    if (ch->in_room != NOWHERE) {
      act("You do not know how to use the $p.", 0, ch, obj, nullptr, TO_CHAR);
      send_to_char(
        "Maybe a little more experience will help you understand!\n\r", ch);
      act("You are zapped by $p and instantly drop it.", 0, ch, obj, nullptr,
        TO_CHAR);
      act("$n is zapped by $p and instantly drops it.", 0, ch, obj, nullptr,
        TO_ROOM);
      obj_to_room(obj, ch->in_room);
      return;
    }
    vlog("ch->in_room = NOWHERE when equipping char.");
    abort();
  }

  ch->equipment[pos] = obj;
  obj->equipped_by = ch;
  obj->eq_pos = pos;

  if (GET_ITEM_TYPE(obj) == ITEM_ARMOR) {
    GET_AC(ch) -= apply_ac(ch, pos);
  }

  for (j = 0; j < MAX_OBJ_AFFECT; j++) {
    affect_modify(ch, obj->affected[j].location, obj->affected[j].modifier,
      obj->obj_flags.bitvector, 1);
  }

  if (GET_ITEM_TYPE(obj) == ITEM_WEAPON) {
    /* some nifty manuevering for strength */
    if (IS_NPC(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF)) {
      give_min_str_to_wield(obj, ch);
    }
  }

  affect_total(ch);
}

struct obj_data* unequip_char(struct char_data* ch, int pos) {
  int j = 0;
  struct obj_data* obj = nullptr;

  assert(pos >= 0 && pos < MAX_WEAR);
  assert(ch->equipment[pos]);

  obj = ch->equipment[pos];

  assert(!obj->in_obj && obj->in_room == NOWHERE && !obj->carried_by);

  if (GET_ITEM_TYPE(obj) == ITEM_ARMOR) {
    GET_AC(ch) += apply_ac(ch, pos);
  }

  ch->equipment[pos] = nullptr;
  obj->equipped_by = nullptr;
  obj->eq_pos = -1;

  for (j = 0; j < MAX_OBJ_AFFECT; j++) {
    affect_modify(ch, obj->affected[j].location, obj->affected[j].modifier,
      obj->obj_flags.bitvector, 0);
  }

  affect_total(ch);
  if (GET_MANA(ch) >= mana_limit(ch)) {
    GET_MANA(ch) = mana_limit(ch);
  }
  if (GET_HIT(ch) >= hit_limit(ch)) {
    GET_HIT(ch) = hit_limit(ch);
  }

  return (obj);
}

struct obj_data* unequip_char_for_save(struct char_data* ch, int pos) {
  int j = 0;
  struct obj_data* obj = nullptr;

  assert(pos >= 0 && pos < MAX_WEAR);
  assert(ch->equipment[pos]);

  obj = ch->equipment[pos];

  assert(!obj->in_obj && obj->in_room == NOWHERE && !obj->carried_by);

  if (GET_ITEM_TYPE(obj) == ITEM_ARMOR) {
    GET_AC(ch) += apply_ac(ch, pos);
  }

  ch->equipment[pos] = nullptr;
  obj->equipped_by = nullptr;
  obj->eq_pos = -1;

  for (j = 0; j < MAX_OBJ_AFFECT; j++) {
    affect_modify(ch, obj->affected[j].location, obj->affected[j].modifier,
      obj->obj_flags.bitvector, 0);
  }

  affect_total(ch);

  return (obj);
}

int get_number(char** name) {
  int i = 0;
  char* ppos = nullptr;
  char number[MAX_INPUT_LENGTH] = "";

  if (((ppos = strchr(*name, '.')) != nullptr) && (ppos[1] != 0)) {
    *ppos++ = '\0';
    strcpy(number, *name);
    strcpy(*name, ppos);

    for (i = 0; *(number + i) != 0; i++) {
      if (isdigit(*(number + i)) == 0) {
        return (0);
      }
    }

    return (atoi(number));
  }

  return (1);
}

/* Search a given list for an object, and return a pointer to that object */
struct obj_data* get_obj_in_list(const char* name, struct obj_data* list) {
  struct obj_data* i = nullptr;
  int j = 0;
  int number = 0;
  char tmpname[MAX_INPUT_LENGTH];
  char* tmp = nullptr;

  strcpy(tmpname, name);
  tmp = tmpname;

  if ((number = get_number(&tmp)) == 0) {
    return (nullptr);
  }

  for (i = list, j = 1; (i != nullptr) && (j <= number); i = i->next_content) {
    if (isname(tmp, i->name) != 0) {
      if (j == number) {
        return (i);
      }
      j++;
    }
  }

  return (nullptr);
}

/* Search a given list for an object number, and return a ptr to that obj */
struct obj_data* get_obj_in_list_num(int num, struct obj_data* list) {
  struct obj_data* i = nullptr;

  for (i = list; i != nullptr; i = i->next_content) {
    if (i->item_number == num) {
      return (i);
    }
  }

  return (nullptr);
}

/*search the entire world for an object, and return a pointer  */
struct obj_data* get_obj(const char* name) {
  struct obj_data* i = nullptr;
  int j = 0;
  int number = 0;
  char tmpname[MAX_INPUT_LENGTH];
  char* tmp = nullptr;

  strcpy(tmpname, name);
  tmp = tmpname;
  if ((number = get_number(&tmp)) == 0) {
    return (nullptr);
  }

  for (i = object_list, j = 1; (i != nullptr) && (j <= number); i = i->next) {
    if (isname(tmp, i->name) != 0) {
      if (j == number) {
        return (i);
      }
      j++;
    }
  }

  return (nullptr);
}

/*search the entire world for an object number, and return a pointer  */
struct obj_data* get_obj_num(int nr) {
  struct obj_data* i = nullptr;

  for (i = object_list; i != nullptr; i = i->next) {
    if (i->item_number == nr) {
      return (i);
    }
  }

  return (nullptr);
}

/* search a room for a char, and return a pointer if found..  */
struct char_data* get_char_room(char* name, int room) {
  struct char_data* i = nullptr;
  int j = 0;
  int number = 0;
  char tmpname[MAX_INPUT_LENGTH];
  char* tmp = nullptr;

  strcpy(tmpname, name);
  tmp = tmpname;
  if ((number = get_number(&tmp)) == 0) {
    return (nullptr);
  }

  for (i = real_roomp(room)->people, j = 1; (i != nullptr) && (j <= number);
    i = i->next_in_room) {
    if (isname(tmp, GET_NAME(i)) != 0) {
      if (j == number) {
        return (i);
      }
      j++;
    }
  }

  return (nullptr);
}

/* search all over the world for a char, and return a pointer if found */
struct char_data* get_char(char* name) {
  struct char_data* i = nullptr;
  int j = 0;
  int number = 0;
  char tmpname[MAX_INPUT_LENGTH];
  char* tmp = nullptr;

  strcpy(tmpname, name);
  tmp = tmpname;
  if ((number = get_number(&tmp)) == 0) {
    return (nullptr);
  }

  for (i = character_list, j = 1; (i != nullptr) && (j <= number);
    i = i->next) {
    if (isname(tmp, GET_NAME(i)) != 0) {
      if (j == number) {
        return (i);
      }
      j++;
    }
  }

  return (nullptr);
}

/* search all over the world for a char num, and return a pointer if found */
struct char_data* get_char_num(int nr) {
  struct char_data* i = nullptr;

  for (i = character_list; i != nullptr; i = i->next) {
    if (i->nr == nr) {
      return (i);
    }
  }

  return (nullptr);
}

/* put an object in a room */
void obj_to_room(struct obj_data* object, int room) {
  if (room == -1) {
    room = 4;
  }

  struct room_data* rp = real_roomp(room);
  assert(rp);
  if (rp == nullptr) {
    vlog("obj_to_room: bad room");
    return;
  }

  assert(!(object->equipped_by) && (object->eq_pos == -1));

  if (object->in_room > NOWHERE) {
    obj_from_room(object);
  }

  object->next_content = rp->contents;
  rp->contents = object;
  object->in_room = room;
  object->carried_by = nullptr;
  object->equipped_by = nullptr; /* should be unnecessary */
}

/* Take an object from a room */
void obj_from_room(struct obj_data* object) {
  struct obj_data* i = nullptr;

  /* remove object from room */

  if (object->in_room <= NOWHERE) {
    if ((object->carried_by != nullptr) || (object->equipped_by != nullptr)) {
      vlog("Eek.. an object was just taken from a char, instead of a room");
      abort();
    }
    return; /* its not in a room */
  }

  if (object == real_roomp(object->in_room)->contents) { /* head of list */
    real_roomp(object->in_room)->contents = object->next_content;

  } else /* locate previous element in list */
  {
    for (i = real_roomp(object->in_room)->contents;
      (i != nullptr) && (i->next_content != object); i = i->next_content) {
      ;
    }

    if (i != nullptr) {
      i->next_content = object->next_content;
    } else {
      vlog("Couldn't find object in room");
      abort();
    }
  }

  object->in_room = NOWHERE;
  object->next_content = nullptr;
}

/* put an object in an object (quaint)  */
void obj_to_obj(struct obj_data* obj, struct obj_data* obj_to) {
  struct obj_data* tmp_obj = nullptr;

  obj->next_content = obj_to->contains;
  obj_to->contains = obj;
  obj->in_obj = obj_to;
  /*
    (jdb)  hopefully this will fix the object problem
    */
  obj->carried_by = nullptr;
  obj->equipped_by = nullptr;

  for (tmp_obj = obj->in_obj; tmp_obj != nullptr;
    GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj), tmp_obj = tmp_obj->in_obj) {
    ;
  }

  if (!IS_OBJ_STAT(obj_to, ITEM_HOLDING)) {
    if (GET_ITEM_TYPE(obj) != ITEM_CONTAINER) {
      for (tmp_obj = obj->in_obj; tmp_obj != nullptr;
        GET_OBJ_VOLUME(tmp_obj) +=
        (GET_OBJ_VOLUME(obj) / vol_mult[obj->obj_flags.material_points]),
          tmp_obj = tmp_obj->in_obj) {
        ;
      }
    } else {
      for (tmp_obj = obj->in_obj; tmp_obj != nullptr;
        GET_OBJ_VOLUME(tmp_obj) += GET_OBJ_VOLUME(obj),
          tmp_obj = tmp_obj->in_obj) {
        ;
      }
    }
  }
}

/* remove an object from an object */
void obj_from_obj(struct obj_data* obj) {
  struct obj_data* tmp = nullptr;
  struct obj_data* obj_from = nullptr;

  assert(!obj->carried_by && !obj->equipped_by && obj->in_room == NOWHERE);

  if (obj->in_obj != nullptr) {
    obj_from = obj->in_obj;
    if (obj == obj_from->contains) { /* head of list */
      obj_from->contains = obj->next_content;
    } else {
      for (tmp = obj_from->contains;
        (tmp != nullptr) && (tmp->next_content != obj);
        tmp = tmp->next_content) {
        ; /* locate previous */
      }

      if (tmp == nullptr) {
        perror("Fatal error in object structures.");
        abort();
      }

      tmp->next_content = obj->next_content;
    }

    /* Subtract weight from containers container */
    for (tmp = obj->in_obj; tmp->in_obj != nullptr; tmp = tmp->in_obj) {
      GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);
      if (!IS_OBJ_STAT(tmp, ITEM_HOLDING)) {
        if (GET_ITEM_TYPE(obj) != ITEM_CONTAINER) {
          GET_OBJ_VOLUME(tmp) -=
            (GET_OBJ_VOLUME(obj) / vol_mult[obj->obj_flags.material_points]);
        } else {
          GET_OBJ_VOLUME(tmp) -= GET_OBJ_VOLUME(obj);
        }
      }
    }

    GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);
    if (!IS_OBJ_STAT(tmp, ITEM_HOLDING)) {
      if (GET_ITEM_TYPE(obj) != ITEM_CONTAINER) {
        GET_OBJ_VOLUME(tmp) -=
          (GET_OBJ_VOLUME(obj) / vol_mult[obj->obj_flags.material_points]);
      } else {
        GET_OBJ_VOLUME(tmp) -= GET_OBJ_VOLUME(obj);
      }
    }

    /* Subtract weight from char that carries the object */
    if (tmp->carried_by != nullptr) {
      IS_CARRYING_W(tmp->carried_by) -= GET_OBJ_WEIGHT(obj);
      if (!IS_OBJ_STAT(tmp, ITEM_HOLDING)) {
        if (GET_ITEM_TYPE(obj) != ITEM_CONTAINER) {
          IS_CARRYING_N(tmp->carried_by) -=
            (GET_OBJ_VOLUME(obj) / vol_mult[obj->obj_flags.material_points]);
        } else {
          IS_CARRYING_N(tmp->carried_by) -= GET_OBJ_VOLUME(obj);
        }
      }
    }

    obj->in_obj = nullptr;
    obj->next_content = nullptr;
  } else {
    perror("Trying to object from object when in no object.");
    abort();
  }
}

/* Set all carried_by to point to new owner */
void object_list_new_owner(struct obj_data* list, struct char_data* ch) {
  if (list != nullptr) {
    object_list_new_owner(list->contains, ch);
    object_list_new_owner(list->next_content, ch);
    list->carried_by = ch;
  }
}

/* Extract an object from the world */
void extract_obj(struct obj_data* obj) {
  struct obj_data* temp1 = nullptr;
  struct obj_data* temp2 = nullptr;

  if (obj->in_room != NOWHERE) {
    obj_from_room(obj);
  } else if (obj->carried_by != nullptr) {
    obj_from_char(obj);
  } else if (obj->equipped_by != nullptr) {
    if (obj->eq_pos > -1) {
      /*
       **  set players equipment slot to 0; that will avoid the garbage items.
       */
      obj->equipped_by->equipment[obj->eq_pos] = nullptr;

    } else {
      vlog("Extract on equipped item in slot -1 on:");
      vlog(obj->equipped_by->player.name);
      vlog(obj->name);
      return;
    }
  } else if (obj->in_obj != nullptr) {
    temp1 = obj->in_obj;
    if (temp1->contains == obj) { /* head of list */
      temp1->contains = obj->next_content;
    } else {
      for (temp2 = temp1->contains;
        (temp2 != nullptr) && (temp2->next_content != obj);
        temp2 = temp2->next_content) {
        ;
      }

      if (temp2 != nullptr) {
        temp2->next_content = obj->next_content;
      }
    }
  }

  while (obj->contains != nullptr) {
    struct obj_data* contained = obj->contains;
    obj->contains = contained->next_content;
    contained->in_obj = nullptr; /* Already unlinked from parent */
    extract_obj(contained);
  }
  /* leaves nothing ! */

  if (object_list == obj) { /* head of list */
    object_list = obj->next;
  } else {
    for (temp1 = object_list; (temp1 != nullptr) && (temp1->next != obj);
      temp1 = temp1->next) {
      ;
    }

    if (temp1 != nullptr) {
      temp1->next = obj->next;
    } else {
      vlog("Couldn't find object in object list.");
      abort();
    }
  }

  if (obj->item_number >= 0) {
    (obj_index[obj->item_number].number)--;
  }
  free_obj(obj);

  obj_count--;
}

void update_object(struct obj_data* obj, int use) {
  if (obj->obj_flags.decay_time > 0) {
    obj->obj_flags.decay_time -= use;
  }
  if (obj->contains != nullptr) {
    update_object(obj->contains, use);
  }
  if (obj->next_content != nullptr) {
    if (obj->next_content != obj) {
      update_object(obj->next_content, use);
    }
  }
}

/* Called when a character that follows/is followed dies */
static void die_follower(struct char_data* ch) {
  struct follow_type* j = nullptr;
  struct follow_type* k = nullptr;

  if (ch->master != nullptr) {
    stop_follower(ch);
  }

  for (k = ch->followers; k != nullptr; k = j) {
    j = k->next;
    stop_follower(k->follower);
  }
}

/* Extract a ch completely from the world, and leave his stuff behind */
void extract_char(struct char_data* ch) {
  struct obj_data* i = nullptr;
  struct char_data* k = nullptr;
  struct char_data* next_char = nullptr;
  struct descriptor_data* t_desc = nullptr;
  int l = 0;
  int was_in = 0;
  int j = 0;

  if (!IS_NPC(ch) && (ch->desc == nullptr)) {
    for (t_desc = descriptor_list; t_desc != nullptr; t_desc = t_desc->next) {
      if (t_desc->original == ch) {
        do_return(t_desc->character, "", 0);
      }
    }
  }

  if (ch->in_room == NOWHERE) {
    vlog("NOWHERE extracting char. (handler.c, extract_char)");
    /*
     **  problem from linkdeath
     */
    char_to_room(ch, 4); /* 4 == all purpose store */
  }

  if ((ch->followers != nullptr) || (ch->master != nullptr)) {
    die_follower(ch);
  }

  if (ch->desc != nullptr) {
    /* Forget snooping */
    if (((ch->desc->snoop.snooping) != nullptr) &&
        ((ch->desc->snoop.snooping->desc) != nullptr)) {
      ch->desc->snoop.snooping->desc->snoop.snoop_by = nullptr;
    }

    if (ch->desc->snoop.snoop_by != nullptr) {
      send_to_char("Your victim is no longer among us.\n\r",
        ch->desc->snoop.snoop_by);
      if (ch->desc->snoop.snoop_by->desc != nullptr) {
        ch->desc->snoop.snoop_by->desc->snoop.snooping = nullptr;
      }
    }

    ch->desc->snoop.snooping = ch->desc->snoop.snoop_by = nullptr;
  }

  if (ch->carrying != nullptr) {
    /* transfer ch's objects to room */

    if (!IS_IMMORTAL(ch)) {
      while (ch->carrying != nullptr) {
        i = ch->carrying;
        obj_from_char(i);
        obj_to_room(i, ch->in_room);
      }
    } else {
      send_to_char(
        "Here, you dropped some stuff, let me help you get rid of that.\n\r",
        ch);
      while (ch->carrying != nullptr) {
        i = ch->carrying;
        obj_from_char(i);
        extract_obj(i);
      }
      /*
  equipment too
  */
      for (j = 0; j < MAX_WEAR; j++) {
        if (ch->equipment[j] != nullptr) {
          extract_obj(unequip_char(ch, j));
        }
      }
    }

  } else {
    if (IS_IMMORTAL(ch)) {
      for (j = 0; j < MAX_WEAR; j++) {
        if (ch->equipment[j] != nullptr) {
          extract_obj(unequip_char(ch, j));
        }
      }
    }
  }

  if (ch->specials.fighting != nullptr) {
    stop_fighting(ch);
  }

  for (k = combat_list; k != nullptr; k = next_char) {
    next_char = k->next_fighting;
    if (k->specials.fighting == ch) {
      stop_fighting(k);
    }
  }

  /* Must remove from room before removing the equipment! */
  was_in = ch->in_room;
  char_from_room(ch);

  /* clear equipment_list */
  for (l = 0; l < MAX_WEAR; l++) {
    if (ch->equipment[l] != nullptr) {
      obj_to_room(unequip_char(ch, l), was_in);
    }
  }

  if (IS_NPC(ch)) {
    for (k = character_list; k != nullptr; k = k->next) {
      if (k->specials.hunting != nullptr) {
        if (k->specials.hunting == ch) {
          k->specials.hunting = nullptr;
        }
      }
      if (Hates(k, ch) != 0) {
        RemHated(k, ch);
      }
      if (Fears(k, ch) != 0) {
        RemFeared(k, ch);
      }
    }
  } else {
    for (k = character_list; k != nullptr; k = k->next) {
      if (k->specials.hunting != nullptr) {
        if (k->specials.hunting == ch) {
          k->specials.hunting = nullptr;
        }
      }
      if (Hates(k, ch) != 0) {
        ZeroHatred(k, ch);
      }
      if (Fears(k, ch) != 0) {
        ZeroFeared(k, ch);
      }
    }
  }
  /* pull the char from the list */

  if (ch == character_list) {
    character_list = ch->next;
  } else {
    for (k = character_list; ((k) != nullptr) && (k->next != ch); k = k->next) {
      ;
    }
    if (k != nullptr) {
      k->next = ch->next;
    } else {
      vlog(
        "Trying to remove ?? from character_list. (handler.c, extract_char)");
      abort();
    }
  }

  GET_AC(ch) = 100;

  if (ch->desc != nullptr) {
    if (ch->desc->original != nullptr) {
      do_return(ch, "", 0);
    }
    save_char(ch, NOWHERE);
  }

  if (IS_NPC(ch)) {
    if (ch->nr > -1) { /* if mobile */
      mob_index[ch->nr].number--;
    }
    // FreeHates(ch);
    // FreeFears(ch);
    mob_count--;
    free_char(ch);
    return;
  }

  if (ch->desc != nullptr) {
    ch->desc->connected = CON_SLCT;
    SEND_TO_Q(MENU, ch->desc);
  }
}

/* ***********************************************************************
   Here follows high-level versions of some earlier routines, ie functionst
   which incorporate the actual player-data.
   *********************************************************************** */

struct char_data* get_char_room_vis(struct char_data* ch, const char* name) {
  struct char_data* i = nullptr;
  int j = 0;
  int number = 0;
  char tmpname[MAX_INPUT_LENGTH];
  char* tmp = nullptr;

  strcpy(tmpname, name);
  tmp = tmpname;
  if ((number = get_number(&tmp)) == 0) {
    return (nullptr);
  }

  for (i = real_roomp(ch->in_room)->people, j = 1;
    (i != nullptr) && (j <= number); i = i->next_in_room) {
    if (isname(tmp, GET_NAME(i)) != 0) {
      if (CAN_SEE(ch, i) != 0) {
        if (j == number) {
          return (i);
        }
        j++;
      }
    }
  }

  return (nullptr);
}

/* get a character from anywhere in the world, doesn't care much about
   being in the same room... */
struct char_data* get_char_vis_world(struct char_data* ch, const char* name,
  int* count)

{
  struct char_data* i = nullptr;
  int j = 0;
  int number = 0;
  char tmpname[MAX_INPUT_LENGTH];
  char* tmp = nullptr;

  strcpy(tmpname, name);
  tmp = tmpname;
  if ((number = get_number(&tmp)) == 0) {
    return (nullptr);
  }

  j = (count != nullptr) ? *count : 1;
  for (i = character_list; (i != nullptr) && (j <= number); i = i->next) {
    if (isname(tmp, GET_NAME(i)) != 0) {
      if (CAN_SEE(ch, i) != 0) {
        if (j == number) {
          return (i);
        }
        j++;
      }
    }
  }
  if (count != nullptr) {
    *count = j;
  }
  return nullptr;
}

struct char_data* get_char_vis(struct char_data* ch, const char* name) {
  struct char_data* i = nullptr;

  /* check location */
  if ((i = get_char_room_vis(ch, name)) != nullptr) {
    return (i);
  }

  return get_char_vis_world(ch, name, nullptr);
}

struct obj_data* get_obj_in_list_vis(struct char_data* ch, const char* name,
  struct obj_data* list) {
  struct obj_data* i = nullptr;
  int j = 0;
  int number = 0;
  char tmpname[MAX_INPUT_LENGTH];
  char* tmp = nullptr;

  strcpy(tmpname, name);
  tmp = tmpname;
  if ((number = get_number(&tmp)) == 0) {
    return (nullptr);
  }

  for (i = list, j = 1; (i != nullptr) && (j <= number); i = i->next_content) {
    if (isname(tmp, i->name) != 0) {
      if (CAN_SEE_OBJ(ch, i)) {
        if (j == number) {
          return (i);
        }
        j++;
      }
    }
  }
  return (nullptr);
}

struct obj_data* get_obj_vis_world(struct char_data* ch, const char* name,
  int* count) {
  struct obj_data* i = nullptr;
  int j = 0;
  int number = 0;
  char tmpname[MAX_INPUT_LENGTH];
  char* tmp = nullptr;

  strcpy(tmpname, name);
  tmp = tmpname;
  if ((number = get_number(&tmp)) == 0) {
    return (nullptr);
  }

  j = (count != nullptr) ? *count : 1;

  /* ok.. no luck yet. scan the entire obj list   */
  for (i = object_list; (i != nullptr) && (j <= number); i = i->next) {
    if (isname(tmp, i->name) != 0) {
      if (CAN_SEE_OBJ(ch, i)) {
        if (j == number) {
          return (i);
        }
        j++;
      }
    }
  }
  if (count != nullptr) {
    *count = j;
  }
  return (nullptr);
}

/*search the entire world for an object, and return a pointer  */
struct obj_data* get_obj_vis(struct char_data* ch, const char* name) {
  struct obj_data* i = nullptr;

  /* scan items carried */
  if ((i = get_obj_in_list_vis(ch, name, ch->carrying)) != nullptr) {
    return (i);
  }

  /* scan room */
  if ((i = get_obj_in_list_vis(ch, name, real_roomp(ch->in_room)->contents)) !=
      nullptr) {
    return (i);
  }

  return get_obj_vis_world(ch, name, nullptr);
}

struct obj_data* get_obj_vis_accessible(struct char_data* ch, char* name) {
  struct obj_data* i = nullptr;
  int j = 0;
  int number = 0;
  char tmpname[MAX_INPUT_LENGTH];
  char* tmp = nullptr;

  strcpy(tmpname, name);
  tmp = tmpname;
  if ((number = get_number(&tmp)) == 0) {
    return (nullptr);
  }

  /* scan items carried */
  for (i = ch->carrying, j = 1; (i != nullptr) && j <= number;
    i = i->next_content) {
    if ((isname(tmp, i->name) != 0) && CAN_SEE_OBJ(ch, i)) {
      if (j == number) {
        return (i);
      }
      j++;
    }
  }
  for (i = real_roomp(ch->in_room)->contents; (i != nullptr) && j <= number;
    i = i->next_content) {
    if ((isname(tmp, i->name) != 0) && CAN_SEE_OBJ(ch, i)) {
      if (j == number) {
        return (i);
      }
      j++;
    }
  }
  return nullptr;
}

struct obj_data* create_money(int amount) {
  struct obj_data* obj = nullptr;
  struct extra_descr_data* new_descr = nullptr;
  char buf[80];

  if (amount <= 0) {
    vlog("ERROR: Try to create negative money.");
    exit(1);
  }

  CREATE(obj, struct obj_data, 1);
  CREATE(new_descr, struct extra_descr_data, 1);
  clear_object(obj);

  if (amount == 1) {
    obj->name = strdup("coin gold");
    obj->short_description = strdup("a gold coin");
    obj->description = strdup("One miserable gold coin.");

    new_descr->keyword = strdup("coin gold");
    new_descr->description = strdup("One miserable gold coin.");
  } else {
    obj->name = strdup("coins gold");
    obj->short_description = strdup("gold coins");
    obj->description = strdup("A pile of gold coins.");

    new_descr->keyword = strdup("coins gold");
    if (amount < 10) {
      sprintf(buf, "There is %d coins.", amount);
      new_descr->description = strdup(buf);
    } else if (amount < 100) {
      sprintf(buf, "There is about %d coins", 10 * (amount / 10));
      new_descr->description = strdup(buf);
    } else if (amount < 1000) {
      sprintf(buf, "It looks like something round %d coins",
        100 * (amount / 100));
      new_descr->description = strdup(buf);
    } else if (amount < 100000) {
      sprintf(buf, "You guess there is %d coins",
        1000 * ((amount / 1000) + number(0, (amount / 1000))));
      new_descr->description = strdup(buf);
    } else {
      new_descr->description = strdup("There is A LOT of coins");
    }
  }

  new_descr->next = nullptr;
  obj->ex_description = new_descr;

  obj->obj_flags.type_flag = ITEM_MONEY;
  obj->obj_flags.wear_flags = ITEM_TAKE;
  obj->obj_flags.decay_time = -1;
  obj->obj_flags.value[0] = amount;
  obj->obj_flags.cost = amount;
  obj->item_number = -1;

  obj->next = object_list;
  object_list = obj;

  return (obj);
}

/* Generic Find, designed to find any object/character                    */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar_ch Will be nullptr if no character was found, otherwise points     */
/* **tar_obj Will be nullptr if no object was found, otherwise points        */
/*                                                                        */
/* The routine returns a pointer to the next word in *arg (just like the  */
/* one_argument routine).                                                 */

int generic_find(const char* arg, int bitvector, struct char_data* ch,
  struct char_data** tar_ch, struct obj_data** tar_obj) {
  static const char* const ignore[] = {"the", "in", "on", "at", "\n"};

  int i = 0;
  char name[256] = "";
  char found = 0;

  found = 0;

  /* Eliminate spaces and "ignore" words */
  while ((*arg != 0) && (found == 0)) {
    for (; *arg == ' '; arg++) {
      ;
    }

    for (i = 0; ((name[i] = *(arg + i)) != 0) && (name[i] != ' '); i++) {
      ;
    }
    name[i] = 0;
    arg += i;
    if (search_block(name, ignore, 1) > -1) {
      found = 1;
    }
  }

  if (name[0] == 0) {
    return (0);
  }

  *tar_ch = nullptr;
  *tar_obj = nullptr;

  if (IS_SET(bitvector, FIND_CHAR_ROOM)) { /* Find person in room */
    if ((*tar_ch = get_char_room_vis(ch, name)) != nullptr) {
      return (FIND_CHAR_ROOM);
    }
  }

  if (IS_SET(bitvector, FIND_CHAR_WORLD)) {
    if ((*tar_ch = get_char_vis(ch, name)) != nullptr) {
      return (FIND_CHAR_WORLD);
    }
  }

  if (IS_SET(bitvector, FIND_OBJ_EQUIP)) {
    for (found = 0, i = 0; i < MAX_WEAR && (found == 0); i++) {
      if ((ch->equipment[i] != nullptr) &&
          str_cmp(name, ch->equipment[i]->name) == 0) {
        *tar_obj = ch->equipment[i];
        found = 1;
      }
    }
    if (found != 0) {
      return (FIND_OBJ_EQUIP);
    }
  }

  if (IS_SET(bitvector, FIND_OBJ_INV)) {
    if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
      if ((*tar_obj = get_obj_vis_accessible(ch, name)) != nullptr) {
        return (FIND_OBJ_INV);
      }
    } else {
      if ((*tar_obj = get_obj_in_list_vis(ch, name, ch->carrying)) != nullptr) {
        return (FIND_OBJ_INV);
      }
    }
  }

  if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
    if ((*tar_obj = get_obj_in_list_vis(ch, name,
           real_roomp(ch->in_room)->contents)) != nullptr) {
      return (FIND_OBJ_ROOM);
    }
  }

  if (IS_SET(bitvector, FIND_OBJ_WORLD)) {
    if ((*tar_obj = get_obj_vis(ch, name)) != nullptr) {
      return (FIND_OBJ_WORLD);
    }
  }

  return (0);
}
