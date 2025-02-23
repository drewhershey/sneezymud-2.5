/* ************************************************************************
 *  file: limits.h , Limit/Gain control module             Part of DIKUMUD *
 *  Usage: declaration of title type                                       *
 ************************************************************************* */

#ifndef LIMITS_H
#define LIMITS_H

#include "structs.h"

int mana_limit(struct char_data* ch);
int mana_gain(struct char_data* ch);
int hit_limit(struct char_data* ch);
int hit_gain(struct char_data* ch);
int move_limit(struct char_data* ch);
int move_gain(struct char_data* ch);
void gain_condition(struct char_data* ch, int condition, int value);
int ClassSpecificStuff(struct char_data* ch);
void gain_exp(struct char_data* ch, int gain);
void advance_level(struct char_data* ch, int class);
void set_title(struct char_data* ch);

#endif
