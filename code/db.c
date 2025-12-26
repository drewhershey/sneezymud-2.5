/**************************************************************************
 *  file: db.c , Database module.                          Part of DIKUMUD *
 *  Usage: Loading/Saving chars, booting world, resetting etc.             *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ***************************************************************************/

#include "db.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <time.h>

#include "board.h"
#include "comm.h"
#include "compat_types.h"
#include "constants.h"
#include "handler.h"
#include "hash.h"
#include "interpreter.h"
#include "limits.h"
#include "mail.h"
#include "multiclass.h"
#include "opinion.h"
#include "race.h"
#include "spells.h"
#include "structs.h"
#include "utils.h"

#define NEW_ZONE_SYSTEM

/**************************************************************************
 *  declarations of most of the 'global' variables                         *
 ************************************************************************ */

struct reset_q_type reset_q;

int top_of_world = 0; /* ref to the top element of world */

#if defined(HASH) && HASH
struct hash_header room_db;
#else
struct room_data* room_db[WORLD_SIZE];
#endif

struct obj_data* object_list = NULL;     /* the global linked list of obj's */
struct char_data* character_list = NULL; /* global l-list of chars          */

struct zone_data* zone_table; /* table of reset data             */
int top_of_zone_table = 0;

struct player_index_element* player_table = 0; /* index to player file   */
int top_of_p_table = 0; /* ref to top of table             */
int top_of_p_file = 0;
long total_bc = 0;
long room_count = 0;
long mob_count = 0;
int obj_count = 0;
long total_mbc = 0;
int no_mail = 0;
long total_obc = 0;

// distributed monster stuff
int mob_tick_count = 0;

char motd[MAX_STRING_LENGTH];
char ansi[MAX_STRING_LENGTH];

FILE* mob_f = NULL;  // file containing mob prototypes
FILE* obj_f = NULL;  // obj prototypes

struct index_data* mob_index; /* index table for mobile file     */
struct index_data* obj_index; /* index table for object file     */

int top_of_mobt = 0; /* top of mobile index table       */
int top_of_objt = 0; /* top of object index table       */

struct time_info_data time_info;  /* the infomation about the time   */
struct weather_data weather_info; /* the infomation about the weather */

static void print_limited_items(void) {
  int i;
  char buf[200];

  for (i = 0; i <= top_of_objt; i++) {
    if (obj_index[i].number > 0) {
      sprintf(buf, "item> %d [%d]", obj_index[i].virtual, obj_index[i].number);
      vlog(buf);
    }
  }
}

/*************************************************************************
 *  routines for booting the system                                       *
 *********************************************************************** */

/* body of the booting system */
void boot_db(void) {
  int i;

  vlog("Boot db -- BEGIN.");

  vlog("Resetting the game time:");
  reset_time();

  vlog("Reading atlas, and motd.");
  file_to_string(ANSI_FILE, ansi);
  file_to_string(MOTD_FILE, motd);

  vlog("Opening mobile, object and help files.");
  if (!(mob_f = fopen(MOB_FILE, "r"))) {
    perror("boot");
    exit(0);
  }

  if (!(obj_f = fopen(OBJ_FILE, "r"))) {
    perror("boot");
    exit(0);
  }

  vlog("Loading zone table.");
  boot_zones();

  vlog("Loading rooms.");
  boot_world();

  vlog("Generating index tables for mobile and object files.");
  mob_index = generate_indices(mob_f, &top_of_mobt);
  obj_index = generate_indices(obj_f, &top_of_objt);

  vlog("Renumbering zone table.");
  renum_zone_table();

  vlog("Generating player index.");
  build_player_index();

  vlog("Loading fight messages.");
  load_messages();

  vlog("Loading social messages.");
  boot_social_messages();

  vlog("Loading pose messages.");
  boot_pose_messages();

  vlog("Assigning function pointers:");
  if (!no_specials) {
    vlog("   Mobiles.");
    assign_mobiles();
    vlog("   Objects.");
    assign_objects();
    vlog("   Room.");
    assign_rooms();
  }

  vlog("   Commands.");
  assign_command_pointers();
  vlog("   Spells.");
  assign_spell_pointers();

  vlog("Updating characters with saved items:");
  update_obj_file();

  vlog(" Booting mail system.");
  if (!scan_file()) {
    vlog("   Mail system error -- mail system disabled!");
    no_mail = 1;
  }

#if LIMITED_ITEMS
  print_limited_items();
#endif

  for (i = 0; i <= top_of_zone_table; i++) {
    char* s;
    int d;
    int e;
    s = zone_table[i].name;
    d = (i ? (zone_table[i - 1].top + 1) : 0);
    e = zone_table[i].top;
    fprintf(stderr, "Performing boot-time reset of %s (rooms %d-%d).\n",
#if 1
      s, d, e);
#else
      zone_table[i].name, (i ? (zone_table[i - 1].top + 1) : 0),
      zone_table[i].top);
#endif
    reset_zone(i);
  }

  reset_q.head = reset_q.tail = 0;

  vlog("Boot db -- DONE.");
}

/* reset the time in the game from file */
void reset_time(void) {
  char buf[80];
  long beginning_of_time = 650336715;

  struct time_info_data mud_time_passed(time_t t2, time_t t1);

  time_info = mud_time_passed(time(0), beginning_of_time);

  moontype = time_info.day;

  switch (time_info.hours) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4: {
      weather_info.sunlight = SUN_DARK;
      break;
    }
    case 5:
    case 6: {
      weather_info.sunlight = SUN_RISE;
      break;
    }
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18: {
      weather_info.sunlight = SUN_LIGHT;
      break;
    }
    case 19:
    case 20: {
      weather_info.sunlight = SUN_SET;
      break;
    }
    case 21:
    case 22:
    case 23:
    default: {
      weather_info.sunlight = SUN_DARK;
      break;
    }
  }

  sprintf(buf, "   Current Gametime: %dH %dD %dM %dY.", time_info.hours,
    time_info.day, time_info.month, time_info.year);
  vlog(buf);

  weather_info.pressure = 960;
  if ((time_info.month >= 7) && (time_info.month <= 12)) {
    weather_info.pressure += dice(1, 50);
  } else {
    weather_info.pressure += dice(1, 80);
  }

  weather_info.change = 0;

  if (weather_info.pressure <= 980) {
    if ((time_info.month >= 3) && (time_info.month <= 14)) {
      weather_info.sky = SKY_LIGHTNING;
    } else {
      weather_info.sky = SKY_LIGHTNING;
    }
  } else if (weather_info.pressure <= 1000) {
    if ((time_info.month >= 3) && (time_info.month <= 14)) {
      weather_info.sky = SKY_RAINING;
    } else {
      weather_info.sky = SKY_RAINING;
    }
  } else if (weather_info.pressure <= 1020) {
    weather_info.sky = SKY_CLOUDY;
  } else {
    weather_info.sky = SKY_CLOUDLESS;
  }
}

/* update the time file */
void update_time(void) {
  FILE* f1;
  long current_time;

  return;

  if (!(f1 = fopen(TIME_FILE, "w"))) {
    perror("update time");
    exit(0);
  }

  current_time = time(0);
  vlog("Time update.");

  fprintf(f1, "#\n");

  fprintf(f1, "%d\n", current_time);
  fprintf(f1, "%d\n", time_info.hours);
  fprintf(f1, "%d\n", time_info.day);
  fprintf(f1, "%d\n", time_info.month);
  fprintf(f1, "%d\n", time_info.year);

  fclose(f1);
}

/* generate index table for the player file */
void build_player_index(void) {
  int nr = -1;
  int i;
  int pc = 0;
  struct char_file_u dummy;
  FILE* fl;

  if (!(fl = fopen(PLAYER_FILE, "rb+"))) {
    perror("build player index");
    exit(0);
  }

  for (; !feof(fl);) {
    fread(&dummy, sizeof(struct char_file_u), 1, fl);
    if (!feof(fl)) /* new record */ {
      /* Create new entry in the list */
      if (nr == -1) {
        CREATE(player_table, struct player_index_element, 3000);
        nr = 0;
      } else {
        if (nr >= pc) {
          if (!(player_table =
                  (struct player_index_element*)realloc(player_table,
                    (++nr + 50) * sizeof(struct player_index_element)))) {
            perror("generate index");
            exit(0);
          }
          pc += 50;
        } else {
          nr++;
        }
      }

      player_table[nr].nr = nr;

      /* Find actual length of name, respecting the 20-byte array limit */
      int name_len = 0;
      while (name_len < 20 && dummy.name[name_len] != '\0') {
        name_len++;
      }

      CREATE(player_table[nr].name, char, (unsigned long)(name_len + 1));
      for (i = 0; i < name_len; i++) {
        player_table[nr].name[i] = LOWER(dummy.name[i]);
      }
      player_table[nr].name[name_len] = '\0';
    }
  }

  fclose(fl);

  top_of_p_table = nr;

  top_of_p_file = top_of_p_table;
}

/* generate index table for object or monster file */
struct index_data* generate_indices(FILE* fl, int* top) {
  int i = 0;
  long bc = 1500;
  struct index_data* index;
  char buf[82];

  rewind(fl);

