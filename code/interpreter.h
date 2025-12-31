#pragma once

#include <stdio.h>

struct char_data;
struct char_file_u;
struct descriptor_data;
struct obj_file_u;

#define MAX_CMD_LIST 400

extern const char* const path[];
extern const char* const command[];
extern int WizLock;
extern int Silence;

struct command_info {
    void (
      *command_pointer)(struct char_data* ch, const char* argument, int cmd);
    signed char minimum_position;
    signed char minimum_level;
};

extern struct command_info cmd_info[MAX_CMD_LIST];

/* File I/O */
int read_objs(FILE* fl, struct obj_file_u* st);

/* Command parsing */
void command_interpreter(struct char_data* ch, char* argument);
int search_block(const char* arg, const char* const* list, char exact);
int old_search_block(const char* argument, int begin, int length,
  const char* const* list, int mode);
void argument_interpreter(const char* argument, char* first_arg,
  char* second_arg);
const char* one_argument(const char* argument, char* first_arg);
void only_argument(const char* argument, char* dest);
int fill_word(char* argument);
void half_chop(const char* string, char* arg1, char* arg2);
int is_abbrev(const char* arg1, const char* arg2);
int is_number(const char* str);

/* Special procedures */
int special(struct char_data* ch, int cmd, const char* arg);

/* Character management */
void nanny(struct descriptor_data* d, char* arg);
int parse_name(const char* arg, char* name);
int find_name(char* name);
void set_title(struct char_data* ch);
void init_char(struct char_data* ch);
void store_to_char(struct char_file_u* st, struct char_data* ch);
int create_entry(char* name);

/* System */
char* crypt(const char*, const char*);
