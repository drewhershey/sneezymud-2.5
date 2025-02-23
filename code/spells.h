/* ************************************************************************
 *  file: spells.h , Implementation of magic spells.       Part of DIKUMUD *
 *  Usage : Spells                                                         *
 ************************************************************************* */

#ifndef SPELLS_H
#define SPELLS_H

#include "structs.h"

#define MAX_BUF_LENGTH 240

#define TYPE_UNDEFINED -1
#define SPELL_RESERVED_DBC 0       /* SKILL NUMBER ZERO */
#define SPELL_ARMOR 1              /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_TELEPORT 2           /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLESS 3              /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLINDNESS 4          /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BURNING_HANDS 5      /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CALL_LIGHTNING 6     /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHARM_PERSON 7       /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHILL_TOUCH 8        /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CLONE 9              /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_COLOUR_SPRAY 10      /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CONTROL_WEATHER 11   /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_FOOD 12       /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_WATER 13      /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_BLIND 14        /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_CRITIC 15       /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_LIGHT 16        /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURSE 17             /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_EVIL 18       /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_INVISIBLE 19  /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_MAGIC 20      /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_POISON 21     /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_EVIL 22       /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_EARTHQUAKE 23        /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENCHANT_WEAPON 24    /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENERGY_DRAIN 25      /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_FIREBALL 26          /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HARM 27              /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HEAL 28              /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INVISIBLE 29         /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LIGHTNING_BOLT 30    /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LOCATE_OBJECT 31     /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_MAGIC_MISSILE 32     /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_POISON 33            /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_PROTECT_FROM_EVIL 34 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_CURSE 35      /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SANCTUARY 36         /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SHOCKING_GRASP 37    /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SLEEP 38             /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_STRENGTH 39          /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SUMMON 40            /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_VENTRILOQUATE 41     /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WORD_OF_RECALL 42    /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_POISON 43     /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SENSE_LIFE 44        /* Reserved Skill[] DO NOT CHANGE */

/* types of attacks and skills must NOT use same numbers as spells! */

#define SKILL_SNEAK 45     /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_HIDE 46      /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_STEAL 47     /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_BACKSTAB 48  /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_PICK_LOCK 49 /* Reserved Skill[] DO NOT CHANGE */

#define SKILL_KICK 50   /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_BASH 51   /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_RESCUE 52 /* MAXIMUM SKILL NUMBER  */

/* END OF SKILL RESERVED "NO TOUCH" NUMBERS */

/* NEW SPELLS are to be inserted here */
#define SPELL_IDENTIFY 53
#define SPELL_INFRAVISION 54
#define SPELL_CAUSE_LIGHT 55
#define SPELL_CAUSE_CRITICAL 56
#define SPELL_FLAMESTRIKE 57
#define SPELL_DISPEL_GOOD 58
#define SPELL_WEAKNESS 59
#define SPELL_DISPEL_MAGIC 60
#define SPELL_KNOCK 61
#define SPELL_KNOW_ALIGNMENT 62
#define SPELL_ANIMATE_DEAD 63
#define SPELL_PARALYSIS 64
#define SPELL_REMOVE_PARALYSIS 65
#define SPELL_FEAR 66
#define SPELL_ACID_BLAST 67
#define SPELL_WATER_BREATH 68
#define SPELL_FLY 69
#define SPELL_CONE_OF_COLD 70
#define SPELL_METEOR_SWARM 71
#define SPELL_ICE_STORM 72
#define SPELL_SHIELD 73
#define SPELL_MON_SUM_1 74 /* done */
#define SPELL_MON_SUM_2 75
#define SPELL_MON_SUM_3 76
#define SPELL_MON_SUM_4 77
#define SPELL_MON_SUM_5 78
#define SPELL_MON_SUM_6 79
#define SPELL_MON_SUM_7 80 /* done  */
#define SPELL_FIRESHIELD 81
#define SPELL_CHARM_MONSTER 82
#define SPELL_CURE_SERIOUS 83
#define SPELL_CAUSE_SERIOUS 84
#define SPELL_REFRESH 85
#define SPELL_SECOND_WIND 86
#define SPELL_TURN 87
#define SPELL_SUCCOR 88
#define SPELL_LIGHT 89
#define SPELL_CONT_LIGHT 90
#define SPELL_CALM 91
#define SPELL_STONE_SKIN 92
#define SPELL_CONJURE_ELEMENTAL 93
#define SPELL_TRUE_SIGHT 94
#define SPELL_MINOR_CREATE 95
#define SPELL_FAERIE_FIRE 96
#define SPELL_FAERIE_FOG 97
#define SPELL_CACAODEMON 98
#define SPELL_POLY_SELF 99
#define SPELL_MANA 100
#define SPELL_ASTRAL_WALK 101
#define SPELL_RESURRECTION 102