  for (;;) {
    if (fgets(buf, sizeof(buf), fl)) {
      if (*buf == '#') {
        if (!i) { /* first cell */
          CREATE(index, struct index_data, bc);
        } else if (i >= bc) {
          if (!(index = (struct index_data*)realloc(index,
                  (i + 50) * sizeof(struct index_data)))) {
            perror("load indices");
            exit(0);
          }
          bc += 50;
        }
        sscanf(buf, "#%d", &index[i].virtual);
        index[i].pos = ftell(fl);
        index[i].number = 0;
        index[i].func.mob_f = NULL;
        index[i].func.obj_f = NULL;
        index[i].func.room_f = NULL;
        index[i].name = (index[i].virtual < 99999) ? fread_string(fl) : "omega";
        i++;
      } else {
        if (*buf == '$') { /* EOF */
          break;
        }
      }
    } else {
      fprintf(stderr, "generate indices");
      exit(0);
    }
  }
  *top = i - 2;
  return (index);
}

void cleanout_room(struct room_data* rp) {
  int i;
  struct extra_descr_data* exptr;
  struct extra_descr_data* nptr;

  free(rp->name);
  free(rp->description);
  for (i = 0; i < 6; i++) {
    if (rp->dir_option[i]) {
      free(rp->dir_option[i]->general_description);
      free(rp->dir_option[i]->keyword);
      free(rp->dir_option[i]);
      rp->dir_option[i] = NULL;
    }
  }

  for (exptr = rp->ex_description; exptr; exptr = nptr) {
    nptr = exptr->next;
    free(exptr->keyword);
    free(exptr->description);
    free(exptr);
  }
}

// Allocates space for a single direction data entry for a room, then reads in
// the data from the .wld file
static void setup_dir(FILE* fl, Room* rp, int dir) {
  assert(rp);

  CREATE(rp->dir_option[dir], struct room_direction_data, 1);
  struct room_direction_data* dd = rp->dir_option[dir];

  dd->general_description = fread_string(fl);
  dd->keyword = fread_string(fl);

  fscanf(fl, " %hd ", &dd->exit_info);
  switch (dd->exit_info) {
    case 4:
      dd->exit_info = EX_ISDOOR | EX_SECRET | EX_PICKPROOF;
      break;
    case 3:
      dd->exit_info = EX_ISDOOR | EX_SECRET;
      break;
    case 2:
      dd->exit_info = EX_ISDOOR | EX_PICKPROOF;
      break;
    case 1:
      dd->exit_info = EX_ISDOOR;
      break;
    case 0:
      break;
    default:
      dd->exit_info = 0;
      break;
  }

  fscanf(fl, " %d ", &dd->key);
  fscanf(fl, " %d ", &dd->to_room);
}

// Reads in the data for a single, already-allocated room from the .wld file.
// Expects the file to already be positioned at the start of the room data,
// right after the room number.
void load_one_room(FILE* fl, Room* rp) {
  assert(rp && rp->number >= 0 &&
         "Room must already be allocated before calling load_one_room");

  rp->name = fread_string(fl);
  rp->description = fread_string(fl);

  if (top_of_zone_table >= 0) {
    fscanf(fl, " %*d ");

    /* OBS: Assumes ordering of input rooms */

    int zone = 0;

    for (; zone >= 0 && zone <= top_of_zone_table; ++zone) {
      if (rp->number <= zone_table[zone].top) {
        break;
      }
    }

    if (zone > top_of_zone_table) {
      fprintf(stderr, "Room %d is outside of any zone.\n", rp->number);
      exit(0);
    }
    rp->zone = (short)zone;
  }

  fscanf(fl, " %ld ", &rp->room_flags);
  fscanf(fl, " %d ", &rp->sector_type);

  if (rp->sector_type == -1) {
    fscanf(fl, " %d ", &rp->tele_time);
    fscanf(fl, " %d ", &rp->tele_targ);
    fscanf(fl, " %hhd ", &rp->tele_look);
    fscanf(fl, " %d ", &rp->sector_type);
  }

  /* river */
  if (rp->sector_type == 7) {
    /* read direction and rate of flow */
    fscanf(fl, " %d ", &rp->river_speed);
    fscanf(fl, " %d ", &rp->river_dir);
  }

  /* read in mobile limit on tunnel */
  if (IS_SET(rp->room_flags, TUNNEL)) {
    fscanf(fl, " %hhu ", &rp->moblim);
  }

  char chk[MAX_STRING_LENGTH];
  int result;
  while ((result = fscanf(fl, " %4095s \n", &chk[0])) != EOF) {
    if (result != 1) {
      vlogf("Error reading room line at file position %ld", ftell(fl));
      fclose(fl);
      exit(0);
    }

    switch (*chk) {
      case 'D':
        setup_dir(fl, rp, atoi(chk + 1));
        continue;

      // extra description field
      case 'E': {
        struct extra_descr_data* new_descr;
        CREATE(new_descr, struct extra_descr_data, 1);

        new_descr->keyword = fread_string(fl);

        if (!new_descr->keyword || !*new_descr->keyword) {
          fprintf(stderr, "No keyword in room %d\n", rp->number);
        }

        new_descr->description = fread_string(fl);

        if (!new_descr->description || !*new_descr->description) {
          fprintf(stderr, "No desc in room %d\n", rp->number);
        }

        new_descr->next = rp->ex_description;
        rp->ex_description = new_descr;
        continue;
      }

      case 'S':
        /* end of current room */
        ++room_count;
        return;

      default: {
        vlogf("Found unknown secondary command '%s' in room load of #%d", chk,
          rp->number);
        break;
      }
    }
  }
}

/* load the rooms */
void boot_world(void) {
#if defined(HASH) && HASH
  init_hash_table(&room_db, sizeof(struct room_data), 2048);
#else
  memset(room_db, 0, sizeof(struct room_data*) * WORLD_SIZE);
#endif

  assert(!character_list && !object_list);

  FILE* fl = fopen(WORLD_FILE, "r");
  if (!fl) {
    perror("fopen");
    vlog("World file not found");
    exit(0);
  }

  int virtual_nr = -1;
  int result;
  while ((result = fscanf(fl, " #%d\n", &virtual_nr)) != EOF) {
    if (result != 1) {
      vlogf("Error reading room number at file position %ld", ftell(fl));
      fclose(fl);
      exit(0);
    }

    load_one_room(fl, allocate_room(virtual_nr));
  }

  fclose(fl);
}

// Find an already-existing room from the in-memory database with the given
// room_number and return a pointer. If one doesn't exist, allocate a new room
// and add it to the database, then return a pointer.
Room* allocate_room(int room_number) {
  if (room_number > top_of_world) {
    top_of_world = room_number;
  }

#if defined(HASH) && HASH
  return hash_find_or_create(&room_db, room_number);
#else
  Room* room = room_find(room_db, room_number);

  if (room) {
    return room;
  }

  room = NULL;
  CREATE(room, struct room_data, 1);
  room->number = (short)room_number;
  room_db[room_number] = room;

  return room;
#endif
}

#define LOG_ZONE_ERROR(ch, type, zone, cmd)                          \
  {                                                                  \
    sprintf(buf, "error in zone %s cmd %d (%c) resolving %s number", \
      zone_table[zone].name, cmd, ch, type);                         \
    vlog(buf);                                                       \
  }

void renum_zone_table(void) {
  int zone;
  int comm;
  struct reset_com* cmd;
  char buf[256];

  for (zone = 0; zone <= top_of_zone_table; zone++) {
    for (comm = 0; zone_table[zone].cmd[comm].command != 'S'; comm++) {
      switch ((cmd = zone_table[zone].cmd + comm)->command) {
        case 'M':
          cmd->arg1 = real_mobile(cmd->arg1);
          if (cmd->arg1 < 0)
            LOG_ZONE_ERROR('M', "mobile", zone, comm);
          if (cmd->arg3 < 0)
            LOG_ZONE_ERROR('M', "room", zone, comm);
          break;
        case 'C':
          cmd->arg1 = real_mobile(cmd->arg1);
          if (cmd->arg1 < 0)
            LOG_ZONE_ERROR('C', "mobile", zone, comm);
          /*cmd->arg3 = real_room(cmd->arg3);*/
          if (cmd->arg3 < 0)
            LOG_ZONE_ERROR('C', "room", zone, comm);
          break;
        case 'O':
          cmd->arg1 = real_object(cmd->arg1);
          if (cmd->arg1 < 0)
            LOG_ZONE_ERROR('O', "object", zone, comm);
          if (cmd->arg3 != NOWHERE) {
            /*cmd->arg3 = real_room(cmd->arg3);*/
            if (cmd->arg3 < 0)
              LOG_ZONE_ERROR('O', "room", zone, comm);
          }
          break;
        case 'G':
          cmd->arg1 = real_object(cmd->arg1);
          if (cmd->arg1 < 0)
            LOG_ZONE_ERROR('G', "object", zone, comm);
          break;
        case 'E':
          cmd->arg1 = real_object(cmd->arg1);
          if (cmd->arg1 < 0)
            LOG_ZONE_ERROR('E', "object", zone, comm);
          break;
        case 'P':
          cmd->arg1 = real_object(cmd->arg1);
          if (cmd->arg1 < 0)
            LOG_ZONE_ERROR('P', "object", zone, comm);
          cmd->arg3 = real_object(cmd->arg3);
          if (cmd->arg3 < 0)
            LOG_ZONE_ERROR('P', "object", zone, comm);
          break;
        case 'D':
          /*cmd->arg1 = real_room(cmd->arg1);*/
          if (cmd->arg1 < 0)
            LOG_ZONE_ERROR('D', "room", zone, comm);
          break;
      }
    }
  }
}

