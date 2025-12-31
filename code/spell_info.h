#pragma once

#include "game_constants.h"
#include "spell_ids.h"
#include "structs.h"

/* Saving throw types */
#define SAVING_PARA 0
#define SAVING_ROD 1
#define SAVING_PETRI 2
#define SAVING_BREATH 3
#define SAVING_SPELL 4

#define MAX_SPL_LIST 215

/* Target flags for spells */
#define TAR_IGNORE (1 << 0)
#define TAR_CHAR_ROOM (1 << 1)
#define TAR_CHAR_WORLD (1 << 2)
#define TAR_FIGHT_SELF (1 << 3)
#define TAR_FIGHT_VICT (1 << 4)
#define TAR_SELF_ONLY (1 << 5) /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_SELF_NONO (1 << 6) /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_OBJ_INV (1 << 7)
#define TAR_OBJ_ROOM (1 << 8)
#define TAR_OBJ_WORLD (1 << 9)
#define TAR_OBJ_EQUIP (1 << 10)
#define TAR_NAME (1 << 11)
#define TAR_VIOLENT (1 << 12)
#define TAR_ROOM (1 << 13)   /* spells which target the room  */
#define TAR_SINGLE (1 << 14) /* spells made specifically for single classes */

/* Forward declarations */
struct char_data;
struct obj_data;

struct spell_info_type {
    void (*spell_pointer)(signed char level, struct char_data* ch,
      const char* arg, int type, struct char_data* tar_ch,
      struct obj_data* tar_obj);
    signed char minimum_position; /* Position for caster */
    unsigned int min_usesmana;    /* Amount of mana used by a spell */
    signed char beats;            /* Heartbeats until ready for next */

    signed char min_level_cleric; /* Level required for cleric */
    signed char min_level_magic;  /* Level required for magic user */
    signed char min_level_anti;   /* Level for antipaladins */
    signed char min_level_pal;    /* Level for paladins */
    signed char min_level_ranger; /* Level for rangers */
    short int targets;            /* See below for use with TAR_XXX */
};

extern struct spell_info_type spell_info[MAX_SPL_LIST];
extern const char* const spells[];

/* Possible Targets:

   bit 0 : IGNORE TARGET
   bit 1 : PC/NPC in room
   bit 2 : PC/NPC in world
   bit 3 : Object held
   bit 4 : Object in inventory
   bit 5 : Object in room
   bit 6 : Object in world
   bit 7 : If fighting, and no argument, select tar_char as self
   bit 8 : If fighting, and no argument, select tar_char as victim (fighting)
   bit 9 : If no argument, select self, if argument check that it IS self.

*/

/* Spell type constants */
#define SPELL_TYPE_SPELL 0
#define SPELL_TYPE_POTION 1
#define SPELL_TYPE_WAND 2
#define SPELL_TYPE_STAFF 3
#define SPELL_TYPE_SCROLL 4

/* Task difficulty constants */
#define TASK_TRIVIAL 0
#define TASK_EASY 1
#define TASK_NORMAL 2
#define TASK_DIFFICULT 3
#define TASK_DANGEROUS 4
#define CRITICAL_FAILURE (-1)
#define CRITICAL_SUCCESS 1
#define NORMAL_RESULT 0

/* External data tables */
extern const signed char saving_throws[8][5][ABS_MAX_LVL];

static_assert(MAX_EXIST_SPELL <= MAX_SPL_LIST,
  "MAX_EXIST_SPELL exceeds MAX_SPL_LIST - spell_info array too small");
static_assert(LAST_BREATH_WEAPON <= MAX_SPL_LIST,
  "LAST_BREATH_WEAPON exceeds MAX_SPL_LIST");
static_assert(MAX_TYPES >= 70, "MAX_TYPES must be at least 70");
