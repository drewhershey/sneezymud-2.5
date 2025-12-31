#pragma once

/*
 * Pulse/Tick timing constants
 */
#define PULSE_COMMAND 0
#define PULSE_TICK 1
#define PULSE_RIVER 15
#define PULSE_TELEPORT 10
#define PULSE_ZONE 240
#define PULSE_MOBILE 60
#define PULSE_VIOLENCE 12
#define WAIT_SEC 4
#define WAIT_ROUND 4

/*
 * Maximum limits
 */
#define MAX_STAT 5
#define MAX_BAN_HOSTS 15
#define MAX_ROOMS 5000
#define MAX_STRING_LENGTH 4096
#define MAX_INPUT_LENGTH 160
#define MAX_MESSAGES 60
#define MAX_ITEMS 153

/*
 * Time constants - real time
 */
#define SECS_PER_REAL_MIN 60
#define SECS_PER_REAL_HOUR (60 * SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY (24 * SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR (365 * SECS_PER_REAL_DAY)

/*
 * Time constants - MUD time
 */
#define SECS_PER_MUD_HOUR 75
#define SECS_PER_MUD_DAY (24 * SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH (35 * SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR (17 * SECS_PER_MUD_MONTH)

/*
 * Level constants
 */
#define ABS_MAX_LVL 70
#define MAX_MORT 50
#define LOW_IMMORTAL 51
#define IMMORTAL 51
#define CREATOR 52
#define SAINT 53
#define DEMIGOD 54
#define LESSER_GOD 55
#define GOD 56
#define GREATER_GOD 57
#define SILLYLORD 58
#define IMPLEMENTOR 59
#define BRUTIUS 60
#define MAX_IMMORT 60

/*
 * Damage types
 */
#define FIRE_DAMAGE 1
#define COLD_DAMAGE 2
#define ELEC_DAMAGE 3
#define BLOW_DAMAGE 4
#define ACID_DAMAGE 5

/*
 * Message types (for combat messages)
 */
#define MESS_ATTACKER 1
#define MESS_VICTIM 2
#define MESS_ROOM 3

/*
 * Poof flags (for immortal entrances/exits)
 */
#define BIT_POOF_IN 1
#define BIT_POOF_OUT 2

/*
 * Tailoring/feature flags
 */
#define LIMITED_ITEMS 1
#define SITELOCK 0

/*
 * Efficiency settings
 */
#define MIN_GLOB_TRACK_LEV 31 /* minimum level for global track */

/*
 * Newbie authorization
 */
#define NEWBIE_REQUEST 1
#define NEWBIE_START 100
#define NEWBIE_AXE 0
#define NEWBIE_CHANCES 3

/*
 * Limited item settings
 */
#define LIM_ITEM_COST_MIN 199999 /* minimum rent cost of a limited item */

/*
 * Distributed monster timing
 */
#define TICK_WRAP_COUNT 3 /* PULSE_MOBILE / PULSE_TELEPORT */
#define PLR_TICK_WRAP 24  /* should be a divisor of 24 (hours) */

/*
 * Object timer special value
 */
#define OBJ_NOTIMER (-7000000)

/*
 * Special attack type for object-caused damage
 */
#define OBJECT_HITTING (-1)

/*
 * Weather/sunlight states
 */
#define SUN_DARK 0
#define SUN_RISE 1
#define SUN_LIGHT 2
#define SUN_SET 3

#define SKY_CLOUDLESS 0
#define SKY_CLOUDY 1
#define SKY_RAINING 2
#define SKY_LIGHTNING 3

/*
 * Connection states (modes of connectedness)
 */
#define CON_PLYNG 0
#define CON_NME 1
#define CON_NMECNF 2
#define CON_PWDNRM 3
#define CON_PWDGET 4
#define CON_PWDCNF 5
#define CON_QSEX 6
#define CON_RMOTD 7
#define CON_SLCT 8
#define CON_EXDSCR 9
#define CON_QCLASS 10
#define CON_LDEAD 11
#define CON_PWDNEW 12
#define CON_PWDNCNF 13
#define CON_WIZLOCK 14
#define CON_QRACE 15
#define CON_RACPAR 16
#define CON_UNUSED 17
#define CON_CITY_CHOICE 18
#define CON_STAT_LIST 19
#define CON_VT_ANSI 20
#define CON_ANSI_NME 21
#define CON_VT_NME 22
#define CON_SCREEN_SIZE 23

static_assert(LOW_IMMORTAL > MAX_MORT,
  "LOW_IMMORTAL must be greater than MAX_MORT");
static_assert(MAX_IMMORT <= ABS_MAX_LVL,
  "MAX_IMMORT must not exceed ABS_MAX_LVL");
static_assert(IMPLEMENTOR < ABS_MAX_LVL,
  "IMPLEMENTOR level must be less than ABS_MAX_LVL");
