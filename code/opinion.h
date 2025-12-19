#pragma once

#include "structs.h"

void FreeFears(struct char_data* ch);
void FreeHates(struct char_data* ch);

int RemHated(struct char_data* ch, struct char_data* pud);
int AddHated(struct char_data* ch, struct char_data* pud);
int AddHatred(struct char_data* ch, int parm_type, int parm);
int Hates(struct char_data* ch, struct char_data* v);
void ZeroHatred(struct char_data* ch, struct char_data* v);
void ZeroFeared(struct char_data* ch, struct char_data* v);
int RemFeared(struct char_data* ch, struct char_data* pud);
int AddFears(struct char_data* ch, int parm_type, int parm);
int AddFeared(struct char_data* ch, struct char_data* pud);
int Fears(struct char_data* ch, struct char_data* v);

struct char_data* FindAFearee(struct char_data* ch);
struct char_data* FindAHatee(struct char_data* ch);