/* load the zone table and command tables */
void boot_zones(void) {
  FILE* fl;
  int zon = 0;
  int cmd_no = 0;
  int expand;
  int tmp;
  int bc = 100;
  int cc = 20;
  char* check;
  char buf[81];

  if (!(fl = fopen(ZONE_FILE, "r"))) {
    perror("boot_zones");
    exit(0);
  }

  for (;;) {
    fscanf(fl, " #%*d\n");
    check = fread_string(fl);

    if (*check == '$') {
      break; /* end of file */
    }

    /* alloc a new zone */

    if (!zon) {
      CREATE(zone_table, struct zone_data, bc);
    } else if (zon >= bc) {
      if (!(zone_table = (struct zone_data*)realloc(zone_table,
              (zon + 10) * sizeof(struct zone_data)))) {
        perror("boot_zones realloc");
        exit(0);
      }
      bc += 10;
    }
    zone_table[zon].name = check;
    fscanf(fl, " %d ", &zone_table[zon].top);
    fscanf(fl, " %d ", &zone_table[zon].lifespan);
    fscanf(fl, " %d ", &zone_table[zon].reset_mode);

    /* read the command table */

    cmd_no = 0;

    for (expand = 1;;) {
      if (expand) {
        if (!cmd_no) {
          CREATE(zone_table[zon].cmd, struct reset_com, cc);
        } else if (cmd_no >= cc) {
          if (!(zone_table[zon].cmd =
                  (struct reset_com*)realloc(zone_table[zon].cmd,
                    (cmd_no + 5) * sizeof(struct reset_com)))) {
            perror("reset command load");
            exit(0);
          }
          cc += 5;
        }
      }

      expand = 1;

      fscanf(fl, " "); /* skip blanks */
      fscanf(fl, "%c", &zone_table[zon].cmd[cmd_no].command);

      if (zone_table[zon].cmd[cmd_no].command == 'S') {
        break;
      }

      if (zone_table[zon].cmd[cmd_no].command == '*') {
        expand = 0;
        fgets(buf, 80, fl); /* skip command */
        continue;
      }

      fscanf(fl, " %d %d %d", &tmp, &zone_table[zon].cmd[cmd_no].arg1,
        &zone_table[zon].cmd[cmd_no].arg2);

      zone_table[zon].cmd[cmd_no].if_flag = tmp;

      if (zone_table[zon].cmd[cmd_no].command == 'M' ||
          zone_table[zon].cmd[cmd_no].command == 'O' ||
          zone_table[zon].cmd[cmd_no].command == 'C' ||
          zone_table[zon].cmd[cmd_no].command == 'E' ||
          zone_table[zon].cmd[cmd_no].command == 'P' ||
          zone_table[zon].cmd[cmd_no].command == 'D') {
        fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);
      }

      fgets(buf, 80, fl); /* read comment */

#if LIMITED_ITEMS
      /*
      **  check for mininum limits on items  (adjust them up or down)
      **  Clearly, this is not implemented yet.
      */
#endif
      cmd_no++;
    }
    zon++;
  }
  top_of_zone_table = --zon;
  free(check);
  fclose(fl);
}

/*************************************************************************
 *  procedures for resetting, both play-time and boot-time	 	 *
 *********************************************************************** */

static void set_racial_stuff(struct char_data* mob) {
  switch (GET_RACE(mob)) {
    case RACE_BIRD:
      SET_BIT(mob->specials.affected_by, AFF_FLYING);
      break;
    case RACE_FISH:
      SET_BIT(mob->specials.affected_by, AFF_WATERBREATH);
      break;
    case RACE_DROW:
    case RACE_DWARF:
    case RACE_GNOME:
    case RACE_MFLAYER:
    case RACE_TROLL:
    case RACE_ORC:
    case RACE_GOBLIN:
    case RACE_HOBBIT:
      SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
      break;
    case RACE_INSECT:
    case RACE_ARACHNID:
      if (IS_PC(mob)) {
        GET_STR(mob) = 18;
        GET_ADD(mob) = 100;
      }
      break;
    case RACE_LYCANTH:
      SET_BIT(mob->M_immune, IMM_NONMAG);
      break;
    case RACE_PREDATOR:
      if (mob->skills) {
        mob->skills[SKILL_HUNT].learned = 100;
      }
      break;

    default:
      break;
  }
}

static char is_valid_position(signed char pos) {
  return pos >= POSITION_DEAD && pos <= POSITION_STANDING;
}

/* read a mobile from MOB_FILE */
struct char_data* read_mobile(int nr, int type) {
  int original_nr = nr;

  if (type == VIRTUAL) {
    nr = real_mobile(nr);

    if (nr < 0) {
      char buf[MAX_STRING_LENGTH];
      sprintf(buf, "Mobile (V) %d does not exist in database.", original_nr);
      return NULL;
    }
  }

  fseek(mob_f, mob_index[nr].pos, 0);

  Mob* mob = NULL;
  CREATE(mob, struct char_data, 1);
  clear_char(mob);

  /***** String data *** */

  mob->player.name = fread_string(mob_f);
  mob->player.short_descr = fread_string(mob_f);
  mob->player.long_descr = fread_string(mob_f);
  mob->player.description = fread_string(mob_f);
  mob->player.title = NULL;

  /* *** Numeric data *** */

  mob->mult_att = 1.0F;

  fscanf(mob_f, "%lu ", &mob->specials.act);
  SET_BIT(mob->specials.act, ACT_ISNPC);

  fscanf(mob_f, " %ld ", &mob->specials.affected_by);
  fscanf(mob_f, " %d ", &mob->specials.alignment);

  mob->player.class = CLASS_WARRIOR;

  char letter = '\0';
  fscanf(mob_f, " %c ", &letter);

