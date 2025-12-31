#pragma once

/*
 * Multiclass level indices
 */
#define MAGE_LEVEL_IND 0
#define CLERIC_LEVEL_IND 1
#define WARRIOR_LEVEL_IND 2
#define THIEF_LEVEL_IND 3
#define ANTIPALADIN_LEVEL_IND 4
#define PALADIN_LEVEL_IND 5
#define MONK_LEVEL_IND 6
#define RANGER_LEVEL_IND 7

/*
 * Hate flags (for mobile AI)
 */
#define HATE_SEX 1
#define HATE_RACE 2
#define HATE_CHAR 4
#define HATE_CLASS 8
#define HATE_EVIL 16
#define HATE_GOOD 32
#define HATE_VNUM 64

/*
 * Fear flags (for mobile AI)
 */
#define FEAR_SEX 1
#define FEAR_RACE 2
#define FEAR_CHAR 4
#define FEAR_CLASS 8
#define FEAR_EVIL 16
#define FEAR_GOOD 32
#define FEAR_VNUM 64

/*
 * Opinion operation types
 */
#define OP_SEX 1
#define OP_RACE 2
#define OP_CHAR 3
#define OP_CLASS 4
#define OP_EVIL 5
#define OP_GOOD 6
#define OP_VNUM 7

/*
 * Immunity flags
 */
#define IMM_FIRE (1 << 0)
#define IMM_COLD (1 << 1)
#define IMM_ELEC (1 << 2)
#define IMM_ENERGY (1 << 3)
#define IMM_BLUNT (1 << 4)
#define IMM_PIERCE (1 << 5)
#define IMM_SLASH (1 << 6)
#define IMM_ACID (1 << 7)
#define IMM_POISON (1 << 8)
#define IMM_DRAIN (1 << 9)
#define IMM_SLEEP (1 << 10)
#define IMM_CHARM (1 << 11)
#define IMM_HOLD (1 << 12)
#define IMM_NONMAG (1 << 13)
#define IMM_PLUS1 (1 << 14)
#define IMM_PLUS2 (1 << 15)
#define IMM_PLUS3 (1 << 16)
#define IMM_PLUS4 (1 << 17)

/*
 * Equipment wear positions
 */
#define WEAR_LIGHT 0
#define WEAR_FINGER_R 1
#define WEAR_FINGER_L 2
#define WEAR_NECK_1 3
#define WEAR_NECK_2 4
#define WEAR_BODY 5
#define WEAR_HEAD 6
#define WEAR_LEGS 7
#define WEAR_FEET 8
#define WEAR_HANDS 9
#define WEAR_ARMS 10
#define WEAR_SHIELD 11
#define WEAR_ABOUT 12
#define WEAR_WAISTE 13
#define WEAR_WRIST_R 14
#define WEAR_WRIST_L 15
#define WIELD 16
#define HOLD 17
#define WEAR_EAR 18
#define WEAR_FACE 19
#define WEAR_RADIO 20

/*
 * Tongue/language indices
 * Note: #2 is used for active/inactive status for dead characters
 */
#define MAX_TOUNGE 3 /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */

#define MAX_SKILLS 200 /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
#define MAX_WEAR 25
#define MAX_AFFECT 25 /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */

/*
 * Predefined conditions
 */
#define DRUNK 0
#define FULL 1
#define THIRST 2

/*
 * Bitvector for 'affected_by' (AFF_* flags)
 */
#define AFF_BLIND (1UL << 0)
#define AFF_INVISIBLE (1UL << 1)
#define AFF_DETECT_EVIL (1UL << 2)
#define AFF_DETECT_INVISIBLE (1UL << 3)
#define AFF_DETECT_MAGIC (1UL << 4)
#define AFF_SENSE_LIFE (1UL << 5)
#define AFF_LIFE_PROT (1UL << 6)
#define AFF_SANCTUARY (1UL << 7)
#define AFF_GROUP (1UL << 8)
/* bit 9 is unused */
#define AFF_CURSE (1UL << 10)
#define AFF_FLYING (1UL << 11)
#define AFF_POISON (1UL << 12)
#define AFF_PROTECT_EVIL (1UL << 13)
#define AFF_PARALYSIS (1UL << 14)
#define AFF_INFRAVISION (1UL << 15)
#define AFF_WATERBREATH (1UL << 16)
#define AFF_SLEEP (1UL << 17)
#define AFF_KILLABLE (1UL << 18)
#define AFF_SNEAK (1UL << 19)
#define AFF_HIDE (1UL << 20)
#define AFF_PROTECT_FROM_GOOD (1UL << 21)
#define AFF_CHARM (1UL << 22)
#define AFF_FOLLOW (1UL << 23)
#define AFF_UNDEF_1 (1UL << 24)
#define AFF_TRUE_SIGHT (1UL << 25)
#define AFF_BREWING (1UL << 26)
#define AFF_FIRESHIELD (1UL << 27)
#define AFF_SILENT (1UL << 28)
#define AFF_GRAPPLE (1UL << 29)
#define AFF_GRAPPLE2 (1UL << 30)
#define AFF_SCRYING (1UL << 31)

/*
 * Modifiers to character abilities (APPLY_* flags)
 */
