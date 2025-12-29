/* ************************************************************************
 *  file: db.h , Database module.                          Part of DIKUMUD *
 *  Usage: Loading/Saving chars booting world.                             *
 ************************************************************************* */

#pragma once

#include "hash.h"
#include "structs.h"

#include <stdio.h>

/* data files used by the game system */

#define DFLT_DIR "lib" /* default data directory     */

#define WORLD_FILE "tinyworld.wld" /* room definitions           */
#define MOB_FILE "tinyworld.mob"   /* monster prototypes         */
#define OBJ_FILE "tinyworld.obj"   /* object prototypes          */
#define ATLAS1_FILE "zone1"        /* New atlas made by stagazer */
#define ATLAS2_FILE "zone2"
#define ATLAS3_FILE "zone3"
#define ATLAS4_FILE "zone4"
#define ZONE_FILE "tinyworld.zon" /* zone defs & command tables */
#define CREDITS_FILE "credits"    /* for the 'credits' command  */
#define NEWS_FILE "news"          /* for the 'news' command     */
#define WIZNEWS_FILE "wiznews"
#define MOTD_FILE "motd" /* messages of today          */
#define ANSI_FILE "ansi"
#define PLAYER_FILE "players"  /* the player database        */
#define TIME_FILE "time"       /* game calendar information  */
#define IDEA_FILE "ideas"      /* for the 'idea'-command     */
#define TYPO_FILE "typos"      /*         'typo'             */
#define BUG_FILE "bugs"        /*         'bug'              */
#define MESS_FILE "messages"   /* damage message             */
#define SOCMESS_FILE "actions" /* messgs for social acts     */
#define HELP_PATH "help/"      /* for HELP <keywrd>          */
#define IMMORTAL_HELP_PATH "help/_immortal"
#define BUILDER_HELP_PATH "help/_builder"
#define HELP_PAGE_FILE "help/general" /* for HELP <CR>              */
#define INFO_FILE "info"              /* for INFO                   */
#define WIZLIST_FILE "wizlist"        /* for WIZLIST                */
#define POSEMESS_FILE "poses"         /* for 'pose'-command         */

/* public procedures in db.c */

void boot_db(void);
void save_char(struct char_data* ch, short int load_room);
int create_entry(char* name);
void zone_update(void);
void init_char(struct char_data* ch);
[[nodiscard]] int load_char(char* name, struct char_file_u* char_element);
void clear_char(struct char_data* ch);
void clear_object(struct obj_data* obj);
void reset_char(struct char_data* ch);
void free_char(struct char_data* ch);
[[nodiscard]] struct room_data* real_roomp(int virtual);
[[nodiscard]] char* fread_string(FILE* fl);
[[nodiscard]] int real_object(int virtual);
[[nodiscard]] int real_mobile(int virtual);
void boot_zones(void);
[[nodiscard]] Room* allocate_room(int room_number);
void boot_world(void);
struct index_data* generate_indices(FILE* fl, int* top);
void build_player_index(void);
void char_to_store(struct char_data* ch, struct char_file_u* st);
void store_to_char(struct char_file_u* st, struct char_data* ch);
int is_empty(int zone_nr);
void reset_zone(int zone);
int file_to_string(const char* name, char* buf);
void renum_zone_table(void);
void reset_time(void);
struct obj_data* unequip_char_for_save(struct char_data* ch, int pos);
void load_messages(void);
void assign_command_pointers(void);
void assign_spell_pointers(void);
void boot_social_messages(void);
void boot_pose_messages(void);
void change_char_file(void); /* In reception.c */
void update_obj_file(void);
int DetermineExp(struct char_data* mob, int exp_flags);

#define REAL 0
#define VIRTUAL 1

/* Memory allocation functions - [[nodiscard]] to catch ignored returns */
[[nodiscard]] struct obj_data* read_object(int nr, int type);
[[nodiscard]] struct char_data* read_mobile(int nr, int type);

/* structure for the reset commands */
struct reset_com {
    char command; /* current command                      */
    char if_flag; /* if true: exe only if preceding exe'd */
    int arg1;     /*                                      */
    int arg2;     /* Arguments to the command             */
    int arg3;     /*                                      */

    /*
     *  Commands:              *
     *  'M': Read a mobile     *
     *  'O': Read an object    *
     *  'G': Give obj to mob   *
     *  'P': Put obj in obj    *
     *  'G': Obj to char       *
     *  'E': Obj to char equip *
     *  'D': Set state of door *
     */
};

/* zone definition structure. for the 'zone-table'   */
struct zone_data {
    char* name;   /* name of this zone                  */
    int lifespan; /* how long between resets (minutes)  */
    int age;      /* current age of this zone (minutes) */
    int top;      /* upper limit for rooms in this zone */

    int reset_mode;        /* conditions for reset (see below)   */
    struct reset_com* cmd; /* command table for reset	           */

    /*
     *  Reset mode:                              *
     *  0: Don't reset, and don't update age.    *
     *  1: Reset if no PC's are located in zone. *
     *  2: Just reset.                           *
     */
};

/* element in monster and object index-tables   */
struct index_data {
    int virtual; /* virtual number of this mob/obj           */
    long pos;    /* file position of this field              */
    int number;  /* number of existing units of this mob/obj	*/
    ProcFn func; /* special procedure for this mob/obj       */
    const char* name;
};

/* for queueing zones for update   */
struct reset_q_element {
    int zone_to_reset; /* ref to zone_data */
    struct reset_q_element* next;
};

/* structure for the update queue     */
struct reset_q_type {
    struct reset_q_element* head;
    struct reset_q_element* tail;
};

extern struct reset_q_type reset_q;

struct player_index_element {
    char* name;
    int nr;
};

struct help_index_element {
    char* keyword;
    long pos;
};

extern struct message_list fight_messages[MAX_MESSAGES];

extern struct index_data* mob_index;
extern struct index_data* obj_index;
extern struct player_index_element* player_table;
extern struct obj_data* object_list;
extern struct char_data* character_list;
extern struct time_info_data time_info;
extern struct weather_data weather_info;

extern struct room_data* room_db[WORLD_SIZE];

extern int top_of_world;
extern int top_of_mobt;
extern int top_of_objt;
extern int top_of_p_table;
extern int top_of_zone_table;
extern long room_count;
extern long mob_count;
extern int obj_count;
extern int no_mail;

extern char motd[MAX_STRING_LENGTH];
extern char ansi[MAX_STRING_LENGTH];
extern struct zone_data* zone_table;

void cleanout_room(struct room_data* rp);
void load_one_room(FILE* fl, struct room_data* rp);
void update_time(void);
void free_obj(struct obj_data* obj);
extern void assign_mobiles(void);
extern void assign_objects(void);
extern void assign_rooms(void);
extern void boot_the_shops(void);
extern void assign_the_shopkeepers(void);