  if (letter == 'S') {
    fscanf(mob_f, "\n");

    fscanf(mob_f, " %hhd ", &mob->player.level[2]);

    if (GET_LEVEL(mob, WARRIOR_LEVEL_IND) < 50) {
      mob->abilities.str = 15;
      mob->abilities.intel = 15;
      mob->abilities.wis = 15;
      mob->abilities.dex = 15;
      mob->abilities.con = 15;
    } else {
      mob->abilities.str = 18;
      mob->abilities.str_add = 100;
      mob->abilities.wis = 18;
      mob->abilities.con = 18;
      mob->abilities.dex = 18;
      mob->abilities.intel = 18;
    }

    fscanf(mob_f, " %hhd ", &mob->points.hitroll);

    fscanf(mob_f, " %hd ", &mob->points.armor);
    mob->points.armor *= 10;

    short hd_num = 0;
    short hd_type = 0;
    short hd_bonus = 0;

    fscanf(mob_f, " %hdd%hd+%hd ", &hd_num, &hd_type, &hd_bonus);
    mob->points.max_hit = DICE(hd_num, hd_type) + hd_bonus;
    mob->points.hit = mob->points.max_hit;

    signed char d_num = 0;
    signed char d_type = 0;
    signed char bonus = 0;

    fscanf(mob_f, " %hhdd%hhd+%hhd \n", &d_num, &d_type, &bonus);
    mob->points.damroll = bonus;
    mob->specials.damnodice = d_num;
    mob->specials.damsizedice = d_type;

    mob->points.mana = 10;
    mob->points.max_mana = 10;

    mob->points.move = 50;
    mob->points.max_move = 50;

    int chk = 0;
    fscanf(mob_f, " %d ", &chk);

    if (chk == -1) {
      fscanf(mob_f, " %d ", &mob->points.gold);
      fscanf(mob_f, " %d ", &mob->points.exp);
      fscanf(mob_f, " %hd \n", &mob->race);
    } else {
      mob->points.gold = chk;
      fscanf(mob_f, " %d \n", &mob->points.exp);
    }

    signed char position = 0;
    signed char default_pos = 0;

    if (fscanf(mob_f, " %hhd ", &position) != 1 ||
        fscanf(mob_f, " %hhd ", &default_pos) != 1) {
      vlogf("Error reading positions for mob %d", mob_index[nr].virtual);
      // Set safe defaults
      position = POSITION_STANDING;
      default_pos = POSITION_STANDING;
    }

    if (!is_valid_position(position) || !is_valid_position(default_pos)) {
      vlogf("Invalid positions %d/%d for mob %d - correcting to STANDING",
        position, default_pos, mob_index[nr].virtual);
      position = POSITION_STANDING;
      default_pos = POSITION_STANDING;
    }

    mob->specials.position = position;
    mob->specials.default_pos = default_pos;

    signed char sex = 0;
    fscanf(mob_f, " %hhd ", &sex);

    if (sex < 3) {
      mob->player.sex = sex;
      mob->immune = 0;
      mob->M_immune = 0;
      mob->susc = 0;
    } else if (sex < 6) {
      mob->player.sex = (signed char)(sex - 3);
      fscanf(mob_f, " %u ", &mob->immune);
      fscanf(mob_f, " %u ", &mob->M_immune);
      fscanf(mob_f, " %u ", &mob->susc);
    } else {
      mob->player.sex = 0;
      mob->immune = 0;
      mob->M_immune = 0;
      mob->susc = 0;
    }

    fscanf(mob_f, "\n");

    mob->player.class = 0;

    mob->player.time.birth = time(0);
    mob->player.time.played = 0;
    mob->player.time.logon = time(0);
    mob->player.weight = 200;
    mob->player.height = 198;

    for (int i = 0; i < 3; i++) {
      GET_COND(mob, i) = -1;
    }

    for (int i = 0; i < 5; i++) {
      mob->specials.apply_saving_throw[i] =
        (short)(20 - (GET_LEVEL(mob, WARRIOR_LEVEL_IND) / 2));
    }
  } else if (letter == 'A' || letter == 'N' || letter == 'B' || letter == 'L') {
    if (letter == 'A' || letter == 'B' || letter == 'L') {
      fscanf(mob_f, " %f ", &mob->mult_att);
    }

    fscanf(mob_f, "\n");

    fscanf(mob_f, " %hhd ", &mob->player.level[2]);

    if (GET_LEVEL(mob, WARRIOR_LEVEL_IND) < 50) {
      mob->abilities.con = 15;
      mob->abilities.dex = 15;
      mob->abilities.str = 15;
      mob->abilities.intel = 15;
      mob->abilities.wis = 15;
    } else {
      mob->abilities.con = 18;
      mob->abilities.dex = 18;
      mob->abilities.str = 18;
      mob->abilities.str_add = 100;
      mob->abilities.intel = 18;
      mob->abilities.wis = 18;
    }

    fscanf(mob_f, " %hhd ", &mob->points.hitroll);
    mob->points.hitroll = (signed char)(20 - mob->points.hitroll);

    fscanf(mob_f, " %hd ", &mob->points.armor);
    mob->points.armor *= 10;

    short hp_bonus = 0;
    fscanf(mob_f, " %hd ", &hp_bonus);
    mob->points.max_hit =
      (short)(DICE((int)GET_LEVEL(mob, WARRIOR_LEVEL_IND), 8) + hp_bonus);
    mob->points.hit = mob->points.max_hit;

    signed char d_num = 0;
    signed char d_type = 0;
    signed char bonus = 0;

    fscanf(mob_f, " %hhdd%hhd+%hhd \n", &d_num, &d_type, &bonus);
    mob->points.damroll = bonus;
    mob->specials.damnodice = d_num;
    mob->specials.damsizedice = d_type;

    mob->points.mana = 10;
    mob->points.max_mana = 10;

    mob->points.move = 50;
    mob->points.max_move = 50;

    int chk = 0;
    fscanf(mob_f, " %d ", &chk);

    if (chk == -1) {
      fscanf(mob_f, " %d ", &mob->points.gold);

      int exp = 0;
      fscanf(mob_f, " %d ", &exp);
      GET_EXP(mob) = DetermineExp(mob, exp) + mob->points.gold;

      fscanf(mob_f, " %hd ", &GET_RACE(mob));
    } else {
      mob->points.gold = chk;

      // this is where the new exp will come into play
      int exp = 0;
      fscanf(mob_f, " %d \n", &exp);
      GET_EXP(mob) = DetermineExp(mob, exp) + mob->points.gold;
    }

    signed char position = 0;
    signed char default_pos = 0;

    if (fscanf(mob_f, " %hhd ", &position) != 1 ||
        fscanf(mob_f, " %hhd ", &default_pos) != 1) {
      vlogf("Error reading positions for mob %d", mob_index[nr].virtual);
      // Set safe defaults
      position = POSITION_STANDING;
      default_pos = POSITION_STANDING;
    }

    if (!is_valid_position(position) || !is_valid_position(default_pos)) {
      vlogf("Invalid positions %d/%d for mob %d - correcting to STANDING",
        position, default_pos, mob_index[nr].virtual);
      position = POSITION_STANDING;
      default_pos = POSITION_STANDING;
    }

    mob->specials.position = position;
    mob->specials.default_pos = default_pos;

    fscanf(mob_f, " %hhd \n", &mob->player.sex);
    if (mob->player.sex >= 3 && mob->player.sex < 6) {
      mob->player.sex -= 3;
      fscanf(mob_f, " %u ", &mob->immune);
      fscanf(mob_f, " %u ", &mob->M_immune);
      fscanf(mob_f, " %u ", &mob->susc);
    } else if (mob->player.sex < 0 || mob->player.sex > 2) {
      mob->player.sex = 0;
    }

    // read in the sound string for a mobile
    if (letter == 'L') {
      mob->player.sounds = fread_string(mob_f);
      mob->player.distant_snds = fread_string(mob_f);
    } else {
      mob->player.sounds = NULL;
      mob->player.distant_snds = NULL;
    }

    mob->player.class = 0;
    mob->player.time.birth = time(0);
    mob->player.time.played = 0;
    mob->player.time.logon = time(0);
    mob->player.weight = 200;
    mob->player.height = 198;

    for (int i = 0; i < 3; ++i) {
      GET_COND(mob, i) = -1;
    }

    for (int i = 0; i < 5; ++i) {
      mob->specials.apply_saving_throw[i] =
        (short)(20 - (GET_LEVEL(mob, WARRIOR_LEVEL_IND) / 2));
    }
  } else { /* The old monsters are down below here */

    fscanf(mob_f, "\n");

    fscanf(mob_f, " %hhd ", &mob->abilities.str);
    fscanf(mob_f, " %hhd ", &mob->abilities.intel);
    fscanf(mob_f, " %hhd ", &mob->abilities.wis);
    fscanf(mob_f, " %hhd ", &mob->abilities.dex);
    fscanf(mob_f, " %hhd \n", &mob->abilities.con);

    short min_hp = 0;
    short max_hp = 0;
    fscanf(mob_f, " %hd ", &min_hp);
    fscanf(mob_f, " %hd ", &max_hp);

    mob->points.max_hit = NUMBER(min_hp, max_hp);
    mob->points.hit = mob->points.max_hit;

    fscanf(mob_f, " %hd ", &mob->points.armor);
    mob->points.armor *= 10;

    fscanf(mob_f, " %hd ", &mob->points.max_mana);
    mob->points.mana = mob->points.max_mana;

    fscanf(mob_f, " %hd ", &mob->points.max_move);
    mob->points.move = mob->points.max_move;

    fscanf(mob_f, " %d ", &mob->points.gold);
    fscanf(mob_f, " %d \n", &GET_EXP(mob));

    signed char position = 0;
    signed char default_pos = 0;

    if (fscanf(mob_f, " %hhd ", &position) != 1 ||
        fscanf(mob_f, " %hhd ", &default_pos) != 1) {
      vlogf("Error reading positions for mob %d", mob_index[nr].virtual);
      position = POSITION_STANDING;
      default_pos = POSITION_STANDING;
    }

    if (!is_valid_position(position) || !is_valid_position(default_pos)) {
      vlogf("Invalid positions %d/%d for mob %d - correcting to STANDING",
        position, default_pos, mob_index[nr].virtual);
      position = POSITION_STANDING;
      default_pos = POSITION_STANDING;
    }

    mob->specials.position = position;
    mob->specials.default_pos = default_pos;

    fscanf(mob_f, " %hhd ", &mob->player.sex);
    fscanf(mob_f, " %hhu ", &mob->player.class);
    fscanf(mob_f, " %hhd ", &GET_LEVEL(mob, WARRIOR_LEVEL_IND));

    int unk_1 = 0;
    fscanf(mob_f, " %d ", &unk_1);
    mob->player.time.birth = time(0);
    mob->player.time.played = 0;
    mob->player.time.logon = time(0);

    fscanf(mob_f, " %hhu ", &mob->player.weight);
    fscanf(mob_f, " %hhu \n", &mob->player.height);

    for (int i = 0; i < 3; i++) {
      fscanf(mob_f, " %hhd ", &GET_COND(mob, i));
    }
    fscanf(mob_f, " \n ");

    for (int i = 0; i < 5; i++) {
      fscanf(mob_f, " %hd ", &mob->specials.apply_saving_throw[i]);
    }

    fscanf(mob_f, " \n ");

    /* Set the damage as some standard 1d4 */
    mob->points.damroll = 0;
    mob->specials.damnodice = 1;
    mob->specials.damsizedice = 6;

    /* Calculate THAC0 as a formular of Level */
    mob->points.hitroll = MAX(1, GET_LEVEL(mob, WARRIOR_LEVEL_IND) - 3);
  }

  mob->tmpabilities = mob->abilities;

  /* Initialisering Ok */
  for (int i = 0; i < MAX_WEAR; ++i) {
    mob->equipment[i] = 0;
  }

  mob->nr = (short)nr;
  mob->desc = NULL;

  if (!IS_SET(mob->specials.act, ACT_ISNPC)) {
    SET_BIT(mob->specials.act, ACT_ISNPC);
  }

  /* insert in list */
  mob->next = character_list;
  character_list = mob;

  if (mob->points.gold > GET_LEVEL(mob, WARRIOR_LEVEL_IND) * 1500) {
    char buf[200];
    sprintf(buf, "%s has gold > level * 1500 (%d)", mob->player.short_descr,
      mob->points.gold);
    vlog(buf);
  }

  set_racial_stuff(mob);

  /* set up distributed movement system */

  mob->specials.tick = (signed char)mob_tick_count++;

  if (mob_tick_count == TICK_WRAP_COUNT) {
    mob_tick_count = 0;
  }

  ++mob_index[nr].number;
  ++mob_count;
  return mob;
}

