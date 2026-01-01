#pragma once

#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

#include "character_flags.h"
#include "compat_types.h"
#include "game_constants.h"
#include "object_flags.h"

/* casino stuff */

struct bet_data {
    long come;
    long crap;
    long slot;
    long eleven;
    long twelve;
    long two;
    long three;
    long horn_bet;
    long field_bet;
    long hard_eight;
    long hard_six;
    long hard_ten;
    long hard_four;
    long seven;
    long one_craps;
};

struct char_bet_data {
    long craps_options;
    long one_roll;
    int roul_options;
};

struct nodes {
    int visited;
    int ancestor;
};

struct room_q {
    int room_nr;
    struct room_q* next_q;
};

struct string_block {
    int size;
    char* data;
};

/*
  memory stuff
*/

struct char_list {
    struct char_data* op_ch;
    char name[50];
    struct char_list* next;
};

typedef struct {
    struct char_list* clist;
    int sex;        /*number 1=male,2=female,3=both,4=neut,5=m&n,6=f&n,7=all*/
    int race;       /*number */
    int char_class; /* 1=m,2=c,4=f,8=t */
    int vnum;       /* # */
    int evil;       /* align < evil = attack */
    int good;       /* align > good = attack */
} Opinion;

struct extra_descr_data {
    char* keyword;                 /* Keyword in look/examine          */
    char* description;             /* What to see                      */
    struct extra_descr_data* next; /* Next in list                     */
};

// NOLINTBEGIN(misc-redundant-expression) - intentional macro value validation
static_assert(MAX_OBJ_AFFECT == 5,
  "MAX_OBJ_AFFECT must be 5 for save file compat");
// NOLINTEND(misc-redundant-expression)

struct obj_flag_data {
    int value[4];          /* Values of the item (see list)    */
    signed char type_flag; /* Type of item                     */
    int wear_flags;        /* Where you can wear it            */
    long extra_flags;      /* If it hums,glows etc             */
    int weight;            /* Weigt what else                  */
    int cost;              /* Value when sold (gp.)            */
    int cost_per_day;      /* Cost to keep pr. real day        */
    int timer;             /* Timer for object                 */
    long bitvector;        /* To set chars bits                */
    int decay_time;
    int struct_points;
    int max_struct_points;
    unsigned char material_points;
    int volume;
};

/* Runtime version for in-memory objects */
struct obj_affected_type {
    short location; /* Which ability to change (APPLY_XXX) */
    long modifier;  /* How much it changes by (signed for penalties) */
};

/* File format version - 32-bit compatible for binary file I/O */
/* Used in OBJ_FILE_ELEM - DO NOT CHANGE SIZE/LAYOUT (maintains original 32-bit
 * layout) */
struct obj_affected_type_file {
    short location;
    compat_long modifier; /* 32-bit signed for file compat */
};

/* ======================== Structure for object ========================= */
struct obj_data {
    short int item_number;          /* Where in data-base               */
    int in_room;                    /* In what room -1 when conta/carr  */
    struct obj_flag_data obj_flags; /* Object information               */
    struct obj_affected_type
      affected[MAX_OBJ_AFFECT]; /* Which abilities in PC to change  */

    struct char_data* killer; /* for use with corpses */
    short int char_vnum;      /* for ressurection     */
    long char_f_pos;          /* for ressurection     */
    char* name;               /* Title of object :get etc.        */
    char* description;        /* When in room                     */
    char* short_description;  /* when worn/carry/in cont.         */
    char* action_description; /* What to write when used          */
    struct extra_descr_data* ex_description; /* extra descriptions     */
    struct char_data* carried_by;  /* Carried by :nullptr in room/conta   */
    signed char eq_pos;            /* what is the equip. pos?          */
    struct char_data* equipped_by; /* equipped by :nullptr in room/conta  */

    struct obj_data* in_obj;   /* In what object nullptr when none    */
    struct obj_data* contains; /* Contains objects                 */

