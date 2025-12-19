#pragma once

#include "structs.h"

int GetClassLevel(struct char_data* ch, int class);
int CountBits(int class);
int OnlyClass(struct char_data* ch, int class);
int HasClass(struct char_data* ch, int class);
int HowManyClasses(struct char_data* ch);
int BestFightingClass(struct char_data* ch);
int BestThiefClass(struct char_data* ch);
int BestMagicClass(struct char_data* ch);
int GetMaxLevel(struct char_data* ch);
int GetTotLevel(struct char_data* ch);