/* read an object from OBJ_FILE */
struct obj_data* read_object(int nr, int type) {
  struct obj_data* obj;
  int tmp;
  int i;
  long bc;
  char chk[50];
  char buf[100];
  struct extra_descr_data* new_descr;

  i = nr;
  if (type == VIRTUAL) {
    nr = real_object(nr);
  }
  if (nr < 0 || nr > top_of_objt) {
    sprintf(buf, "Object (V) %d does not exist in database.", i);
    return (0);
  }

  fseek(obj_f, obj_index[nr].pos, 0);

  CREATE(obj, struct obj_data, 1);
  bc = sizeof(struct obj_data);

  clear_object(obj);

  /* *** string data *** */

  obj->name = fread_string(obj_f);
  if (obj->name && *obj->name) {
    bc += strlen(obj->name);
  }
  obj->short_description = fread_string(obj_f);
  if (obj->short_description && *obj->short_description) {
    bc += strlen(obj->short_description);
  }
  obj->description = fread_string(obj_f);
  if (obj->description && *obj->description) {
    bc += strlen(obj->description);
  }
  obj->action_description = fread_string(obj_f);
  if (obj->action_description && *obj->action_description) {
    bc += strlen(obj->action_description);
  }

  /* *** numeric data *** */

  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.type_flag = (signed char)tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.extra_flags = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.wear_flags = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.value[0] = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.value[1] = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.value[2] = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.value[3] = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.weight = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.cost = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.cost_per_day = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.struct_points = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.max_struct_points = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.decay_time = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.volume = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.material_points = (unsigned char)tmp;

  /* *** extra descriptions *** */

  obj->ex_description = 0;

  while (fscanf(obj_f, " %s \n", chk), *chk == 'E') {
    CREATE(new_descr, struct extra_descr_data, 1);
    bc += sizeof(struct extra_descr_data);
    new_descr->keyword = fread_string(obj_f);
    if (new_descr->keyword && *new_descr->keyword) {
      bc += strlen(new_descr->keyword);
    }
    new_descr->description = fread_string(obj_f);
    if (new_descr->description && *new_descr->description) {
      bc += strlen(new_descr->description);
    }

    new_descr->next = obj->ex_description;
    obj->ex_description = new_descr;
  }

  for (i = 0; (i < MAX_OBJ_AFFECT) && (*chk == 'A'); i++) {
    fscanf(obj_f, " %d ", &tmp);
    obj->affected[i].location = (short)tmp;
    fscanf(obj_f, " %d \n", &tmp);
    obj->affected[i].modifier = (unsigned long)tmp;
    fscanf(obj_f, " %s \n", chk);
  }

  for (; (i < MAX_OBJ_AFFECT); i++) {
    obj->affected[i].location = APPLY_NONE;
    obj->affected[i].modifier = 0;
  }

  obj->in_room = NOWHERE;
  obj->next_content = 0;
  obj->carried_by = 0;
  obj->equipped_by = 0;
  obj->eq_pos = -1;
  obj->in_obj = 0;
  obj->contains = 0;
  obj->item_number = nr;

  obj->next = object_list;
  object_list = obj;

  obj_index[nr].number++;

  if (ITEM_TYPE(obj) == ITEM_BOARD) {
    InitABoard(obj);
  }

  obj_count++;
#if BYTE_COUNT
  fprintf(stderr, "Object [%d] uses %d bytes\n", obj_index[nr].virtual, bc);
#endif
  total_obc += bc;
  return (obj);
}

#define ZO_DEAD 999

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void zone_update(void) {
  int i;
  struct reset_q_element* update_u;
  struct reset_q_element* temp;
  struct reset_q_element* tmp2;

  /* enqueue zones */

  for (i = 0; i <= top_of_zone_table; i++) {
    if (zone_table[i].age < zone_table[i].lifespan &&
        zone_table[i].reset_mode) {
      (zone_table[i].age)++;
    } else if (zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode) {
      /* enqueue zone */

      CREATE(update_u, struct reset_q_element, 1);

      update_u->zone_to_reset = i;
      update_u->next = 0;

      if (!reset_q.head) {
        reset_q.head = reset_q.tail = update_u;
      } else {
        reset_q.tail->next = update_u;
        reset_q.tail = update_u;
      }

      zone_table[i].age = ZO_DEAD;
    }
  }

  /* dequeue zones (if possible) and reset */

  for (update_u = reset_q.head; update_u; update_u = tmp2) {
    if (update_u->zone_to_reset > top_of_zone_table) {
      /*  this may or may not work */
      /*  may result in some lost memory, but the loss is not signifigant
    over the short run
    */
      update_u->zone_to_reset = 0;
      update_u->next = 0;
    }
    tmp2 = update_u->next;

    if (zone_table[update_u->zone_to_reset].reset_mode == 2 ||
        is_empty(update_u->zone_to_reset)) {
      reset_zone(update_u->zone_to_reset);
      /* dequeue */

      if (update_u == reset_q.head) {
        reset_q.head = reset_q.head->next;
      } else {
        for (temp = reset_q.head; temp->next != update_u; temp = temp->next) {
          ;
        }

        if (!update_u->next) {
          reset_q.tail = temp;
        }

        temp->next = update_u->next;
      }

      free(update_u);
    }
  }
}

#define ZCMD zone_table[zone].cmd[cmd_no]

/* execute the reset command table of a given zone */
void reset_zone(int zone) {
  int cmd_no;
  int last_cmd = 1;
  char buf[256];
  struct char_data* mob;
  struct char_data* master;
  struct obj_data* obj;
  struct obj_data* obj_to;
  struct room_data* rp;

  mob = 0;

  for (cmd_no = 0;; cmd_no++) {
    if (ZCMD.command == 'S') {
      break;
    }

    if (last_cmd || !ZCMD.if_flag) {
      switch (ZCMD.command) {
        case 'M': /* read a mobile */
          if (mob_index[ZCMD.arg1].number < ZCMD.arg2) {
            mob = read_mobile(ZCMD.arg1, REAL);
            mob->specials.zone = zone;
            char_to_room(mob, ZCMD.arg3);
            last_cmd = 1;
          } else {
            last_cmd = 0;
          }
          break;

        case 'C': /* read a mobile.  Charm them to follow prev. */
          if (mob_index[ZCMD.arg1].number < ZCMD.arg2) {
            master = mob;
            mob = read_mobile(ZCMD.arg1, REAL);
            mob->specials.zone = zone;
            char_to_room(mob, ZCMD.arg3);
            if (master) {
              /*
              add the charm bit to the dude.
              */
              add_follower(mob, master);
              SET_BIT(mob->specials.affected_by, AFF_CHARM);
            }
            last_cmd = 1;
          } else {
            last_cmd = 0;
          }
          break;

        case 'O': /* read an object */
          if (obj_index[ZCMD.arg1].number < ZCMD.arg2) {
            if (ZCMD.arg3 >= 0 && ((rp = real_roomp(ZCMD.arg3)) != NULL)) {
              if ((obj = read_object(ZCMD.arg1, REAL)) != NULL) {
                obj_to_room(obj, ZCMD.arg3);
                last_cmd = 1;
              } else {
                last_cmd = 0;
              }
            } else if (obj = read_object(ZCMD.arg1, REAL)) {
              sprintf(buf, "Error finding room #%d", ZCMD.arg3);
              vlog(buf);
              extract_obj(obj);
              last_cmd = 1;
            } else {
              last_cmd = 0;
            }
          }
          break;

        case 'P': /* object to object */
          if (obj_index[ZCMD.arg1].number < ZCMD.arg2) {
            obj = read_object(ZCMD.arg1, REAL);
            obj_to = get_obj_num(ZCMD.arg3);
            if (obj_to && obj) {
              obj_to_obj(obj, obj_to);
              last_cmd = 1;
            } else {
              last_cmd = 0;
            }
          } else {
            last_cmd = 0;
          }
          break;

        case 'G': /* obj_to_char */
          if (mob && obj_index[ZCMD.arg1].number < ZCMD.arg2 &&
              (obj = read_object(ZCMD.arg1, REAL))) {
            obj_to_char(obj, mob);
            last_cmd = 1;
          } else {
            last_cmd = 0;
          }
          break;

        case 'H': /* hatred to char */
          if (mob && AddHatred(mob, ZCMD.arg1, ZCMD.arg2)) {
            last_cmd = 1;
          } else {
            last_cmd = 0;
          }
          break;

        case 'F': /* fear to char */
          if (mob && AddFears(mob, ZCMD.arg1, ZCMD.arg2)) {
            last_cmd = 1;
          } else {
            last_cmd = 0;
          }
          break;

        case 'E': /* object to equipment list */
          if (mob && obj_index[ZCMD.arg1].number < ZCMD.arg2 &&
              (obj = read_object(ZCMD.arg1, REAL))) {
            equip_char(mob, obj, ZCMD.arg3);
            last_cmd = 1;
          } else {
            last_cmd = 0;
          }
          break;

        case 'D': /* set state of door */
          rp = real_roomp(ZCMD.arg1);
          if (rp && rp->dir_option[ZCMD.arg2]) {
            switch (ZCMD.arg3) {
              case 0:
                REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
                REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
                break;
              case 1:
                SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
                REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
                break;
              case 2:
                SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
                SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
                break;
            }
            last_cmd = 1;
          } else {
            /* that exit doesn't exist anymore */
          }
          break;

        default:
          sprintf(buf, "Undefd cmd in reset table; zone %d cmd %d.\n\r", zone,
            cmd_no);
          vlog(buf);
          break;
      }
    } else {
      last_cmd = 0;
    }
  }

  zone_table[zone].age = 0;
}

