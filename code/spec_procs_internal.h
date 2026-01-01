#pragma once

struct char_data;
struct obj_data;

/* Internal constants */
#define INQ_SHOUT 1
#define INQ_LOOSE 0

#define SWORD_ANCIENTS 25000
#define ELF_HOME 1414
#define BAKERY 3009
#define DUMP_ROOM 3030
#define IVORY_GATE 1499

#define MAX_NPC_CORPSE_TIME 5
#define MAX_PC_CORPSE_TIME 10

/* Internal struct types */
struct social_type {
    char* cmd;
    int next_line;
};

/* Shared helper functions - declared here, defined in spec_procs_common.c */
void make_head(struct char_data* ch);
int gain_level(struct char_data* ch, int char_class);

int check_nomagic(struct char_data* ch, char* msg_ch, char* msg_rm);
int num_charmed_followers_in_room(struct char_data* ch);
int check_for_blocked_move(struct char_data* ch, int cmd, int room,
  int from_dir, int at_door);
int room_of_object(struct obj_data* obj);

/* Combat helpers */
int get_dam_bonus(struct obj_data* w);
int get_damage(struct obj_data* w, struct char_data* ch);
int get_hand_damage(struct char_data* ch);
int is_weapon(struct obj_data* o);

/* Misc helpers */
int has_object(struct char_data* ch, int ob_num);
