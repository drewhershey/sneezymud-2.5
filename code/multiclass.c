#include "accessors.h"
#include "bit_ops.h"
#include "character_flags.h"
#include "multiclass.h"
#include "structs.h"
#include "utils.h"

int GetClassLevel(struct char_data* ch, int char_class) {
  if (IS_SET(ch->player.char_class, char_class)) {
    return (GET_LEVEL(ch, CountBits(char_class) - 1));
  }
  return (0);
}

int CountBits(int char_class) {
  if (char_class == 1) {
    return (1);
  }
  if (char_class == 2) {
    return (2);
  }
  if (char_class == 4) {
    return (3);
  }
  if (char_class == 8) {
    return (4);
  }
  if (char_class == 16) {
    return (5);
  }
  if (char_class == 32) {
    return (6);
  }
  if (char_class == 64) {
    return (7);
  }
  if (char_class == 128) {
    return (8);
  }
  return 0;
}

int OnlyClass(struct char_data* ch, int char_class) {
  int i = 0;

  for (i = 1; i <= 8; i *= 2) {
    if (GetClassLevel(ch, i) != 0) {
      if (i != char_class) {
        return 0;
      }
    }
  }
  return 1;
}

int HasClass(struct char_data* ch, int char_class) {
  if (!IS_PC(ch)) {
    if (!IS_SET(char_class, CLASS_MONK)) {
      return 1;
    }
  }

  if (IS_SET(ch->player.char_class, char_class)) {
    return 1;
  }

  return 0;
}

int HowManyClasses(struct char_data* ch) {
  short i = 0;
  short tot = 0;

  for (i = 0; i < 8; i++) {
    if (GET_LEVEL(ch, i)) {
      tot++;
    }
  }
  if (tot) {
    return (tot);
  }
  if (IS_SET(ch->player.char_class, CLASS_MAGIC_USER)) {
    tot++;
  }

  if (IS_SET(ch->player.char_class, CLASS_WARRIOR)) {
    tot++;
  }

  if (IS_SET(ch->player.char_class, CLASS_THIEF)) {
    tot++;
  }

  if (IS_SET(ch->player.char_class, CLASS_CLERIC)) {
    tot++;
  }

  if (IS_SET(ch->player.char_class, CLASS_ANTIPALADIN)) {
    tot++;
  }

  if (IS_SET(ch->player.char_class, CLASS_PALADIN)) {
    tot++;
  }

  if (IS_SET(ch->player.char_class, CLASS_RANGER)) {
    tot++;
  }

  if (IS_SET(ch->player.char_class, CLASS_MONK)) {
    tot++;
  }

  return tot;
}

int BestFightingClass(struct char_data* ch) {
  if (GET_LEVEL(ch, WARRIOR_LEVEL_IND)) {
    return (WARRIOR_LEVEL_IND);
  }
  if (GET_LEVEL(ch, PALADIN_LEVEL_IND)) {
    return (PALADIN_LEVEL_IND);
  }
  if (GET_LEVEL(ch, ANTIPALADIN_LEVEL_IND)) {
    return (ANTIPALADIN_LEVEL_IND);
  }
  if (GET_LEVEL(ch, RANGER_LEVEL_IND)) {
    return (RANGER_LEVEL_IND);
  }
  if (GET_LEVEL(ch, MONK_LEVEL_IND)) {
    return (MONK_LEVEL_IND);
  }
  if (GET_LEVEL(ch, CLERIC_LEVEL_IND)) {
    return (CLERIC_LEVEL_IND);
  }
  if (GET_LEVEL(ch, THIEF_LEVEL_IND)) {
    return (THIEF_LEVEL_IND);
  }
  if (GET_LEVEL(ch, MAGE_LEVEL_IND)) {
    return (MAGE_LEVEL_IND);
  }

  vlog("Massive error.. character has no recognized class.");
  vlog(GET_NAME(ch));

  return (1);
}