#undef ZCMD

/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int is_empty(int zone_nr) {
  struct descriptor_data* i;

  for (i = descriptor_list; i; i = i->next) {
    if (!i->connected) {
      if (real_roomp(i->character->in_room)->zone == zone_nr) {
        return (0);
      }
    }
  }

  return (1);
}

/*************************************************************************
 *  stuff related to the save/load player system								  *
 *********************************************************************** */

/* Load a char, TRUE if loaded, FALSE if not */
int load_char(char* name, struct char_file_u* char_element) {
  FILE* fl;
  int player_i;

  if ((player_i = find_name(name)) >= 0) {
    if (!(fl = fopen(PLAYER_FILE, "r"))) {
      perror("Opening player file for reading. (db.c, load_char)");
      exit(0);
    }

    fseek(fl, (long)(player_table[player_i].nr * sizeof(struct char_file_u)),
      0);

    fread(char_element, sizeof(struct char_file_u), 1, fl);
    fclose(fl);
    /*
    **  Kludge for ressurection
    */
    char_element->talks[2] = TRUE;
    return (player_i);
  }
  return (-1);
}

/* copy data from the file structure to a char struct */
void store_to_char(struct char_file_u* st, struct char_data* ch) {
  int i;

  GET_SEX(ch) = st->sex;
  ch->player.class = st->class;

  for (i = MAGE_LEVEL_IND; i <= RANGER_LEVEL_IND; i++) {
    ch->player.level[i] = st->level[i];
  }

  GET_RACE(ch) = st->race;

  ch->player.short_descr = 0;
  ch->player.long_descr = 0;

  if (*st->title) {
    CREATE(ch->player.title, char, strlen(st->title) + 1);
    strcpy(ch->player.title, st->title);
  } else {
    GET_TITLE(ch) = 0;
  }

  if (*st->description) {
    CREATE(ch->player.description, char, strlen(st->description) + 1);
    strcpy(ch->player.description, st->description);
  } else {
    ch->player.description = 0;
  }

  ch->player.hometown = st->hometown;

  ch->player.time.birth = st->birth;
  ch->player.time.played = st->played;
  ch->player.time.logon = time(0);

  for (i = 0; i <= MAX_TOUNGE - 1; i++) {
    ch->player.talks[i] = st->talks[i];
  }

  ch->player.weight = st->weight;
  ch->player.height = st->height;

  ch->abilities = st->abilities;
  ch->tmpabilities = st->abilities;
  ch->points = st->points;

  SpaceForSkills(ch);

  for (i = 0; i <= MAX_SKILLS - 1; i++) {
    ch->skills[i] = st->skills[i];
  }

  ch->specials.spells_to_learn = st->spells_to_learn;
  ch->specials.alignment = st->alignment;

  ch->specials.act = st->act;
  ch->specials.carry_weight = 0;
  ch->specials.carry_items = 0;
  ch->points.armor = 100;
  ch->points.hitroll = 0;
  ch->points.damroll = 0;

  CREATE(GET_NAME(ch), char, strlen(st->name) + 1);
  strcpy(GET_NAME(ch), st->name);

  /* Not used as far as I can see (Michael) */
  for (i = 0; i <= 4; i++) {
    ch->specials.apply_saving_throw[i] = st->apply_saving_throw[i];
  }

  for (i = 0; i <= 2; i++) {
    GET_COND(ch, i) = st->conditions[i];
  }

  /* Add all spell effects */
  for (i = 0; i < MAX_AFFECT; i++) {
    if (st->affected[i].type) {
      /* Convert from file format to runtime format */
      struct affected_type af;
      af.type = st->affected[i].type;
      af.duration = st->affected[i].duration;
      af.modifier = st->affected[i].modifier;
      af.location = st->affected[i].location;
      af.bitvector = COMPAT_TO_LONG(st->affected[i].bitvector);
      af.next = NULL;
      affect_to_char(ch, &af);
    }
  }
  ch->in_room = st->load_room;
  affect_total(ch);
} /* store_to_char */

/* copy vital data from a players char-structure to the file structure */
void char_to_store(struct char_data* ch, struct char_file_u* st) {
  int i;
  struct affected_type* af;
  struct obj_data* char_eq[MAX_WEAR];

  /* Unaffect everything a character can be affected by */

  for (i = 0; i < MAX_WEAR; i++) {
    if (ch->equipment[i]) {
      char_eq[i] = unequip_char_for_save(ch, i);
    } else {
      char_eq[i] = 0;
    }
  }

  for (af = ch->affected, i = 0; i < MAX_AFFECT; i++) {
    if (af) {
      /* Copy affected_type to affected_type_file with compat conversion */
      st->affected[i].type = af->type;
      st->affected[i].duration = af->duration;
      st->affected[i].modifier = af->modifier;
      st->affected[i].location = af->location;
      st->affected[i].bitvector = LONG_TO_COMPAT(af->bitvector);
      st->affected[i].next = 0;
      /* subtract effect of the spell or the effect will be doubled */
      affect_modify(ch, st->affected[i].location, st->affected[i].modifier,
        COMPAT_TO_LONG(st->affected[i].bitvector), FALSE);
      af = af->next;
    } else {
      st->affected[i].type = 0; /* Zero signifies not used */
      st->affected[i].duration = 0;
      st->affected[i].modifier = 0;
      st->affected[i].location = 0;
      st->affected[i].bitvector = 0;
      st->affected[i].next = 0;
    }
  }

  if ((i >= MAX_AFFECT) && af && af->next) {
    vlog("WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");
  }

  ch->tmpabilities = ch->abilities;

  st->birth = ch->player.time.birth;
  st->played = ch->player.time.played;
  st->played += (long)(time(0) - ch->player.time.logon);
  st->last_logon = time(0);

  ch->player.time.played = st->played;
  ch->player.time.logon = time(0);

  st->hometown = ch->player.hometown;
  st->weight = GET_WEIGHT(ch);
  st->height = GET_HEIGHT(ch);
  st->sex = GET_SEX(ch);
  st->class = ch->player.class;
  for (i = MAGE_LEVEL_IND; i <= RANGER_LEVEL_IND; i++) {
    st->level[i] = ch->player.level[i];
  }
  st->race = GET_RACE(ch);

  st->abilities = ch->abilities;
  st->points = ch->points;
  st->alignment = ch->specials.alignment;
  st->spells_to_learn = ch->specials.spells_to_learn;
  st->act = ch->specials.act;

  st->points.armor = 100;
  st->points.hitroll = 0;
  st->points.damroll = 0;

  if (GET_TITLE(ch)) {
    strcpy(st->title, GET_TITLE(ch));
  } else {
    *st->title = '\0';
  }

  if (ch->player.description) {
    strcpy(st->description, ch->player.description);
  } else {
    *st->description = '\0';
  }

  for (i = 0; i <= MAX_TOUNGE - 1; i++) {
    st->talks[i] = ch->player.talks[i];
  }

  for (i = 0; i <= MAX_SKILLS - 1; i++) {
    st->skills[i] = ch->skills[i];
  }

  /* Copy name, ensuring null termination within 20-byte limit */
  (void)snprintf(st->name, sizeof(st->name), "%s", GET_NAME(ch));

  for (i = 0; i <= 4; i++) {
    st->apply_saving_throw[i] = ch->specials.apply_saving_throw[i];
  }

  for (i = 0; i <= 2; i++) {
    st->conditions[i] = GET_COND(ch, i);
  }

  for (af = ch->affected, i = 0; i < MAX_AFFECT; i++) {
    if (af) {
      /* Add effect of the spell or it will be lost */
      /* When saving without quitting               */
      affect_modify(ch, st->affected[i].location, st->affected[i].modifier,
        st->affected[i].bitvector, TRUE);
      af = af->next;
    }
  }

  for (i = 0; i < MAX_WEAR; i++) {
    if (char_eq[i]) {
      equip_char(ch, char_eq[i], i);
    }
  }

  affect_total(ch);
} /* Char to store */

/* create a new entry in the in-memory index table for the player file */
int create_entry(char* name) {
  int i;

  if (top_of_p_table == -1) {
    CREATE(player_table, struct player_index_element, 1);
    top_of_p_table = 0;
  } else if (!(player_table = (struct player_index_element*)realloc(
                 player_table, sizeof(struct player_index_element) *
                                 (++top_of_p_table + 1)))) {
    perror("create entry");
    exit(1);
  }

  CREATE(player_table[top_of_p_table].name, char, strlen(name) + 1);

  /* copy lowercase equivalent of name to table field */
  for (i = 0; *(player_table[top_of_p_table].name + i) = LOWER(*(name + i));
    i++) {
    ;
  }

  player_table[top_of_p_table].nr = top_of_p_table;

  return (top_of_p_table);
}

