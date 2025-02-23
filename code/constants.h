#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "structs.h"

struct QuestItem {
    int item;
    const char* where;
};

extern const struct QuestItem QuestList[4][IMMORTAL];

struct title_type {
    const char* title_m;
    const char* title_f;
    int exp;
};

extern const struct title_type titles[8][ABS_MAX_LVL];

extern const char* const exits[];
extern const char* const dirs[];
extern struct dex_app_type dex_app[26];
extern struct int_app_type int_app[26];
extern struct wis_app_type wis_app[26];
extern struct con_app_type con_app[26];
extern const struct dex_skill_type dex_app_skill[26];
extern const struct str_app_type str_app[31];
extern const char* const RaceName[];
extern const char* const drinks[];
extern const int rev_dir[];
extern const int vol_mult[];
extern const char* const drinknames[];
extern const char* const affected_bits[];
extern const char* const apply_types[];
extern const char* const item_types[];
extern const char* const extra_bits[];
extern const char* const material_types[];

#endif