    struct obj_data* next_content; /* For 'contains' lists             */
    struct obj_data* next;         /* For the object list              */
};

/* ======================================================================= */

struct room_direction_data {
    char* general_description; /* When look DIR.                  */
    char* keyword;             /* for open/close                  */
    short int exit_info;       /* Exit info                       */
    int key;                   /* Key's number (-1 for no key)    */
    int to_room;               /* Where direction leeds (NOWHERE) */
};

/* ========================= Structure for room ========================== */
struct room_data {
    short int number; /* Rooms number                       */
    short int zone;   /* room_data zone (for resetting)          */
    int sector_type;  /* sector type (move/hide)            */

    int river_dir;   /* dir of flow on river               */
    int river_speed; /* speed of flow on river             */

    int tele_time;        /* time to a teleport                 */
    int tele_targ;        /* target room of a teleport          */
    char tele_look;       /* do a do_look or not when
       teleported                         */
    unsigned char moblim; /* # of mobs allowed in room.         */

    char* name;        /* Rooms name 'You are ...'           */
    char* description; /* Shown when entered                 */
    struct extra_descr_data* ex_description;   /* for examine/look       */
    struct room_direction_data* dir_option[6]; /* Directions           */
    long room_flags;   /* DEATH,DARK ... etc                 */
    signed char light; /* Number of lightsources in room     */
    int (*funct)(struct char_data*, int, const char*); /* special procedure */

    struct obj_data* contents; /* List of items in room              */
    struct char_data* people;  /* List of NPC / PC in room           */
};

extern struct room_data* world;

/* ======================================================================== */

// NOLINTBEGIN(misc-redundant-expression) - intentional macro value validation
static_assert(WEAR_RADIO < MAX_WEAR, "WEAR_RADIO must be less than MAX_WEAR");
static_assert(MAX_TOUNGE >= 3,
  "MAX_TOUNGE must be at least 3 for save file compat");
static_assert(MAX_SKILLS == 200, "MAX_SKILLS must be 200 for save file compat");
static_assert(MAX_AFFECT == 25, "MAX_AFFECT must be 25 for save file compat");
// NOLINTEND(misc-redundant-expression)

/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data {
    signed char hours, day, month;
    short int year;
};

/* These data contain information about a players time data */
struct time_data {
    time_t birth; /* This represents the characters age                */
    time_t logon; /* Time of the last logon (used to calculate played) */
    int played;   /* This is the total accumulated time played in secs */
};

struct char_player_data {
    char* name;               /* PC / NPC s name (kill ...  )         */
    char* short_descr;        /* for 'actions'                        */
    char* long_descr;         /* for 'look'.. Only here for testing   */
    char* description;        /* Extra descriptions                   */
    char* title;              /* PC / NPC s title                     */
    char* sounds;             /* Sound that the monster makes (in room) */
    char* distant_snds;       /* Sound that the monster makes (other) */
    signed char sex;          /* PC / NPC s sex                       */
    unsigned char char_class; /* PC s class or NPC alignment          */
    signed char level[8];     /* PC / NPC s level                     */
    int hometown;             /* PC s Hometown (zone)                 */
    char talks[MAX_TOUNGE];   /* PC s Tounges 0 for NPC           */
    struct time_data time;    /* PC s AGE in days                 */
    unsigned char weight;     /* PC / NPC s weight                    */
    unsigned char height;     /* PC / NPC s height                    */
};

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_ability_data {
    signed char str;
    signed char str_add; /* 000 - 100 if strength 18             */
    signed char intel;
    signed char wis;
    signed char dex;
    signed char con;
};

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_point_data {
    short int mana;
    short int max_mana;

    short int hit;
    short int max_hit; /* Max hit for NPC                         */
    short int move;
    short int max_move; /* Max move for NPC                        */

    short int armor; /* Internal -100..100, external -10..10 AC */
    int gold;        /* Money carried                           */
    int bankgold;    /* gold in the bank.                       */
    int exp;         /* The experience of the player            */

    signed char hitroll; /* Any bonus or penalty to the hit roll    */
    signed char damroll; /* Any bonus or penalty to the damage roll */
};