/* write the vital data of a player to the player file */
void save_char(struct char_data* ch, short int load_room) {
  struct char_file_u st;
  FILE* fl;
  char mode[4];
  int expand;
  struct char_data* tmp;

  if (IS_NPC(ch) && !(IS_SET(ch->specials.act, ACT_POLYSELF))) {
    return;
  }

  if (IS_NPC(ch)) {
    if (!ch->desc) {
      return;
    }
    tmp = ch->desc->original;
    if (!tmp) {
      return;
    }

  } else {
    if (!ch->desc) {
      return;
    }
    tmp = 0;
  }

  if (expand = (ch->desc->pos > top_of_p_file)) {
    strcpy(mode, "a");
    top_of_p_file++;
  } else {
    strcpy(mode, "r+");
  }

  if (!tmp) {
    char_to_store(ch, &st);
  } else {
    char_to_store(tmp, &st);
  }

  st.load_room = load_room;

  strcpy(st.pwd, ch->desc->pwd);

  if (!(fl = fopen(PLAYER_FILE, mode))) {
    perror("save char");
    exit(1);
  }

  if (!expand) {
    fseek(fl, ch->desc->pos * sizeof(struct char_file_u), 0);
  }

  fwrite(&st, sizeof(struct char_file_u), 1, fl);

  fclose(fl);
}

/* for possible later use with qsort */
int compare(struct player_index_element* arg1,
  struct player_index_element* arg2) {
  return (str_cmp(arg1->name, arg2->name));
}

/************************************************************************
 *  procs of a (more or less) general utility nature			*
 ********************************************************************** */

/* read and allocate space for a '~'-terminated string from a given file */
char* fread_string(FILE* fl) {
  char buf[MAX_STRING_LENGTH];
  char tmp[MAX_STRING_LENGTH];
  memset(buf, 0, sizeof(buf));

  int flag = 0;

  do {
    if (!fgets(tmp, MAX_STRING_LENGTH, fl)) {
      perror("fread_str");
      vlog("File read error.");
      return ("Empty");
    }

    if (strlen(tmp) + strlen(buf) + 1 > MAX_STRING_LENGTH) {
      vlog("fread_string: string too large (db.c)");
      exit(0);
    } else {
      strcat(buf, tmp);
    }

    // Check if buf is empty before attempting to access characters
    if (strlen(buf) < 2) {
      continue;
    }

    // Move point to second-to-last char, checking bounds
    char* point = buf + strlen(buf) - 2;

    if (point < buf) {
      point = buf;
    }

    // Skip whitespace, ensuring we don't go before start of buffer
    while (point >= buf && isspace(*point)) {
      point--;
    }

    flag = point >= buf && *point == '~';

    if (flag) {
      if (strlen(buf) >= 3 && *(buf + strlen(buf) - 3) == '\n') {
        *(buf + strlen(buf) - 2) = '\r';
        *(buf + strlen(buf) - 1) = '\0';
      } else {
        // Ensure we don't write before buffer start
        if (strlen(buf) >= 2) {
          *(buf + strlen(buf) - 2) = '\0';
        } else {
          *buf = '\0';
        }
      }
    } else {
      // Ensure we have room for \r\0
      if (strlen(buf) + 2 <= MAX_STRING_LENGTH) {
        *(buf + strlen(buf) + 1) = '\0';
        *(buf + strlen(buf)) = '\r';
      }
    }
  } while (!flag);

  /* do the allocate boogie  */
  char* rslt = NULL;
  if (strlen(buf) > 0) {
    CREATE(rslt, char, strlen(buf) + 1);
    strcpy(rslt, buf);
  }
  return rslt;
}

/* release memory allocated for a char struct */
void free_char(struct char_data* ch) {
  struct affected_type* af;

  free(GET_NAME(ch));

  if (ch->player.title) {
    free(ch->player.title);
  }
  if (ch->act_ptr) {
    free(ch->act_ptr);
  }
  if (ch->player.short_descr) {
    free(ch->player.short_descr);
  }
  if (ch->player.long_descr) {
    free(ch->player.long_descr);
  }
  if (ch->player.description) {
    free(ch->player.description);
  }
  if (ch->player.sounds) {
    free(ch->player.sounds);
  }
  if (ch->player.distant_snds) {
    free(ch->player.distant_snds);
  }

  struct affected_type* next_af;
  for (af = ch->affected; af; af = next_af) {
    next_af = af->next;
    affect_remove(ch, af);
  }
  if (ch->skills) {
    free(ch->skills);
  }

  FreeHates(ch);
  FreeFears(ch);

  free(ch);
}

/* release memory allocated for an obj struct */
void free_obj(struct obj_data* obj) {
  struct extra_descr_data* this;
  struct extra_descr_data* next_one;

  free(obj->name);
  if (obj->description && *obj->description) {
    free(obj->description);
  }
  if (obj->short_description && *obj->short_description) {
    free(obj->short_description);
  }
  if (obj->action_description && *obj->action_description) {
    free(obj->action_description);
  }

  for (this = obj->ex_description; (this != 0); this = next_one) {
    next_one = this->next;
    if (this->keyword) {
      free(this->keyword);
    }
    if (this->description) {
      free(this->description);
    }
    free(this);
  }

  free(obj);
}

/* read contents of a text file, and place in buf */
int file_to_string(char* name, char* buf) {
  FILE* fl;
  char tmp[100];

  *buf = '\0';

  if (!(fl = fopen(name, "r"))) {
    perror("file-to-string");
    *buf = '\0';
    return (-1);
  }

  do {
    fgets(tmp, 99, fl);

    if (!feof(fl)) {
      if (strlen(buf) + strlen(tmp) + 2 > MAX_STRING_LENGTH) {
        vlog("fl->strng: string too big (db.c, file_to_string)");
        *buf = '\0';
        fclose(fl);
        return (-1);
      }

      strcat(buf, tmp);
      *(buf + strlen(buf) + 1) = '\0';
      *(buf + strlen(buf)) = '\r';
    }
  } while (!feof(fl));

  fclose(fl);

  return (0);
}

void clear_dead_bit(struct char_data* ch) {
  FILE* fl;
  struct char_file_u st;

  fl = fopen(PLAYER_FILE, "r+");
  if (!fl) {
    perror("player file");
    exit(0);
  }

  fseek(fl, ch->desc->pos * sizeof(struct char_file_u), 0);
  fread(&st, sizeof(struct char_file_u), 1, fl);
  /*
   **   this is a serious kludge, and must be changed before multiple
   **   languages can be implemented
   */
  if (st.talks[2]) {
    st.talks[2] = 0; /* fix the 'resurrectable' bit */
    fseek(fl, ch->desc->pos * sizeof(struct char_file_u), 0);
    fwrite(&st, sizeof(struct char_file_u), 1, fl);
    ch->player.talks[2] = 0; /* fix them both */
  }
  fclose(fl);
}

