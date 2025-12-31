#pragma once

struct affected_type;
struct char_data;
struct descriptor_data;
struct obj_data;
struct string_block;

extern struct char_data* combat_list;

/* handling the affected-structures */
void affect_total(struct char_data* ch);
void affect_modify(struct char_data* ch, signed char loc, long mod, long bitv,
  char add);
void affect_to_char(struct char_data* ch, struct affected_type* af);
void affect_remove(struct char_data* ch, struct affected_type* af);
void affect_from_char(struct char_data* ch, short skill);
char affected_by_spell(struct char_data* ch, short skill);
void affect_join(struct char_data* ch, struct affected_type* af, char avg_dur,
  char avg_mod);

/* utility */
[[nodiscard]] struct obj_data* create_money(int amount);
int isname(const char* str, const char* namelist);
[[nodiscard]] char* fname(char* namelist);

/* ******** objects *********** */

void obj_to_char(struct obj_data* object, struct char_data* ch);
void obj_from_char(struct obj_data* object);

void equip_char(struct char_data* ch, struct obj_data* obj, int pos);
[[nodiscard]] struct obj_data* unequip_char(struct char_data* ch, int pos);

[[nodiscard]] struct obj_data* get_obj_in_list(const char* name,
  struct obj_data* list);
[[nodiscard]] struct obj_data* get_obj_in_list_num(int num,
  struct obj_data* list);
[[nodiscard]] struct obj_data* get_obj(const char* name);
[[nodiscard]] struct obj_data* get_obj_num(int nr);
[[nodiscard]] struct obj_data* get_obj_vis_accessible(struct char_data* ch,
  char* name);

void obj_to_room(struct obj_data* object, int room);
void obj_from_room(struct obj_data* object);
void obj_to_obj(struct obj_data* obj, struct obj_data* obj_to);
void obj_from_obj(struct obj_data* obj);
void object_list_new_owner(struct obj_data* list, struct char_data* ch);

void extract_obj(struct obj_data* obj);

/* ******* characters ********* */

[[nodiscard]] struct char_data* get_char_room(char* name, int room);
[[nodiscard]] struct char_data* get_char_num(int nr);
[[nodiscard]] struct char_data* get_char(char* name);

void char_from_room(struct char_data* ch);
void char_to_room(struct char_data* ch, int room);

/* find if character can see */
[[nodiscard]] struct char_data* get_char_room_vis(struct char_data* ch,
  const char* name);
[[nodiscard]] struct char_data* get_char_vis_world(struct char_data* ch,
  const char* name, int* count);
[[nodiscard]] struct char_data* get_char_vis(struct char_data* ch,
  const char* name);
[[nodiscard]] struct obj_data* get_obj_in_list_vis(struct char_data* ch,
  const char* name, struct obj_data* list);
[[nodiscard]] struct obj_data* get_obj_vis(struct char_data* ch,
  const char* name);
[[nodiscard]] struct obj_data* get_obj_vis_world(struct char_data* ch,
  const char* name, int* count);

void extract_char(struct char_data* ch);

/* Generic Find */

int generic_find(const char* arg, int bitvector, struct char_data* ch,
  struct char_data** tar_ch, struct obj_data** tar_obj);

#define FIND_CHAR_ROOM 1
#define FIND_CHAR_WORLD 2
#define FIND_OBJ_INV 4
#define FIND_OBJ_ROOM 8
#define FIND_OBJ_WORLD 16
#define FIND_OBJ_EQUIP 32

void append_to_string_block(struct string_block* sb, const char* str);
void init_string_block(struct string_block* sb);
void page_string_block(struct string_block* sb, struct char_data* ch);
void destroy_string_block(struct string_block* sb);
int page_file(struct descriptor_data* d, const char* input);
void page_string(struct descriptor_data* d, char* str, int keep_internal);
int get_number(char** name);
void update_object(struct obj_data* obj, int use);