#define SPELL_H_FEAST 103
#define SPELL_FLY_GROUP 104
#define SPELL_DRAGON_BREATH 105
#define SPELL_WEB 106
#define SPELL_MINOR_TRACK 107
#define SPELL_MAJOR_TRACK 108
#define SPELL_FULL_HEAL 109
#define SPELL_VITALIZE_MANA 110
#define SPELL_PORTAL 111
#define SPELL_SILENCE 112
#define SPEC_SHOOT 113
#define SPELL_HEAL_SPRAY 114
#define SPELL_VAMPIRIC_TOUCH 115
#define SPELL_SYNOSTODWEOMER 116
#define SPELL_LIFE_LEECH 117
#define SPELL_CONTROL_UNDEAD 118
#define SPELL_WELL_OF_KNOWLEDGE 119
#define SPELL_FARLOOK 120
#define SPELL_DISPEL_INVISIBLE 121
#define SPEC_BOW 122
#define SPELL_PROTECT_GOOD 123
#define SPELL_DISINTEGRATE 124
#define SPELL_GOLEM 125

#define MAX_EXIST_SPELL 125 /* move this and change it */

#define SPELL_PWORD_KILL
#define SPELL_PWORD_BLIND
#define SPELL_SUMMON_OBJ
#define SPELL_FAMILIAR
#define SPELL_CHAIN_LIGHTNING
#define SPELL_SCARE
#define SPELL_SENDING
#define SPELL_HOLY_WORD
#define SPELL_UNHOLY_WORD
#define SPELL_AID
#define SPELL_COMMAND

/* MAX_SKILL is 200!  */
/* No more than 200 spells!!!!!! */
/*  Ooh!  What a limitation!  */

#define SKILL_SIGN 160
#define SKILL_SWIM 161
#define SKILL_CONS_UNDEAD 162
#define SKILL_CONS_VEGGIE 163
#define SKILL_CONS_DEMON 164
#define SKILL_CONS_ANIMAL 165
#define SKILL_CONS_REPTILE 166
#define SKILL_CONS_PEOPLE 167
#define SKILL_CONS_GIANT 168
#define SKILL_CONS_OTHER 169
#define SKILL_SWITCH_OPP 170
#define SKILL_FEIGN_DEATH 171
#define SKILL_FIRST_AID 172
#define SKILL_DODGE 173
#define SKILL_QUIV_PALM 174
#define SKILL_SPRING_LEAP 175
#define SKILL_LAY_HANDS 176
#define SKILL_REMOVE_TRAP 177
#define SKILL_FIND_TRAP 178
#define SKILL_RETREAT 179
#define SKILL_HUNT 180
#define SKILL_SET_TRAP 182
#define SKILL_DISARM 183
#define SKILL_READ_MAGIC 184
#define SKILL_GRAPPLE 186
#define SKILL_HEADBUTT 187
#define SKILL_SUBTERFUGE 188
#define SKILL_THROW 189
#define SKILL_BREW 190
#define SKILL_SCRIBE 191
#define SKILL_DOUBLE_ATTACK 192
#define SKILL_DEATHSTROKE 193
#define SKILL_BODYSLAM 194
#define SKILL_DETECT_SECRET 196
#define SKILL_SPY 197
#define SKILL_DOORBASH 198

#define SPELL_GREEN_SLIME 199

#define FIRST_BREATH_WEAPON 200
#define SPELL_GEYSER 200
#define SPELL_FIRE_BREATH 201
#define SPELL_GAS_BREATH 202
#define SPELL_FROST_BREATH 203
#define SPELL_ACID_BREATH 204
#define SPELL_LIGHTNING_BREATH 205
#define LAST_BREATH_WEAPON 205

#define TYPE_HIT 206
#define TYPE_BLUDGEON 207
#define TYPE_PIERCE 208
#define TYPE_SLASH 209
#define TYPE_WHIP 210  /* EXAMPLE */
#define TYPE_CLAW 211  /* NO MESSAGES WRITTEN YET! */
#define TYPE_BITE 212  /* NO MESSAGES WRITTEN YET! */
#define TYPE_STING 213 /* NO MESSAGES WRITTEN YET! */
#define TYPE_CRUSH 214 /* NO MESSAGES WRITTEN YET! */
#define TYPE_CLEAVE 215
#define TYPE_STAB 216
#define TYPE_SMASH 217
#define TYPE_SMITE 218

#define TYPE_SUFFERING 220

/* More anything but spells and weapontypes can be insterted here! */

#define MAX_TYPES 70

#define SAVING_PARA 0
#define SAVING_ROD 1
#define SAVING_PETRI 2
#define SAVING_BREATH 3
#define SAVING_SPELL 4

#define MAX_SPL_LIST 215

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

struct spell_info_type {
    void (*spell_pointer)(byte level, struct char_data* ch, char* arg, int type,
      struct char_data* tar_ch, struct obj_data* tar_obj);
    byte minimum_position;     /* Position for caster 			*/
    unsigned int min_usesmana; /* Amount of mana used by a spell	 */
    byte beats;                /* Heartbeats until ready for next */

