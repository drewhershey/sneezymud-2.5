/* ************************************************************************
 *  file: spec_procs.c , Special module.                   Part of DIKUMUD *
 *  Usage: Procedures handling special procedures for object/room/mobile   *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */
#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include <features.h>

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "area.h"
#include "comm.h"
#include "constants.h"
#include "db.h"
#include "handler.h"
#include "hash.h"
#include "interpreter.h"
#include "limits.h"
#include "multiclass.h"
#include "opinion.h"
#include "race.h"
#include "spec_procs.h"
#include "spells.h"
#include "structs.h"
#include "utils.h"

#define INQ_SHOUT 1
#define INQ_LOOSE 0

#define SWORD_ANCIENTS 25000
/*
 *  list of room #s
 */
#define Elf_Home 1414
#define Bakery 3009
#define Dump 3030
#define Ivory_Gate 1499

struct social_type {
    char* cmd;
    int next_line;
};

#define MAX_NPC_CORPSE_TIME 5
#define MAX_PC_CORPSE_TIME 10

static void make_head(struct char_data* ch) {
  struct obj_data *corpse, *o;
  char buf[MAX_STRING_LENGTH];

  CREATE(corpse, struct obj_data, 1);
  clear_object(corpse);

  corpse->item_number = NOWHERE;
  corpse->in_room = NOWHERE;
  corpse->name = strdup("head");
  sprintf(buf, "The bloody, mangled head of %s",
    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
  corpse->short_description = strdup(buf);

  sprintf(buf, "The bloody, mangled, severed head of %s is lying here.",
    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
  corpse->description = strdup(buf);

  corpse->contains = 0;
  corpse->obj_flags.type_flag = ITEM_CONTAINER;
  corpse->obj_flags.wear_flags = ITEM_TAKE;
  corpse->obj_flags.value[0] = 0;
  corpse->obj_flags.value[3] = 1;
  if (IS_NPC(ch))
    corpse->obj_flags.decay_time = MAX_NPC_CORPSE_TIME;
  else
    corpse->obj_flags.decay_time = MAX_PC_CORPSE_TIME;

  corpse->obj_flags.weight = GET_WEIGHT(ch) >> 3;
  corpse->obj_flags.cost_per_day = 1000;

  corpse->next = object_list;
  object_list = corpse;

  obj_to_room(corpse, ch->in_room);
}

#if 0
int MakeQuest(struct char_data *ch, struct char_data *gm, int Class)
{
   struct obj_data *qt, *tmp, *item=0;
   int i;

   if ((qt=read_object(QuestList[Class][GET_LEVEL(ch, Class)].item, VIRTUAL))
       ==NULL) {
     return(TRUE); /* no item for this level, or item not in db */
   }

   /*
     check eq
   */
   for (i=0;i<MAX_WEAR;i++) {
     if (ch->equipment[i]->item_number == qt->item_number)
       item = ch->equipment[i];
       break;
   }

   /*
     check carrying
   */
   if (!item)
   for (tmp = ch->carrying; tmp; tmp = tmp->next_content)
     if (tmp->item_number == qt->item_number) {
       item = tmp;
       break;
     }

   if (!item) {
     act("$N says 'First, you must bring me $o'.",
	 FALSE, ch, qt, gm, TO_ROOM);
     send_to_char("It can be found ", ch);
     send_to_char(QuestList[Class][GET_LEVEL(ch, Class)].where, ch);
     extract_obj(qt);
     return(FALSE);
   } else {
     extract_obj(qt);
     act("$N graciously takes your gift of $o.",
	 FALSE, ch, tmp, gm, TO_CHAR);
     if (tmp->carried_by) {
       obj_from_char(tmp);
     } else if (tmp->equipped_by) {
       unequip_char(ch, tmp->eq_pos);
     }
     extract_obj(tmp);
     return(TRUE);
   }

}

#endif

/* ********************************************************************
 *  Special procedures for rooms                                       *
 ******************************************************************** */

char* how_good(int percent) {
  static char buf[256];

  if (percent == 0)
    strcpy(buf, " (not learned)");
  else if (percent <= 10)
    strcpy(buf, " (awful)");
  else if (percent <= 20)
    strcpy(buf, " (bad)");
  else if (percent <= 40)
    strcpy(buf, " (poor)");
  else if (percent <= 55)
    strcpy(buf, " (average)");
  else if (percent <= 70)
    strcpy(buf, " (fair)");
  else if (percent <= 80)
    strcpy(buf, " (good)");
  else if (percent <= 85)
    strcpy(buf, " (very good)");
  else
    strcpy(buf, " (Superb)");

  return (buf);
}

static int GainLevel(struct char_data* ch, int class) {
  char buf[255];

  if (GET_EXP(ch) >= titles[class][GET_LEVEL(ch, class) + 1].exp) {
    send_to_char("You raise a level\n\r", ch);
    sprintf(buf, "%s just raised a level! Congratulate them!\n\r",
      GET_NAME(ch));
    send_to_all(buf);
    advance_level(ch, class);
    set_title(ch);

  } else {
    send_to_char("You haven't got enough experience!\n\r", ch);
  }
  return (FALSE);
}

struct char_data* FindMobInRoomWithFunction(int room, mob_proc_t func) {
  struct room_data* rp = real_roomp(room);

  if (room <= NOWHERE || !rp) {
    return NULL;
  }

  for (struct char_data* ch = rp->people; ch; ch = ch->next_in_room) {
    if (IS_MOB(ch) && mob_index[ch->nr].func.mob_f == func) {
      return ch;
    }
  }

  return NULL;
}

int AntiGuildMaster(struct char_data* ch, int cmd, char* arg) {
  int number, i, percent, sk_num;
  char buf[MAX_INPUT_LENGTH];
  struct char_data* guildmaster;
  char type[100], num[100];

  if ((cmd != 164) && (cmd != 170) && (cmd != 243))
    return (FALSE);

  if (!ch->skills)
    return (FALSE);

  if (IS_IMMORTAL(ch))
    return (FALSE);

  if (check_soundproof(ch))
    return (FALSE);

  guildmaster = FindMobInRoomWithFunction(ch->in_room, AntiGuildMaster);

  if (!guildmaster)
    return (FALSE);

  if (HasClass(ch, CLASS_ANTIPALADIN)) {
    if (cmd == 243) {
      if (GET_LEVEL(ch, ANTIPALADIN_LEVEL_IND) <
          GetMaxLevel(guildmaster) - 10) {
        GainLevel(ch, ANTIPALADIN_LEVEL_IND);
      } else {
        send_to_char("I cannot train you...you MUST find another.\n\r", ch);
      }
      return (TRUE);
    }

    if (!*arg) {
      sprintf(buf, "You have got %d practice sessions left.\n\r",
        ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      send_to_char("You can practice any of the following:\n\r", ch);
      for (i = 0; *spells[i] != '\n'; i++)
        if (spell_info[i + 1].spell_pointer &&
            (spell_info[i + 1].min_level_anti <=
              GET_LEVEL(ch, ANTIPALADIN_LEVEL_IND)) &&
            (spell_info[i + 1].min_level_anti <=
              GetMaxLevel(guildmaster) - 10)) {
          sprintf(buf, "[%d] %s %s\n\r", spell_info[i + 1].min_level_anti,
            spells[i], how_good(ch->skills[i + 1].learned));
          send_to_char(buf, ch);
        }
      sprintf(buf, " kick:    %s\n\r",
        how_good(ch->skills[SKILL_KICK].learned));
      send_to_char(buf, ch);
      sprintf(buf, " bash:    %s\n\r",
        how_good(ch->skills[SKILL_BASH].learned));
      send_to_char(buf, ch);
      sprintf(buf, " backstab:%s\n\r",
        how_good(ch->skills[SKILL_BACKSTAB].learned));
      send_to_char(buf, ch);
      sprintf(buf, " hide:    %s\n\r",
        how_good(ch->skills[SKILL_HIDE].learned));
      send_to_char(buf, ch);
      sprintf(buf, " double attack: %s\n\r",
        how_good(ch->skills[SKILL_DOUBLE_ATTACK].learned));
      send_to_char(buf, ch);
      return (TRUE);
    }

    one_argument(arg, type);

    only_argument(arg, num);

    if (is_abbrev(type, "kick")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_KICK].learned >= 75) {
        send_to_char("You are already learned in this area.\n\r", ch);
        return (TRUE);
      }
      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;
      percent = ch->skills[SKILL_KICK].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_KICK].learned = MIN(75, percent);
      return (TRUE);

      if (ch->skills[SKILL_KICK].learned >= 75) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }
    } else if (is_abbrev(type, "double")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("What are you stupid you have no practices.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_DOUBLE_ATTACK].learned >= 40) {
        send_to_char("You must be stupid you already know this skill.\n\r", ch);
        return (TRUE);
      }
      send_to_char("You practice for a while....\n\r", ch);
      ch->specials.spells_to_learn--;
      percent =
        ch->skills[SKILL_DOUBLE_ATTACK].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_DOUBLE_ATTACK].learned = MIN(40, percent);
      return (TRUE);

      if (ch->skills[SKILL_DOUBLE_ATTACK].learned >= 40) {
        send_to_char("You are all-knowing in this area.\n\r", ch);
        return (TRUE);
      }
    } else if (is_abbrev(type, "bash")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_BASH].learned >= 75) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent = ch->skills[SKILL_BASH].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_BASH].learned = MIN(75, percent);
      return (TRUE);
      if (ch->skills[SKILL_BASH].learned >= 75) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }
    } else if (is_abbrev(type, "hide")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_HIDE].learned >= 90) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent = ch->skills[SKILL_HIDE].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_HIDE].learned = MIN(90, percent);
      return (TRUE);

      if (ch->skills[SKILL_HIDE].learned >= 90) {
        send_to_char("You are learned in this area.\n\r", ch);
        return (TRUE);
      }
    } else if (is_abbrev(type, "backstab")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_BACKSTAB].learned >= 90) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent = ch->skills[SKILL_BACKSTAB].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_BACKSTAB].learned = MIN(90, percent);
      return (TRUE);
      if (ch->skills[SKILL_BACKSTAB].learned >= 90) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }
    }
    for (; isspace(*arg); arg++)
      ;
    number = old_search_block(arg, 0, strlen(arg), spells, FALSE);
    if (number == -1) {
      send_to_char("You do not know of this spell...\n\r", ch);
      return (TRUE);
    }
    if (GET_LEVEL(ch, ANTIPALADIN_LEVEL_IND) <
        spell_info[number].min_level_anti) {
      send_to_char("You do not know of this spell....\n\r", ch);
      return (TRUE);
    }
    if (GetMaxLevel(guildmaster) - 10 < spell_info[number].min_level_anti) {
      do_say(guildmaster, "I don't know of this spell.", 0);
      return (TRUE);
    }
    if (ch->specials.spells_to_learn <= 0) {
      send_to_char("You do not seem to be able to practice now.\n\r", ch);
      return (TRUE);
    }
    if (ch->skills[number].learned >= 95) {
      send_to_char("You are already learned in this area.\n\r", ch);
      return (TRUE);
    }

    send_to_char("You practice for a while...\n\r", ch);
    ch->specials.spells_to_learn--;

    percent = ch->skills[number].learned + int_app[GET_INT(ch)].learn;
    ch->skills[number].learned = MIN(95, percent);

    if (ch->skills[number].learned >= 95) {
      send_to_char("You are now learned in this area.\n\r", ch);
      return (TRUE);
    }
  } else {
    send_to_char("Oh...i bet you think you're an anti-paladin?\n\r", ch);
    return (FALSE);
  }
}

int MageGuildMaster(struct char_data* ch, int cmd, char* arg) {
  int number, i, percent;
  char buf[MAX_INPUT_LENGTH];
  struct char_data* guildmaster;
  char type[100];

  if ((cmd != 164) && (cmd != 170) && (cmd != 243))
    return (FALSE);

  if (!ch->skills)
    return (FALSE);

  if (IS_IMMORTAL(ch))
    return (FALSE);

  if (check_soundproof(ch))
    return (FALSE);

  guildmaster = FindMobInRoomWithFunction(ch->in_room, MageGuildMaster);

  if (!guildmaster)
    return (FALSE);

  if (HasClass(ch, CLASS_MAGIC_USER)) {
    if (cmd == 243) { /* gain */
      if (GET_LEVEL(ch, MAGE_LEVEL_IND) < GetMaxLevel(guildmaster) - 10) {
        GainLevel(ch, MAGE_LEVEL_IND);
      } else {
        send_to_char("I cannot train you.. You must find another.\n\r", ch);
      }
      return (TRUE);
    }

    if (!*arg) {
      sprintf(buf, "You have got %d practice sessions left.\n\r",
        ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      send_to_char("You can practise any of these spells:\n\r", ch);
      for (i = 0; *spells[i] != '\n'; i++)
        if (spell_info[i + 1].spell_pointer &&
            (spell_info[i + 1].min_level_magic <=
              GET_LEVEL(ch, MAGE_LEVEL_IND)) &&
            (spell_info[i + 1].min_level_magic <=
              GetMaxLevel(guildmaster) - 10)) {
          sprintf(buf, "[%d] %s %s ", spell_info[i + 1].min_level_magic,
            spells[i], how_good(ch->skills[i + 1].learned));
          if (IS_SET(spell_info[i + 1].targets, TAR_SINGLE)) {
            strcat(buf, "[Single class ONLY]");
          }

          strcat(buf, "\n\r");
          send_to_char(buf, ch);
        }
      sprintf(buf, "Scribe %s:\n\r",
        how_good(ch->skills[SKILL_SCRIBE].learned));
      send_to_char(buf, ch);
      sprintf(buf, "Brew %s:\n\r", how_good(ch->skills[SKILL_BREW].learned));
      send_to_char(buf, ch);
      return (TRUE);
    }

    one_argument(arg, type);

    if (is_abbrev(type, "brew")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You don't seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_BREW].learned >= 90) {
        send_to_char("You re already learned in that area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;
      percent = ch->skills[SKILL_BREW].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_BREW].learned = MIN(90, percent);
      return (TRUE);
    } else if (is_abbrev(type, "scribe")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You don't seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_SCRIBE].learned >= 90) {
        send_to_char("You are already learned in that area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;
      percent = ch->skills[SKILL_SCRIBE].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_SCRIBE].learned = MIN(90, percent);
      return (TRUE);
    } else {
      for (; isspace(*arg); arg++)
        ;
      number = old_search_block(arg, 0, strlen(arg), spells, FALSE);
      if (number == -1) {
        send_to_char("You do not know of this spell...\n\r", ch);
        return (TRUE);
      }
      if (GET_LEVEL(ch, MAGE_LEVEL_IND) < spell_info[number].min_level_magic) {
        send_to_char("You do not know of this spell...\n\r", ch);
        return (TRUE);
      }
      if (GetMaxLevel(guildmaster) - 10 < spell_info[number].min_level_magic) {
        do_say(guildmaster, "I don't know of this spell.", 0);
        return (TRUE);
      }
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }

      if (ch->skills[number].learned >= 95) {
        send_to_char("You are already learned in this area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You Practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent = ch->skills[number].learned + int_app[GET_INT(ch)].learn;
      ch->skills[number].learned = MIN(95, percent);

      if (ch->skills[number].learned >= 95) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }
    }
  } else {
    send_to_char("Oh.. i bet you think you're a magic user?\n\r", ch);
    return (FALSE);
  }
}

int PaladinGuildMaster(struct char_data* ch, int cmd, char* arg) {
  int number, i, percent, sk_num;
  char buf[MAX_INPUT_LENGTH];
  struct char_data* guildmaster;
  char type[100], num[100];

  if ((cmd != 164) && (cmd != 170) && (cmd != 243))
    return (FALSE);

  if (!ch->skills)
    return (FALSE);

  if (IS_IMMORTAL(ch))
    return (FALSE);

  if (check_soundproof(ch))
    return (FALSE);

  guildmaster = FindMobInRoomWithFunction(ch->in_room, PaladinGuildMaster);

  if (!guildmaster)
    return (FALSE);

  if (HasClass(ch, CLASS_PALADIN)) {
    if (cmd == 243) {
      if (GET_LEVEL(ch, PALADIN_LEVEL_IND) < GetMaxLevel(guildmaster) - 10) {
        GainLevel(ch, PALADIN_LEVEL_IND);
      } else {
        send_to_char("I cannot train you...you MUST find another.\n\r", ch);
      }
      return (TRUE);
    }

    if (!*arg) {
      sprintf(buf, "You have got %d practice sessions left.\n\r",
        ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      send_to_char("You can practice any of the following:\n\r", ch);
      for (i = 0; *spells[i] != '\n'; i++)
        if (spell_info[i + 1].spell_pointer &&
            (spell_info[i + 1].min_level_pal <=
              GET_LEVEL(ch, PALADIN_LEVEL_IND)) &&
            (spell_info[i + 1].min_level_pal <=
              GetMaxLevel(guildmaster) - 10)) {
          sprintf(buf, "[%d] %s %s\n\r", spell_info[i + 1].min_level_pal,
            spells[i], how_good(ch->skills[i + 1].learned));
          send_to_char(buf, ch);
        }
      sprintf(buf, " kick:    %s\n\r",
        how_good(ch->skills[SKILL_KICK].learned));
      send_to_char(buf, ch);
      sprintf(buf, " bash:    %s\n\r",
        how_good(ch->skills[SKILL_BASH].learned));
      send_to_char(buf, ch);
      sprintf(buf, " rescue:  %s\n\r",
        how_good(ch->skills[SKILL_RESCUE].learned));
      send_to_char(buf, ch);
      sprintf(buf, " double attack: %s\n\r",
        how_good(ch->skills[SKILL_DOUBLE_ATTACK].learned));
      send_to_char(buf, ch);
      sprintf(buf, " lay hands: %s\n\r",
        how_good(ch->skills[SKILL_LAY_HANDS].learned));
      send_to_char(buf, ch);
      return (TRUE);
    }

    one_argument(arg, type);

    if (is_abbrev(type, "kick")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_KICK].learned >= 75) {
        send_to_char("You are already learned in this area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent = ch->skills[SKILL_KICK].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_KICK].learned = MIN(75, percent);
      return (TRUE);
      if (ch->skills[SKILL_KICK].learned >= 75) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }
    } else if (is_abbrev(type, "double")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_DOUBLE_ATTACK].learned >= 50) {
        send_to_char("You are already learned in this area.\n\r", ch);
        return (TRUE);
      }
      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent =
        ch->skills[SKILL_DOUBLE_ATTACK].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_DOUBLE_ATTACK].learned = MIN(50, percent);
      return (TRUE);
    } else if (is_abbrev(type, "bash")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_BASH].learned >= 75) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent = ch->skills[SKILL_BASH].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_BASH].learned = MIN(75, percent);
      return (TRUE);
      if (ch->skills[SKILL_BASH].learned >= 75) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }
    } else if (is_abbrev(type, "rescue")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_RESCUE].learned >= 90) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent = ch->skills[SKILL_RESCUE].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_RESCUE].learned = MIN(90, percent);
      return (TRUE);

      if (ch->skills[SKILL_RESCUE].learned >= 90) {
        send_to_char("You are learned in this area.\n\r", ch);
        return (TRUE);
      }
    } else if (is_abbrev(type, "lay")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_LAY_HANDS].learned >= 90) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }
      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent =
        ch->skills[SKILL_LAY_HANDS].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_LAY_HANDS].learned = MIN(90, percent);
      return (TRUE);

      if (ch->skills[SKILL_LAY_HANDS].learned >= 90) {
        send_to_char("You are already learned in this area.\n\r", ch);
        return (TRUE);
      }
    } else {
      for (; isspace(*arg); arg++)
        ;
      number = old_search_block(arg, 0, strlen(arg), spells, FALSE);
      if (number == -1) {
        send_to_char("You do not know of this spell...\n\r", ch);
        return (TRUE);
      }
      if (GET_LEVEL(ch, PALADIN_LEVEL_IND) < spell_info[number].min_level_pal) {
        send_to_char("You do not know of this spell....\n\r", ch);
        return (TRUE);
      }
      if (GetMaxLevel(guildmaster) - 10 < spell_info[number].min_level_pal) {
        do_say(guildmaster, "I don't know of this spell.", 0);
        return (TRUE);
      }
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[number].learned >= 95) {
        send_to_char("You are already learned in this area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent = ch->skills[number].learned + int_app[GET_INT(ch)].learn;
      ch->skills[number].learned = MIN(95, percent);

      if (ch->skills[number].learned >= 95) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }
    }
  } else {
    send_to_char("Oh...i bet you think you're an paladin?\n\r", ch);
    return (FALSE);
  }
}

int RangerGuildMaster(struct char_data* ch, int cmd, char* arg) {
  int number, i, percent, sk_num;
  char buf[MAX_INPUT_LENGTH];
  struct char_data* guildmaster;
  char type[100], num[100];

  if ((cmd != 164) && (cmd != 170) && (cmd != 243))
    return (FALSE);

  if (!ch->skills)
    return (FALSE);

  if (IS_IMMORTAL(ch))
    return (FALSE);

  if (check_soundproof(ch))
    return (FALSE);

  guildmaster = FindMobInRoomWithFunction(ch->in_room, RangerGuildMaster);

  if (!guildmaster)
    return (FALSE);

  if (HasClass(ch, CLASS_RANGER)) {
    if (cmd == 243) {
      if (GET_LEVEL(ch, RANGER_LEVEL_IND) < GetMaxLevel(guildmaster) - 10) {
        GainLevel(ch, RANGER_LEVEL_IND);
      } else {
        send_to_char("I cannot train you...you MUST find another.\n\r", ch);
      }
      return (TRUE);
    }

    if (!*arg) {
      sprintf(buf, "You have got %d practice sessions left.\n\r",
        ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      send_to_char("You can practice any of the following:\n\r", ch);
      for (i = 0; *spells[i] != '\n'; i++)
        if (spell_info[i + 1].spell_pointer &&
            (spell_info[i + 1].min_level_ranger <=
              GET_LEVEL(ch, RANGER_LEVEL_IND)) &&
            (spell_info[i + 1].min_level_ranger <=
              GetMaxLevel(guildmaster) - 10)) {
          sprintf(buf, "[%d] %s %s\n\r", spell_info[i + 1].min_level_ranger,
            spells[i], how_good(ch->skills[i + 1].learned));
          send_to_char(buf, ch);
        }
      sprintf(buf, " kick:    %s\n\r",
        how_good(ch->skills[SKILL_KICK].learned));
      send_to_char(buf, ch);
      sprintf(buf, " track:    %s\n\r",
        how_good(ch->skills[SKILL_HUNT].learned));
      send_to_char(buf, ch);
      sprintf(buf, " bash:    %s\n\r",
        how_good(ch->skills[SKILL_BASH].learned));
      send_to_char(buf, ch);
      sprintf(buf, " sneak:%s\n\r", how_good(ch->skills[SKILL_SNEAK].learned));
      send_to_char(buf, ch);
      sprintf(buf, " rescue:    %s\n\r",
        how_good(ch->skills[SKILL_RESCUE].learned));
      send_to_char(buf, ch);
      sprintf(buf, "double attack:   %s\n\r",
        how_good(ch->skills[SKILL_DOUBLE_ATTACK].learned));
      send_to_char(buf, ch);
      return (TRUE);
    }

    for (; isspace(*arg); arg++)
      ;

    one_argument(arg, type);

    if (is_abbrev(type, "kick")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_KICK].learned >= 75) {
        send_to_char("You are already learned in this area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent = ch->skills[SKILL_KICK].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_KICK].learned = MIN(75, percent);
      return (TRUE);

      if (ch->skills[SKILL_KICK].learned >= 75) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }
    } else if (is_abbrev(type, "double")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You don't seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_DOUBLE_ATTACK].learned >= 60) {
        send_to_char("You are already learned in this area.\n\r", ch);
        return (TRUE);
      }
      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent =
        ch->skills[SKILL_DOUBLE_ATTACK].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_DOUBLE_ATTACK].learned = MIN(60, percent);
      return (TRUE);

      if (ch->skills[SKILL_DOUBLE_ATTACK].learned >= 60) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }
    } else if (is_abbrev(type, "track")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_HUNT].learned >= 90) {
        send_to_char("You are already learned in this area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent = ch->skills[SKILL_HUNT].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_HUNT].learned = MIN(75, percent);
      return (TRUE);

      if (ch->skills[SKILL_HUNT].learned >= 90) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }
    } else if (is_abbrev(type, "bash")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_BASH].learned >= 75) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent = ch->skills[SKILL_BASH].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_BASH].learned = MIN(75, percent);
      return (TRUE);

      if (ch->skills[SKILL_BASH].learned >= 75) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }
    } else if (is_abbrev(type, "sneak")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_SNEAK].learned >= 90) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent = ch->skills[SKILL_SNEAK].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_SNEAK].learned = MIN(90, percent);
      return (TRUE);

      if (ch->skills[SKILL_SNEAK].learned >= 90) {
        send_to_char("You are learned in this area.\n\r", ch);
        return (TRUE);
      }
    } else if (is_abbrev(type, "rescue")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_RESCUE].learned >= 90) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent = ch->skills[SKILL_RESCUE].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_RESCUE].learned = MIN(90, percent);
      return (TRUE);

      if (ch->skills[SKILL_RESCUE].learned >= 90) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }
    } else {
      for (; isspace(*arg); arg++)
        ;
      number = old_search_block(arg, 0, strlen(arg), spells, FALSE);
      if (number == -1) {
        send_to_char("You do not know of this spell...\n\r", ch);
        return (TRUE);
      }
      if (GET_LEVEL(ch, RANGER_LEVEL_IND) <
          spell_info[number].min_level_ranger) {
        send_to_char("You do not know of this spell....\n\r", ch);
        return (TRUE);
      }
      if (GetMaxLevel(guildmaster) - 10 < spell_info[number].min_level_ranger) {
        do_say(guildmaster, "I don't know of this spell.", 0);
        return (TRUE);
      }
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[number].learned >= 95) {
        send_to_char("You are already learned in this area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent = ch->skills[number].learned + int_app[GET_INT(ch)].learn;
      ch->skills[number].learned = MIN(95, percent);

      if (ch->skills[number].learned >= 95) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }
    }
  } else {
    send_to_char("Oh...i bet you think you're an ranger?\n\r", ch);
    return (FALSE);
  }
}

int ClericGuildMaster(struct char_data* ch, int cmd, char* arg) {
  int number, i, percent;
  char buf[MAX_INPUT_LENGTH];
  struct char_data* guildmaster;
  char type[100];

  if (!ch->skills)
    return (FALSE);

  if (check_soundproof(ch))
    return (FALSE);

  if ((cmd != 164) && (cmd != 170) && (cmd != 243))
    return (FALSE);

  if (IS_IMMORTAL(ch))
    return (FALSE);

  guildmaster = FindMobInRoomWithFunction(ch->in_room, ClericGuildMaster);

  if (!guildmaster)
    return (FALSE);

  if (HasClass(ch, CLASS_CLERIC)) {
    if (cmd == 243) { /* gain */
      if (GET_LEVEL(ch, CLERIC_LEVEL_IND) < GetMaxLevel(guildmaster) - 10) {
        GainLevel(ch, CLERIC_LEVEL_IND);
      } else {
        send_to_char("I cannot train you.. You must find another.\n\r", ch);
      }
      return (TRUE);
    }

    if (!*arg) {
      sprintf(buf, "You have got %d practice sessions left.\n\r",
        ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      send_to_char("You can practise any of these spells:\n\r", ch);
      for (i = 0; *spells[i] != '\n'; i++)
        if (spell_info[i + 1].spell_pointer &&
            (spell_info[i + 1].min_level_cleric <=
              GET_LEVEL(ch, CLERIC_LEVEL_IND)) &&
            (spell_info[i + 1].min_level_cleric <=
              GetMaxLevel(guildmaster) - 10)) {
          sprintf(buf, "[%d] %s %s\n\r", spell_info[i + 1].min_level_cleric,
            spells[i], how_good(ch->skills[i + 1].learned));
          send_to_char(buf, ch);
        }
      sprintf(buf, "Brew %s\n\r", how_good(ch->skills[SKILL_BREW].learned));
      send_to_char(buf, ch);
      sprintf(buf, "Scribe %s\n\r", how_good(ch->skills[SKILL_SCRIBE].learned));
      send_to_char(buf, ch);
      return (TRUE);
    }

    one_argument(arg, type);

    if (is_abbrev(type, "brew")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You don't seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_BREW].learned >= 90) {
        send_to_char("You are already learned in that area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;
      percent = ch->skills[SKILL_BREW].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_BREW].learned = MIN(90, percent);
      return (TRUE);
    } else if (is_abbrev(type, "scribe")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You don't seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_SCRIBE].learned >= 90) {
        send_to_char("You are already learned in that area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;
      percent = ch->skills[SKILL_SCRIBE].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_SCRIBE].learned = MIN(90, percent);
      return (TRUE);
    } else {
      for (; isspace(*arg); arg++)
        ;
      number = old_search_block(arg, 0, strlen(arg), spells, FALSE);
      if (number == -1) {
        send_to_char("You do not know of this spell...\n\r", ch);
        return (TRUE);
      }
      if (GET_LEVEL(ch, CLERIC_LEVEL_IND) <
          spell_info[number].min_level_cleric) {
        send_to_char("You do not know of this spell...\n\r", ch);
        return (TRUE);
      }
      if (GetMaxLevel(guildmaster) - 10 < spell_info[number].min_level_cleric) {
        do_say(guildmaster, "I don't know of this spell.", 0);
        return (TRUE);
      }
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[number].learned >= 95) {
        send_to_char("You are already learned in this area.\n\r", ch);
        return (TRUE);
      }
      send_to_char("You Practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent = ch->skills[number].learned + int_app[GET_INT(ch)].learn;
      ch->skills[number].learned = MIN(95, percent);

      if (ch->skills[number].learned >= 95) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }
    }
  } else {
    send_to_char("What do you think you are, a cleric??\n\r", ch);
    return (FALSE);
  }
}

int ThiefGuildMaster(struct char_data* ch, int cmd, char* arg) {
  int number, i, percent;
  char buf[MAX_INPUT_LENGTH];
  struct char_data* guildmaster;
  char type[100];

  static char* t_skills[] = {"sneak", "hide", "steal", "backstab", "pick",
    "\n"};

  if ((cmd != 164) && (cmd != 170) && (cmd != 243))
    return (FALSE);

  if (!ch->skills)
    return (FALSE);

  if (IS_IMMORTAL(ch))
    return (FALSE);

  if (check_soundproof(ch))
    return (FALSE);

  guildmaster = FindMobInRoomWithFunction(ch->in_room, ThiefGuildMaster);

  if (!guildmaster)
    return (FALSE);

  if (HasClass(ch, CLASS_THIEF)) {
    if (cmd == 243) { /* gain */
      if (GET_LEVEL(ch, THIEF_LEVEL_IND) < GetMaxLevel(guildmaster) - 10) {
        GainLevel(ch, THIEF_LEVEL_IND);
      } else {
        send_to_char("I cannot train you.. You must find another.\n\r", ch);
      }
      return (TRUE);
    }
    if (!*arg) {
      sprintf(buf, "You have got %d practice sessions left.\n\r",
        ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      send_to_char("You can practise any of these skills:\n\r", ch);
      for (i = 0; *t_skills[i] != '\n'; i++) {
        send_to_char(t_skills[i], ch);
        send_to_char(how_good(ch->skills[i + 45].learned), ch);
        send_to_char("\n\r", ch);
      }
      sprintf(buf, "Subterfuge %s\n\r",
        how_good(ch->skills[SKILL_SUBTERFUGE].learned));
      send_to_char(buf, ch);
      sprintf(buf, "Detect Secret %s\n\r",
        how_good(ch->skills[SKILL_DETECT_SECRET].learned));
      send_to_char(buf, ch);
      sprintf(buf, "Double Attack %s\n\r",
        how_good(ch->skills[SKILL_DOUBLE_ATTACK].learned));
      send_to_char(buf, ch);
      return (TRUE);
    }
    one_argument(arg, type);

    if (is_abbrev(type, "subterfuge")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_SUBTERFUGE].learned >= 90) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }
      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent =
        ch->skills[SKILL_SUBTERFUGE].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_SUBTERFUGE].learned = MIN(90, percent);
      return (TRUE);

      if (ch->skills[SKILL_SUBTERFUGE].learned >= 90) {
        send_to_char("You are learned in this area.\n\r", ch);
        return (TRUE);
      }
    } else if (is_abbrev(type, "detect")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_DETECT_SECRET].learned >= 75) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }
      send_to_char("You practice for a while...nr", ch);
      ch->specials.spells_to_learn--;

      percent =
        ch->skills[SKILL_DETECT_SECRET].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_DETECT_SECRET].learned = MIN(75, percent);
      return (TRUE);

      if (ch->skills[SKILL_DETECT_SECRET].learned >= 75) {
        send_to_char("You are learned in this area.\n\r", ch);
        return (TRUE);
      }
    } else if (is_abbrev(type, "double")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You don't seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_DOUBLE_ATTACK].learned >= 50) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }
      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent =
        ch->skills[SKILL_DOUBLE_ATTACK].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_DOUBLE_ATTACK].learned = MIN(50, percent);
      return (TRUE);

      if (ch->skills[SKILL_DOUBLE_ATTACK].learned >= 50) {
        send_to_char("You are learned in this area.\n\r", ch);
        return (TRUE);
      }
    } else {
      for (; isspace(*arg); arg++)
        ;
      number = search_block(arg, t_skills, FALSE);
      if (number == -1) {
        send_to_char("You do not know of this skill...\n\r", ch);
        return (TRUE);
      }
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[number + SKILL_SNEAK].learned >= 90) {
        send_to_char("You are already learned in this area.\n\r", ch);
        return (TRUE);
      }
      send_to_char("You Practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent =
        ch->skills[number + SKILL_SNEAK].learned + int_app[GET_INT(ch)].learn;
      ch->skills[number + SKILL_SNEAK].learned = MIN(90, percent);

      if (ch->skills[number + SKILL_SNEAK].learned >= 90) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }
    }
  } else {
    send_to_char("What do you think you are, a thief??\n\r", ch);
    return (FALSE);
  }
}

int WarriorGuildMaster(struct char_data* ch, int cmd, char* arg) {
  int number, i, percent;
  char buf[MAX_INPUT_LENGTH];
  struct char_data* guildmaster;
  char type[100];
  static char* w_skills[] = {"kick", /* No. 50 */
    "bash", "rescue", "\n"};

  if ((cmd != 164) && (cmd != 170) && (cmd != 243))
    return (FALSE);

  if (!ch->skills)
    return (FALSE);

  if (IS_IMMORTAL(ch))
    return (FALSE);

  if (check_soundproof(ch))
    return (FALSE);

  guildmaster = FindMobInRoomWithFunction(ch->in_room, WarriorGuildMaster);

  if (!guildmaster)
    return (FALSE);

  if (HasClass(ch, CLASS_WARRIOR)) {
    if (cmd == 243) { /* gain */
      if (GET_LEVEL(ch, WARRIOR_LEVEL_IND) < GetMaxLevel(guildmaster) - 10) {
        GainLevel(ch, WARRIOR_LEVEL_IND);
      } else {
        send_to_char("I cannot train you.. You must find another.\n\r", ch);
      }
      return (TRUE);
    }

    if (!*arg) {
      sprintf(buf, "You have got %d practice sessions left.\n\r",
        ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      send_to_char("You can practise any of these skills:\n\r", ch);
      for (i = 0; *w_skills[i] != '\n'; i++) {
        send_to_char(w_skills[i], ch);
        send_to_char(how_good(ch->skills[i + SKILL_KICK].learned), ch);
        send_to_char("\n\r", ch);
      }
      sprintf(buf, "Headbutt %s\n\r",
        how_good(ch->skills[SKILL_HEADBUTT].learned));
      send_to_char(buf, ch);
      sprintf(buf, "Throw %s\n\r", how_good(ch->skills[SKILL_THROW].learned));
      send_to_char(buf, ch);
      sprintf(buf, "Grapple %s\n\r",
        how_good(ch->skills[SKILL_GRAPPLE].learned));
      send_to_char(buf, ch);
      sprintf(buf, "Double Attack %s\n\r",
        how_good(ch->skills[SKILL_DOUBLE_ATTACK].learned));
      send_to_char(buf, ch);
      sprintf(buf, "Deathstroke %s\n\r",
        how_good(ch->skills[SKILL_DEATHSTROKE].learned));
      send_to_char(buf, ch);
      sprintf(buf, "Bodyslam %s\n\r",
        how_good(ch->skills[SKILL_BODYSLAM].learned));
      send_to_char(buf, ch);
      return (TRUE);
    }

    one_argument(arg, type);

    if (is_abbrev(type, "throw")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_THROW].learned >= 90) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;
      percent = ch->skills[SKILL_THROW].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_THROW].learned = MIN(90, percent);
      return (TRUE);
    } else if (is_abbrev(type, "double")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_DOUBLE_ATTACK].learned >= 90) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;
      percent =
        ch->skills[SKILL_DOUBLE_ATTACK].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_DOUBLE_ATTACK].learned = MIN(90, percent);
      return (TRUE);
    } else if (is_abbrev(type, "deathstroke")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_DEATHSTROKE].learned >= 75) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;
      percent = ch->skills[SKILL_DEATHSTROKE].learned +
                (int_app[GET_INT(ch)].learn / 2);
      ch->skills[SKILL_DEATHSTROKE].learned = MIN(75, percent);
      return (TRUE);
    } else if (is_abbrev(type, "bodyslam")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_BODYSLAM].learned >= 75) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return TRUE;
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;
      percent =
        ch->skills[SKILL_BODYSLAM].learned + (int_app[GET_INT(ch)].learn / 2);
      ch->skills[SKILL_BODYSLAM].learned = MIN(75, percent);
      return (TRUE);
    } else if (is_abbrev(type, "grapple")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_GRAPPLE].learned >= 75) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;
      percent = ch->skills[SKILL_GRAPPLE].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_GRAPPLE].learned = MIN(75, percent);
      return (TRUE);
    } else if (is_abbrev(type, "headbutt")) {
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[SKILL_HEADBUTT].learned >= 90) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }

      send_to_char("You practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent = ch->skills[SKILL_HEADBUTT].learned + int_app[GET_INT(ch)].learn;
      ch->skills[SKILL_HEADBUTT].learned = MIN(90, percent);
      return (TRUE);

      if (ch->skills[SKILL_HEADBUTT].learned >= 90) {
        send_to_char("You are learned in this area.\n\r", ch);
        return (TRUE);
      }
    } else {
      for (; isspace(*arg); arg++)
        ;
      number = search_block(arg, w_skills, FALSE);
      if (number == -1) {
        send_to_char("You do not have ability to practise this skill!\n\r", ch);
        return (TRUE);
      }
      if (ch->specials.spells_to_learn <= 0) {
        send_to_char("You do not seem to be able to practice now.\n\r", ch);
        return (TRUE);
      }
      if (ch->skills[number + SKILL_KICK].learned >= 90) {
        send_to_char("You are already learned in this area.\n\r", ch);
        return (TRUE);
      }
      send_to_char("You Practice for a while...\n\r", ch);
      ch->specials.spells_to_learn--;

      percent =
        ch->skills[number + SKILL_KICK].learned + int_app[GET_INT(ch)].learn;
      ch->skills[number + SKILL_KICK].learned = MIN(90, percent);

      if (ch->skills[number + SKILL_KICK].learned >= 90) {
        send_to_char("You are now learned in this area.\n\r", ch);
        return (TRUE);
      }
    }
  } else {
    send_to_char("Oh.. i bet you think you're a fighter??\n\r", ch);
    return (FALSE);
  }
}

int no_order(struct char_data* ch, int cmd, char* arg) {
  if ((cmd == 87) || (IS_MOB(ch)))
    return (TRUE);

  return (FALSE);
}

int mag_room(struct char_data* ch, int cmd, char* arg) {
  if ((cmd == 1) || (cmd == 2) || (cmd == 3) || (cmd == 4) || (cmd == 5) ||
      (cmd == 6) || (cmd == 151) || (cmd == 207)) {
    send_to_char("You are unable to escape the power of magneto!\n\r", ch);
    return (TRUE);
  } else {
    return (FALSE);
  }
}

#define INC18(X) X = ((X < 18) ? X + 1 : X)
#define COST 1000000
#define COST_FOR_HIGHER_STATS 20000000
#define COST_FOR_HUNGER 5000000

int metahospital(struct char_data* ch, int cmd, char* arg) {
  char buf[MAX_STRING_LENGTH];
  int i, k, opt, cost;

  if (IS_NPC(ch))
    return (FALSE);
  if (cmd == 59) { /* List */
    send_to_char("1 - Strength improvement.         Price 1000000 exp\n\r", ch);
    send_to_char("2 - Dexterity lesson.             Price 1000000 exp\n\r", ch);
    send_to_char("3 - Wisdom enhancement.           Price 1000000 exp\n\r", ch);
    send_to_char("4 - Intelligence addition.        Price 1000000 exp\n\r", ch);
    send_to_char("5 - Hit points inflator.          Price 1000000 exp\n\r", ch);
    send_to_char("6 - Improved Constitution.        Price 1000000 exp\n\r", ch);
    send_to_char("7 - Freedom from hunger.          Price 5000000 exp\n\r", ch);
    send_to_char("8 - Freedom from thirst.          Price 5000000 exp\n\r", ch);
    send_to_char("9 - Hobbit's dexterity to 19.     Price 20000000 exp\n\r",
      ch);
    send_to_char("10 - Ogre's strength to 19.       Price 20000000 exp\n\r",
      ch);
    send_to_char("11 - Elf's intelligence to 19.    Price 20000000 exp\n\r",
      ch);
    send_to_char("12 - Gnome's wisdom to 19.        Price 20000000 exp\n\r",
      ch);
    send_to_char("13 - Dwarf's constitution to 19.  Price 20000000 exp\n\r",
      ch);
    return (TRUE);
  } else if (cmd == 56) { /* Buy */
    arg = one_argument(arg, buf);
    opt = atoi(buf);

    for (i = 0; i < MAX_WEAR; i++) {
      if (ch->equipment[i]) {
        send_to_char(
          "You must remove all to use the metaphysician. Come on now! Don't be "
          "bashful!\n\r",
          ch);
        return (TRUE);
      }
    }

    if ((opt >= 1) && (opt <= 6)) {
      if (COST > GET_EXP(ch)) {
        send_to_char("Come back when you are more experienced.\n\r", ch);
        return (TRUE);
      }
      ch->points.exp -= COST;
    } else if ((opt >= 7) && (opt <= 8)) {
      cost = COST_FOR_HUNGER;
      if (cost > GET_EXP(ch)) {
        send_to_char("Come back when you are more experienced.\n\r", ch);
        return (TRUE);
      }
      ch->points.exp -= cost;
    }
    switch (opt) {
      case 1:
        k = ch->abilities.str;
        if (k < 18) {
          ++k;
          ch->abilities.str = k;
        } else {
          k = 100 - ch->abilities.str_add;
          ch->abilities.str_add = 100 - k / 2;
        }
        send_to_char("GROWL...\n\r", ch);
        return (TRUE);
        break;
      case 2:
        INC18(ch->abilities.dex);
        send_to_char("Jack be nimble, Jack be quick...\n\r", ch);
        return (TRUE);
        break;
      case 3:
        INC18(ch->abilities.wis);
        send_to_char("You feel philosophical.\n\r", ch);
        return (TRUE);
        break;
      case 4:
        INC18(ch->abilities.intel);
        send_to_char("You feel like reading Sartre.\n\r", ch);
        return (TRUE);
        break;
      case 5:
        k = ch->points.max_hit;
        if (k < 500)
          ch->points.max_hit += number(1, 5);
        else if (k < 1000)
          ch->points.max_hit += number(1, 4);
        else
          ch->points.max_hit += number(1, 2);
        send_to_char("@>->->--\n\r", ch);
        return (TRUE);
        break;
      case 6:
        INC18(ch->abilities.con);
        send_to_char("O-------O\n\r", ch);
        return (TRUE);
        break;
      case 7:
        ch->specials.conditions[1] = (-1);
        send_to_char("You will never again be hungry.\n\r", ch);
        return (TRUE);
        break;
      case 8:
        ch->specials.conditions[2] = (-1);
        send_to_char("You will never again be thirsty.\n\r", ch);
        return (TRUE);
        break;
      case 9:
        cost = COST_FOR_HIGHER_STATS;
        if (GET_RACE(ch) != RACE_HOBBIT) {
          send_to_char("You are no hobbit!\n\r", ch);
          return (TRUE);
        }
        if (GET_EXP(ch) < cost) {
          send_to_char(
            "A nineteen stat requires a very experienced person, sorry.\n\r",
            ch);
          return (TRUE);
        }
        send_to_char("You will now sit and learn the arts of dexterity.\n\r",
          ch);
        GET_EXP(ch) -= cost;
        ch->abilities.dex = 19;
        ch->tmpabilities.dex = 19;
        do_save(ch, "", 0);
        return (TRUE);
        break;
      case 10:
        cost = COST_FOR_HIGHER_STATS;
        if (GET_RACE(ch) != RACE_OGRE) {
          send_to_char("You are no ogre!\n\r", ch);
          return (TRUE);
        }
        if (GET_EXP(ch) < cost) {
          return (TRUE);
        }
        send_to_char("You feel your muscles tingle greatly.\n\r", ch);
        GET_EXP(ch) -= cost;
        ch->abilities.str = 19;
        ch->tmpabilities.str = 19;
        do_save(ch, "", 0);
        return (TRUE);
        break;
      case 11:
        cost = COST_FOR_HIGHER_STATS;
        if (GET_RACE(ch) != RACE_ELVEN) {
          send_to_char("You are no elf!\n\r", ch);
          return (TRUE);
        }
        if (GET_EXP(ch) < cost) {
          return (TRUE);
        }
        send_to_char("You suddenly feel much smarter.\n\r", ch);
        GET_EXP(ch) -= cost;
        ch->abilities.intel = 19;
        ch->tmpabilities.intel = 19;
        do_save(ch, "", 0);
        return (TRUE);
        break;
      case 12:
        cost = COST_FOR_HIGHER_STATS;
        if (GET_RACE(ch) != RACE_GNOME) {
          send_to_char("You are no gnome!\n\r", ch);
          return (TRUE);
        }
        if (GET_EXP(ch) < cost) {
          return (TRUE);
        }
        send_to_char("You suddenly feel much older and wiser.\n\r", ch);
        GET_EXP(ch) -= cost;
        ch->abilities.wis = 19;
        ch->tmpabilities.wis = 19;
        do_save(ch, "", 0);
        return (TRUE);
        break;
      case 13:
        cost = COST_FOR_HIGHER_STATS;
        if (GET_RACE(ch) != RACE_DWARF) {
          send_to_char("You are no dwarf!\n\r", ch);
          return (TRUE);
        }
        if (GET_EXP(ch) < cost) {
          return (TRUE);
        }
        send_to_char("You suddenly feel much more sturdy.\n\r", ch);
        GET_EXP(ch) -= cost;
        ch->abilities.con = 19;
        ch->tmpabilities.con = 19;
        do_save(ch, "", 0);
        return (TRUE);
        break;
    }
    if (opt < 5)
      ch->tmpabilities = ch->abilities;
  } else {
    return (FALSE);
  }
  return (FALSE);
}

int dump(struct char_data* ch, int cmd, char* arg) {
  struct obj_data* k;
  char buf[100];
  struct char_data* tmp_char;
  int value = 0;

  void do_drop(struct char_data * ch, char* argument, int cmd);
  char* fname(char* namelist);

  for (k = real_roomp(ch->in_room)->contents; k;
       k = real_roomp(ch->in_room)->contents) {
    sprintf(buf, "The %s vanish in a puff of smoke.\n\r", fname(k->name));
    for (tmp_char = real_roomp(ch->in_room)->people; tmp_char;
         tmp_char = tmp_char->next_in_room)
      if (CAN_SEE_OBJ(tmp_char, k))
        send_to_char(buf, tmp_char);
    extract_obj(k);
  }

  if (cmd != 60)
    return (FALSE);

  do_drop(ch, arg, cmd);

  value = 0;

  for (k = real_roomp(ch->in_room)->contents; k;
       k = real_roomp(ch->in_room)->contents) {
    sprintf(buf, "The %s vanish in a puff of smoke.\n\r", fname(k->name));
    for (tmp_char = real_roomp(ch->in_room)->people; tmp_char;
         tmp_char = tmp_char->next_in_room)
      if (CAN_SEE_OBJ(tmp_char, k))
        send_to_char(buf, tmp_char);
    value += (MIN(1000, MAX(k->obj_flags.cost / 4, 1)));
    /*
value += MAX(1, MIN(50, k->obj_flags.cost/10));
*/
    extract_obj(k);
  }

  if (value) {
    act("You are awarded for outstanding performance.", FALSE, ch, 0, 0,
      TO_CHAR);
    act("$n has been awarded for being a good citizen.", TRUE, ch, 0, 0,
      TO_ROOM);

    if (GetMaxLevel(ch) < 3)
      gain_exp(ch, MIN(100, value));
    else
      GET_GOLD(ch) += value;
  }
}

int mayor(struct char_data* ch, int cmd, char* arg) {
  static char open_path[] =
    "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

  static char close_path[] =
    "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

  static char* path;
  static int index;
  static bool move = FALSE;

  void do_move(struct char_data * ch, char* argument, int cmd);
  void do_open(struct char_data * ch, char* argument, int cmd);
  void do_lock(struct char_data * ch, char* argument, int cmd);
  void do_unlock(struct char_data * ch, char* argument, int cmd);
  void do_close(struct char_data * ch, char* argument, int cmd);

  if (!move) {
    if (time_info.hours == 6) {
      move = TRUE;
      path = open_path;
      index = 0;
    } else if (time_info.hours == 20) {
      move = TRUE;
      path = close_path;
      index = 0;
    }
  }

  if (cmd || !move || (GET_POS(ch) < POSITION_SLEEPING) ||
      (GET_POS(ch) == POSITION_FIGHTING))
    return FALSE;

  switch (path[index]) {
    case '0':
    case '1':
    case '2':
    case '3':
      do_move(ch, "", path[index] - '0' + 1);
      break;

    case 'W':
      GET_POS(ch) = POSITION_STANDING;
      act("$n awakens and groans loudly.", FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'S':
      GET_POS(ch) = POSITION_SLEEPING;
      act("$n lies down and instantly falls asleep.", FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'a':
      if (check_soundproof(ch))
        return (FALSE);
      act("$n says 'Hello Honey!'", FALSE, ch, 0, 0, TO_ROOM);
      act("$n smirks.", FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'b':
      if (check_soundproof(ch))
        return (FALSE);
      act("$n says 'What a view! I must get something done about that dump!'",
        FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'c':
      if (check_soundproof(ch))
        return (FALSE);
      act(
        "$n says 'Vandals! Youngsters nowadays have no respect for anything!'",
        FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'd':
      if (check_soundproof(ch))
        return (FALSE);
      act("$n says 'Good day, citizens!'", FALSE, ch, 0, 0, TO_ROOM);
      break;

    case 'e':
      if (check_soundproof(ch))
        return (FALSE);
      act("$n says 'I hereby declare the bazaar open!'", FALSE, ch, 0, 0,
        TO_ROOM);
      break;

    case 'E':
      if (check_soundproof(ch))
        return (FALSE);
      act("$n says 'I hereby declare Midgaard closed!'", FALSE, ch, 0, 0,
        TO_ROOM);
      break;

    case 'O':
      do_unlock(ch, "gate", 0);
      do_open(ch, "gate", 0);
      break;

    case 'C':
      do_close(ch, "gate", 0);
      do_lock(ch, "gate", 0);
      break;

    case '.':
      move = FALSE;
      break;
  }

  index++;
  return FALSE;
}

int andy_wilcox(struct char_data* ch, int cmd, char* arg) {
#define THE_PUB 3940
#define ACT_OVER_21 1
#define ACT_SNICKER 2
  static int open = 1; /* 0 closed;  1 open;  2 last call */
  char argm[100], newarg[100], buf[MAX_STRING_LENGTH];
  struct obj_data *temp1, *temp2;
  struct char_data* temp_char;
  struct char_data* andy;
  int num, i, cost;

  static struct pub_beers {
      int container, contains, quant, actflag;
  } sold_here[] = {{3903, 3902, 6, 1}, {3905, 3904, 6, 1}, {3907, 3906, 6, 1},
    {3909, 3908, 6, 3}, {3911, 3910, 6, 3}, {3913, 3912, 6, 3}, {3914, 0, 0, 1},
    {3930, 0, 0, 0}, {3931, 0, 0, 0}, {3932, 0, 0, 0}, {3102, 0, 0, 0}, {-1}},
    *scan;

  andy = 0;

  if (check_soundproof(ch))
    return (FALSE);

  for (temp_char = real_roomp(ch->in_room)->people; (!andy) && (temp_char);
       temp_char = temp_char->next_in_room)
    if (IS_MOB(temp_char))
      if (mob_index[temp_char->nr].func.mob_f == andy_wilcox)
        andy = temp_char;

  if (open == 0 && time_info.hours == 11) {
    open = 1;
    do_unlock(andy, "door", 0);
    do_open(andy, "door", 0);
    act("$n says 'We're open for lunch, come on in.'", FALSE, andy, 0, 0,
      TO_ROOM);
  }
  if (open == 1 && time_info.hours == 1) {
    open = 2;
    act("$n says 'Last call, guys and gals.'", FALSE, andy, 0, 0, TO_ROOM);
  }
  if (open == 2 && time_info.hours == 2) {
    open = 0;
    act(
      "$n says 'We're closing for the night.\n  Thanks for coming, all, and "
      "come again!'",
      FALSE, andy, 0, 0, TO_ROOM);
    do_close(andy, "door", 0);
    do_lock(andy, "door", 0);
  }

  switch (cmd) {
    case 25:  /* kill */
    case 70:  /* hit */
    case 157: /* bash */
    case 159: /* kick */
      only_argument(arg, argm);

      if (andy == ch)
        return TRUE;
      if (andy == get_char_room(argm, ch->in_room)) {
        int hitsleft;
        act(
          "$n says 'Get this, $N wants to kill me', and\n falls down laughing.",
          FALSE, andy, 0, ch, TO_ROOM);
        hitsleft = dice(2, 6) + 6;
        if (hitsleft < GET_HIT(ch) && GetMaxLevel(ch) <= MAX_MORT) {
          act("$n beats the shit out of $N.", FALSE, andy, 0, ch, TO_NOTVICT);
          act("$n beats the shit out of you.  OUCH!", FALSE, andy, 0, ch,
            TO_VICT);
          GET_HIT(ch) = hitsleft;
        } else {
          act("$n grabs $N in a vicious sleeper hold.", FALSE, andy, 0, ch,
            TO_NOTVICT);
          act("$n puts you in a vicious sleeper hold.", FALSE, andy, 0, ch,
            TO_VICT);
        }
        GET_POS(ch) = POSITION_SLEEPING;
      } else {
        do_action(andy, ch->player.name, 130); /* slap */
        act("$n says 'Hey guys, I run a quiet pub.  Take it outside.'", FALSE,
          andy, 0, 0, TO_ROOM);
      }
      return TRUE;
      break;

    case 156: /* steal */
      if (andy == ch)
        return TRUE;
      do_action(andy, ch->player.name, 130 /* slap */);
      act("$n tells you 'Who the hell do you think you are?'", FALSE, andy, 0,
        ch, TO_VICT);
      do_action(andy, ch->player.name, 116 /* glare */);
      return TRUE;
      break;

    case 84:
    case 207:
    case 172: /* cast, recite, use */
      if (andy == ch)
        return TRUE;
      do_action(andy, ch->player.name, 94 /* poke */);
      act("$n tells you 'Hey, no funny stuff.'.", FALSE, andy, 0, ch, TO_VICT);
      return TRUE;
      break;

    case 56: /* buy */
      if (ch->in_room != THE_PUB) {
        act(
          "$n tells you 'Hey man, I'm on my own time, but stop by the Pub some "
          "time.'",
          FALSE, andy, 0, ch, TO_VICT);
        return TRUE;
      }
      if (open == 0) {
        act("$n tells you 'Sorry, we're closed, come back for lunch.'", FALSE,
          andy, 0, ch, TO_VICT);
        return TRUE;
      }
      only_argument(arg, argm);
      if (!(*argm)) {
        act("$n tells you 'Sure, what do you want to buy?'", FALSE, andy, 0, ch,
          TO_VICT);
        return TRUE;
      }

#if 1
      /* multiple buy code */
      if ((num = getabunch(argm, newarg)) != NULL) {
        strcpy(argm, newarg);
      }
      if (num == 0)
        num = 1;
#endif

      if (!(temp1 = get_obj_in_list_vis(ch, argm, andy->carrying))) {
        act("$n tells you 'Sorry, but I don't sell that.'", FALSE, andy, 0, ch,
          TO_VICT);
        return TRUE;
      }
      for (scan = sold_here; scan->container >= 0; scan++) {
        if (scan->container == obj_index[temp1->item_number].virtual)
          break;
      }
      if (scan->container < 0) {
        act("$n tells you 'Sorry, that's not for sale.'", FALSE, andy, 0, ch,
          TO_VICT);
        return TRUE;
      }

      if (scan->actflag & ACT_OVER_21 && GET_AGE(ch) < 21) {
        if (IS_IMMORTAL(ch)) {
          act("$N manages to slip a fake ID past $n.", FALSE, andy, 0, ch,
            TO_NOTVICT);
        } else if (!IS_NPC(ch)) {
          act(
            "$n tells you 'Sorry, I could lose my license if I served you "
            "alcohol.'",
            FALSE, andy, 0, ch, TO_VICT);
          act("$n cards $N and $N is BUSTED.", FALSE, andy, 0, ch, TO_NOTVICT);
          return TRUE;
        }
      }

      temp2 = read_object(scan->contains, VIRTUAL);
      cost = ((temp2) ? (scan->quant * temp2->obj_flags.cost) : 0) +
             temp1->obj_flags.cost;
      cost *= 9;
      cost /= 10;
      cost++;
      if (temp2)
        extract_obj(temp2);

      for (; num > 0; num--) {
        if (GET_GOLD(ch) < cost) {
          act("$n tells you 'Sorry, man, no bar tabs.'", FALSE, andy, 0, ch,
            TO_VICT);
          return TRUE;
        }
        temp1 = read_object(temp1->item_number, REAL);
        for (i = 0; i < scan->quant; i++) {
          temp2 = read_object(scan->contains, VIRTUAL);
          obj_to_obj(temp2, temp1);
        }
        obj_to_char(temp1, ch);
        GET_GOLD(ch) -= cost;
        act("$N buys a $p from $n", FALSE, andy, temp1, ch, TO_NOTVICT);
        if (scan->actflag & ACT_SNICKER) {
          act("$n snickers softly.", FALSE, andy, NULL, ch, TO_ROOM);
        } else {
          act((scan->actflag & ACT_OVER_21)
                ? "$n tells you 'Drink in good health' and gives you $p"
                : "$n tells you 'Enjoy' and gives you $p",
            FALSE, andy, temp1, ch, TO_VICT);
        }
      }
      return TRUE;
      break;

    case 59: /* list */
      act("$n says 'We have", FALSE, andy, NULL, ch, TO_VICT);
      for (scan = sold_here; scan->container >= 0; scan++) {
        temp1 = read_object(scan->container, VIRTUAL);
        temp2 = (scan->contains) ? read_object(scan->contains, VIRTUAL) : NULL;
        cost = (temp2 ? (scan->quant * temp2->obj_flags.cost) : 0) +
               temp1->obj_flags.cost;
        cost *= 9;
        cost /= 10;
        cost++;
        sprintf(buf, "%s for %d gold coins.\n\r", temp1->short_description,
          cost);
        send_to_char(buf, ch);
        extract_obj(temp1);
        if (temp2)
          extract_obj(temp2);
      }
      return TRUE;
      break;
  }

  return FALSE;
}

int eric_johnson(struct char_data* ch, int cmd, char* arg) {
  /* if more than one eric johnson exists in a game, it will
     get confused because of the state variables */
#define E_HACKING 0
#define E_SLEEPING 1
#define E_SHORT_BEER_RUN 2
#define E_LONG_BEER_RUN 3
#define E_STOCK_FRIDGE 4
#define E_SKYDIVING 5
#define Erics_Lair 3941
#define DanjerKitchen 3904
#define DanjerLiving 3901
#define DanjerPorch 3900
  static int fighting = 0, state = E_HACKING;
  struct obj_data* temp1;
  struct char_data *eric, *temp_char;
  char buf[100];

  eric = 0;

  if (check_soundproof(ch))
    return (FALSE);

  eric = FindMobInRoomWithFunction(ch->in_room, eric_johnson);
  for (temp_char = real_roomp(ch->in_room)->people; (!eric) && (temp_char);
       temp_char = temp_char->next_in_room)
    if (IS_MOB(temp_char))
      if (mob_index[temp_char->nr].func.mob_f == eric_johnson)
        eric = temp_char;

  if (ch == eric) {
    if (cmd != 0)
      return FALSE; /* prevent recursion when eric wants to move */

    if (!fighting && ch->specials.fighting) {
      act("$n says 'What the fuck?'", FALSE, eric, 0, 0, TO_ROOM);
      fighting = 1;
    }
    if (fighting && !ch->specials.fighting) {
      act("$n says 'I wonder what their problem was.'", FALSE, eric, 0, 0,
        TO_ROOM);
      fighting = 0;
    }
    if (fighting) {
      struct char_data* target = eric->specials.fighting;
      act("$n yells for help.", FALSE, eric, 0, 0, TO_ROOM);
      act("$n throws some nasty judo on $N.", FALSE, eric, 0, target,
        TO_NOTVICT);
      act("$n throws some nasty judo on you.", FALSE, eric, 0, target, TO_VICT);
      damage(eric, target, dice(2, 4), TYPE_HIT);
      if (!saves_spell(target, SAVING_SPELL)) {
        struct affected_type af;
        af.type = SPELL_SLEEP;
        af.duration = 2;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_SLEEP;
        affect_join(target, &af, FALSE, FALSE);
        if (target->specials.fighting)
          stop_fighting(target);
        if (eric->specials.fighting)
          stop_fighting(eric);
        act("$N is out cold.", FALSE, eric, 0, target, TO_NOTVICT);
        act("You are out cold.", FALSE, eric, 0, target, TO_VICT);
        GET_POS(target) = POSITION_SLEEPING;
        RemHated(eric, target);
        RemHated(target, eric);
      }
      return FALSE;
    }

    switch (state) {
      case E_HACKING:
        if (GET_POS(eric) == POSITION_SLEEPING) {
          do_wake(eric, "", -1);
          return TRUE;
        }
        break;
      case E_SLEEPING:
        if (GET_POS(eric) != POSITION_SLEEPING) {
          act("$n says 'Go away, I'm sleeping'", FALSE, eric, 0, 0, TO_ROOM);
          do_sleep(eric, "", -1);
          return TRUE;
        }
        break;
      default:
        if (GET_POS(eric) == POSITION_SLEEPING) {
          do_wake(eric, "", -1);
          return TRUE;
        } else if (GET_POS(eric) != POSITION_STANDING) {
          do_stand(eric, "", -1);
          return TRUE;
        }
        break;
    }

    switch (state) {
      char* s;
      case E_SLEEPING:
        if (time_info.hours > 9 && time_info.hours < 12) {
          do_wake(eric, "", -1);
          act("$n says 'Ahh, that was a good night's sleep'", FALSE, eric, 0, 0,
            TO_ROOM);
          state = E_HACKING;
          return TRUE;
        }
        return TRUE;
        break;
      case E_HACKING:
        if (eric->in_room != Erics_Lair) {
          /* he's not in his lair, get him there. */
          int dir;
          if (eric->in_room == DanjerLiving) {
            do_close(eric, "front", 0);
            do_lock(eric, "front", 0);
          }
          dir = choose_exit_global(eric->in_room, Erics_Lair, -100);
          if (dir < 0) {
            if (eric->in_room == DanjerPorch) {
              do_unlock(eric, "front", 0);
              do_open(eric, "front", 0);
              return TRUE;
            }
            dir = choose_exit_global(eric->in_room, DanjerPorch, -100);
          }
          if (dir < 0) {
            if (dice(1, 2) == 1)
              act("$n says 'Shit, I'm totally lost.", FALSE, eric, 0, 0,
                TO_ROOM);
            else
              act("$n says 'Can you show me the way back to the DanjerHaus?'",
                FALSE, eric, 0, 0, TO_ROOM);
          } else
            go_direction(eric, dir);

        } else {
          if (time_info.hours > 22 || time_info.hours < 3) {
            state = E_SLEEPING;
            do_sleep(eric, 0, -1);
            return TRUE;
          }

          do_sit(eric, "", -1);
          if (3 == dice(1, 5)) {
            /* he's in his lair, do lair things */
            switch (dice(1, 5)) {
              case 1:
                s = "$n looks at you, then resumes hacking";
                break;
              case 2:
                s = "$n swears at the terminal and resumes hacking";
                break;
              case 3:
                s = "$n looks around and says 'Where's Big Guy?'";
                break;
              case 4:
                s = "$n says 'Dude, RS/6000s suck.'";
                break;
              case 5:
                temp1 = get_obj_in_list_vis(eric, "beer", eric->carrying);
                if (temp1 == NULL ||
                    temp1->obj_flags.type_flag != ITEM_DRINKCON ||
                    temp1->obj_flags.value[1] <= 0) {
                  s = "$n says 'Damn, out of beer'";
                  do_stand(eric, "", -1);
                  state = E_SHORT_BEER_RUN;
                } else {
                  do_drink(eric, "beer", -1 /* irrelevant */);
                  s = "$n licks his lips";
                }
                break;
            }
            act(s, FALSE, eric, 0, 0, TO_ROOM);
          }
        }
        break;
      case E_SHORT_BEER_RUN:
        if (eric->in_room != DanjerKitchen) {
          int dir;
          dir = choose_exit_global(eric->in_room, DanjerKitchen, -100);
          if (dir < 0) {
            if (dice(1, 3) != 1)
              act("$n says 'Dammit, where's the beer?", FALSE, eric, 0, 0,
                TO_ROOM);
            else
              act("$n says 'Christ, who stole my kitchen?'", FALSE, eric, 0, 0,
                TO_ROOM);
          } else {
            go_direction(eric, dir);
          }
        } else {
          /* we're in the kitchen, find beer */
          temp1 = get_obj_in_list_vis(eric, "fridge",
            real_roomp(eric->in_room)->contents);
          if (temp1 == NULL) {
            act("$n says 'Alright, who stole my refrigerator!'", FALSE, eric, 0,
              0, TO_ROOM);
          } else if (IS_SET(temp1->obj_flags.value[1], CONT_CLOSED)) {
            do_drop(eric, "bottle", -1 /* irrelevant */);
            do_open(eric, "fridge", -1 /* irrelevant */);
          } else if (NULL == (temp1 = get_obj_in_list_vis(eric, "sixpack",
                                eric->carrying))) {
            strcpy(buf, "get sixpack fridge");
            command_interpreter(eric, buf);
            if (NULL == get_obj_in_list_vis(eric, "sixpack", eric->carrying)) {
              act("$n says 'Aw, man.  Someone's been drinking all the beer.",
                FALSE, eric, 0, 0, TO_ROOM);
              do_close(eric, "fridge", -1 /* irrelevant */);
              state = E_LONG_BEER_RUN;
            }
          } else if (NULL == (temp1 = get_obj_in_list_vis(eric, "beer",
                                eric->carrying))) {
            strcpy(buf, "get beer sixpack");
            command_interpreter(eric, buf);
            if (NULL == get_obj_in_list_vis(eric, "beer", eric->carrying)) {
              act("$n says 'Well, that one's finished...'", FALSE, eric, 0, 0,
                TO_ROOM);
              do_drop(eric, "sixpack", -1 /* irrelevant */);
            }
          } else {
            strcpy(buf, "put sixpack fridge");
            command_interpreter(eric, buf);
            do_close(eric, "fridge", -1 /* irrelevant */);
            state = E_HACKING;
          }
        }
        break;
      case E_LONG_BEER_RUN: {
        static struct char_data* andy = 0;
        int dir;
        static char **scan,
          *shopping_list[] = {"guinness", "harp", "sierra", "2.harp", NULL};

        for (temp_char = character_list; temp_char; temp_char = temp_char->next)
          if (IS_MOB(temp_char))
            if (mob_index[temp_char->nr].func.mob_f == andy_wilcox)
              andy = temp_char;

        if (eric->in_room != andy->in_room) {
          if (eric->in_room == DanjerPorch) {
            do_close(eric, "front", 0);
            do_lock(eric, "front", 0); /* this takes no time */
          } else if (eric->in_room == DanjerLiving) {
            do_unlock(eric, "front", 0);
            do_open(eric, "front", 0);
            return TRUE; /* this takes one turn */
          }
          dir = choose_exit_global(eric->in_room, andy->in_room, -100);
          if (dir < 0) {
            dir = choose_exit_global(eric->in_room, DanjerLiving, -100);
          }
          if (dir < 0) {
            act("$n says 'Aw, man.  Where am I going to get more beer?", FALSE,
              eric, 0, 0, TO_ROOM);
            state = E_HACKING;
          } else
            go_direction(eric, dir);
        } else {
          for (scan = shopping_list; *scan; scan++) {
            if (NULL == get_obj_in_list_vis(eric, *scan, eric->carrying)) {
              char* s;
              s = (scan[0][1] == '.') ? scan[0] + 2 : scan[0];
              sprintf(buf, "buy %s", s);
              command_interpreter(eric, buf);
              if (NULL == get_obj_in_list_vis(eric, *scan, eric->carrying)) {
                act(
                  "$n says 'ARGH, where's my deadbeat roommate with the rent.'",
                  FALSE, eric, 0, 0, TO_ROOM);
                act("$n says 'I need beer money.'", FALSE, eric, 0, 0, TO_ROOM);
                state = (scan == shopping_list) ? E_HACKING : E_STOCK_FRIDGE;
                return TRUE;
              }
              break;
            }
          }
          if (*scan == NULL || 1 == dice(1, 4)) {
            act("$n says 'Catch you later, dude.'", FALSE, eric, 0, 0, TO_ROOM);
            state = E_STOCK_FRIDGE;
          }
        }
      } break;
      case E_STOCK_FRIDGE:
        if (eric->in_room != DanjerKitchen) {
          int dir;
          if (eric->in_room == DanjerLiving) {
            do_close(eric, "front", 0);
            do_lock(eric, "front", 0);
          }
          dir = choose_exit_global(eric->in_room, DanjerKitchen, -100);
          if (dir < 0) {
            if (eric->in_room == DanjerPorch) {
              do_unlock(eric, "front", 0);
              do_open(eric, "front", 0);
              return TRUE;
            }
            dir = choose_exit_global(eric->in_room, DanjerPorch, -100);
          }
          if (dir < 0) {
            if (dice(1, 3) != 1)
              act("$n says 'Dammit, where's the fridge?", FALSE, eric, 0, 0,
                TO_ROOM);
            else
              act("$n says 'Christ, who stole my kitchen?'", FALSE, eric, 0, 0,
                TO_ROOM);
          } else {
            go_direction(eric, dir);
          }
        } else {
          /* we're in the kitchen, find beer */
          temp1 = get_obj_in_list_vis(eric, "fridge",
            real_roomp(eric->in_room)->contents);
          if (temp1 == NULL) {
            act("$n says 'Alright, who stole my refrigerator!'", FALSE, eric, 0,
              0, TO_ROOM);
          } else if (IS_SET(temp1->obj_flags.value[1], CONT_CLOSED)) {
            do_open(eric, "fridge", -1 /* irrelevant */);
          } else if (NULL == (temp1 = get_obj_in_list_vis(eric, "beer",
                                eric->carrying))) {
            strcpy(buf, "get beer sixpack");
            command_interpreter(eric, buf);
            if (NULL == get_obj_in_list_vis(eric, "beer", eric->carrying)) {
              act("$n says 'What the hell, I just bought this?!'", FALSE, eric,
                0, 0, TO_ROOM);
              do_drop(eric, "sixpack", -1 /* irrelevant */);
              if (NULL == get_obj_in_list_vis(eric, "sixpack", eric->carrying))
                state = E_HACKING;
            }
          } else {
            strcpy(buf, "put all.sixpack fridge");
            command_interpreter(eric, buf);
            do_close(eric, "fridge", -1 /* irrelevant */);
            state = E_HACKING;
          }
        }
        break;
    }
  }

  return FALSE;
}

/* ********************************************************************
 *  General special procedures for mobiles *
 ******************************************************************** */

/* SOCIAL GENERAL PROCEDURES

If first letter of the command is '!' this will mean that the following
command will be executed immediately.

"G",n      : Sets next line to n
"g",n      : Sets next line relative to n, fx. line+=n
"m<dir>",n : move to <dir>, <dir> is 0,1,2,3,4 or 5
"w",n      : Wake up and set standing (if possible)
"c<txt>",n : Look for a person named <txt> in the room
"o<txt>",n : Look for an object named <txt> in the room
"r<int>",n : Test if the npc in room number <int>?
"s",n      : Go to sleep, return false if can't go sleep
"e<txt>",n : echo <txt> to the room, can use $o/$p/$N depending on
             contents of the **thing
"E<txt>",n : Send <txt> to person pointed to by thing
"B<txt>",n : Send <txt> to room, except to thing
"?<num>",n : <num> in [1..99]. A random chance of <num>% success rate.
             Will as usual advance one line upon sucess, and change
             relative n lines upon failure.
"O<txt>",n : Open <txt> if in sight.
"C<txt>",n : Close <txt> if in sight.
"L<txt>",n : Lock <txt> if in sight.
"U<txt>",n : Unlock <txt> if in sight.    */

/* Execute a social command.                                        */
static void exec_social(struct char_data* npc, char* cmd, int next_line,
  int* cur_line, void** thing) {
  bool ok;

  if (GET_POS(npc) == POSITION_FIGHTING)
    return;

  ok = TRUE;

  switch (*cmd) {
    case 'G':
      *cur_line = next_line;
      return;

    case 'g':
      *cur_line += next_line;
      return;

    case 'e':
      act(cmd + 1, FALSE, npc, *thing, *thing, TO_ROOM);
      break;

    case 'E':
      act(cmd + 1, FALSE, npc, 0, *thing, TO_VICT);
      break;

    case 'B':
      act(cmd + 1, FALSE, npc, 0, *thing, TO_NOTVICT);
      break;

    case 'm':
      do_move(npc, "", *(cmd + 1) - '0' + 1);
      break;

    case 'w':
      if (GET_POS(npc) != POSITION_SLEEPING)
        ok = FALSE;
      else
        GET_POS(npc) = POSITION_STANDING;
      break;

    case 's':
      if (GET_POS(npc) <= POSITION_SLEEPING)
        ok = FALSE;
      else
        GET_POS(npc) = POSITION_SLEEPING;
      break;

    case 'c': /* Find char in room */
      *thing = get_char_room_vis(npc, cmd + 1);
      ok = (*thing != 0);
      break;

    case 'o': /* Find object in room */
      *thing =
        get_obj_in_list_vis(npc, cmd + 1, real_roomp(npc->in_room)->contents);
      ok = (*thing != 0);
      break;

    case 'r': /* Test if in a certain room */
      ok = (npc->in_room == atoi(cmd + 1));
      break;

    case 'O': /* Open something */
      do_open(npc, cmd + 1, 0);
      break;

    case 'C': /* Close something */
      do_close(npc, cmd + 1, 0);
      break;

    case 'L': /* Lock something  */
      do_lock(npc, cmd + 1, 0);
      break;

    case 'U': /* UnLock something  */
      do_unlock(npc, cmd + 1, 0);
      break;

    case '?': /* Test a random number */
      if (atoi(cmd + 1) <= number(1, 100))
        ok = FALSE;
      break;

    default:
      break;
  } /* End Switch */

  if (ok)
    (*cur_line)++;
  else
    (*cur_line) += next_line;
}

static void npc_steal(struct char_data* ch, struct char_data* victim) {
  int gold;

  if (IS_NPC(victim))
    return;
  if (GetMaxLevel(victim) > MAX_MORT)
    return;

  if (AWAKE(victim) && (number(0, GetMaxLevel(ch)) == 0)) {
    act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0,
      victim, TO_VICT);
    act("$n tries to steal gold from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
  } else {
    /* Steal some gold coins */
    gold = (int)((GET_GOLD(victim) * number(1, 10)) / 100);
    if (gold > 0) {
      GET_GOLD(ch) += gold;
      GET_GOLD(victim) -= gold;
    }
  }
}

int sheriff(struct char_data* ch, int cmd, char* arg) {
  void do_shoot(struct char_data * ch, char* arg, int cmd);
  struct obj_data* gun;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buff[MAX_STRING_LENGTH];
  gun = ch->equipment[HOLD];

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (GET_POS(ch) != POSITION_FIGHTING)
    return FALSE;

  if (!gun || gun->obj_flags.type_flag != ITEM_FIREWEAPON)
    return FALSE;

  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {
    if (gun->obj_flags.value[3] >= 1) {
      act("$n shouts '$N is a bloody coward!'", 1, ch, 0, ch->specials.fighting,
        TO_ROOM);
      do_shoot(ch, GET_NAME(ch->specials.fighting), 0);
    } else {
      sprintf(buf, "remove shotgun");
      command_interpreter(ch, buf);
      sprintf(buff, "reload shotgun shells");
      command_interpreter(ch, buff);
      sprintf(buf2, "hold shotgun");
      command_interpreter(ch, buf2);
      return (TRUE);
    }
  }
  return (FALSE);
}

int bow_shooter(struct char_data* ch, int cmd, char* arg) {
  void do_fire(struct char_data * ch, char* arg, int cmd);
  struct obj_data* bow;
  char buf[MAX_STRING_LENGTH];
  char buff[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  bow = ch->equipment[HOLD];

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (GET_POS(ch) != POSITION_FIGHTING)
    return (FALSE);

  if (!bow || bow->obj_flags.type_flag != ITEM_BOW)
    return FALSE;

  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {
    if (bow->obj_flags.value[3] >= 1) {
      act("$n shouts '$N deserves to die!'", 1, ch, 0, ch->specials.fighting,
        TO_ROOM);
      do_fire(ch, GET_NAME(ch->specials.fighting), 0);
    } else {
      sprintf(buf, "remove bow");
      command_interpreter(ch, buf);
      sprintf(buff, "bload bow arrow");
      command_interpreter(ch, buff);
      sprintf(buf2, "hold bow");
      command_interpreter(ch, buf2);
      return (TRUE);
    }
  }
  return (FALSE);
}

int snake(struct char_data* ch, int cmd, char* arg) {
  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (GET_POS(ch) != POSITION_FIGHTING)
    return FALSE;

  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {
    act("$n poisons $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n poisons you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
      ch->specials.fighting, 0);
    return TRUE;
  }
  return FALSE;
}

int ninja_master(struct char_data* ch, int cmd, char* arg) {
  char buf[256];
  static char* n_skills[] = {
    "track",  /* No. 180 */
    "disarm", /* No. 245 */
    "\n",
  };
  int percent = 0, number = 0;
  int charge, sk_num, mult;

  if (!AWAKE(ch))
    return (FALSE);

  if (!ch->skills)
    return (FALSE);

  if (check_soundproof(ch))
    return (FALSE);

  for (; *arg == ' '; arg++)
    ; /* ditch spaces */

  if ((cmd == 164) || (cmd == 170)) {
    /* So far, just track */
    if (!arg || (strlen(arg) == 0)) {
      sprintf(buf, " track:   %s\n\r",
        how_good(ch->skills[SKILL_HUNT].learned));
      send_to_char(buf, ch);
      sprintf(buf, " disarm:  %s\n\r",
        how_good(ch->skills[SKILL_DISARM].learned));
      send_to_char(buf, ch);
      return (TRUE);
    } else {
      number = old_search_block(arg, 0, strlen(arg), n_skills, FALSE);
      send_to_char("The ninja master says ", ch);
      if (number == -1) {
        send_to_char("'I do not know of this skill.'\n\r", ch);
        return (TRUE);
      }
      charge = GetMaxLevel(ch) * 10000;
      switch (number) {
        case 0:
        case 1:
          sk_num = SKILL_HUNT;
          break;
        case 2:
          sk_num = SKILL_DISARM;
          mult = 1;
          if (HasClass(ch, CLASS_MAGIC_USER))
            mult = 4;
          if (HasClass(ch, CLASS_CLERIC))
            mult = 3;
          if (HasClass(ch, CLASS_THIEF))
            mult = 2;
          if (HasClass(ch, CLASS_WARRIOR))
            mult = 1;
          charge *= mult;

          break;
        default:
          sprintf(buf, "Strangeness in ninjamaster (%d)", number);
          vlog(buf);
          return FALSE;
      }
    }

    if (GET_GOLD(ch) < (charge / 2)) {
      send_to_char("'Ah, but you do not have enough money to pay.'\n\r", ch);
      return (FALSE);
    }

    if (ch->skills[sk_num].learned >= 95) {
      send_to_char(
        "'You are a master of this art, I can teach you no more.'\n\r", ch);
      return (FALSE);
    }

    if (ch->specials.spells_to_learn <= 0) {
      send_to_char("'You must first use the knowledge you already have.\n\r",
        ch);
      return (FALSE);
    }

    GET_GOLD(ch) -= (charge / 2);
    send_to_char("'We will now begin.'\n\r", ch);
    ch->specials.spells_to_learn--;

    percent = ch->skills[sk_num].learned + int_app[GET_INT(ch)].learn;
    ch->skills[sk_num].learned = MIN(95, percent);

    if (ch->skills[sk_num].learned >= 95) {
      send_to_char("'You are now a master of this art.'\n\r", ch);
      return (TRUE);
    }
  } else {
    return (FALSE);
  }
}

static int HasObject(struct char_data* ch, int ob_num) {
  int j, found;
  struct obj_data* i;

  /*
     equipment too
  */

  found = 0;

  for (j = 0; j < MAX_WEAR; j++)
    if (ch->equipment[j])
      found += RecCompObjNum(ch->equipment[j], ob_num);

  if (found > 0)
    return (TRUE);

  /* carrying  */
  for (i = ch->carrying; i; i = i->next_content)
    found += RecCompObjNum(i, ob_num);

  if (found > 0)
    return (TRUE);
  else
    return (FALSE);
}

#define PGShield 25100

int PaladinGuildGuard(struct char_data* ch, int cmd, char* arg) {
  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (!cmd) {
    if (ch->specials.fighting) {
      fighter(ch, cmd, arg);
    }
  } else if (cmd >= 1 && cmd <= 6) {
    if (cmd == 4)
      return (FALSE); /* can always go west */
    if (!HasObject(ch, PGShield)) {
      send_to_char("The guard shakes his head, and blocks your way.\n\r", ch);
      act("The guard shakes his head, and blocks $n's way.", TRUE, ch, 0, 0,
        TO_ROOM);
      return (TRUE);
    }
  }
  return (FALSE);
}

int AbyssGateKeeper(struct char_data* ch, int cmd, char* arg) {
  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (!cmd) {
    if (ch->specials.fighting) {
      fighter(ch, cmd, arg);
    }
  } else if ((cmd >= 1 && cmd <= 6) && (!IS_IMMORTAL(ch))) {
    if ((cmd == 6) || (cmd == 1)) {
      send_to_char("The gatekeeper shakes his head, and blocks your way.\n\r",
        ch);
      act("The guard shakes his head, and blocks $n's way.", TRUE, ch, 0, 0,
        TO_ROOM);
      return (TRUE);
    }
  }
  return (FALSE);
}

int blink(struct char_data* ch, int cmd, char* arg) {
  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (GET_HIT(ch) < (int)GET_MAX_HIT(ch) / 3) {
    act("$n blinks.", TRUE, ch, 0, 0, TO_ROOM);
    cast_teleport(12, ch, "", SPELL_TYPE_SPELL, ch, 0);
    return (TRUE);
  } else {
    return (FALSE);
  }
}

int MidgaardCitizen(struct char_data* ch, int cmd, char* arg) {
  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    fighter(ch, cmd, arg);

    if (check_soundproof(ch))
      return (FALSE);

    if (number(0, 18) == 0) {
      do_shout(ch, "Guards! Help me! Please!", 0);
    } else {
      act("$n shouts 'Guards!  Help me! Please!'", TRUE, ch, 0, 0, TO_ROOM);
    }

    if (ch->specials.fighting)
      CallForGuard(ch, ch->specials.fighting, 3, MIDGAARD);

    return (TRUE);

  } else {
    return (FALSE);
  }
}

int ghoul(struct char_data* ch, int cmd, char* arg) {
  struct char_data* tar;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  tar = ch->specials.fighting;

  if (tar && (tar->in_room == ch->in_room)) {
    if ((!IS_AFFECTED(tar, AFF_PROTECT_EVIL)) &&
        (!IS_AFFECTED(tar, AFF_SANCTUARY))) {
      act("$n touches $N!", 1, ch, 0, tar, TO_NOTVICT);
      act("$n touches you!", 1, ch, 0, tar, TO_VICT);
      if (!IS_AFFECTED(tar, AFF_PARALYSIS)) {
        cast_paralyze(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, tar, 0);
        return TRUE;
      }
    }
  }
  return FALSE;
}

int CarrionCrawler(struct char_data* ch, int cmd, char* arg) {
  struct char_data* tar;
  int i;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = 0; i < 1; i++) {
    if ((tar = FindAHatee(ch)) == NULL)
      tar = FindVictim(ch);

    if (tar && (tar->in_room == ch->in_room)) {
      act("$n touches $N!", 1, ch, 0, tar, TO_NOTVICT);
      act("$n touches you!", 1, ch, 0, tar, TO_VICT);
      if (!IS_AFFECTED(tar, AFF_PARALYSIS)) {
        cast_paralyze(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, tar, 0);
        return TRUE;
      }
    }
  }
  return FALSE;
}

int WizardGuard(struct char_data* ch, int cmd, char* arg) {
  struct char_data *tch, *evil;
  int max_evil;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    fighter(ch, cmd, arg);
    CallForGuard(ch, ch->specials.fighting, 9, MIDGAARD);
  }
  max_evil = 1000;
  evil = 0;

  for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if (tch->specials.fighting) {
      if ((GET_ALIGNMENT(tch) < max_evil) &&
          (IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
        max_evil = GET_ALIGNMENT(tch);
        evil = tch;
      }
    }
  }

  if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0) &&
      !check_peaceful(ch, "")) {
    if (!check_soundproof(ch)) {
      act("$n screams 'DEATH!!!!!!!!'", FALSE, ch, 0, 0, TO_ROOM);
    }
    hit(ch, evil, TYPE_UNDEFINED);
    return (TRUE);
  }
  return (FALSE);
}

int vampire(struct char_data* ch, int cmd, char* arg) {
  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {
    act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
      ch->specials.fighting, 0);
    if (ch->specials.fighting &&
        (ch->specials.fighting->in_room == ch->in_room)) {
      cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
        ch->specials.fighting, 0);
    }
    return TRUE;
  }
  return FALSE;
}

int arch_vampire(struct char_data* ch, int cmd, char* arg) {
  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {
    act("$n bites $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n bites you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
      ch->specials.fighting, 0);
    if (ch->specials.fighting &&
        (ch->specials.fighting->in_room == ch->in_room)) {
      cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
        ch->specials.fighting, 0);
    }
    return TRUE;
  }
  return FALSE;
}

int wraith(struct char_data* ch, int cmd, char* arg) {
  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {
    act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
      ch->specials.fighting, 0);
    return TRUE;
  }
  return FALSE;
}

int shadow(struct char_data* ch, int cmd, char* arg) {
  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {
    act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_chill_touch(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
      ch->specials.fighting, 0);
    if (ch->specials.fighting)
      cast_weakness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
        ch->specials.fighting, 0);
    return TRUE;
  }
  return FALSE;
}

int geyser(struct char_data* ch, int cmd, char* arg) {
  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (number(0, 3) == 0) {
    act("You erupt.", 1, ch, 0, 0, TO_CHAR);
    cast_geyser(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, 0, 0);
    return (TRUE);
  }
}

int green_slime(struct char_data* ch, int cmd, char* arg) {
  struct char_data* cons;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (cons = real_roomp(ch->in_room)->people; cons; cons = cons->next_in_room)
    if ((!IS_NPC(cons)) && (GetMaxLevel(cons) < LOW_IMMORTAL))
      cast_green_slime(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, cons, 0);
}

struct breath_victim {
    struct char_data* ch;
    int yesno; /* 1 0 */
    struct breath_victim* next;
};

struct breath_victim* choose_victims(struct char_data* ch,
  struct char_data* first_victim) {
  /* this is goofy, dopey extraordinaire */
  struct char_data* cons;
  struct breath_victim *head = NULL, *temp = NULL;

  for (cons = real_roomp(ch->in_room)->people; cons;
       cons = cons->next_in_room) {
    temp = (void*)malloc(sizeof(*temp));
    temp->ch = cons;
    temp->next = head;
    head = temp;
    if (first_victim == cons) {
      temp->yesno = 1;
    } else if (ch == cons) {
      temp->yesno = 0;
    } else if ((in_group(first_victim, cons) || cons == first_victim->master ||
                 cons->master == first_victim) &&
               (temp->yesno = (3 != dice(1, 5)))) {
      /* group members will get hit 4/5 times */
    } else if (cons->specials.fighting == ch) {
      /* people fighting the dragon get hit 4/5 times */
      temp->yesno = (3 != dice(1, 5));
    } else /* bystanders get his 2/5 times */
      temp->yesno = (dice(1, 5) < 3);
  }
  return head;
}

void free_victims(struct breath_victim* head) {
  struct breath_victim* temp;

  while (head) {
    temp = head->next;
    free(head);
    head = temp;
  }
}

int breath_weapon(struct char_data* ch, struct char_data* target, int mana_cost,
  funcp func) {
  struct breath_victim *hitlist, *scan;
  int victim;

  hitlist = choose_victims(ch, target);

  act("$n rears back and inhales", 1, ch, 0, ch->specials.fighting, TO_ROOM);
  victim = 0;
  for (scan = hitlist; scan; scan = scan->next) {
    if (!scan->yesno || IS_IMMORTAL(scan->ch) ||
        scan->ch->in_room != ch->in_room /* this should not happen */
    )
      continue;
    victim = 1;
    cast_fear(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, scan->ch, 0);
  }

  if (func != NULL && victim) {
    act("$n Breathes...", 1, ch, 0, ch->specials.fighting, TO_ROOM);

    for (scan = hitlist; scan; scan = scan->next) {
      if (!scan->yesno || IS_IMMORTAL(scan->ch) ||
          scan->ch->in_room != ch->in_room /* this could happen if
                      someone fled, I guess */
      )
        continue;
      func(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, scan->ch, 0);
    }
  } else {
    act("$n Breathes...coughs and sputters...", 1, ch, 0, ch->specials.fighting,
      TO_ROOM);
    do_flee(ch, "", 0);
  }

  free_victims(hitlist);
}

int use_breath_weapon(struct char_data* ch, struct char_data* target, int cost,
  funcp func) {
  if (GET_MANA(ch) >= 0) {
    breath_weapon(ch, target, cost, func);
  } else if (GET_HIT(ch) < GET_MAX_HIT(ch) / 2) {
    breath_weapon(ch, target, cost, func);
  } else if (GET_HIT(ch) < GET_MAX_HIT(ch) / 4) {
    breath_weapon(ch, target, cost, func);
  }
}

static funcp breaths[] = {cast_acid_breath, 0, cast_frost_breath, 0,
  cast_lightning_breath, 0, cast_fire_breath, 0, cast_acid_breath,
  cast_fire_breath, cast_lightning_breath, 0};

const struct breather breath_monsters[] = {
  {230, 55, breaths + 0},
  {233, 55, breaths + 0},
  {243, 55, breaths + 0},
  {3670, 30, breaths + 0},
  {3674, 45, breaths + 0},
  {3675, 45, breaths + 0},
  {3676, 30, breaths + 0},
  {3952, 20, breaths + 8},
  {5005, 55, breaths + 4},
  {6112, 55, breaths + 2},
  {6801, 55, breaths + 0},
  {6802, 55, breaths + 0},
  {6824, 55, breaths + 0},
  {7040, 55, breaths + 6},
  {9217, 45, breaths + 4},
  {15858, 45, breaths + 0},
  {16620, 45, breaths + 2},
  {16700, 45, breaths + 4},
  {16738, 75, breaths + 0},
  {18003, 20, breaths + 8},
  {20002, 55, breaths + 0},
  {20017, 55, breaths + 0},
  {20016, 55, breaths + 0},
  {25009, 30, breaths + 6},
  {25504, 30, breaths + 4},
  {27016, 30, breaths + 6},
  {28449, 30, breaths + 6},
  {29954, 45, breaths + 2},
  {-1},
};

int BreathWeapon(struct char_data* ch, int cmd, char* arg) {
  char buf[MAX_STRING_LENGTH];
  struct breather* scan;
  int count;

  if (cmd)
    return FALSE;

  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {
    for (scan = breath_monsters;
         scan->vnum >= 0 && scan->vnum != mob_index[ch->nr].virtual; scan++)
      ;

    if (scan->vnum < 0) {
      sprintf(buf, "monster %s tries to breath, but isn't listed.",
        ch->player.short_descr);
      vlog(buf);
      return FALSE;
    }

    for (count = 0; scan->breaths[count]; count++)
      ;

    if (count < 1) {
      sprintf(buf, "monster %s has no breath weapons", ch->player.short_descr);
      vlog(buf);
      return FALSE;
    }

    use_breath_weapon(ch, ch->specials.fighting, scan->cost,
      scan->breaths[dice(1, count) - 1]);
  }

  return TRUE;
}

int DracoLich(struct char_data* ch, int cmd, char* arg) {}

int Drow(struct char_data* ch, int cmd, char* arg) {}

int Leader(struct char_data* ch, int cmd, char* arg) {}

int thief(struct char_data* ch, int cmd, char* arg) {
  struct char_data* cons;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (GET_POS(ch) != POSITION_STANDING)
    return FALSE;

  for (cons = real_roomp(ch->in_room)->people; cons; cons = cons->next_in_room)
    if ((!IS_NPC(cons)) && (GetMaxLevel(cons) < LOW_IMMORTAL) &&
        (number(1, 5) == 1))
      npc_steal(ch, cons);

  return TRUE;
}

int Astraller(struct char_data* ch, int cmd, char* arg) {
  struct descriptor_data* d;
  struct char_data* targ = 0;
  struct char_list* i;
  char buf[200];

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (check_soundproof(ch))
    return (FALSE);

  if (ch->specials.fighting)
    return (FALSE);

  if (IS_SET(ch->hatefield, HATE_CHAR)) {
    if (ch->hates.clist) {
      for (i = ch->hates.clist; i; i = i->next) {
        if (i->op_ch) {
          targ = i->op_ch;
          break;
        } else {
          for (d = descriptor_list; d; d = d->next) {
            if (d->character && strcmp(GET_NAME(d->character), i->name) == 0) {
              targ = d->character;
              break;
            }
          }
        }
      }
    }
  }

  if (targ) {
    spell_astral_walk(GetMaxLevel(ch), ch, targ, 0);
    sprintf(buf, "Nightcrawler says 'AHA %s, I have FOUND you!'\n\r",
      GET_NAME(targ));
    if (targ->in_room == ch->in_room) {
      send_to_char(buf, targ);
      hit(ch, targ, 0);
    }
    return (FALSE);
  } else {
    return (FALSE);
  }
}

int Summoner(struct char_data* ch, int cmd, char* arg) {
  struct descriptor_data* d;
  struct char_data* targ = 0;
  struct char_list* i;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (check_soundproof(ch))
    return (FALSE);

  if (ch->specials.fighting)
    return (FALSE);

  /*
  **  wait till at 75% of hitpoints.
  */

  if (GET_HIT(ch) > ((GET_MAX_HIT(ch) * 3) / 4)) {
    /*
    **  check for hatreds
    */
    if (IS_SET(ch->hatefield, HATE_CHAR)) {
      if (ch->hates.clist) {
        for (i = ch->hates.clist; i; i = i->next) {
          if (i->op_ch) { /* if there is a char_ptr */
            targ = i->op_ch;
            break;
          } else { /* look up the char_ptr */
            for (d = descriptor_list; d; d = d->next) {
              if ((d->connected == CON_PLYNG) && (d->character) &&
                  (strcmp(GET_NAME(d->character), i->name) == 0)) {
                targ = d->character;
                break;
              }
            }
          }
        }
      }
    }
    if (targ) {
      act("$n utters the words 'Your ass is mine!'.", 1, ch, 0, 0, TO_ROOM);
      spell_summon(GetMaxLevel(ch), ch, targ, 0);
      if (targ->in_room == ch->in_room) {
        hit(ch, targ, 0);
      }
      return (FALSE);
    } else {
      return (FALSE);
    }
  } else {
    return (FALSE);
  }
}

int nightcrawler(struct char_data* ch, int cmd, char* arg) {
  struct char_data* vict;
  char buf[200];

  if (cmd || !AWAKE(ch) || IS_AFFECTED(ch, AFF_PARALYSIS))
    return (FALSE);

  if (!ch->specials.fighting) {
    if (!ch->desc) {
      return (Astraller(ch, cmd, arg));
    }
  }

  if ((GET_HIT(ch) < (GET_MAX_HIT(ch) / 4)) &&
      (!IS_SET(ch->specials.act, ACT_AGGRESSIVE))) {
    act("$n utters the words 'BIFF. BAMM. BOOM.'", 1, ch, 0, 0, TO_ROOM);
    cast_teleport(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
    return (FALSE);
  }
}

static void StandUp(struct char_data* ch) {
  if ((GET_POS(ch) < POSITION_STANDING) && (GET_POS(ch) > POSITION_STUNNED)) {
    if (ch->points.hit > (ch->points.max_hit / 2))
      act("$n quickly stands up.", 1, ch, 0, 0, TO_ROOM);
    else if (ch->points.hit > (ch->points.max_hit / 6))
      act("$n slowly stands up.", 1, ch, 0, 0, TO_ROOM);
    else
      act("$n gets to $s feet very slowly.", 1, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_STANDING;
  }
}

static int check_nomagic(struct char_data* ch, char* msg_ch, char* msg_rm) {
  struct room_data* rp;

  rp = real_roomp(ch->in_room);
  if (rp && rp->room_flags & NO_MAGIC) {
    if (msg_ch)
      send_to_char(msg_ch, ch);
    if (msg_rm)
      act(msg_rm, FALSE, ch, 0, 0, TO_ROOM);
    return 1;
  }
  return 0;
}

static int NumCharmedFollowersInRoom(struct char_data* ch) {
  struct char_data* t;
  long count = 0;
  struct room_data* rp;

  rp = real_roomp(ch->in_room);
  if (rp) {
    count = 0;
    for (t = rp->people; t; t = t->next_in_room) {
      if (IS_AFFECTED(t, AFF_CHARM) && (t->master == ch))
        count++;
    }
    return (count);
  } else
    return (0);

  return (0);
}

static struct char_data* FindMobDiffZoneSameRace(struct char_data* ch) {
  int num;
  struct char_data* t;
  struct room_data *rp1, *rp2;

  num = number(1, 100);

  for (t = character_list; t; t = t->next, num--) {
    if (GET_RACE(t) == GET_RACE(ch) && IS_NPC(t) && !IS_PC(t) && num == 0) {
      rp1 = real_roomp(ch->in_room);
      rp2 = real_roomp(t->in_room);
      if (rp1->zone != rp2->zone)
        return (t);
    }
  }
  return (0);
}

int magic_user(struct char_data* ch, int cmd, char* arg) {
  struct char_data* vict;
  struct room_data* rp;
  byte lspell;
  char buf[200];

  if (cmd || !AWAKE(ch) || IS_AFFECTED(ch, AFF_PARALYSIS))
    return (FALSE);

  if (!ch->specials.fighting && !IS_PC(ch)) {
    SET_BIT(ch->player.class, CLASS_MAGIC_USER);
    if (GetMaxLevel(ch) < 25)
      return FALSE;
    else {
      if (!ch->desc) {
        if (Summoner(ch, cmd, arg))
          return (TRUE);
        else {
          if (NumCharmedFollowersInRoom(ch) < 5 &&
              IS_SET(ch->hatefield, HATE_CHAR)) {
            act("$n utters the words 'Here boy!'.", 1, ch, 0, 0, TO_ROOM);
            cast_mon_sum7(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            do_order(ch, "followers guard on", 0);
            return (TRUE);
          }
        }
      }
      return FALSE;
    }
  }

  if (!ch->specials.fighting)
    return FALSE;

  if (!IS_PC(ch)) {
    if ((GET_POS(ch) > POSITION_STUNNED) && (GET_POS(ch) < POSITION_FIGHTING)) {
      if (GET_HIT(ch) > GET_HIT(ch->specials.fighting) / 2)
        StandUp(ch);
      else {
        StandUp(ch);
        do_flee(ch, "\0", 0);
      }

      return (TRUE);
    }
  }

  if (check_soundproof(ch))
    return (FALSE);

  if (check_nomagic(ch, 0, 0))
    return (FALSE);

  /* Find a dude to to evil things upon ! */

  vict = FindVictim(ch);

  if (!vict)
    vict = ch->specials.fighting;

  if (!vict)
    return (FALSE);

  lspell = number(0, GetMaxLevel(ch)); /* gen number from 0 to level */
  if (!IS_PC(ch)) {
    lspell += GetMaxLevel(ch) / 5; /* weight it towards the upper levels of
                                      the mage's range */
  }
  lspell = MIN(GetMaxLevel(ch), lspell);

  /*
  **  check your own problems:
  */

  if (lspell < 1)
    lspell = 1;

  if (IS_AFFECTED(ch, AFF_BLIND) && (lspell > 15)) {
    act("$n utters the words 'Let me see the light!'.", TRUE, ch, 0, 0,
      TO_ROOM);
    cast_cure_blind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
    return TRUE;
  }

  if (IS_AFFECTED(ch, AFF_BLIND))
    return (FALSE);

  if ((IS_AFFECTED(vict, AFF_SANCTUARY)) && (lspell > 10) &&
      (GetMaxLevel(ch) > (GetMaxLevel(vict)))) {
    act("$n utters the words 'Use MagicAway Instant Magic Remover'.", 1, ch, 0,
      0, TO_ROOM);
    cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    return (FALSE);
  }

  if ((IS_AFFECTED(vict, AFF_FIRESHIELD)) && (lspell > 10) &&
      (GetMaxLevel(ch) > (GetMaxLevel(vict)))) {
    act("$n utters the words 'Use MagicAway Instant Magic Remover'.", 1, ch, 0,
      0, TO_ROOM);
    cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    return (FALSE);
  }

  if (!IS_PC(ch)) {
    if ((GET_HIT(ch) < (GET_MAX_HIT(ch) / 4)) && (lspell > 28) &&
        !IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
      act("$n checks $s watch.", TRUE, ch, 0, 0, TO_ROOM);
      act("$n utters the words 'Oh my, would you just LOOK at the time!'", 1,
        ch, 0, 0, TO_ROOM);

      vict = FindMobDiffZoneSameRace(ch);
      if (vict) {
        cast_astral_walk(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        return (TRUE);
      }
      cast_teleport(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return (FALSE);
    }
  }

  if (!IS_PC(ch)) {
    if ((GET_HIT(ch) < (GET_MAX_HIT(ch) / 4)) && (lspell > 15) &&
        (!IS_SET(ch->specials.act, ACT_AGGRESSIVE))) {
      act("$n utters the words 'Woah! I'm outta here!'", 1, ch, 0, 0, TO_ROOM);
      cast_teleport(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return (FALSE);
    }
  }

  if (GET_HIT(ch) > (GET_MAX_HIT(ch) / 2) &&
      !IS_SET(ch->specials.act, ACT_AGGRESSIVE) &&
      GetMaxLevel(vict) < GetMaxLevel(ch) && (number(0, 1))) {
    /*
     **  Non-damaging case:
     */

    if (((lspell > 8) && (lspell < 50)) && (number(0, 6) == 0)) {
      act("$n utters the words 'Icky Sticky!'.", 1, ch, 0, 0, TO_ROOM);
      cast_web(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return TRUE;
    }

    if (((lspell > 5) && (lspell < 10)) && (number(0, 6) == 0)) {
      act("$n utters the words 'You wimp'.", 1, ch, 0, 0, TO_ROOM);
      cast_weakness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return TRUE;
    }

    if (((lspell > 5) && (lspell < 10)) && (number(0, 7) == 0)) {
      act("$n utters the words 'Bippety boppity Boom'.", 1, ch, 0, 0, TO_ROOM);
      cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return TRUE;
    }

    if (((lspell > 12) && (lspell < 20)) && (number(0, 7) == 0)) {
      act("$n utters the words '&#%^^@%*#'.", 1, ch, 0, 0, TO_ROOM);
      cast_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return TRUE;
    }

    if (((lspell > 10) && (lspell < 20)) && (number(0, 5) == 0)) {
      act("$n utters the words 'yabba dabba do'.", 1, ch, 0, 0, TO_ROOM);
      cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return TRUE;
    }

    if (((lspell > 8) && (lspell < 40)) && (number(0, 5) == 0) &&
        (vict->specials.fighting != ch)) {
      act("$n utters the words 'You are getting sleepy'.", 1, ch, 0, 0,
        TO_ROOM);
      cast_charm_monster(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      if (IS_AFFECTED(vict, AFF_CHARM)) {
        char buf[200];

        if (!vict->specials.fighting) {
          sprintf(buf, "%s kill %s", GET_NAME(vict),
            GET_NAME(ch->specials.fighting));
          do_order(ch, buf, 0);
        } else {
          sprintf(buf, "%s remove all", GET_NAME(vict));
          do_order(ch, buf, 0);
        }
      }
    }

    /*
    **  The really nifty case:
    */
    switch (lspell) {
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
        act("$n utters the words 'Here boy!'.", 1, ch, 0, 0, TO_ROOM);
        cast_mon_sum1(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        do_order(ch, "followers guard on", 0);
        return (TRUE);
        break;
      case 11:
      case 12:
      case 13:
        act("$n utters the words 'Here boy!'.", 1, ch, 0, 0, TO_ROOM);
        cast_mon_sum2(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        do_order(ch, "followers guard on", 0);
        return (TRUE);
        break;
      case 14:
      case 15:
        act("$n utters the words 'Here boy!'.", 1, ch, 0, 0, TO_ROOM);
        cast_mon_sum3(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        do_order(ch, "followers guard on", 0);
        return (TRUE);
        break;
      case 16:
      case 17:
      case 18:
        act("$n utters the words 'Here boy!'.", 1, ch, 0, 0, TO_ROOM);
        cast_mon_sum4(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        do_order(ch, "followers guard on", 0);
        return (TRUE);
        break;
      case 19:
      case 20:
      case 21:
      case 22:
        act("$n utters the words 'Here boy!'.", 1, ch, 0, 0, TO_ROOM);
        cast_mon_sum5(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        do_order(ch, "followers guard on", 0);
        return (TRUE);
        break;
      case 23:
      case 24:
      case 25:
        act("$n utters the words 'Here boy!'.", 1, ch, 0, 0, TO_ROOM);
        cast_mon_sum6(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        do_order(ch, "followers guard on", 0);
        return (TRUE);
        break;
      case 26:
      default:
        act("$n utters the words 'Here boy!'.", 1, ch, 0, 0, TO_ROOM);
        cast_mon_sum7(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        do_order(ch, "followers guard on", 0);
        return (TRUE);
        break;
    }

  } else {
    /*
     */

    switch (lspell) {
      case 1:
      case 2:
        act("$n utters the words 'bang! bang! pow!'.", 1, ch, 0, 0, TO_ROOM);
        cast_magic_missile(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
      case 3:
      case 4:
      case 5:
        act("$n utters the words 'ZZZZzzzzzzTTTT'.", 1, ch, 0, 0, TO_ROOM);
        cast_shocking_grasp(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
      case 6:
      case 7:
      case 8:
        if (ch->attackers <= 2) {
          act("$n utters the words 'Icky Sticky!'.", 1, ch, 0, 0, TO_ROOM);
          cast_web(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
          break;
        } else {
          act("$n utters the words 'Fwoosh!'.", 1, ch, 0, 0, TO_ROOM);
          cast_burning_hands(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict,
            0);
          break;
        }
      case 9:
      case 10:
        act("$n utters the words 'SPOOGE!'.", 1, ch, 0, 0, TO_ROOM);
        cast_acid_blast(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
      case 11:
      case 12:
      case 13:
        if (ch->attackers <= 2) {
          act("$n utters the words 'KAZAP!'.", 1, ch, 0, 0, TO_ROOM);
          cast_lightning_bolt(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict,
            0);
          break;
        } else {
          act("$n utters the words 'Ice Ice Baby!'.", 1, ch, 0, 0, TO_ROOM);
          cast_ice_storm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
          break;
        }
      case 14:
      case 15:
        act("$n utters the words 'Ciao!'.", 1, ch, 0, 0, TO_ROOM);
        cast_teleport(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
      case 16:
      case 17:
      case 18:
        if (ch->attackers <= 2) {
          act("$n utters the words 'Look! A rainbow!'.", 1, ch, 0, 0, TO_ROOM);
          cast_colour_spray(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
          break;
        } else {
          act("$n utters the words 'Get the sensation!'.", 1, ch, 0, 0,
            TO_ROOM);
          cast_cone_of_cold(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
          break;
        }
      case 19:
      case 20:
      case 21:
      case 22:
      case 23:
      case 24:
        act("$n utters the words 'Hasta la vista, Baby'.", 1, ch, 0, 0,
          TO_ROOM);
        cast_fireball(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
      case 26:
      case 27:
      case 28:
      case 29:
        if (IS_EVIL(ch)) {
          act("$n utters the words 'slllrrrrrrpppp'.", 1, ch, 0, 0, TO_ROOM);
          cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
          break;
        }
      default:
        if (ch->attackers <= 2) {
          act("$n utters the words 'Jankity Jank'.", 1, ch, 0, 0, TO_ROOM);
          cast_disintegrate(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
          break;
        } else {
          act("$n utters the words 'Hasta la vista, Baby'.", 1, ch, 0, 0,
            TO_ROOM);
          cast_fireball(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
          break;
        }
    }
  }
  return TRUE;
}

int cleric(struct char_data* ch, int cmd, char* arg) {
  struct char_data* vict;
  byte lspell, healperc = 0;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (GET_POS(ch) != POSITION_FIGHTING) {
    if ((GET_POS(ch) < POSITION_STANDING) && (GET_POS(ch) > POSITION_STUNNED)) {
      StandUp(ch);
    }
    return FALSE;
  }

  if (check_soundproof(ch))
    return (FALSE);

  if (!ch->specials.fighting)
    return FALSE;

  /* Find a dude to to evil things upon ! */

  if ((vict = FindAHatee(ch)) == NULL)
    vict = FindVictim(ch);

  if (!vict)
    vict = ch->specials.fighting;

  if (!vict)
    return (FALSE);

  /*
    gen number from 0 to level
    */

  lspell = number(0, GetMaxLevel(ch));

  if (lspell < 1)
    lspell = 1;

  /*
    first -- hit a foe, or help yourself?
    */

  if (ch->points.hit < (ch->points.max_hit / 2))
    healperc = 7;
  else if (ch->points.hit < (ch->points.max_hit / 4))
    healperc = 5;
  else if (ch->points.hit < (ch->points.max_hit / 8))
    healperc = 3;

  if (number(1, healperc + 2) > 3) {
    /* do harm */

    /* call lightning */
    if (OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING) && (lspell >= 15) &&
        (number(0, 5) == 0)) {
      act("$n whistles.", 1, ch, 0, 0, TO_ROOM);
      act("$n utters the words 'Here Lightning!'.", 1, ch, 0, 0, TO_ROOM);
      cast_call_lightning(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return (TRUE);
    }

    switch (lspell) {
      case 1:
      case 2:
      case 3:
        act("$n utters the words 'Moo ha ha!'.", 1, ch, 0, 0, TO_ROOM);
        cast_cause_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
      case 4:
      case 5:
      case 6:
        act("$n utters the words 'Hocus Pocus!'.", 1, ch, 0, 0, TO_ROOM);
        cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
      case 7:
        act("$n utters the words 'Va-Voom!'.", 1, ch, 0, 0, TO_ROOM);
        cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
      case 8:
        act("$n utters the words 'Urgle Blurg'.", 1, ch, 0, 0, TO_ROOM);
        cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
      case 9:
      case 10:
        act("$n utters the words 'Take That!'.", 1, ch, 0, 0, TO_ROOM);
        cast_cause_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
      case 11:
        act("$n utters the words 'Burn Baby Burn'.", 1, ch, 0, 0, TO_ROOM);
        cast_flamestrike(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
      case 13:
      case 14:
      case 15:
      case 16: {
        if (!IS_SET(vict->M_immune, IMM_FIRE)) {
          act("$n utters the words 'Burn Baby Burn'.", 1, ch, 0, 0, TO_ROOM);
          cast_flamestrike(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        } else if (IS_AFFECTED(vict, AFF_SANCTUARY) &&
                   (GetMaxLevel(ch) > GetMaxLevel(vict))) {
          act("$n utters the words 'Va-Voom!'.", 1, ch, 0, 0, TO_ROOM);
          cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        } else {
          act("$n utters the words 'Take That!'.", 1, ch, 0, 0, TO_ROOM);
          cast_cause_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        }
        break;
      }
      case 17:
      case 18:
      case 19:
      default:
        act("$n utters the words 'Hurts, doesn't it??'.", 1, ch, 0, 0, TO_ROOM);
        cast_harm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    }

    return (TRUE);

  } else {
    /* do heal */

    if (IS_AFFECTED(ch, AFF_BLIND) && (lspell >= 4) & (number(0, 3) == 0)) {
      act("$n utters the words 'Praise <Deity Name>, I can SEE!'.", 1, ch, 0, 0,
        TO_ROOM);
      cast_cure_blind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return (TRUE);
    }

    if (IS_AFFECTED(ch, AFF_CURSE) && (lspell >= 6) && (number(0, 6) == 0)) {
      act("$n utters the words 'I'm rubber, you're glue.", 1, ch, 0, 0,
        TO_ROOM);
      cast_remove_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return (TRUE);
    }

    if (IS_AFFECTED(ch, AFF_POISON) && (lspell >= 5) && (number(0, 6) == 0)) {
      act(
        "$n utters the words 'Praise <Deity Name> I don't feel sick no more!'.",
        1, ch, 0, 0, TO_ROOM);
      cast_remove_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return (TRUE);
    }

    switch (lspell) {
      case 1:
      case 2:
      case 3:
      case 4:
        act("$n utters the words 'Abrazak'.", 1, ch, 0, 0, TO_ROOM);
        cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
      case 5:
      case 6:
      case 7:
      case 8:
        act("$n utters the words 'I feel good!'.", 1, ch, 0, 0, TO_ROOM);
        cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
      case 14:
      case 15:
      case 16:
        act("$n utters the words 'Woah! I feel GOOD! Heh.'.", 1, ch, 0, 0,
          TO_ROOM);
        cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
      case 17:
      case 18: /* heal */
        act("$n utters the words 'What a Rush!'.", 1, ch, 0, 0, TO_ROOM);
        cast_full_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
      default:
        act("$n utters the words 'Oooh, pretty!'.", 1, ch, 0, 0, TO_ROOM);
        cast_sanctuary(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    }

    return (TRUE);
  }
}

/* ********************************************************************
 *  Special procedures for mobiles                                      *
 ******************************************************************** */

static int CheckForBlockedMove(struct char_data* ch, int cmd, char* arg,
  int room, int dir, int class) {
  char buf[256], buf2[256];

  if (cmd > 6 || cmd < 1)
    return (FALSE);

  strcpy(buf, "The guard humiliates you, and block your way.\n\r");
  strcpy(buf2, "The guard humiliates $n, and blocks $s way.");

  if ((IS_NPC(ch)) || (GetMaxLevel(ch) >= DEMIGOD) ||
      (IS_AFFECTED(ch, AFF_SNEAK)))
    return (FALSE);

  if ((ch->in_room == room) && (cmd == dir + 1)) {
    if (!HasClass(ch, class)) {
      act(buf2, FALSE, ch, 0, 0, TO_ROOM);
      send_to_char(buf, ch);
      return TRUE;
    }
  }
  return FALSE;
}

int guild_guard(struct char_data* ch, int cmd, char* arg) {
  if (!cmd) {
    if (ch->specials.fighting) {
      return (fighter(ch, cmd, arg));
    }
  } else {
    switch (ch->in_room) {
      case 3017:
        return (CheckForBlockedMove(ch, cmd, arg, 3017, 2, CLASS_MAGIC_USER));
        break;
      case 3004:
        return (CheckForBlockedMove(ch, cmd, arg, 3004, 0, CLASS_CLERIC));
        break;
      case 3027:
        return (CheckForBlockedMove(ch, cmd, arg, 3027, 1, CLASS_THIEF));
        break;
      case 3021:
        return (CheckForBlockedMove(ch, cmd, arg, 3021, 1, CLASS_WARRIOR));
        break;
      case 29901:
        return (CheckForBlockedMove(ch, cmd, arg, 29901, 5, CLASS_ANTIPALADIN));
        break;
      case 29904:
        return (CheckForBlockedMove(ch, cmd, arg, 29904, 2, CLASS_PALADIN));
        break;
      case 29910:
        return (CheckForBlockedMove(ch, cmd, arg, 29910, 0, CLASS_RANGER));
        break;
    }
  }

  return FALSE;
}

int Inquisitor(struct char_data* ch, int cmd, char* arg) {
  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    return (fighter(ch, cmd, arg));
  }
}

int puff(struct char_data* ch, int cmd, char* arg) {
  struct char_data *i, *tmp, *tmp_ch;
  char buf[80];

  void do_emote(struct char_data * ch, char* argument, int cmd);
  void do_shout(struct char_data * ch, char* argument, int cmd);

  if (cmd)
    return (0);

  if (check_soundproof(ch))
    return (FALSE);

  switch (number(0, 250)) {
    case 0:
      sprintf(buf, "Pass the bong, dude\n");
      do_say(ch, buf, 0);
      return (1);
    case 1:
      do_say(ch, "How'd all those fish get up here?", 0);
      return (1);
    case 2:
      do_say(ch, "I'm a very female dragon.", 0);
      return (1);
    case 3:
    case 4:
      do_shout(ch, "Can someone summon me, please!", 0);
      return (1);
    case 5:
      do_emote(ch, "gropes you.", 0);
      return (1);
    case 6:
      do_emote(ch,
        "gives you a long and passionate kiss.  It seems to last forever.", 0);
      return (1);
    case 7: {
      for (i = character_list; i; i = i->next) {
        if (!IS_NPC(i)) {
          if (number(0, 5) == 0) {
            if (!strcmp(GET_NAME(i), "Brutius")) {
              do_shout(ch, "Brutius!  I have some files for you to copy!", 0);
            } else if (!strcmp(GET_NAME(i), "Batopr")) {
              do_shout(ch, "Batopr!  Why have you been messing with the code?",
                0);
            } else if (!strcmp(GET_NAME(i), "Stargazer")) {
              do_shout(ch, "Stargazer! Get back to work!!", 0);
            } else if (!strcmp(GET_NAME(i), "Gonge")) {
              do_shout(ch, "Gonge! I heard you have done kiddie porn!", 0);
            } else if (GET_SEX(i) == SEX_MALE) {
              sprintf(buf, "Hey, %s, how about some MUDSex?", GET_NAME(i));
              do_shout(ch, buf, 0);
            } else {
              sprintf(buf, "I'm much prettier than %s, don't you think?",
                GET_NAME(i));
              do_shout(ch, buf, 0);
            }
          }
        }
        break;
      }
    }
      return (1);
    case 8:
      do_say(ch, "Brutius is my hero!", 0);
      return (1);
    case 9:
      do_say(ch, "So, wanna neck?", 0);
      return (1);
    case 10: {
      tmp_ch = (struct char_data*)FindAnyVictim(ch);
      if (!IS_NPC(ch)) {
        sprintf(buf, "Party on, %s", GET_NAME(tmp_ch));
        do_say(ch, buf, 0);
        return (1);
      } else {
        return (0);
      }
    }
    case 11:
      do_shout(ch, "NOT!!!", 0);
      return (1);
    case 12:
      do_say(ch, "Bad news.  Termites.", 0);
      return (1);
    case 13:
      for (i = character_list; i; i = i->next) {
        if (!IS_NPC(i)) {
          if (number(0, 15) == 0) {
            sprintf(buf, "%s shout I love Puff!", GET_NAME(i));
            do_force(ch, buf, 0);
            do_restore(ch, GET_NAME(i), 0);
            return (TRUE);
          }
        }
      }
      return (1);
    case 14:
      do_say(ch, "I'll be back.", 0);
      return (1);
    case 15:
      do_say(ch, "Aren't wombat's so cute?", 0);
      return (1);
    case 16:
      do_emote(ch, "fondly fondles you.", 0);
      return (1);
    case 17:
      do_emote(ch, "winks at you.", 0);
      return (1);
    case 18:
      do_say(ch, "ACHOOOOOOOO! This mud makes me sneeze!", 0);
      return (1);
    case 19:
      do_say(ch, "If the Mayor is in a room alone, ", 0);
      do_say(ch, "Does he say 'Good morning citizens.'?", 0);
      return (0);
    case 20:
      for (i = character_list; i; i = i->next) {
        if (!IS_NPC(i)) {
          if (number(0, 15) == 0) {
            sprintf(buf, "Top of the morning to you %s!", GET_NAME(i));
            do_shout(ch, buf, 0);
            return (TRUE);
          }
        }
      }
      break;
    case 21:
      for (i = real_roomp(ch->in_room)->people; i; i = i->next_in_room) {
        if (!IS_NPC(i)) {
          if (number(0, 3) == 0) {
            sprintf(buf,
              "Pardon me, %s, but are those bugle boy jeans you are wearing?",
              GET_NAME(i));
            do_say(ch, buf, 0);
            return (TRUE);
          }
        }
      }
      break;
    case 22:
      for (i = real_roomp(ch->in_room)->people; i; i = i->next_in_room) {
        if (!IS_NPC(i)) {
          if (number(0, 3) == 0) {
            sprintf(buf, "Pardon me, %s, but do you have any Grey Poupon?",
              GET_NAME(i));
            do_say(ch, buf, 0);
            return (TRUE);
          }
        }
      }
      break;
    case 23:
      if (number(0, 20) == 0) {
        do_shout(ch, "Level!!!!!!", 0);
      }
      break;
    case 24:
      do_say(ch, "MTV... get off the air!", 0);
      return (TRUE);
      break;
    case 25:
      do_say(ch, "Time for a RatFest Quest!", 0);
      return (TRUE);
      break;
    case 26:
      if (number(0, 10) == 0)
        do_shout(ch,
          "Don't touch that you fool!  Thats the history eraser button!", 0);
      break;
    case 27:
      do_say(ch, "RESOLVED:  The future's so bright, I gotta wear shades!", 0);
      return (TRUE);
    case 28:
      do_shout(ch, "SAVE!  The game is becoming self-aware!", 0);
      return (TRUE);
    case 29:
      do_say(ch, "HEY!  KOOLAID!!!", 0);
      return (TRUE);
    case 30:
      do_say(ch, "I'm fully functional, you know.", 0);
      return (TRUE);
    case 31:
      do_say(ch, "Join the Deamons, and live forever.", 0);
      return (TRUE);
    case 32:
      if (number(0, 10) == 0) {
        do_shout(ch, "Sex for sale!", 0);
        return (TRUE);
      }
      break;
    case 33:
      if (number(0, 10) == 0) {
        do_shout(ch, "Anyone want an equalizer??", 0);
        return (TRUE);
      }
      break;
    case 34:
      if (number(0, 5) == 0) {
        for (i = character_list; i; i = i->next) {
          if (mob_index[i->nr].func.mob_f == Inquisitor) {
            do_shout(ch, "I wasn't expecting the Spanish Inquisition!", 0);
            return (TRUE);
          }
        }
        return (TRUE);
      }
      break;
    case 35:
      do_say(ch, "Are you crazy, is that your problem?", 0);
      return (TRUE);
    case 36:
      for (i = real_roomp(ch->in_room)->people; i; i = i->next_in_room) {
        if (!IS_NPC(i)) {
          if (number(0, 3) == 0) {
            sprintf(buf, "%s, do you 'Think I'm Going Bald'?", GET_NAME(i));
            do_say(ch, buf, 0);
            return (TRUE);
          }
        }
      }
      break;
    case 37:
      do_say(ch, "This is your brain.", 0);
      do_say(ch, "This is MUD.", 0);
      do_say(ch, "This is your brain on MUD.", 0);
      do_say(ch, "Any questions?", 0);
      return (TRUE);
    case 38:
      for (i = character_list; i; i = i->next) {
        if (!IS_NPC(i)) {
          if (number(0, 20) == 0) {
            if (i->in_room != NOWHERE) {
              sprintf(buf, "%s save", GET_NAME(i));
              do_force(ch, buf, 0);
              return (TRUE);
            }
          }
        }
      }
      return (TRUE);
    case 39:
      do_say(ch, "I'm Puff the Magic Dragon, who the hell are you?", 0);
      return (TRUE);
    case 40:
      do_say(ch, "Attention all planets of the Solar Federation!", 0);
      do_say(ch, "We have assumed control.", 0);
      return (TRUE);
    case 41:
      if (Silence == 1) {
        do_shout(ch, "I can shout and you can't!!!!!", 0);
        return (TRUE);
      }
      break;
    case 42:
      if (number(0, 10) == 0) {
        do_shout(ch, "SPOON!", 0);
        return (TRUE);
      }
      break;
    case 43:
      do_say(ch, "Pardon me boys, is this the road to Great Cthulhu?", 0);
      return (TRUE);
    case 44:
      do_say(ch, "May the Force be with you... Always.", 0);
      return (TRUE);
    case 45:
      do_say(ch, "Eddies in the space time continuum.", 0);
      return (TRUE);
    case 46:
      do_say(ch, "Quick!  Reverse the polarity of the neutron flow!", 0);
      return (TRUE);
    case 47:
      if (number(0, 10) == 0) {
        do_shout(ch, "It's the dragon that you love to hate!", 0);
        do_shout(ch, "Coming out of Mississippi State!", 0);
        return (TRUE);
      }
      break;
    case 48:
      do_say(ch, "Shh...  I'm beta testing.  I need complete silence!", 0);
      return (TRUE);
    case 49:
      do_say(ch, "Do you have any more of that Plutonium Nyborg!", 0);
      return (TRUE);
    case 50:
      do_say(ch, "I'm the real implementor, you know.", 0);
      return (TRUE);

    default:
      return (0);
  }
}

int regenerator(struct char_data* ch, int cmd, char* arg) {
  if (cmd)
    return (FALSE);

  if (GET_HIT(ch) < GET_MAX_HIT(ch)) {
    GET_HIT(ch) += 9;
    GET_HIT(ch) = MIN(GET_HIT(ch), GET_MAX_HIT(ch));

    act("$n regenerates.", TRUE, ch, 0, 0, TO_ROOM);
    return (TRUE);
  }
}

int mega_regenerator(struct char_data* ch, int cmd, char* arg) {
  if (cmd)
    return (FALSE);

  if (GET_HIT(ch) < GET_MAX_HIT(ch)) {
    GET_HIT(ch) += 549;
    GET_HIT(ch) = MIN(GET_HIT(ch), GET_MAX_HIT(ch));

    act("$n licks his wounds and regenerates!", TRUE, ch, 0, 0, TO_ROOM);
    return (TRUE);
  }
}

int replicant(struct char_data* ch, int cmd, char* arg) {
  struct char_data* mob;

  if (cmd)
    return FALSE;

  if (GET_HIT(ch) < GET_MAX_HIT(ch)) {
    act("Drops of $n's blood hit the ground, and spring up into another one!",
      TRUE, ch, 0, 0, TO_ROOM);
    mob = read_mobile(ch->nr, REAL);
    char_to_room(mob, ch->in_room);
    act("Two undamaged opponents face you now.", TRUE, ch, 0, 0, TO_ROOM);
    GET_HIT(ch) = GET_MAX_HIT(ch);
  }

  return FALSE;
}

#define TYT_NONE 0
#define TYT_CIT 1
#define TYT_WHAT 2
#define TYT_TELL 3
#define TYT_HIT 4

int Tytan(struct char_data* ch, int cmd, char* arg) {
  struct char_data* vict;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    return (magic_user(ch, cmd, arg));
  } else {
    if (!ch->act_ptr) /* no state info */
      ch->act_ptr = (int*)calloc(1, (sizeof(int)));
    switch ((*((int*)ch->act_ptr))) {
      case TYT_NONE:
        if (vict = FindVictim(ch)) {
          (*((int*)ch->act_ptr)) = TYT_CIT;
          SetHunting(ch, vict);
        }
        break;
      case TYT_CIT:
        if (ch->specials.hunting) {
          if (IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
            REMOVE_BIT(ch->specials.act, ACT_AGGRESSIVE);
          }
          if (ch->in_room == ch->specials.hunting->in_room) {
            act("Where is the Citadel?", TRUE, ch, 0, 0, TO_ROOM);
            (*((int*)ch->act_ptr)) = TYT_WHAT;
          }
        } else {
          (*((int*)ch->act_ptr)) = TYT_NONE;
        }
        break;
      case TYT_WHAT:
        if (ch->specials.hunting) {
          if (ch->in_room == ch->specials.hunting->in_room) {
            act("What must we do?", TRUE, ch, 0, 0, TO_ROOM);
            (*((int*)ch->act_ptr)) = TYT_TELL;
          }
        } else {
          (*((int*)ch->act_ptr)) = TYT_NONE;
        }
        break;
      case TYT_TELL:
        if (ch->specials.hunting) {
          if (ch->in_room == ch->specials.hunting->in_room) {
            act("Tell Us!  Command Us!", TRUE, ch, 0, 0, TO_ROOM);
            (*((int*)ch->act_ptr)) = TYT_HIT;
          }
        } else {
          (*((int*)ch->act_ptr)) = TYT_NONE;
        }
        break;
      case TYT_HIT:
        if (ch->specials.hunting) {
          if (ch->in_room == ch->specials.hunting->in_room) {
            if (!check_peaceful(ch, "The Tytan screams in anger")) {
              hit(ch, ch->specials.hunting, TYPE_UNDEFINED);
              if (!IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
                SET_BIT(ch->specials.act, ACT_AGGRESSIVE);
              }
              (*((int*)ch->act_ptr)) = TYT_NONE;
            } else {
              (*((int*)ch->act_ptr)) = TYT_CIT;
            }
          }
        } else {
          (*((int*)ch->act_ptr)) = TYT_NONE;
        }
        break;
      default:
        (*((int*)ch->act_ptr)) = TYT_NONE;
    }
  }
}

int AbbarachDragon(struct char_data* ch, int cmd, char* arg) {
  struct char_data* targ;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (!ch->specials.fighting) {
    targ = (struct char_data*)FindAnyVictim(ch);
    if (targ && !check_peaceful(ch, "")) {
      hit(ch, targ, TYPE_UNDEFINED);
      act("You have now payed the price of crossing.", TRUE, ch, 0, 0, TO_ROOM);
      return (TRUE);
    }
  } else {
    return (BreathWeapon(ch, cmd, arg));
  }
}

int fido(struct char_data* ch, int cmd, char* arg) {
  register struct obj_data *i, *temp, *next_obj, *next_r_obj;
  register struct char_data *v, *next;
  register struct room_data* rp;
  char found = FALSE;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if ((rp = real_roomp(ch->in_room)) == 0)
    return (FALSE);

  for (v = rp->people; (v && (!found)); v = next) {
    next = v->next_in_room;
    if ((IS_NPC(v)) && (mob_index[v->nr].virtual == 100) &&
        CAN_SEE(ch, v)) { /* is a zombie */
      if (v->specials.fighting)
        stop_fighting(v);
      make_corpse(v);
      extract_char(v);
      found = TRUE;
    }
  }

  for (i = real_roomp(ch->in_room)->contents; i; i = next_r_obj) {
    next_r_obj = i->next_content;
    if (GET_ITEM_TYPE(i) == ITEM_CONTAINER && i->obj_flags.value[3]) {
      act("$n savagely devours a corpse.", FALSE, ch, 0, 0, TO_ROOM);
      for (temp = i->contains; temp; temp = next_obj) {
        next_obj = temp->next_content;
        obj_from_obj(temp);
        obj_to_room(temp, ch->in_room);
      }
      extract_obj(i);
      return (TRUE);
    }
  }
  return (FALSE);
}

int janitor(struct char_data* ch, int cmd, char* arg) {
  struct obj_data *i, *temp, *next_obj;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
    if (IS_SET(i->obj_flags.wear_flags, ITEM_TAKE) &&
        ((i->obj_flags.type_flag == ITEM_DRINKCON) ||
          (i->obj_flags.cost <= 10))) {
      act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);

      obj_from_room(i);
      obj_to_char(i, ch);
      return (TRUE);
    }
  }
  return (FALSE);
}

int tormentor(struct char_data* ch, int cmd, char* arg) {
  if (!cmd)
    return (FALSE);

  if (IS_NPC(ch))
    return (FALSE);

  if (IS_IMMORTAL(ch))
    return (FALSE);

  return (TRUE);
}

int magneto(struct char_data* ch, int cmd, char* arg) {
  struct char_data* victim;
  struct obj_data *finger, *neck, *weapon, *held, *body;
  int percent;
  char buf[200];

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (!ch->specials.fighting)
    return (FALSE);

  percent = number(1, 5);

  victim = FindVictim(ch);

  if (!victim)
    victim = ch->specials.fighting;

  if (!victim)
    return FALSE;

  switch (percent) {
    case 1:
    case 2:
    case 3:
    case 4: {
      send_to_char("You sure are lucky.\n\r", victim);
      return FALSE;
      break;
    }
    case 5: {
      act(
        "$n has become very upset. He has decided to use his mutant powers on "
        "$N",
        TRUE, ch, 0, victim, TO_ROOM);
      weapon = victim->equipment[WIELD];
      neck = victim->equipment[WEAR_NECK_1];
      held = victim->equipment[HOLD];
      body = victim->equipment[WEAR_BODY];
      finger = victim->equipment[WEAR_FINGER_R];

      send_to_char("Magneto has forced you to remove your equipment!.\n\r",
        victim);
      send_to_char("Magneto makes you give him your equipment!.\n\r", victim);

      if (weapon) {
        do_remove(victim, weapon->name, 0);
        do_drop(victim, weapon->name, 0);
        sprintf(buf, "get all");
        command_interpreter(ch, buf);
        sprintf(buf, "wear all");
        command_interpreter(ch, buf);
      } else if (held) {
        do_remove(victim, held->name, 0);
        do_drop(victim, held->name, 0);
        sprintf(buf, "get all");
        command_interpreter(ch, buf);
        sprintf(buf, "wear all");
        command_interpreter(ch, buf);
      } else if (finger) {
        do_remove(victim, finger->name, 0);
        do_drop(victim, finger->name, 0);
        sprintf(buf, "get all");
        command_interpreter(ch, buf);
        sprintf(buf, "wear all");
        command_interpreter(ch, buf);
      } else if (neck) {
        do_remove(victim, neck->name, 0);
        do_drop(victim, neck->name, 0);
        sprintf(buf, "get all");
        command_interpreter(ch, buf);
        sprintf(buf, "wear all");
        command_interpreter(ch, buf);
      }
      break;
    }
  }
}

int RustMonster(struct char_data* ch, int cmd, char* arg) {
  struct char_data* vict;
  struct obj_data* t_item;
  int t_pos;

  return (FALSE);

  /*
  **   find a victim
  */
  if (ch->specials.fighting) {
    vict = ch->specials.fighting;
  } else {
    vict = FindVictim(ch);
    if (!vict) {
      return (FALSE);
    }
  }

  /*
  **   choose an item of armor or a weapon that is metal
  **  since metal isn't defined, we'll just use armor and weapons
  */

  /*
  **  choose a weapon first, then if no weapon, choose a shield,
  **  if no shield, choose breast plate, then leg plate, sleeves,
  **  helm
  */

  if (vict->equipment[WIELD]) {
    t_item = vict->equipment[WIELD];
    t_pos = WIELD;
  } else if (vict->equipment[WEAR_SHIELD]) {
    t_item = vict->equipment[WEAR_SHIELD];
    t_pos = WEAR_SHIELD;
  } else if (vict->equipment[WEAR_BODY]) {
    t_item = vict->equipment[WEAR_BODY];
    t_pos = WEAR_BODY;
  } else if (vict->equipment[WEAR_LEGS]) {
    t_item = vict->equipment[WEAR_LEGS];
    t_pos = WEAR_LEGS;
  } else if (vict->equipment[WEAR_ARMS]) {
    t_item = vict->equipment[WEAR_ARMS];
    t_pos = WEAR_ARMS;
  } else if (vict->equipment[WEAR_HEAD]) {
    t_item = vict->equipment[WEAR_HEAD];
    t_pos = WEAR_HEAD;
  } else {
    return (FALSE);
  }
}

int temple_labrynth_liar(struct char_data* ch, int cmd, char* arg) {
  if (cmd || !AWAKE(ch))
    return (0);

  if (check_soundproof(ch))
    return (FALSE);

  switch (number(0, 15)) {
    case 0:
      do_say(ch, "I'd go west if I were you.", 0);
      return (1);
    case 1:
      do_say(ch, "I heard that Vile is a cute babe.", 0);
      return (1);
    case 2:
      do_say(ch, "Going east will avoid the beast!", 0);
      return (1);
    case 4:
      do_say(ch, "North is the way to go.", 0);
      return (1);
    case 6:
      do_say(ch, "Dont dilly dally go south.", 0);
      return (1);
    case 8:
      do_say(ch, "Great treasure lies ahead", 0);
      return (1);
    case 10:
      do_say(ch,
        "I wouldn't kill the sentry if I were more than level 9. No way!", 0);
      return (1);
    case 12:
      do_say(ch, "I am a very clever liar.", 0);
      return (1);
    case 14:
      do_say(ch, "Loki is a really great guy!", 0);
      do_say(ch, "Well.... maybe not...", 0);
      return (1);
    default:
      do_say(ch, "Then again I could be wrong!", 0);
      return (1);
  }
}

int temple_labrynth_sentry(struct char_data* ch, int cmd, char* arg) {
  struct char_data* tch;
  int counter;

  if (cmd || !AWAKE(ch))
    return FALSE;

  if (GET_POS(ch) != POSITION_FIGHTING)
    return FALSE;

  if (!ch->specials.fighting)
    return FALSE;

  if (check_soundproof(ch))
    return (FALSE);

  /* Find a dude to do very evil things upon ! */

  for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if (GetMaxLevel(tch) > 10 && CAN_SEE(ch, tch)) {
      act("The sentry snaps out of his trance and ...", 1, ch, 0, 0, TO_ROOM);
      do_say(ch, "You will die for your insolence, pig-dog!", 0);
      for (counter = 0; counter < 4; counter++)
        if (GET_POS(tch) > POSITION_SITTING)
          cast_fireball(15, ch, "", SPELL_TYPE_SPELL, tch, 0);
        else
          return TRUE;
      return TRUE;
    } else {
      act("The sentry looks concerned and continues to push you away", 1, ch, 0,
        0, TO_ROOM);
      do_say(ch, "Leave me alone. My vows do not permit me to kill you!", 0);
    }
  }
  return TRUE;
}

#define WW_LOOSE 0
#define WW_FOLLOW 1

int Whirlwind(struct char_data* ch, int cmd, char* arg) {
  struct char_data* tmp;
  const char* names[] = {"Loki", "Belgarath", 0};
  int i = 0;

  if (ch->in_room == -1)
    return (FALSE);
  if (!ch->act_ptr)
    ch->act_ptr = (int*)calloc(1, sizeof(int));
  if (cmd == 0 && (*((int*)ch->act_ptr)) == WW_LOOSE) {
    for (tmp = real_roomp(ch->in_room)->people; tmp; tmp = tmp->next_in_room) {
      while (names[i]) {
        if (!strcmp(GET_NAME(tmp), names[i]) &&
            (*((int*)ch->act_ptr)) == WW_LOOSE) {
          /* start following */
          if (circle_follow(ch, tmp))
            return (FALSE);
          if (ch->master)
            stop_follower(ch);
          add_follower(ch, tmp);
          (*((int*)ch->act_ptr)) = WW_FOLLOW;
        }
        i++;
      }
    }
    if ((*((int*)ch->act_ptr)) == WW_LOOSE && !cmd) {
      act("The $n suddenly dissispates into nothingness.", 0, ch, 0, 0,
        TO_ROOM);
      extract_char(ch);
    }
  }
}

#define NN_LOOSE 0
#define NN_FOLLOW 1
#define NN_STOP 2

int NudgeNudge(struct char_data* ch, int cmd, char* arg) {
  struct char_data* vict;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    return (FALSE);
  }

  if (!ch->act_ptr)
    ch->act_ptr = (int*)calloc(1, sizeof(int));
  switch ((*((int*)ch->act_ptr))) {
    case NN_LOOSE:
      /*
      ** find a victim
      */
      vict = FindVictim(ch);
      if (!vict)
        return (FALSE);
      /* start following */
      if (circle_follow(ch, vict)) {
        return (FALSE);
      }
      if (ch->master)
        stop_follower(ch);
      add_follower(ch, vict);
      (*((int*)ch->act_ptr)) = NN_FOLLOW;
      if (!check_soundproof(ch))
        do_say(ch, "Good Evenin' Squire!", 0);
      act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
      break;
    case NN_FOLLOW:
      switch (number(0, 20)) {
        case 0:
          if (!check_soundproof(ch))
            do_say(ch, "Is your wife a goer?  Know what I mean, eh?", 0);
          act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
          act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
          break;
        case 1:
          act("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
          act("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
          act("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
          act("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
          act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
          act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
          act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
          act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
          if (!check_soundproof(ch))
            do_say(ch, "Say no more!  Say no MORE!", 0);
          break;
        case 2:
          if (!check_soundproof(ch)) {
            do_say(ch, "You been around, eh?", 0);
            do_say(ch, "...I mean you've ..... done it, eh?", 0);
          }
          act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
          act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
          act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
          act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
          break;
        case 3:
          if (!check_soundproof(ch))
            do_say(ch, "A nod's as good as a wink to a blind bat, eh?", 0);
          act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
          act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
          act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
          act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
          break;
        case 4:
          if (!check_soundproof(ch))
            do_say(ch, "You're WICKED, eh!  WICKED!", 0);
          act("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
          act("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
          act("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
          act("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
          break;
        case 5:
          if (!check_soundproof(ch))
            do_say(ch, "Wink. Wink.", 0);
          break;
        case 6:
          if (!check_soundproof(ch))
            do_say(ch, "Nudge. Nudge.", 0);
          break;
        case 7:
        case 8:
          (*((int*)ch->act_ptr)) = NN_STOP;
          break;
        default:
          break;
      }
      break;
    case NN_STOP:
      /*
      **  Stop following
      */
      if (!check_soundproof(ch))
        do_say(ch, "Evening, Squire", 0);
      stop_follower(ch);
      (*((int*)ch->act_ptr)) = NN_LOOSE;
      break;
    default:
      (*((int*)ch->act_ptr)) = NN_LOOSE;
      break;
  }
}

int AGGRESSIVE(struct char_data* ch, int cmd, char* arg) {
  struct char_data *i, *next;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->in_room > -1) {
    for (i = real_roomp(ch->in_room)->people; i; i = next) {
      next = i->next_in_room;
      if (i->nr != ch->nr) {
        if (!IS_IMMORTAL(i)) {
          hit(ch, i, TYPE_UNDEFINED);
        }
      }
    }
  }
}

int citizen(struct char_data* ch, int cmd, char* arg) {
  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    fighter(ch, cmd, arg);

    if (check_soundproof(ch))
      return (FALSE);

    if (number(0, 18) == 0) {
      do_shout(ch, "Guards! Help me! Please!", 0);
    } else {
      act("$n shouts 'Guards!  Help me! Please!'", TRUE, ch, 0, 0, TO_ROOM);
    }
  }
  return (FALSE);
}

int aunt_bee(struct char_data* ch, int cmd, char* arg) {
  struct char_data *tch, *evil, *i;

  if (cmd || !AWAKE(ch))
    return utility_irritable(ch, cmd, arg, aunt_bee);
  if (ch->specials.fighting) {
    if (number(0, 20) == 0) {
      do_shout(ch, "Help me! Help me! I am being attacked!!", 0);
    } else {
      act("$n shouts 'Help me! Help me! I am being attacked!'", TRUE, ch, 0, 0,
        TO_ROOM);
    }
    CallForGuard(ch, ch->specials.fighting, 3, BEE);

    return (TRUE);
  }
}

int cityguard(struct char_data* ch, int cmd, char* arg) {
  struct char_data *tch, *evil, *i;
  int max_evil, lev;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    fighter(ch, cmd, arg);

    if (!check_soundproof(ch)) {
      if (number(0, 20) == 0) {
        do_shout(ch, "To me, my fellows! I am in need of thy aid!", 0);
      } else {
        act("$n shouts 'To me, my fellows! I need thy aid!'", TRUE, ch, 0, 0,
          TO_ROOM);
      }

      if (ch->specials.fighting)
        CallForGuard(ch, ch->specials.fighting, 3, MIDGAARD);

      return (TRUE);
    }
  }

  max_evil = 1000;
  evil = 0;

  if (check_peaceful(ch, ""))
    return FALSE;

  for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if ((IS_PC(tch)) &&
        (IS_SET(tch->specials.act, PLR_KILLER) ||
          IS_SET(tch->specials.act, PLR_OUTLAW)) &&
        CAN_SEE(ch, tch)) {
      act("$n screams '$N is a CRIMINAL! All OUTLAWS must DIE!!'", FALSE, ch, 0,
        tch, TO_ROOM);
      hit(ch, tch, TYPE_UNDEFINED);
      return (TRUE);
    }

    if ((IS_NPC(tch)) && (IsUndead(tch)) && CAN_SEE(ch, tch)) {
      max_evil = -1000;
      evil = tch;
      if (!check_soundproof(ch))
        act("$n screams 'EVIL!!!  BANZAI!  SPOOON!'", FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, evil, TYPE_UNDEFINED);
      return (TRUE);
    }
    if (tch->specials.fighting) {
      if ((GET_ALIGNMENT(tch) < max_evil) &&
          (IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
        max_evil = GET_ALIGNMENT(tch);
        evil = tch;
      }
    }
  }

  if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0)) {
    if (!check_soundproof(ch))
      act("$n screams 'PROTECT THE INNOCENT! BANZAI!!! CHARGE!!! SPOON!'",
        FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, evil, TYPE_UNDEFINED);
    return (TRUE);
  }

  return (FALSE);
}

#define ONE_RING 1105

static int room_of_object(struct obj_data* obj) {
  if (obj->in_room != NOWHERE)
    return obj->in_room;
  else if (obj->carried_by)
    return obj->carried_by->in_room;
  else if (obj->equipped_by)
    return obj->equipped_by->in_room;
  else if (obj->in_obj)
    return room_of_object(obj->in_obj);
  else
    return NOWHERE;
}

int is_target_room_p(int room, int tgt_room) { return room == tgt_room; }

int Ringwraith(struct char_data* ch, int cmd, char* arg) {
  static char buf[256];
  struct char_data* victim;
  static int quantrings = -1;
  struct obj_data* ring;

  int rnum, dir;

  if (!AWAKE(ch) || !IS_NPC(ch) || cmd) {
    return (FALSE);
  }

  if (ch->specials.fighting) {
    fighter(ch, cmd, arg);
    return (FALSE);
  }

  if (quantrings == -1) { /* how many one rings are in the game? */
    quantrings = 1;
    get_obj_vis_world(ch, "999.one ring.", &quantrings);
  }

  struct mob_act_ringwraith* wh;

  /* does our ringwraith have his state info? */
  if (!ch->act_ptr) {
    ch->act_ptr = malloc(sizeof(struct mob_act_data));
    if (!ch->act_ptr) {
      vlog("Unable to allocate memory for act_ptr in Ringwraith proc");
      return FALSE;
    }
    ch->act_ptr->type = MOB_ACT_RINGWRAITH;
    wh = &ch->act_ptr->data.ringwraith;
    wh->chances = 0;
    wh->ringnumber = 0;
  } else {
    if (ch->act_ptr->type != MOB_ACT_RINGWRAITH) {
      vlog("Ringwraith proc called on non-Ringwraith mob");
      return FALSE;
    }
    wh = &ch->act_ptr->data.ringwraith;
  }

  /* is he currently tracking a ring */
  if (!wh->ringnumber) {
    wh->chances = 0;
    wh->ringnumber = number(1, quantrings++);
  }

  sprintf(buf, "%d.one ring.", (int)wh->ringnumber); /* where is this ring? */
  if (NULL == (ring = get_obj_vis_world(ch, buf, NULL))) {
    /* there aren't as many one rings in the game as we thought */
    quantrings = 1;
    get_obj_vis_world(ch, "999.one ring.", &quantrings);
    wh->ringnumber = 0;
    return FALSE;
  }

  rnum = room_of_object(ring);

  if (rnum != ch->in_room) {
    struct find_path_data fpd;
    fpd.type = FIND_TARGET_ROOM;
    fpd.fn_data.target_room = rnum;
    fpd.fn.is_target_room_fn = is_target_room_p;

    dir = find_path(ch->in_room, &fpd, -5000, 0);

    if (dir < 0) {
      /* we can't find the ring */
      wh->ringnumber = 0;
      return FALSE;
    }

    go_direction(ch, dir);
    return TRUE;
  }

  /* the ring is in the same room! */

  if (victim = char_holding(ring)) {
    if (victim == ch) {
      obj_from_char(ring);
      extract_obj(ring);
      wh->ringnumber = 0;
      act("$n grimaces happily.", FALSE, ch, NULL, victim, TO_ROOM);
    } else {
      switch (wh->chances) {
        case 0:
          do_wake(ch, GET_NAME(victim), 0);
          if (!check_soundproof(ch))
            act("$n says '$N, give me The Ring'.", FALSE, ch, NULL, victim,
              TO_ROOM);
          else
            act("$n pokes you in the ribs.", FALSE, ch, NULL, victim, TO_ROOM);
          wh->chances++;
          return (TRUE);
          break;
        case 1:
          if (IS_NPC(victim)) {
            act("$N quickly surrenders The Ring to $n.", FALSE, ch, NULL,
              victim, TO_ROOM);
            if (ring->carried_by)
              obj_from_char(ring);
            else if (ring->equipped_by)
              unequip_char(victim, ring->eq_pos);
            obj_to_char(ring, ch);
          } else {
            if (!check_soundproof(ch))
              act("$n says '$N, give me The Ring *NOW*'.", FALSE, ch, NULL,
                victim, TO_ROOM);
            else {
              act("$n pokes you in the ribs very painfully.", FALSE, ch, NULL,
                victim, TO_ROOM);
            }

            wh->chances++;
          }
          return (TRUE);
          break;
        default:
          if (check_peaceful(ch, "Damn, he's in a safe spot.")) {
            if (!check_soundproof(ch))
              act("$n says 'You can't stay here forever, $N'.", FALSE, ch, NULL,
                victim, TO_ROOM);
          } else {
            if (!check_soundproof(ch))
              act("$n says 'I guess I'll just have to get it myself'.", FALSE,
                ch, NULL, victim, TO_ROOM);
            hit(ch, victim, TYPE_UNDEFINED);
          }
          break;
      }
    }
  } else if (ring->in_obj) {
    /* the ring is in an object */
    obj_from_obj(ring);
    obj_to_char(ring, ch);
    act("$n gets the One Ring.", FALSE, ch, NULL, victim, TO_ROOM);
  } else if (ring->in_room != NOWHERE) {
    obj_from_room(ring);
    obj_to_char(ring, ch);
    act("$n gets the Ring.", FALSE, ch, NULL, 0, TO_ROOM);
  } else {
    vlog("a One Ring was completely disconnected!?");
    wh->ringnumber = 0;
  }
  return TRUE;
}

int WarrenGuard(struct char_data* ch, int cmd, char* arg) {
  struct char_data *tch, *good, *i;
  int max_good, lev;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    fighter(ch, cmd, arg);
    return (TRUE);
  }

  max_good = -1000;
  good = 0;

  if (check_peaceful(ch, ""))
    return FALSE;

  for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if (tch->specials.fighting) {
      if ((GET_ALIGNMENT(tch) > max_good) &&
          (IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
        max_good = GET_ALIGNMENT(tch);
        good = tch;
      }
    }
  }

  if (good && (GET_ALIGNMENT(good->specials.fighting) <= 0)) {
    if (!check_soundproof(ch))
      act("$n screams 'DEATH TO GOODY-GOODIES!!!!'", FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, good, TYPE_UNDEFINED);
    return (TRUE);
  }

  return (FALSE);
}

int zm_tired(struct char_data* zmaster) {
  return GET_HIT(zmaster) < GET_MAX_HIT(zmaster) / 2 || GET_MANA(zmaster) < 40;
}

int zm_stunned_followers(struct char_data* zmaster) {
  struct follow_type* fwr;
  for (fwr = zmaster->followers; fwr; fwr = fwr->next)
    if (GET_POS(fwr->follower) == POSITION_STUNNED)
      return TRUE;
  return FALSE;
}

void zm_init_combat(struct char_data* zmaster, struct char_data* target) {
  struct follow_type* fwr;
  for (fwr = zmaster->followers; fwr; fwr = fwr->next)
    if (IS_AFFECTED(fwr->follower, AFF_CHARM) &&
        fwr->follower->specials.fighting == NULL &&
        fwr->follower->in_room == target->in_room)
      if (GET_POS(fwr->follower) == POSITION_STANDING) {
        hit(fwr->follower, target, TYPE_UNDEFINED);
      } else if (GET_POS(fwr->follower) > POSITION_SLEEPING &&
                 GET_POS(fwr->follower) < POSITION_FIGHTING) {
        do_stand(fwr->follower, "", -1);
      }
}

int zm_kill_fidos(struct char_data* zmaster) {
  struct char_data* fido_b;
  fido_b = FindMobInRoomWithFunction(zmaster->in_room, fido);
  if (fido_b) {
    if (!check_soundproof(zmaster)) {
      act("$n shrilly screams 'Kill that carrion beast!'", FALSE, zmaster, 0, 0,
        TO_ROOM);
      zm_init_combat(zmaster, fido_b);
    }
    return TRUE;
  }
  return FALSE;
}

int zm_kill_aggressor(struct char_data* zmaster) {
  struct follow_type* fwr;
  if (zmaster->specials.fighting) {
    if (!check_soundproof(zmaster)) {
      act("$n bellows 'Kill that mortal that dares lay hands on me!'", FALSE,
        zmaster, 0, 0, TO_ROOM);
      zm_init_combat(zmaster, zmaster->specials.fighting);
      return TRUE;
    }
  }
  for (fwr = zmaster->followers; fwr; fwr = fwr->next)
    if (fwr->follower->specials.fighting &&
        IS_AFFECTED(fwr->follower, AFF_CHARM)) {
      if (!check_soundproof(zmaster)) {
        act("$n bellows 'Assist your brethren, my loyal servants!'", FALSE,
          zmaster, 0, 0, TO_ROOM);
        zm_init_combat(zmaster, fwr->follower->specials.fighting);
        return TRUE;
      }
    }
  return FALSE;
}

static int named_object_on_ground(int room, char* c_data) {
  return 0 != get_obj_in_list(c_data, real_roomp(room)->contents);
}

static const struct find_path_data find_corpse_on_ground = {
  .type = FIND_OBJECT_ON_GROUND,
  .fn_data.obj_name = "corpse",
  .fn.obj_on_ground_fn = named_object_on_ground,
};

int zombie_master(struct char_data* ch, int cmd, char* arg)
#define ZM_MANA 10
#define ZM_NEMESIS 3060
{
  struct obj_data* temp1;
  struct char_data* zmaster;
  char buf[240];

  zmaster = FindMobInRoomWithFunction(ch->in_room, zombie_master);

  if (cmd != 0 || ch != zmaster || !AWAKE(ch))
    return FALSE;

  if (!check_peaceful(ch, "") &&
      (zm_kill_fidos(zmaster) || zm_kill_aggressor(zmaster))) {
    do_stand(zmaster, "", -1);
    return TRUE;
  }

  switch (GET_POS(zmaster)) {
    case POSITION_RESTING:
      if (!zm_tired(zmaster))
        do_stand(zmaster, "", -1);
      break;
    case POSITION_SITTING:
      if (!zm_stunned_followers(zmaster)) {
        if (!check_soundproof(ch))
          act("$n says 'It took you long enough...'", FALSE, zmaster, 0, 0,
            TO_ROOM);
        do_stand(zmaster, "", -1);
      }
      break;
    case POSITION_STANDING: {
      if (zm_tired(zmaster)) {
        do_rest(zmaster, "", -1);
        return TRUE;
      }

      temp1 = get_obj_in_list_vis(zmaster, "corpse",
        real_roomp(zmaster->in_room)->contents);

      if (temp1) {
        if (GET_MANA(zmaster) < ZM_MANA) {
          if (1 == dice(1, 20))
            if (!check_soundproof(ch))
              act("$n says 'So many bodies, so little time' and sighs.", FALSE,
                zmaster, 0, 0, TO_ROOM);
        } else {
          if (!check_soundproof(ch)) {
            act(
              "$n says 'Wonderful, another loyal follower!' and grins "
              "maniacly.",
              FALSE, zmaster, 0, 0, TO_ROOM);
            GET_MANA(zmaster) -= ZM_MANA;
            spell_animate_dead(GetMaxLevel(zmaster), ch, NULL, temp1);
            /* assume the new follower is top of the list? */
            AddHatred(zmaster->followers->follower, OP_VNUM, ZM_NEMESIS);
          }
        }
        return TRUE;
      }

      if (zm_stunned_followers(zmaster)) {
        do_sit(zmaster, "", -1);
        return TRUE;
      }

      if (1 == dice(1, 20)) {
        act("$n searches for bodies.", FALSE, zmaster, 0, 0, TO_ROOM);
        return TRUE;
      }

      int dir = find_path(zmaster->in_room, &find_corpse_on_ground, -200, 0);

      if (0 <= dir) {
        go_direction(zmaster, dir);
        return TRUE;
      }

      if (1 == dice(1, 5)) {
        act("$n can't find any bodies.", FALSE, zmaster, 0, 0, TO_ROOM);
        return TRUE;
      }

      mobile_wander(zmaster);
      break;
    }
  }

  return FALSE;
}

int pet_shops(struct char_data* ch, int cmd, char* arg) {
  char buf[MAX_STRING_LENGTH], pet_name[256];
  int pet_room;
  struct char_data* pet;

  pet_room = ch->in_room + 1;

  if (cmd == 59) { /* List */
    send_to_char("Available pets are:\n\r", ch);
    for (pet = real_roomp(pet_room)->people; pet; pet = pet->next_in_room) {
      sprintf(buf, "%8d - %s\n\r", 24 * GET_EXP(pet), pet->player.short_descr);
      send_to_char(buf, ch);
    }
    return (TRUE);
  } else if (cmd == 56) { /* Buy */

    arg = one_argument(arg, buf);
    only_argument(arg, pet_name);
    /* Pet_Name is for later use when I feel like it */

    if (!(pet = get_char_room(buf, pet_room))) {
      send_to_char("There is no such pet!\n\r", ch);
      return (TRUE);
    }

    if (GET_GOLD(ch) < (GET_EXP(pet) * 10)) {
      send_to_char("You don't have enough gold!\n\r", ch);
      return (TRUE);
    }

    GET_GOLD(ch) -= GET_EXP(pet) * 10;

    pet = read_mobile(pet->nr, REAL);
    GET_EXP(pet) = 0;
    SET_BIT(pet->specials.affected_by, AFF_CHARM);

    if (*pet_name) {
      sprintf(buf, "%s %s", pet->player.name, pet_name);
      free(pet->player.name);
      pet->player.name = strdup(buf);

      sprintf(buf,
        "%sA small sign on a chain around the neck says 'My Name is %s'\n\r",
        pet->player.description, pet_name);
      free(pet->player.description);
      pet->player.description = strdup(buf);
    }

    char_to_room(pet, ch->in_room);
    add_follower(pet, ch);

    IS_CARRYING_W(pet) = 0;
    IS_CARRYING_N(pet) = 0;

    send_to_char("May you enjoy your pet.\n\r", ch);
    act("$n bought $N as a pet.", FALSE, ch, 0, pet, TO_ROOM);

    return (TRUE);
  }

  /* All commands except list and buy */
  return (FALSE);
}

int Fountain(struct char_data* ch, int cmd, char* arg) {
  int bits, water, level;
  char buf[MAX_INPUT_LENGTH];
  struct char_data* tmp_char;
  struct obj_data* obj;

  if (cmd == 248) {               /* fill */
    arg = one_argument(arg, buf); /* buf = object */
    bits = generic_find(buf, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch,
      &tmp_char, &obj);

    if (!bits)
      return (FALSE);

    if (ITEM_TYPE(obj) != ITEM_DRINKCON) {
      send_to_char("Thats not a drink container!\n\r", ch);
      return (TRUE);
    }

    if ((obj->obj_flags.value[2] != LIQ_WATER) &&
        (obj->obj_flags.value[1] != 0)) {
      name_from_drinkcon(obj);
      obj->obj_flags.value[2] = LIQ_SLIME;
      name_to_drinkcon(obj, LIQ_SLIME);
    } else {
      /* Calculate water it can contain */
      water = obj->obj_flags.value[0] - obj->obj_flags.value[1];

      if (water > 0) {
        obj->obj_flags.value[2] = LIQ_WATER;
        obj->obj_flags.value[1] += water;
        weight_change_object(obj, water);
        name_from_drinkcon(obj);
        name_to_drinkcon(obj, LIQ_WATER);
        act("$p is filled.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n fills $p with water.", FALSE, ch, obj, 0, TO_ROOM);
      }
    }
    return (TRUE);

  } else if (cmd == 11) { /* drink */
    only_argument(arg, buf);

    if (!is_abbrev(buf, "fountain") && !is_abbrev(buf, "water"))
      return (FALSE);

    send_to_char("You drink from the fountain.\n\r", ch);

    if (GET_COND(ch, THIRST) >= 0)
      GET_COND(ch, THIRST) = 24;
    if (GET_COND(ch, FULL) >= 0)
      GET_COND(ch, FULL) += 1;

    if (GET_COND(ch, THIRST) > 20)
      act("You do not feel thirsty.", FALSE, ch, 0, 0, TO_CHAR);
    if (GET_COND(ch, FULL) > 20)
      act("You are full.", FALSE, ch, 0, 0, TO_CHAR);

    return (TRUE);
  }

  /* All commands except fill and drink */
  return (FALSE);
}

int bank(struct char_data* ch, int cmd, char* arg) {
  static char buf[256];
  int money;

  money = atoi(arg);

  if (!IS_NPC(ch))
    save_char(ch, ch->in_room);

  if (GET_BANK(ch) > GetMaxLevel(ch) * 200000) {
    send_to_char(
      "I'm sorry, but we can no longer hold more than 200000 coins per "
      "level.\n\r",
      ch);
    GET_GOLD(ch) += GET_BANK(ch) - GetMaxLevel(ch) * 200000;
    GET_BANK(ch) = GetMaxLevel(ch) * 200000;
  }

  /*deposit*/
  if (cmd == 219) {
    if (HasClass(ch, CLASS_MONK) && (GetMaxLevel(ch) < 40)) {
      send_to_char("Your vows forbid you to retain personal wealth\n\r", ch);
      return (TRUE);
    }
    if (money > GET_GOLD(ch)) {
      send_to_char("You don't have enough for that!\n\r", ch);
      return (TRUE);
    } else if (money <= 0) {
      send_to_char("Go away, you bother me.\n\r", ch);
      return (TRUE);
    } else if (money + GET_BANK(ch) > GetMaxLevel(ch) * 40000) {
      send_to_char(
        "I'm sorry, Regulations only allow us to ensure 40000 coins per "
        "level.\n\r",
        ch);
      return (TRUE);
    } else {
      send_to_char("Thank you.\n\r", ch);
      GET_GOLD(ch) = GET_GOLD(ch) - money;
      GET_BANK(ch) = GET_BANK(ch) + money;
      sprintf(buf, "Your balance is %d.\n\r", GET_BANK(ch));
      send_to_char(buf, ch);
      return (TRUE);
    }
    /*withdraw*/
  } else if (cmd == 220) {
    if (HasClass(ch, CLASS_MONK) && (GetMaxLevel(ch) < 40)) {
      send_to_char("Your vows forbid you to retain personal wealth\n\r", ch);
      return (TRUE);
    }

    if (money > GET_BANK(ch)) {
      send_to_char("You don't have enough in the bank for that!\n\r", ch);
      return (TRUE);
    } else if (money <= 0) {
      send_to_char("Go away, you bother me.\n\r", ch);
      return (TRUE);
    } else {
      send_to_char("Thank you.\n\r", ch);
      GET_GOLD(ch) = GET_GOLD(ch) + money;
      GET_BANK(ch) = GET_BANK(ch) - money;
      sprintf(buf, "Your balance is %d.\n\r", GET_BANK(ch));
      send_to_char(buf, ch);
      return (TRUE);
    }
  } else if (cmd == 221) {
    sprintf(buf, "Your balance is %d.\n\r", GET_BANK(ch));
    send_to_char(buf, ch);
    return (TRUE);
  }
  return (FALSE);
}

/* Idea of the LockSmith is functionally similar to the Pet Shop */
/* The problem here is that each key must somehow be associated  */
/* with a certain player. My idea is that the players name will  */
/* appear as the another Extra description keyword, prefixed     */
/* by the words 'item_for_' and followed by the player name.     */
/* The (keys) must all be stored in a room which is (virtually)  */
/* adjacent to the room of the lock smith.                       */

int pray_for_items(struct char_data* ch, int cmd, char* arg) {
  char buf[256];
  int key_room, gold;
  bool found;
  struct obj_data *tmp_obj, *obj;
  struct extra_descr_data* ext;

  if (cmd != 176) /* You must pray to get the stuff */
    return FALSE;

  key_room = 1 + ch->in_room;

  strcpy(buf, "item_for_");
  strcat(buf, GET_NAME(ch));

  gold = 0;
  found = FALSE;

  for (tmp_obj = real_roomp(key_room)->contents; tmp_obj;
       tmp_obj = tmp_obj->next_content)
    for (ext = tmp_obj->ex_description; ext; ext = ext->next)
      if (str_cmp(buf, ext->keyword) == 0) {
        if (gold == 0) {
          gold = 1;
          act("$n kneels and at the altar and chants a prayer to Odin.", FALSE,
            ch, 0, 0, TO_ROOM);
          act("You notice a faint light in Odin's eye.", FALSE, ch, 0, 0,
            TO_CHAR);
        }
        obj = read_object(tmp_obj->item_number, REAL);
        obj_to_room(obj, ch->in_room);
        act("$p slowly fades into existence.", FALSE, ch, obj, 0, TO_ROOM);
        act("$p slowly fades into existence.", FALSE, ch, obj, 0, TO_CHAR);
        gold += obj->obj_flags.cost;
        found = TRUE;
      }

  if (found) {
    GET_GOLD(ch) -= gold;
    GET_GOLD(ch) = MAX(0, GET_GOLD(ch));
    return TRUE;
  }

  return FALSE;
}

/* ********************************************************************
 *  Special procedures for objects                                     *
 ******************************************************************** */

#define CHAL_ACT \
  "You are torn out of reality!\n\r\
You roll and tumble through endless voids for what seems like eternity...\n\r\
\n\r\
After a time, a new reality comes into focus... you are elsewhere.\n\r"

int vorpal(Mob* victim, int cmd, char* arg, Obj* me) {
  struct char_data* ch;
  int exp, vhit, num;
  int percent;
  char buf[1024];

  if (cmd != OBJECT_HITTING)
    return FALSE;

  ch = me->equipped_by;

  vhit = GET_HIT(victim);

  percent = 300 * (((float)GET_HIT(victim)) / ((float)GET_MAX_HIT(victim)));
  if (percent < 1)
    percent = 2;

  num = number(1, percent);

  if ((num == 1) || (GET_HIT(victim) < 50)) {
    if (IS_NPC(victim) || victim->desc)
      if (IS_AFFECTED(ch, AFF_GROUP)) {
        group_gain(ch, victim);
      } else {
        if (IS_NPC(ch))
          exp += (exp * MIN(4, (GetMaxLevel(victim) - GetMaxLevel(ch)))) >> 3;
        else
          exp += (exp * MIN(8, (GetMaxLevel(victim) - GetMaxLevel(ch)))) >> 3;
        exp = MAX(exp, 1);

        if ((!IS_MOB(ch)) && (!IS_MOB(victim)))
          exp = 1;

        gain_exp(ch, exp);
        change_alignment(ch, victim);
      }

    act("$n cuts off $N's head! \n$N is dead! R.I.P.", FALSE, ch, 0, victim,
      TO_ROOM);
    act("You completely behead $N! \n$N is dead! R.I.P.", FALSE, ch, 0, victim,
      TO_CHAR);
    GET_HIT(victim) = 1;
    make_head(victim);
    die(victim);
    return TRUE;
  }
  return FALSE;
}

#if 0

int chalice(struct char_data *ch, int cmd, char *arg)
{


  struct obj_data *chalice;
  char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
  static int chl = -1, achl = -1;

  if (chl < 1)
    {
      chl = real_object(222);
      achl = real_object(223);
    }

  switch(cmd)
    {
    case 10:    /* get */
      if (!(chalice = get_obj_in_list_num(chl,
					  real_roomp(ch->in_room)->contents))
	  && CAN_SEE_OBJ(ch, chalice))
	if (!(chalice = get_obj_in_list_num(achl,
					    real_roomp(ch->in_room)->contents)) && CAN_SEE_OBJ(ch, chalice))
	  return(0);

      /* we found a chalice.. now try to get us */
      do_get(ch, arg, cmd);
      /* if got the altar one, switch her */
      if (chalice == get_obj_in_list_num(achl, ch->carrying))
	{
	  extract_obj(chalice);
	  chalice = read_object(chl, VIRTUAL);
	  obj_to_char(chalice, ch);
	}
      return(1);
      break;
    case 67: /* put */
      if (!(chalice = get_obj_in_list_num(chl, ch->carrying)))
	return(0);

      argument_interpreter(arg, buf1, buf2);
      if (!str_cmp(buf1, "chalice") && !str_cmp(buf2, "altar"))
	{
	  extract_obj(chalice);
	  chalice = read_object(achl, VIRTUAL);
	  obj_to_room(chalice, ch->in_room);
	  send_to_char("Ok.\n\r", ch);
	}
      return(1);
      break;
    case 176: /* pray */
      if (!(chalice = get_obj_in_list_num(achl,
					  real_roomp(ch->in_room)->contents)))
	return(0);

      do_action(ch, arg, cmd);  /* pray */
      send_to_char(CHAL_ACT, ch);
      extract_obj(chalice);
      act("$n is torn out of existence!", TRUE, ch, 0, 0, TO_ROOM);
      char_from_room(ch);
      char_to_room(ch, 2500);   /* before the fiery gates */
      do_look(ch, "", 15);
      return(1);
      break;
    default:
      return(0);
      break;
    }
}



int kings_hall(struct char_data *ch, int cmd, char *arg)
{
  if (cmd != 176)
    return(0);

  do_action(ch, arg, 176);

  send_to_char("You feel as if some mighty force has been offended.\n\r", ch);
  send_to_char(CHAL_ACT, ch);
  act("$n is struck by an intense beam of light and vanishes.",
      TRUE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, 1420);  /* behind the altar */
  do_look(ch, "", 15);
  return(1);
}

#endif

/*
**  donation room
*/
int Donation(struct char_data* ch, int cmd, char* arg) {
  char check[40], *tmp;

  if ((cmd != 10) && (cmd != 167)) {
    return (FALSE);
  }

  tmp = one_argument(arg, check);

  if (*check) {
    if (strncmp(check, "all", 3) == 0) {
      send_to_char("Now now, that would be greedy!\n\r", ch);
      return (TRUE);
    }
  }
  return (FALSE);
}

int hospital(struct char_data* ch, int cmd, char* arg) {
  char buf[MAX_STRING_LENGTH];
  int k, opt, cost;
  if (IS_NPC(ch))
    return (FALSE);
  cost = 6000 * GetMaxLevel(ch);
  if (cmd == 59) {
    send_to_char("1 - Healing of the Physical Self\n\r", ch);
    send_to_char("2 - Healing of the Mind\n\r", ch);
    sprintf(buf, "Any of these for %d coins.\n\r", cost);
    send_to_char(buf, ch);
    return (TRUE);
  } else if (cmd == 56) { /* Buy */
    arg = one_argument(arg, buf);
    opt = atoi(buf);
    if (cost > GET_GOLD(ch)) {
      send_to_char("Sorry, no medicare, medicaid or insurance allowed.\n\r",
        ch);
      return FALSE;
    }

    if ((opt >= 1) && (opt <= 2)) {
      GET_GOLD(ch) -= cost;
      switch (opt) {
        case 1:
          GET_HIT(ch) = hit_limit(ch);
          send_to_char("You are HEALED my child!\n\r", ch);
          update_pos(ch);
          WAIT_STATE(ch, 6 * PULSE_VIOLENCE);
          break;
        case 2:
          GET_MANA(ch) = GET_MAX_MANA(ch);
          send_to_char("Your mind is filled with great thoughts.\n\r", ch);
          update_pos(ch);
          WAIT_STATE(ch, 6 * PULSE_VIOLENCE);
          break;
      }
    } else {
      send_to_char("That's not available at THIS hospital!\n\r", ch);
      return FALSE;
    }
    return (TRUE);
  }
  return (FALSE);
}

int hospital_entrance(struct char_data* ch, int cmd, char* arg) {
  char buf[100];
  struct room_data* rm = 0;
  struct room_data* rp = 0;

  if (cmd != 1)
    return (FALSE);

  rm = real_roomp(3198);
  rp = real_roomp(3196);
  if (((MobCountInRoom(rm->people)) + (MobCountInRoom(rp->people))) > 8) {
    send_to_char("The hospital is full. Sorry.\n\r", ch);
    return (TRUE);
  } else {
    return (FALSE);
  }
}

int board_room_entrance(struct char_data* ch, int cmd, char* arg) {
  char buf[100];
  struct room_data* rm = 0;
  struct room_data* rp = 0;

  rm = real_roomp(2998);
  rp = real_roomp(2997);

  if (cmd == 2) {
    if ((MobCountInRoom(rm->people)) >= 1) {
      send_to_char("Someone is in the writing room already.\n\r", ch);
      return (TRUE);
    }
  } else if (cmd == 4) {
    if ((MobCountInRoom(rp->people)) >= 1) {
      send_to_char("Someone is in the reading room already.\n\r", ch);
      return (TRUE);
    }
  }
  return (FALSE);
}

int mirror_room(struct char_data* ch, int cmd, char* arg) {
  char buf[100], name[100];
  int to_room;

  if (cmd != 7)
    return (FALSE);

  if (ch->in_room == 100)
    to_room = 11301;
  else if (ch->in_room == 11301)
    to_room = 100;

  one_argument(arg, name);

  if (*name) {
    if (is_abbrev(name, "mirror")) {
      act("$n steps into the mirror and is tossed through!", FALSE, ch, 0, 0,
        TO_ROOM);
      char_from_room(ch);
      char_to_room(ch, to_room);
      act("$n arrives in the room through the magic mirror!", FALSE, ch, 0, 0,
        TO_ROOM);
      act("You step through the mirror and are tossed out the other side!",
        FALSE, ch, 0, 0, TO_CHAR);
      do_look(ch, "", 15);
      return (TRUE);
    }
    return (FALSE);
  }
  return (FALSE);
}

/*
  house routine for saved items.
*/

int House(struct char_data* ch, int cmd, char* arg) {
  char buf[100];
  struct char_data* mob;
  struct obj_cost cost;
  int i;
  int count = 0;

  if (IS_NPC(ch))
    return (FALSE);

  if ((cmd != 22) && (cmd != 92))
    return (FALSE);

  if (strncmp(GET_NAME(ch), real_roomp(ch->in_room)->name,
        strlen(GET_NAME(ch)))) {
    send_to_char("Sorry, you'll have to find your own house.\n\r", ch);
    return (FALSE);
  }
  if (cmd == 22) {
    mob = read_mobile(3005, VIRTUAL);
    char_to_room(mob, ch->in_room);
    return (TRUE);
  }
  return (FALSE);
}

static void FighterMove(struct char_data* ch) {
  int num;

  if (!ch->skills)
    SpaceForSkills(ch);
  num = number(1, 4);
  if (num <= 2) {
    if (!ch->skills[SKILL_BASH].learned)
      ch->skills[SKILL_BASH].learned = 10 + GetMaxLevel(ch) * 4;
    do_bash(ch, GET_NAME(ch->specials.fighting), 0);
  } else if (num == 3) {
    if (ch->equipment[WIELD]) {
      if (!ch->skills[SKILL_DISARM].learned)
        ch->skills[SKILL_DISARM].learned = 10 + GetMaxLevel(ch) * 4;
      do_disarm(ch, GET_NAME(ch->specials.fighting), 0);
    } else {
      if (!ch->skills[SKILL_DISARM].learned)
        ch->skills[SKILL_DISARM].learned = 60 + GetMaxLevel(ch) * 4;
      do_disarm(ch, GET_NAME(ch->specials.fighting), 0);
    }
  } else {
    if (!ch->skills[SKILL_KICK].learned)
      ch->skills[SKILL_KICK].learned = 10 + GetMaxLevel(ch) * 4;
    do_kick(ch, GET_NAME(ch->specials.fighting), 0);
  }
}

/***********************************************************************

         CHESSBOARD PROCS

 ***********************************************************************/

#define SISYPHUS_MAX_LEVEL 9

/* This is the highest level of PC that can enter.  The highest level
   monster currently in the section is 14th.  It should require a fairly
   large party to sweep the section. */

int sisyphus(struct char_data* ch, int cmd, char* arg) {
  if (cmd) {
    if (cmd <= 6 && cmd >= 1 && !IS_NPC(ch)) {
      send_to_char("Sisyphus looks at you\n\r", ch);
      if ((ch->in_room == Ivory_Gate) && (cmd == 4)) {
        if ((SISYPHUS_MAX_LEVEL < GetMaxLevel(ch)) &&
            (GetMaxLevel(ch) < LOW_IMMORTAL)) {
          if (!check_soundproof(ch)) {
            act("Sisyphus tells you 'First you'll have to get past me.'", TRUE,
              ch, 0, 0, TO_CHAR);
          }
          act("Sisyphus grins evilly.", TRUE, ch, 0, 0, TO_CHAR);
          return (TRUE);
        }
      }
      return (FALSE);
    } /* cmd 1 - 6 */
    return (FALSE);
  } else {
    if (ch->specials.fighting) {
      if ((GET_POS(ch) < POSITION_FIGHTING) &&
          (GET_POS(ch) > POSITION_STUNNED)) {
        StandUp(ch);
      } else {
        FighterMove(ch);
      }
      return (FALSE);
    }
  }
  return (FALSE);
} /* end sisyphus */

int jabberwocky(struct char_data* ch, int cmd, char* arg) {
  if (cmd)
    return (FALSE);

  if (ch->specials.fighting) {
    if ((GET_POS(ch) < POSITION_FIGHTING) && (GET_POS(ch) > POSITION_STUNNED)) {
      StandUp(ch);
    } else {
      FighterMove(ch);
    }
    return (FALSE);
  }
}

int flame(struct char_data* ch, int cmd, char* arg) {
  if (cmd)
    return (FALSE);
  if (ch->specials.fighting) {
    if ((GET_POS(ch) < POSITION_FIGHTING) && (GET_POS(ch) > POSITION_STUNNED)) {
      StandUp(ch);
    } else {
      FighterMove(ch);
    }
    return (FALSE);
  }
}

int banana(struct char_data* ch, int cmd, char* arg) {
  if (!cmd)
    return (FALSE);

  if ((cmd >= 1) && (cmd <= 6) && (GET_POS(ch) == POSITION_STANDING) &&
      (!IS_NPC(ch))) {
    if (!saves_spell(ch, SAVING_PARA)) {
      act("$N tries to leave, but slips on a banana and falls.", TRUE, ch, 0,
        ch, TO_NOTVICT);
      act("As you try to leave, you slip on a banana.", TRUE, ch, 0, ch,
        TO_VICT);
      GET_POS(ch) = POSITION_SITTING;
      return (TRUE); /* stuck */
    }
    return (FALSE); /* he got away */
  }
  return (FALSE);
}

int paramedics(struct char_data* ch, int cmd, char* arg) {
  struct char_data *vict, *most_hurt;

  if (!cmd) {
    if (ch->specials.fighting) {
      return (cleric(ch, 0, ""));
    } else {
      if (GET_POS(ch) == POSITION_STANDING) {
        /* Find a dude to do good things upon ! */

        most_hurt = real_roomp(ch->in_room)->people;
        for (vict = real_roomp(ch->in_room)->people; vict;
             vict = vict->next_in_room) {
          if (((float)GET_HIT(vict) / (float)hit_limit(vict) <
                (float)GET_HIT(most_hurt) / (float)hit_limit(most_hurt)) &&
              (CAN_SEE(ch, vict)))
            most_hurt = vict;
        }
        if (!most_hurt)
          return (FALSE); /* nobody here */

        if ((float)GET_HIT(most_hurt) / (float)hit_limit(most_hurt) > 0.66) {
          if (number(0, 5) == 0) {
            act("$n shrugs helplessly in unison.", 1, ch, 0, 0, TO_ROOM);
          }
          return TRUE; /* not hurt enough */
        }

        if (!check_soundproof(ch)) {
          if (number(0, 4) == 0) {
            if (most_hurt != ch) {
              act("$n looks at $N.", 1, ch, 0, most_hurt, TO_NOTVICT);
              act("$n looks at you.", 1, ch, 0, most_hurt, TO_VICT);
            }
            act("$n utters the words 'judicandus dies' in unison.", 1, ch, 0, 0,
              TO_ROOM);
            cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
              most_hurt, 0);
            return (TRUE);
          }
        }
      } else { /* I'm asleep or sitting */
        return (FALSE);
      }
    }
  }
  return (FALSE);
}

static char* elf_comm[] = {"wake", "yawn", "stand", "say Well, back to work.",
  "get all", "eat bread", "wink", "w", "w", "s", "s", "s", "d", "open gate",
  "e",                                             /* home to gate*/
  "close gate", "e", "e", "e", "e", "n", "w", "n", /* gate to baker */
  "give all.bread baker",                          /* pretend to give a bread */
  "give all.pastry baker", /* pretend to give a pastry */
  "say That'll be 33 coins, please.",
  "echo The baker gives some coins to the Elf", "wave", "s", "e", "n", "n", "e",
  "drop all.bread", "drop all.pastry", "w", "s", "s", /* to main square */
  "s", "w", "w", "w", "w",                            /* back to gate */
  "pat sisyphus", "open gate", "w", "close gate", "u", "n", "n", "n", "e",
  "e", /* to home */
  "say Whew, I'm exhausted.", "rest", "$"};

int delivery_elf(struct char_data* ch, int cmd, char* arg) {
#define ELF_INIT 0
#define ELF_RESTING 1
#define ELF_GETTING 2
#define ELF_DELIVERY 3
#define ELF_DUMP 4
#define ELF_RETURN_TOWER 5
#define ELF_RETURN_HOME 6

  if (cmd)
    return (FALSE);

  if (ch->specials.fighting)
    return FALSE;

  if (!ch->act_ptr)
    ch->act_ptr = (int*)calloc(1, sizeof(int));
  switch ((*((int*)ch->act_ptr))) {
    case ELF_INIT:
      if (ch->in_room == 0) {
        /* he has been banished to the Void */
      } else if (ch->in_room != Elf_Home) {
        if (GET_POS(ch) == POSITION_SLEEPING) {
          do_wake(ch, "", 0);
          do_stand(ch, "", 0);
        }
        do_say(ch, "Woah! How did i get here!", 0);
        do_emote(ch, "waves his arm, and vanishes!", 0);
        char_from_room(ch);
        char_to_room(ch, Elf_Home);
        do_emote(ch, "arrives with a Bamf!", 0);
        do_emote(ch, "yawns", 0);
        do_sleep(ch, "", 0);
        (*((int*)ch->act_ptr)) = ELF_RESTING;
      } else {
        (*((int*)ch->act_ptr)) = ELF_RESTING;
      }
      return (FALSE);
      break;
    case ELF_RESTING: {
      if ((time_info.hours > 6) && (time_info.hours < 9)) {
        do_wake(ch, "", 0);
        do_stand(ch, "", 0);
        (*((int*)ch->act_ptr)) = ELF_GETTING;
      }
      return (FALSE);
    } break;

    case ELF_GETTING: {
      do_get(ch, "all.loaf", 0);
      do_get(ch, "all.biscuit", 0);
      (*((int*)ch->act_ptr)) = ELF_DELIVERY;
      return (FALSE);
    } break;
    case ELF_DELIVERY: {
      if (ch->in_room != Bakery) {
        int dir;
        dir = choose_exit_global(ch->in_room, Bakery, -100);
        if (dir < 0) {
          (*((int*)ch->act_ptr)) = ELF_INIT;
          return (FALSE);
        } else {
          go_direction(ch, dir);
        }
      } else {
        do_give(ch, "6*biscuit baker", 0);
        do_give(ch, "6*loaf baker", 0);
        do_say(ch, "That'll be 33 coins, please.", 0);
        (*((int*)ch->act_ptr)) = ELF_DUMP;
      }
      return (FALSE);
    } break;
    case ELF_DUMP: {
      if (ch->in_room != Dump) {
        int dir;
        dir = choose_exit_global(ch->in_room, Dump, -100);
        if (dir < 0) {
          (*((int*)ch->act_ptr)) = ELF_INIT;
          return (FALSE);
        } else {
          go_direction(ch, dir);
        }
      } else {
        do_drop(ch, "10*biscuit", 0);
        do_drop(ch, "10*loaf", 0);
        (*((int*)ch->act_ptr)) = ELF_RETURN_TOWER;
      }
      return (FALSE);
    } break;
    case ELF_RETURN_TOWER: {
      if (ch->in_room != Ivory_Gate) {
        int dir;
        dir = choose_exit_global(ch->in_room, Ivory_Gate, -200);
        if (dir < 0) {
          (*((int*)ch->act_ptr)) = ELF_INIT;
          return (FALSE);
        } else {
          go_direction(ch, dir);
        }
      } else {
        (*((int*)ch->act_ptr)) = ELF_RETURN_HOME;
      }
      return (FALSE);
    } break;
    case ELF_RETURN_HOME:
      if (ch->in_room != Elf_Home) {
        int dir;
        dir = choose_exit_global(ch->in_room, Elf_Home, -200);
        if (dir < 0) {
          (*((int*)ch->act_ptr)) = ELF_INIT;
          return (FALSE);
        } else {
          go_direction(ch, dir);
        }
      } else {
        if (time_info.hours > 21) {
          do_say(ch, "Done at last!", 0);
          do_sleep(ch, "", 0);
          (*((int*)ch->act_ptr)) = ELF_RESTING;
        } else {
          do_say(ch, "An elf's work is never done.", 0);
          (*((int*)ch->act_ptr)) = ELF_GETTING;
        }
      }
      return (FALSE);
      break;
    default:
      (*((int*)ch->act_ptr)) = ELF_INIT;
      return (FALSE);
  }
}

int delivery_beast(struct char_data* ch, int cmd, char* arg) {
  struct obj_data* o;

  if (cmd)
    return (FALSE);

  if (time_info.hours == 6) {
    do_drop(ch, "all.loaf", 0);
    do_drop(ch, "all.biscuit", 0);
  } else if (time_info.hours < 2) {
    if (number(0, 1)) {
      o = read_object(3012, VIRTUAL);
      obj_to_char(o, ch);
    } else {
      o = read_object(3013, VIRTUAL);
      obj_to_char(o, ch);
    }
  } else {
    if (GET_POS(ch) > POSITION_SLEEPING) {
      do_sleep(ch, "", 0);
    }
  }
}

int Keftab(struct char_data* ch, int cmd, char* arg) {
  int found, targ_item;
  struct char_data* i;

  if (cmd)
    return (FALSE);

  if (!ch->specials.hunting) {
    /* find a victim */

    for (i = character_list; i; i = i->next) {
      if (!IS_NPC(ch)) {
        targ_item = SWORD_ANCIENTS;
        found = FALSE;
        while (!found) {
          if ((HasObject(i, targ_item)) && (GetMaxLevel(i) < 30)) {
            AddHated(ch, i);
            SetHunting(ch, i);
            return (TRUE);
          } else {
            targ_item++;
            if (targ_item > SWORD_ANCIENTS + 20)
              found = TRUE;
          }
        }
      }
    }
    return (FALSE);
  } else {
    /* check to make sure that the victim still has an item */
    found = FALSE;
    targ_item = SWORD_ANCIENTS;
    while (!found) {
      if (HasObject(ch->specials.hunting, targ_item)) {
        return (FALSE);
      } else {
        targ_item++;
        if (targ_item == SWORD_ANCIENTS + 20)
          found = FALSE;
      }
      ch->specials.hunting = 0;
      return (FALSE);
    }
  }
}

int StormGiant(struct char_data* ch, int cmd, char* arg) {
  struct char_data* vict;

  if (cmd)
    return (FALSE);

  if (ch->specials.fighting) {
    if ((GET_POS(ch) < POSITION_FIGHTING) && (GET_POS(ch) > POSITION_STUNNED)) {
      StandUp(ch);
    } else {
      if (number(0, 5)) {
        fighter(ch, cmd, arg);
      } else {
        act("$n creates a lightning bolt", TRUE, ch, 0, 0, TO_ROOM);
        if ((vict = FindAHatee(ch)) == NULL)
          vict = FindVictim(ch);
        if (!vict)
          return (FALSE);
        cast_lightning_bolt(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);

        /* do nothing */
      }
    }
    return (FALSE);
  }
}

int Manticore(struct char_data* ch, int cmd, char* arg) {}

int Kraken(struct char_data* ch, int cmd, char* arg) {}

static int GetDamBonus(struct obj_data* w) {
  int j, tot = 0;

  /* return the damage bonus from a weapon */
  for (j = 0; j < MAX_OBJ_AFFECT; j++) {
    if (w->affected[j].location == APPLY_DAMROLL ||
        w->affected[j].location == APPLY_HITNDAM) {
      tot += w->affected[j].modifier;
    }
  }
  return (tot);
}

static int GetDamage(struct obj_data* w, struct char_data* ch) {
  float ave;
  int num, size, iave;
  /*
    return the average damage of the weapon, with plusses.
  */

  ave = w->obj_flags.value[2] / 2.0 + 0.5;

  ave *= w->obj_flags.value[1];

  ave += GetDamBonus(w);
  /*
    check for immunity:
    */
  iave = ave;
  if (ch->specials.fighting) {
    iave = PreProcDam(ch->specials.fighting, ITEM_TYPE(w), iave);
    iave = WeaponCheck(ch, ch->specials.fighting, ITEM_TYPE(w), iave);
  }
  return (iave);
}

static int GetHandDamage(struct char_data* ch) {
  float ave;
  int num, size, iave;
  /*
    return the hand damage of the weapon, with plusses.
  dam += dice(ch->specials.damnodice, ch->specials.damsizedice);

    */

  num = ch->specials.damnodice;
  size = ch->specials.damsizedice;

  ave = size / 2.0 + 0.5;

  ave *= num;

  /*
    check for immunity:
    */
  iave = ave;
  if (ch->specials.fighting) {
    iave = PreProcDam(ch->specials.fighting, TYPE_HIT, iave);
    iave = WeaponCheck(ch, ch->specials.fighting, TYPE_HIT, iave);
  }
  return (iave);
}

static int is_weapon(struct obj_data* o) {
  return o->obj_flags.type_flag == ITEM_WEAPON;
}

static bool FindABetterWeapon(struct char_data* mob) {
  struct obj_data *o, *best;
  /*
    pick up and wield weapons
    Similar code for armor, etc.
    */

  /* check whether this mob can wield */
  if (!HasHands(mob))
    return (FALSE);

  if (!real_roomp(mob->in_room))
    return (FALSE);

  /* check room */
  best = 0;
  for (o = real_roomp(mob->in_room)->contents; o; o = o->next_content) {
    if (best && is_weapon(o)) {
      if (GetDamage(o, mob) > GetDamage(best, mob)) {
        best = o;
      }
    } else {
      if (is_weapon(o)) {
        best = o;
      }
    }
  }
  /* check inv */
  for (o = mob->carrying; o; o = o->next_content) {
    if (best && is_weapon(o)) {
      if (GetDamage(o, mob) > GetDamage(best, mob)) {
        best = o;
      }
    } else {
      if (is_weapon(o)) {
        best = o;
      }
    }
  }

  if (mob->equipment[WIELD]) {
    if (best) {
      if (GetDamage(mob->equipment[WIELD], mob) >= GetDamage(best, mob)) {
        best = mob->equipment[WIELD];
      }
    } else {
      best = mob->equipment[WIELD];
    }
  }

  if (best) {
    if (GetHandDamage(mob) > GetDamage(best, mob)) {
      best = 0;
    }
  } else {
    return (FALSE); /* nothing to choose from */
  }

  if (best) {
    /*
out with the old, in with the new
    */
    if (best->carried_by == mob) {
      if (mob->equipment[WIELD]) {
        do_remove(mob, mob->equipment[WIELD]->name, 0);
      }
      do_wield(mob, best->name, 0);
    } else if (best->equipped_by == mob) {
      /* do nothing */
      return (TRUE);
    } else {
      do_get(mob, best->name, 0);
    }
  } else {
    if (mob->equipment[WIELD]) {
      do_remove(mob, mob->equipment[WIELD]->name, 0);
    }
  }
}

int fighter(struct char_data* ch, int cmd, char* arg) {
  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    if (GET_POS(ch) == POSITION_FIGHTING) {
      FighterMove(ch);
    } else {
      StandUp(ch);
    }
    FindABetterWeapon(ch);
  }
}

int web_slinger(struct char_data* ch, int cmd, char* arg) {
  struct char_data* victim;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    victim = ch->specials.fighting;
    act("$n throws webs on you!", FALSE, ch, 0, victim, TO_VICT);
    act("$n throws webs on $N!", FALSE, ch, 0, victim, TO_NOTVICT);
    SET_BIT(victim->specials.affected_by, AFF_GRAPPLE);
  }
}

int juggernaut(struct char_data* ch, int cmd, char* arg) {
  struct char_data* victim;
  int dam;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    victim = ch->specials.fighting;
    act("$n charges at you, knocking you over.", FALSE, ch, 0, 0, TO_VICT);
    act("$n charges at $N, knocking them over.", FALSE, ch, 0, victim,
      TO_NOTVICT);

    damage(ch, victim, dice(2, 250), TYPE_HIT);
  }
}

void BlowChar(struct char_data* ch) {
  struct room_data* rp;
  int or ;
  int num;

  num = number(1, 20);

  rp = real_roomp(ch->in_room);
  char_from_room(ch);
  char_to_room(ch, (3001 + num));
  do_look(ch, "\0", 15);
}

void BouncerThrow(struct char_data* ch) {
  struct room_data *rp, *rp2;
  int or ;

  rp = real_roomp(ch->in_room);
  rp2 = real_roomp((ch->in_room) - 1);
  if (rp && rp2) {
    send_to_char(
      "The bouncer picks you up over his head and throws you toward the "
      "door.\n\r",
      ch);
    act("The bouncer picks up $n and hurls $m toward the door.", FALSE, ch, 0,
      0, TO_ROOM);
    or = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, (or -1));
    do_look(ch, "\0", 15);
  }
}

int prof_x(struct char_data* ch, int cmd, char* arg) {
  struct char_data* victim;
  struct affected_type af;
  int num;
  char buf[200];

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (!ch->specials.fighting)
    return (FALSE);

  num = number(1, 5);

  switch (num) {
    case 1: {
      act("$n says 'How dare you try to fight me?!?", 1, ch, 0, 0, TO_ROOM);
      break;
    }
    case 2: {
      act("$n says 'You are very stupid to attack me!!", 1, ch, 0, 0, TO_ROOM);
      break;
    }
    case 3: {
      act("$n says 'There is no way you will beat me!!", 1, ch, 0, 0, TO_ROOM);
      break;
    }
    case 4: {
      act("$n says 'I am the powerful Charles Xavier. How DARE you attack me?!",
        1, ch, 0, 0, TO_ROOM);
      break;
    }
    case 5: {
      act("$n says 'You have forced me to show you my powers!!", 1, ch, 0, 0,
        TO_ROOM);
      break;
    }
    default: {
      act("$n laughs hysterically.", 1, ch, 0, 0, TO_ROOM);
      break;
    }
  }

  act("$n waves his arms, and utters the words 'Muhahahahaha'", 1, ch, 0, 0,
    TO_ROOM);
  victim = ch->specials.fighting;
  if (!victim)
    return (FALSE);

  switch (num) {
    case 1: {
      send_to_char("Professor X tells you 'You are now stuck.'\n\r", victim);
      WAIT_STATE(victim, PULSE_VIOLENCE * 8);
      break;
    }
    case 2: {
      send_to_char("Professor X tells you 'You will now sleep.'\n\r", victim);
      stop_fighting(ch);
      stop_fighting(victim);

      af.type = SPELL_SLEEP;
      af.duration = 1;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_SLEEP;
      affect_join(victim, &af, FALSE, FALSE);

      if (GET_POS(victim) > POSITION_SLEEPING) {
        act("$N stops fighting and is put to sleep by $n!", 1, ch, 0, victim,
          TO_NOTVICT);
        act("You are knocked cold by $n!", 1, ch, 0, victim, TO_VICT);
        GET_POS(victim) = POSITION_SLEEPING;
      }
      break;
    }
    case 3: {
      send_to_char("You are REALLY stuck now!\n\r", victim);
      stop_fighting(ch);
      stop_fighting(victim);

      af.type = SPELL_PARALYSIS;
      af.duration = 1;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_PARALYSIS;
      affect_join(victim, &af, FALSE, FALSE);

      act("$N is frozen by $n!", 1, ch, 0, victim, TO_NOTVICT);
      act("You are frozen by $n!", 1, ch, 0, victim, TO_VICT);
      break;
    }
    case 4: {
      send_to_char("Professor X says 'Thou shalt not see!'\n\r", victim);

      af.type = SPELL_BLINDNESS;
      af.duration = 1;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_BLIND;
      affect_join(victim, &af, FALSE, FALSE);

      act("$N is blinded by $n!", 1, ch, 0, victim, TO_NOTVICT);
      break;
    }
    case 5: {
      send_to_char("This is your lucky day.\n\r", victim);
      break;
    }
    default: {
      return (FALSE);
    }
  }
}

int elektro(struct char_data* ch, int cmd, char* arg) {
  struct char_data *victim, *tmp_victim, *temp;
  int dam;
  char buf[200];

  if (cmd || !AWAKE(ch) || !ch->specials.fighting)
    return (FALSE);

  dam = dice(2, 200);

  act("$n says 'Get ready for a JOLT!'", 1, ch, 0, 0, TO_ROOM);
  for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next;
    if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
      if (IS_IMMORTAL(tmp_victim))
        return FALSE;
      MissileDamage(ch, tmp_victim, dam, SPELL_LIGHTNING_BOLT);
    }
  }
}

int iceman(struct char_data* ch, int cmd, char* arg) {
  struct char_data* victim;
  int dam;
  char buf[200];

  if (cmd || !AWAKE(ch) || !ch->specials.fighting)
    return (FALSE);

  dam = dice(100, 3);

  victim = FindVictim(ch);

  if (!victim)
    victim = ch->specials.fighting;

  if (!victim)
    return FALSE;

  sprintf(buf, "Iceman says 'Here comes some %sCOLD%s stuff!'\n\r", ANSI_WHITE,
    ANSI_NORMAL);
  send_to_room(buf, ch->in_room);
  MissileDamage(ch, victim, dam, SPELL_CONE_OF_COLD);
}

int cyclops(struct char_data* ch, int cmd, char* arg) {
  struct char_data *victim, *tmp_victim, *temp;
  int dam;
  char buf[200];
}

int storm(struct char_data* ch, int cmd, char* arg) {
  struct char_data *victim, *tmp_victim, *temp;
  int dam, percent, thrownum;
  char buf[200];

  percent = number(1, 10);

  if (cmd || !AWAKE(ch) || IS_AFFECTED(ch, AFF_PARALYSIS))
    return (FALSE);

  if (!ch->specials.fighting)
    return (FALSE);

  if ((GET_POS(ch) > POSITION_STUNNED) && (GET_POS(ch) < POSITION_FIGHTING)) {
    StandUp(ch);
    return (TRUE);
  }
  act("$n screams 'I summon the FULL power....of the STORM!'", 1, ch, 0, 0,
    TO_ROOM);

  switch (percent) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8: {
      dam = dice(1, 200);

      act("$n says 'Let it rain!'", 1, ch, 0, 0, TO_ROOM);
      for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
        temp = tmp_victim->next;
        if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
          send_to_char("You are hit by torrential rains!\n\r", tmp_victim);
          damage(ch, tmp_victim, dam, TYPE_HIT);
        }
      }
      break;
    }
    case 9:
    case 10: {
      act("$n waves her arms.", 1, ch, 0, 0, TO_ROOM);
      act("$n has summoned a great hurricane!", 1, ch, 0, 0, TO_ROOM);
      for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
           tmp_victim = tmp_victim->next_in_room) {
        if ((ch != tmp_victim) && IS_PC(tmp_victim)) {
          BlowChar(tmp_victim);
        }
      }
      break;
    }
    default:
      return (FALSE);
  }
}

/*
**  NEW THALOS MOBS:******************************************************
*/

#define NTMOFFICE 13554
#define NTMNGATE 3622
#define NTMEGATE 3631
#define NTMSGATE 3613
#define NTMWGATE 3623

#define NTMWMORN 0
#define NTMSTARTM 1
#define NTMGOALNM 2
#define NTMGOALEM 3
#define NTMGOALSM 4
#define NTMGOALWM 5
#define NTMGOALOM 6
#define NTMWNIGHT 7
#define NTMSTARTN 8
#define NTMGOALNN 9
#define NTMGOALEN 10
#define NTMGOALSN 11
#define NTMGOALWN 12
#define NTMGOALON 13
#define NTMSUSP 14
#define NTM_FIX 15

int NewThalosMayor(struct char_data* ch, int cmd, char* arg) {
  if (cmd || !AWAKE(ch))
    return (FALSE);
  if (!ch->act_ptr)
    ch->act_ptr = (int*)calloc(1, sizeof(int));
  if (ch->specials.fighting) {
    return (FALSE);
  } else {
    switch ((*((int*)ch->act_ptr))) { /* state info */
      case NTMWMORN:                  /* wait for morning */
        if (time_info.hours == 6) {
          (*((int*)ch->act_ptr)) = NTMGOALNM;
          return (FALSE);
        }
        break;
      case NTMGOALNM: /* north gate */ {
        if (ch->in_room != NTMNGATE) {
          int dir;
          dir = choose_exit_global(ch->in_room, NTMNGATE, -100);
          if (dir < 0) {
            (*((int*)ch->act_ptr)) = NTM_FIX;
            return (FALSE);
          } else {
            go_direction(ch, dir);
          }
        } else {
          /*
           * unlock and open door.
           */
          do_unlock(ch, " gate", 0);
          do_open(ch, " gate", 0);
          (*((int*)ch->act_ptr)) = NTMGOALEM;
        }
        return (FALSE);
      } break;
      case NTMGOALEM: {
        if (ch->in_room != NTMEGATE) {
          int dir;
          dir = choose_exit_global(ch->in_room, NTMEGATE, -100);
          if (dir < 0) {
            (*((int*)ch->act_ptr)) = NTM_FIX;
            return (FALSE);
          } else {
            go_direction(ch, dir);
          }
        } else {
          /*
           * unlock and open door.
           */
          do_unlock(ch, " gate", 0);
          do_open(ch, " gate", 0);
          (*((int*)ch->act_ptr)) = NTMGOALSM;
        }
        return (FALSE);
      }
      case NTMGOALSM: {
        if (ch->in_room != NTMSGATE) {
          int dir;
          dir = choose_exit_global(ch->in_room, NTMSGATE, -100);
          if (dir < 0) {
            (*((int*)ch->act_ptr)) = NTM_FIX;
            return (FALSE);
          } else {
            go_direction(ch, dir);
          }
        } else {
          /*
           * unlock and open door.
           */
          do_unlock(ch, " gate", 0);
          do_open(ch, " gate", 0);
          (*((int*)ch->act_ptr)) = NTMGOALWM;
        }
        return (FALSE);
      }
      case NTMGOALWM: {
        if (ch->in_room != NTMWGATE) {
          int dir;
          dir = choose_exit_global(ch->in_room, NTMWGATE, -100);
          if (dir < 0) {
            (*((int*)ch->act_ptr)) = NTM_FIX;
            return (FALSE);
          } else {
            go_direction(ch, dir);
          }
        } else {
          /*
           * unlock and open door.
           */
          do_unlock(ch, " gate", 0);
          do_open(ch, " gate", 0);
          (*((int*)ch->act_ptr)) = NTMGOALOM;
        }
        return (FALSE);
      }
      case NTMGOALOM: {
        if (ch->in_room != NTMOFFICE) {
          int dir;
          dir = choose_exit_global(ch->in_room, NTMOFFICE, -100);
          if (dir < 0) {
            (*((int*)ch->act_ptr)) = NTM_FIX;
            return (FALSE);
          } else {
            go_direction(ch, dir);
          }
        } else {
          (*((int*)ch->act_ptr)) = NTMWNIGHT;
        }
        return (FALSE);
      }
      case NTMWNIGHT: /* go back to wait for 7pm */
        if (time_info.hours == 19) {
          (*((int*)ch->act_ptr)) = NTMGOALNN;
        }
      case NTMGOALNN: /* north gate */ {
        if (ch->in_room != NTMNGATE) {
          int dir;
          dir = choose_exit_global(ch->in_room, NTMNGATE, -100);
          if (dir < 0) {
            (*((int*)ch->act_ptr)) = NTM_FIX;
            return (FALSE);
          } else {
            go_direction(ch, dir);
          }
        } else {
          /*
           * lock and open door.
           */
          do_lock(ch, " gate", 0);
          do_close(ch, " gate", 0);
          (*((int*)ch->act_ptr)) = NTMGOALEN;
        }
        return (FALSE);
      }
      case NTMGOALEN: {
        if (ch->in_room != NTMEGATE) {
          int dir;
          dir = choose_exit_global(ch->in_room, NTMEGATE, -100);
          if (dir < 0) {
            (*((int*)ch->act_ptr)) = NTM_FIX;
            return (FALSE);
          } else {
            go_direction(ch, dir);
          }
        } else {
          /*
           * lock and open door.
           */
          do_lock(ch, " gate", 0);
          do_close(ch, " gate", 0);
          (*((int*)ch->act_ptr)) = NTMGOALSN;
        }
        return (FALSE);
      }
      case NTMGOALSN: {
        if (ch->in_room != NTMSGATE) {
          int dir;
          dir = choose_exit_global(ch->in_room, NTMSGATE, -100);
          if (dir < 0) {
            (*((int*)ch->act_ptr)) = NTM_FIX;
            return (FALSE);
          } else {
            go_direction(ch, dir);
          }
        } else {
          /*
           * lock and open door.
           */
          do_lock(ch, " gate", 0);
          do_close(ch, " gate", 0);
          (*((int*)ch->act_ptr)) = NTMGOALWN;
        }
        return (FALSE);
      }
      case NTMGOALWN: {
        if (ch->in_room != NTMWGATE) {
          int dir;
          dir = choose_exit_global(ch->in_room, NTMWGATE, -100);
          if (dir < 0) {
            (*((int*)ch->act_ptr)) = NTM_FIX;
            return (FALSE);
          } else {
            go_direction(ch, dir);
          }
        } else {
          /*
           * unlock and open door.
           */
          do_lock(ch, " gate", 0);
          do_close(ch, " gate", 0);
          (*((int*)ch->act_ptr)) = NTMGOALOM;
        }
        return (FALSE);
      }
      case NTMGOALON: {
        if (ch->in_room != NTMOFFICE) {
          int dir;
          dir = choose_exit_global(ch->in_room, NTMOFFICE, -100);
          if (dir < 0) {
            (*((int*)ch->act_ptr)) = NTM_FIX;
            return (FALSE);
          } else {
            go_direction(ch, dir);
          }
        } else {
          (*((int*)ch->act_ptr)) = NTMWMORN;
        }
        return (FALSE);
        break;
      }
      case NTM_FIX: {
        /*
         * move to correct spot (office)
         */
        do_say(ch, "Woah! How did i get here!", 0);
        char_from_room(ch);
        char_to_room(ch, NTMOFFICE);
        (*((int*)ch->act_ptr)) = NTMWMORN;
        return (FALSE);
        break;
      }
      default: {
        (*((int*)ch->act_ptr)) = NTM_FIX;
        return (FALSE);
        break;
      }
    }
  }
}

int SultanGuard(struct char_data* ch, int cmd, char* arg) {
  struct char_data *tch, *evil, *i;
  int max_evil, lev;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    fighter(ch, cmd, arg);

    if (!check_soundproof(ch)) {
      if (number(0, 20) == 0) {
        do_shout(ch, "To me, my fellows! I am in need of thy aid!", 0);
      } else {
        act("$n shouts 'To me, my fellows! I need thy aid!'", TRUE, ch, 0, 0,
          TO_ROOM);
      }

      if (ch->specials.fighting)
        CallForGuard(ch, ch->specials.fighting, 3, NEWTHALOS);

      return (TRUE);
    }
  }

  max_evil = 1000;
  evil = 0;

  if (check_peaceful(ch, ""))
    return FALSE;

  for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if (tch->specials.fighting) {
      if ((GET_ALIGNMENT(tch) < max_evil) &&
          (IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
        max_evil = GET_ALIGNMENT(tch);
        evil = tch;
      }
    }
  }

  if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0)) {
    if (!check_soundproof(ch)) {
      act("$n screams 'PROTECT THE INNOCENT! BANZAI!!! CHARGE!!! SPOON!'",
        FALSE, ch, 0, 0, TO_ROOM);
    }
    hit(ch, evil, TYPE_UNDEFINED);
    return (TRUE);
  }

  return (FALSE);
}

int NewThalosCitizen(struct char_data* ch, int cmd, char* arg) {
  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    fighter(ch, cmd, arg);

    if (!check_soundproof(ch)) {
      if (number(0, 18) == 0) {
        do_shout(ch, "Guards! Help me! Please!", 0);
      } else {
        act("$n shouts 'Guards!  Help me! Please!'", TRUE, ch, 0, 0, TO_ROOM);
      }

      if (ch->specials.fighting)
        CallForGuard(ch, ch->specials.fighting, 3, NEWTHALOS);

      return (TRUE);
    }
  } else {
    return (FALSE);
  }
}

int NewThalosGuildGuard(struct char_data* ch, int cmd, char* arg) {
  if (!cmd) {
    if (ch->specials.fighting) {
      return (fighter(ch, cmd, arg));
    }
  } else {
    if (cmd >= 1 && cmd <= 6) {
      switch (ch->in_room) {
        case 13532:
          return (CheckForBlockedMove(ch, cmd, arg, 13532, 2, CLASS_THIEF));
          break;
        case 13512:
          return (CheckForBlockedMove(ch, cmd, arg, 13512, 2, CLASS_CLERIC));
          break;
        case 13526:
          return (CheckForBlockedMove(ch, cmd, arg, 13526, 2, CLASS_WARRIOR));
          break;
        case 13525:
          return (
            CheckForBlockedMove(ch, cmd, arg, 13525, 0, CLASS_MAGIC_USER));
          break;
      }
    }
  }
  return (FALSE);
}

/*
New improved magic_user
*/

int magic_user2(struct char_data* ch, int cmd, char* arg) {
  struct char_data* vict;
  byte lspell;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (!ch->specials.fighting)
    return FALSE;

  if ((GET_POS(ch) > POSITION_STUNNED) && (GET_POS(ch) < POSITION_FIGHTING)) {
    StandUp(ch);
    return (TRUE);
  }

  vict = FindVictim(ch);

  if (!vict)
    vict = ch->specials.fighting;

  if (!vict)
    return (FALSE);

  lspell = number(0, GetMaxLevel(ch)); /* gen number from 0 to level */

  if (lspell < 1)
    lspell = 1;

  switch (lspell) {
    case 1:
      act("$n utters the words 'Magic Missile'.", 1, ch, 0, 0, TO_ROOM);
      cast_magic_missile(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 2:
      act("$n utters the words 'Shocking Grasp'.", 1, ch, 0, 0, TO_ROOM);
      cast_shocking_grasp(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 3:
    case 4:
      act("$n utters the words 'Chill Touch'.", 1, ch, 0, 0, TO_ROOM);
      cast_chill_touch(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 5:
      act("$n utters the words 'Burning Hands'.", 1, ch, 0, 0, TO_ROOM);
      cast_burning_hands(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 6:
      if (!IS_AFFECTED(vict, AFF_SANCTUARY)) {
        act("$n utters the words 'Dispel Magic'.", 1, ch, 0, 0, TO_ROOM);
        cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      } else {
        act("$n utters the words 'Chill Touch'.", 1, ch, 0, 0, TO_ROOM);
        cast_chill_touch(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      }
      break;
    case 7:
      act("$n utters the words 'Ice Storm'.", 1, ch, 0, 0, TO_ROOM);
      cast_ice_storm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 8:
      act("$n utters the words 'Blindness'.", 1, ch, 0, 0, TO_ROOM);
      cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 9:
      act("$n utters the words 'Fear'.", 1, ch, 0, 0, TO_ROOM);
      cast_fear(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 10:
    case 11:
      act("$n utters the words 'Lightning Bolt'.", 1, ch, 0, 0, TO_ROOM);
      cast_lightning_bolt(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 12:
    case 13:
      act("$n utters the words 'Color Spray'.", 1, ch, 0, 0, TO_ROOM);
      cast_colour_spray(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 14:
      act("$n utters the words 'Cone Of Cold'.", 1, ch, 0, 0, TO_ROOM);
      cast_cone_of_cold(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
      act("$n utters the words 'Fireball'.", 1, ch, 0, 0, TO_ROOM);
      cast_fireball(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;

    default:
      act("$n utters the words 'frag'.", 1, ch, 0, 0, TO_ROOM);
      cast_meteor_swarm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
  }

  return TRUE;
}

/******************Mordilnia citizens************************************/

int MordGuard(struct char_data* ch, int cmd, char* arg) {
  struct char_data *tch, *evil, *i;
  int max_evil, lev;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    fighter(ch, cmd, arg);

    if (!check_soundproof(ch)) {
      if (number(0, 20) == 0) {
        do_shout(ch, "To me, my fellows! I am in need of thy aid!", 0);
      } else {
        act("$n shouts 'To me, my fellows! I need thy aid!'", TRUE, ch, 0, 0,
          TO_ROOM);
      }

      if (ch->specials.fighting)
        CallForGuard(ch, ch->specials.fighting, 3, MORDILNIA);

      return (TRUE);
    }
  }

  max_evil = 1000;
  evil = 0;

  if (check_peaceful(ch, ""))
    return FALSE;

  for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if (tch->specials.fighting) {
      if ((GET_ALIGNMENT(tch) < max_evil) &&
          (IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
        max_evil = GET_ALIGNMENT(tch);
        evil = tch;
      }
    }
  }

  if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0)) {
    if (!check_soundproof(ch)) {
      act("$n screams 'PROTECT THE INNOCENT! BANZAI!!! CHARGE!!! SPOON!'",
        FALSE, ch, 0, 0, TO_ROOM);
    }
    hit(ch, evil, TYPE_UNDEFINED);
    return (TRUE);
  }

  return (FALSE);
}

int MordCitizen(struct char_data* ch, int cmd, char* arg) {
  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    fighter(ch, cmd, arg);

    if (!check_soundproof(ch)) {
      if (number(0, 18) == 0) {
        do_shout(ch, "Guards! Help me! Please!", 0);
      } else {
        act("$n shouts 'Guards!  Help me! Please!'", TRUE, ch, 0, 0, TO_ROOM);
      }

      if (ch->specials.fighting)
        CallForGuard(ch, ch->specials.fighting, 3, MORDILNIA);

      return (TRUE);
    }
  } else {
    return (FALSE);
  }
}

int MordGuildGuard(struct char_data* ch, int cmd, char* arg) {
  if (!cmd) {
    if (ch->specials.fighting) {
      return (fighter(ch, cmd, arg));
    }
  } else {
    if (cmd >= 1 && cmd <= 6) {
      switch (ch->in_room) {
        case 18266:
          return (
            CheckForBlockedMove(ch, cmd, arg, 18266, 2, CLASS_MAGIC_USER));
          break;
        case 18276:
          return (CheckForBlockedMove(ch, cmd, arg, 18276, 2, CLASS_CLERIC));
          break;
        case 18272:
          return (CheckForBlockedMove(ch, cmd, arg, 18272, 2, CLASS_THIEF));
          break;
        case 18256:
          return (CheckForBlockedMove(ch, cmd, arg, 18256, 0, CLASS_WARRIOR));
          break;
      }
    } else {
      return (FALSE);
    }
  }
  return (FALSE);
}

int Devil(struct char_data* ch, int cmd, char* arg) {
  return (magic_user(ch, cmd, arg));
}

int Demon(struct char_data* ch, int cmd, char* arg) {
  return (magic_user(ch, cmd, arg));
}

int CaravanGuildGuard(struct char_data* ch, int cmd, char* arg) {
  if (!cmd) {
    if (ch->specials.fighting) {
      return (fighter(ch, cmd, arg));
    }
  } else {
    if (cmd >= 1 && cmd <= 6) {
      switch (ch->in_room) {
        case 16115:
          return (
            CheckForBlockedMove(ch, cmd, arg, 16115, 1, CLASS_MAGIC_USER));
          break;
        case 16126:
          return (CheckForBlockedMove(ch, cmd, arg, 16116, 1, CLASS_CLERIC));
          break;
        case 16117:
          return (CheckForBlockedMove(ch, cmd, arg, 16117, 3, CLASS_THIEF));
          break;
        case 16110:
          return (CheckForBlockedMove(ch, cmd, arg, 16110, 3, CLASS_WARRIOR));
          break;
      }
    } else {
      return (FALSE);
    }
  }
  return (FALSE);
}

int StatTeller(struct char_data* ch, int cmd, char* arg) {
  int choice;
  char buf[200];

  if (cmd) {
    if (cmd == 56) { /* buy */

      /*
      ** randomly tells a player 3 of his/her stats.. for a price
      */
      if (GET_GOLD(ch) < 100000) {
        send_to_char("You do not have the money to pay me.\n\r", ch);
        return (TRUE);
      } else {
        GET_GOLD(ch) -= 100000;
      }

      choice = number(0, 2);
      switch (choice) {
        case 0:
          sprintf(buf, "STR: %d, WIS: %d, DEX: %d\n\r", GET_STR(ch),
            GET_WIS(ch), GET_DEX(ch));
          send_to_char(buf, ch);
          break;
        case 1:
          sprintf(buf, "INT: %d, DEX:  %d, CON: %d \n\r", GET_INT(ch),
            GET_DEX(ch), GET_CON(ch));
          send_to_char(buf, ch);
          break;
        case 2:
          sprintf(buf, "CON: %d, INT: %d , WIS: %d\n\r", GET_CON(ch),
            GET_INT(ch), GET_WIS(ch));
          send_to_char(buf, ch);
          break;
        default:
          send_to_char("We are experiencing Technical difficulties\n\r", ch);
          return (TRUE);
      }

    } else {
      return (FALSE);
    }
  } else {
    /*
    **  in combat, issues a more potent curse.
    */

    if (ch->specials.fighting) {
      act("$n gives you the evil eye!  You feel your hitpoints ebbing away",
        FALSE, ch, 0, ch->specials.fighting, TO_VICT);
      act("$n gives $N the evil eye!  $N seems weaker!", FALSE, ch, 0,
        ch->specials.fighting, TO_NOTVICT);
      ch->specials.fighting->points.max_hit -= 10;
      ch->specials.fighting->points.hit -= 10;
      return (FALSE);
    }
  }
  return (FALSE);
}

void ThrowChar(struct char_data* ch, struct char_data* v, int dir) {
  struct room_data* rp;
  int or, dam;
  char buf[200];

  rp = real_roomp(v->in_room);
  if (rp && rp->dir_option[dir] && rp->dir_option[dir]->to_room &&
      (EXIT(v, dir)->to_room != NOWHERE)) {
    if (v->specials.fighting) {
      send_to_char("Not while fighting!\n\r", ch);
      return;
    }
    sprintf(buf, "You pick up %s and throw them %s\n\r",
      (IS_NPC(v) ? v->player.short_descr : GET_NAME(v)), dirs[dir]);
    send_to_char(buf, ch);
    sprintf(buf, "%s picks you up and throws you %s\n\r",
      (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), dirs[dir]);
    send_to_char(buf, v);
    act("$N is thrown out of the room by $n.\n\r", TRUE, ch, 0, v, TO_NOTVICT);
    or = v->in_room;
    char_from_room(v);
    char_to_room(v, (real_roomp(or))->dir_option[dir]->to_room);
    do_look(v, "\0", 15);
    WAIT_STATE(v, PULSE_VIOLENCE);
  } else {
    sprintf(buf, "You slam %s into the wall!\n\r",
      (IS_NPC(v) ? v->player.short_descr : GET_NAME(v)));
    send_to_char(buf, ch);
    sprintf(buf, "%s slams you into the wall!\n\r",
      (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
    send_to_char(buf, v);
    act("$N is slammed into the wall by $n!\n\r", TRUE, ch, 0, v, TO_ROOM);
  }
}

int ThrowerMob(struct char_data* ch, int cmd, char* arg) {
  struct char_data* vict;

  /*
  **  Throws people in various directions
  */

  if (!cmd) {
    if (AWAKE(ch) && ch->specials.fighting) {
      /*
      **  take this person and throw them
      */
      vict = ch->specials.fighting;
      switch (ch->in_room) {
        case 13912:
          ThrowChar(ch, vict, 1); /* throw chars to the east */
          return (FALSE);
          break;
        default:
          return (FALSE);
      }
    }
  } else {
    switch (ch->in_room) {
      case 13912: {
        if (cmd == 1) { /* north+1 */
          send_to_char("The Troll blocks your way.\n", ch);
          return (TRUE);
        }
        break;
      }
      default:
        return (FALSE);
    }
  }
  return (FALSE);
}

#if 0
/*
Smart thief special
*/

Thief(struct char_data *ch, char *arg, ind cmd)
{

  if (cmd || !AWAKE(ch)) return;

}

#endif

#if 0
/*
Swallower special
*/
Tyrannosaurus_swallower(struct char_data *ch, char *arg, ind cmd)
{
  struct obj_data *co, *o *tmp;
  struct char_data *targ;


  if (cmd && cmd != 156) return(FALSE);

  if (cmd == 156) {
    send_to_char("You're much too afraid to steal anything!\n\r", ch);
    return(TRUE);
  }


/*
**  swallow
*/

  if (AWAKE(ch)) {
    if ((targ = FindAnAttacker(ch))!=NULL) {
      act("$n opens $s gaping mouth", TRUE, ch, 0, 0, TO_ROOM);
      if (!saves_spell(targ, SAVING_PARA)) {
	act("In a single gulp, $N is swallowed whole!\n\r",
	    TRUE, ch, 0. targ, TO_ROOM);
	send_to_char("In a single gulp, you are swallowed whole!\n\r", targ);
	send_to_char("The horror!  The horror!\n\r", targ);
	send_to_char("MMM.  yum!\n\r", ch);
	/*
	  kill target:
	*/
	die(targ);
	/*
	  all stuff to monster:  this one is tricky.  assume that corpse is
	  top item on item_list now that corpse has been made.
	*/
	for (co = object_list; co; co = co->next) {
	  if (IS_CORPSE(co))  {  /* assume 1st corpse is victims */
	    for (o = co->contains; o; o = tmp) {
	      tmp = o->next_content;
	      obj_from_obj(o);
	      obj_to_char(o, ch);
	    }
	    extract_obj(co);  /* remove the corpse */
	    return(TRUE);
	  }
	}
      }
    }
  }
}
#endif

int soap(Mob* ch, int cmd, char* arg, Obj* me) {
  struct char_data* t;
  struct obj_data* obj;
  char dummy[80], name[80];
  int (*wash)();

  wash = soap;

  if (cmd != 172)
    return (FALSE);

  if (!(obj = ch->equipment[HOLD]))
    return (FALSE);
  if (obj_index[obj->item_number].func.obj_f != wash)
    return (FALSE);

  arg = one_argument(arg, dummy);
  if (!(*dummy))
    return (FALSE);
  only_argument(arg, name);
  if (!(*name))
    return (FALSE);

  if (!(t = get_char_room_vis(ch, name)))
    return (FALSE);

  if (affected_by_spell(t, SPELL_WEB)) {
    affect_from_char(t, SPELL_WEB);
    act("$n washes some webbing off $N with $p.", TRUE, ch, obj, t, TO_ROOM);
    act("You wash some webbing off $N with $p.", FALSE, ch, obj, t, TO_CHAR);
  } else {
    act("$n gives $N a good lathering with $p.", TRUE, ch, obj, t, TO_ROOM);
    act("You give $N a good lathering with $p.", FALSE, ch, obj, t, TO_CHAR);
  }

  obj->obj_flags.value[0]--;
  if (!obj->obj_flags.value[0]) {
    act("That used up $p.", FALSE, ch, obj, t, TO_CHAR);
    extract_obj(obj);
  }
}

int nodrop(Mob* ch, int cmd, char* arg, Obj* me) {
  struct char_data* t;
  struct obj_data *obj, *i;
  char buf[80], obj_name[80], vict_name[80], *name;
  bool do_all;
  int j, num;
  int (*knowdrop)();

  switch (cmd) {
    case 10:  /* Get */
    case 60:  /* Drop */
    case 72:  /* Give */
    case 156: /* Steal */
      break;
    default:
      return (FALSE);
  }

  knowdrop = nodrop;

  arg = one_argument(arg, obj_name);
  if (!*obj_name)
    return (FALSE);

  obj = 0x0;
  do_all = FALSE;

  if (!(strncmp(obj_name, "all", 3))) {
    do_all = TRUE;
    num = IS_CARRYING_N(ch);
  } else {
    strcpy(buf, obj_name);
    name = buf;
    if (!(num = get_number(&name)))
      return (FALSE);
  }

  /* Look in the room first, in get case */
  if (cmd == 10)
    for (i = real_roomp(ch->in_room)->contents, j = 1; i && (j <= num);
         i = i->next_content)
      if (i->item_number >= 0)
        if (do_all || isname(name, i->name))
          if (do_all || j == num) {
            if (obj_index[i->item_number].func.obj_f == knowdrop) {
              obj = i;
              break;
            }
          } else
            ++j;

  /* Check the character's inventory for give, drop, steal. */
  if (!obj)
    /* Don't bother with get anymore */
    if (cmd == 10)
      return (FALSE);
  for (i = ch->carrying, j = 1; i && (j <= num); i = i->next_content)
    if (i->item_number >= 0)
      if (do_all || isname(name, i->name))
        if (do_all || j == num) {
          if (obj_index[i->item_number].func.obj_f == knowdrop) {
            obj = i;
            break;
          } else if (!do_all)
            return (FALSE);
        } else
          ++j;

  /* Musta been something else */
  if (!obj)
    return (FALSE);

  if ((cmd == 72) || (cmd == 156)) {
    only_argument(arg, vict_name);
    if ((!*vict_name) || (!(t = get_char_room_vis(ch, vict_name))))
      return (FALSE);
  }

  switch (cmd) {
    case 10:
      if (GetMaxLevel(ch) <= MAX_MORT) {
        act("$p disintegrates when you try to pick it up!", FALSE, ch, obj, 0,
          TO_CHAR);
        act("$n tries to get $p, but it disintegrates in his hand!", FALSE, ch,
          obj, 0, TO_ROOM);
        extract_obj(obj);
        if (do_all)
          return (FALSE);
        else
          return (TRUE);
      } else
        return (FALSE);

    case 60:
      if (!IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
        act("You drop $p to the ground, and it shatters!", FALSE, ch, obj, 0,
          TO_CHAR);
        act("$n drops $p, and it shatters!", FALSE, ch, obj, 0, TO_ROOM);
        i = read_object(30, VIRTUAL);
        sprintf(buf, "Scraps from %s lie in a pile here.",
          obj->short_description);
        i->description = strdup(buf);
        obj_to_room(i, ch->in_room);
        obj_from_char(obj);
        extract_obj(obj);
        if (do_all)
          return (FALSE);
        else
          return (TRUE);
      } else
        return (FALSE);

    case 72:
      if (!IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
        if (GetMaxLevel(ch) <= MAX_MORT) {
          act("You try to give $p to $N, but it vanishes!", FALSE, ch, obj, t,
            TO_CHAR);
          act("$N tries to give $p to you, but it fades away!", FALSE, t, obj,
            ch, TO_CHAR);
          act("As $n tries to give $p to $N, it vanishes!", FALSE, ch, obj, t,
            TO_ROOM);
          extract_obj(obj);
          if (do_all)
            return (FALSE);
          else
            return (TRUE);
        } else
          return (FALSE);
      } else
        return (FALSE);

    case 156: /* Steal */
      if (!IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
        act("You cannot seem to steal $p from $N.", FALSE, ch, obj, t, TO_CHAR);
        act("$N tried to steal something from you!", FALSE, t, obj, ch,
          TO_CHAR);
        act("$N tried to steal something from $n!", FALSE, t, obj, ch, TO_ROOM);
        return (TRUE);
      } else
        return (FALSE);

    default:
      return (FALSE);
  }

  return (FALSE);
}

static const char* const lattimore_descs[] = {
  "A small orc is trying to break into a locker.\n\r",
  "A small orc is walking purposefully down the hall.\n\r",
  "An orc is feeding it's face with rat stew.\n\r",
  "A small orc is cowering underneath a bunk\n\r",
  "An orc sleeps restlessly on a bunk.\n\r",
  "There is an orc stading on a barrel here.\n\r",
  "An orc is traveling down the corridor at high speed.\n\r",
};

static int affect_status(struct mob_act_lattimore* mem, struct char_data* ch,
  struct char_data* t, int aff_status) {
  int i;

  if (mem->c)
    for (i = 0; i < mem->c; ++i)
      if (!(strcmp(GET_NAME(t), mem->names[i]))) {
        mem->status[i] += aff_status;
        return (i);
        break;
      }

  if (!mem->c) {
    mem->names = (char**)malloc(sizeof(char));
    mem->status = (int*)malloc(sizeof(char));
  } else {
    mem->names = (char**)realloc(mem->names, (sizeof(char) * mem->c));
    mem->status = (int*)realloc(mem->status, (sizeof(char) * mem->c));
  }
  mem->names[mem->c] = (char*)malloc(sizeof(strlen(GET_NAME(t) + 2)));
  strcpy(mem->names[mem->c], GET_NAME(t));
  mem->status[mem->c] = (int)malloc(sizeof(int));
  mem->status[mem->c] = aff_status;
  ++mem->c;
  return (mem->c - 1);
}

static enum LattimoreState {
  LATTIMORE_STATE_INITIALIZE,
  LATTIMORE_STATE_LOCKERS,
  LATTIMORE_STATE_FOOD_RUN,
  LATTIMORE_STATE_EATING,
  LATTIMORE_STATE_GO_HOME,
  LATTIMORE_STATE_HIDING,
  LATTIMORE_STATE_SLEEPING,
  LATTIMORE_STATE_RUN,
  LATTIMORE_STATE_ITEM,
};

static enum LattimoreLocation {
  LATTIMORE_LOCATION_KITCHEN = 21310,
  LATTIMORE_LOCATION_BARRACKS = 21277,
  LATTIMORE_LOCATION_STOREROOM = 21319,
  LATTIMORE_LOCATION_CONF = 21322,
  LATTIMORE_LOCATION_TRAP = 21335,
  LATTIMORE_LOCATION_EARTHQ = 21334,
};

static struct mob_act_lattimore* init_lattimore(struct char_data* ch) {
  assert(ch && "ch cannot be NULL");
  assert(!ch->act_ptr && "ch->act_ptr should be NULL");

  struct mob_act_lattimore* mem = NULL;

  ch->act_ptr = create(struct mob_act_data, 1);
  ch->act_ptr->type = MOB_ACT_LATTIMORE;
  mem = &ch->act_ptr->data.lattimore;
  mem->pointer = mem->c = mem->index = 0;

  return mem;
}

int lattimore(struct char_data* ch, int cmd, char* arg) {
  assert(ch && "ch cannot be NULL");
  assert((!ch->act_ptr || ch->act_ptr->type == MOB_ACT_LATTIMORE) &&
         "ch->act_ptr should be NULL or of type MOB_ACT_LATTIMORE");

  static const int CrowBar = 21114;
  static const int PostKey = 21150;

  struct mob_act_lattimore* mem;

  if (!cmd) {
    if (!AWAKE(ch))
      return FALSE;

    if (!ch->act_ptr) {
      mem = init_lattimore(ch);
      mem->pointer = 0;
      mem->c = 0;
      mem->index = 0;
    } else {
      mem = &ch->act_ptr->data.lattimore;
    }

    if (ch->master) {
      mem->pointer = 0;
      return (FALSE);
    }

    if (ch->specials.fighting) {
      if (!IS_MOB(ch->specials.fighting) && CAN_SEE(ch, ch->specials.fighting))
        affect_status(mem, ch, ch->specials.fighting, -5);

      if (mem->status[mem->index] < 0) {
        strcpy(ch->player.long_descr, lattimore_descs[6]);
        mem->pointer = LATTIMORE_STATE_RUN;
      } else if (mem->status[mem->index] > 19)
        mem->pointer = LATTIMORE_STATE_ITEM;

      return (FALSE);
    }

    switch (mem->pointer) {
      /* This case is used at startup, and after player interaction*/
      case LATTIMORE_STATE_INITIALIZE:
        if (time_info.hours < 5 || time_info.hours > 21) {
          strcpy(ch->player.long_descr, lattimore_descs[3]);

          if (ch->in_room != LATTIMORE_LOCATION_BARRACKS) {
            char_from_room(ch);
            char_to_room(ch, LATTIMORE_LOCATION_BARRACKS);
          }

          mem->pointer = LATTIMORE_STATE_HIDING;
        } else if (time_info.hours < 11) {
          strcpy(ch->player.long_descr, lattimore_descs[4]);
          if (ch->in_room != LATTIMORE_LOCATION_BARRACKS) {
            char_from_room(ch);
            char_to_room(ch, LATTIMORE_LOCATION_BARRACKS);
          }
          mem->pointer = LATTIMORE_STATE_SLEEPING;
        } else if ((time_info.hours < 16) ||
                   ((time_info.hours > 17) && (time_info.hours < 22))) {
          strcpy(ch->player.long_descr, lattimore_descs[0]);
          if (ch->in_room != LATTIMORE_LOCATION_BARRACKS) {
            char_from_room(ch);
            char_to_room(ch, LATTIMORE_LOCATION_BARRACKS);
          }
          mem->pointer = LATTIMORE_STATE_LOCKERS;
        } else if (time_info.hours < 19) {
          strcpy(ch->player.long_descr, lattimore_descs[1]);
          mem->pointer = LATTIMORE_STATE_FOOD_RUN;
        }

        return (FALSE);

      case LATTIMORE_STATE_LOCKERS:
        if (time_info.hours == 17) {
          strcpy(ch->player.long_descr, lattimore_descs[1]);
          mem->pointer = LATTIMORE_STATE_FOOD_RUN;
        } else if (time_info.hours > 21) {
          act("$n cocks his head, as if listening.", FALSE, ch, 0, 0, TO_ROOM);
          act("$n looks frightened, and dives under the nearest bunk.", FALSE,
            ch, 0, 0, TO_ROOM);
          strcpy(ch->player.long_descr, lattimore_descs[3]);
          mem->pointer = LATTIMORE_STATE_HIDING;
        }

        return (FALSE);

      case LATTIMORE_STATE_FOOD_RUN: {
        if (ch->in_room != LATTIMORE_LOCATION_KITCHEN) {
          int dir =
            choose_exit_global(ch->in_room, LATTIMORE_LOCATION_KITCHEN, 100);

          if (dir < 0) {
            act("$n says 'Man, am I lost!'", FALSE, ch, 0, 0, TO_ROOM);

            dir =
              choose_exit_global(ch->in_room, LATTIMORE_LOCATION_BARRACKS, 100);

            if (dir < 0) {
              char_from_room(ch);
              char_to_room(ch, LATTIMORE_LOCATION_BARRACKS);
            }

            return (FALSE);
          } else
            go_direction(ch, dir);
        } else {
          act("$n gets utensils off the counter, and ladels himself some stew.",
            FALSE, ch, 0, 0, TO_ROOM);
          strcpy(ch->player.long_descr, lattimore_descs[2]);
          mem->pointer = LATTIMORE_STATE_EATING;
        }

        return (FALSE);
      }

      case LATTIMORE_STATE_EATING:
        if (time_info.hours > 18) {
          act("$n rubs his stomach and smiles happily.", FALSE, ch, 0, 0,
            TO_ROOM);
          strcpy(ch->player.long_descr, lattimore_descs[1]);
          mem->pointer = LATTIMORE_STATE_GO_HOME;
        } else if (!number(0, 2)) {
          act("$n gets some bread from the oven to go with his stew.", FALSE,
            ch, 0, 0, TO_ROOM);
          act("$n dips the bread in the stew and eats it.", FALSE, ch, 0, 0,
            TO_ROOM);
        }
        return (FALSE);
        break;

      case LATTIMORE_STATE_GO_HOME: {
        if (ch->in_room != LATTIMORE_LOCATION_BARRACKS) {
          int dir =
            choose_exit_global(ch->in_room, LATTIMORE_LOCATION_BARRACKS, 100);

          if (dir < 0) {
            act("$n says 'Man, am I lost!'", FALSE, ch, 0, 0, TO_ROOM);

            dir =
              choose_exit_global(ch->in_room, LATTIMORE_LOCATION_KITCHEN, 100);

            if (dir < 0) {
              char_from_room(ch);
              char_to_room(ch, LATTIMORE_LOCATION_BARRACKS);
            }

            return (FALSE);
          } else
            go_direction(ch, dir);
        } else {
          act("$n pulls out a crowbar and tries to open another locker.", FALSE,
            ch, 0, 0, TO_ROOM);
          strcpy(ch->player.long_descr, lattimore_descs[0]);
          mem->pointer = LATTIMORE_STATE_LOCKERS;
        }

        return (FALSE);
      }

      case LATTIMORE_STATE_HIDING:
        if ((time_info.hours > 5) && (time_info.hours < 22)) {
          strcpy(ch->player.long_descr, lattimore_descs[4]);
          mem->pointer = LATTIMORE_STATE_SLEEPING;
        }
        return (FALSE);
        break;

      case LATTIMORE_STATE_SLEEPING:
        if (time_info.hours > 11) {
          act("$n awakens, rises and stretches with a yawn.", FALSE, ch, 0, 0,
            TO_ROOM);
          act("$n pulls out a crowbar and tries to open another locker.", FALSE,
            ch, 0, 0, TO_ROOM);
          strcpy(ch->player.long_descr, lattimore_descs[0]);
          mem->pointer = LATTIMORE_STATE_LOCKERS;
        }
        return (FALSE);
        break;

      case LATTIMORE_STATE_RUN: {
        if (ch->in_room != LATTIMORE_LOCATION_STOREROOM &&
            ch->in_room != LATTIMORE_LOCATION_TRAP) {
          if (ch->in_room == LATTIMORE_LOCATION_EARTHQ)
            return (FALSE);

          int dir =
            choose_exit_global(ch->in_room, LATTIMORE_LOCATION_STOREROOM, 100);

          if (dir < 0) {
            act("$n says 'Man, am I lost!'", FALSE, ch, 0, 0, TO_ROOM);

            dir =
              choose_exit_global(ch->in_room, LATTIMORE_LOCATION_KITCHEN, 100);

            if (dir < 0) {
              char_from_room(ch);
              char_to_room(ch, LATTIMORE_LOCATION_BARRACKS);
            }

            return (FALSE);
          } else
            go_direction(ch, dir);
        } else if (ch->in_room == LATTIMORE_LOCATION_TRAP) {
          if (!IS_AFFECTED(ch, AFF_FLYING)) {
            /* Get him up off the floor */
            act("$n grins evilly, and quickly stands on a barrel.", FALSE, ch,
              0, 0, TO_ROOM);
            SET_BIT(ch->specials.affected_by, AFF_FLYING);
            strcpy(ch->player.long_descr, lattimore_descs[5]);
            mem->index = 0;
          } else {
            ++mem->index;
          }

          /* Wait a while, then go home */
          if (mem->index == 50) {
            go_direction(ch, 1);
            mem->pointer = LATTIMORE_STATE_GO_HOME;
            REMOVE_BIT(ch->specials.affected_by, AFF_FLYING);
            strcpy(ch->player.long_descr, lattimore_descs[1]);
          }
        }

        return (FALSE);
      }

      case LATTIMORE_STATE_ITEM: {
        if (ch->in_room != LATTIMORE_LOCATION_CONF) {
          int dir =
            choose_exit_global(ch->in_room, LATTIMORE_LOCATION_CONF, 100);

          if (dir < 0) {
            act("$n says 'Man, am I lost!'", FALSE, ch, 0, 0, TO_ROOM);

            dir =
              choose_exit_global(ch->in_room, LATTIMORE_LOCATION_BARRACKS, 100);

            if (dir < 0) {
              char_from_room(ch);
              char_to_room(ch, LATTIMORE_LOCATION_BARRACKS);
            }

            return (FALSE);
          } else
            go_direction(ch, dir);
        } else {
          for (struct char_data* t = real_roomp(ch->in_room)->people; t;
               t = t->next_in_room) {
            if (!IS_NPC(t) && CAN_SEE(ch, t)) {
              if (!(strcmp(mem->names[mem->index], GET_NAME(t)))) {
                act("$n crawls under the large table.", FALSE, ch, 0, 0,
                  TO_ROOM);

                struct obj_data* obj = read_object(PostKey, VIRTUAL);

                if ((IS_CARRYING_N(t) + 1) < CAN_CARRY_N(t)) {
                  act("$N emerges with $p, and gives it to you.", FALSE, t, obj,
                    ch, TO_CHAR);

                  act("$n emerges with $p, and gives it to $N.", FALSE, ch, obj,
                    t, TO_ROOM);

                  obj_to_char(obj, t);
                } else {
                  act("$n emerges with $p, and drops it for $N.", FALSE, ch,
                    obj, t, TO_ROOM);

                  obj_to_room(obj, ch->in_room);
                }
              }
            }
          }

          /* Dude's not here - oh well, go home. */
          mem->status[mem->index] = 0; /* Duty discharged */
          mem->pointer = LATTIMORE_STATE_GO_HOME;
          return (FALSE);
        }
        break;
      }
      default:
        mem->pointer = LATTIMORE_STATE_INITIALIZE;
        return (FALSE);
    }

    return TRUE;
  }

  if (cmd != 72) {
    return FALSE;
  }

  char obj_name[80];
  arg = one_argument(arg, obj_name);

  struct obj_data* obj;
  if (!*obj_name || !(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
    return FALSE;
  }

  char player_name[80];
  only_argument(arg, player_name);

  struct char_data* latt;
  if (!*player_name || !(latt = get_char_room_vis(ch, player_name)) ||
      mob_index[latt->nr].func.mob_f != lattimore)
    return FALSE;

  // The mob's act_ptr should have always been initialized by now
  assert(
    latt->act_ptr && latt->act_ptr->type == MOB_ACT_LATTIMORE &&
    "latt->act_ptr should not be null and should be of type MOB_ACT_LATTIMORE");

  act("You give $p to $N.", TRUE, ch, obj, latt, TO_CHAR);
  act("$n gives $p to $N.", TRUE, ch, obj, latt, TO_ROOM);
  obj_from_char(obj);

  int status_change = 0;

  switch (obj->obj_flags.type_flag) {
    case ITEM_FOOD:
      if (obj->obj_flags.value[3]) {
        act("$n sniffs $p, then discards it with disgust.", TRUE, latt, obj, 0,
          TO_ROOM);

        obj_to_room(obj, latt->in_room);
        status_change = -5;

      } else {
        act("$n takes $p and hungrily wolfs it down.", TRUE, latt, obj, 0,
          TO_ROOM);

        extract_obj(obj);
        status_change = 5;
      }

      break;
    default:
      /* What he really wants */
      if (obj_index[obj->item_number].virtual == CrowBar) {
        act("$n takes $p and jumps up and down in joy.", TRUE, latt, obj, 0,
          TO_ROOM);

        const struct obj_data* const held = latt->equipment[HOLD];

        if (held) {
          if (held->item_number == obj->item_number) {
            extract_obj(obj);
          } else {
            obj_to_char(unequip_char(latt, HOLD), latt);
            equip_char(latt, obj, HOLD);
          }
        }

        status_change = 20;
      } else {
        /* Any other types of items */
        act("$n looks at $p curiously, then discards it with a shrug.", TRUE,
          latt, obj, 0, TO_ROOM);
        obj_to_room(obj, latt->in_room);
      }
      break;
  }

  if (status_change != 0 && !IS_MOB(ch) && CAN_SEE(latt, ch)) {
    // They gave something to him, and the status was affected, now we set the
    // pointer according to the status value

    mem->index = affect_status(mem, latt, ch, status_change);

    if (mem->status[mem->index] < 0) {
      strcpy(latt->player.long_descr, lattimore_descs[6]);
      mem->pointer = LATTIMORE_STATE_RUN;
    } else if (mem->status[mem->index] > 19) {
      strcpy(latt->player.long_descr, lattimore_descs[6]);
      mem->pointer = LATTIMORE_STATE_ITEM;
    }
  }

  return TRUE;
}

/* Returns the index to the dude who did it */

int coldcaster(struct char_data* ch, int cmd, char* arg) {
  struct char_data* vict;
  byte lspell;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  /* Find a dude to to evil things upon ! */

  vict = FindVictim(ch);

  if (!vict)
    vict = ch->specials.fighting;

  if (!vict)
    return (FALSE);

  lspell = number(0, 9);

  switch (lspell) {
    case 0:
    case 1:
    case 2:
    case 3:
      act("$N touches you!", 1, vict, 0, ch, TO_CHAR);
      cast_chill_touch(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 4:
    case 5:
    case 6:
      cast_cone_of_cold(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 7:
    case 8:
    case 9:
      cast_ice_storm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
  }

  return (TRUE);
}

int trapper(struct char_data* ch, int cmd, char* arg) {
  struct char_data* tch;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  /* Okay, the idea is this: If the PC or NPC in this room isn't flying,
     it is walking on the trapper. Doesn't matter if it's sneaking, or
     invisible, or whatever. The trapper will attack both PCs and NPCs,
     so don't have a lot of wandering NPCs around it. */

  if (!ch->specials.fighting) {
    for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room)
      if ((ch != tch) && !IS_IMMORTAL(tch) && !IS_AFFECTED(tch, AFF_FLYING)) {
        set_fighting(ch, tch);
        return (TRUE);
      }
    /* Nobody here */
    return (FALSE);
  } else {
    if (GetMaxLevel(ch->specials.fighting) > MAX_MORT)
      return (FALSE);

    /* Equipment must save against crush - will fail 25% of the time */

    /* Make the poor sucker save against paralzyation, or suffocate */
    if (saves_spell(ch->specials.fighting, SAVING_PARA)) {
      act("You can hardly breathe, $N is suffocating you!", FALSE,
        ch->specials.fighting, 0, ch, TO_CHAR);
      act("$N is stifling $n, who will suffocate soon!", FALSE,
        ch->specials.fighting, 0, ch, TO_ROOM);
      return (FALSE);
    } else {
      act("You gasp for air inside $N!", FALSE, ch->specials.fighting, 0, ch,
        TO_CHAR);
      act("$N stifles you. You asphyxiate and die!", FALSE,
        ch->specials.fighting, 0, ch, TO_CHAR);
      act("$n has suffocated inside $N!", FALSE, ch->specials.fighting, 0, ch,
        TO_ROOM);
      act("$n is dead!", FALSE, ch->specials.fighting, 0, ch, TO_ROOM);
      die(ch->specials.fighting);
      ch->specials.fighting = 0x0;
      return (TRUE);
    }
  }
}

int trogcook(struct char_data* ch, int cmd, char* arg) {
  struct char_data* tch;
  struct obj_data* corpse;
  char buf[MAX_INPUT_LENGTH];

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    if (GET_POS(ch) != POSITION_FIGHTING)
      StandUp(ch);
    return (FALSE);
  }

  for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room)
    if (IS_NPC(tch) && IsAnimal(tch) && CAN_SEE(ch, tch)) {
      if (!check_soundproof(ch))
        act("$n cackles 'Something else for the pot!'", FALSE, ch, 0, 0,
          TO_ROOM);
      hit(ch, tch, TYPE_UNDEFINED);
      return (TRUE);
    }

  corpse = get_obj_in_list_vis(ch, "corpse", real_roomp(ch->in_room)->contents);
  if (corpse) {
    // TODO: This should return value for success/failure to make this sort of
    // behavior safely possible
    do_get(ch, "corpse", -1);
    act("$n cackles 'Into the soup with it!'", FALSE, ch, 0, 0, TO_ROOM);
    sprintf(buf, "put corpse pot");
    command_interpreter(ch, buf);
    return (TRUE);
  }
}

int shaman(struct char_data* ch, int cmd, char* arg) {
#define DEITY 21124
#define DEITY_NAME "golgar"
  struct char_data *god, *tch;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    if (number(0, 3) == 0) {
      for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room)
        if ((!IS_NPC(tch)) && (GetMaxLevel(tch) > 20) && CAN_SEE(ch, tch)) {
          if (!(god = get_char_room_vis(ch, DEITY_NAME))) {
            act("$n screams 'Golgar, I summon thee to aid thy servants!'",
              FALSE, ch, 0, 0, TO_ROOM);
            if (number(0, 8) == 0) {
              act("There is a blinding flash of light!", FALSE, ch, 0, 0,
                TO_ROOM);
              god = read_mobile(DEITY, VIRTUAL);
              char_to_room(god, ch->in_room);
            }
          } else if (number(0, 2) == 0)
            act("$n shouts 'Now you will die!'", FALSE, ch, 0, 0, TO_ROOM);
        }
    } else
      return (cleric(ch, cmd, arg));
  }
}

int golgar(struct char_data* ch, int cmd, char* arg) {
#define SHAMAN_NAME "shaman"
  struct char_data *shaman, *tch;

  if (cmd)
    return (FALSE);

  if (!ch->specials.fighting) {
    if (!(shaman = get_char_room_vis(ch, SHAMAN_NAME))) {
      for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room)
        if (IS_NPC(tch) && (GET_RACE(tch) == RACE_TROGMAN))
          if ((tch->specials.fighting) && (!IS_NPC(tch->specials.fighting))) {
            act("$n growls 'Death to those attacking my people!'", FALSE, ch, 0,
              0, TO_ROOM);
            hit(ch, tch->specials.fighting, TYPE_UNDEFINED);
            return (FALSE);
          }
      if (number(0, 5) == 0) {
        act("$n slowly fades into ethereal emptiness.", FALSE, ch, 0, 0,
          TO_ROOM);
        extract_char(ch);
      }
    } else {
      if (!shaman->specials.fighting) {
        act("$n growls 'How dare you summon me!'", FALSE, ch, 0, 0, TO_ROOM);
        hit(ch, shaman, TYPE_UNDEFINED);
        return (FALSE);
      } else {
        act("$n screams 'You dare touch my holy messenger!? DIE!'", FALSE, ch,
          0, 0, TO_ROOM);
        hit(ch, shaman->specials.fighting, TYPE_UNDEFINED);
        return (FALSE);
      }
    }
  } else
    return (magic_user(ch, cmd, arg));
}

int troguard(struct char_data* ch, int cmd, char* arg) {
  struct char_data *tch, *good, *i;
  int max_good, lev;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    if (GET_POS(ch) == POSITION_FIGHTING) {
      FighterMove(ch);
    } else {
      StandUp(ch);
    }

    if (!check_soundproof(ch)) {
      act("$n shouts 'The enemy is upon us! Help me, my brothers!'", TRUE, ch,
        0, 0, TO_ROOM);
      if (ch->specials.fighting)
        CallForGuard(ch, ch->specials.fighting, 3, TROGCAVES);
      return (TRUE);
    }
  }

  max_good = -1001;
  good = 0;

  for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room)
    if ((GET_ALIGNMENT(tch) > max_good) && !IS_IMMORTAL(tch) &&
        (!IS_NPC(tch) || (IS_NPC(tch) && (GET_RACE(tch) != RACE_TROGMAN) &&
                           (GET_RACE(tch) != RACE_ARACHNID)))) {
      max_good = GET_ALIGNMENT(tch);
      good = tch;
    }

  if (check_peaceful(ch, ""))
    return FALSE;

  if (good) {
    if (!check_soundproof(ch))
      act("$n screams 'Die invading scum! Take that!'", FALSE, ch, 0, 0,
        TO_ROOM);
    hit(ch, good, TYPE_UNDEFINED);
    return (TRUE);
  }

  return (FALSE);
}

int keystone(struct char_data* ch, int cmd, char* arg) {
#define START_ROOM 21276
#define END_ROOM 21333
#define GhostSoldier 21138
#define GhostLieutenant 21139
  /* Must be a unique identifier for this mob type, or we lose */
#define Identifier "gds"

  struct char_data *ghost, *t, *master;
  int i;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (time_info.hours == 22) {
    if (!(ghost = get_char_vis_world(ch, Identifier, 0))) {
      act("$n cries 'Awaken my soldiers! Our time is nigh!'", FALSE, ch, 0, 0,
        TO_ROOM);
      act("You suddenly feel very, very afraid.", FALSE, ch, 0, 0, TO_ROOM);
      for (i = START_ROOM; i < END_ROOM; ++i)
        if (number(0, 2) == 0) {
          ghost = read_mobile(GhostSoldier, VIRTUAL);
          char_to_room(ghost, i);
        } else if (number(0, 7) == 0) {
          ghost = read_mobile(GhostLieutenant, VIRTUAL);
          char_to_room(ghost, i);
        }
      for (t = character_list; t; t = t->next)
        if (real_roomp(ch->in_room)->zone == real_roomp(t->in_room)->zone)
          act("You hear a strange cry that fills your soul with fear!", FALSE,
            t, 0, 0, TO_CHAR);
    }
  }

  if (ch->specials.fighting) {
    if (IS_NPC(ch->specials.fighting) &&
        !IS_SET((ch->specials.fighting)->specials.act, ACT_POLYSELF))
      if ((master = (ch->specials.fighting)->master) && CAN_SEE(ch, master)) {
        stop_fighting(ch);
        hit(ch, master, TYPE_UNDEFINED);
      }
    if (GET_POS(ch) == POSITION_FIGHTING) {
      FighterMove(ch);
    } else {
      StandUp(ch);
    }
    CallForGuard(ch, ch->specials.fighting, 3, OUTPOST);
  } else
    return (FALSE);
}

int ghostsoldier(struct char_data* ch, int cmd, char* arg) {
  struct char_data *tch, *good, *master;
  int max_good;
  int (*gs)(), (*gc)();

  gs = ghostsoldier;
  gc = keystone;

  if (cmd)
    return (FALSE);

  if (time_info.hours > 4 && time_info.hours < 22) {
    act("$n slowly fades out of existence.", FALSE, ch, 0, 0, TO_ROOM);
    extract_char(ch);
    return (TRUE);
  }

  max_good = -1001;
  good = 0;

  for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room)
    if (!(mob_index[tch->nr].func.mob_f == gs) && /* Another ghost soldier? */
        !(mob_index[tch->nr].func.mob_f == gc) && /* The ghost captain? */
        (GET_ALIGNMENT(tch) > max_good) &&        /* More good than prev? */
        !IS_IMMORTAL(tch) &&                      /* A god? */
        (GET_RACE(tch) >= 4)) {                   /* Attack only npc races */
      max_good = GET_ALIGNMENT(tch);
      good = tch;
    }

  /* What is a ghost Soldier doing in a peaceful room? */
  if (check_peaceful(ch, ""))
    return FALSE;

  if (good) {
    if (!check_soundproof(ch))
      act("$N attacks you with an unholy scream!", FALSE, good, 0, ch, TO_CHAR);
    hit(ch, good, TYPE_UNDEFINED);
    return (TRUE);
  }

  if (ch->specials.fighting) {
    if (IS_NPC(ch->specials.fighting) &&
        !IS_SET((ch->specials.fighting)->specials.act, ACT_POLYSELF))
      if ((master = (ch->specials.fighting)->master) && CAN_SEE(ch, master)) {
        stop_fighting(ch);
        hit(ch, master, TYPE_UNDEFINED);
      }
    if (GET_POS(ch) == POSITION_FIGHTING) {
      FighterMove(ch);
    } else {
      StandUp(ch);
    }
    CallForGuard(ch, ch->specials.fighting, 3, OUTPOST);
  }
  return (FALSE);
}

char* quest_one[] = {"The second artifact you must find is the ring of Tlanic.",
  "Tlanic was an elven warrior who left Rhyodin five years after",
  "Lorces; he also was given an artifact to aid him.",
  "He went to find out what happened to Lorces, his friend, and to",
  "find a way to the north if he could.",
  "He also failed. Return the ring to me for further instructions."};

char* quest_two[] = {
  "When Tlanic had been gone for many moons; his brother Evistar",
  "went to find him.",
  "Evistar, unlike his brother, was not a great warrior, but he was",
  "the finest tracker among our people.",
  "Given to him as an aid to his travels, was a neverempty chalice.",
  "Bring this magical cup to me, if you wish to enter the kingdom",
  "of the Rhyodin."};

char* quest_three[] = {
  "When Evistar did not return, ages passed before another left.",
  "A mighty wizard was the next to try. His name was C*zarnak.",
  "It is feared that he was lost in the deep caves, like the others.",
  "He wore an enchanted circlet on his brow.",
  "Find it and bring it to me, and I will tell you more."};

char* necklace[] = {
  "You have brought me all the items lost by those who sought the",
  "path from the kingdom to the outside world.",
  "Furthermore, you have found the way through the mountains",
  "yourself, proving your ability to track and map.",
  "You are now worthy to be an ambassador to my kingdom.",
  "Take this necklace, and never part from it!",
  "Give it to the gatekeeper, and he will let you pass.",
};

char* nonecklace[] = {
  "You have brought me all the items lost by those who sought the",
  "path from the kingdom to the outside world.",
  "Furthermore, you have found the way through the mountains",
  "yourself, proving your ability to track and map.",
  "You are now worthy to be an ambassador to my kingdom.",
  "Your final quest is to obtain the Necklace of Wisdom.",
  "The gatekeeper will recognize it, and let you pass.",
};

char* quest_intro[] = {
  "My name is Valik, and I am the Lorekeeper of the Rhyodin.",
  "Rhyodin is kingdom southeast of the Great Eastern Desert.",
  "To enter the kingdom of Rhyodin, you must first pass this test.",
  "When the only route to the kingdom collapsed, commerce with",
  "the outside world was cutoff.",
  "Lorces was the first to try to find a path back to the outside,",
  "and for his bravery to him a shield was given.",
  "The first step towards entry to the kingdom of Rhyodin is to",
  "return to me the shield of Lorces.",
};

static const int Necklace = 21122;

int Valik(struct char_data* ch, int cmd, char* arg) {
#define Valik_Wandering 0
#define Valik_Meditating 1
#define Valik_Qone 2
#define Valik_Qtwo 3
#define Valik_Qthree 4
#define Valik_Qfour 5
#define Shield 21113
#define Ring 21120
#define Chalice 21121
#define Circlet 21117
#define Med_Chambers 21324

  char obj_name[80], vict_name[80], buf[MAX_INPUT_LENGTH];
  int i;
  struct char_data *vict, *tch, *master;
  struct obj_data* obj;
  int (*valik)();
  bool gave_this_click = FALSE;
  short quest_lines[4] = {6, 7, 5, 7};
  short valik_dests[9] = {104, 1638, 7902, 13551, 16764, 17330, 19244, 21325,
    25230};

  if ((cmd && (cmd != 72) && (cmd != 86)) || (!AWAKE(ch)))
    return (FALSE);

  if (!cmd)
    if (ch->specials.fighting) {
      if (IS_NPC(ch->specials.fighting) &&
          !IS_SET((ch->specials.fighting)->specials.act, ACT_POLYSELF))
        if ((master = (ch->specials.fighting)->master) && CAN_SEE(ch, master)) {
          stop_fighting(ch);
          hit(ch, master, TYPE_UNDEFINED);
        }
      return (magic_user(ch, cmd, arg));
    }

  vict = get_char_room_vis(ch, "valik");

  valik = Valik;
  if (!vict->act_ptr)
    vict->act_ptr = (int*)calloc(1, sizeof(int));
  switch ((*((int*)vict->act_ptr))) {
    case Valik_Wandering:
    case Valik_Qone:
    case Valik_Qtwo:
    case Valik_Qthree:
      if (cmd == 72) { /* give */
        arg = one_argument(arg, obj_name);
        if ((!*obj_name) ||
            !(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)))
          return (FALSE);
        only_argument(arg, vict_name);
        if ((!*vict_name) || (!(vict = get_char_room_vis(ch, vict_name))))
          return (FALSE);
        /* the target is valik */
        if (mob_index[vict->nr].func.mob_f == valik) {
          act("You give $p to $N.", TRUE, ch, obj, vict, TO_CHAR);
          act("$n gives $p to $N.", TRUE, ch, obj, vict, TO_ROOM);
        } else
          return (FALSE);
        gave_this_click = TRUE;
      } else if (cmd == 86) { /* ask */
        arg = one_argument(arg, vict_name);
        if ((!*vict_name) || (!(vict = get_char_room_vis(ch, vict_name))))
          return (FALSE);
        if (!(mob_index[vict->nr].func.mob_f == valik))
          return (FALSE);
        else {
          if (!(strcmp(arg, " What is the quest of the Rhyodin?")))
            for (i = 0; i < 9; ++i) {
              sprintf(buf, "%s %s", GET_NAME(ch), quest_intro[i]);
              do_tell(vict, buf, 19);
            }
          return (TRUE);
        }
      }
      break;
    case Valik_Meditating:
      if (time_info.hours < 22 && time_info.hours > 5) {
        do_stand(ch, "", -1);
        act("$n says 'Perhaps today will be different.'", FALSE, ch, 0, 0,
          TO_ROOM);
        act("$n slowly fades out of existence.", FALSE, ch, 0, 0, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, valik_dests[number(0, 8)]);
        act("The world warps, dissolves, and reforms.", FALSE, ch, 0, 0,
          TO_ROOM);
        return (FALSE);
      } else {
        for (vict = real_roomp(ch->in_room)->people; vict;
             vict = vict->next_in_room)
          if (!IS_NPC(vict) && (GetMaxLevel(vict) < LOW_IMMORTAL) &&
              (number(0, 3) == 0)) {
            act("$n snaps out of his meditation.", FALSE, ch, 0, 0, TO_ROOM);
            do_stand(ch, "", -1);
            hit(ch, vict, TYPE_UNDEFINED);
            return (FALSE);
          }
        return (FALSE);
      }
      break;
    default:
      return (FALSE);
  }

  /* There are four valid objects */
  switch (*((int*)vict->act_ptr)) {
    case Valik_Wandering:
      if (gave_this_click) {
        /* Take it, in either case */
        obj_from_char(obj);
        obj_to_char(obj, vict);
        if (obj_index[obj->item_number].virtual == Shield) {
          if (!check_soundproof(ch)) {
            act("$N says 'The Shield of Lorces!'", FALSE, ch, 0, vict, TO_CHAR);
            act("$N says 'You may now undertake the first quest.'", FALSE, ch,
              0, vict, TO_CHAR);
          }
          *((int*)vict->act_ptr) = Valik_Qone;
        } else {
          act("$N takes the $p and bows in thanks.'", FALSE, ch, obj, vict,
            TO_CHAR);
          act("$N takes the $p from $n and bows in thanks.'", FALSE, ch, obj,
            vict, TO_ROOM);
          return (TRUE);
        }
      } else if (cmd)
        return (FALSE);
      else {
        if (time_info.hours > 21) {
          if (!check_soundproof(ch))
            act("$n says 'It is time to meditate.'", FALSE, ch, 0, 0, TO_ROOM);
          act("$n disappears in a flash of light!", FALSE, ch, 0, 0, TO_ROOM);
          char_from_room(ch);
          char_to_room(ch, Med_Chambers);
          act("Reality warps and spins around you!", FALSE, ch, 0, 0, TO_ROOM);
          sprintf(buf, "close mahogany");
          command_interpreter(ch, buf);
          do_rest(ch, "", -1);
          *((int*)ch->act_ptr) = Valik_Meditating;
          return (FALSE);
        }
        return (magic_user(ch, cmd, arg));
      }
      break;
    case Valik_Qone:
      if (gave_this_click)
        if (obj_index[obj->item_number].virtual == Ring) {
          if (!check_soundproof(ch)) {
            act("$N says 'You have brought me the ring of Tlanic.'", FALSE, ch,
              0, vict, TO_CHAR);
            act("$N says 'You may now undertake the second quest.'", FALSE, ch,
              0, vict, TO_CHAR);
          }
          obj_from_char(obj);
          obj_to_char(obj, vict);
          *((int*)vict->act_ptr) = Valik_Qtwo;
        } else {
          act("$N shakes his head - it is the wrong item.", FALSE, ch, 0, vict,
            TO_CHAR);
          act("$N gives $p back to you.", TRUE, ch, obj, vict, TO_CHAR);
          act("$N gives $p to $n.", TRUE, ch, obj, vict, TO_ROOM);
          return (TRUE);
        }
      else
        return (FALSE);
      break;
    case Valik_Qtwo:
      if (gave_this_click)
        if (obj_index[obj->item_number].virtual == Chalice) {
          if (!check_soundproof(ch)) {
            act("$N says 'You have brought me the chalice of Evistar.'", FALSE,
              ch, 0, vict, TO_CHAR);
            act("$N says 'You may now undertake the third quest.'", FALSE, ch,
              0, vict, TO_CHAR);
          }
          obj_from_char(obj);
          obj_to_char(obj, vict);
          *((int*)vict->act_ptr) = Valik_Qthree;
        } else {
          act("$N shakes his head - it is the wrong item.'", FALSE, ch, 0, 0,
            TO_CHAR);
          act("$N gives $p back to you.", TRUE, ch, obj, vict, TO_CHAR);
          act("$N gives $p to $n.", TRUE, ch, obj, vict, TO_ROOM);
          return (TRUE);
        }
      else
        return (FALSE);
      break;
    case Valik_Qthree:
      if (gave_this_click)
        if (obj_index[obj->item_number].virtual == Circlet) {
          if (!check_soundproof(ch)) {
            act("$N says 'You have brought me the circlet of C*zarnak.'", FALSE,
              ch, 0, vict, TO_CHAR);
            act("$N says 'You may now undertake the final quest.'", FALSE, ch,
              0, vict, TO_CHAR);
          }
          obj_from_char(obj);
          obj_to_char(obj, vict);
          *((int*)vict->act_ptr) = Valik_Qfour;
        } else {
          act("$N says 'That is not the item I require.'", FALSE, ch, 0, vict,
            TO_CHAR);
          act("$N gives $p back to you.", TRUE, ch, obj, vict, TO_CHAR);
          act("$N gives $p to $n.", TRUE, ch, obj, vict, TO_ROOM);
          return (TRUE);
        }
      else
        return (FALSE);
      break;
    default:
      return (FALSE);
  }

  /* The final switch, where we tell the player what the quests are */
  switch (*((int*)vict->act_ptr)) {
    case Valik_Qone:
      for (i = 0; i < quest_lines[(*((int*)vict->act_ptr)) - 2]; ++i)
        do_say(vict, quest_one[i], 0);
      return (TRUE);
      break;
    case Valik_Qtwo:
      for (i = 0; i < quest_lines[(*((int*)vict->act_ptr)) - 2]; ++i)
        do_say(vict, quest_two[i], 0);
      return (TRUE);
      break;
    case Valik_Qthree:
      for (i = 0; i < quest_lines[(*((int*)vict->act_ptr)) - 2]; ++i)
        do_say(vict, quest_three[i], 0);
      return (TRUE);
      break;
    case Valik_Qfour:
      if (obj_index[vict->equipment[WEAR_NECK_1]->item_number].virtual ==
          Necklace) {
        for (i = 0; i < quest_lines[(*((int*)vict->act_ptr)) - 2]; ++i)
          do_say(vict, necklace[i], 0);
        act("$N takes the Necklace of Wisdom and hands it to you.", FALSE, ch,
          0, vict, TO_CHAR);
        act("$N takes the Necklace of Wisdom and hands it to $n.", FALSE, ch, 0,
          vict, TO_ROOM);
        obj_to_char(unequip_char(vict, WEAR_NECK_1), ch);
      } else {
        for (i = 0; i < quest_lines[(*((int*)vict->act_ptr)) - 2]; ++i)
          do_say(vict, nonecklace[i], 0);
      }
      (*((int*)vict->act_ptr)) = Valik_Wandering;
      return (TRUE);
      break;
    default:
      vlog("Ack! Foo! Heimdall screws up!");
      return (FALSE);
  }
}

int guardian(struct char_data* ch, int cmd, char* arg) {
  assert(ch && "guardian proc: ch should not be NULL");

  static const char* const RHYODIN_FILE = "rhyodin";

  // If no cmd, only proceed if ch has the guardian proc assigned
  if (!cmd && mob_index[ch->nr].func.mob_f != guardian) {
    return FALSE;
  }

  // Initialize mob's act_ptr if this is the first time the proc is called (or
  // somehow it has the wrong type of act_ptr)
  if (!cmd && (!ch->act_ptr || ch->act_ptr->type != MOB_ACT_GUARDIAN)) {
    if (ch->act_ptr && ch->act_ptr->type != MOB_ACT_GUARDIAN) {
      vlog(
        "guardian proc: ch has guardian proc assigned but ch->act_ptr is not "
        "of type MOB_ACT_GUARDIAN. "
        "Resetting.");
      free(ch->act_ptr);
      ch->act_ptr = NULL;
    }

    struct mob_act_guardian* act_data = NULL;

    FILE* pass = fopen(RHYODIN_FILE, "r");

    if (!pass) {
      vlog("guardian proc: Rhyodin access file unreadable or non-existant");
      return FALSE;
    }

    CREATE(ch->act_ptr, struct mob_act_data, 1);
    ch->act_ptr->type = MOB_ACT_GUARDIAN;

    act_data = &ch->act_ptr->data.guardian;
    act_data->num_names = 0;
    act_data->names = NULL;

    char name[15];
    while (fscanf(pass, " %s\n", name)) {
      ++act_data->num_names;
      RECREATE(act_data->names, char*, act_data->num_names);
      CREATE(act_data->names[act_data->num_names - 1], char, 15);
      strcpy(act_data->names[act_data->num_names - 1], name);
    }
  }

  // Default behavior when proc executes without a cmd
  if (!cmd) {
    struct char_data* opponent = ch->specials.fighting;

    if (!opponent || !AWAKE(ch)) {
      return FALSE;
    }

    if (!IS_PC(opponent)) {
      struct char_data* master = opponent->master;

      if (master && CAN_SEE(ch, master)) {
        stop_fighting(ch);
        set_fighting(ch, master);
      }
    }

    if (GET_POS(ch) == POSITION_FIGHTING) {
      FighterMove(ch);
    } else {
      StandUp(ch);
    }

    return TRUE;
  }

  if (!IS_PC(ch)) {
    return FALSE;
  }

  if (cmd == 72) {
    struct obj_data* obj;
    char obj_name[80];
    arg = one_argument(arg, obj_name);

    if (!*obj_name ||
        !(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
      return FALSE;
    }

    char player_name[80];
    only_argument(arg, player_name);

    struct char_data* g;
    if (!*player_name || !(g = get_char_room_vis(ch, player_name))) {
      return FALSE;
    }

    if (mob_index[g->nr].func.mob_f != guardian) {
      return FALSE;
    }

    act("You give $p to $N.", TRUE, ch, obj, g, TO_CHAR);
    act("$n gives $p to $N.", TRUE, ch, obj, g, TO_ROOM);

    if (obj_index[obj->item_number].virtual != Necklace) {
      act("$n refuses to accept $p.", TRUE, g, obj, 0, TO_ROOM);
      return FALSE;
    }

    const bool room_soundproof = check_soundproof(ch);

    FILE* pass = fopen(RHYODIN_FILE, "a");

    if (!pass) {
      vlog("ERROR: Couldn't open file for writing permanent Rhyodin passlist.");
      if (!room_soundproof) {
        act("$n says 'I'm sorry, but I can't do that.'", FALSE, g, 0, 0,
          TO_ROOM);
      }
      act("$N gives $p to you.", TRUE, ch, obj, g, TO_CHAR);
      act("$N gives $p to $n.", TRUE, ch, obj, g, TO_ROOM);
      return FALSE;
    }

    /* Go to the end of the file and write the character's name */
    fprintf(pass, " %s\n", GET_NAME(ch));
    fclose(pass);

    // Add the name to the mob's act_ptr
    struct mob_act_guardian* act_data = &g->act_ptr->data.guardian;
    ++act_data->num_names;
    RECREATE(act_data->names, char*, act_data->num_names);
    CREATE(act_data->names[act_data->num_names - 1], char, 15);
    strcpy(act_data->names[act_data->num_names - 1], ch->player.name);

    /* Take it away */
    obj_from_char(obj);
    extract_obj(obj);
    act("$p pulses in $s hand, and disappears.", FALSE, g, obj, 0, TO_ROOM);

    if (!room_soundproof) {
      act("$N says 'You have proven youself worthy.'", FALSE, ch, 0, g,
        TO_CHAR);
      act("$N says 'You are now an ambassador from the north to Rhyodin.'",
        FALSE, ch, 0, g, TO_CHAR);
    }

    struct room_data* rp = real_roomp(ch->in_room);
    int to_room = NOWHERE;

    if (!rp || !rp->dir_option[2] ||
        (to_room = rp->dir_option[2]->to_room) == NOWHERE ||
        ch->specials.fighting) {
      return FALSE;
    }

    /* Okay, now take person and all followers in this room to next room */
    act("$N opens the gate and guides you through.", FALSE, ch, 0, g, TO_CHAR);

    char_from_room(ch);
    char_to_room(ch, to_room);
    do_look(ch, "\0", 0);

    /* First level followers can tag along */
    if (ch->followers) {
      act("$n says 'If they're with you, they can enter as well.'", FALSE, g, 0,
        0, TO_ROOM);

      for (struct follow_type* fol = ch->followers; fol; fol = fol->next) {
        if (fol->follower->specials.fighting)
          continue;

        if (real_roomp(fol->follower->in_room) &&
            EXIT(fol->follower, 2)->to_room != NOWHERE &&
            GET_POS(fol->follower) >= POSITION_STANDING) {
          char_from_room(fol->follower);
          char_to_room(fol->follower, to_room);
          do_look(fol->follower, "\0", 0);
        }
      }
    }

    return TRUE;
  }

  // Trying to move south, check against namelist
  if (cmd == 3) {
    struct char_data* g = FindMobInRoomWithFunction(ch->in_room, guardian);

    assert(g);

    struct mob_act_guardian* gstruct = &g->act_ptr->data.guardian;

    for (int j = 0; j < gstruct->num_names; ++j) {
      if (strcmp(gstruct->names[j], GET_NAME(ch))) {
        continue;
      }

      struct room_data* rp = real_roomp(ch->in_room);
      int to_room = NOWHERE;

      if (!rp || !rp->dir_option[2] ||
          (to_room = rp->dir_option[2]->to_room) == NOWHERE ||
          ch->specials.fighting) {
        return FALSE;
      }

      act("$N recognizes you, and escorts you through the gate.", FALSE, ch, 0,
        g, TO_CHAR);
      act("$N recognizes $n, and escorts them through the gate.", FALSE, ch, 0,
        g, TO_ROOM);

      char_from_room(ch);
      char_to_room(ch, to_room);
      do_look(ch, "\0", 0);

      /* Follower stuff again */
      if (ch->followers) {
        act("$N says 'If they're with you, they can enter as well.'", FALSE, ch,
          0, g, TO_CHAR);

        for (struct follow_type* fol = ch->followers; fol; fol = fol->next) {
          if (fol->follower->specials.fighting)
            continue;

          if (real_roomp(fol->follower->in_room) &&
              EXIT(fol->follower, 2)->to_room != NOWHERE &&
              GET_POS(fol->follower) >= POSITION_STANDING) {
            char_from_room(fol->follower);
            char_to_room(fol->follower, to_room);
            do_look(fol->follower, "\0", 0);
          }
        }
      }
    }

    return TRUE;
  }

  return FALSE;
}

int bouncer(struct char_data* ch, int cmd, char* arg) {
  struct char_data *tmp_victim, *temp;
  int dir;

  if (cmd || !AWAKE(ch) || !ch->specials.fighting)
    return (FALSE);

  for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next;
    if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
      if (tmp_victim->specials.fighting) {
        if (tmp_victim->specials.fighting == ch) {
          if (!saves_spell(tmp_victim, SAVING_PETRI)) {
            BouncerThrow(tmp_victim);
          }
        }
      }
    }
  }
}

int gilbert(struct char_data* ch, int cmd, char* arg) {
  struct char_data* victim;
  struct obj_data* o;
  int num;
  char buf[255];

  if (cmd)
    return (FALSE);

  if (!AWAKE(ch))
    return (FALSE);

  num = number(1, 2);

  if (num == 1)
    return (FALSE);

  if (!ch->specials.fighting)
    return (FALSE);

  victim = ch->specials.fighting;

  act(
    "$n becomes disgusted by $N and pulls a scroll of recall from his pocket.",
    FALSE, ch, 0, victim, TO_ROOM);
  o = read_object(3052, VIRTUAL);
  obj_to_char(o, ch);
  send_to_room("Gilbert screams ' Scum like you don't belong in my bar!\n\r",
    ch->in_room);
  if (!saves_spell(victim, SAVING_PARA)) {
    sprintf(buf, "scroll %s", GET_NAME(victim));
    do_recite(ch, buf, 207);
    sprintf(buf, "%s .......and stay out.", GET_NAME(victim));
    do_tell(ch, buf, 19);
  } else {
    send_to_char(
      "As Gilbert is about to recite the scroll, you jolt his arm and he drops "
      "it.\n\r",
      victim);
    act("$N slams $n into the wall and jolts the scroll from his hand.", FALSE,
      ch, 0, victim, TO_NOTVICT);
    obj_from_char(o);
    obj_to_room(o, ch->in_room);
  }
}

int toilet_thing(struct char_data* ch, int cmd, char* arg) {
  struct char_data *tmp_victim, *temp;
  int dam;

  if (cmd)
    return (FALSE);

  if (!AWAKE(ch))
    return (FALSE);

  if (!ch->specials.fighting)
    return (FALSE);

  dam = 40 + (dice(1, 15));

  send_to_room(
    "The thing floating in the toilet spews forth filth and noxious crud.\n\r",
    ch->in_room);

  for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next;
    if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
      if (IS_IMMORTAL(tmp_victim))
        send_to_char("You are unaffected by the funk from the toilet.\n\r",
          tmp_victim);
      else
        MissileDamage(ch, tmp_victim, dam, 300);
    }
  }
}

int zombie_hater(struct char_data* ch, int cmd, char* arg) {
  struct char_data *vict, *next_v;

  if (cmd || !AWAKE(ch) || !ch->specials.fighting)
    return (FALSE);

  for (vict = real_roomp(ch->in_room)->people; vict; vict = next_v) {
    next_v = vict->next_in_room;
    if (GET_RACE(vict) == RACE_UNDEAD) {
      act("$n screams 'How dare you let an undead creature attack me!", FALSE,
        ch, 0, vict, TO_ROOM);
      act("$n waves $s hands and totally demolishes $N", FALSE, ch, 0, vict,
        TO_ROOM);
      extract_char(vict);
    }
  }
}

int dishboy(struct char_data* ch, int cmd, char* arg) {
  struct char_data *tmp_victim, *temp;
  int dam;

  if (cmd || !AWAKE(ch) || !ch->specials.fighting)
    return (FALSE);

  dam = dice(1, 10) + 30;

  send_to_room("The dishboy pulls out a hose and aims it at you.\n\r",
    ch->in_room);
  for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next;
    if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
      if (IS_IMMORTAL(tmp_victim))
        send_to_char("The dishboys spray is turned away by your power.\n\r",
          ch);
      else
        MissileDamage(ch, tmp_victim, dam, 301);
    }
  }
}

int game_wizard(struct char_data* ch, int cmd, char* arg) {
  struct char_data *tmp_victim, *temp;
  int dam;

  if (cmd || !AWAKE(ch) || !ch->specials.fighting)
    return (FALSE);

  dam = dice(1, 15) + 25;

  send_to_room(
    "The Game Wizard pulls a Super-Bomb from within his outfit and hurls it at "
    "you...\n\r",
    ch->in_room);

  for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next;
    if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
      if (IS_IMMORTAL(tmp_victim))
        send_to_char(
          "You spit on the game wizards super bomb and dissolve it.\n\r",
          tmp_victim);
      else
        MissileDamage(ch, tmp_victim, dam, 302);
    }
  }
}

/* monks */

static void MonkMove(struct char_data* ch) {
  if (!ch->skills) {
    SpaceForSkills(ch);
    ch->skills[SKILL_DODGE].learned = GetMaxLevel(ch) + 50;
    SET_BIT(ch->player.class, CLASS_MONK);
  }

  if (!ch->specials.fighting)
    return;

  if (GET_POS(ch) < POSITION_FIGHTING) {
    if (!ch->skills[SKILL_SPRING_LEAP].learned)
      ch->skills[SKILL_SPRING_LEAP].learned = (GetMaxLevel(ch) * 3) / 2 + 25;
    do_springleap(ch, GET_NAME(ch->specials.fighting), 0);
    return;
  } else {
    if (GetMaxLevel(ch) > 30 && !number(0, 4)) {
      if (GetMaxLevel(ch->specials.fighting) <= GetMaxLevel(ch)) {
        if (GET_MAX_HIT(ch->specials.fighting) < 2 * GET_MAX_HIT(ch)) {
          if (!affected_by_spell(ch, SKILL_QUIV_PALM)) {
            if (!ch->skills[SKILL_QUIV_PALM].learned)
              ch->skills[SKILL_QUIV_PALM].learned = GetMaxLevel(ch) * 2 - 5;
            do_quivering_palm(ch, GET_NAME(ch->specials.fighting), 0);
            return;
          }
        }
      }
    }
    if (ch->specials.fighting->equipment[WIELD]) {
      if (!ch->skills[SKILL_DISARM].learned)
        ch->skills[SKILL_DISARM].learned = (GetMaxLevel(ch) * 3) / 2 + 25;
      do_disarm(ch, GET_NAME(ch->specials.fighting), 0);
      return;
    }
    if (!ch->skills[SKILL_KICK].learned)
      ch->skills[SKILL_KICK].learned = (GetMaxLevel(ch) * 3) / 2 + 25;
    if (number(1, 4) == 1)
      do_kick(ch, GET_NAME(ch->specials.fighting), 0);
  }
}

int monk(struct char_data* ch, int cmd, char* arg) {
  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    MonkMove(ch);
  }
  return (FALSE);
}

int monk_master(struct char_data* ch, int cmd, char* arg) {
  char buf[256];
  static char* n_skills[] = {
    "quivering palm", /* No. 245 */
    "feign death",    /* No. 259 */
    "retreat",
    "kick",
    "hide",
    "sneak",
    "pick locks",
    "disarm",
    "dodge",
    "switch opponents",
    "springleap",
    "\n",
  };
  int percent = 0, number = 0;
  int charge, sk_num, mult;

  if (!AWAKE(ch))
    return (FALSE);

  if (!cmd) {
    if (ch->specials.fighting) {
      return (fighter(ch, cmd, arg));
    }
    return (FALSE);
  }

  if (!ch->skills)
    return (FALSE);

  if (check_soundproof(ch))
    return (FALSE);

  for (; *arg == ' '; arg++)
    ; /* ditch spaces */

  if ((cmd == 164) || (cmd == 170)) {
    if (!arg || (strlen(arg) == 0)) {
      sprintf(buf, "You have %d practices remaining\n\r",
        ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      sprintf(buf, " disarm  :        %s\n\r",
        how_good(ch->skills[SKILL_DISARM].learned));
      send_to_char(buf, ch);
      sprintf(buf, " quivering palm:  %s [Must be 30th level]\n\r",
        how_good(ch->skills[SKILL_QUIV_PALM].learned));
      send_to_char(buf, ch);
      sprintf(buf, " springleap:      %s\n\r",
        how_good(ch->skills[SKILL_SPRING_LEAP].learned));
      send_to_char(buf, ch);
      sprintf(buf, " retreat:         %s\n\r",
        how_good(ch->skills[SKILL_RETREAT].learned));
      send_to_char(buf, ch);
      sprintf(buf, " pick locks:      %s\n\r",
        how_good(ch->skills[SKILL_PICK_LOCK].learned));
      send_to_char(buf, ch);
      sprintf(buf, " hide:            %s\n\r",
        how_good(ch->skills[SKILL_HIDE].learned));
      send_to_char(buf, ch);
      sprintf(buf, " sneak:           %s\n\r",
        how_good(ch->skills[SKILL_SNEAK].learned));
      send_to_char(buf, ch);
      sprintf(buf, " feign death:     %s\n\r",
        how_good(ch->skills[SKILL_FEIGN_DEATH].learned));
      send_to_char(buf, ch);
      sprintf(buf, " dodge:           %s\n\r",
        how_good(ch->skills[SKILL_DODGE].learned));
      send_to_char(buf, ch);
      sprintf(buf, " switch:          %s\n\r",
        how_good(ch->skills[SKILL_SWITCH_OPP].learned));
      send_to_char(buf, ch);
      sprintf(buf, " kick:            %s\n\r",
        how_good(ch->skills[SKILL_KICK].learned));
      send_to_char(buf, ch);
      return (TRUE);
    } else {
      number = old_search_block(arg, 0, strlen(arg), n_skills, FALSE);
      send_to_char("The ancient master says ", ch);
      if (number == -1) {
        send_to_char("'I do not know of this skill.'\n\r", ch);
        return (TRUE);
      }
      charge = GetMaxLevel(ch) * 100;
      switch (number) {
        case 0:
        case 1:
          sk_num = SKILL_QUIV_PALM;
          if (!HasClass(ch, CLASS_MONK)) {
            send_to_char("'You do not possess the proper skills'\n\r", ch);
            return (TRUE);
          } else if (GET_LEVEL(ch, MONK_LEVEL_IND) < 30) {
            send_to_char("'You are not high enough level'\n\r", ch);
            return (TRUE);
          }
          break;
        case 2:
          sk_num = SKILL_FEIGN_DEATH;
          if (!HasClass(ch, CLASS_MONK)) {
            send_to_char("'You do not possess the proper skills'\n\r", ch);
            return (TRUE);
          }
          break;
        case 3:
          sk_num = SKILL_RETREAT;
          if (!HasClass(ch, CLASS_WARRIOR) && !(HasClass(ch, CLASS_MONK))) {
            send_to_char(
              "'You do not possess the necessary fighting skills'\n\r", ch);
            return (TRUE);
          }
          break;
        case 4:
          sk_num = SKILL_KICK;
          if (!HasClass(ch, CLASS_WARRIOR) && !(HasClass(ch, CLASS_MONK))) {
            send_to_char(
              "'You do not possess the necessary fighting skills'\n\r", ch);
            return (TRUE);
          }
          break;
        case 5:
          sk_num = SKILL_HIDE;
          if (!HasClass(ch, CLASS_THIEF) && !(HasClass(ch, CLASS_MONK))) {
            send_to_char("'You do not possess the necessary skills'\n\r", ch);
            return (TRUE);
          }
          break;
        case 6:
          sk_num = SKILL_SNEAK;
          if (!HasClass(ch, CLASS_THIEF) && !(HasClass(ch, CLASS_MONK))) {
            send_to_char("'You do not possess the necessary skills'\n\r", ch);
            return (TRUE);
          }
          break;
        case 7:
          sk_num = SKILL_PICK_LOCK;
          if (!HasClass(ch, CLASS_THIEF) && !(HasClass(ch, CLASS_MONK))) {
            send_to_char("'You do not possess the necessary skills'\n\r", ch);
            return (TRUE);
          }
          break;
        case 8:
          sk_num = SKILL_DISARM;
          if (!HasClass(ch, CLASS_WARRIOR) && !(HasClass(ch, CLASS_MONK))) {
            send_to_char(
              "'You do not possess the necessary fighting skills'\n\r", ch);
            return (TRUE);
          }
          break;
        case 9:
          sk_num = SKILL_DODGE;
          if (!HasClass(ch, CLASS_WARRIOR) && !(HasClass(ch, CLASS_MONK))) {
            send_to_char(
              "'You do not possess the necessary fighting skills'\n\r", ch);
            return (TRUE);
          }
          break;
        case 10:
          sk_num = SKILL_SWITCH_OPP;
          if (!HasClass(ch, CLASS_WARRIOR) && !(HasClass(ch, CLASS_MONK))) {
            send_to_char(
              "'You do not possess the necessary fighting skills'\n\r", ch);
            return (TRUE);
          }
          break;
        case 11:
          sk_num = SKILL_SPRING_LEAP;
          if (!HasClass(ch, CLASS_MONK)) {
            send_to_char("'You do not possess the proper skills'\n\r", ch);
            return (TRUE);
          }
          break;

        default:
          sprintf(buf, "Strangeness in monk master (%d)", number);
          vlog(buf);
          send_to_char("'Ack!  I feel faint!'\n\r", ch);
          return (TRUE);
      }
    }

    if (!HasClass(ch, CLASS_MONK) && GET_GOLD(ch) < charge) {
      send_to_char("'Ah, but you do not have enough money to pay.'\n\r", ch);
      return (TRUE);
    }

    if ((sk_num == SKILL_SNEAK) || (sk_num == SKILL_DODGE)) {
      if (ch->skills[sk_num].learned >= 95) {
        send_to_char(
          "'You are a master of this art, I can teach you no more.'\n\r", ch);
        return (TRUE);
      }
    } else {
      if (ch->skills[sk_num].learned > 45) {
        send_to_char("'You must learn from practice and experience now.'\n\r",
          ch);
        return TRUE;
      }
    }

    if (ch->specials.spells_to_learn <= 0) {
      send_to_char("'You must first use the knowledge you already have.\n\r",
        ch);
      return (TRUE);
    }

    if (!HasClass(ch, CLASS_MONK)) {
      GET_GOLD(ch) -= charge;
    }
    send_to_char("'We will now begin.'\n\r", ch);
    ch->specials.spells_to_learn--;

    percent = ch->skills[sk_num].learned + int_app[GET_INT(ch)].learn;
    ch->skills[sk_num].learned = MIN(95, percent);

    if (ch->skills[sk_num].learned >= 95) {
      send_to_char("'You are now a master of this art.'\n\r", ch);
      return (TRUE);
    }
  } else if (cmd == 243) {
    if (HasClass(ch, CLASS_MONK)) {
      GainLevel(ch, MONK_LEVEL_IND);
      return (TRUE);
    }

  } else {
    return (FALSE);
  }
}

#define NOD 35
#define MONK_MOB 650
#define FLEE 151

int monk_challenge_room(struct char_data* ch, int cmd, char* arg) {
  struct char_data* i;
  struct room_data* me;
  int rm;

  rm = ch->in_room;

  me = real_roomp(ch->in_room);
  if (!me)
    return (FALSE);

  if (!me->river_speed)
    return (FALSE);

  if (IS_PC(ch)) {
    REMOVE_BIT(ch->specials.act, PLR_WIMPY);
  }

  if (cmd == FLEE) {
    /* this person just lost */
    send_to_char("You lose\n\r", ch);
    if (IS_PC(ch)) {
      GET_EXP(ch) = MIN(
        titles[MONK_LEVEL_IND][GET_LEVEL(ch, MONK_LEVEL_IND)].exp, GET_EXP(ch));
      send_to_char("Go home\n\r", ch);
      char_from_room(ch);
      char_to_room(ch, rm - 1);
      me->river_speed = 0;
      return (TRUE);
    } else {
      extract_char(ch);
      /*
        find pc in room;
        */
      for (i = me->people; i; i = i->next_in_room)
        if (IS_PC(i)) {
          GET_EXP(i) = MAX(
            titles[MONK_LEVEL_IND][GET_LEVEL(i, MONK_LEVEL_IND) + 1].exp + 1,
            GET_EXP(i));
          send_to_char(
            "You win. You are obviously ready for the next level.\n\r", i);
          GainLevel(i, MONK_LEVEL_IND);
          char_from_room(i);
          char_to_room(i, rm - 1);

          while (me->people)
            extract_char(me->people);

          while (me->contents)
            extract_obj(me->contents);

          me->river_speed = 0;
          return (TRUE);
        } else {
          return (FALSE);
        }
    }
  }
  return (FALSE);
}

int monk_challenge_prep_room(struct char_data* ch, int cmd, char* arg) {
  struct room_data *me, *chal;
  int i, newr;
  struct obj_data* o;
  struct char_data* mob;

  me = real_roomp(ch->in_room);
  if (!me)
    return (FALSE);

  chal = real_roomp(ch->in_room + 1);
  if (!chal) {
    send_to_char("The challenge room is gone.. please contact a god\n\r", ch);
    return (TRUE);
  }

  if (cmd == NOD) {
    if (!HasClass(ch, CLASS_MONK)) {
      send_to_char("You're no monk\n\r", ch);
      return (FALSE);
    }

    if (GET_LEVEL(ch, MONK_LEVEL_IND) < 10) {
      send_to_char("You have no business here, kid.\n\r", ch);
      return (FALSE);
    }

    if (GET_EXP(ch) <=
        titles[MONK_LEVEL_IND][GET_LEVEL(ch, MONK_LEVEL_IND) + 1].exp - 100) {
      send_to_char("You cannot advance now\n\r", ch);
      return (TRUE);
    }

    if (chal->river_speed != 0) {
      send_to_char("The challenge room is busy.. please wait\n\r", ch);
      return (TRUE);
    }
    for (i = 0; i < MAX_WEAR; i++) {
      if (ch->equipment[i]) {
        o = unequip_char(ch, i);
        obj_to_char(o, ch);
      }
    }
    while (ch->carrying)
      extract_obj(ch->carrying);

    spell_dispel_magic(IMPLEMENTOR, ch, ch, 0);
    send_to_char("You are taken into the combat room.\n\r", ch);
    act("$n is ushered into the combat room", FALSE, ch, 0, 0, TO_ROOM);
    newr = ch->in_room + 1;
    char_from_room(ch);
    char_to_room(ch, newr);
    /* load the mob at the same lev as char */
    mob = read_mobile(MONK_MOB + GET_LEVEL(ch, MONK_LEVEL_IND) - 10, VIRTUAL);
    if (!mob) {
      send_to_char("The fight is called off.  go home\n\r", ch);
      return (TRUE);
    }
    char_to_room(mob, ch->in_room);
    chal->river_speed = 1;
    do_look(ch, "", 0);
    REMOVE_BIT(ch->specials.act, PLR_WIMPY);
    return (TRUE);
  }

  return (FALSE);
}

/* Bjs Shit Begin */

#define Bandits_Path 2180
#define BASIL_GATEKEEPER_MAX_LEVEL 10
#define Fountain_Level 20
#define DEMON 29000

int ghost(struct char_data* ch, int cmd, char* arg) {
  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {
    act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
      ch->specials.fighting, 0);

    return TRUE;
  }
  return FALSE;
}

int Magic_Fountain(struct char_data* ch, int cmd, char* arg) {
  int bits, water, level;
  char buf[MAX_INPUT_LENGTH];
  struct char_data *tmp_char, *mob;
  struct obj_data* obj;
  int cost;

  if (cmd == 11) { /* drink */

    cost = ((3 * GetMaxLevel(ch)) * (2 * GetMaxLevel(ch)));

    if (GET_GOLD(ch) < cost) {
      send_to_char("You don't have enough money to use the magic fountain.\n\r",
        ch);
      return (FALSE);
    } else {
      sprintf(buf, "You drop %d coins in the fountain\n\r", cost);
      send_to_char(buf, ch);
      GET_GOLD(ch) -= cost;
    }

    only_argument(arg, buf);

    if (str_cmp(buf, "fountain") && str_cmp(buf, "water")) {
      return (FALSE);
    }

    send_to_char("You drink from the fountain\n\r", ch);
    act("$n drinks from the fountain", FALSE, ch, 0, 0, TO_ROOM);

    switch (number(0, 45)) {
        /* Lets try and make 1-10 Good, 11-26 Bad, 27-40 Nothing */
      case 1:
        cast_refresh(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
      case 2:
        cast_stone_skin(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
      case 3:
        cast_cure_serious(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
      case 4:
        cast_cure_light(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
      case 5:
        cast_armor(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
      case 6:
        cast_bless(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
      case 7:
        cast_invisibility(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
      case 8:
        cast_strength(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
      case 9:
        cast_remove_poison(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
      case 10:
        cast_true_seeing(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;

        /* Time for the nasty Spells */

      case 11:
        send_to_room("A large demon appears from the fountain.\n\r",
          ch->in_room);
        mob = read_mobile(DEMON, VIRTUAL);
        char_to_room(mob, ch->in_room);
        act("$n utters the words, 'All your magic will now cease.'", FALSE, mob,
          0, ch, TO_ROOM);
        cast_dispel_magic(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
        extract_char(mob);
        send_to_char("And just as soon disappears.....\n\r", ch);
        break;
      case 12:
        send_to_room("A large demon appears from the fountain.\n\r",
          ch->in_room);
        mob = read_mobile(DEMON, VIRTUAL);
        char_to_room(mob, ch->in_room);
        act("$n utters the words, 'Get out of my face!'", FALSE, mob, 0, ch,
          TO_ROOM);
        cast_teleport(GetMaxLevel(mob), ch, "", SPELL_TYPE_SPELL, ch, 0);
        extract_char(mob);
        send_to_char("And just as soon disappears....\n\r", ch);
        break;
      case 13:
        send_to_room("A large demon appears from the fountain.\n\r",
          ch->in_room);
        mob = read_mobile(DEMON, VIRTUAL);
        char_to_room(mob, ch->in_room);
        act("$n utters the words, 'Let's get sticky!'", FALSE, mob, 0, ch,
          TO_ROOM);
        cast_web(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
        extract_char(mob);
        send_to_char("And just as soon disappears....\n\r", ch);
        break;
      case 14:
        send_to_room("A large demon appears from the fountain.\n\r",
          ch->in_room);
        mob = read_mobile(DEMON, VIRTUAL);
        char_to_room(mob, ch->in_room);
        act("$n utters the words, 'Does this hurt?'", FALSE, mob, 0, ch,
          TO_ROOM);
        cast_curse(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
        extract_char(mob);
        send_to_char("And just as soon disappears....\n\r", ch);
        break;
      case 15:
        send_to_room("A large demon appears from the fountain.\n\r",
          ch->in_room);
        mob = read_mobile(DEMON, VIRTUAL);
        char_to_room(mob, ch->in_room);
        act("$n utters the words, 'Ray Charles'", FALSE, mob, 0, ch, TO_ROOM);
        cast_blindness(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
        extract_char(mob);
        send_to_char("And just as soon disappears....\n\r", ch);
        break;
      case 16:
        send_to_room("A large demon appears from the fountain.\n\r",
          ch->in_room);
        mob = read_mobile(DEMON, VIRTUAL);
        char_to_room(mob, ch->in_room);
        act("$n utters the words, 'Wimpy girly man'", FALSE, mob, 0, ch,
          TO_ROOM);
        cast_weakness(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
        extract_char(mob);
        send_to_char("And just as soon disappears....\n\r", ch);
        break;
      case 17:
        send_to_room("A large demon appears from the fountain.\n\r",
          ch->in_room);
        mob = read_mobile(DEMON, VIRTUAL);
        char_to_room(mob, ch->in_room);
        act("$n utters the words, 'You look sickly'", FALSE, mob, 0, ch,
          TO_ROOM);
        cast_poison(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
        extract_char(mob);
        send_to_char("And just as soon disappears....\n\r", ch);
        break;
      case 18:
        send_to_room("A large demon appears from the fountain.\n\r",
          ch->in_room);
        mob = read_mobile(DEMON, VIRTUAL);
        char_to_room(mob, ch->in_room);
        act("$n utters the words, 'This will only hurt a little.'", FALSE, mob,
          0, ch, TO_ROOM);
        cast_cause_light(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
        extract_char(mob);
        send_to_char("And just as soon disappears....\n\r", ch);
        break;
      case 19:
        send_to_room("A large demon appears from the fountain.\n\r",
          ch->in_room);
        mob = read_mobile(DEMON, VIRTUAL);
        char_to_room(mob, ch->in_room);
        act("$n utters the words, 'This will only hurt a good bit.'", FALSE,
          mob, 0, ch, TO_ROOM);
        cast_cause_critic(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
        extract_char(mob);
        send_to_char("And just as soon disappears....\n\r", ch);
        break;
      case 20:
      case 21:
        send_to_room("A large demon appears from the fountain.\n\r",
          ch->in_room);
        mob = read_mobile(DEMON, VIRTUAL);
        char_to_room(mob, ch->in_room);
        act("$n utters the words, 'Fourth of July!'", FALSE, mob, 0, ch,
          TO_ROOM);
        cast_magic_missile(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
        extract_char(mob);
        send_to_char("And just as soon disappears....\n\r", ch);
        break;
      case 22:
        send_to_room("A large demon appears from the fountain.\n\r",
          ch->in_room);
        mob = read_mobile(DEMON, VIRTUAL);
        char_to_room(mob, ch->in_room);
        act("$n utters the words, 'Ah, easier to hit!'", FALSE, mob, 0, ch,
          TO_ROOM);
        cast_faerie_fire(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
        extract_char(mob);
        send_to_char("And just as soon disappears....\n\r", ch);
        break;
      case 23:
        cast_flamestrike(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
      case 24:
        cast_burning_hands(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
      case 25:
        cast_acid_blast(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
      case 26:
        cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
      case 27:
      case 28:
      case 29:
      case 30:
      case 31:
      case 32:
      case 33:
      case 34:
      case 35:
      case 36:
      case 37:
      case 38:
      case 39:
      case 40: {
        send_to_char("The fluid tastes like dry sand in your mouth.\n\r", ch);
        break;
      }
      case 41: {
        cast_heal(40, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
      }
      case 42: {
        cast_full_heal(40, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
      }
      case 43: {
        send_to_room("A large demon appears from the fountain.\n\r",
          ch->in_room);
        mob = read_mobile(DEMON, VIRTUAL);
        char_to_room(mob, ch->in_room);
        act("$n utters the words, 'This is gonna hurt.'", FALSE, mob, 0, ch,
          TO_ROOM);
        cast_meteor_swarm(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
        extract_char(mob);
        send_to_char("And just as soon disappears....\n\r", ch);
        break;
      }
      case 44: {
        send_to_room("A large demon appears from the fountain.\n\r",
          ch->in_room);
        mob = read_mobile(DEMON, VIRTUAL);
        char_to_room(mob, ch->in_room);
        act("$n utters the words, 'BZZZZZZZZZZZZZZZZT'", FALSE, mob, 0, ch,
          TO_ROOM);
        cast_lightning_bolt(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL, ch, 0);
        extract_char(mob);
        send_to_char("And just as soon disappears....\n\r", ch);
        break;
      }
      default: {
        send_to_char("The water feels cool and refreshing..\n\r", ch);
        break;
      }
    }
    return (TRUE);
  }

  /* All commands except fill and drink */
  return (FALSE);
}

typedef struct T1000_data {
    int state;
    struct char_data* vict;
} T1000_data;

#if 0
#define T1000_SEARCHING 0
#define T1000_HUNTING 1

int T1000( struct char_data *ch, char *line, int cmd)
{
   int count;
   struct descriptor_data *i;

   if (!ch->act_ptr)
      ch->act_ptr = (int *) calloc(1, sizeof(int));
   if (ch->specials.hunting == 0)
     (*((int *) ch->act_ptr)) = T1000_SEARCHING;

    switch(*((int *) ch->act_ptr)) {
       case T1000_SEARCHING: {
     count = number(0,200);
     for (i = descriptor_list; count>0; i= i->next) {
        if (!i) {
          i = descriptor_list;
        }
     }
     if (i) {
        ch->specials.hunting = i->character;
        (*((int *) ch->act_ptr)) = T1000_HUNTING;
     }
       }
       case T1000_HUNTING: {
     if (ch->in_room == ch->specials.hunting->in_room) {
     } else {
     }
       }
    }
}
#endif

void invert(char* arg1, char* arg2) {
  register int i = 0;
  register int len = strlen(arg1) - 1;

  while (i <= len) {
    *(arg2 + i) = *(arg1 + (len - i));
    i++;
  }
  *(arg2 + i) = '\0';
}

int jive_box(Mob* ch, int cmd, char* arg, Obj* me) {
  char buf[255], buf2[255], buf3[255], tmp[255];

  switch (cmd) {
    case 17:
    case 169:
      invert(arg, buf);
      do_say(ch, buf, cmd);
      return TRUE;
      break;
    case 19:
      half_chop(arg, tmp, buf);
      invert(buf, buf2);
      sprintf(buf3, "%s %s", tmp, buf);
      do_tell(ch, buf3, cmd);
      return TRUE;
      break;
    case 18:
      invert(arg, buf);
      do_shout(ch, buf, cmd);
      return TRUE;
      break;
    default:
      return FALSE;
  }
}

int new_ninja_master(struct char_data* ch, int cmd, char* arg) {
  char buf[256];
  static char* n_skills[] = {
    "disarm",   /* No. 245 */
    "doorbash", /* No. 259 */
    "spy",
    "retreat",
    "\n",
  };
  int percent = 0, number = 0;
  int charge, sk_num, mult;

  if (!AWAKE(ch))
    return (FALSE);

  if (!cmd) {
    if (ch->specials.fighting) {
      return (fighter(ch, cmd, arg));
    }
    return (FALSE);
  }

  if (!ch->skills)
    return (FALSE);

  if (check_soundproof(ch))
    return (FALSE);

  for (; *arg == ' '; arg++)
    ; /* ditch spaces */

  if ((cmd == 164) || (cmd == 170)) {
    if (!arg || (strlen(arg) == 0)) {
      sprintf(buf, " disarm  :  %s\n\r",
        how_good(ch->skills[SKILL_DISARM].learned));
      send_to_char(buf, ch);
      sprintf(buf, " doorbash:  %s\n\r",
        how_good(ch->skills[SKILL_DOORBASH].learned));
      send_to_char(buf, ch);
      sprintf(buf, " spy:       %s\n\r",
        how_good(ch->skills[SKILL_SPY].learned));
      send_to_char(buf, ch);
      sprintf(buf, " retreat:   %s\n\r",
        how_good(ch->skills[SKILL_RETREAT].learned));
      send_to_char(buf, ch);
      return (TRUE);
    } else {
      number = old_search_block(arg, 0, strlen(arg), n_skills, FALSE);
      send_to_char("The ninja master says ", ch);
      if (number == -1) {
        send_to_char("'I do not know of this skill.'\n\r", ch);
        return (TRUE);
      }
      charge = GetMaxLevel(ch) * 1000;
      switch (number) {
        case 0:
        case 1:
          sk_num = SKILL_DISARM;
          if (!HasClass(ch, CLASS_WARRIOR)) {
            send_to_char(
              "'You do not possess the necessary fighting skills'\n\r", ch);
            return (TRUE);
          }
          break;
        case 2:
          sk_num = SKILL_DOORBASH;
          if (!HasClass(ch, CLASS_WARRIOR) && !HasClass(ch, CLASS_PALADIN)) {
            send_to_char(
              "'You do not possess the necessary fighting skills'\n\r", ch);
            return (TRUE);
          }
          break;
        case 3:
          sk_num = SKILL_SPY;
          if (!HasClass(ch, CLASS_THIEF)) {
            send_to_char(
              "'You do not possess the necessary thieving skills'\n\r", ch);
            return (TRUE);
          }
          break;
        case 4:
          sk_num = SKILL_RETREAT;
          if (!HasClass(ch, CLASS_THIEF)) {
            send_to_char("'I only teach this skill to thieves'\n\r", ch);
            return (TRUE);
          }
          break;
        default:
          sprintf(buf, "Strangeness in ninjamaster (%d)", number);
          vlog(buf);
          send_to_char("'Ack!  I feel faint!'\n\r", ch);
          return (TRUE);
      }
    }

    if (sk_num == SKILL_HUNT) {
      if (ch->skills[sk_num].learned >= 90) {
        send_to_char(
          "'You are a master of this art, I can teach you no more.'\n\r", ch);
        return (TRUE);
      }
    } else {
      if (ch->skills[sk_num].learned >= 45) {
        send_to_char("You must learn from practice and experience now.\n\r",
          ch);
        return (TRUE);
      }
    }

    if (GET_GOLD(ch) < charge) {
      send_to_char("'Ah, but you do not have enough money to pay.'\n\r", ch);
      return (TRUE);
    }

    if (ch->specials.spells_to_learn <= 0) {
      send_to_char("'You must first use the knowledge you already have.'\n\r",
        ch);
      return (FALSE);
    }

    GET_GOLD(ch) -= charge;
    send_to_char("'We will now begin.'\n\r", ch);
    ch->specials.spells_to_learn--;

    percent = ch->skills[sk_num].learned + int_app[GET_INT(ch)].learn;
    ch->skills[sk_num].learned = MIN(90, percent);

    if (ch->skills[sk_num].learned >= 90) {
      send_to_char("'You are now a master of this art.'\n\r", ch);
      return (TRUE);
    }
  } else {
    return (FALSE);
  }
}

int RepairGuy(struct char_data* ch, int cmd, char* arg) {
  char obj_name[80], vict_name[80], buf[MAX_INPUT_LENGTH];
  int cost, ave;
  struct char_data* vict;
  struct obj_data* obj;
  int (*rep_guy)(); /* special procedure for this mob/obj       */

  if (!AWAKE(ch))
    return (FALSE);

  rep_guy = RepairGuy;

  if (IS_NPC(ch)) {
    send_to_char("I don't deal with monsters.\n\r", ch);
    return (FALSE);
  }

  if (cmd == 72) { /* give */
    /* determine the correct obj */
    arg = one_argument(arg, obj_name);
    if (!*obj_name) {
      send_to_char("Give what?\n\r", ch);
      return (FALSE);
    }
    if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
      send_to_char("Give what?\n\r", ch);
      return (TRUE);
    }
    arg = one_argument(arg, vict_name);
    if (!*vict_name) {
      send_to_char("To who?\n\r", ch);
      return (FALSE);
    }
    if (!(vict = get_char_room_vis(ch, vict_name))) {
      send_to_char("To who?\n\r", ch);
      return (FALSE);
    }
    /* the target is the repairman, or an NPC */
    if (!IS_NPC(vict))
      return (FALSE);

    if (mob_index[vict->nr].func.mob_f == rep_guy) {
      /* we have the repair guy, and we can give him the stuff */
      act("You give $p to $N.", TRUE, ch, obj, vict, TO_CHAR);
      act("$n gives $p to $N.", TRUE, ch, obj, vict, TO_ROOM);
    } else {
      return (FALSE);
    }

    act("$N looks at $p.", TRUE, ch, obj, vict, TO_CHAR);
    act("$N looks at $p.", TRUE, ch, obj, vict, TO_ROOM);

    /* make all the correct tests to make sure that everything is kosher */

    if (ITEM_TYPE(obj) == ITEM_ARMOR && obj->obj_flags.value[1] > 0) {
      if (obj->obj_flags.value[1] > obj->obj_flags.value[0]) {
        /* get the value of the object */
        cost = obj->obj_flags.cost;
        /* divide by value[1]   */
        cost /= obj->obj_flags.value[1];
        /* then cost = difference between value[0] and [1] */
        cost *= (obj->obj_flags.value[1] - obj->obj_flags.value[0]);
        if (GetMaxLevel(vict) > 25) /* super repair guy */
          cost *= 2;
        if (cost > GET_GOLD(ch)) {
          if (check_soundproof(ch)) {
            act("$N shakes $S head.\n\r", TRUE, ch, 0, vict, TO_ROOM);
            act("$N shakes $S head.\n\r", TRUE, ch, 0, vict, TO_CHAR);
          } else {
            act("$N says 'I'm sorry, you don't have enough money.'", TRUE, ch,
              0, vict, TO_ROOM);
            act("$N says 'I'm sorry, you don't have enough money.'", TRUE, ch,
              0, vict, TO_CHAR);
          }
        } else {
          GET_GOLD(ch) -= cost;

          sprintf(buf, "You give $N %d coins.", cost);
          act(buf, TRUE, ch, 0, vict, TO_CHAR);
          act("$n gives some money to $N.", TRUE, ch, obj, vict, TO_ROOM);

          /* fix the armor */
          act("$N fiddles with $p.", TRUE, ch, obj, vict, TO_ROOM);
          act("$N fiddles with $p.", TRUE, ch, obj, vict, TO_CHAR);
          if (GetMaxLevel(vict) > 25) {
            obj->obj_flags.value[0] = obj->obj_flags.value[1];
          } else {
            ave = MAX(obj->obj_flags.value[0],
              (obj->obj_flags.value[0] + obj->obj_flags.value[1]) / 2);
            obj->obj_flags.value[0] = ave;
            obj->obj_flags.value[1] = ave;
          }
          if (check_soundproof(ch)) {
            act("$N smiles broadly.", TRUE, ch, 0, vict, TO_ROOM);
            act("$N smiles broadly.", TRUE, ch, 0, vict, TO_CHAR);
          } else {
            act("$N says 'All fixed.'", TRUE, ch, 0, vict, TO_ROOM);
            act("$N says 'All fixed.'", TRUE, ch, 0, vict, TO_CHAR);
          }
        }
      } else {
        if (check_soundproof(ch)) {
          act("$N shrugs.", TRUE, ch, 0, vict, TO_ROOM);
          act("$N shrugs.", TRUE, ch, 0, vict, TO_CHAR);
        } else {
          act("$N says 'Your armor looks fine to me.'", TRUE, ch, 0, vict,
            TO_ROOM);
          act("$N says 'Your armor looks fine to me.'", TRUE, ch, 0, vict,
            TO_CHAR);
        }
      }
    } else {
      if (GetMaxLevel(vict) < 25 || (ITEM_TYPE(obj) != ITEM_WEAPON)) {
        if (check_soundproof(ch)) {
          act("$N shakes $S head.\n\r", TRUE, ch, 0, vict, TO_ROOM);
          act("$N shakes $S head.\n\r", TRUE, ch, 0, vict, TO_CHAR);
        } else {
          if (ITEM_TYPE(obj) != ITEM_ARMOR) {
            act("$N says 'That isn't armor.'", TRUE, ch, 0, vict, TO_ROOM);
            act("$N says 'That isn't armor.'", TRUE, ch, 0, vict, TO_CHAR);
          } else {
            act("$N says 'I can't fix that...'", TRUE, ch, 0, vict, TO_CHAR);
            act("$N says 'I can't fix that...'", TRUE, ch, 0, vict, TO_ROOM);
          }
        }
      } else {
        struct obj_data* new;

        /* weapon repair.  expensive!   */
        cost = obj->obj_flags.cost;
        new = read_object(obj->item_number, REAL);
        if (obj->obj_flags.value[2])
          cost /= obj->obj_flags.value[2];

        cost *= (new->obj_flags.value[2] - obj->obj_flags.value[2]);

        if (cost > GET_GOLD(ch)) {
          if (check_soundproof(ch)) {
            act("$N shakes $S head.\n\r", TRUE, ch, 0, vict, TO_ROOM);
            act("$N shakes $S head.\n\r", TRUE, ch, 0, vict, TO_CHAR);
          } else {
            act("$N says 'I'm sorry, you don't have enough money.'", TRUE, ch,
              0, vict, TO_ROOM);
            act("$N says 'I'm sorry, you don't have enough money.'", TRUE, ch,
              0, vict, TO_CHAR);
          }
        } else {
          GET_GOLD(ch) -= cost;

          sprintf(buf, "You give $N %d coins.", cost);
          act(buf, TRUE, ch, 0, vict, TO_CHAR);
          act("$n gives some money to $N.", TRUE, ch, obj, vict, TO_ROOM);

          /* fix the weapon */
          act("$N fiddles with $p.", TRUE, ch, obj, vict, TO_ROOM);
          act("$N fiddles with $p.", TRUE, ch, obj, vict, TO_CHAR);

          obj->obj_flags.value[2] = new->obj_flags.value[2];
          extract_obj(new);

          if (check_soundproof(ch)) {
            act("$N smiles broadly.", TRUE, ch, 0, vict, TO_ROOM);
            act("$N smiles broadly.", TRUE, ch, 0, vict, TO_CHAR);
          } else {
            act("$N says 'All fixed.'", TRUE, ch, 0, vict, TO_ROOM);
            act("$N says 'All fixed.'", TRUE, ch, 0, vict, TO_CHAR);
          }
        }
      }
    }

    act("$N gives you $p.", TRUE, ch, obj, vict, TO_CHAR);
    act("$N gives $p to $n.", TRUE, ch, obj, vict, TO_ROOM);
    return (TRUE);
  } else {
    if (cmd)
      return FALSE;
    return (fighter(ch, cmd, arg));
  }
}

int Samah(struct char_data* ch, int cmd, char* arg) {
  char *p, buf[256];
  struct char_data* Sammy; /* Samah's own referent pointer */
  struct char_data *t, *t2, *t3;
  int purge_nr;
  struct room_data* rp;

  rp = real_roomp(ch->in_room);
  if (!rp)
    return (FALSE);

  if (cmd) {
    if (GET_RACE(ch) == RACE_SARTAN || GET_RACE(ch) == RACE_PATRYN ||
        GetMaxLevel(ch) == BRUTIUS)
      return (FALSE);

    Sammy = (struct char_data*)FindMobInRoomWithFunction(ch->in_room, Samah);

    for (; *arg == ' '; arg++)
      ; /* skip whitespace */
    strcpy(buf, arg);

    if (cmd == 207) { /* recite */
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      p = (char*)strtok(buf, " ");
      if (strncmp("recall", p, strlen(p)) == 0) {
        act("$n says 'And just where do you think you're going, Mensch?", FALSE,
          Sammy, 0, 0, TO_ROOM);
        return (TRUE);
      }
    } else if (cmd == 84) { /* cast */
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      /* we use strlen(p)-1 because if we use the full length, there is
    the obligatory ' at the end.  We must ignore this ', and get
    on with our lives */
      p = (char*)strtok(buf, " ");
      if (strncmp("'word of recall'", p, strlen(p) - 1) == 0) {
        act("$n says 'And just where do you think you're going, Mensch?", FALSE,
          Sammy, 0, 0, TO_ROOM);
        return (TRUE);
      } else if (strncmp("'astral walk'", p, strlen(p) - 1) == 0) {
        act(
          "$n says 'Do you think you can astral walk in and out of here like "
          "the ",
          FALSE, Sammy, 0, 0, TO_ROOM);
        act("wind,...Mensch?'", FALSE, Sammy, 0, 0, TO_ROOM);
        return (TRUE);
      } else if (strncmp("'teleport'", p, strlen(p) - 1) == 0) {
        act("$n says 'And just where do you think you're going, Mensch?", FALSE,
          Sammy, 0, 0, TO_ROOM);
        return (TRUE);
      } else if (strncmp("'polymorph'", p, strlen(p) - 1) == 0) {
        act("$n says 'I like you the way you are...Mensch.", FALSE, Sammy, 0, 0,
          TO_ROOM);
        return (TRUE);
      }
    } else if (cmd == 17 || cmd == 169) { /* say */
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      act("$n says 'Mensch should be seen, and not heard'", FALSE, Sammy, 0, 0,
        TO_ROOM);
      return (TRUE);
    } else if (cmd == 40 || cmd == 214 || cmd == 177) { /* emote */
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      act("$n says 'Cease your childish pantonimes, Mensch.'", FALSE, Sammy, 0,
        0, TO_ROOM);
      return (TRUE);
    } else if (cmd == 19 || cmd == 18 || cmd == 83) { /* say, shout whisp */
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      act("$n says 'Speak only when spoken to, Mensch.'", FALSE, Sammy, 0, 0,
        TO_ROOM);
      return (TRUE);
    } else if (cmd == 86) { /* ask */
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      act(
        "$n says 'Your ignorance is too immense to be rectified at this time. "
        "Mensch.'",
        FALSE, Sammy, 0, 0, TO_ROOM);
      return (TRUE);
    } else if (cmd == 87 || cmd == 46) { /* order, force */
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      act("$n says 'I'll be the only one giving orders here, Mensch'", FALSE,
        Sammy, 0, 0, TO_ROOM);
      return (TRUE);
    } else if (cmd == 151) {
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      act("$n says 'Cease this cowardly behavior, Mensch'", FALSE, Sammy, 0, ch,
        TO_ROOM);
      return (TRUE);
    } else if (cmd == 63) {
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      act("$n says 'Pay attention when I am speaking, Mensch'", FALSE, Sammy, 0,
        ch, TO_ROOM);
      return (TRUE);
    }
  } else {
    if (ch->specials.fighting) {
    } else {
      /*
   check for followers in the room
   */
      for (t = rp->people; t; t = t->next_in_room) {
        if (IS_NPC(t) && !IS_PC(t) && t->master && t != ch && t->master != ch) {
          break;
        }
      }
      if (t) {
        act("$n says 'What is $N doing here?'", FALSE, ch, 0, t, TO_ROOM);
        act("$n makes a magical gesture", FALSE, ch, 0, 0, TO_ROOM);
        purge_nr = t->nr;
        for (t2 = rp->people; t2; t2 = t3) {
          t3 = t2->next_in_room;
          if (t2->nr == purge_nr && !IS_PC(t2)) {
            act("$N, looking very surprised, quickly fades out of existence.",
              FALSE, ch, 0, t2, TO_ROOM);
            extract_char(t2);
          }
        }
      } else {
        /*
          check for polymorphs in the room
          */
        for (t = rp->people; t; t = t->next_in_room) {
          if (IS_NPC(t) && IS_PC(t)) { /* ah.. polymorphed :-) */
            /*
              I would like to digress at this point, and state that
              I feel that George Bush is an incompetent fool.
              Thank you.
              */
            act("$n glares at $N", FALSE, ch, 0, t, TO_NOTVICT);
            act("$n glares at you", FALSE, ch, 0, t, TO_VICT);
            act(
              "$n says 'Seek not to disguise your true form from me...Mensch.",
              FALSE, ch, 0, t, TO_ROOM);
            act("$n traces a small rune in the air", FALSE, ch, 0, 0, TO_ROOM);
            act("$n has forced you to return to your original form!", FALSE, ch,
              0, t, TO_VICT);
            do_return(t, "", 1);
            return (TRUE);
          }
        }
      }
    }
  }
  return (FALSE);
}

#if EGO

int BitterBlade(struct char_data* ch, int cmd, char* arg, struct obj_data* tobj,
  int type) {
  struct obj_data *obj, *blade;
  struct char_data *joe, *holder;
  struct char_data* lowjoe = 0;
  char arg1[128];

  if (type != PULSE_COMMAND)
    return (FALSE);

  if (IS_IMMORTAL(ch))
    return (FALSE);
  if (!real_roomp(ch->in_room))
    return (FALSE);

  for (obj = real_roomp(ch->in_room)->contents; obj; obj = obj->next_content) {
    if (obj_index[obj->item_number].func == BitterBlade) {
      /* I am on the floor */
      for (joe = real_roomp(ch->in_room)->people; joe;
           joe = joe->next_in_room) {
        if ((GET_ALIGNMENT(joe) <= -400) && (!IS_IMMORTAL(joe))) {
          if (lowjoe) {
            if (GET_ALIGNMENT(joe) < GET_ALIGNMENT(lowjoe)) {
              lowjoe = joe;
            }
          } else
            lowjoe = joe;
        }
      }
      if (lowjoe) {
        if (CAN_GET_OBJ(lowjoe, obj)) {
          obj_from_room(obj);
          obj_to_char(obj, lowjoe);
          send_to_char("A blade leaps in to your hands!\n\r", lowjoe);
          act("A blade jumps from the floor and leaps in to $n's hands!", FALSE,
            lowjoe, 0, 0, TO_ROOM);
          if (!EgoBladeSave(lowjoe)) {
            if (!lowjoe->equipment[WIELD]) {
              send_to_char("The blade forces you to wield it!\n\r", lowjoe);
              wear(lowjoe, obj, 12);
              return (FALSE);
            } else {
              send_to_char(
                "You can feel the blade attempt to make you wield it!\n\r",
                lowjoe);
              return (FALSE);
            }
          }
        }
      }
    }
  }
  for (holder = real_roomp(ch->in_room)->people; holder;
       holder = holder->next_in_room) {
    for (obj = holder->carrying; obj; obj = obj->next_content) {
      if ((obj_index[obj->item_number].func) &&
          (obj_index[obj->item_number].func != board)) {
        /*held*/
        if (holder->equipment[WIELD]) {
          if ((!EgoBladeSave(holder)) && (!EgoBladeSave(holder))) {
            send_to_char(
              "The blade gets pissed off that you are wielding another "
              "weapon!!\n\r",
              holder);
            act("The blade knocks $p out of your hands!!", FALSE, holder,
              holder->equipment[WIELD], 0, TO_CHAR);
            blade = unequip_char(holder, WIELD);
            if (blade)
              obj_to_room(blade, holder->in_room);
            if (!holder->equipment[WIELD]) {
              send_to_char("The blade forces you to wield it!\n\r", holder);
              wear(holder, obj, 12);
              return (FALSE);
            }
          }
        }
        if (!EgoBladeSave(holder)) {
          if (!EgoBladeSave(holder)) {
            if (!holder->equipment[WIELD]) {
              send_to_char("The blade forces you to wield it!\n\r", holder);
              wear(holder, obj, 12);
              return (FALSE);
            }
          }
        }
        if (affected_by_spell(holder, SPELL_CHARM_PERSON)) {
          affect_from_char(holder, SPELL_CHARM_PERSON);
          send_to_char(
            "Due to the blade, you feel less enthused about your master.\n\r",
            holder);
        }
      }
    }
    if (holder->equipment[WIELD]) {
      if ((obj_index[holder->equipment[WIELD]->item_number].func) &&
          (obj_index[holder->equipment[WIELD]->item_number].func != board)) {
        /*YES! I am being held!*/
        obj = holder->equipment[WIELD];
        if (affected_by_spell(holder, SPELL_CHARM_PERSON)) {
          affect_from_char(holder, SPELL_CHARM_PERSON);
          send_to_char(
            "Due to the blade, you feel less enthused about your master.\n\r",
            holder);
        }
        if (holder->specials.fighting) {
          send_to_char("The blade almost sings in your hand!!\n\r", holder);
          act("You can hear $n's blade almost sing with joy!", FALSE, holder, 0,
            0, TO_ROOM);
          if ((holder == ch) && (cmd == 151)) {
            if (EgoBladeSave(ch) && EgoBladeSave(ch)) {
              send_to_char(
                "You can feel the blade attempt to stay in the fight!\n\r", ch);
              return (FALSE);
            } else {
              send_to_char(
                "The blade laughs at your attempt to flee from a fight!!\n\r",
                ch);
              send_to_char("The blade gives you a little warning...\n\r", ch);
              send_to_char("The blade twists around and smacks you!\n\r", ch);
              act("Wow! $n's blade just whipped around and smacked $m one!",
                FALSE, ch, 0, 0, TO_ROOM);
              GET_HIT(ch) -= 25;
              if (GET_HIT(ch) < 0) {
                GET_HIT(ch) = 0;
                GET_POS(ch) = POSITION_STUNNED;
              }
              return (TRUE);
            }
          }
        }
        if ((cmd == 66) && (holder == ch)) {
          one_argument(arg, arg1);
          if (strcmp(arg1, "all") == 0) {
            if (!EgoBladeSave(ch)) {
              send_to_char("The blade laughs at your attempt to remove it!\n\r",
                ch);
              send_to_char("The blade gives you a little warning...\n\r", ch);
              send_to_char("The blade twists around and smacks you hard!\n\r",
                ch);
              act("Wow! $n's blade just whipped around and smacked $m one!",
                FALSE, ch, 0, 0, TO_ROOM);
              GET_HIT(ch) -= 25;
              if (GET_HIT(ch) < 0) {
                GET_HIT(ch) = 0;
                GET_POS(ch) = POSITION_STUNNED;
              }
              return (TRUE);
            } else {
              send_to_char(
                "You can feel the blade attempt to stay wielded!\n\r", ch);
              return (FALSE);
            }
          } else {
            if (isname(arg1, obj->name)) {
              if (!EgoBladeSave(ch)) {
                send_to_char(
                  "The blade laughs at your attempt to remove it!\n\r", ch);
                send_to_char("The blade gives you a little warning...\n\r", ch);
                send_to_char("The blade twists around and smacks you hard!\n\r",
                  ch);
                act("Wow! $n's blade just whipped around and smacked $m one!",
                  FALSE, ch, 0, 0, TO_ROOM);
                GET_HIT(ch) -= 25;
                if (GET_HIT(ch) < 0) {
                  GET_HIT(ch) = 0;
                  GET_POS(ch) = POSITION_STUNNED;
                }
                return (TRUE);
              } else {
                send_to_char(
                  "You can feel the blade attempt to stay wielded!\n\r", ch);
                return (FALSE);
              }
            }
          }
        }
        for (joe = real_roomp(holder->in_room)->people; joe;
             joe = joe->next_in_room) {
          if ((GET_ALIGNMENT(joe) >= 500) && (IS_MOB(joe)) &&
              (CAN_SEE(holder, joe)) && (holder != joe)) {
            if (lowjoe) {
              if (GET_ALIGNMENT(joe) > GET_ALIGNMENT(lowjoe)) {
                lowjoe = joe;
              }
            } else
              lowjoe = joe;
          }
        }
        if (lowjoe) {
          if (!EgoBladeSave(holder)) {
            if (GET_POS(holder) != POSITION_STANDING) {
              send_to_char("The blade yanks you to your feet!\n\r", ch);
              GET_POS(holder) = POSITION_STANDING;
            }
            send_to_char("The blade leaps out of control!!\n\r", holder);
            act("A blade jumps for $n's neck!", FALSE, lowjoe, 0, 0, TO_ROOM);
            do_hit(holder, lowjoe->player.name, 0);
            return (TRUE);
          } else {
            return (FALSE);
          }
        }
        if ((cmd == 70) && (holder == ch)) {
          send_to_char("The blade almost sings in your hand!!\n\r", ch);
          act("You can hear $n's blade almost sing with joy!", FALSE, ch, 0, 0,
            TO_ROOM);
          return (FALSE);
        }
      }
    }
  }
  return (FALSE);
}
#endif

#define GIVE 72
#define GAIN 243

int MakeQuest(struct char_data* ch, struct char_data* gm, int Class, char* arg,
  int cmd) {
  char obj_name[50], vict_name[50];
  struct char_data* vict;
  struct obj_data* obj;

  int i;

#if EASY_LEVELS
  if (GET_LEVEL(ch, Class) > 0) { /* for now.. so as not to give it away */
    if (cmd == GAIN) {
      GainLevel(ch, Class);
      return (TRUE);
    }
    return (FALSE);
  }
#endif

  if (cmd == GIVE) {
    arg = one_argument(arg, obj_name);
    arg = one_argument(arg, vict_name);
    if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
      send_to_char("You do not seem to have anything like that.\n\r", ch);
      return (FALSE);
    }
    if (!(vict = get_char_room_vis(ch, vict_name))) {
      send_to_char("No one by that name around here.\n\r", ch);
      return FALSE;
    }
    if (vict == gm) {
      if (obj_index[obj->item_number].virtual ==
          QuestList[Class][GET_LEVEL(ch, Class)].item) {
        act("$n graciously takes your gift of $p", FALSE, gm, obj, ch, TO_VICT);
        obj_from_char(obj);
        extract_obj(obj);
        GainLevel(ch, Class);
        return (TRUE);
      } else {
        act("$n shakes $s head", FALSE, gm, 0, 0, TO_ROOM);
        act("$n says 'That is not the item which i desire'", FALSE, gm, 0, 0,
          TO_ROOM);
        return (FALSE);
      }
    } else {
      return (FALSE);
    }

  } else if (cmd == GAIN) {
    if (GET_EXP(ch) < titles[Class][GET_LEVEL(ch, Class) + 1].exp) {
      send_to_char("You are not yet ready to gain\n\r", ch);
      return (FALSE);
    }

    if (QuestList[Class][GET_LEVEL(ch, Class)].item) {
      act("$n shakes $s head", FALSE, gm, 0, 0, TO_ROOM);
      act("$n tells you 'First you must prove your mastery of knowledge'",
        FALSE, gm, 0, ch, TO_VICT);
      act("$n tells you 'Give to me the item that answers this riddle'", FALSE,
        gm, 0, ch, TO_VICT);
      act("$n tells you 'And you shall have your level'\n\r", FALSE, gm, 0, ch,
        TO_VICT);
      send_to_char(QuestList[Class][GET_LEVEL(ch, Class)].where, ch);
      send_to_char("\n\rGood luck", ch);
      return (FALSE);
    }
  } else {
    return (FALSE);
  }
}

int creeping_death(struct char_data* ch, int cmd, char* arg) {
  struct char_data *t, *next;
  struct room_data* rp;
  struct obj_data *co, *o;

  if (cmd)
    return (FALSE);
  if (!ch->act_ptr)
    ch->act_ptr = (int*)calloc(1, sizeof(int));
  if (ch->specials.fighting) { /* kill */

    t = ch->specials.fighting;
    if (t->in_room == ch->in_room) {
      act("$N is engulfed by $n!", FALSE, ch, 0, t, TO_NOTVICT);
      act("You are engulfed by $n, and are quickly disassembled", FALSE, ch, 0,
        t, TO_VICT);
      act("$N is quickly reduced to a bloody pile of bones by $n", FALSE, ch, 0,
        t, TO_NOTVICT);
      GET_HIT(ch) -= GET_HIT(t);
      die(t);
      /* find the corpse and destroy it */
      rp = real_roomp(ch->in_room);
      if (!rp)
        return (FALSE);
      for (co = rp->contents; co; co = co->next_content) {
        if (IS_CORPSE(co)) { /* assume 1st corpse is victim's */
          while (co->contains) {
            o = co->contains;
            obj_from_obj(o);
            obj_to_room(o, ch->in_room);
          }
          extract_obj(co); /* remove the corpse */
        }
      }
    }
    if (GET_HIT(ch) < 0) {
      act("$n dissapates, you breath a sigh of relief", FALSE, ch, 0, 0,
        TO_ROOM);
      GET_HIT(ch) = 1;
      ch->points.max_hit = 10;
      REMOVE_BIT(ch->specials.act, ACT_SPEC);
      return (TRUE);
    }
    return (TRUE);
  }

  /* the act_ptr is the direction of travel */

  if (number(0, 1) == 0) { /* move */
    if (!ValidMove(ch, (*((int*)ch->act_ptr)))) {
      act("$n dissapates, you breath a sigh of relief", FALSE, ch, 0, 0,
        TO_ROOM);
      GET_HIT(ch) = 1;
      ch->points.max_hit = 10;
      REMOVE_BIT(ch->specials.act, ACT_SPEC);
      return (FALSE);
    } else {
      do_move(ch, "\0", (*((int*)ch->act_ptr)));
      return (FALSE);
    }
  } else {
    /* make everyone with any brains flee */
    for (t = real_roomp(ch->in_room)->people; t; t = next) {
      next = t->next_in_room;
      if (t != ch) {
        if (!saves_spell(t, SAVING_PETRI)) {
          do_flee(t, "\0", 0);
        }
      }
    }

    /* find someone in the room to flay */
    for (t = real_roomp(ch->in_room)->people; t; t = next) {
      next = t->next_in_room;
      if (!IS_IMMORTAL(t) && t != ch && number(0, 2) == 0) {
        act("$N is engulfed by $n!", FALSE, ch, 0, t, TO_NOTVICT);
        act("You are engulfed by $n, and are quickly disassembled", FALSE, ch,
          0, t, TO_VICT);
        act("$N is quickly reduced to a bloody pile of bones by $n", FALSE, ch,
          0, t, TO_NOTVICT);
        GET_HIT(ch) -= GET_HIT(t);
        die(t);
        /* find the corpse and destroy it */
        rp = real_roomp(ch->in_room);
        if (!rp)
          return (FALSE);
        for (co = rp->contents; co; co = co->next_content) {
          if (IS_CORPSE(co)) { /* assume 1st corpse is victim's */
            while (co->contains) {
              o = co->contains;
              obj_from_obj(o);
              obj_to_room(o, ch->in_room);
            }
            extract_obj(co); /* remove the corpse */
          }
        }

        if (GET_HIT(ch) < 0) {
          act("$n dissapates, you breath a sigh of relief", FALSE, ch, 0, 0,
            TO_ROOM);
          GET_HIT(ch) = 1;
          ch->points.max_hit = 10;
          REMOVE_BIT(ch->specials.act, ACT_SPEC);
          return (TRUE);
        }

        break; /* end the loop */
      }
    }
  }
}

#if 0
/*
   shanty town kids
*/
int shanty_town_kids( struct char_data *ch, int cmd, char *arg)
{

  if (!AWAKE(ch)) return(FALSE);


  /*
    harrass low levellers.
  */
  if (cmd >= 1 && cmd <= 6) {
    if (GetMaxLevel(ch) <= 5 && number(0,2)==0) {
      act("A street kid sticks out a foot and trips you as you try to leave",
     FALSE, ch, 0, 0, TO_CHAR);
      act("A street kid sticks out a foot and trips $n",
     FALSE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
      act("The street kid laughs at you", FALSE,ch, 0, 0, TO_CHAR);
      act("The street kid laughs at $n", FALSE,ch, 0, 0, TO_ROOM);
    }
  }

  /*
    steal from mid-levellers
  */


  /*
    backstab high levellers
    */


  if (cmd) {


  }

  if (ch->specials.fighting) {
    act("$N runs between $n's legs", FALSE, ch->specials.fighting, 0, ch, TO_ROOM);
    act("$N runs between your legs", FALSE, ch->specials.fighting, 0, ch, TO_CHAR);
    vict = ch->specials.fighting;
    stop_fighting(ch);
    stop_fighting(vict);
  }

}

#endif

int GenericCityguardHateUndead(struct char_data* ch, int cmd, char* arg,
  int type) {
  struct char_data *tch, *evil, *i;
  int max_evil, lev;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    fighter(ch, cmd, arg);

    if (!check_soundproof(ch)) {
      if (number(0, 20) == 0) {
        do_shout(ch, "To me, my fellows! I am in need of thy aid!", 0);
      } else {
        act("$n shouts 'To me, my fellows! I need thy aid!'", TRUE, ch, 0, 0,
          TO_ROOM);
      }

      if (ch->specials.fighting)
        CallForGuard(ch, ch->specials.fighting, 3, type);

      return (TRUE);
    }
  }

  max_evil = 0;
  evil = 0;

  if (check_peaceful(ch, ""))
    return FALSE;

  for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if ((IS_NPC(tch)) && (IsUndead(tch)) && CAN_SEE(ch, tch)) {
      max_evil = -1000;
      evil = tch;
      if (!check_soundproof(ch))
        act("$n screams 'EVIL!!!  BANZAI!  SPOOON!'", FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, evil, TYPE_UNDEFINED);
      return (TRUE);
    }
    if (!IS_PC(tch)) {
      if (tch->specials.fighting) {
        if ((GET_ALIGNMENT(tch) < max_evil) &&
            (!IS_PC(tch) || !IS_PC(tch->specials.fighting))) {
          max_evil = GET_ALIGNMENT(tch);
          evil = tch;
        }
      }
    }
  }

  if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0)) {
    if (GET_HIT(evil->specials.fighting) > GET_HIT(evil) ||
        (evil->specials.fighting->attackers > 3)) {
      if (!check_soundproof(ch))
        act("$n screams 'PROTECT THE INNOCENT! BANZAI!!! CHARGE!!! SPOON!'",
          FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, evil, TYPE_UNDEFINED);
      return (TRUE);
    } else {
      if (!check_soundproof(ch))
        act("$n yells 'There's no need to fear! $n is here!'", FALSE, ch, 0, 0,
          TO_ROOM);

      if (!ch->skills)
        SpaceForSkills(ch);

      if (!ch->skills[SKILL_RESCUE].learned)
        ch->skills[SKILL_RESCUE].learned = GetMaxLevel(ch) * 3 + 30;
      do_rescue(ch, GET_NAME(evil->specials.fighting), 0);
    }
  }

  return (FALSE);
}

int GenericCityguard(struct char_data* ch, int cmd, char* arg, int type) {
  struct char_data *tch, *evil, *i;
  int max_evil, lev;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    fighter(ch, cmd, arg);

    if (!check_soundproof(ch)) {
      if (number(0, 20) == 0) {
        do_shout(ch, "To me, my fellows! I am in need of thy aid!", 0);
      } else {
        act("$n shouts 'To me, my fellows! I need thy aid!'", TRUE, ch, 0, 0,
          TO_ROOM);
      }

      if (ch->specials.fighting)
        CallForGuard(ch, ch->specials.fighting, 3, type);

      return (TRUE);
    }
  }

  max_evil = 1000;
  evil = 0;

  if (check_peaceful(ch, ""))
    return FALSE;

  for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if (tch->specials.fighting) {
      if ((GET_ALIGNMENT(tch) < max_evil) &&
          (IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
        max_evil = GET_ALIGNMENT(tch);
        evil = tch;
      }
    }
  }

  if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0)) {
    if (!check_soundproof(ch)) {
      act("$n screams 'PROTECT THE INNOCENT! BANZAI!!! CHARGE!!! SPOON!'",
        FALSE, ch, 0, 0, TO_ROOM);
    }
    hit(ch, evil, TYPE_UNDEFINED);
    return (TRUE);
  }

  return (FALSE);
}

int loremaster(struct char_data* ch, int cmd, char* arg) {
  char buf[256];
  static char* n_skills[] = {
    "necromancy",
    "vegetable lore",
    "animal lore",
    "reptile lore",
    "people lore",
    "giant lore",
    "other lore",
    "read magic",
    "demonology",
    "sign language",
    "\n",
  };
  int percent = 0, number = 0;
  int charge, sk_num, mult;

  if (!AWAKE(ch))
    return (FALSE);

  if (!ch->skills)
    return (FALSE);

  if (!cmd) {
    if (ch->specials.fighting) {
      return (fighter(ch, cmd, arg));
    }
    return (FALSE);
  }

  if (check_soundproof(ch))
    return (FALSE);

  for (; *arg == ' '; arg++)
    ; /* ditch spaces */

  if ((cmd == 164) || (cmd == 170)) {
    if (!arg || (strlen(arg) == 0)) {
      sprintf(buf, " necromancy      :  %s\n\r",
        how_good(ch->skills[SKILL_CONS_UNDEAD].learned));
      send_to_char(buf, ch);
      sprintf(buf, " vegetable lore  :  %s\n\r",
        how_good(ch->skills[SKILL_CONS_VEGGIE].learned));
      send_to_char(buf, ch);
      sprintf(buf, " demonology      :  %s\n\r",
        how_good(ch->skills[SKILL_CONS_DEMON].learned));
      send_to_char(buf, ch);
      sprintf(buf, " animal lore     :  %s\n\r",
        how_good(ch->skills[SKILL_CONS_ANIMAL].learned));
      send_to_char(buf, ch);
      sprintf(buf, " reptile lore  :  %s\n\r",
        how_good(ch->skills[SKILL_CONS_REPTILE].learned));
      send_to_char(buf, ch);
      sprintf(buf, " people lore     :  %s\n\r",
        how_good(ch->skills[SKILL_CONS_PEOPLE].learned));
      send_to_char(buf, ch);
      sprintf(buf, " giant lore      :  %s\n\r",
        how_good(ch->skills[SKILL_CONS_GIANT].learned));
      send_to_char(buf, ch);
      sprintf(buf, " other lore      :  %s\n\r",
        how_good(ch->skills[SKILL_CONS_OTHER].learned));
      send_to_char(buf, ch);
      sprintf(buf, " read magic      :  %s\n\r",
        how_good(ch->skills[SKILL_READ_MAGIC].learned));
      send_to_char(buf, ch);
      sprintf(buf, " sign language   :  %s\n\r",
        how_good(ch->skills[SKILL_SIGN].learned));
      send_to_char(buf, ch);
      return (TRUE);
    } else {
      number = old_search_block(arg, 0, strlen(arg), n_skills, FALSE);
      send_to_char("The loremaster says ", ch);
      if (number == -1) {
        send_to_char("'I do not know of this skill.'\n\r", ch);
        return (TRUE);
      }
      charge = GetMaxLevel(ch) * 100;
      switch (number) {
        case 0:
        case 1:
          sk_num = SKILL_CONS_UNDEAD;
          break;
        case 2:
          sk_num = SKILL_CONS_VEGGIE;
          break;
        case 3:
          sk_num = SKILL_CONS_ANIMAL;
          break;
        case 4:
          sk_num = SKILL_CONS_REPTILE;
          break;
        case 5:
          sk_num = SKILL_CONS_PEOPLE;
          break;
        case 6:
          sk_num = SKILL_CONS_GIANT;
          break;
        case 7:
          sk_num = SKILL_CONS_OTHER;
          break;
        case 8:
          if (!HasClass(ch, CLASS_CLERIC) && !HasClass(ch, CLASS_MAGIC_USER)) {
            sk_num = SKILL_READ_MAGIC;
          } else {
            send_to_char("'You already have this skill!'\n\r", ch);
            if (ch->skills)
              if (!ch->skills[SKILL_READ_MAGIC].learned)
                ch->skills[SKILL_READ_MAGIC].learned = 95;
            return TRUE;
          }
          break;

        case 9:
          sk_num = SKILL_CONS_DEMON;
          break;
        case 10:
          sk_num = SKILL_SIGN;
          break;

        default:
          sprintf(buf, "Strangeness in loremaster (%d)", number);
          vlog(buf);
          send_to_char("'Ack!  I feel faint!'\n\r", ch);
          return (TRUE);
      }
    }

    if (GET_GOLD(ch) < charge) {
      send_to_char("'Ah, but you do not have enough money to pay.'\n\r", ch);
      return (TRUE);
    }

    if (ch->specials.spells_to_learn <= 0) {
      send_to_char("'You must first earn more practices you already have.\n\r",
        ch);
      return (TRUE);
    }

    GET_GOLD(ch) -= charge;
    send_to_char("'We will now begin.'\n\r", ch);
    ch->specials.spells_to_learn--;

    percent = ch->skills[sk_num].learned + int_app[GET_INT(ch)].learn;
    ch->skills[sk_num].learned = MIN(95, percent);

    if (ch->skills[sk_num].learned >= 95) {
      send_to_char("'You are now a master of this art.'\n\r", ch);
      return (TRUE);
    }
  } else {
    return (FALSE);
  }
}

void station() {
  int t;
  int i;
  struct obj_data* obj;
  struct char_data* ch;
  t = time_info.hours;
  if ((t == 6) || (t == 10) || (t == 14) || (t == 18) || (t == 22)) {
    send_to_room(
      "The train to the casino has arrived, and will leave in one hour.\n\r",
      18999);
    obj = read_object(9001, VIRTUAL);
    obj_to_room(obj, 18999);
  }
  if ((t == 9) || (t == 13) || (t == 17) || (t == 21) || (t == 1)) {
    send_to_room(
      "The train back to the mainland has arrived, and will be heading back in "
      "one hour.\n\r",
      8600);
    obj = read_object(9001, VIRTUAL);
    obj_to_room(obj, 8600);
  }

  if ((t == 9) || (t == 13) || (t == 17) || (t == 21) || (t == 1)) {
    send_to_room(
      "The train has arrived at its destination and you are pushed immediatly "
      "off.\n\r",
      8602);
    ch = real_roomp(8602)->people;
    while (ch) {
      char_from_room(ch);
      char_to_room(ch, 8600);
      ch = real_roomp(8602)->people;
    }
  }

  if ((t == 12) || (t == 16) || (t == 20) || (t == 0) || (t == 4)) {
    send_to_room(
      "The train has arrived at its destination and you are pushed immediatly "
      "off.\n\r",
      8601);
    ch = real_roomp(8601)->people;
    while (ch) {
      char_from_room(ch);
      char_to_room(ch, 18999);
      ch = real_roomp(8601)->people;
    }
  }

  if ((t == 7) || (t == 11) || (t == 15) || (t == 19) || (t == 23)) {
    send_to_room("The train has left for the casino!\n\r", 18999);
    send_to_room("The train rumbles as you depart for the casino!\n\r", 8602);
    obj = get_obj_in_list("train", real_roomp(18999)->contents);
    if (obj)
      extract_obj(obj);
  }

  if ((t == 10) || (t == 14) || (t == 18) || (t == 22) || (t == 2)) {
    send_to_room("The train has departed for the city.\n\r", 8600);
    send_to_room(
      "The train rumbles and shakes as it heads back to the city.\n\r", 8601);
    obj = get_obj_in_list("train", real_roomp(8600)->contents);
    if (obj)
      extract_obj(obj);
  }
}

int train_station(struct char_data* ch, int cmd, char* arg) {
  int t;
  if (cmd != 312)
    return FALSE;
  t = time_info.hours;
  if ((ch->in_room == 18999) &&
      ((t == 6) || (t == 10) || (t == 14) || (t == 18) || (t == 22))) {
    act("$n boards the train to the casino!", FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("You board the train to the casino.\n\r", ch);
    char_from_room(ch);
    char_to_room(ch, 8602);
    act("$n joins the trip to the casino!", FALSE, ch, 0, 0, TO_ROOM);
    return TRUE;
  } else if (ch->in_room == 18999) {
    send_to_char("The train isn't here!\n\r", ch);
    return TRUE;
  }

  if ((ch->in_room == 8600) &&
      ((t == 9) || (t == 13) || (t == 17) || (t == 21) || (t == 1))) {
    act("$n boards the train to the city.", FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("You board the train to the city.\n\r", ch);
    char_from_room(ch);
    char_to_room(ch, 8601);
    act("$n has just joined you for the ride back to the city.", FALSE, ch, 0,
      0, TO_ROOM);
    return TRUE;
  } else if (ch->in_room == 8600)
    send_to_char("The train isnt here!\n\r", ch);
  return TRUE;
}

int dragon(struct char_data* ch, int cmd, char* arg) {
  struct char_data *t1, *t2;
  int damage;
  if (cmd)
    return FALSE;
  if (!ch->specials.fighting)
    return FALSE;
  if (number(1, 10) < 7) {
    act("$n breathes a cone of frost!", FALSE, ch, 0, 0, TO_ROOM);
    for (t1 = real_roomp(ch->in_room)->people; t1; t1 = t1->next_in_room) {
      if (!IS_MOB(t1)) {
        send_to_char("The cold is FREEZING you!\n\r", t1);
        damage = MIN(number(10, 100), GET_HIT(t1) + 8);
        if (damage > 0)
          GET_HIT(t1) -= damage;
      }
    }
  } else {
    t2 = 0;
    for (t1 = real_roomp(ch->in_room)->people; t1; t1 = t1->next_in_room) {
      if (!IS_MOB(t1)) {
        if (!t2)
          t2 = t1;
        else if ((GET_HIT(t1) > -1) && (GET_HIT(t1) < GET_HIT(t2)))
          t2 = t1;
      }
    }
    if (t2) {
      act("The Dragon whips $n with his tail!", FALSE, t2, 0, 0, TO_ROOM);
      send_to_char("The Dragon slashes you with his spiked tail! OUCH!\n\r",
        t2);
      damage = MIN(number(8, 100), GET_HIT(t2) + 1);
      if (damage > 0)
        GET_HIT(t2) -= damage;
    }
  }
}

int hunter(struct char_data* ch, int cmd, char* arg) {
  char buf[256];
  static char* n_skills[] = {
    "track",
    "find traps",
    "disarm traps",
    "\n",
  };
  int percent = 0, number = 0;
  int charge, sk_num, mult;

  if (!AWAKE(ch))
    return (FALSE);

  if (!cmd) {
    if (ch->specials.fighting) {
      return (fighter(ch, cmd, arg));
    }
    return (FALSE);
  }

  if (!ch->skills)
    return (FALSE);

  if (check_soundproof(ch))
    return (FALSE);

  for (; *arg == ' '; arg++)
    ; /* ditch spaces */

  if ((cmd == 164) || (cmd == 170)) {
    if (!arg || (strlen(arg) == 0)) {
      sprintf(buf, " track          :  %s\n\r",
        how_good(ch->skills[SKILL_HUNT].learned));
      send_to_char(buf, ch);
      sprintf(buf, " find traps     :  %s\n\r",
        how_good(ch->skills[SKILL_FIND_TRAP].learned));
      send_to_char(buf, ch);
      sprintf(buf, " disarm traps   :  %s\n\r",
        how_good(ch->skills[SKILL_REMOVE_TRAP].learned));
      send_to_char(buf, ch);
      return (TRUE);
    } else {
      if (!HasClass(ch, CLASS_THIEF)) {
        send_to_char("'You're not the sort I'll teach to'\n\r", ch);
        return (TRUE);
      }

      number = old_search_block(arg, 0, strlen(arg), n_skills, FALSE);
      send_to_char("The hunter says ", ch);
      if (number == -1) {
        send_to_char("'I do not know of this skill.'\n\r", ch);
        return (TRUE);
      }
      charge = GetMaxLevel(ch) * 1000;
      switch (number) {
        case 0:
        case 1:
          sk_num = SKILL_HUNT;
          break;
        case 2:
          sk_num = SKILL_FIND_TRAP;
          break;
        case 3:
          sk_num = SKILL_REMOVE_TRAP;
          break;
        default:
          sprintf(buf, "Strangeness in hunter (%d)", number);
          vlog(buf);
          send_to_char("'Ack!  I feel faint!'\n\r", ch);
          return (FALSE);
      }
    }

    if (GET_GOLD(ch) < charge) {
      send_to_char("'Ah, but you do not have enough money to pay.'\n\r", ch);
      return (TRUE);
    }

    if (ch->skills[sk_num].learned >= 90) {
      send_to_char(
        "'You are a master of this art, I can teach you no more.'\n\r", ch);
      return (TRUE);
    }

    if (ch->specials.spells_to_learn <= 0) {
      send_to_char("'You must first earn more practices you already have.\n\r",
        ch);
      return (TRUE);
    }

    GET_GOLD(ch) -= charge;
    send_to_char("'We will now begin.'\n\r", ch);
    ch->specials.spells_to_learn--;

    percent = ch->skills[sk_num].learned + int_app[GET_INT(ch)].learn;
    ch->skills[sk_num].learned = MIN(90, percent);

    if (ch->skills[sk_num].learned >= 90) {
      send_to_char("'You are now a master of this art.'\n\r", ch);
      return (TRUE);
    }
  } else {
    return (FALSE);
  }
}

int scraps(struct char_data* ch, int cmd, char* arg, struct obj_data* obj,
  int type) {
  if (type == PULSE_COMMAND) {
    return (FALSE);
  } else {
    if (obj->obj_flags.value[0])
      obj->obj_flags.value[0]--;

    if (obj->obj_flags.value[0] == 0 && obj->in_room) {
      if ((obj->in_room != NOWHERE) && (real_roomp(obj->in_room)->people)) {
        act("$p disintegrates into atomic particles!", FALSE,
          real_roomp(obj->in_room)->people, obj, 0, TO_ROOM);
        act("$p disintegrates into atomic particles!", FALSE,
          real_roomp(obj->in_room)->people, obj, 0, TO_CHAR);
      }
      extract_obj(obj);
    }
  }
}

/* START:  Stargazer Work: */

struct char_data* char_with_name(char* name) {
  struct descriptor_data* d;

  for (d = descriptor_list; d; d = d->next) {
    if ((d->connected == CON_PLYNG) && (d->character) &&
        (!strcasecmp(GET_NAME(d->character), name)))
      return d->character;
  }

  return NULL;
}

int adjacent_room(int room) {
  struct room_data* rp;
  int dir;

  if (rp = real_roomp(room))
    for (dir = 0; dir < 6; dir++)
      if (rp->dir_option[dir] && (rp->dir_option[dir]->to_room > 0))
        return rp->dir_option[dir]->to_room;

  return -1;
}

#define HUNTER_ID "Hunter,"

int bounty_hunter(struct char_data* ch, int cmd, char* arg) {
  assert(ch);

  if (cmd) {
    if (!arg)
      return FALSE;

    for (; *arg == ' '; arg++)
      ;

    // catch rent/drop/give
    if (cmd == 92 || cmd == 60 || cmd == 72) {
      struct room_data* rp = real_roomp(ch->in_room);

      if (!rp) {
        return FALSE;
      }

      for (struct char_data* me = rp->people; me; me = me->next_in_room) {
        if (!IS_MOB(me) || mob_index[me->nr].func.mob_f != bounty_hunter ||
            !me->act_ptr || !me->act_ptr->type == MOB_ACT_BOUNTYHUNTER) {
          continue;
        }

        struct mob_act_bountyhunter* job = &me->act_ptr->data.bountyhunter;

        if (*job->hunted_item == '\0') {
          continue;
        }

        char buf[256];
        sprintf(buf, "1.%s", job->hunted_item);

        struct obj_data* temp_obj = get_obj_vis_world(me, buf, NULL);

        if (!temp_obj || ch != char_holding(temp_obj)) {
          continue;
        }

        if (cmd == 92) {
          do_say(me, "You're not getting away that easy.", 17);
          act("$n stands between you and the receptionist!", FALSE, me, 0, ch,
            TO_VICT);
          act("$n blocks $N from renting!", FALSE, me, 0, ch, TO_NOTVICT);
          return TRUE;
        }

        if (cmd == 72) {
          char buf2[256];

          if (sscanf(arg, " %s %s", buf, buf2) != 2 ||
              !isname(buf, temp_obj->name)) {
            continue;
          }

          if (isname(buf2, GET_NAME(me))) {
            do_give(ch, arg, 72);
            do_action(me, GET_NAME(ch), 23);
            do_say(me,
              "I'm glad you've come to your senses!  I would surely "
              "smite thee!",
              17);
          } else {
            do_say(me, "I'll take that!", 17);
            act("$n almost grabs it from your hands!  Whew!", FALSE, me, 0, ch,
              TO_VICT);
            act("$n almost grabs $p from $N's hands!", FALSE, me, temp_obj, ch,
              TO_NOTVICT);
          }

          return TRUE;
        }

        //  cmd == 60
        sscanf(arg, " %s ", buf);
        if (!isname(buf, temp_obj->name)) {
          continue;
        }

        do_drop(ch, arg, 60);
        do_say(me, "Thanks, sport!", 17);
        do_get(me, arg, 60);

        return TRUE;
      }

      return FALSE;
    }

    if (cmd == 202) {
      // catch group
      struct char_data* me =
        FindMobInRoomWithFunction(ch->in_room, bounty_hunter);

      assert(me);

      do_action(me, ch->player.name, 190);  // growl
      do_say(me, "Noone is grouping here, scum.", 17);
      return TRUE;
    }

    if (cmd == 207 &&
        (strcasestr(arg, "recall") || strcasestr(arg, "scroll"))) {
      struct char_data* me =
        FindMobInRoomWithFunction(ch->in_room, bounty_hunter);

      assert(me);

      do_say(me, "Noone is recalling until I finish my job.", 17);
      act("$n knocks the scroll from your hand!", FALSE, me, 0, ch, TO_VICT);
      act("$n knocks the scroll from $N's hand!", FALSE, me, 0, ch, TO_NOTVICT);
      return TRUE;
    }

    if (cmd == 84) { /* cast */
      if (*arg != '\'') {
        return FALSE;
      }

      int offset = 1;

      for (; *(arg + offset) && (*(arg + offset) != '\''); ++offset) {
        *(arg + offset) = LOWER(*(arg + offset));
      }

      if (*(arg + offset) != '\'')
        return FALSE;

      int spell = old_search_block(arg, 1, offset - 1, spells, 0);

      if (spell == 101 || spell == 2 || spell == 111 || spell == 42) {
        struct char_data* me =
          FindMobInRoomWithFunction(ch->in_room, bounty_hunter);

        assert(me);

        act("$n disrupts your spell!", FALSE, me, 0, ch, TO_VICT);
        act("$n disrupts $N's spell!", FALSE, me, 0, ch, TO_NOTVICT);
        do_say(me, "Noone is going anywhere until I finish my job!", 17);
        return TRUE;
      }

      return FALSE;
    }

    if ((cmd == 17 || cmd == 169) && IS_IMMORTAL(ch)) {
      if (strncasecmp(arg, HUNTER_ID, strlen(HUNTER_ID)))
        return FALSE;

      arg += strlen(HUNTER_ID);

      struct char_data* me =
        FindMobInRoomWithFunction(ch->in_room, bounty_hunter);

      assert(me);

      if (!strncasecmp(arg, " version", 8)) {
        act("$n says 'I am version 1.11 of the Bounty Hunter spec_proc.'",
          FALSE, me, 0, 0, TO_ROOM);
        act("$n says 'I was conceived of and designed by Stargazer.'", FALSE,
          me, 0, 0, TO_ROOM);
        return TRUE;
      }

      act("$n whispers something to $N.", FALSE, me, 0, ch, TO_NOTVICT);

      if (!strncasecmp(arg, " status", 7)) {
        if (!me->act_ptr || me->act_ptr->type != MOB_ACT_BOUNTYHUNTER) {
          act("$n whispers 'Master $N, I am currently idle with no memory.'",
            FALSE, me, 0, ch, TO_VICT);

          if (me->act_ptr && me->act_ptr->type != MOB_ACT_BOUNTYHUNTER) {
            vlog(
              "bounty_hunter proc: ch->act_ptr is not of type "
              "MOB_ACT_BOUNTYHUNTER.  Resetting.");
            free(me->act_ptr);
            me->act_ptr = NULL;
          }

          return TRUE;
        }

        struct mob_act_bountyhunter* job = &me->act_ptr->data.bountyhunter;

        char buf2[256];

        if (*job->hunted_item != '\0') {
          if (*job->hunted_victim != '\0') {
            sprintf(buf2,
              "$n whispers 'Master $N, I am hunting the %s carried by %s.'",
              job->hunted_item, job->hunted_victim);
          } else {
            sprintf(buf2, "$n whispers 'Master $N, I am currently hunting %s.'",
              job->hunted_item);
          }

          if (job->num_retrieved > 0) {
            act(buf2, FALSE, me, 0, ch, TO_VICT);
            sprintf(buf2,
              "$n whispers 'I have already retrieved and destroyed %d.'",
              job->num_retrieved);
          }
        } else if (*job->hunted_victim != '\0') {
          sprintf(buf2, "$n whispers 'Master $N, I am going to kill %s.'",
            job->hunted_victim);

          if (job->num_retrieved > 0) {
            act(buf2, FALSE, me, 0, ch, TO_VICT);
            sprintf(buf2, "$n whispers 'I have already killed him %d %s.'",
              job->num_retrieved, job->num_retrieved == 1 ? "time" : "times");
          }
        } else {
          sprintf(buf2,
            "$n whispers 'Master $N, I am currently idle with memory "
            "allocated.'");
        }

        act(buf2, FALSE, me, 0, ch, TO_VICT);

        if (job->num_chances == -99)
          act("$n whispers 'I am showing no mercy.'", FALSE, me, 0, ch,
            TO_VICT);
        else
          act("$n whispers 'I am showing mercy.  Do I *have* to?'", FALSE, me,
            0, ch, TO_VICT);

        return TRUE;
      }

      struct mob_act_bountyhunter* job = NULL;

      if (!me->act_ptr || me->act_ptr->type != MOB_ACT_BOUNTYHUNTER) {
        if (me->act_ptr && me->act_ptr->type != MOB_ACT_BOUNTYHUNTER) {
          vlog(
            "bounty_hunter proc: ch->act_ptr is not of type "
            "MOB_ACT_BOUNTYHUNTER.  Resetting.");
          free(me->act_ptr);
          me->act_ptr = NULL;
        }

        me->act_ptr = create(struct mob_act_data, 1);
        me->act_ptr->type = MOB_ACT_BOUNTYHUNTER;
        job = &me->act_ptr->data.bountyhunter;
        *job->hunted_item = '\0';
        *job->hunted_victim = '\0';
        act("$n whispers 'Master $N, I am preparing to remember your will.",
          FALSE, me, 0, ch, TO_VICT);
      }

      assert(job);

      if (job->level_command > GetMaxLevel(ch)) {
        act("$n whispers 'Sorry, $N.  I report to a higher authority.'", FALSE,
          me, 0, ch, TO_VICT);
        return TRUE;
      }

      char buf[256];
      char buf2[256];

      if (sscanf(arg, " repo %s", buf) == 1) {
        if (strlen(buf) > 79) {
          return TRUE;
        }

        sprintf(buf2, "$n whispers 'Master $N, I will now reposess all %s.'",
          buf);
        act(buf2, FALSE, me, 0, ch, TO_VICT);

        vlogf("%s just ordered %s to repo %s", GET_NAME(ch),
          me->player.short_descr, buf);

        *job->hunted_victim = '\0';
        strcpy(job->hunted_item, buf);

        job->level_command = GetMaxLevel(ch);
        job->num_retrieved = 0;

        if (job->num_chances != -99)
          job->num_chances = 9;

        return TRUE;
      }

      if (sscanf(arg, " kill %s", buf) == 1) {
        if (strlen(buf) > 79)
          return TRUE;

        sprintf(buf2, "$n whispers 'Master $N, I will now kill %s.'", buf);
        act(buf2, FALSE, me, 0, ch, TO_VICT);

        vlogf("%s just sent out %s to kill %s", GET_NAME(ch),
          me->player.short_descr, buf);

        *job->hunted_item = '\0';
        strcpy(job->hunted_victim, buf);

        job->level_command = GetMaxLevel(ch);
        job->num_retrieved = 0;

        if (job->num_chances != -99)
          job->num_chances = 9;

        return TRUE;
      }

      if (!strncasecmp(arg, " forget", 7)) {
        act("$n whispers 'Master $N, I am clearing my memory.'", FALSE, me, 0,
          ch, TO_VICT);
        free(me->act_ptr);
        me->act_ptr = NULL;
        return TRUE;
      }

      if (!strncasecmp(arg, " no mercy", 9)) {
        act("$n whispers 'Master $N, I will have no mercy.'", FALSE, me, 0, ch,
          TO_VICT);

        job->num_chances = -99;

        vlogf(
          "%s just ordered %s to show no mercy! (and better have a damn good "
          "reason)",
          GET_NAME(ch), me->player.short_descr);

        return TRUE;
      }

      if (!strncasecmp(arg, " show mercy", 11)) {
        act("$n whispers 'Master $N, I will now show mercy.'", FALSE, me, 0, ch,
          TO_VICT);
        job->num_chances = 9;
        return TRUE;
      }

      act("$n whispers 'Master $N, I don't understand your request.'", FALSE,
        me, 0, ch, TO_VICT);
      act("$n whispers 'Please forgive me!'", FALSE, me, 0, ch, TO_VICT);

      return TRUE;
    }

    return FALSE;
  }

  // Cmd == 0, default behavior follows
  if (!ch->act_ptr || ch->act_ptr->type != MOB_ACT_BOUNTYHUNTER || !AWAKE(ch) ||
      !IS_NPC(ch) || ch->specials.fighting)
    return FALSE;

  struct mob_act_bountyhunter* job = &ch->act_ptr->data.bountyhunter;

  if (*job->hunted_item != '\0') {
    char buf[256];
    sprintf(buf, "1.%s", job->hunted_item);

    struct obj_data* temp_obj = get_obj_vis_world(ch, buf, NULL);

    if (!temp_obj) {
      *job->hunted_victim = '\0';
      return FALSE;
    }

    struct char_data* targ = char_holding(temp_obj);

    if (targ) {
      if (targ == ch) {
        if (temp_obj->equipped_by) {
          unequip_char(targ, temp_obj->eq_pos);
        }

        extract_obj(temp_obj);
        (job->num_retrieved)++;

        if (*job->hunted_victim != '\0') {
          do_action(ch, job->hunted_victim, 141);
          *job->hunted_victim = '\0';
        } else
          act("$n appears pleased.", FALSE, ch, NULL, targ, TO_ROOM);

        if (ch->master)
          stop_follower(ch);

        if (job->num_chances != -99)
          job->num_chances = 9;
        return TRUE;
      }

      if (*(job->hunted_victim) != '\0') {
        if (strcasecmp(job->hunted_victim, GET_NAME(targ))) {
          strcpy(job->hunted_victim, GET_NAME(targ));
          if (ch->master)
            stop_follower(ch);
          if (job->num_chances != -99)
            job->num_chances = 9;
        }
      } else {
        strcpy(job->hunted_victim, GET_NAME(targ));
        if (ch->master)
          stop_follower(ch);
        if (job->num_chances != -99)
          job->num_chances = 9;
      }
    }

    int room = room_of_object(temp_obj);

    if (room != ch->in_room) {
      static struct find_path_data find_room = {
        .type = FIND_TARGET_ROOM,
        .fn_data.target_room = 0,
        .fn.is_target_room_fn = is_target_room_p,
      };

      find_room.fn_data.target_room = room;
      int dir = find_path(ch->in_room, &find_room, -5000, 0);

      if (dir < 0) {
        /* unable to find a path */
        if (room > 0) {
          do_emote(ch,
            "looks about for clues of passage, as if tracking someone.", 0);
          do_emote(ch, "fades almost unnoticed into the shadows.", 0);
          char_from_room(ch);
          char_to_room(ch, room);
          do_emote(ch, "steps from the shadows.", 0);
        }

        return FALSE;
      }

      go_direction(ch, dir);
    } else {
      if (targ) {
        do_wake(ch, targ->player.name, 46);

        if (!circle_follow(ch, targ) && ch->master != targ) {
          if (ch->master) {
            stop_follower(ch);
          }

          add_follower(ch, targ);
        }

        switch (job->num_chances) {
          case 9:
            do_action(ch, targ->player.name, 187);

            sprintf(buf, "Thats a pretty nice %s you have there, %s.",
              job->hunted_item, targ->player.name);
            do_say(ch, buf, 17);

            do_say(ch,
              "If you do not give the item to me, I will kill you and take it "
              "myself.",
              17);

            if (IS_NPC(targ))
              do_say(targ, "Buzz off, creep.", 17);

          case 8:
          case 7:
          case 5:
          case 4:
          case 2:
          case 1:
            --job->num_chances;
            break;

          case 6:
            do_action(ch, targ->player.name, 190);

            sprintf(buf, "I'm not kidding.  Hand over the %s!",
              job->hunted_item);
            do_say(ch, buf, 17);

            --job->num_chances;

            if (IS_NPC(targ))
              do_say(targ, "I said, BUZZ OFF!", 17);

            break;

          case 3:
            do_action(ch, targ->player.name, 94);

            sprintf(buf, "This is your *LAST* warning.  Give me the %s or DIE!",
              job->hunted_item);
            do_say(ch, buf, 17);

            --job->num_chances;

            if (IS_NPC(targ))
              act("$n falls down laughing at $N.", FALSE, targ, NULL, ch,
                TO_ROOM);

            break;

          default:
            if (IS_NPC(targ)) {
              act("$n puts $N in a sleeper hold.", FALSE, ch, NULL, targ,
                TO_ROOM);

              sprintf(buf, "$n quickly surrenders the %s.", job->hunted_item);
              act(buf, FALSE, targ, NULL, targ, TO_ROOM);

              if (temp_obj->in_obj)
                obj_from_obj(temp_obj);
              else if (temp_obj->carried_by)
                obj_from_char(temp_obj);
              else if (temp_obj->equipped_by)
                unequip_char(targ, temp_obj->eq_pos);

              obj_to_char(temp_obj, ch);
            } else if (targ->desc && targ->desc->connected == CON_PLYNG) {
              if (check_peaceful(ch, "")) {
                if (job->num_chances > -5) {
                  --job->num_chances;

                  sprintf(buf,
                    "%s, I want your %s, and I MIGHT VERY WELL get tired of "
                    "waiting!",
                    GET_NAME(targ), job->hunted_item);
                  do_say(ch, buf, 17);

                  if (job->num_chances == -5) {
                    do_say(ch,
                      "Hell, thats it.  I *AM* tired of waiting for you.", 17);
                  }
                } else {
                  do_action(ch, GET_NAME(targ), 143);

                  if ((room = adjacent_room(ch->in_room)) < 1) {
                    act("$n mystically waves his hands at $N.", FALSE, ch, NULL,
                      targ, TO_NOTVICT);
                    act("$n mystically waves his hands at you!", FALSE, ch,
                      NULL, targ, TO_VICT);
                    act("$n and $N disappear into a swirling vortex!", FALSE,
                      ch, NULL, targ, TO_NOTVICT);
                    act("$n and you are sucked into a swirling vortex!", FALSE,
                      ch, NULL, targ, TO_VICT);

                    room = 6000; /* 6000 == edge of forest */
                  } else {
                    /* throw them in some direction */
                    act("$n grabs $N and throws $M from the room!", FALSE, ch,
                      NULL, targ, TO_NOTVICT);
                    act("$n grabs you and throws you from the room!", FALSE, ch,
                      NULL, targ, TO_VICT);
                  }

                  char_from_room(ch);
                  char_from_room(targ);
                  char_to_room(ch, room);
                  char_to_room(targ, room);
                  do_look(ch, "\0", 0);
                  do_look(targ, "\0", 0);

                  if (!check_peaceful(ch, "")) {
                    hit(ch, targ, 0);
                  }
                }
              } else
                hit(ch, targ, 0);
            } else {
              act("$n gets something from a link dead player and laughs evily.",
                FALSE, ch, NULL, targ, TO_ROOM);

              if (temp_obj->carried_by)
                obj_from_char(temp_obj);
              else if (temp_obj->equipped_by)
                unequip_char(targ, temp_obj->eq_pos);
              else if (temp_obj->in_obj)
                obj_from_obj(temp_obj);

              obj_to_char(temp_obj, ch);
            }

            break;
        }
      } else if (temp_obj->in_obj) { /* item is in object */
        obj_from_obj(temp_obj);
        obj_to_char(temp_obj, ch);
        act("$n picks up something quickly.", FALSE, ch, NULL, 0, TO_ROOM);
      } else if (temp_obj->in_room != NOWHERE) {
        obj_from_room(temp_obj);
        obj_to_char(temp_obj, ch);
        act("$n picks up something quickly.", FALSE, ch, NULL, 0, TO_ROOM);
      } else {
        sprintf(buf, "Bounty hunter stuck looking for disconnected %s.",
          job->hunted_item);
        vlog(buf);
        do_say(ch, "Damn am I confused!", 17);
        free(job);
        ch->act_ptr = NULL;
      }
    }
    return TRUE;
  }

  if (*(job->hunted_victim) != '\0') {
    struct char_data* targ = char_with_name(job->hunted_victim);

    if (!targ) {
      return FALSE;
    }

    if (targ->in_room == ch->in_room) {
      do_wake(ch, targ->player.name, 46);

      if (!circle_follow(ch, targ)) {
        if (ch->master != targ) {
          if (ch->master) {
            stop_follower(ch);
          }

          add_follower(ch, targ);
        }
      }

      switch (job->num_chances) {
        case 9:
          do_action(ch, targ->player.name, 187);
          do_say(ch, "You have been requested to log off by my employer.", 17);
          do_say(ch, "I suggest you comply.", 17);
        case 8:
        case 7:
        case 5:
        case 4:
        case 2:
        case 1:
          --job->num_chances;
          break;
        case 6:
          do_action(ch, targ->player.name, 190);
          do_say(ch, "I'm not kidding.", 17);
          --job->num_chances;
          break;
        case 3:
          do_action(ch, targ->player.name, 94);
          do_say(ch, "This is your *LAST* warning.", 17);
          --job->num_chances;
          break;
        default:
          if (check_peaceful(ch, ""))
            do_say(ch, "You can't stay here forever, you know.", 17);
          else
            hit(ch, targ, 0);
          break;
      }

      return TRUE;
    }

    static struct find_path_data find_room = {
      .type = FIND_TARGET_ROOM,
      .fn_data.target_room = 0,
      .fn.is_target_room_fn = is_target_room_p,
    };
    find_room.fn_data.target_room = targ->in_room;
    int dir = find_path(ch->in_room, &find_room, -5000, 0);

    if (dir >= 0) {
      go_direction(ch, dir);
      return TRUE;
    }
  }

  return FALSE;
}

int potentially_annoying(int cmd) {
  return ((cmd == 27) ||  /* laugh  */
          (cmd == 30) ||  /* puke   */
          (cmd == 31) ||  /* growl  */
          (cmd == 33) ||  /* insult */
          (cmd == 111) || /* fart   */
          (cmd == 112) || /* flip   */
          (cmd == 113) || /* fondle */
          (cmd == 118) || /* grope  */
          (cmd == 130) || /* slap   */
          (cmd == 137) || /* spit   */
          (cmd == 160) || /* french */
          (cmd == 182) || /* fume   */
          (cmd == 189) || /* punch  */
          (cmd == 190) || /* snarl  */
          (cmd == 191) || /* spank  */
          (cmd == 193) || /* tackle */
          (cmd == 194) || /* taunt  */
          (cmd == 196) || /* whine  */
          (cmd == 261) || /* bonk   */
          (cmd == 262) || /* scold  */
          (cmd == 264) || /* rip    */
          (cmd == 271) || /* belittle */
          (cmd == 272) || /* piledrive */
          (cmd == 285) || /* flipoff */
          (cmd == 286) || /* moon   */
          (cmd == 287) || /* pinch  */
          (cmd == 288));  /* bite   */
}

/* DO NOT use the following as a stand-alone spec_proc... it is meant to be  */
/* called by other spec_procs.  Use the more general i_am_irritable instead  */
int utility_irritable(struct char_data* ch, int cmd, char* arg, int (*func)()) {
  struct char_data* me;
  struct char_data* targ;

  if (!cmd || IS_NPC(ch))
    return FALSE;

  if (potentially_annoying(cmd)) {
    do_action(ch, arg, cmd);
    for (; ((*arg == ' ') || (*arg == '\t')); arg++)
      ;
    if (targ = get_char_room_vis(ch, arg))
      for (me = real_roomp(ch->in_room)->people; me; me = me->next_in_room)
        if (IS_MOB(me) && mob_index[me->nr].func.mob_f == func)
          if (!(me->specials.fighting) &&
              !strcmp(GET_NAME(targ), GET_NAME(me))) {
            act("$n becomes enraged by $N's behavior!", 1, me, 0, ch,
              TO_NOTVICT);
            act("$n becomes enraged with you!", 1, me, 0, ch, TO_VICT);
            do_hit(me, GET_NAME(ch), TYPE_UNDEFINED);
          }
    return TRUE;
  }
  return FALSE;
}

/* use this for mobs whose _ONLY_ spec_proc function is to be irritable */
int i_am_irritable(struct char_data* ch, int cmd, char* arg) {
  utility_irritable(ch, cmd, arg, i_am_irritable);
}

void PoliceHunt(struct char_data* ch, struct char_data* tch) {
#if NOTRACK
  return;
#endif
  SET_BIT(ch->specials.act, ACT_HUNTING);
  ch->specials.hunting = tch;
  ch->hunt_dist = 30;
  ch->persist = 100;
  ch->old_room = ch->in_room;
}

/* like utility_irritable, this function is to be called by OTHER spec_procs.
   If you want a mob to JUST be a police person, call the i_am_police function
   below.  - SG */
int utilityPolice(struct char_data* ch, int cmd, char* arg, int (*func)()) {
  struct char_data *targ, *me;

  if (cmd) { /* "ch" is not me.  It is a player. */
    if (IS_PC(ch) && (IS_OUTLAW(ch) || IS_KILLER(ch)) &&
        (ch->in_room != NOWHERE)) {
      for (me = real_roomp(ch->in_room)->people; me; me = me->next_in_room)
        if (IS_MOB(me) && (mob_index[me->nr].func.mob_f == func))
          if (AWAKE(me) && !(me->specials.fighting) &&
              (me->specials.hunting != ch) && CAN_SEE(me, ch)) {
            PoliceHunt(me, ch);
            if (!check_peaceful(ch, "")) {
              act("$n points at $N screaming 'DIE CRIMINAL!'", 1, me, 0, ch,
                TO_NOTVICT);
              act("$n points at you screaming 'DIE CRIMINAL!'", 1, me, 0, ch,
                TO_VICT);
              hit(me, ch, TYPE_UNDEFINED);
            }
          }
    }
  } else { /* "ch" is me, the policeman */
    if (AWAKE(ch) && !(ch->specials.fighting) && (ch->in_room != NOWHERE))
      for (targ = real_roomp(ch->in_room)->people; targ;
           targ = targ->next_in_room)
        if (IS_PC(ch) && (ch->specials.hunting != targ) &&
            (IS_OUTLAW(targ) || IS_KILLER(targ)) && (CAN_SEE(ch, targ))) {
          PoliceHunt(ch, targ);
          if (!check_peaceful(ch, "")) {
            hit(ch, targ, TYPE_UNDEFINED);
            act("$n points at $N screaming 'DIE CRIMINAL!'", 1, ch, 0, targ,
              TO_NOTVICT);
            act("$n points at you screaming 'DIE CRIMINAL!'", 1, ch, 0, targ,
              TO_VICT);
          }
          break;
        }
  }

  return FALSE;
}

/* use this for mobs whose _ONLY_ spec_proc function is to be police */
int i_am_police(Mob* ch, int cmd, char* arg) {
  return utilityPolice(ch, cmd, arg, i_am_police);
}

void obj_act(char* message, Mob* ch, Obj* o, Mob* vict) {
  char buffer[256];

  sprintf(buffer, "$n's $p %s", message);
  act(buffer, 1, ch, o, vict, TO_ROOM);
  sprintf(buffer, "Your $p %s", message);
  act(buffer, 1, ch, o, vict, TO_CHAR);
}

int warMaker(Mob* ch, int cmd, char* arg, Obj* o) {
  char buf[256];

  if ((cmd > 83) && (o->in_room == -1)) {
    if ((o->equipped_by != ch) && (o->carried_by != ch))
      return FALSE; /* safety net... should never happen */
    switch (cmd) {
      case 84:
      case 207:
        act("$n's $p hums 'Get this:  $n wants to use some sissy magic!'", 1,
          ch, o, NULL, TO_ROOM);
        act("Your $p hums 'Get this:  $n wants to use some sissy magic!'", 1,
          ch, o, NULL, TO_CHAR);
        send_to_char(
          "You feel confused... what was it you were going to do?\n\r", ch);
        return TRUE;
      case 151:
        if (o->equipped_by) {
          act("$n's $p glows red-hot in $s hands!", 1, ch, o, NULL, TO_ROOM);
          act("Your $p glows red-hot in your hands!", 1, ch, o, NULL, TO_CHAR);
          GET_HIT(ch) -= dice(5, 5);
          if (GET_HIT(ch) < 0) {
            GET_HIT(ch) = 0;
            GET_POS(ch) = POSITION_STUNNED;
          }
          if (dice(1, 20) > GET_CON(ch)) {
            unequip_char(ch, o->eq_pos);
            obj_to_room(o, ch->in_room);
            act("$n screams loudly, dropping $s $p.", 1, ch, o, NULL, TO_ROOM);
            act("You scream loudly, dropping your $p.", 1, ch, o, NULL,
              TO_CHAR);
          }
        }
      default:
        return FALSE;
    }
  } else if (!cmd) {
    if (o->in_room != -1) {
      sprintf(buf, "%s moves a bit... as if alive!\n\r", o->short_description);
      send_to_room(buf, room_of_object(o));
    } else if (ch = o->equipped_by) {
      if (!(ch->specials.fighting)) {
        switch (dice(1, 30)) {
          case 1:
            obj_act("sings 'Follow the Yellow Brick Road.'", ch, o, NULL);
            break;
          case 2:
            obj_act(
              "says, 'Let's go to the Shire, I want to waste some halfling "
              "youths.'",
              ch, o, NULL);
            break;
          case 3:
            obj_act("says, 'Use the Force, L.., I mean:  go get um tiger.'", ch,
              o, NULL);
            break;
          case 4:
            obj_act(
              "bemoans, 'How is it such a wonderous sword as me gets stuck "
              "with a wimp and coward like you?'",
              ch, o, NULL);
            break;
          case 5:
            obj_act(
              "says, 'We gonna stand here all day, or we gonna kill this "
              "thing?'",
              ch, o, NULL);
            break;
          case 6:
            obj_act("says, 'Group me.'", ch, o, NULL);
            break;
          case 7:
            obj_act("says, 'That's it.  Puff must die.'", ch, o, NULL);
            break;
          case 8:
            obj_act("says, 'Some of my closest friends are training daggers.'",
              ch, o, NULL);
            break;
          case 9:
            obj_act(
              "grins, 'I love it when they buff up mobs, it gives me a "
              "challenge.'",
              ch, o, NULL);
            break;
          case 10:
            obj_act(
              "wonders, 'Is it me, or are most other swords rather quiet?'", ch,
              o, NULL);
            break;
          case 11:
            obj_act("says, 'In a past life, I was a pipeweed bread.'", ch, o,
              NULL);
            break;
          case 12:
            obj_act("says, 'Puff sure is a friendly dragon, isn't he.'", ch, o,
              NULL);
            break;
          case 13:
            obj_act(
              "says, 'The imps must hate the players to make a sword SOooO "
              "AnnOYing!'",
              ch, o, NULL);
            break;
          case 14:
            obj_act(
              "wonders, 'How come I'm never forced to shout how much I love "
              "Puff?'",
              ch, o, NULL);
            break;
          case 15:
            obj_act("says, 'Let's see who's using Tin tin.  Snowy shouts 'Yo''",
              ch, o, NULL);
            break;
          case 16:
            obj_act(
              "screams, 'BANZAI, CHARGE, SPORK, GERONIMO, DIE VERMIN, HIEYAH!'",
              ch, o, NULL);
            obj_act(
              "blushes, 'Ooops, sorry, got over anxious to kill something.'",
              ch, o, NULL);
            break;
          case 17:
            obj_act(
              "offers, 'A good Swedish massage would cure what ails you.'", ch,
              o, NULL);
            break;
          case 18:
            obj_act("muses: 'How exactly *does* an inanimate object talk?'", ch,
              o, NULL);
            break;
          case 19:
            obj_act("states, 'I'd rather be playing scrabble.'", ch, o, NULL);
            break;
          case 20:
            obj_act(
              "says, 'A corpse is something to be cherished forever...until it "
              "rots.'",
              ch, o, NULL);
            break;
          case 21:
            obj_act(
              "sniffs the air, 'I love the smell of blood and ichor in the "
              "morning!'",
              ch, o, NULL);
            break;
          case 22:
            obj_act(
              "chants, 'You might have armor or you might have fur, I'll hack "
              "them both, cuz I'm War-make-er.'",
              ch, o, NULL);
            break;
          case 23:
            obj_act("scoffs, 'Sword of the Ancients, what a piece of junk!'",
              ch, o, NULL);
            break;
          case 24:
            obj_act(
              "says, 'Do I have to fight again?  I just got all the gore off "
              "from last time.'",
              ch, o, NULL);
            break;
          case 26:
            obj_act(
              "moans, 'If I have to kill Aunt Bee one more time, I'm gonna "
              "scream!'",
              ch, o, NULL);
            break;
          default:
            obj_act("grumbles 'Lets go kill something!'", ch, o, NULL);
            break;
        }
      }
    } else if (o->in_obj) {
      sprintf(buf,
        "Something grumbles 'Damnit, I'm %s.  Let me out of here.  Its "
        "dark.'\n\r",
        o->short_description);
      send_to_room(buf, room_of_object(o));
    } else if (ch = o->carried_by) {
      act("$n's $p begs $m to wield it.", 1, ch, o, NULL, TO_ROOM);
      act("Your $p begs you to wield it.", 1, ch, o, NULL, TO_CHAR);
    }
  } else if ((cmd == OBJECT_HITTING) && (dice(1, 10) == 1)) {
    switch (dice(1, 20)) {
      case 1:
        obj_act("criticizes $N's lineage.", o->equipped_by, o, ch);
        break;
      case 2:
        obj_act("says to $N, 'I fart in your general direction.'",
          o->equipped_by, o, ch);
        break;
      case 3:
        obj_act(
          "looks at $N and asks, 'Is that your real face or are you just "
          "naturally ugly?'",
          o->equipped_by, o, ch);
        break;
      case 4:
        obj_act("tells $N, 'If they held an ugly pageant, you'd be a shoe in.'",
          o->equipped_by, o, ch);
        break;
      case 5:
        obj_act("tells $N, 'I wave my private parts in your direction!'",
          o->equipped_by, o, ch);
        break;
      case 6:
        obj_act("snickers something about $N's mother and combat boots.",
          o->equipped_by, o, ch);
        break;
      case 7:
        obj_act(
          "asks $N with a smirk, 'So, how many newbies have killed you today?'",
          o->equipped_by, o, ch);
        break;
      case 8:
        obj_act("states, 'Ya know, your sister was a mighty fine piece of ...'",
          o->equipped_by, o, ch);
        break;
      case 9:
        obj_act("looks at $N and says 'Not worth the time.'", o->equipped_by, o,
          ch);
        break;
      case 10:
        obj_act(
          "tells $N, 'Killing you is about as challenging as buying bread.'",
          o->equipped_by, o, ch);
        break;
      case 11:
        obj_act("laughs at the stupidity of $N.", o->equipped_by, o, ch);
        break;
      case 12:
        obj_act("ponders: 'think I can kill $N in 5 rounds or less?'",
          o->equipped_by, o, ch);
        break;
      case 13:
        obj_act("snickers, 'OOH! Look at the the fangs on that one.'",
          o->equipped_by, o, ch);
        break;
      case 14:
        obj_act(
          "groans, '$N?!!  Surely you jest?  Haven't we killed $M enough all "
          "ready?'",
          o->equipped_by, o, ch);
        break;
      case 15:
        obj_act(
          "looks at $N and sighs, 'When you get around to attacking something "
          "real, wake me.'",
          o->equipped_by, o, ch);
        break;
      case 16:
        obj_act(
          "grins, 'Your head will look great mounted up above my mantle "
          "piece.'",
          o->equipped_by, o, ch);
        break;
      case 17:
        obj_act(
          "looks at $N and says, 'Are you as stupid as you look, or do you "
          "just look stupid?'",
          o->equipped_by, o, ch);
        break;
      default:
        obj_act("taunts $N mercilessly.", o->equipped_by, o, ch);
        break;
    }
    if (ch->specials.fighting != o->equipped_by) {
      act("$N turns towards $n with rage in $S eyes.", 1, o->equipped_by, o, ch,
        TO_ROOM);
      act("$N turns towards you with rage in $S eyes.", 1, o->equipped_by, o,
        ch, TO_CHAR);
      stop_fighting(ch);
      set_fighting(ch, o->equipped_by);
    }
  }

  return FALSE;
}

int orbOfDestruction(Mob* ch, int cmd, char* arg, Obj* o) {
  char buffer[256];
  Mob *v, *n;
  Room* r;

  if (cmd == 172) { /* use */
    arg = one_argument(arg, buffer);
    if (isname(buffer, o->name)) {
      if (GET_INT(ch) < 15)
        send_to_char("You can't figure out how to use it.\n\r", ch);
      else if (!o->equipped_by)
        send_to_char("You must be holding it to use it.\n\r", ch);
      else {
        act("You trigger $p, causing it to explode in a fiery blast of light!",
          1, ch, o, NULL, TO_CHAR);
        act("$n's $p explodes in a fiery blast of light!", 1, ch, o, NULL,
          TO_ROOM);
        unequip_char(ch, o->eq_pos);
        extract_obj(o);
        r = real_roomp(ch->in_room);
        for (v = r->people; v; v = n) {
          n = v->next_in_room;
          if (v != ch)
            MissileDamage(ch, v, dice(10, 100), SPELL_DISINTEGRATE);
        }
        GET_HIT(ch) = 1;
        spell_teleport(50, ch, ch, NULL);
      }
      return TRUE;
    }
  }
  return FALSE;
}
