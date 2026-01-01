#include <math.h>
#include <sys/param.h>

#include "accessors.h"
#include "character_flags.h"
#include "handler.h"
#include "structs.h"
#include "utils.h"

void SwitchStuff(struct char_data* giver, struct char_data* taker) {
  struct obj_data* obj = nullptr;
  struct obj_data* next = nullptr;
  float ratio = NAN;
  int j = 0;

  /*
   *  take all the stuff from the giver, put in on the
   *  taker
   */

  for (j = 0; j < MAX_WEAR; j++) {
    if (giver->equipment[j] != nullptr) {
      obj = unequip_char(giver, j);
      obj_to_char(obj, taker);
    }
  }

  for (obj = giver->carrying; obj != nullptr; obj = next) {
    next = obj->next_content;
    obj_from_char(obj);
    obj_to_char(obj, taker);
  }

  /*
   *    gold...
   */

  GET_GOLD(taker) = GET_GOLD(giver);

  /*
   *   hit point ratio
   */

  if (GET_HIT(taker) > GET_HIT(giver)) {
    GET_HIT(taker) = GET_HIT(giver);
  }

  /*
   * experience
   */

  GET_EXP(taker) = GET_EXP(giver);
  GET_EXP(taker) = MIN(GET_EXP(taker), 100000000);

  /*
   *  humanoid monsters can cast spells
   */

  if (IS_NPC(taker)) {
    taker->player.char_class = giver->player.char_class;
    if (taker->skills == nullptr) {
      SpaceForSkills(taker);
    }
    for (j = 0; j < MAX_SKILLS; j++) {
      taker->skills[j].learned = giver->skills[j].learned;
      taker->skills[j].recognise = giver->skills[j].recognise;
    }
    for (j = 0; j <= 3; j++) {
      taker->player.level[j] = giver->player.level[j];
    }
  }

  GET_MANA(taker) = GET_MANA(giver);
  GET_ALIGNMENT(taker) = GET_ALIGNMENT(giver);
}