    byte min_level_cleric; /* Level required for cleric       */
    byte min_level_magic;  /* Level required for magic user   */
    byte min_level_anti;   /* Level for antipaladins          */
    byte min_level_pal;    /* Level for paladins              */
    byte min_level_ranger; /* Level for rangers               */
    sh_int targets;        /* See below for use with TAR_XXX  */
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

#define SPELL_TYPE_SPELL 0
#define SPELL_TYPE_POTION 1
#define SPELL_TYPE_WAND 2
#define SPELL_TYPE_STAFF 3
#define SPELL_TYPE_SCROLL 4

/* Attacktypes with grammar */

#define TASK_TRIVIAL 0
#define TASK_EASY 1
#define TASK_NORMAL 2
#define TASK_DIFFICULT 3
#define TASK_DANGEROUS 4
#define CRITICAL_FAILURE (-1)
#define CRITICAL_SUCCESS 1
#define NORMAL_RESULT 0

extern const byte saving_throws[8][5][ABS_MAX_LVL];
extern const funcp bweapons[];

bool saves_spell(struct char_data* ch, sh_int save_type);
int can_do_verbal(struct char_data* ch);

// Spell behavior fns
void spell_dispel_magic(byte level, struct char_data* ch,
  struct char_data* victim, struct obj_data* obj);
void spell_animate_dead(byte level, struct char_data* ch,
  struct char_data* victim, struct obj_data* corpse);

// Cast handlers
void cast_teleport(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void spell_teleport(byte level, struct char_data* ch, struct char_data* victim,
  struct obj_data* obj);
void cast_fear(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void spell_astral_walk(byte level, struct char_data* ch,
  struct char_data* victim, struct obj_data* obj);
void spell_summon(byte level, struct char_data* ch, struct char_data* victim,
  struct obj_data* obj);
void cast_cure_blind(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_dispel_magic(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_astral_walk(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_web(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_armor(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_charm_monster(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_mon_sum1(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_mon_sum2(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_mon_sum3(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_mon_sum4(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_mon_sum5(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_mon_sum6(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_mon_sum7(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_farlook(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_shocking_grasp(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* victim, struct obj_data* tar_obj);
void cast_acid_blast(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* victim, struct obj_data* tar_obj);
void cast_lightning_bolt(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* victim, struct obj_data* tar_obj);
void cast_ice_storm(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* victim, struct obj_data* tar_obj);
void cast_vampiric_touch(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* victim, struct obj_data* tar_obj);
void cast_call_lightning(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* victim, struct obj_data* tar_obj);
void cast_cone_of_cold(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* victim, struct obj_data* tar_obj);
void cast_disintegrate(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* victim, struct obj_data* tar_obj);
void cast_create_golem(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* victim, struct obj_data* tar_obj);
void cast_cause_light(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* victim, struct obj_data* tar_obj);
void cast_cause_critic(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* victim, struct obj_data* tar_obj);
void cast_flamestrike(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* victim, struct obj_data* tar_obj);
void cast_harm(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* victim, struct obj_data* tar_obj);
void cast_animate_dead(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_conjure_elemental(byte level, struct char_data* ch, char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_bless(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_blindness(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_burning_hands(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_charm_person(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_cacaodemon(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_chill_touch(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_clone(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_colour_spray(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_control_weather(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_create_food(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_create_water(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_cure_critic(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_cure_light(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_curse(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_cont_light(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_calm(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_detect_evil(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_detect_invisibility(byte level, struct char_data* ch, char* arg,
  int si, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_detect_magic(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_detect_poison(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_dispel_evil(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_dispel_good(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_earthquake(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_enchant_weapon(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_energy_drain(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_fireball(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_flying(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_heal(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_full_heal(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_infravision(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_invisibility(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_knock(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_know_alignment(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_true_seeing(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_minor_creation(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_faerie_fire(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_faerie_fog(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_heroes_feast(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_fly_group(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_minor_track(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_major_track(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_mana(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_light(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_locate_object(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_magic_missile(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_vitalize_mana(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_meteor_swarm(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_poly_self(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_poison(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_protection_from_evil(byte level, struct char_data* ch, char* arg,
  int si, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_remove_curse(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_sanctuary(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_sleep(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_strength(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_stone_skin(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_summon(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_ventriloquate(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_word_of_recall(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_water_breath(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_remove_poison(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_remove_paralysis(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_weakness(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_sense_life(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_identify(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_paralyze(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_dragon_breath(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* potion);
void cast_fireshield(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_cure_serious(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_cause_serious(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_refresh(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_second_wind(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_shield(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_turn(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_well_of_knowledge(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_succor(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_resurrection(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_portal(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_dispel_invisible(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_silence(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_heal_spray(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_life_leech(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_synostodweomer(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_control_undead(byte level, struct char_data* ch, char* arg, int si,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_geyser(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* victim, struct obj_data* tar_obj);
void cast_green_slime(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* victim, struct obj_data* tar_obj);
void cast_frost_breath(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_acid_breath(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_gas_breath(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_lightning_breath(byte level, struct char_data* ch, char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_fire_breath(byte level, struct char_data* ch, char* arg, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj);

#endif