struct char_poofin_data {
    char* poofin;
    char* poofout;
    int pmask;
};

struct char_special_data {
    int zone;                   /* zone that an NPC lives in */
    struct char_data* fighting; /* Opponent                             */

    struct char_data* hunting; /* Hunting person..                     */

    long affected_by; /* Bitvector for spells/skills affected by */

    signed char tick; /* the tick that the mob/player is on  */

    signed char position;    /* Standing or ...                         */
    signed char default_pos; /* Default position for NPC                */
    unsigned long act;       /* flags for NPC behavior                  */

    unsigned char spells_to_learn; /* How many can you learn yet this level   */

    int carry_weight; /* Carried weight                          */
    int carry_items;  /* Number of items carried                 */
    int timer;        /* Timer for update                        */
    int was_in_room;  /* storage of location for linkdead people */
    short int apply_saving_throw[5]; /* Saving throw (Bonuses)             */
    signed char conditions[3]; /* Drunk full etc.                        */

    signed char damnodice;      /* The number of damage dice's            */
    signed char damsizedice;    /* The size of the damage dice's          */
    signed char last_direction; /* The last direction the monster went    */
    int attack_type;            /* The Attack Type Bitvector for NPC's    */
    int alignment;              /* +-1000 for alignments                  */
};

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_skill_data {
    signed char learned; /* % chance for success 0 = not learned   */
    char recognise;      /* If you can recognise the scroll etc.   */
};

/* Runtime version - used for in-memory linked lists */
struct affected_type {
    short type;           /* The type of spell that caused this      */
    short int duration;   /* For how long its effects will last      */
    signed char modifier; /* This is added to apropriate ability     */
    signed char location; /* Tells which ability to change(APPLY_XXX)*/
    long bitvector;       /* Tells which bits to set (AFF_XXX)       */

    struct affected_type* next;
};

/* File format version - 32-bit compatible for binary file I/O */
/* Used in CHAR_FILE_U *DO*NOT*CHANGE* (maintains original 32-bit layout) */
struct affected_type_file {
    short type;
    short int duration;
    signed char modifier;
    signed char location;
    compat_long bitvector; /* 32-bit for file compat */
    compat_ptr next;       /* 32-bit placeholder (unused in files) */
};

struct follow_type {
    struct char_data* follower;
    struct follow_type* next;
};

enum mob_act_type {
  MOB_ACT_LATTIMORE,
  MOB_ACT_RINGWRAITH,
  MOB_ACT_GUARDIAN,
  MOB_ACT_BOUNTYHUNTER,
};

struct mob_act_ringwraith {
    int ringnumber;
    int chances;
};

struct mob_act_lattimore {
    short pointer;
    char** names;
    int* status;
    short index;
    short c;
};

struct mob_act_guardian {
    char** names;
    short num_names;
};

struct mob_act_bountyhunter {
    char hunted_item[80];
    char hunted_victim[80];
    int num_chances;
    int level_command;
    int num_retrieved;
};

struct mob_act_data {
    enum mob_act_type type;
    union {
        struct mob_act_lattimore lattimore;
        struct mob_act_ringwraith ringwraith;
        struct mob_act_guardian guardian;
        struct mob_act_bountyhunter bountyhunter;
    } data;
};

/* ================== Structure for player/non-player ===================== */
struct char_data {
    short int nr; /* monster nr (pos in file)    */
    int in_room;  /* Location                    */
    /*
      will need several new affects

      maybe these should go in points?  (and thusly be saved and restored?)

    */

    unsigned immune;   /* Immunities                  */
    unsigned M_immune; /* Meta Immunities             */
    unsigned susc;     /* susceptibilities            */
    float mult_att;    /* the number of attacks       */
    signed char attackers;