int BestThiefClass(struct char_data* ch) {
  if (GET_LEVEL(ch, THIEF_LEVEL_IND)) {
    return (THIEF_LEVEL_IND);
  }
  if (GET_LEVEL(ch, MONK_LEVEL_IND)) {
    return (MONK_LEVEL_IND);
  }
  if (GET_LEVEL(ch, ANTIPALADIN_LEVEL_IND)) {
    return (ANTIPALADIN_LEVEL_IND);
  }
  if (GET_LEVEL(ch, MAGE_LEVEL_IND)) {
    return (MAGE_LEVEL_IND);
  }
  if (GET_LEVEL(ch, WARRIOR_LEVEL_IND)) {
    return (WARRIOR_LEVEL_IND);
  }
  if (GET_LEVEL(ch, RANGER_LEVEL_IND)) {
    return (RANGER_LEVEL_IND);
  }
  if (GET_LEVEL(ch, CLERIC_LEVEL_IND)) {
    return (CLERIC_LEVEL_IND);
  }
  if (GET_LEVEL(ch, PALADIN_LEVEL_IND)) {
    return (PALADIN_LEVEL_IND);
  }

  vlog("Massive error.. character has no recognized class.");
  vlog(GET_NAME(ch));

  return (1);
}

int BestMagicClass(struct char_data* ch) {
  if (GET_LEVEL(ch, MAGE_LEVEL_IND)) {
    return (MAGE_LEVEL_IND);
  }
  if (GET_LEVEL(ch, CLERIC_LEVEL_IND)) {
    return (CLERIC_LEVEL_IND);
  }
  if (GET_LEVEL(ch, PALADIN_LEVEL_IND)) {
    return (PALADIN_LEVEL_IND);
  }
  if (GET_LEVEL(ch, RANGER_LEVEL_IND)) {
    return (RANGER_LEVEL_IND);
  }
  if (GET_LEVEL(ch, ANTIPALADIN_LEVEL_IND)) {
    return (ANTIPALADIN_LEVEL_IND);
  }
  if (GET_LEVEL(ch, THIEF_LEVEL_IND)) {
    return (THIEF_LEVEL_IND);
  }
  if (GET_LEVEL(ch, WARRIOR_LEVEL_IND)) {
    return (WARRIOR_LEVEL_IND);
  }
  if (GET_LEVEL(ch, MONK_LEVEL_IND)) {
    return (MONK_LEVEL_IND);
  }

  vlog("Massive error.. character has no recognized class.");
  vlog(GET_NAME(ch));

  return (1);
}

static int get_a_level(struct char_data* ch, int which) {
  signed char ind[4];
  signed char j = 0;
  signed char k = 0;
  signed char i = 0;

  for (i = MAGE_LEVEL_IND; i <= THIEF_LEVEL_IND; i++) {
    ind[i] = GET_LEVEL(ch, i);
  }

  /*
   *  chintzy sort. (just to prove that I did learn something in college)
   */

  for (i = 0; i <= 2; i++) {
    for (j = i + 1; j <= 3; j++) {
      if (ind[j] > ind[i]) {
        k = ind[i];
        ind[i] = ind[j];
        ind[j] = k;
      }
    }
  }

  if (which > -1 && which < 4) {
    return (ind[which]);
  }
  return 0;
}

int GetMaxLevel(struct char_data* ch) {
  int max = 0;
  int i = 0;

  for (i = MAGE_LEVEL_IND; i <= RANGER_LEVEL_IND; i++) {
    if (GET_LEVEL(ch, i) > max) {
      max = GET_LEVEL(ch, i);
    }
  }

  return (max);
}

int GetTotLevel(struct char_data* ch) {
  return (GET_LEVEL(ch, 0) + GET_LEVEL(ch, 1) + GET_LEVEL(ch, 2) +
          GET_LEVEL(ch, 3) + GET_LEVEL(ch, 4) + GET_LEVEL(ch, 5) +
          GET_LEVEL(ch, 6) + GET_LEVEL(ch, 7));
}