#define APPLY_NONE 0
#define APPLY_STR 1
#define APPLY_DEX 2
#define APPLY_INT 3
#define APPLY_WIS 4
#define APPLY_CON 5
#define APPLY_SEX 6
#define APPLY_CLASS 7
#define APPLY_LEVEL 8
#define APPLY_AGE 9
#define APPLY_CHAR_WEIGHT 10
#define APPLY_CHAR_HEIGHT 11
#define APPLY_MANA 12
#define APPLY_HIT 13
#define APPLY_MOVE 14
#define APPLY_GOLD 15
#define APPLY_EXP 16
#define APPLY_AC 17
#define APPLY_ARMOR 17
#define APPLY_HITROLL 18
#define APPLY_DAMROLL 19
#define APPLY_SAVING_PARA 20
#define APPLY_SAVING_ROD 21
#define APPLY_SAVING_PETRI 22
#define APPLY_SAVING_BREATH 23
#define APPLY_SAVING_SPELL 24
#define APPLY_SAVE_ALL 25
#define APPLY_IMMUNE 26
#define APPLY_SUSC 27
#define APPLY_M_IMMUNE 28
#define APPLY_SPELL 29
#define APPLY_WEAPON_SPELL 30
#define APPLY_EAT_SPELL 31
#define APPLY_BACKSTAB 32
#define APPLY_KICK 33
#define APPLY_SNEAK 34
#define APPLY_HIDE 35
#define APPLY_BASH 36
#define APPLY_PICK 37
#define APPLY_STEAL 38
#define APPLY_TRACK 39
#define APPLY_HITNDAM 40
#define APPLY_DOUBLE_ATTACK 41
#define APPLY_DEATHSTROKE 42
#define APPLY_PARRY 43
#define APPLY_THROW 44
#define APPLY_GRAPPLE 45

/*
 * Character classes (CLASS_* flags)
 */
#define CLASS_MAGIC_USER 1
#define CLASS_CLERIC 2
#define CLASS_WARRIOR 4
#define CLASS_THIEF 8
#define CLASS_ANTIPALADIN 16
#define CLASS_PALADIN 32
#define CLASS_MONK 64
#define CLASS_RANGER 128

/*
 * Character sex
 */
#define SEX_NEUTRAL 0
#define SEX_MALE 1
#define SEX_FEMALE 2

/*
 * Character positions
 */
#define POSITION_DEAD 0
#define POSITION_MORTALLYW 1
#define POSITION_INCAP 2
#define POSITION_STUNNED 3
#define POSITION_SLEEPING 4
#define POSITION_RESTING 5
#define POSITION_SITTING 6
#define POSITION_FIGHTING 7
#define POSITION_STANDING 8

/*
 * Mobile action flags (ACT_* for specials.act on NPCs)
 */
#define ACT_SPEC (1 << 0)       /* special routine to be called if exist   */
#define ACT_SENTINEL (1 << 1)   /* this mobile not to be moved             */
#define ACT_SCAVENGER (1 << 2)  /* pick up stuff lying around              */
#define ACT_ISNPC (1 << 3)      /* This bit is set for use with IS_NPC()   */
#define ACT_NICE_THIEF (1 << 4) /* Set if a thief should NOT be killed     */
#define ACT_AGGRESSIVE (1 << 5) /* Set if automatic attack on NPC's        */
#define ACT_STAY_ZONE (1 << 6)  /* MOB Must stay inside its own zone       */
#define ACT_WIMPY (1 << 7)      /* MOB Will flee when injured, and if      */
                                /* aggressive only attack sleeping players */
#define ACT_ANNOYING (1 << 8)   /* MOB is so utterly irritating that other */
                                /* monsters will attack it...              */
#define ACT_HATEFUL (1 << 9)    /* MOB will attack a PC or NPC matching a  */
                                /* specified name                          */
#define ACT_AFRAID (1 << 10)    /* MOB is afraid of a certain PC or NPC,   */
                                /* and will always run away ....           */
#define ACT_IMMORTAL (1 << 11)  /* MOB is a natural event, can't be killed */
#define ACT_HUNTING (1 << 12)   /* MOB is hunting someone                  */
#define ACT_DEADLY (1 << 13)    /* MOB has deadly poison                   */
#define ACT_POLYSELF (1 << 14)  /* MOB is a polymorphed person             */
#define ACT_META_AGG (1 << 15)  /* MOB is _very_ aggressive                */
#define ACT_GUARDIAN (1 << 16)  /* MOB will guard master                   */

/*
 * Player flags (PLR_* for specials.act on PCs)
 */
#define PLR_BRIEF (1 << 0)
#define PLR_COMPACT (1 << 1)
#define PLR_WIMPY (1 << 2) /* character will flee when seriously injured */
#define PLR_DONTSET (1 << 3)
#define PLR_NOHASSLE (1 << 4) /* char won't be attacked by aggressives */
#define PLR_STEALTH \
  (1 << 5) /* char won't be announced in a variety of situations */
#define PLR_HUNTING \
  (1 << 6) /* the player is hunting someone, do a track each look */
#define PLR_MAILING (1 << 7)
#define PLR_LOGGED (1 << 8)
#define PLR_KILLER (1 << 9)
#define PLR_VT100 (1 << 10) /* VT100 capable */
#define PLR_COLOR (1 << 11)
#define PLR_OUTLAW (1 << 12)
#define PLR_ANSI (1 << 13)
#define PLR_NOSHOUT (1 << 14)  /* the player is not allowed to shout */
#define PLR_BANISHED (1 << 15) /* The player goes to hell on login */