    short int fallspeed; /* rate of descent for player */
    short int race;
    short int hunt_dist; /* max dist the player can hunt */

    unsigned short hatefield;
    unsigned short fearfield;

    Opinion hates;
    Opinion fears;

    short int persist;
    int old_room;

    struct mob_act_data* act_ptr; /* data for mobile actions */

    struct char_player_data player;        /* Normal data                 */
    struct char_ability_data abilities;    /* Abilities                   */
    struct char_ability_data tmpabilities; /* The abilities we will use   */
    struct char_point_data points;         /* Points                      */
    struct char_special_data specials;     /* Special plaing constants    */
    struct char_skill_data* skills;        /* Skills                 */

    struct affected_type* affected;       /* affected by what spells     */
    struct obj_data* equipment[MAX_WEAR]; /* Equipment array             */

    struct obj_data* carrying;    /* Head of list                */
    struct descriptor_data* desc; /* nullptr for mobiles            */
    struct char_data* orig;       /* Special for polymorph       */

    struct char_data* next_in_room;  /* For room->people - list       */
    struct char_data* next;          /* For either monster or ppl-lis */
    struct char_data* next_fighting; /* For fighting list             */

    struct follow_type* followers; /* List of chars followers     */
    struct char_data* master;      /* Who is char following?      */
    int invis_level;               /* visibility of gods */
    short wimpy;                   /* If wimpy set, max hp before autoflee */
    int point_roll;
    struct bet_data bet;
    struct char_bet_data bet_opt;
    struct char_poofin_data poof;
};

typedef union {
    int (*mob_f)(struct char_data*, int, const char*);
    int (*obj_f)(struct char_data*, int, const char*, struct obj_data*);
    int (*room_f)(struct char_data*, int, const char*);
} ProcFn;

/* ======================================================================== */

struct weather_data {
    int pressure; /* How is the pressure ( Mb ) */
    int change;   /* How fast and what way does it change. */
    int sky;      /* How is the sky. */
    int sunlight; /* And how much sun. */
};

/* ***********************************************************************
 *  file element for player file. BEWARE: Changing it will ruin the file  *
 *  Uses compat types to maintain 32-bit binary file compatibility        *
 *********************************************************************** */

struct char_file_u {
    signed char sex;
    unsigned char char_class;
    signed char level[8];
    compat_time birth; /* Time of birth of character (32-bit for compat) */
    int played;        /* Number of secs played in total */

    int race;
    unsigned char weight;
    unsigned char height;

    char title[80];
    short int hometown;
    char description[240];
    char talks[MAX_TOUNGE];

    short int load_room; /* Which room to place char in  */

    struct char_ability_data abilities;

    struct char_point_data points;

    struct char_skill_data skills[MAX_SKILLS];

    struct affected_type_file affected[MAX_AFFECT];

    /* specials */

    signed char spells_to_learn;
    int alignment;

    compat_time last_logon; /* Time (in secs) of last logon (32-bit compat) */
    compat_ulong act;       /* ACT Flags (32-bit for compat) */

    /* char data */
    char name[20];
    char pwd[11];
    short int apply_saving_throw[5];
    int conditions[3];
};

/* ***********************************************************************
 *  file element for object file. BEWARE: Changing it will ruin the file  *
 *********************************************************************** */

struct obj_cost { /* used in act.other.c:do_save as
         well as in reception2.c */
    int total_cost;
    int no_carried;
    char ok;
};

#define MAX_OBJ_SAVE 200 /* Used in OBJ_FILE_U *DO*NOT*CHANGE* */
// NOLINTNEXTLINE(misc-redundant-expression) - intentional macro value
// validation
static_assert(MAX_OBJ_SAVE == 200,
  "MAX_OBJ_SAVE must be 200 for save file compat");

struct obj_file_elem {
    short int item_number;

