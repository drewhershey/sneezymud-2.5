#pragma once

/*
 * Object type flags (for obj_flags.type_flag)
 */
#define ITEM_LIGHT 1
#define ITEM_SCROLL 2
#define ITEM_WAND 3
#define ITEM_STAFF 4
#define ITEM_WEAPON 5
#define ITEM_FIREWEAPON 6
#define ITEM_MISSILE 7
#define ITEM_TREASURE 8
#define ITEM_ARMOR 9
#define ITEM_POTION 10
#define ITEM_WORN 11
#define ITEM_OTHER 12
#define ITEM_TRASH 13
#define ITEM_TRAP 14
#define ITEM_CONTAINER 15
#define ITEM_NOTE 16
#define ITEM_DRINKCON 17
#define ITEM_KEY 18
#define ITEM_FOOD 19
#define ITEM_MONEY 20
#define ITEM_PEN 21
#define ITEM_BOAT 22
#define ITEM_AUDIO 23
#define ITEM_BOARD 24
#define ITEM_BOW 25
#define ITEM_ARROW 26
#define ITEM_RADIO 27
#define ITEM_CORPSE 28
#define ITEM_SPELLBAG 29
#define ITEM_COMPONENT 30
#define ITEM_BOOK 31

/*
 * Bitvector for 'wear_flags' (where item can be worn/used)
 */
#define ITEM_TAKE (1 << 0)
#define ITEM_WEAR_FINGER (1 << 1)
#define ITEM_WEAR_NECK (1 << 2)
#define ITEM_WEAR_BODY (1 << 3)
#define ITEM_WEAR_HEAD (1 << 4)
#define ITEM_WEAR_LEGS (1 << 5)
#define ITEM_WEAR_FEET (1 << 6)
#define ITEM_WEAR_HANDS (1 << 7)
#define ITEM_WEAR_ARMS (1 << 8)
#define ITEM_WEAR_SHIELD (1 << 9)
#define ITEM_WEAR_ABOUT (1 << 10)
#define ITEM_WEAR_WAISTE (1 << 11)
#define ITEM_WEAR_WRIST (1 << 12)
#define ITEM_WIELD (1 << 13)
#define ITEM_HOLD (1 << 14)
#define ITEM_THROW (1 << 15)
/* bit 16 unused - was ITEM_LIGHT_SOURCE */
#define ITEM_WEAR_EAR (1 << 17)
#define ITEM_WEAR_FACE (1 << 18)
#define ITEM_WORN_AS_RADIO (1 << 19)

/*
 * Bitvector for 'extra_flags' (item properties)
 */
#define ITEM_GLOW (1 << 0)
#define ITEM_HUM (1 << 1)
#define ITEM_LEVEL15 (1 << 2)
#define ITEM_LEVEL25 (1 << 3)
#define ITEM_LEVEL35 (1 << 4)
#define ITEM_INVISIBLE (1 << 5)
#define ITEM_MAGIC (1 << 6)
#define ITEM_NODROP (1 << 7)
#define ITEM_BLESS (1 << 8)
#define ITEM_ANTI_GOOD (1 << 9)     /* not usable by good people    */
#define ITEM_ANTI_EVIL (1 << 10)    /* not usable by evil people    */
#define ITEM_ANTI_NEUTRAL (1 << 11) /* not usable by neutral people */
#define ITEM_ANTI_CLERIC (1 << 12)
#define ITEM_ANTI_MAGE (1 << 13)
#define ITEM_ANTI_THIEF (1 << 14)
#define ITEM_ANTI_FIGHTER (1 << 15)
#define ITEM_BRITTLE (1 << 16) /* weapons/armor that break after 1 hit */
#define ITEM_LEVEL10 (1 << 17) /* can't be worn by levels < 10 */
#define ITEM_LEVEL20 (1 << 18) /* can't be worn by levels < 20 */
#define ITEM_LEVEL30 (1 << 19) /* can't be worn by levels < 30 */
#define ITEM_ANTI_ANTI (1 << 20)
#define ITEM_ANTI_PALA (1 << 21)
#define ITEM_ANTI_RANGER (1 << 22)
#define ITEM_ANTI_MONK (1 << 23)
#define ITEM_LEVEL40 (1 << 24)
#define ITEM_HOLDING (1 << 25)

/*
 * Liquid types (for drink containers)
 */
#define LIQ_WATER 0
#define LIQ_BEER 1
#define LIQ_WINE 2
#define LIQ_ALE 3
#define LIQ_DARKALE 4
#define LIQ_WHISKY 5
#define LIQ_LEMONADE 6
#define LIQ_FIREBRT 7
#define LIQ_LOCALSPC 8
#define LIQ_SLIME 9
#define LIQ_MILK 10
#define LIQ_TEA 11
#define LIQ_COFFE 12
#define LIQ_BLOOD 13
#define LIQ_SALTWATER 14
#define LIQ_COKE 15

/*
 * Special drink flags
 */
#define DRINK_POISON (1 << 0)
#define DRINK_PERM (1 << 1)

/*
 * Container flags (for value[1] of containers)
 */
#define CONT_CLOSEABLE (1 << 0)
#define CONT_PICKPROOF (1 << 1)
#define CONT_CLOSED (1 << 2)
#define CONT_LOCKED (1 << 3)

/*
 * Maximum object affects
 * Used in OBJ_FILE_ELEM - DO NOT CHANGE
 */
#define MAX_OBJ_AFFECT 5
