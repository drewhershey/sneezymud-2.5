#pragma once

struct char_data;

int GetClassLevel(struct char_data* ch, int char_class);
int CountBits(int char_class);
int OnlyClass(struct char_data* ch, int char_class);
int HasClass(struct char_data* ch, int char_class);
int HowManyClasses(struct char_data* ch);
int BestFightingClass(struct char_data* ch);
int BestThiefClass(struct char_data* ch);
int BestMagicClass(struct char_data* ch);
int GetMaxLevel(struct char_data* ch);
int GetTotLevel(struct char_data* ch);