/* clear some of the the working variables of a char */
void reset_char(struct char_data* ch) {
  char buf[100];
  char recipient[100];
  char* tmp;
  struct affected_type* af;

  int i;
  int j;

  for (i = 0; i < MAX_WEAR; i++) { /* Initializing */
    ch->equipment[i] = 0;
  }

  spell_dispel_magic(IMPLEMENTOR, ch, ch, 0);

  if (IS_SET(ch->specials.act, PLR_MAILING)) {
    REMOVE_BIT(ch->specials.act, PLR_MAILING);
  }

  if (GET_RACE(ch) == RACE_OGRE) {
    if (GET_WEIGHT(ch) < 100) {
      GET_WEIGHT(ch) = (200 + number(1, 10));
    }
  }

  ch->desc->screen_size = 24;

  if (!strcmp(ch->player.name, "Brutius") || !strcmp(ch->player.name, "Peel") ||
      !strcmp(ch->player.name, "Damescena") ||
      !strcmp(ch->player.name, "Dash") || !strcmp(ch->player.name, "Jesus")) {
    GET_LEVEL(ch, 0) = BRUTIUS;
    GET_LEVEL(ch, 1) = BRUTIUS;
    GET_LEVEL(ch, 2) = BRUTIUS;
    GET_LEVEL(ch, 3) = BRUTIUS;
    GET_EXP(ch) = 200000000;
  }

  ch->followers = 0;
  ch->master = 0;
  ch->carrying = 0;
  ch->next = 0;

  ch->immune = 0;
  ch->M_immune = 0;
  ch->susc = 0;
  ch->mult_att = 1.0;

  ch->point_roll = 0;
  ch->bet_opt.craps_options = 0;
  ch->bet_opt.one_roll = 0;

  if (!GET_RACE(ch)) {
    GET_RACE(ch) = RACE_HUMAN;
  }
  if (GET_RACE(ch) == RACE_DWARF) {
    if (!IS_AFFECTED(ch, AFF_INFRAVISION)) {
      SET_BIT(ch->specials.affected_by, AFF_INFRAVISION);
    }
  }

  if (HasClass(ch, CLASS_PALADIN)) {
    if (!IS_AFFECTED(ch, AFF_PROTECT_EVIL)) {
      SET_BIT(ch->specials.affected_by, AFF_PROTECT_EVIL);
    }
  }

  if ((ch->player.class == 3) && (GET_LEVEL(ch, THIEF_LEVEL_IND))) {
    ch->player.class = 8;
    send_to_char("Setting your class to THIEF only.\n\r", ch);
  }

  for (i = 0; i <= 3; i++) {
    if (GET_LEVEL(ch, i) > BRUTIUS) {
      GET_LEVEL(ch, i) = 51;
    }
  }

  ch->hunt_dist = 0;
  ch->hatefield = 0;
  ch->fearfield = 0;
  ch->hates.clist = 0;
  ch->fears.clist = 0;

  /* AC adjustment */
  GET_AC(ch) += dex_app[GET_DEX(ch)].defensive;
  if (GET_AC(ch) > 100) {
    GET_AC(ch) = 100;
  }

  GET_HITROLL(ch) = 0;
  GET_DAMROLL(ch) = 0;

  ch->next_fighting = 0;
  ch->next_in_room = 0;
  ch->specials.fighting = 0;
  ch->specials.position = POSITION_STANDING;
  ch->specials.default_pos = POSITION_STANDING;
  ch->specials.carry_weight = 0;
  ch->specials.carry_items = 0;

  if (GET_HIT(ch) <= 0) {
    GET_HIT(ch) = 1;
  }
  if (GET_MOVE(ch) <= 0) {
    GET_MOVE(ch) = 1;
  }
  if (GET_MANA(ch) <= 0) {
    GET_MANA(ch) = 1;
  }

  ch->points.max_mana = 0;
  ch->points.max_move = 0;

  if (IS_IMMORTAL(ch)) {
    GET_BANK(ch) = 0;
    GET_GOLD(ch) = 100000;
  }

  if (GET_BANK(ch) > GetMaxLevel(ch) * 100000) {
    sprintf(buf, "%s has %d coins in bank.", GET_NAME(ch), GET_BANK(ch));
    vlog(buf);
  }
  if (GET_GOLD(ch) > GetMaxLevel(ch) * 100000) {
    sprintf(buf, "%s has %d coins.", GET_NAME(ch), GET_GOLD(ch));
    vlog(buf);
  }

  /*
    Class specific Stuff
    */

  ClassSpecificStuff(ch);

  parse_name(GET_NAME(ch), recipient);

  for (tmp = recipient; *tmp; tmp++) {
    if (isupper(*tmp)) {
      *tmp = tolower(*tmp);
    }
  }

  if (has_mail(recipient)) {
    sprintf(buf, "You have %sMAIL%s.\n\r", VT_BOLDTEX, VT_NORMALT);
    send_to_char(buf, ch);
  }

  if (HasClass(ch, CLASS_MONK)) {
    GET_AC(ch) -= MIN(200, (GET_LEVEL(ch, MONK_LEVEL_IND) * 5));
    GET_HITROLL(ch) += GET_LEVEL(ch, MONK_LEVEL_IND) / 10;
    ch->points.max_move += GET_LEVEL(ch, MONK_LEVEL_IND);
  }

  /*
    racial stuff
    */
  set_racial_stuff(ch);

  /*
    update the affects on the character.
  */

  for (af = ch->affected; af; af = af->next) {
    affect_modify(ch, af->location, (unsigned)af->modifier, af->bitvector,
      TRUE);
  }

  if (!HasClass(ch, CLASS_MONK)) {
    GET_AC(ch) += dex_app[GET_DEX(ch)].defensive;
  }
  if (GET_AC(ch) > 100) {
    GET_AC(ch) = 100;
  }

  for (i = 0; i < 5; i++) {
    ch->specials.apply_saving_throw[i] = 20 - (GetMaxLevel(ch) / 2);
  }

  /*
    clear out the 'dead' bit on characters
  */
  if (ch->desc) {
    clear_dead_bit(ch);
  }
}

/* clear ALL the working variables of a char and do NOT free any space
 * alloc'ed*/
void clear_char(struct char_data* ch) {
  memset(ch, '\0', sizeof(struct char_data));

  ch->in_room = NOWHERE;
  ch->specials.was_in_room = NOWHERE;
  ch->specials.position = POSITION_STANDING;
  ch->specials.default_pos = POSITION_STANDING;
  GET_AC(ch) = 100; /* Basic Armor */
  ch->player.name = NULL;
}

void clear_object(struct obj_data* obj) {
  memset(obj, '\0', sizeof(struct obj_data));

  obj->item_number = -1;
  obj->in_room = NOWHERE;
  obj->eq_pos = -1;
}

/* initialize a new character only if class is set */
void init_char(struct char_data* ch) {
  int i;

  /* *** if this is our first player --- he be God *** */

  if (!strcmp(ch->player.name, "Brutius")) {
    GET_EXP(ch) = 200000000;
    GET_LEVEL(ch, 0) = BRUTIUS;
    GET_LEVEL(ch, 1) = BRUTIUS;
    GET_LEVEL(ch, 2) = BRUTIUS;
    GET_LEVEL(ch, 3) = BRUTIUS;
  }

  set_title(ch);

  ch->player.short_descr = 0;
  ch->player.long_descr = 0;
  ch->player.description = 0;

  ch->player.hometown = number(1, 4);

  ch->player.time.birth = time(0);
  ch->player.time.played = 0;
  ch->player.time.logon = time(0);

  for (i = 0; i < MAX_TOUNGE; i++) {
    ch->player.talks[i] = 0;
  }

  GET_STR(ch) = 9;
  GET_INT(ch) = 9;
  GET_WIS(ch) = 9;
  GET_DEX(ch) = 9;
  GET_CON(ch) = 9;

  /* make favors for sex */
  if (GET_RACE(ch) == RACE_HUMAN) {
    if (ch->player.sex == SEX_MALE) {
      ch->player.weight = number(120, 180);
      ch->player.height = number(160, 200);
    } else {
      ch->player.weight = number(100, 160);
      ch->player.height = number(150, 180);
    }
  } else if (GET_RACE(ch) == RACE_DWARF) {
    if (ch->player.sex == SEX_MALE) {
      ch->player.weight = number(120, 180);
      ch->player.height = number(100, 150);
    } else {
      ch->player.weight = number(100, 160);
      ch->player.height = number(100, 150);
    }

  } else if (GET_RACE(ch) == RACE_ELVEN) {
    if (ch->player.sex == SEX_MALE) {
      ch->player.weight = number(100, 150);
      ch->player.height = number(160, 200);
    } else {
      ch->player.weight = number(80, 230);
      ch->player.height = number(150, 180);
    }
  } else if (GET_RACE(ch) == RACE_OGRE) {
    if (ch->player.sex == SEX_MALE) {
      ch->player.weight = 255;
      ch->player.height = number(200, 250);
    } else {
      ch->player.weight = 255;
      ch->player.height = number(200, 250);
    }
  } else if (GET_RACE(ch) == RACE_GNOME) {
    if (ch->player.sex == SEX_MALE) {
      ch->player.weight = number(85, 100);
      ch->player.height = number(80, 130);
    } else {
      ch->player.weight = number(65, 80);
      ch->player.height = number(60, 110);
    }
  } else if (GET_RACE(ch) == RACE_HOBBIT) {
    if (ch->player.sex == SEX_MALE) {
      ch->player.weight = number(70, 90);
      ch->player.height = number(60, 100);
    } else {
      ch->player.weight = number(60, 80);
      ch->player.height = number(50, 90);
    }
  } else {
    if (ch->player.sex == SEX_MALE) {
      ch->player.weight = number(120, 180);
      ch->player.height = number(160, 200);
    } else {
      ch->player.weight = number(100, 160);
      ch->player.height = number(150, 180);
    }
  }

  ch->points.mana = GET_MAX_MANA(ch);
  ch->points.hit = GET_MAX_HIT(ch);
  ch->points.move = GET_MAX_MOVE(ch);

  ch->points.armor = 100;

  if (!ch->skills) {
    SpaceForSkills(ch);
  }

  for (i = 0; i <= MAX_SKILLS - 1; i++) {
    if (GetMaxLevel(ch) < IMPLEMENTOR) {
      ch->skills[i].learned = 0;
      ch->skills[i].recognise = FALSE;
    } else {
      ch->skills[i].learned = 100;
      ch->skills[i].recognise = FALSE;
    }
  }

  ch->specials.affected_by = 0;
  ch->specials.spells_to_learn = 0;

  for (i = 0; i < 5; i++) {
    ch->specials.apply_saving_throw[i] = 0;
  }

  for (i = 0; i < 3; i++) {
    GET_COND(ch, i) = (GetMaxLevel(ch) > GOD ? -1 : 24);
  }
}

struct room_data* real_roomp(int virtual) {
#if defined(HASH) && HASH
  return hash_find(&room_db, virtual);
#else
  return (virtual < WORLD_SIZE) && (virtual > -1) ? room_db[virtual] : NULL;
#endif
}

/* returns the real number of the monster with given virtual number */
int real_mobile(int virtual) {
  int bot;
  int top;
  int mid;

  bot = 0;
  top = top_of_mobt;

  /* perform binary search on mob-table */
  for (;;) {
    mid = (bot + top) / 2;

    if ((mob_index + mid)->virtual == virtual) {
      return (mid);
    }
    if (bot >= top) {
      return (-1);
    }
    if ((mob_index + mid)->virtual > virtual) {
      top = mid - 1;
    } else {
      bot = mid + 1;
    }
  }
}

/* returns the real number of the object with given virtual number */
int real_object(int virtual) {
  int bot;
  int top;
  int mid;

  bot = 0;
  top = top_of_objt;

  /* perform binary search on obj-table */
  for (;;) {
    mid = (bot + top) / 2;

    if ((obj_index + mid)->virtual == virtual) {
      return (mid);
    }
    if (bot >= top) {
      return (-1);
    }
    if ((obj_index + mid)->virtual > virtual) {
      top = mid - 1;
    } else {
      bot = mid + 1;
    }
  }
}