    int value[4];
    int extra_flags;
    int weight;
    int timer;
    compat_long bitvector; /* 32-bit for file compat */
    char name[128];        /* big, but not horrendously so */
    char sd[128];
    char desc[256];
    struct obj_affected_type_file affected[MAX_OBJ_AFFECT];
    int decay_time;
    int struct_points;
    int max_struct_points;
    unsigned char material_points;
    int volume;
};

struct obj_file_u {
    char owner[20];           /* Name of player                     */
    int gold_left;            /* Number of goldcoins left at owner  */
    int total_cost;           /* The cost for all items, per day    */
    compat_long last_update;  /* Time in seconds (32-bit compat)  */
    compat_long minimum_stay; /* For stasis (32-bit compat)       */
    int number;               /* number of objects */
    struct obj_file_elem objects[MAX_OBJ_SAVE];
};

/* ***********************************************************
 *  The following structures are related to descriptor_data   *
 *********************************************************** */

struct txt_block {
    char* text;
    struct txt_block* next;
};

struct txt_q {
    struct txt_block* head;
    struct txt_block* tail;
};

struct snoop_data {
    struct char_data* snooping;
    /* Who is this char snooping */
    struct char_data* snoop_by;
    /* And who is snooping on this char */
};

struct descriptor_data {
    int descriptor;                    /* file descriptor for socket */
    char host[50];                     /* hostname                   */
    char pwd[12];                      /* password                   */
    int pos;                           /* position in player-file    */
    int connected;                     /* mode of 'connectedness'    */
    int wait;                          /* wait for how many loops    */
    char* showstr_head;                /* for paging through texts	*/
    char* showstr_point;               /*       -                    */
    char** str;                        /* for the modify-str system  */
    int max_str;                       /* -                          */
    int prompt_mode;                   /* control of prompt-printing */
    char buf[MAX_STRING_LENGTH];       /* buffer for raw input       */
    char last_input[MAX_INPUT_LENGTH]; /* the last input         */
    char stat[MAX_STAT];
    struct txt_q output;          /* q of strings to send       */
    struct txt_q input;           /* q of unprocessed input     */
    struct char_data* character;  /* linked to char             */
    struct char_data* original;   /* original char              */
    struct snoop_data snoop;      /* to snoop people.	         */
    struct descriptor_data* next; /* link to next descriptor    */
    char* pagedfile;              /* what file is getting paged */
    long position;                /* where in that file 		 	*/
    char name[20];
    char* prompt;
    int screen_size;
};

struct msg_type {
    char* attacker_msg; /* message to attacker */
    char* victim_msg;   /* message to victim   */
    char* room_msg;     /* message to room     */
};

struct message_type {
    struct msg_type die_msg;       /* messages when death            */
    struct msg_type miss_msg;      /* messages when miss             */
    struct msg_type hit_msg;       /* messages when hit              */
    struct msg_type sanctuary_msg; /* messages when hit on sanctuary */
    struct msg_type god_msg;       /* messages when hit on god       */
    struct message_type* next;     /* to next messages of this kind.*/
};

struct message_list {
    int a_type;               /* Attack type 						 */
    int number_of_attacks;    /* How many attack messages to chose from. */
    struct message_type* msg; /* List of messages.				 */
};

struct dex_skill_type {
    short int p_pocket;
    short int p_locks;
    short int traps;
    short int sneak;
    short int hide;
    int volume;
};

struct dex_app_type {
    short int reaction;
    short int miss_att;
    short int defensive;
};

struct str_app_type {
    short int tohit;   /* To Hit (THAC0) Bonus/Penalty        */
    short int todam;   /* Damage Bonus/Penalty                */
    short int carry_w; /* Maximum weight that can be carrried */
    short int wield_w; /* Maximum weight that can be wielded  */
};

struct wis_app_type {
    signed char bonus; /* how many bonus skills a player can */
                       /* practice pr. level                 */
};

struct int_app_type {
    signed char learn; /* how many % a player learns a spell/skill */
};

struct con_app_type {
    short int hitp;
    short int shock;
};
