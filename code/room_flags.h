#pragma once

/*
 * Special room values
 */
#define NOWHERE (-1)   /* nil reference for room-database      */
#define AUTO_RENT (-2) /* other special room, for auto-renting */

/*
 * Bitvector for 'room_flags'
 */
#define DARK (1 << 0)
#define DEATH (1 << 1)
#define NO_MOB (1 << 2)
#define INDOORS (1 << 3)
#define PEACEFUL (1 << 4) /* no fighting */
#define NOSTEAL (1 << 5)  /* no thieving */
#define NO_SUM (1 << 6)   /* no summoning */
#define NO_MAGIC (1 << 7)
#define TUNNEL (1 << 8)
#define PRIVATE (1 << 9)
#define SILENCE (1 << 10)
#define NO_ORDER (1 << 11)
#define ANARCHY (1 << 12)
#define HAVE_TO_WALK (1 << 13)
#define ARENA (1 << 14)
#define NO_HEAL (1 << 15)
#define HOSPITAL (1 << 16)

/*
 * Direction constants
 */
#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3
#define UP 4
#define DOWN 5

/*
 * Exit flags
 */
#define EX_ISDOOR (1 << 0)
#define EX_CLOSED (1 << 1)
#define EX_LOCKED (1 << 2)
#define EX_SECRET (1 << 3)
#define EX_RSLOCKED (1 << 4)
#define EX_PICKPROOF (1 << 5)

/*
 * Sector types (terrain/movement)
 */
#define SECT_INSIDE 0
#define SECT_CITY 1
#define SECT_FIELD 2
#define SECT_FOREST 3
#define SECT_HILLS 4
#define SECT_MOUNTAIN 5
#define SECT_WATER_SWIM 6
#define SECT_WATER_NOSWIM 7
#define SECT_AIR 8
#define SECT_UNDERWATER 9
#define SECT_DESERT 10
