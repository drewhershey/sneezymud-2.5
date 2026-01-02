#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include <algorithm>

#include "accessors.h"
#include "bit_ops.h"
#include "character_flags.h"
#include "comm.h"
#include "commands.h"
#include "constants.h"
#include "db.h"
#include "game_constants.h"
#include "handler.h"
#include "interpreter.h"
#include "limits.h"
#include "memory_macros.h"
#include "multiclass.h"
#include "object_flags.h"
#include "opinion.h"
#include "race.h"
#include "room_flags.h"
#include "spell_ids.h"
#include "spell_info.h"
#include "spells.h"
#include "structs.h"
#include "utils.h"

struct room_data* world = nullptr; /* dyn alloc'ed array of rooms     */

static void spell_burning_hands(signed char level, struct char_data* ch) {
  int dam = 0;
  struct char_data* tmp_victim = nullptr;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(1, 6) + level + 1;

  send_to_char("Searing flame fans out in front of you!\n\r", ch);
  act("$n sends a fan of flame shooting from the fingertips!\n\r", 0, ch,
    nullptr, nullptr, TO_ROOM);

  for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim != nullptr;
    tmp_victim = tmp_victim->next_in_room) {
    if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
      if ((GetMaxLevel(tmp_victim) > LOW_IMMORTAL) && (!IS_NPC(tmp_victim))) {
        return;
      }
      if (in_group(ch, tmp_victim) == 0) {
        act("You are seared by the burning flame!\n\r", 0, ch, nullptr,
          tmp_victim, TO_VICT);
        if (saves_spell(tmp_victim, SAVING_SPELL) != 0) {
          dam >>= 1;
        }
        MissileDamage(ch, tmp_victim, dam, SPELL_BURNING_HANDS);
      } else {
        act("You are able to avoid the flames!\n\r", 0, ch, nullptr, tmp_victim,
          TO_VICT);
      }
    }
  }
}

void cast_burning_hands(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*victim*/,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
      spell_burning_hands(level, ch);
      break;
    default:
      vlog("Serious screw-up in burning hands!");
      break;
  }
}

static void spell_call_lightning(signed char level, struct char_data* ch,
  struct char_data* victim) {
  int dam = 0;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(MAX(level, 15), 6);

  if (OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING)) {
    if (saves_spell(victim, SAVING_SPELL) != 0) {
      dam >>= 1;
    }

    MissileDamage(ch, victim, dam, SPELL_CALL_LIGHTNING);
  }
}

void cast_call_lightning(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* victim,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING)) {
        spell_call_lightning(level, ch, victim);
      } else {
        send_to_char("You fail to call upon the lightning from the sky!\n\r",
          ch);
      }
      break;
    case SPELL_TYPE_POTION:
      if (OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING)) {
        spell_call_lightning(level, ch, ch);
      }
      break;
    case SPELL_TYPE_SCROLL:
      if (OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING)) {
        if (victim != nullptr) {
          spell_call_lightning(level, ch, victim);
        } else if (tar_obj == nullptr) {
          spell_call_lightning(level, ch, ch);
        }
      }
      break;
    case SPELL_TYPE_STAFF:
      if (OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING)) {
        for (victim = real_roomp(ch->in_room)->people; victim != nullptr;
          victim = victim->next_in_room) {
          if (in_group(victim, ch) == 0) {
            spell_call_lightning(level, ch, victim);
          }
        }
      }
      break;
    default:
      vlog("Serious screw-up in call lightning!");
      break;
  }
}

static void spell_chill_touch(signed char level, struct char_data* ch,
  struct char_data* victim) {
  struct affected_type af{};
  int dam = 0;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = number(level, 3 * level);

  if (saves_spell(victim, SAVING_SPELL) == 0) {
    af.type = SPELL_CHILL_TOUCH;
    af.duration = 6;
    af.modifier = -1;
    af.location = APPLY_STR;
    af.bitvector = 0;
    affect_join(victim, &af, 1, 0);
  } else {
    dam >>= 1;
  }
  damage(ch, victim, dam, SPELL_CHILL_TOUCH);
}

void cast_chill_touch(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* victim,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
      spell_chill_touch(level, ch, victim);
      break;
    default:
      vlog("Serious screw-up in chill touch!");
      break;
  }
}

static void spell_vampiric_touch(signed char level, struct char_data* ch,
  struct char_data* victim) {
  int hitp = 0;

  assert(victim && ch);
  assert((level >= 1) && (level < ABS_MAX_LVL));

  hitp = MIN(dice(level, 2), GET_MAX_HIT(victim));

  if (IS_IMMORTAL(victim)) {
    send_to_char("Drain a gods blood? NEVER!!\n\r", ch);
    send_to_char("Someone just tried to use a vampiric touch on you!!\n\r", ch);
  }

  if (saves_spell(victim, SAVING_SPELL) != 0) {
    hitp >>= 1;
  }

  if (GET_POS(victim) > POSITION_STUNNED) {
    GET_HIT(victim) -= hitp;

    if (GET_HIT(victim) <= 0) {
      GET_HIT(victim) = 1;
    }

    GET_HIT(ch) += hitp;

    GET_HIT(ch) = std::min<int>(GET_HIT(ch), hit_limit(ch));

    SetVictFighting(ch, victim);

    update_pos(victim);
    update_pos(ch);

    send_to_char("You drain the blood from your prey!\n\r", ch);
    send_to_char("You feel drained as you lose some blood!\n\r", victim);
    WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
  } else {
    send_to_char("The victim has no hit points to drain!\n\r", ch);
    return;
  }
}

void cast_vampiric_touch(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* victim,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
      spell_vampiric_touch(level, ch, victim);
      break;
    default:
      vlog("Serious screw-up in vampiric touch!");
      break;
  }
}

static void spell_life_leech(signed char level, struct char_data* ch) {
  int hitp = 0;
  struct char_data* tmp_victim = nullptr;
  struct char_data* temp = nullptr;

  assert(ch);

  send_to_char("You try to leech everyone in the room.\n\r", ch);

  for (tmp_victim = character_list; tmp_victim != nullptr; tmp_victim = temp) {
    temp = tmp_victim->next;
    if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
      if (in_group(ch, tmp_victim) == 0) {
        if ((GetMaxLevel(tmp_victim) >= LOW_IMMORTAL) &&
            (!IS_NPC(tmp_victim))) {
          send_to_char("Some puny mortal tried to drink you blood!\n\r",
            tmp_victim);
          return;
        }
        hitp = MIN(dice(level, 2), GET_MAX_HIT(tmp_victim));
        GET_HIT(tmp_victim) -= hitp;
        GET_HIT(ch) += hitp;
        send_to_char("You feel your blood pressure drop!!\n\r", tmp_victim);
        SetVictFighting(ch, tmp_victim);

      } else {
        act("You dodge your groupmembers attempt to leech your life!!!\n\r", 0,
          ch, nullptr, tmp_victim, TO_VICT);
      }
      WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
    }
  }
}

void cast_life_leech(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*victim*/,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_life_leech(level, ch);
      break;
    default:
      vlog("Serious screw-up in life leech!");
      break;
  }
}

static void spell_shocking_grasp(signed char level, struct char_data* ch,
  struct char_data* victim) {
  int dam = 0;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = number(1, 8) + level;

  if (saves_spell(victim, SAVING_SPELL) != 0) {
    dam >>= 1;
  }

  damage(ch, victim, dam, SPELL_SHOCKING_GRASP);
}

void cast_shocking_grasp(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* victim,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_shocking_grasp(level, ch, victim);
      break;
    default:
      vlog("Serious screw-up in shocking grasp!");
      break;
  }
}

static void spell_colour_spray(signed char level, struct char_data* ch,
  struct char_data* victim) {
  int dam = 0;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = 4 * level;

  if (saves_spell(victim, SAVING_SPELL) != 0) {
    dam >>= 1;
  }

  MissileDamage(ch, victim, dam, SPELL_COLOUR_SPRAY);
}

void cast_colour_spray(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* victim,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_colour_spray(level, ch, victim);
      break;
    case SPELL_TYPE_SCROLL:
      if (victim != nullptr) {
        spell_colour_spray(level, ch, victim);
      } else if (tar_obj == nullptr) {
        spell_colour_spray(level, ch, ch);
      }
      break;
    case SPELL_TYPE_WAND:
      if (victim != nullptr) {
        spell_colour_spray(level, ch, victim);
      }
      break;
    default:
      vlog("Serious screw-up in colour spray!");
      break;
  }
}

static void spell_earthquake(signed char level, struct char_data* ch) {
  int dam = 0;

  struct char_data* tmp_victim = nullptr;
  struct char_data* temp = nullptr;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(1, 4) + level + 1;

  send_to_char("The earth trembles beneath your feet!\n\r", ch);
  act("$n makes the earth tremble and shiver\n\r", 0, ch, nullptr, nullptr,
    TO_ROOM);

  for (tmp_victim = character_list; tmp_victim != nullptr; tmp_victim = temp) {
    temp = tmp_victim->next;
    if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
      if (in_group(ch, tmp_victim) == 0) {
        if ((GetMaxLevel(tmp_victim) < LOW_IMMORTAL) || (IS_NPC(tmp_victim))) {
          MissileDamage(ch, tmp_victim, dam, SPELL_EARTHQUAKE);
          act("You fall and hurt yourself!!\n\r", 0, ch, nullptr, tmp_victim,
            TO_VICT);
        }

      } else {
        act("You almost fall and hurt yourself!!\n\r", 0, ch, nullptr,
          tmp_victim, TO_VICT);
      }
    } else {
      if (real_roomp(ch->in_room)->zone ==
          real_roomp(tmp_victim->in_room)->zone) {
        send_to_char("The earth trembles...", tmp_victim);
      }
    }
  }
}

void cast_earthquake(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*victim*/,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_earthquake(level, ch);
      break;
    default:
      vlog("Serious screw-up in earthquake!");
      break;
  }
}

/* Drain XP, MANA, HP - caster gains HP and MANA */
static void spell_energy_drain(signed char level, struct char_data* ch,
  struct char_data* victim) {
  int dam = 0;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (saves_spell(victim, SAVING_SPELL) == 0) {
    GET_ALIGNMENT(ch) = MIN(-1000, GET_ALIGNMENT(ch) - 200);

    if (GetMaxLevel(victim) <= 1) {
      damage(ch, victim, 100, SPELL_ENERGY_DRAIN); /* Kill the sucker */
    } else if ((!IS_NPC(victim)) && (GetMaxLevel(victim) >= LOW_IMMORTAL)) {
      send_to_char("Some puny mortal just tried to drain you...\n\r", victim);
    } else {
      if (!IS_SET(victim->M_immune, IMM_DRAIN)) {
        send_to_char("Your life energy is drained!\n\r", victim);
        dam = dice(level, 8); /* nasty spell */
        damage(ch, victim, dam, SPELL_ENERGY_DRAIN);
      } else {
        if (!IS_SET(ch->M_immune, IMM_DRAIN)) {
          send_to_char("Your spell backfires!\n\r", ch);
          dam = dice(level, 8); /* nasty spell */
          damage(ch, victim, dam, SPELL_ENERGY_DRAIN);
        } else {
          send_to_char("Your spell fails utterly.\n\r", ch);
        }
      }
    }
  } else {
    damage(ch, victim, 0, SPELL_ENERGY_DRAIN); /* Miss */
  }
}

void cast_energy_drain(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* victim,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_energy_drain(level, ch, victim);
      break;
    case SPELL_TYPE_POTION:
      spell_energy_drain(level, ch, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (victim != nullptr) {
        spell_energy_drain(level, ch, victim);
      } else if (tar_obj == nullptr) {
        spell_energy_drain(level, ch, ch);
      }
      break;
    case SPELL_TYPE_WAND:
      if (victim != nullptr) {
        spell_energy_drain(level, ch, victim);
      }
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim != nullptr;
        victim = victim->next_in_room) {
        if (in_group(ch, victim) == 0) {
          if (victim != ch) {
            spell_energy_drain(level, ch, victim);
          }
        }
      }
      break;
    default:
      vlog("Serious screw-up in energy drain!");
      break;
  }
}

static void spell_fireball(signed char level, struct char_data* ch) {
  int dam = 0;
  struct char_data* tmp_victim = nullptr;
  struct char_data* temp = nullptr;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(level, 6);
  /*
    this one should be in_world, not in room, so that the message can
    be sent to everyone.
  */

  for (tmp_victim = character_list; tmp_victim != nullptr; tmp_victim = temp) {
    temp = tmp_victim->next;
    if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
      if (in_group(ch, tmp_victim) == 0) {
        if ((GetMaxLevel(tmp_victim) >= LOW_IMMORTAL) &&
            (!IS_NPC(tmp_victim))) {
          send_to_char("Some puny mortal tries to toast you with a fireball",
            tmp_victim);
          return;
        }
        if (saves_spell(tmp_victim, SAVING_SPELL) != 0) {
          dam >>= 1;
        }
        MissileDamage(ch, tmp_victim, dam, SPELL_FIREBALL);

      } else {
        act("You dodge the mass of flame!!\n\r", 0, ch, nullptr, tmp_victim,
          TO_VICT);
      }
    } else {
      if (tmp_victim->in_room != NOWHERE) {
        if (real_roomp(ch->in_room)->zone ==
            real_roomp(tmp_victim->in_room)->zone) {
          send_to_char("You feel a blast of hot air.\n\r", tmp_victim);
        }
      }
    }
  }
}

void cast_fireball(signed char level, struct char_data* ch, const char* /*arg*/,
  int type, struct char_data* /*victim*/, struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
      spell_fireball(level, ch);
      break;
    default:
      vlog("Serious screw-up in fireball");
      break;
  }
}

static void spell_harm(signed char level, struct char_data* ch,
  struct char_data* victim) {
  int dam = 0;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = GET_HIT(victim) - dice(1, 4);

  if (dam < 0) {
    dam = 100; /* Kill the suffering bastard */
  } else {
    if (saves_spell(victim, SAVING_SPELL) != 0) {
      dam = 0;
    }
  }
  dam = MIN(dam, 100);

  damage(ch, victim, dam, SPELL_HARM);
}

void cast_harm(signed char level, struct char_data* ch, const char* /*arg*/,
  int type, struct char_data* victim, struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_harm(level, ch, victim);
      break;
    case SPELL_TYPE_POTION:
      spell_harm(level, ch, ch);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim != nullptr;
        victim = victim->next_in_room) {
        if (in_group(ch, victim) == 0) {
          spell_harm(level, ch, victim);
        }
      }
      break;
    default:
      vlog("Serious screw-up in harm!");
      break;
  }
}

static void spell_lightning_bolt(signed char level, struct char_data* ch,
  struct char_data* victim) {
  int dam = 0;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(level, 6);

  if (saves_spell(victim, SAVING_SPELL) != 0) {
    dam >>= 1;
  }

  MissileDamage(ch, victim, dam, SPELL_LIGHTNING_BOLT);
}

void cast_lightning_bolt(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* victim,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_lightning_bolt(level, ch, victim);
      break;
    case SPELL_TYPE_SCROLL:
      if (victim != nullptr) {
        spell_lightning_bolt(level, ch, victim);
      } else if (tar_obj == nullptr) {
        spell_lightning_bolt(level, ch, ch);
      }
      break;
    case SPELL_TYPE_WAND:
      if (victim != nullptr) {
        spell_lightning_bolt(level, ch, victim);
      }
      break;
    default:
      vlog("Serious screw-up in lightning bolt!");
      break;
  }
}

static void spell_acid_blast(signed char level, struct char_data* ch,
  struct char_data* victim) {
  int dam = 0;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(level, 6);

  if (saves_spell(victim, SAVING_SPELL) != 0) {
    dam >>= 1;
  }

  MissileDamage(ch, victim, dam, SPELL_ACID_BLAST);
}

void cast_acid_blast(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* victim,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_acid_blast(level, ch, victim);
      break;
    case SPELL_TYPE_SCROLL:
      if (victim != nullptr) {
        spell_acid_blast(level, ch, victim);
      } else {
        spell_acid_blast(level, ch, ch);
      }
      break;
    case SPELL_TYPE_WAND:
      if (victim != nullptr) {
        spell_acid_blast(level, ch, victim);
      }
      break;
    default:
      vlog("Serious screw-up in acid blast!");
      break;
  }
}

static void spell_cone_of_cold(signed char level, struct char_data* ch) {
  int dam = 0;
  struct char_data* tmpv = nullptr;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(level, 3) + level + 1;

  send_to_char("A cone of freezing air fans out before you\n\r", ch);
  act("$n sends a cone of ice shooting from the fingertips!\n\r", 0, ch,
    nullptr, nullptr, TO_ROOM);

  for (tmpv = real_roomp(ch->in_room)->people; tmpv != nullptr;
    tmpv = tmpv->next_in_room) {
    if ((ch->in_room == tmpv->in_room) && (ch != tmpv)) {
      if ((GetMaxLevel(tmpv) > LOW_IMMORTAL) && (!IS_NPC(tmpv))) {
        return;
      }
      if (in_group(ch, tmpv) == 0) {
        act("You are chilled to the bone!\n\r", 0, ch, nullptr, tmpv, TO_VICT);
        if (saves_spell(tmpv, SAVING_SPELL) != 0) {
          dam >>= 1;
        }
        MissileDamage(ch, tmpv, dam, SPELL_CONE_OF_COLD);
      } else {
        act("You are able to avoid the cone!\n\r", 0, ch, nullptr, tmpv,
          TO_VICT);
      }
    }
  }
}

void cast_cone_of_cold(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*victim*/,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
      spell_cone_of_cold(level, ch);
      break;

    default:
      vlog("Serious screw-up in cone of cold!");
      break;
  }
}

static void spell_ice_storm(signed char level, struct char_data* ch) {
  int dam = 0;
  struct char_data* tmpv = nullptr;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(3, 10);

  send_to_char("You conjure a storm of ice.\n\r", ch);
  act("$n conjures an ice storm!\n\r", 0, ch, nullptr, nullptr, TO_ROOM);

  for (tmpv = real_roomp(ch->in_room)->people; tmpv != nullptr;
    tmpv = tmpv->next_in_room) {
    if ((ch->in_room == tmpv->in_room) && (ch != tmpv)) {
      if (in_group(ch, tmpv) == 0) {
        act("You are blasted by the storm!\n\r", 0, ch, nullptr, tmpv, TO_VICT);
        if (saves_spell(tmpv, SAVING_SPELL) != 0) {
          dam >>= 1;
        }

        MissileDamage(ch, tmpv, dam, SPELL_ICE_STORM);
      } else {
        act("You are able to dodge the storm!\n\r", 0, ch, nullptr, tmpv,
          TO_VICT);
      }
    }
  }
}

void cast_ice_storm(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*victim*/,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
      spell_ice_storm(level, ch);
      break;

    default:
      vlog("Serious screw-up in ice storm");
      break;
  }
}

static void spell_meteor_swarm(signed char level, struct char_data* ch,
  struct char_data* victim) {
  int dam = 0;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(level, 10);

  if (saves_spell(victim, SAVING_SPELL) != 0) {
    dam >>= 1;
  }

  MissileDamage(ch, victim, dam, SPELL_METEOR_SWARM);
}

void cast_meteor_swarm(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* victim,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_meteor_swarm(level, ch, victim);
      break;
    case SPELL_TYPE_SCROLL:
      if (victim != nullptr) {
        spell_meteor_swarm(level, ch, victim);
      } else {
        spell_meteor_swarm(level, ch, ch);
      }
      break;
    case SPELL_TYPE_WAND:
      if (victim != nullptr) {
        spell_meteor_swarm(level, ch, victim);
      }
      break;
    default:
      vlog("Serious screw-up in meteor swarm!");
      break;
  }
}

static void spell_disintegrate(signed char level, struct char_data* ch,
  struct char_data* victim) {
  int dam = 0;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(level, 15);

  if (OnlyClass(ch, CLASS_MAGIC_USER) != 0) {
    MissileDamage(ch, victim, dam, SPELL_DISINTEGRATE);
  }
}

void cast_disintegrate(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* victim,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_disintegrate(level, ch, victim);
      break;
    case SPELL_TYPE_SCROLL:
      if (victim != nullptr) {
        spell_disintegrate(level, ch, victim);
      } else {
        spell_disintegrate(level, ch, ch);
      }
      break;
    case SPELL_TYPE_WAND:
      if (victim != nullptr) {
        spell_disintegrate(level, ch, victim);
      }
      break;
    default:
      vlog("Serious screw-up in meteor swarm!");
      break;
  }
}

static void spell_flamestrike(signed char level, struct char_data* ch,
  struct char_data* victim) {
  int dam = 0;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(6, 8);

  if (saves_spell(victim, SAVING_SPELL) != 0) {
    dam >>= 1;
  }

  MissileDamage(ch, victim, dam, SPELL_FLAMESTRIKE);
}

void cast_flamestrike(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* victim,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_flamestrike(level, ch, victim);
      break;
    case SPELL_TYPE_SCROLL:
      if (victim != nullptr) {
        spell_flamestrike(level, ch, victim);
      } else if (tar_obj == nullptr) {
        spell_flamestrike(level, ch, ch);
      }
      break;
    case SPELL_TYPE_WAND:
      if (victim != nullptr) {
        spell_flamestrike(level, ch, victim);
      }
      break;
    default:
      vlog("Serious screw-up in flamestrike!");
      break;
  }
}

static void spell_magic_missile(signed char level, struct char_data* ch,
  struct char_data* victim) {
  int dam = 0;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = number((level / 2) + 1, 4) + (level / 2);

  if (affected_by_spell(victim, SPELL_SHIELD) != 0) {
    dam = 0;
  }

  MissileDamage(ch, victim, dam, SPELL_MAGIC_MISSILE);
}

void cast_magic_missile(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* victim,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_magic_missile(level, ch, victim);
      break;
    case SPELL_TYPE_SCROLL:
      if (victim != nullptr) {
        spell_magic_missile(level, ch, victim);
      } else if (tar_obj == nullptr) {
        spell_magic_missile(level, ch, ch);
      }
      break;
    case SPELL_TYPE_WAND:
      if (victim != nullptr) {
        spell_magic_missile(level, ch, victim);
      }
      break;
    default:
      vlog("Serious screw-up in magic missile!");
      break;
  }
}

static void spell_cause_light(signed char level, struct char_data* ch,
  struct char_data* victim) {
  int dam = 0;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(1, 8);

  damage(ch, victim, dam, SPELL_CAUSE_LIGHT);
}

void cast_cause_light(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* victim,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cause_light(level, ch, victim);
      break;
    case SPELL_TYPE_POTION:
      spell_cause_light(level, ch, ch);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim != nullptr;
        victim = victim->next_in_room) {
        if (in_group(ch, victim) == 0) {
          spell_cause_light(level, ch, victim);
        }
      }
      break;
    default:
      vlog("Serious screw-up in cause light wounds!");
      break;
  }
}

static void spell_cause_serious(signed char level, struct char_data* ch,
  struct char_data* victim) {
  int dam = 0;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(2, 8) + 2;

  damage(ch, victim, dam, SPELL_CAUSE_SERIOUS);
}

void cast_cause_serious(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* victim,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cause_serious(level, ch, victim);
      break;
    case SPELL_TYPE_POTION:
      spell_cause_serious(level, ch, ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj != nullptr) {
        return;
      }
      if (victim == nullptr) {
        victim = ch;
      }
      spell_cause_serious(level, ch, victim);
      [[fallthrough]];
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim != nullptr;
        victim = victim->next_in_room) {
        if (in_group(ch, victim) == 0) {
          spell_cause_serious(level, ch, victim);
        }
      }
      break;
    default:
      vlog("Serious screw-up in cause serious wounds!");
      break;
  }
}

static void spell_cause_critical(signed char level, struct char_data* ch,
  struct char_data* victim) {
  int dam = 0;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(3, 8) + 3;

  damage(ch, victim, dam, SPELL_CAUSE_CRITICAL);
}

void cast_cause_critic(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* victim,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cause_critical(level, ch, victim);
      break;
    case SPELL_TYPE_SCROLL:
      if (victim == nullptr) {
        victim = ch;
      }
      spell_cause_critical(level, ch, victim);
      break;
    case SPELL_TYPE_WAND:
      if (victim == nullptr) {
        victim = ch;
      }
      spell_cause_critical(level, ch, victim);
      break;
    case SPELL_TYPE_POTION:
      if (victim == nullptr) {
        victim = ch;
      }
      spell_cause_critical(level, ch, victim);
      break;
    case SPELL_TYPE_STAFF:
      for (victim = real_roomp(ch->in_room)->people; victim != nullptr;
        victim = victim->next_in_room) {
        if (in_group(ch, victim) == 0) {
          spell_cause_critical(level, ch, victim);
        }
      }
      break;
    default:
      vlog("Serious screw-up in cause critical!");
      break;
  }
}

static void spell_geyser(signed char level, struct char_data* ch) {
  int dam = 0;

  struct char_data* tmpv = nullptr;
  struct char_data* temp = nullptr;

  if (ch->in_room < 0) {
    return;
  }
  dam = dice(level, 3);

  act("The Geyser erupts in a huge column of steam!\n\r", 0, ch, nullptr,
    nullptr, TO_ROOM);

  for (tmpv = real_roomp(ch->in_room)->people; tmpv != nullptr; tmpv = temp) {
    temp = tmpv->next_in_room;
    if ((ch != tmpv) && (ch->in_room == tmpv->in_room)) {
      if ((GetMaxLevel(tmpv) < LOW_IMMORTAL) || (IS_NPC(tmpv))) {
        if (MissileDamage(ch, tmpv, dam, SPELL_GEYSER) != 0) {
          return;
        }
        act("You are seared by the boiling water!!\n\r", 0, ch, nullptr, tmpv,
          TO_VICT);
      } else {
        act("You are almost seared by the boiling water!!\n\r", 0, ch, nullptr,
          tmpv, TO_VICT);
      }
    }
  }
}

void cast_geyser(signed char level, struct char_data* ch, const char* /*arg*/,
  int type, struct char_data* /*victim*/, struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_geyser(level, ch);
      break;
    default:
      vlog("Serious screw-up in geyser!");
      break;
  }
}

static void spell_green_slime(signed char level, struct char_data* ch,
  struct char_data* victim) {
  int dam = 0;
  int hpch = 0;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  hpch = GET_MAX_HIT(ch);
  hpch = std::max(hpch, 10);

  dam = (hpch / 10);

  if (saves_spell(victim, SAVING_BREATH) != 0) {
    dam >>= 1;
  }

  send_to_char("You are attacked by green slime!\n\r", victim);

  damage(ch, victim, dam, SPELL_GREEN_SLIME);
}

void cast_green_slime(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* victim,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_green_slime(level, ch, victim);
      break;
    case SPELL_TYPE_SCROLL:
      if (victim != nullptr) {
        spell_green_slime(level, ch, victim);
      } else if (tar_obj == nullptr) {
        spell_green_slime(level, ch, ch);
      }
      break;
    case SPELL_TYPE_WAND:
      if (victim != nullptr) {
        spell_green_slime(level, ch, victim);
      }
      break;
    default:
      vlog("Serious screw-up in green Slime!");
      break;
  }
}

static int enforce_verbal(struct char_data* ch) {
  if (ch == nullptr) {
    return 0;
  }

  if (can_do_verbal(ch) == 0) {
    act("$n opens his mouth as if to say something.", 1, ch, nullptr, nullptr,
      TO_ROOM);
    send_to_char("You are unable to chant the mantra!\n\r", ch);
    return 0;
  }

  return 1;
}

static int perform_gestural(struct char_data* ch) {
  if (ch == nullptr) {
    return 0;
  }

  if (ch->equipment[WIELD] != nullptr) {
    send_to_char(
      "You cannot perform the required gestures while wielding something!\n\r",
      ch);
    return 0;
  }
  if (ch->equipment[WEAR_SHIELD] != nullptr) {
    send_to_char(
      "You cannot perform the required gestures while using an item as a "
      "shield!\n\r",
      ch);
    return 0;
  }

  act("$n traces a magical rune in the air with his hands.", 1, ch, nullptr,
    nullptr, TO_ROOM);
  send_to_char("You trace a rune in the air with your hands.\n\r", ch);
  return 1;
}

static int use_component(struct char_data* ch, struct obj_data* o) {
  int strength = 0;

  if (o == nullptr) {
    return 0;
  }

  strength = (ITEM_TYPE(o) == ITEM_COMPONENT) ? o->obj_flags.value[0] : 1;
  act("$n throws $p into the air... it explodes in a blast of light!", 1, ch, o,
    nullptr, TO_ROOM);
  act("You throw $p into the air... it explodes in a blast of light!", 1, ch, o,
    nullptr, TO_CHAR);
  extract_obj(o);

  return strength;
}

#define WOOD_GOLEM 26
#define ROCK_GOLEM 27
#define IRON_GOLEM 28
#define DIAMOND_GOLEM 29
#define WOOD_COMPONENT 26
#define ROCK_COMPONENT 27
#define IRON_COMPONENT 28
#define DIAMOND_COMPONENT 29

static struct obj_data* find_component(struct char_data* ch, int vnum) {
  struct obj_data* item = nullptr;

  if ((ch == nullptr) || ((item = ch->equipment[HOLD]) == nullptr)) {
    return nullptr;
  }

  if (((item->item_number >= 0) ? obj_index[item->item_number].vnum : 0) ==
      vnum) {
    return item;
  }

  if (ITEM_TYPE(item) == ITEM_SPELLBAG) {
    for (item = item->contains; item != nullptr; item = item->next_content) {
      if (((item->item_number >= 0) ? obj_index[item->item_number].vnum : 0) ==
          vnum) {
        return item;
      }
    }
  }

  return nullptr;
}

static int num_classes(struct char_data* ch) {
  int i = 0;
  int x = 0;

  for (i = 0; i < 8; i++) {
    if (GET_LEVEL(ch, i)) {
      x++;
    }
  }
  return x;
}

#define BASE_CF 10
#define BASE_CS 10

static int task_check(struct char_data* ch, int difficulty, int modifier) {
  int cf = 0;
  int cs = 0;
  int check = 0;

  if (((ch) != nullptr) && (IS_IMMORTAL(ch))) {
    return CRITICAL_SUCCESS;
  }

  modifier = MIN(modifier, 7);
  modifier = MAX(modifier, -7);

  switch (difficulty) {
    case TASK_TRIVIAL:
      cf = (BASE_CF / 3) - (modifier / 3);
      cs = (BASE_CS * 3) + (modifier * 3);
      break;
    case TASK_EASY:
      cf = (BASE_CF / 2) - (modifier / 2);
      cs = (BASE_CS * 2) + (modifier * 2);
      break;
    case TASK_DIFFICULT:
      cf = (BASE_CF * 2) - (modifier * 2);
      cs = (BASE_CS / 2) + (modifier / 2);
      break;
    case TASK_DANGEROUS:
      cf = (BASE_CF * 3) - (modifier * 3);
      cs = (BASE_CS / 3) + (modifier / 3);
      break;
    case TASK_NORMAL:
    default:
      cf = BASE_CF - modifier;
      cs = BASE_CS + modifier;
      break;
  }
  cs = cf + cs;
  check = dice(1, 100);
  if (cf >= check) {
    return CRITICAL_FAILURE;
  }
  if (cs >= check) {
    return CRITICAL_SUCCESS;
  }
  return NORMAL_RESULT;
}

static void spell_create_golem(struct char_data* ch) {
  int control = 0;
  int power = 0;
  int nc = 0;
  int modifier = 0;
  int target = 0;
  struct affected_type af{};
  struct char_data* golem = nullptr;

  if (ch == nullptr) {
    return;
  }

  if ((check_peaceful(ch,
         "The peaceful force protecting this room breaks your "
         "concentration!\n\r") != 0) ||
      (enforce_verbal(ch) == 0) || (perform_gestural(ch) == 0)) {
    send_to_char("The spell has failed!\n\r", ch);
    return;
  }

  if ((power = use_component(ch, find_component(ch, WOOD_COMPONENT))) != 0) {
    control = TASK_EASY;
    target = WOOD_GOLEM;
  } else if ((power = use_component(ch, find_component(ch, ROCK_COMPONENT))) !=
             0) {
    control = TASK_NORMAL;
    target = ROCK_GOLEM;
  } else if ((power = use_component(ch, find_component(ch, IRON_COMPONENT))) !=
             0) {
    control = TASK_DIFFICULT;
    target = IRON_GOLEM;
  } else if ((power = use_component(ch,
                find_component(ch, DIAMOND_COMPONENT))) != 0) {
    control = TASK_DANGEROUS;
    target = DIAMOND_GOLEM;
  } else {
    send_to_char("You have nothing to create the golem with!\n\r", ch);
    act("$n looks around stupidly, as if trying to find something.", 1, ch,
      nullptr, golem, TO_ROOM);
    return;
  }

  if ((golem = read_mobile(target, VIRTUAL)) == nullptr) {
    vlog("Spell 'create golem' unable to load golem [bad!]...");
    send_to_char("Unable to create the golem.  Please report this\n\r.", ch);
    return;
  }

  GET_HIT(golem) = golem->points.max_hit +=
    power; /* quality of component figures in */
  golem->points.exp = 0;

  char_to_room(golem, ch->in_room);
  act("$n arrives in a puff of blue smoke!", 1, golem, nullptr, ch, TO_ROOM);

  /* spell requires high INT and WIS  */
  modifier +=
    static_cast<int>(GET_WIS(ch) > 17) + static_cast<int>(GET_WIS(ch) > 16) +
    static_cast<int>(GET_WIS(ch) > 15) + -static_cast<int>(GET_WIS(ch) < 14) +
    -static_cast<int>(GET_WIS(ch) < 13) + -static_cast<int>(GET_WIS(ch) < 12) +
    static_cast<int>(GET_INT(ch) > 17) + static_cast<int>(GET_INT(ch) > 16) +
    static_cast<int>(GET_INT(ch) > 15) + -static_cast<int>(GET_INT(ch) < 14) +
    -static_cast<int>(GET_INT(ch) < 13) + -static_cast<int>(GET_INT(ch) < 12);

  if ((nc = num_classes(ch)) == 2) {
    modifier -= 2; /* penalty for dual-class */
    GET_HIT(golem) = golem->points.max_hit = (int)(GET_MAX_HIT(golem) * .75);
  } else if (nc == 3) {
    modifier -= 3; /* penalty for triple-class */
    GET_HIT(golem) = golem->points.max_hit = (int)(GET_MAX_HIT(golem) * .55);
  }

  if ((control = task_check(ch, control, modifier)) == CRITICAL_FAILURE) {
    act("$n loses control of the magic he has unleashed!", 1, ch, nullptr,
      golem, TO_ROOM);
    act("You lose control of the magic you have unleased!", 1, ch, nullptr,
      golem, TO_CHAR);
    hit(golem, ch, TYPE_UNDEFINED);
  } else { /* golem has permanent charm */
    if (golem->master != nullptr) {
      stop_follower(golem);
    }
    add_follower(golem, ch);
    af.type = SPELL_CHARM_PERSON;
    af.duration = 24 * 365;
    af.location = af.modifier = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(golem, &af);
    if (control == CRITICAL_SUCCESS) {
      act("$n beams with pride.  $N flexes.", 1, ch, nullptr, golem, TO_ROOM);
      send_to_char("You have created an unusually strong golem!\n\r", ch);
      GET_HIT(golem) = golem->points.max_hit = (int)(GET_MAX_HIT(golem) * 1.5);
    }
    if (!IS_SET(golem->specials.act, ACT_SENTINEL)) {
      SET_BIT(golem->specials.act, ACT_SENTINEL);
    }
  }
}

void cast_create_golem(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*victim*/,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_create_golem(ch);
      break;
    default:
      vlog("Create golem called in a non-spell way (not defined!)");
      break;
  }
}

struct PolyType {
    char name[20];
    int level;
    int number;
};

const struct PolyType poly_list[40] = {{"goblin", 4, 201}, {"parrot", 4, 9001},
  {"frog", 4, 215}, {"gnoll", 6, 211}, {"parrot", 6, 9010}, {"lizard", 6, 224},
  {"ogre", 8, 4113}, {"parrot", 8, 9011}, {"wolf", 8, 3094}, {"spider", 9, 227},
  {"beast", 9, 242}, {"minotaur", 9, 247}, {"snake", 10, 249},
  {"bull", 10, 1008}, {"warg", 10, 6100}, {"sapling", 12, 1421},
  {"ogre-maji", 12, 257}, {"black", 12, 230}, {"troll", 14, 4101},
  {"crocodile", 14, 259}, {"mindflayer", 14, 7202}, {"giant", 16, 261},
  {"bear", 16, 9024}, {"blue", 16, 233}, {"enfan", 18, 21001},
  {"lamia", 18, 5201}, {"drider", 18, 5011}, {"wyvern", 20, 3415},
  {"mindflayer", 20, 7201}, {"spider", 20, 20010}, {"snog", 22, 27008},
  {"roc", 22, 3724}, {"giant", 24, 9406}, {"white", 26, 243},
  {"master", 28, 7200}, {"mulichort", 35, 15830}, {"beholder", 40, 5200}};

#define LAST_POLY_MOB 36

/*
 **   requires the sacrifice of 150k coins, victim loses a con point, and
 **   caster is knocked down to 1 hp, 1 mp, 1 mana, and sits for a LONG
 **   time (if a pc)
 */

static void spell_resurrection(struct char_data* ch, struct char_data* victim,
  struct obj_data* obj) {
  struct affected_type af{};
  struct obj_data* obj_object = nullptr;
  struct obj_data* next_obj = nullptr;

  if (obj == nullptr) {
    return;
  }

  if (IS_CORPSE(obj)) {
    if (obj->char_vnum != 0) { /* corpse is a npc */
      if (GET_GOLD(ch) < 10000) {
        send_to_char("The gods are not happy with your sacrifice.\n\r", ch);
        return;
      }
      GET_GOLD(ch) -= 10000;

      victim = read_mobile(obj->char_vnum, VIRTUAL);
      if (IS_SET(victim->specials.act, ACT_IMMORTAL)) {
        send_to_char("You can't seem to control the magic.\n\r", ch);
        extract_char(victim);
        return;
      }

      if ((IsImmune(victim, IMM_CHARM) == 0) ||
          (IsResist(victim, IMM_CHARM) == 0)) {
        char_to_room(victim, ch->in_room);
        GET_GOLD(victim) = 0;
        GET_EXP(victim) = 0;
        GET_HIT(victim) = 1;
        GET_POS(victim) = POSITION_STUNNED;
      } else {
        send_to_char(
          "You just don't have the power to resurrect that corpse.\n\r", ch);
        extract_char(victim);
        return;
      }

      act("With mystic power, $n resurrects a corpse.", 1, ch, nullptr, nullptr,
        TO_ROOM);
      act("$N slowly rises from the ground.", 0, ch, nullptr, victim, TO_ROOM);

      /*
   should be charmed and follower ch
   */

      if ((IsImmune(victim, IMM_CHARM) != 0) ||
          (IsResist(victim, IMM_CHARM) != 0)) {
        act("$N says 'Thank you'", 0, ch, nullptr, victim, TO_ROOM);
      } else {
        af.type = SPELL_CHARM_PERSON;
        af.duration = 36;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_CHARM;

        affect_to_char(victim, &af);

        add_follower(victim, ch);
      }

      IS_CARRYING_W(victim) = 0;
      IS_CARRYING_N(victim) = 0;

      /*
   take all from corpse, and give to person
   */

      for (obj_object = obj->contains; obj_object != nullptr;
        obj_object = next_obj) {
        next_obj = obj_object->next_content;
        obj_from_obj(obj_object);
        obj_to_char(obj_object, victim);
      }

      /*
   get rid of corpse
   */
      extract_obj(obj);

    } else { /* corpse is a pc  */
      send_to_char("You can't resurrect players anymore, sorry./n/r", ch);
    }
  }
}

void cast_resurrection(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*tar_ch*/,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (tar_obj == nullptr) {
        return;
      }
      spell_resurrection(ch, nullptr, tar_obj);
      break;
    case SPELL_TYPE_STAFF:
      if (tar_obj == nullptr) {
        return;
      }
      spell_resurrection(ch, nullptr, tar_obj);
      break;
    default:
      vlog("Serious problem in 'resurrection'");
      break;
  }
}

static void spell_track(signed char level, struct char_data* ch,
  struct char_data* targ, int obj) {
  struct affected_type af{};

  if (ch != targ) {
    send_to_char("You feel your awareness grow!\n\r", targ);
  } else {
    send_to_char("You feel your awareness grow!\n\r", ch);
  }

  act("$N's eyes take on an emerald hue for just a moment.", 0, ch, nullptr,
    targ, TO_ROOM);

  if (obj == 0) {
    af.type = SPELL_MINOR_TRACK;
    af.duration = level;
  } else {
    af.type = SPELL_MAJOR_TRACK;
    af.duration = level * 2;
  }

  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = 0;
  affect_to_char(targ, &af);
}

void cast_major_track(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_track(level, ch, tar_ch, 1);
      break;
    case SPELL_TYPE_POTION:
      spell_track(level, ch, ch, 1);
      break;
    case SPELL_TYPE_WAND:
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_track(level, ch, tar_ch, 1);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_track(level, ch, tar_ch, 1);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_track(level, ch, tar_ch, 1);
        }
      }
      break;
    default:
      vlog("Serious problem in 'track'");
      break;
  }
}

void cast_minor_track(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_track(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_track(level, ch, ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_track(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_track(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_track(level, ch, tar_ch, 0);
        }
      }
      break;
    default:
      vlog("Serious problem in 'track'");
      break;
  }
}

static void spell_mana(signed char level, struct char_data* ch) {
  int dam = 0;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(level, 4);
  dam = MAX(dam, level * 2);

  if (GET_MANA(ch) + dam > GET_MAX_MANA(ch)) {
    GET_MANA(ch) = GET_MAX_MANA(ch);
  } else {
    GET_MANA(ch) += dam;
  }
}

void cast_mana(signed char level, struct char_data* ch, const char* /*arg*/,
  int type, struct char_data* tar_ch, struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_POTION:
      spell_mana(level, ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_mana(level, ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_mana(level, ch);
        }
      }
      [[fallthrough]];
    default:
      vlog("Serious problem in 'mana'");
      break;
  }
}

static void spell_armor(signed char level, struct char_data* ch,
  struct char_data* victim) {
  struct affected_type af{};

  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  if (affected_by_spell(victim, SPELL_ARMOR) == 0) {
    af.type = SPELL_ARMOR;
    af.duration = 24;
    af.modifier = -20;
    af.location = APPLY_AC;
    af.bitvector = 0;

    affect_to_char(victim, &af);
    send_to_char("You feel someone protecting you.\n\r", victim);
  } else {
    send_to_char("Nothing New seems to happen\n\r", ch);
  }
}

void cast_armor(signed char level, struct char_data* ch, const char* /*arg*/,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (affected_by_spell(tar_ch, SPELL_ARMOR) != 0) {
        send_to_char("Nothing seems to happen.\n\r", ch);
        return;
      }
      if (ch != tar_ch) {
        act("$N is protected.", 0, ch, nullptr, tar_ch, TO_CHAR);
      }

      spell_armor(level, ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      if (affected_by_spell(ch, SPELL_ARMOR) != 0) {
        return;
      }
      spell_armor(level, ch, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      if (affected_by_spell(tar_ch, SPELL_ARMOR) != 0) {
        return;
      }
      spell_armor(level, ch, ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj != nullptr) {
        return;
      }
      if (affected_by_spell(tar_ch, SPELL_ARMOR) != 0) {
        return;
      }
      spell_armor(level, ch, ch);
      break;
    default:
      vlog("Serious screw-up in armor!");
      break;
  }
}

static void spell_stone_skin(signed char level, struct char_data* ch) {
  struct affected_type af{};

  assert(ch);

  if (affected_by_spell(ch, SPELL_STONE_SKIN) == 0) {
    act("$n's skin turns grey and granite-like.", 1, ch, nullptr, nullptr,
      TO_ROOM);
    act("Your skin turns to a stone-like substance.", 1, ch, nullptr, nullptr,
      TO_CHAR);

    af.type = SPELL_STONE_SKIN;
    af.duration = level;
    af.modifier = -40;
    af.location = APPLY_AC;
    af.bitvector = 0;
    affect_to_char(ch, &af);

    /* resistance to piercing weapons */

    af.type = SPELL_STONE_SKIN;
    af.duration = level;
    af.modifier = 32;
    af.location = APPLY_IMMUNE;
    af.bitvector = 0;
    affect_to_char(ch, &af);
  }
}

void cast_stone_skin(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*tar_ch*/,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (affected_by_spell(ch, SPELL_STONE_SKIN) != 0) {
        send_to_char("Nothing seems to happen.\n\r", ch);
        return;
      }
      spell_stone_skin(level, ch);
      break;
    case SPELL_TYPE_POTION:
      if (affected_by_spell(ch, SPELL_STONE_SKIN) != 0) {
        return;
      }
      spell_stone_skin(level, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (affected_by_spell(ch, SPELL_STONE_SKIN) != 0) {
        return;
      }
      spell_stone_skin(level, ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj != nullptr) {
        return;
      }
      if (affected_by_spell(ch, SPELL_STONE_SKIN) != 0) {
        return;
      }
      spell_stone_skin(level, ch);
      break;
    default:
      vlog("Serious screw-up in stone_skin!");
      break;
  }
}

static void spell_vitalize_mana(struct char_data* ch) {
  send_to_char("Vitalize mana has been disabled.\n\r", ch);
}

void cast_vitalize_mana(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_vitalize_mana(ch);
      break;
    case SPELL_TYPE_POTION:
      spell_vitalize_mana(ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = world[ch->in_room].people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_vitalize_mana(ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in vitalize mana!");
      break;
  }
}

void spell_astral_walk(signed char /*level*/, struct char_data* ch,
  struct char_data* victim, struct obj_data* /*obj*/) {
  int location = 0;
  struct room_data* rp = nullptr;

  assert(ch && victim);

  location = victim->in_room;
  rp = real_roomp(location);

  if (GetMaxLevel(victim) > MAX_MORT || (rp == nullptr) ||
      IS_SET(rp->room_flags, PRIVATE) || IS_SET(rp->room_flags, NO_SUM) ||
      IS_SET(rp->room_flags, HAVE_TO_WALK) || IS_NPC(victim) ||
      IS_SET(rp->room_flags, NO_MAGIC)) {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  if (dice(1, 8) == 8) {
    send_to_char("You failed.\n\r", ch);
    return;
  }
  act("$n opens a door to another dimension and steps through!", 0, ch, nullptr,
    nullptr, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, location);
  act("You are blinded for a moment as $n appears in a flash of light!", 0, ch,
    nullptr, nullptr, TO_ROOM);
  do_look(ch, "", 15);
}

void cast_astral_walk(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SPELL:

      if (tar_ch == nullptr) {
        send_to_char("Yes, but who do you wish to walk to?\n", ch);
      } else {
        spell_astral_walk(level, ch, tar_ch, nullptr);
      }
      break;

    default:
      vlog("Serious screw-up in astral walk!");
      break;
  }
}

static void spell_farlook(struct char_data* ch, struct char_data* victim) {
  short int target = 0;
  struct char_data* tmpv = nullptr;
  struct char_data* temp = nullptr;
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH];

  target = real_roomp(victim->in_room)->number;

  if (GetMaxLevel(victim) > 50) {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  strcpy(buf,
    "You conjure up a large cloud which shimmers, and\n\r"
    "then ... turns transparent ans shows ... \n\r");
  strcpy(buf1,
    "$n conjures up a large cloud which shimmers, and\n\r"
    "then ... turns transparent and shows ...\n\r");
  act(buf, 0, ch, nullptr, nullptr, TO_CHAR);
  act(buf1, 0, ch, nullptr, nullptr, TO_ROOM);

  sprintf(buf1, "%d look", target);
  for (tmpv = character_list; tmpv != nullptr; tmpv = temp) {
    temp = tmpv->next;
    if ((ch->in_room == tmpv->in_room) && !IS_NPC(tmpv)) {
      do_at(tmpv, buf1, 0);
    }
  }
}

void cast_farlook(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_WAND:

      if (tar_ch == nullptr) {
        send_to_char("Yes, but who do you wish to base your farlook on?\n", ch);
      } else {
        spell_farlook(ch, tar_ch);
      }
      break;

    default:
      vlog("Serious fuck up in farlook!");
      break;
  }
}

static void spell_portal(struct char_data* ch, struct char_data* victim) {
  char buf[30000];
  struct obj_data* tmp_obj = nullptr;
  struct obj_data* next_tmp_obj = nullptr;
  struct room_data* rp = nullptr;

  assert(ch && victim);

  rp = real_roomp(victim->in_room);

  if ((!IS_NPC(victim)) && (GetMaxLevel(victim) > MAX_MORT)) {
    send_to_char("Portalling to a God could be hazardous to your health.\n\r",
      ch);
    return;
  }

  if (GetMaxLevel(victim) >= 50 || (rp == nullptr) ||
      IS_SET(rp->room_flags, NO_SUM) || IS_SET(rp->room_flags, HAVE_TO_WALK) ||
      IS_SET(rp->room_flags, NO_MAGIC)) {
    send_to_char("You can't seem to penetrate the defenses of that area.\n\r",
      ch);
    return;
  }

  CREATE(tmp_obj, struct obj_data, 1);
  clear_object(tmp_obj);

  tmp_obj->name = strdup("portal");
  tmp_obj->short_description = strdup("A Magic Portal");
  sprintf(buf, "A portal going to %s is in the room.",
    real_roomp(victim->in_room)->name);
  tmp_obj->description = strdup(buf);
  tmp_obj->obj_flags.type_flag = ITEM_TRASH;
  tmp_obj->obj_flags.wear_flags = 0;
  tmp_obj->obj_flags.decay_time = 5;
  tmp_obj->obj_flags.weight = 0;
  tmp_obj->obj_flags.cost = 1;
  tmp_obj->obj_flags.cost_per_day = 1;
  tmp_obj->obj_flags.value[0] = victim->in_room;

  tmp_obj->next = object_list;
  object_list = tmp_obj;

  obj_to_room(tmp_obj, ch->in_room);
  tmp_obj->item_number = -1;
  CREATE(next_tmp_obj, struct obj_data, 1);
  clear_object(next_tmp_obj);
  next_tmp_obj->name = strdup("portal");
  next_tmp_obj->short_description = strdup("A Magic Portal");
  sprintf(buf, "A portal going to %s is in the room.",
    real_roomp(ch->in_room)->name);
  next_tmp_obj->description = strdup(buf);
  next_tmp_obj->obj_flags.type_flag = ITEM_TRASH;
  next_tmp_obj->obj_flags.wear_flags = 0;
  next_tmp_obj->obj_flags.decay_time = 5;
  next_tmp_obj->obj_flags.weight = 1;
  next_tmp_obj->obj_flags.cost = 1;
  next_tmp_obj->obj_flags.cost_per_day = 0;
  next_tmp_obj->obj_flags.value[0] = ch->in_room;

  next_tmp_obj->next = object_list;
  object_list = next_tmp_obj;

  obj_to_room(next_tmp_obj, victim->in_room);

  next_tmp_obj->item_number = -1;
  act("$p suddenly appears out of a swirling mist.", 1, ch, tmp_obj, nullptr,
    TO_ROOM);
  act("$p suddenly appears out of a swirling mist.", 1, ch, tmp_obj, nullptr,
    TO_CHAR);
}

void cast_portal(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:

      if (tar_ch == nullptr) {
        send_to_char("Yes, but who do you wish to portal to?\n", ch);
      } else {
        spell_portal(ch, tar_ch);
      }
      break;

    default:
      vlog("Serious screw-up in portal!");
      break;
  }
}

void spell_teleport(signed char /*level*/, struct char_data* ch,
  struct char_data* victim, struct obj_data* /*obj*/) {
  int to_room = 0;
  struct room_data* room = nullptr;

  assert(ch && victim);

  if (victim != ch) {
    if (saves_spell(victim, SAVING_SPELL) != 0) {
      send_to_char("Your spell has no effect.\n\r", ch);
      if (IS_NPC(victim)) {
        if (victim->specials.fighting == nullptr) {
          set_fighting(victim, ch);
        }
      } else {
        send_to_char("You feel strange, but the effect fades.\n\r", victim);
      }
      return;
    }
    ch = victim; /* the character (target) is now the victim */
  }

  do {
    to_room = number(0, top_of_world);
    room = real_roomp(to_room);
    if (room != nullptr) {
      if (IS_SET(room->room_flags, PRIVATE)) {
        room = nullptr;
      }
    }

  } while (room == nullptr);

  act("$n slowly fade out of existence.", 0, ch, nullptr, nullptr, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, to_room);
  act("$n slowly fade in to existence.", 0, ch, nullptr, nullptr, TO_ROOM);

  do_look(ch, "", 0);

  if (IS_SET(real_roomp(to_room)->room_flags, DEATH) &&
      GetMaxLevel(ch) < LOW_IMMORTAL) {
    death_cry(ch);
    zero_rent(ch);
    extract_char(ch);
  }
}

void cast_teleport(signed char level, struct char_data* ch, const char* /*arg*/,
  int type, struct char_data* tar_ch, struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SPELL:
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_teleport(level, ch, tar_ch, nullptr);
      break;

    case SPELL_TYPE_WAND:
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_teleport(level, ch, tar_ch, nullptr);
      break;

    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_teleport(level, ch, tar_ch, nullptr);
        }
      }
      break;

    default:
      vlog("Serious screw-up in teleport!");
      break;
  }
}

static void spell_bless(signed char level, struct char_data* ch,
  struct char_data* victim, struct obj_data* obj) {
  struct affected_type af{};

  assert(ch && (victim || obj));
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  if (obj != nullptr) {
    if ((5 * GET_LEVEL(ch, CLERIC_LEVEL_IND) > GET_OBJ_WEIGHT(obj)) &&
        (GET_POS(ch) != POSITION_FIGHTING) &&
        !IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)) {
      SET_BIT(obj->obj_flags.extra_flags, ITEM_BLESS);
      act("$p briefly glows.", 0, ch, obj, nullptr, TO_CHAR);
    }
  } else {
    if ((GET_POS(victim) != POSITION_FIGHTING) &&
        (affected_by_spell(victim, SPELL_BLESS) == 0)) {
      send_to_char("You feel righteous.\n\r", victim);
      af.type = SPELL_BLESS;
      af.duration = 6;
      af.modifier = 1;
      af.location = APPLY_HITROLL;
      af.bitvector = 0;
      affect_to_char(victim, &af);

      af.location = APPLY_SAVING_SPELL;
      af.modifier = -1; /* Make better */
      affect_to_char(victim, &af);
    }
  }
}

void cast_bless(signed char level, struct char_data* ch, const char* /*arg*/,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (tar_obj != nullptr) { /* It's an object */
        if (IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS)) {
          send_to_char("Nothing seems to happen.\n\r", ch);
          return;
        }
        spell_bless(level, ch, nullptr, tar_obj);

      } else { /* Then it is a PC | NPC */

        if ((affected_by_spell(tar_ch, SPELL_BLESS) != 0) ||
            (GET_POS(tar_ch) == POSITION_FIGHTING)) {
          send_to_char("Nothing seems to happen.\n\r", ch);
          return;
        }
        spell_bless(level, ch, tar_ch, nullptr);
      }
      break;
    case SPELL_TYPE_POTION:
      if ((affected_by_spell(ch, SPELL_BLESS) != 0) ||
          (GET_POS(ch) == POSITION_FIGHTING)) {
        return;
      }
      spell_bless(level, ch, ch, nullptr);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) { /* It's an object */
        if (IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS)) {
          return;
        }
        spell_bless(level, ch, nullptr, tar_obj);

      } else { /* Then it is a PC | NPC */

        if (tar_ch == nullptr) {
          tar_ch = ch;
        }

        if ((affected_by_spell(tar_ch, SPELL_BLESS) != 0) ||
            (GET_POS(tar_ch) == POSITION_FIGHTING)) {
          return;
        }
        spell_bless(level, ch, tar_ch, nullptr);
      }
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj != nullptr) { /* It's an object */
        if (IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS)) {
          return;
        }
        spell_bless(level, ch, nullptr, tar_obj);

      } else { /* Then it is a PC | NPC */

        if ((affected_by_spell(tar_ch, SPELL_BLESS) != 0) ||
            (GET_POS(tar_ch) == POSITION_FIGHTING)) {
          return;
        }
        spell_bless(level, ch, tar_ch, nullptr);
      }
      break;
    default:
      vlog("Serious screw-up in bless!");
      break;
  }
}

static void spell_infravision(signed char level, struct char_data* ch,
  struct char_data* victim) {
  struct affected_type af{};

  assert(victim && ch);

  if (!IS_AFFECTED(victim, AFF_INFRAVISION)) {
    if (ch != victim) {
      send_to_char("Your eyes glow red.\n\r", victim);
      act("$n's eyes glow red.\n\r", 0, victim, nullptr, nullptr, TO_ROOM);
    } else {
      send_to_char("Your eyes glow red.\n\r", ch);
      act("$n's eyes glow red.\n\r", 0, ch, nullptr, nullptr, TO_ROOM);
    }

    af.type = SPELL_INFRAVISION;
    af.duration = 4 * level;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_INFRAVISION;
    affect_to_char(victim, &af);
  }
}

void cast_infravision(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (IS_AFFECTED(tar_ch, AFF_INFRAVISION)) {
        send_to_char("Nothing seems to happen.\n\r", ch);
        return;
      }
      spell_infravision(level, ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      if (IS_AFFECTED(ch, AFF_INFRAVISION)) {
        return;
      }
      spell_infravision(level, ch, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      if (IS_AFFECTED(tar_ch, AFF_INFRAVISION)) {
        return;
      }
      spell_infravision(level, ch, tar_ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj != nullptr) {
        return;
      }
      if (IS_AFFECTED(tar_ch, AFF_INFRAVISION)) {
        return;
      }
      spell_infravision(level, ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          if (!(IS_AFFECTED(tar_ch, AFF_INFRAVISION))) {
            spell_infravision(level, ch, tar_ch);
          }
        }
      }
      break;
    default:
      vlog("Serious screw-up in infravision!");
      break;
  }
}

static void spell_true_seeing(signed char level, struct char_data* ch,
  struct char_data* victim) {
  struct affected_type af{};

  assert(victim && ch);

  if (!IS_AFFECTED(victim, AFF_TRUE_SIGHT)) {
    if (ch != victim) {
      send_to_char("Your eyes glow silver for a moment.\n\r", victim);
      act("$n's eyes take on a silvery hue.\n\r", 0, victim, nullptr, nullptr,
        TO_ROOM);
    } else {
      send_to_char("Your eyes glow silver.\n\r", ch);
      act("$n's eyes glow silver.\n\r", 0, ch, nullptr, nullptr, TO_ROOM);
    }

    af.type = SPELL_TRUE_SIGHT;
    af.duration = 2 * level;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_TRUE_SIGHT;
    affect_to_char(victim, &af);
  } else {
    send_to_char("Nothing seems to happen\n\r", ch);
  }
}

void cast_true_seeing(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT)) {
        send_to_char("Nothing seems to happen.\n\r", ch);
        return;
      }
      spell_true_seeing(level, ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      if (IS_AFFECTED(ch, AFF_TRUE_SIGHT)) {
        return;
      }
      spell_true_seeing(level, ch, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      if (IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT)) {
        return;
      }
      spell_true_seeing(level, ch, tar_ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj != nullptr) {
        return;
      }
      if (IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT)) {
        return;
      }
      spell_true_seeing(level, ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          if (!(IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT))) {
            spell_true_seeing(level, ch, tar_ch);
          }
        }
      }
      break;
    default:
      vlog("Serious screw-up in true_seeing!");
      break;
  }
}

static void spell_blindness(signed char level, struct char_data* ch,
  struct char_data* victim) {
  struct affected_type af{};

  assert(ch && victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  if ((saves_spell(victim, SAVING_SPELL) != 0) ||
      (affected_by_spell(victim, SPELL_BLINDNESS) != 0)) {
    return;
  }

  act("$n seems to be blinded!", 1, victim, nullptr, nullptr, TO_ROOM);
  send_to_char("You have been blinded!\n\r", victim);

  af.type = SPELL_BLINDNESS;
  af.location = APPLY_HITROLL;
  af.modifier = -4; /* Make hitroll worse */
  af.duration = level / 2;
  af.bitvector = AFF_BLIND;
  affect_to_char(victim, &af);

  af.location = APPLY_AC;
  af.modifier = +20; /* Make AC Worse! */
  affect_to_char(victim, &af);

  if ((victim->specials.fighting == nullptr) && (victim != ch)) {
    set_fighting(victim, ch);
  }
}

void cast_blindness(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (IS_AFFECTED(tar_ch, AFF_BLIND)) {
        send_to_char("Nothing seems to happen.\n\r", ch);
        return;
      }
      spell_blindness(level, ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      if (IS_AFFECTED(ch, AFF_BLIND)) {
        return;
      }
      spell_blindness(level, ch, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      if (IS_AFFECTED(tar_ch, AFF_BLIND)) {
        return;
      }
      spell_blindness(level, ch, tar_ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      if (IS_AFFECTED(tar_ch, AFF_BLIND)) {
        return;
      }
      spell_blindness(level, ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (in_group(ch, tar_ch) == 0) {
          if (!(IS_AFFECTED(tar_ch, AFF_BLIND))) {
            spell_blindness(level, ch, tar_ch);
          }
        }
      }
      break;
    default:
      vlog("Serious screw-up in blindness!");
      break;
  }
}

static void spell_light(signed char level, struct char_data* ch) {
  /*
     creates a ball of light in the hands.
  */
  struct obj_data* tmp_obj = nullptr;

  assert(ch);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  tmp_obj = read_object(20, VIRTUAL); /* this is all you have to do */
  if (tmp_obj != nullptr) {
    tmp_obj->obj_flags.value[2] = 24 + level;
    obj_to_char(tmp_obj, ch);
  } else {
    send_to_char("Sorry, I can't create the ball of light\n\r", ch);
    return;
  }

  act("$n twiddles $s thumbs and $p suddenly appears.", 1, ch, tmp_obj, nullptr,
    TO_ROOM);
  act("You twiddle your thumbs and $p suddenly appears.", 1, ch, tmp_obj,
    nullptr, TO_CHAR);
}

void cast_light(signed char level, struct char_data* ch, const char* /*arg*/,
  int type, struct char_data* /*tar_ch*/, struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_light(level, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      spell_light(level, ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj != nullptr) {
        return;
      }
      spell_light(level, ch);
      break;
    default:
      vlog("Serious screw-up in light!");
      break;
  }
}

static void spell_cont_light(signed char level, struct char_data* ch) {
  struct obj_data* tmp_obj = nullptr;

  assert(ch);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  tmp_obj = read_object(20, VIRTUAL);
  if (tmp_obj != nullptr) {
    obj_to_char(tmp_obj, ch);
  } else {
    send_to_char("Sorry, I can't create the ball of light\n\r", ch);
    return;
  }

  act("$n twiddles $s thumbs and $p suddenly appears.", 1, ch, tmp_obj, nullptr,
    TO_ROOM);
  act("You twiddle your thumbs and $p suddenly appears.", 1, ch, tmp_obj,
    nullptr, TO_CHAR);
}

void cast_cont_light(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*tar_ch*/,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cont_light(level, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      spell_cont_light(level, ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj != nullptr) {
        return;
      }
      spell_cont_light(level, ch);
      break;
    default:
      vlog("Serious screw-up in continual light!");
      break;
  }
}

/* removes aggressive bit from monsters */
static void spell_calm(struct char_data* ch, struct char_data* victim) {
  assert(ch && victim);

  const char is_aggressive =
    static_cast<const char>(IS_SET(victim->specials.act, ACT_AGGRESSIVE));

  if (IS_PC(victim) || (is_aggressive == 0) ||
      (saves_spell(victim, SAVING_PARA) == 0)) {
    if (is_aggressive != 0) {
      REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
    }

    send_to_char("You feel calm.\n\r", victim);
    return;
  }

  send_to_char("You feel happy and easygoing, but the effect soon fades.\n\r",
    victim);

  if (victim->specials.fighting == nullptr) {
    set_fighting(victim, ch);
  }
}

void cast_calm(signed char /*level*/, struct char_data* ch, const char* /*arg*/,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_calm(ch, tar_ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_calm(ch, tar_ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_calm(ch, ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        spell_calm(ch, tar_ch);
      }
      break;
    default:
      vlog("Serious screw-up in continual light!");
      break;
  }
}

static void spell_web(signed char level, struct char_data* ch,
  struct char_data* victim) {
  struct affected_type af{};

  assert(ch && victim);

  af.type = SPELL_WEB;
  af.location = 0;
  af.modifier = level;
  af.duration = level;
  af.bitvector = 0;
  affect_to_char(victim, &af);

  af.location = APPLY_HITROLL;
  af.modifier = -level / 5;
  affect_to_char(victim, &af);

  af.location = APPLY_AC;
  af.modifier = 5;
  affect_to_char(victim, &af);
}

void cast_web(signed char level, struct char_data* ch, const char* /*arg*/,
  int /*type*/, struct char_data* tar_ch, struct obj_data* /*tar_obj*/) {
  if (affected_by_spell(tar_ch, SPELL_WEB) != 0) {
    return;
  }
  spell_web(level, ch, tar_ch);
}

static void spell_clone(signed char level, struct char_data* ch,
  struct char_data* victim, struct obj_data* obj) {
  assert(ch && (victim || obj));
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  send_to_char("Clone is not ready yet.", ch);

  if (obj != nullptr) {
  } else {
    /* clone_char(victim); */
  }
}

void cast_clone(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int /*type*/, struct char_data* /*tar_ch*/,
  struct obj_data* /*tar_obj*/) {
  send_to_char("Not *YET* implemented.", ch);

  /* clone both char and obj !!*/

  /*
    switch (type) {
      case SPELL_TYPE_SPELL:
        if (tar_ch) {
          sprintf(buf, "You create a duplicate of %s.\n\r", GET_NAME(tar_ch));
          send_to_char(buf, ch);
          sprintf(buf, "%%s creates a duplicate of %s,\n\r", GET_NAME(tar_ch));
          perform(buf, ch, false);

          spell_clone(level,ch,tar_ch,0);
        } else {
          sprintf(buf, "You create a duplicate of %s
    %s.\n\r",SANA(tar_obj),tar_obj->short_description); send_to_char(buf, ch);
          sprintf(buf, "%%s creates a duplicate of %s
    %s,\n\r",SANA(tar_obj),tar_obj->short_description); perform(buf, ch, false);

          spell_clone(level,ch,0,tar_obj);
        };
        break;


      default :
           vlog("Serious screw-up in clone!");
           break;
    }
  */
  /* MISSING REST OF SWITCH -- POTION, SCROLL, WAND */
}

void cast_control_weather(signed char /*level*/, struct char_data* ch,
  const char* arg, int type, struct char_data* /*tar_ch*/,
  struct obj_data* /*tar_obj*/) {
  char buffer[MAX_STRING_LENGTH];

  switch (type) {
    case SPELL_TYPE_SPELL:

      one_argument(arg, buffer);

      if ((str_cmp("better", buffer) != 0) && (str_cmp("worse", buffer) != 0)) {
        send_to_char("Do you want it to get better or worse?\n\r", ch);
        return;
      }
      if (!OUTSIDE(ch)) {
        send_to_char("You need to be outside.\n\r", ch);
      }

      if (str_cmp("better", buffer) == 0) {
        if (weather_info.sky == SKY_CLOUDLESS) {
          return;
        }
        if (weather_info.sky == SKY_CLOUDY) {
          send_to_outdoor("The clouds disappear.\n\r");
          weather_info.sky = SKY_CLOUDLESS;
        }
        if (weather_info.sky == SKY_RAINING) {
          if ((time_info.month > 3) && (time_info.month < 14)) {
            send_to_outdoor("The rain has stopped.\n\r");
          } else {
            send_to_outdoor("The snow has stopped. \n\r");
          }
          weather_info.sky = SKY_CLOUDY;
        }
        if (weather_info.sky == SKY_LIGHTNING) {
          if ((time_info.month > 3) && (time_info.month < 14)) {
            send_to_outdoor(
              "The lightning has gone, but it is still raining.\n\r");
          } else {
            send_to_outdoor(
              "The blizzard is over, but it is still snowing.\n\r");
          }
          weather_info.sky = SKY_RAINING;
        }
        return;
      }
      if (weather_info.sky == SKY_CLOUDLESS) {
        send_to_outdoor("The sky is getting cloudy.\n\r");
        weather_info.sky = SKY_CLOUDY;
        return;
      }
      if (weather_info.sky == SKY_CLOUDY) {
        if ((time_info.month > 3) && (time_info.month < 14)) {
          send_to_outdoor("It starts to rain.\n\r");
        } else {
          send_to_outdoor("It starts to snow. \n\r");
        }
        weather_info.sky = SKY_RAINING;
      }
      if (weather_info.sky == SKY_RAINING) {
        if ((time_info.month > 3) && (time_info.month < 14)) {
          send_to_outdoor("You are caught in lightning storm.\n\r");
        } else {
          send_to_outdoor("You are caught in a blizzard. \n\r");
        }
        weather_info.sky = SKY_LIGHTNING;
      }
      if (weather_info.sky == SKY_LIGHTNING) {
        return;
      }

      return;

      break;

    default:
      vlog("Serious screw-up in control weather!");
      break;
  }
}

static void spell_create_food(signed char level, struct char_data* ch) {
  int foodnum = 0;
  struct obj_data* tmp_obj = nullptr;

  assert(ch);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  foodnum = number(1, 10);

  CREATE(tmp_obj, struct obj_data, 1);
  clear_object(tmp_obj);

  if (foodnum == 1) {
    tmp_obj->name = strdup("piece cake");
    tmp_obj->short_description = strdup("A piece of strawberry cake");
    tmp_obj->description =
      strdup("A scrumptous looking piece of cake lies here.");
  } else {
    tmp_obj->name = strdup("mushroom");
    tmp_obj->short_description = strdup("A Magic Mushroom");
    tmp_obj->description =
      strdup("A really delicious looking magic mushroom lies here.");
  }

  tmp_obj->obj_flags.type_flag = ITEM_FOOD;
  tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
  tmp_obj->obj_flags.value[0] = 5 + level;
  tmp_obj->obj_flags.weight = 1;
  tmp_obj->obj_flags.cost = 10;
  tmp_obj->obj_flags.decay_time = 100;
  tmp_obj->obj_flags.cost_per_day = 1;

  tmp_obj->next = object_list;
  object_list = tmp_obj;

  obj_to_room(tmp_obj, ch->in_room);

  tmp_obj->item_number = -1;

  act("$p suddenly appears.", 1, ch, tmp_obj, nullptr, TO_ROOM);
  act("$p suddenly appears.", 1, ch, tmp_obj, nullptr, TO_CHAR);
}

void cast_create_food(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      act("$n magically creates a mushroom.", 0, ch, nullptr, nullptr, TO_ROOM);
      spell_create_food(level, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch != nullptr) {
        return;
      }
      spell_create_food(level, ch);
      break;
    default:
      vlog("Serious screw-up in create food!");
      break;
  }
}

static void spell_create_water(signed char level, struct char_data* ch,
  struct obj_data* obj) {
  assert(ch && obj);

  int water = 0;

  if (GET_ITEM_TYPE(obj) == ITEM_DRINKCON) {
    if ((obj->obj_flags.value[2] != LIQ_WATER) &&
        (obj->obj_flags.value[1] != 0)) {
      name_from_drinkcon(obj);
      obj->obj_flags.value[2] = LIQ_SLIME;
      name_to_drinkcon(obj, LIQ_SLIME);

    } else {
      water = 2 * level * ((weather_info.sky >= SKY_RAINING) ? 2 : 1);

      /* Calculate water it can contain, or water created */
      water = MIN(obj->obj_flags.value[0] - obj->obj_flags.value[1], water);

      if (water > 0) {
        obj->obj_flags.value[2] = LIQ_WATER;
        obj->obj_flags.value[1] += water;

        weight_change_object(obj, water);

        name_from_drinkcon(obj);
        name_to_drinkcon(obj, LIQ_WATER);
        act("$p is partially filled.", 0, ch, obj, nullptr, TO_CHAR);
      }
    }
  }
}

void cast_create_water(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*tar_ch*/,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (tar_obj->obj_flags.type_flag != ITEM_DRINKCON) {
        send_to_char("It is unable to hold water.\n\r", ch);
        return;
      }
      spell_create_water(level, ch, tar_obj);
      break;
    default:
      vlog("Serious screw-up in create water!");
      break;
  }
}

static void spell_water_breath(struct char_data* ch, struct char_data* victim) {
  struct affected_type af{};

  assert(ch && victim);

  act("You feel fishy!", 1, ch, nullptr, victim, TO_VICT);
  if (victim != ch) {
    act("$N makes a face like a fish.", 1, ch, nullptr, victim, TO_CHAR);
  }
  act("$N makes a face like a fish.", 1, ch, nullptr, victim, TO_NOTVICT);

  af.type = SPELL_WATER_BREATH;
  af.duration = 6;
  af.modifier = 0;
  af.location = 0;
  af.bitvector = AFF_WATERBREATH;
  affect_to_char(victim, &af);
}

void cast_water_breath(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_water_breath(ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_water_breath(ch, tar_ch);
      break;
    case SPELL_TYPE_WAND:
      spell_water_breath(ch, tar_ch);
      break;

    default:
      vlog("Serious screw-up in water breath");
      break;
  }
}

static void spell_fly(struct char_data* ch, struct char_data* victim) {
  struct affected_type af{};

  assert(ch && victim);

  act("You feel lighter than air!", 1, ch, nullptr, victim, TO_VICT);
  if (victim != ch) {
    act("$N's feet rise off the ground.", 1, ch, nullptr, victim, TO_CHAR);
  } else {
    send_to_char("Your feet rise up off the ground.\n\r", ch);
  }
  act("$N's feet rise off the ground.", 1, ch, nullptr, victim, TO_NOTVICT);

  af.type = SPELL_FLY;
  af.duration = GET_LEVEL(ch, BestMagicClass(ch)) + 3;
  af.modifier = 0;
  af.location = 0;
  af.bitvector = AFF_FLYING;
  affect_to_char(victim, &af);
}

void cast_flying(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_fly(ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_fly(ch, tar_ch);
      break;
    case SPELL_TYPE_WAND:
      spell_fly(ch, tar_ch);
      break;

    default:
      vlog("Serious screw-up in fly");
      break;
  }
}

static void spell_fly_group(struct char_data* ch) {
  struct affected_type af{};
  struct char_data* tch = nullptr;

  assert(ch);

  if (real_roomp(ch->in_room) == nullptr) {
    return;
  }

  for (tch = real_roomp(ch->in_room)->people; tch != nullptr;
    tch = tch->next_in_room) {
    if (in_group(ch, tch) != 0) {
      act("You feel lighter than air!", 1, ch, nullptr, tch, TO_VICT);
      if (tch != ch) {
        act("$N's feet rise off the ground.", 1, ch, nullptr, tch, TO_CHAR);
      } else {
        send_to_char("Your feet rise up off the ground.", ch);
      }
      act("$N's feet rise off the ground.", 1, ch, nullptr, tch, TO_NOTVICT);

      af.type = SPELL_FLY;
      af.duration = GET_LEVEL(ch, BestMagicClass(ch)) + 3;
      af.modifier = 0;
      af.location = 0;
      af.bitvector = AFF_FLYING;
      affect_to_char(tch, &af);
    }
  }
}

void cast_fly_group(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_fly_group(ch);
      break;
    case SPELL_TYPE_POTION:
      spell_fly(ch, tar_ch);
      break;
    default:
      vlog("Serious screw-up in fly");
      break;
  }
}

static void spell_heroes_feast(struct char_data* ch) {
  struct char_data* tch = nullptr;

  if (real_roomp(ch->in_room) == nullptr) {
    return;
  }

  for (tch = real_roomp(ch->in_room)->people; tch != nullptr;
    tch = tch->next_in_room) {
    if (((in_group(tch, ch)) != 0) && (GET_POS(ch) > POSITION_SLEEPING)) {
      send_to_char("You partake of a magnificent feast!\n\r", ch);

      if (GET_COND(ch, FULL) >= 0) {
        gain_condition(tch, FULL, 24);
      }
      if (GET_COND(ch, THIRST) >= 0) {
        gain_condition(tch, THIRST, 24);
      }
      if (GET_HIT(ch) < GET_MAX_HIT(ch)) {
        GET_HIT(ch) += 1;
      }
    }
  }
}

void cast_heroes_feast(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*tar_ch*/,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_heroes_feast(ch);
      break;
    default:
      vlog("Serious screw-up in heroes feast");
      break;
  }
}

static void spell_synostodweomer(struct char_data* ch,
  struct char_data* victim) {
  int hitp = 0;
  assert(victim);

  hitp = (GET_HIT(ch) / 2);

  GET_HIT(victim) += hitp;

  if (GET_HIT(victim) > hit_limit(victim)) {
    GET_HIT(ch) = hit_limit(ch);
  }

  GET_HIT(ch) -= hitp;

  GET_ALIGNMENT(ch) += (hitp / 2);

  GET_ALIGNMENT(ch) = std::min(GET_ALIGNMENT(ch), 1000);

  update_pos(victim);
  update_pos(ch);

  send_to_char("You feel like you have been touched by god!!\n\r", victim);
  send_to_char("You give half of your hit points to a needy person!\n\r", ch);
}

void cast_synostodweomer(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_synostodweomer(ch, tar_ch);
      break;
    default:
      vlog("Serious screw up in synostodweomer!!");
      break;
  }
}

static void spell_heal_spray(struct char_data* ch) {
  struct char_data* tch = nullptr;

  if (real_roomp(ch->in_room) == nullptr) {
    return;
  }

  for (tch = real_roomp(ch->in_room)->people; tch != nullptr;
    tch = tch->next_in_room) {
    if (((in_group(tch, ch)) != 0) && (GET_POS(ch) > POSITION_SLEEPING)) {
      send_to_char("You send out a magnificent heal spray!\n\r", ch);
      GET_HIT(tch) += 100;

      if (GET_HIT(tch) >= hit_limit(tch)) {
        GET_HIT(tch) = hit_limit(tch) - dice(1, 4);
      }
    }
  }
}

void cast_heal_spray(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*tar_ch*/,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_heal_spray(ch);
      break;
    default:
      vlog("Serious screw-up in heal spray!");
      break;
  }
}

static void spell_cure_blind(signed char level, struct char_data* victim) {
  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  if (affected_by_spell(victim, SPELL_BLINDNESS) != 0) {
    affect_from_char(victim, SPELL_BLINDNESS);

    send_to_char("Your vision returns!\n\r", victim);
  }
}

void cast_cure_blind(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cure_blind(level, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_cure_blind(level, ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_cure_blind(level, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in cure blind!");
      break;
  }
}

static void spell_cure_critic(signed char level, struct char_data* victim) {
  int healpoints = 0;

  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  healpoints = dice(3, 8) + 3;

  if ((healpoints + GET_HIT(victim)) > hit_limit(victim)) {
    GET_HIT(victim) = hit_limit(victim);
  } else {
    GET_HIT(victim) += healpoints;
  }

  send_to_char("You feel better!\n\r", victim);

  update_pos(victim);
}

void cast_cure_critic(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cure_critic(level, tar_ch);
      break;
    case SPELL_TYPE_SCROLL:
      spell_cure_critic(level, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_cure_critic(level, ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_cure_critic(level, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_cure_critic(level, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in cure critic!");
      break;
  }
}

static void spell_cure_light(signed char level, struct char_data* victim) {
  int healpoints = 0;

  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  healpoints = dice(1, 8);

  if ((healpoints + GET_HIT(victim)) > hit_limit(victim)) {
    GET_HIT(victim) = hit_limit(victim);
  } else {
    GET_HIT(victim) += healpoints;
  }

  send_to_char("You feel better!\n\r", victim);

  update_pos(victim);
}

void cast_cure_light(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cure_light(level, tar_ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_cure_light(level, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_cure_light(level, ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_cure_light(level, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_cure_light(level, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in cure light!");
      break;
  }
}

static void spell_cure_serious(signed char level, struct char_data* ch,
  struct char_data* victim) {
  int dam = 0;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(2, 8) + 2;

  if ((dam + GET_HIT(victim)) > hit_limit(victim)) {
    GET_HIT(victim) = hit_limit(victim);
  } else {
    GET_HIT(victim) += dam;
  }

  send_to_char("You feel better!\n\r", victim);

  update_pos(victim);
}

void cast_cure_serious(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cure_serious(level, ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_cure_serious(level, ch, ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_cure_serious(level, ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_cure_serious(level, ch, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in cure serious!");
      break;
  }
}

static void spell_refresh(signed char level, struct char_data* ch,
  struct char_data* victim) {
  int dam = 0;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(level, 4) + level;
  dam = MAX(dam, 20);

  if ((dam + GET_MOVE(victim)) > move_limit(victim)) {
    GET_MOVE(victim) = move_limit(victim);
  } else {
    GET_MOVE(victim) += dam;
  }

  send_to_char("You feel less tired\n\r", victim);
}

void cast_refresh(signed char level, struct char_data* ch, const char* /*arg*/,
  int type, struct char_data* tar_ch, struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_refresh(level, ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_refresh(level, ch, ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_refresh(level, ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_refresh(level, ch, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in refresh!");
      break;
  }
}

static void spell_second_wind(signed char level, struct char_data* ch,
  struct char_data* victim) {
  int dam = 0;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  dam = dice(level, 8) + level;

  if ((dam + GET_MOVE(victim)) > move_limit(victim)) {
    GET_MOVE(victim) = move_limit(victim);
  } else {
    GET_MOVE(victim) += dam;
  }

  send_to_char("You feel less tired\n\r", victim);
}

void cast_second_wind(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_second_wind(level, ch, tar_ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_second_wind(level, ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_second_wind(level, ch, ch);
      break;
    case SPELL_TYPE_WAND:

    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_second_wind(level, ch, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in second_wind!");
      break;
  }
}

static void spell_shield(signed char level, struct char_data* ch,
  struct char_data* victim) {
  struct affected_type af{};

  assert(victim && ch);

  if (affected_by_spell(victim, SPELL_SHIELD) == 0) {
    act("$N is surrounded by a strong force shield.", 1, ch, nullptr, victim,
      TO_NOTVICT);
    if (ch != victim) {
      act("$N is surrounded by a strong force shield.", 1, ch, nullptr, victim,
        TO_CHAR);
      act("You are surrounded by a strong force shield.", 1, ch, nullptr,
        victim, TO_VICT);
    } else {
      act("You are surrounded by a strong force shield.", 1, ch, nullptr,
        victim, TO_VICT);
    }

    af.type = SPELL_SHIELD;
    af.duration = 8 + level;
    af.modifier = -10;
    af.location = APPLY_AC;
    af.bitvector = 0;
    affect_to_char(victim, &af);
  }
}

void cast_shield(signed char level, struct char_data* ch, const char* /*arg*/,
  int type, struct char_data* tar_ch, struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_shield(level, ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_shield(level, ch, ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_shield(level, ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_shield(level, ch, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in shield!");
      break;
  }
}

static void spell_curse(signed char level, struct char_data* ch,
  struct char_data* victim, struct obj_data* obj) {
  struct affected_type af{};

  assert(victim || obj);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  if (obj != nullptr) {
    SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
    SET_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);

    /* LOWER ATTACK DICE BY -1 */
    if (obj->obj_flags.type_flag == ITEM_WEAPON) {
      obj->obj_flags.value[2]--;
    }
    act("$p glows red.", 0, ch, obj, nullptr, TO_CHAR);
  } else {
    if ((saves_spell(victim, SAVING_SPELL) != 0) ||
        (affected_by_spell(victim, SPELL_CURSE) != 0)) {
      return;
    }

    af.type = SPELL_CURSE;
    af.duration = 24 * 7; /* 7 Days */
    af.modifier = -1;
    af.location = APPLY_HITROLL;
    af.bitvector = AFF_CURSE;
    affect_to_char(victim, &af);

    af.location = APPLY_SAVING_PARA;
    af.modifier = 1; /* Make worse */
    affect_to_char(victim, &af);

    act("$n briefly reveal a red aura!", 0, victim, nullptr, nullptr, TO_ROOM);
    act("You feel very uncomfortable.", 0, victim, nullptr, nullptr, TO_CHAR);
    if (IS_NPC(victim) && (victim->specials.fighting == nullptr)) {
      set_fighting(victim, ch);
    }
  }
}

void cast_curse(signed char level, struct char_data* ch, const char* /*arg*/,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
  char buf[255];

  switch (type) {
    case SPELL_TYPE_SPELL:
      if (tar_obj != nullptr) { /* It is an object */
        spell_curse(level, ch, nullptr, tar_obj);
      } else { /* Then it is a PC | NPC */
        spell_curse(level, ch, tar_ch, nullptr);
      }
      break;
    case SPELL_TYPE_POTION:
      spell_curse(level, ch, ch, nullptr);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) { /* It is an object */
        spell_curse(level, ch, nullptr, tar_obj);
      } else { /* Then it is a PC | NPC */
        if (tar_ch == nullptr) {
          tar_ch = ch;
        }
        spell_curse(level, ch, tar_ch, nullptr);
      }
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj != nullptr) { /* It is an object */
        spell_curse(level, ch, nullptr, tar_obj);
      } else { /* Then it is a PC | NPC */
        if (tar_ch == nullptr) {
          tar_ch = ch;
        }
        spell_curse(level, ch, tar_ch, nullptr);
      }
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_curse(level, ch, tar_ch, nullptr);
        }
      }
      break;
    default:
      sprintf(buf, "Serious screw up in curse! Char = %s.", ch->player.name);
      vlog(buf);
      break;
  }
}

static void spell_dispel_invisible(struct char_data* ch,
  struct char_data* victim, struct obj_data* obj) {
  assert((ch && obj) || victim);

  if (obj != nullptr) {
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE)) {
      act("$p fades into visibility.", 0, ch, obj, nullptr, TO_CHAR);
      act("$p fades into visibility.", 1, ch, obj, nullptr, TO_ROOM);
      obj->obj_flags.extra_flags -= ITEM_INVISIBLE;
    }
  } else {
    if (affected_by_spell(victim, SPELL_INVISIBLE) != 0) {
      act("$n slowly fades into existance.", 1, victim, nullptr, nullptr,
        TO_ROOM);
      send_to_char("You turn visible.\n\r", ch);
      affect_from_char(ch, SPELL_INVISIBLE);
      if (IS_SET(ch->specials.affected_by, AFF_INVISIBLE)) {
        ch->specials.affected_by -= AFF_INVISIBLE;
      }
    }
  }
}

void cast_dispel_invisible(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  char buf[255];

  switch (type) {
    case SPELL_TYPE_SPELL:
      if (tar_obj != nullptr) {
        spell_dispel_invisible(ch, nullptr, tar_obj);
      } else {
        spell_dispel_invisible(ch, tar_ch, nullptr);
      }
      break;
    case SPELL_TYPE_POTION:
      spell_dispel_invisible(ch, ch, nullptr);
      break;
    default:
      sprintf(buf, "Serious fuck up in dispel invisible!\n\r", ch);
      vlog(buf);
      break;
  }
}

static void spell_detect_evil(signed char level, struct char_data* victim) {
  struct affected_type af{};

  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  if (affected_by_spell(victim, SPELL_DETECT_EVIL) != 0) {
    return;
  }

  af.type = SPELL_DETECT_EVIL;
  af.duration = level * 5;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_EVIL;

  affect_to_char(victim, &af);

  act("$n's eyes briefly glow white", 0, victim, nullptr, nullptr, TO_ROOM);
  send_to_char("Your eyes tingle.\n\r", victim);
}

void cast_detect_evil(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (affected_by_spell(tar_ch, SPELL_DETECT_EVIL) != 0) {
        send_to_char("Nothing seems to happen.\n\r", tar_ch);
        return;
      }
      spell_detect_evil(level, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      if (affected_by_spell(ch, SPELL_DETECT_EVIL) != 0) {
        return;
      }
      spell_detect_evil(level, ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          if (!(IS_AFFECTED(tar_ch, AFF_DETECT_EVIL))) {
            spell_detect_evil(level, tar_ch);
          }
        }
      }
      break;
    default:
      vlog("Serious screw-up in detect evil!");
      break;
  }
}

static void spell_detect_invisibility(signed char level, struct char_data* ch,
  struct char_data* victim) {
  struct affected_type af{};

  assert(ch && victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  if (affected_by_spell(victim, SPELL_DETECT_INVISIBLE) != 0) {
    return;
  }

  af.type = SPELL_DETECT_INVISIBLE;
  af.duration = level * 5;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_INVISIBLE;

  affect_to_char(victim, &af);
  act("$n's eyes briefly glow yellow", 0, victim, nullptr, nullptr, TO_ROOM);
  send_to_char("Your eyes tingle.\n\r", victim);
}

void cast_detect_invisibility(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (IS_AFFECTED(tar_ch, AFF_DETECT_INVISIBLE)) {
        send_to_char("Nothing seems to happen.\n\r", tar_ch);
        return;
      }
      spell_detect_invisibility(level, ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      if (IS_AFFECTED(ch, AFF_DETECT_INVISIBLE)) {
        return;
      }
      spell_detect_invisibility(level, ch, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      if (IS_AFFECTED(tar_ch, AFF_DETECT_INVISIBLE)) {
        return;
      }
      spell_detect_invisibility(level, ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (in_group(ch, tar_ch) != 0) {
          if (!(IS_AFFECTED(tar_ch, AFF_DETECT_INVISIBLE))) {
            spell_detect_invisibility(level, ch, tar_ch);
          }
        }
      }
      break;
    default:
      vlog("Serious screw-up in detect invisibility!");
      break;
  }
}

static void spell_detect_magic(signed char level, struct char_data* victim) {
  struct affected_type af{};

  assert(victim);
  assert((level >= 0) && (level <= ABS_MAX_LVL));

  if (affected_by_spell(victim, SPELL_DETECT_MAGIC) != 0) {
    return;
  }

  af.type = SPELL_DETECT_MAGIC;
  af.duration = level * 5;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_MAGIC;

  affect_to_char(victim, &af);
  send_to_char("Your eyes tingle.\n\r", victim);
}

void cast_detect_magic(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (affected_by_spell(tar_ch, SPELL_DETECT_MAGIC) != 0) {
        send_to_char("Nothing seems to happen.\n\r", tar_ch);
        return;
      }
      spell_detect_magic(level, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      if (affected_by_spell(ch, SPELL_DETECT_MAGIC) != 0) {
        return;
      }
      spell_detect_magic(level, ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          if (!(IS_AFFECTED(tar_ch, SPELL_DETECT_MAGIC))) {
            spell_detect_magic(level, tar_ch);
          }
        }
      }
      break;
    default:
      vlog("Serious screw-up in detect magic!");
      break;
  }
}

static void spell_detect_poison(struct char_data* ch, struct char_data* victim,
  struct obj_data* obj) {
  assert(ch && (victim || obj));

  if (victim != nullptr) {
    if (victim == ch) {
      if (IS_AFFECTED(victim, AFF_POISON)) {
        send_to_char("You can sense poison in your blood.\n\r", ch);
      } else {
        send_to_char("You feel healthy.\n\r", ch);
      }
    } else if (IS_AFFECTED(victim, AFF_POISON)) {
      act("You sense that $E is poisoned.", 0, ch, nullptr, victim, TO_CHAR);
    } else {
      act("You sense that $E is poisoned", 0, ch, nullptr, victim, TO_CHAR);
    }
  } else { /* It's an object */
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      if (obj->obj_flags.value[3] != 0) {
        act("Poisonous fumes are revealed.", 0, ch, nullptr, nullptr, TO_CHAR);
      } else {
        send_to_char("It looks very delicious.\n\r", ch);
      }
    }
  }
}

void cast_detect_poison(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_detect_poison(ch, tar_ch, tar_obj);
      break;
    case SPELL_TYPE_POTION:
      spell_detect_poison(ch, ch, nullptr);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        spell_detect_poison(ch, nullptr, tar_obj);
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_detect_poison(ch, tar_ch, nullptr);
      break;
    default:
      vlog("Serious screw-up in detect poison!");
      break;
  }
}

static void spell_dispel_evil(signed char level, struct char_data* ch,
  struct char_data* victim) {
  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (IsExtraPlanar(victim) != 0) {
    if (IS_EVIL(ch)) {
      victim = ch;
    } else {
      if (IS_GOOD(victim)) {
        act("Good protects $N.", 0, ch, nullptr, victim, TO_CHAR);
        return;
      }
    }
    if (saves_spell(victim, SAVING_SPELL) == 0) {
      act("$n forces $N from this plane.", 1, ch, nullptr, victim, TO_ROOM);
      act("You force $N from this plane.", 1, ch, nullptr, victim, TO_CHAR);
      act("$n forces you from this plane.", 1, ch, nullptr, victim, TO_VICT);
      gain_exp(ch, MIN(GET_EXP(victim) / 2, 50000));
      extract_char(victim);
    }
  } else {
    act("$N laughs at you.", 1, ch, nullptr, victim, TO_CHAR);
    act("$N laughs at $n.", 1, ch, nullptr, victim, TO_NOTVICT);
    act("You laugh at $n.", 1, ch, nullptr, victim, TO_VICT);
  }
}

void cast_dispel_evil(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_dispel_evil(level, ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_dispel_evil(level, ch, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_dispel_evil(level, ch, tar_ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj != nullptr) {
        return;
      }
      spell_dispel_evil(level, ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (in_group(tar_ch, ch) == 0) {
          spell_dispel_evil(level, ch, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in dispel evil!");
      break;
  }
}

static void spell_dispel_good(signed char level, struct char_data* ch,
  struct char_data* victim) {
  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (IsExtraPlanar(victim) != 0) {
    if (IS_GOOD(ch)) {
      victim = ch;
    } else if (IS_EVIL(victim)) {
      act("Evil protects $N.", 0, ch, nullptr, victim, TO_CHAR);
      return;
    }

    if (saves_spell(victim, SAVING_SPELL) == 0) {
      act("$n forces $N from this plane.", 1, ch, nullptr, victim, TO_NOTVICT);
      act("You force $N from this plane.", 1, ch, nullptr, victim, TO_CHAR);
      act("$n forces you from this plane.", 1, ch, nullptr, victim, TO_VICT);
      gain_exp(ch, MIN(GET_EXP(victim) / 2, 50000));
      extract_char(victim);
    }
  } else {
    act("$N laughs at you.", 1, ch, nullptr, victim, TO_CHAR);
    act("$N laughs at $n.", 1, ch, nullptr, victim, TO_NOTVICT);
    act("You laugh at $n.", 1, ch, nullptr, victim, TO_VICT);
  }
}

void cast_dispel_good(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_dispel_good(level, ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_dispel_good(level, ch, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_dispel_good(level, ch, tar_ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj != nullptr) {
        return;
      }
      spell_dispel_good(level, ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (in_group(tar_ch, ch) == 0) {
          spell_dispel_good(level, ch, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in dispel good!");
      break;
  }
}

static void spell_faerie_fire(signed char level, struct char_data* ch,
  struct char_data* victim) {
  struct affected_type af{};

  assert(ch && victim);

  if (affected_by_spell(victim, SPELL_FAERIE_FIRE) != 0) {
    send_to_char("Nothing new seems to happen", ch);
    return;
  }

  act("$n points at $N.", 1, ch, nullptr, victim, TO_ROOM);
  act("You point at $N.", 1, ch, nullptr, victim, TO_CHAR);
  act("$N is surrounded by a pink outline", 1, ch, nullptr, victim, TO_ROOM);
  act("$N is surrounded by a pink outline", 1, ch, nullptr, victim, TO_CHAR);

  af.type = SPELL_FAERIE_FIRE;
  af.duration = level;
  af.modifier = 10;
  af.location = APPLY_ARMOR;
  af.bitvector = 0;

  affect_to_char(victim, &af);
}

void cast_faerie_fire(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_faerie_fire(level, ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_faerie_fire(level, ch, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_faerie_fire(level, ch, tar_ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj != nullptr) {
        return;
      }
      spell_faerie_fire(level, ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (in_group(tar_ch, ch) == 0) {
          spell_faerie_fire(level, ch, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in dispel good!");
      break;
  }
}

static void spell_enchant_weapon(signed char level, struct char_data* ch,
  struct obj_data* obj) {
  int i = 0;

  assert(ch && obj);
  static_assert(MAX_OBJ_AFFECT >= 2, "");

  if ((GET_ITEM_TYPE(obj) == ITEM_WEAPON) &&
      !IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if (obj->affected[i].location != APPLY_NONE) {
        return;
      }
    }

    SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);

    obj->affected[0].location = APPLY_HITROLL;
    obj->affected[0].modifier = 1;
    if (level > 20) {
      obj->affected[0].modifier += 1;
    }
    if (level > 40) {
      obj->affected[0].modifier += 1;
    }
    if (level > MAX_MORT) {
      obj->affected[0].modifier += 1;
    }

    obj->affected[1].location = APPLY_DAMROLL;
    obj->affected[1].modifier = 1;
    if (level > 15) {
      obj->affected[1].modifier += 1;
    }
    if (level > 30) {
      obj->affected[1].modifier += 1;
    }
    if (level > MAX_MORT) {
      obj->affected[1].modifier += 1;
    }

    if (IS_GOOD(ch)) {
      SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL);
      act("$p glows blue.", 0, ch, obj, nullptr, TO_CHAR);
    } else if (IS_EVIL(ch)) {
      SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
      act("$p glows red.", 0, ch, obj, nullptr, TO_CHAR);
    } else {
      act("$p glows yellow.", 0, ch, obj, nullptr, TO_CHAR);
    }
  }
}

void cast_enchant_weapon(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*tar_ch*/,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_enchant_weapon(level, ch, tar_obj);
      break;

    case SPELL_TYPE_SCROLL:
      if (tar_obj == nullptr) {
        return;
      }
      spell_enchant_weapon(level, ch, tar_obj);
      break;
    default:
      vlog("Serious screw-up in enchant weapon!");
      break;
  }
}

static void cast_enchant_armor(int type) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      /*			spell_enchant_armor(level, ch, 0,tar_obj);
            break;
      */
    case SPELL_TYPE_SCROLL:
      /*			if(!tar_obj) return;
            spell_enchant_armor(level, ch, 0,tar_obj);
            break;
      */
    default:
      vlog("Serious screw-up in enchant armor!");
      break;
  }
}

static void spell_heal(signed char level, struct char_data* victim) {
  assert(victim);

  spell_cure_blind(level, victim);

  GET_HIT(victim) += 100;

  if (GET_HIT(victim) >= hit_limit(victim)) {
    GET_HIT(victim) = hit_limit(victim) - dice(1, 4);
  }

  update_pos(victim);

  send_to_char("A warm feeling fills your body.\n\r", victim);
}

void cast_heal(signed char level, struct char_data* ch, const char* /*arg*/,
  int type, struct char_data* tar_ch, struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      act("$n heals $N.", 0, ch, nullptr, tar_ch, TO_NOTVICT);
      act("You heal $N.", 0, ch, nullptr, tar_ch, TO_CHAR);
      spell_heal(level, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_heal(level, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_heal(level, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_heal(level, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in heal!");
      break;
  }
}

static void spell_full_heal(signed char level, struct char_data* victim) {
  assert(victim);

  spell_cure_blind(level, victim);

  GET_HIT(victim) += 200;

  if (GET_HIT(victim) >= hit_limit(victim)) {
    GET_HIT(victim) = hit_limit(victim) - dice(1, 8);
  }

  update_pos(victim);

  send_to_char("A hot rush runs through your body.\n\r", victim);
}

void cast_full_heal(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      act("$n heals $N.", 0, ch, nullptr, tar_ch, TO_NOTVICT);
      act("You heal $N.", 0, ch, nullptr, tar_ch, TO_CHAR);
      spell_full_heal(level, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_full_heal(level, ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_full_heal(level, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in full heal!");
      break;
  }
}

static void spell_invis_group(signed char level, struct char_data* ch) {
  struct char_data* tmpv = nullptr;
  struct affected_type af{};

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  for (tmpv = real_roomp(ch->in_room)->people; tmpv != nullptr;
    tmpv = tmpv->next_in_room) {
    if ((ch->in_room == tmpv->in_room) && (ch != tmpv)) {
      if (in_group(ch, tmpv) != 0) {
        if (affected_by_spell(tmpv, SPELL_INVISIBLE) == 0) {
          act("$n slowly fades out of existence.", 1, tmpv, nullptr, nullptr,
            TO_ROOM);
          send_to_char("You vanish.\n\r", tmpv);

          af.type = SPELL_INVISIBLE;
          af.duration = 24;
          af.modifier = -40;
          af.location = APPLY_AC;
          af.bitvector = AFF_INVISIBLE;
          affect_to_char(tmpv, &af);
        }
      }
    }
  }
}

static void spell_invisibility(struct char_data* ch, struct char_data* victim,
  struct obj_data* obj) {
  struct affected_type af{};

  assert((ch && obj) || victim);

  if (obj != nullptr) {
    if (!IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE)) {
      act("$p turns invisible.", 0, ch, obj, nullptr, TO_CHAR);
      act("$p turns invisible.", 1, ch, obj, nullptr, TO_ROOM);
      SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
    }
  } else { /* Then it is a PC | NPC */
    if (affected_by_spell(victim, SPELL_INVISIBLE) == 0) {
      act("$n slowly fades out of existence.", 1, victim, nullptr, nullptr,
        TO_ROOM);
      send_to_char("You vanish.\n\r", victim);

      af.type = SPELL_INVISIBLE;
      af.duration = 24;
      af.modifier = -40;
      af.location = APPLY_AC;
      af.bitvector = AFF_INVISIBLE;
      affect_to_char(victim, &af);
    }
  }
}

void cast_invisibility(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (tar_obj != nullptr) {
        if (IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE)) {
          send_to_char("Nothing new seems to happen.\n\r", ch);
        } else {
          spell_invisibility(ch, nullptr, tar_obj);
        }
      } else { /* tar_ch */
        if (IS_AFFECTED(tar_ch, AFF_INVISIBLE)) {
          send_to_char("Nothing new seems to happen.\n\r", ch);
        } else {
          spell_invisibility(ch, tar_ch, nullptr);
        }
      }
      break;
    case SPELL_TYPE_POTION:
      if (!IS_AFFECTED(ch, AFF_INVISIBLE)) {
        spell_invisibility(ch, ch, nullptr);
      }
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE))) {
          spell_invisibility(ch, nullptr, tar_obj);
        }
      } else { /* tar_ch */
        if (tar_ch == nullptr) {
          tar_ch = ch;
        }

        if (!(IS_AFFECTED(tar_ch, AFF_INVISIBLE))) {
          spell_invisibility(ch, tar_ch, nullptr);
        }
      }
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj != nullptr) {
        if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE))) {
          spell_invisibility(ch, nullptr, tar_obj);
        }
      } else { /* tar_ch */
        if (!(IS_AFFECTED(tar_ch, AFF_INVISIBLE))) {
          spell_invisibility(ch, tar_ch, nullptr);
        }
      }
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          if (!(IS_AFFECTED(tar_ch, AFF_INVISIBLE))) {
            spell_invisibility(ch, tar_ch, nullptr);
          }
        }
      }
      break;
    default:
      vlog("Serious screw-up in invisibility!");
      break;
  }
}

static void spell_locate_object(signed char level, struct char_data* ch,
  const char* obj) {
  struct obj_data* i = nullptr;
  char name[256];
  char buf[MAX_STRING_LENGTH];
  int j = 0;

  assert(ch);

  strcpy(name, obj);

  j = level >> 1;

  for (i = object_list; (i != nullptr) && (j > 0); i = i->next) {
    if (isname(name, i->name) != 0) {
      if (i->carried_by != nullptr) {
        if (strlen(PERS(i->carried_by, ch)) > 0) {
          sprintf(buf, "%s carried by %s.\n\r", i->short_description,
            PERS(i->carried_by, ch));
          send_to_char(buf, ch);
        }
      } else if (i->equipped_by != nullptr) {
        if (strlen(PERS(i->equipped_by, ch)) > 0) {
          sprintf(buf, "%s equipped by %s.\n\r", i->short_description,
            PERS(i->equipped_by, ch));
          send_to_char(buf, ch);
        }
      } else if (i->in_obj != nullptr) {
        sprintf(buf, "%s in %s.\n\r", i->short_description,
          i->in_obj->short_description);
        send_to_char(buf, ch);
      } else {
        sprintf(buf, "%s in %s.\n\r", i->short_description,
          (i->in_room == NOWHERE ? "use but uncertain."
                                 : real_roomp(i->in_room)->name));
        send_to_char(buf, ch);
        j--;
      }
    }
  }

  if (j == 0) {
    send_to_char("You are very confused.\n\r", ch);
  }
  if (j == level >> 1) {
    send_to_char("No such object.\n\r", ch);
  }
}

void cast_locate_object(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* /*tar_ch*/,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_locate_object(level, ch, arg);
      break;
    default:
      vlog("Serious screw-up in locate object!");
      break;
  }
}

static char imp_save_spell(struct char_data* ch, short int save_type, int mod) {
  int save = 0;

  /* Positive mod is better for save */

  /* Negative apply_saving_throw makes saving throw better! */

  save = ch->specials.apply_saving_throw[save_type] - mod;

  if (!IS_NPC(ch)) {
    /*
    **  Remove-For-Multi-Class
    */
    save += saving_throws[BestMagicClass(ch)][save_type]
                         [GET_LEVEL(ch, BestMagicClass(ch))];
    if (GetMaxLevel(ch) >= LOW_IMMORTAL) {
      return 1;
    }
  }

  return static_cast<char>(MAX(1, save) < number(1, 20));
}

static void spell_poison(signed char level, struct char_data* ch,
  struct char_data* victim, struct obj_data* obj) {
  struct affected_type af{};

  assert(victim || obj);

  if (victim != nullptr) {
    if (IsImmune(victim, IMM_POISON) != 0) {
      send_to_char("Your spell backfires horribly.\n\r", ch);
      damage(ch, ch, number(1, (GetMaxLevel(ch) * 2)), SPELL_POISON);
      return;
    }
    if (IS_NPC(ch)) {
      if (!IS_SET(ch->specials.act, ACT_DEADLY)) {
        if (imp_save_spell(victim, SAVING_PARA, 0) == 0) {
          af.type = SPELL_POISON;
          af.duration = level * 2;
          af.modifier = -2;
          af.location = APPLY_STR;
          af.bitvector = AFF_POISON;

          affect_join(victim, &af, 0, 0);

          send_to_char("You feel very sick.\n\r", victim);
          if (victim->specials.fighting == nullptr) {
            set_fighting(victim, ch);
          }
        } else {
          return;
        }
      } else {
        if (imp_save_spell(victim, SAVING_PARA, 0) == 0) {
          act("Deadly poison fills your veins.", 1, ch, nullptr, nullptr,
            TO_CHAR);
          damage(victim, victim, MAX(100, GET_HIT(victim) * 2), SPELL_POISON);
        } else {
          return;
        }
      }
    } else {
      if (imp_save_spell(victim, SAVING_PARA, 0) == 0) {
        af.type = SPELL_POISON;
        af.duration = level * 2;
        af.modifier = -2;
        af.location = APPLY_STR;
        af.bitvector = AFF_POISON;

        affect_join(victim, &af, 0, 0);

        send_to_char("You feel very sick.\n\r", victim);
      }
    }
    if (victim->specials.fighting == nullptr) {
      set_fighting(victim, ch);
    }
  } else { /* Object poison */
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      obj->obj_flags.value[3] = 1;
    }
  }
}

void cast_poison(signed char level, struct char_data* ch, const char* /*arg*/,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
      spell_poison(level, ch, tar_ch, tar_obj);
      break;
    case SPELL_TYPE_POTION:
      spell_poison(level, ch, ch, nullptr);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_poison(level, ch, tar_ch, nullptr);
        }
      }
      break;
    default:
      vlog("Serious screw-up in poison!");
      break;
  }
}

static void spell_protection_from_evil(struct char_data* ch,
  struct char_data* victim) {
  struct affected_type af{};

  assert(victim);

  if (IS_EVIL(victim)) {
    act("$N's evilness disallows use of this spell.", 0, ch, nullptr, victim,
      TO_CHAR);
    return;
  }

  if (affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL) == 0) {
    af.type = SPELL_PROTECT_FROM_EVIL;
    af.duration = 24;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_PROTECT_EVIL;
    affect_to_char(victim, &af);
    send_to_char("You have a righteous feeling!\n\r", victim);
  }
}

void cast_protection_from_evil(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_protection_from_evil(ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_protection_from_evil(ch, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_protection_from_evil(ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_protection_from_evil(ch, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in protection from evil!");
      break;
  }
}

static void spell_protection_from_good(struct char_data* ch,
  struct char_data* victim) {
  struct affected_type af{};

  assert(victim);

  if (IS_GOOD(victim)) {
    act("$N is to holy for that spell.", 0, ch, nullptr, victim, TO_CHAR);
    return;
  }

  if (affected_by_spell(victim, SPELL_PROTECT_GOOD) == 0) {
    af.type = SPELL_PROTECT_GOOD;
    af.duration = 24;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_PROTECT_EVIL;
    affect_to_char(victim, &af);
    send_to_char("You have a righteous feeling!\n\r", victim);
  }
}

static void cast_protection_from_good(struct char_data* ch, int type,
  struct char_data* tar_ch, struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_protection_from_good(ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_protection_from_good(ch, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_protection_from_good(ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_protection_from_good(ch, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in protection from good!");
      break;
  }
}

static void spell_remove_curse(struct char_data* ch, struct char_data* victim,
  struct obj_data* obj) {
  assert(ch && (victim || obj));

  if (obj != nullptr) {
    if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
      act("$p briefly glows blue.", 1, ch, obj, nullptr, TO_CHAR);
      REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
    }
  } else { /* Then it is a PC | NPC */
    if (affected_by_spell(victim, SPELL_CURSE) != 0) {
      act("$n briefly glows red, then blue.", 0, victim, nullptr, nullptr,
        TO_ROOM);
      act("You feel better.", 0, victim, nullptr, nullptr, TO_CHAR);
      affect_from_char(victim, SPELL_CURSE);
    }
  }
}

void cast_remove_curse(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_remove_curse(ch, tar_ch, tar_obj);
      break;
    case SPELL_TYPE_POTION:
      spell_remove_curse(ch, ch, nullptr);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        spell_remove_curse(ch, nullptr, tar_obj);
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_remove_curse(ch, tar_ch, nullptr);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_remove_curse(ch, tar_ch, nullptr);
        }
      }
      break;
    default:
      vlog("Serious screw-up in remove curse!");
      break;
  }
}

static void spell_remove_poison(struct char_data* ch, struct char_data* victim,
  struct obj_data* obj) {
  assert(ch && (victim || obj));

  if (victim != nullptr) {
    if (affected_by_spell(victim, SPELL_POISON) != 0) {
      affect_from_char(victim, SPELL_POISON);
      act("A warm feeling runs through your body.", 0, victim, nullptr, nullptr,
        TO_CHAR);
      act("$N looks better.", 0, ch, nullptr, victim, TO_ROOM);
    }
  } else {
    if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
        (obj->obj_flags.type_flag == ITEM_FOOD)) {
      obj->obj_flags.value[3] = 0;
      act("The $p steams briefly.", 0, ch, obj, nullptr, TO_CHAR);
    }
  }
}

void cast_remove_poison(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_remove_poison(ch, tar_ch, tar_obj);
      break;
    case SPELL_TYPE_POTION:
      spell_remove_poison(ch, ch, nullptr);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_remove_poison(ch, tar_ch, nullptr);
        }
      }
      break;
    default:
      vlog("Serious screw-up in remove poison!");
      break;
  }
}

static void spell_remove_paralysis(struct char_data* ch,
  struct char_data* victim) {
  assert(ch && victim);

  if (affected_by_spell(victim, SPELL_PARALYSIS) != 0) {
    affect_from_char(victim, SPELL_PARALYSIS);
    act("A warm feeling runs through your body.", 0, victim, nullptr, nullptr,
      TO_CHAR);
    act("$N looks better.", 0, ch, nullptr, victim, TO_ROOM);
  }
}

void cast_remove_paralysis(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_remove_paralysis(ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_remove_paralysis(ch, ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_remove_paralysis(ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_remove_paralysis(ch, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in remove paralysis!");
      break;
  }
}

static void spell_sanctuary(signed char level, struct char_data* victim) {
  struct affected_type af{};

  if ((affected_by_spell(victim, SPELL_SANCTUARY) == 0) &&
      (!IS_AFFECTED(victim, AFF_SANCTUARY))) {
    act("$n is surrounded by a white aura.", 1, victim, nullptr, nullptr,
      TO_ROOM);
    act("You start glowing.", 1, victim, nullptr, nullptr, TO_CHAR);

    af.type = SPELL_SANCTUARY;
    af.duration = (level < LOW_IMMORTAL) ? 3 : level;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char(victim, &af);
  }
}

void cast_sanctuary(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_sanctuary(level, tar_ch);
      break;

    case SPELL_TYPE_WAND:
    case SPELL_TYPE_POTION:
      spell_sanctuary(level, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_sanctuary(level, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_sanctuary(level, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in sanctuary!");
      break;
  }
}

static void spell_silence(signed char level, struct char_data* ch,
  struct char_data* victim) {
  struct affected_type af{};

  if (GetMaxLevel(victim) > GetMaxLevel(ch)) {
    send_to_char("Sorry you can't silence a higher level than you.\n\r", ch);
    send_to_char("Some butthead just tried to silence you!\n\r", victim);
    return;
  }

  if ((affected_by_spell(victim, SPELL_SILENCE) == 0) &&
      (!IS_AFFECTED(victim, AFF_SILENT))) {
    act("$n has been muzzled!", 1, victim, nullptr, nullptr, TO_ROOM);
    act("You have been muzzled!", 1, victim, nullptr, nullptr, TO_CHAR);

    af.type = SPELL_SILENCE;
    af.duration = level;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_SILENT;
    affect_to_char(victim, &af);
  }
}

void cast_silence(signed char level, struct char_data* ch, const char* /*arg*/,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_silence(level, ch, tar_ch);
      break;

    case SPELL_TYPE_WAND:
    case SPELL_TYPE_POTION:
      spell_silence(level, ch, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_silence(level, ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_silence(level, ch, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in silence!");
      break;
  }
}

static void spell_fireshield(struct char_data* victim) {
  struct affected_type af{};

  if (affected_by_spell(victim, SPELL_FIRESHIELD) == 0) {
    act("$n is surrounded by a glowing red aura.", 1, victim, nullptr, nullptr,
      TO_ROOM);
    act("You start glowing red.", 1, victim, nullptr, nullptr, TO_CHAR);

    af.type = SPELL_FIRESHIELD;
    af.duration = 3;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_FIRESHIELD;
    affect_to_char(victim, &af);
  }
}

void cast_fireshield(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_fireshield(tar_ch);
      break;

    case SPELL_TYPE_WAND:
    case SPELL_TYPE_POTION:
      spell_fireshield(ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_fireshield(tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_fireshield(tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in fireshield!");
      break;
  }
}

static void fail_sleep(struct char_data* victim, struct char_data* ch) {
  send_to_char("You feel sleepy for a moment, but then you recover\n\r",
    victim);
  if (IS_NPC(victim)) {
    if ((victim->specials.fighting == nullptr) &&
        (GET_POS(victim) > POSITION_SLEEPING)) {
      set_fighting(victim, ch);
    }
  }
}

static void spell_sleep(signed char level, struct char_data* ch,
  struct char_data* victim) {
  struct affected_type af{};

  assert(victim);

  if (IsImmune(victim, IMM_SLEEP) != 0) {
    fail_sleep(victim, ch);
    return;
  }
  if (IsResist(victim, IMM_SLEEP) != 0) {
    if (saves_spell(victim, SAVING_SPELL) != 0) {
      fail_sleep(victim, ch);
      return;
    }
    if (saves_spell(victim, SAVING_SPELL) != 0) {
      fail_sleep(victim, ch);
      return;
    }
  } else if (IsSusc(victim, IMM_SLEEP) == 0) {
    if (saves_spell(victim, SAVING_SPELL) != 0) {
      fail_sleep(victim, ch);
      return;
    }
  }

  af.type = SPELL_SLEEP;
  af.duration = 4 + level;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_SLEEP;
  affect_join(victim, &af, 0, 0);

  if (GET_POS(victim) > POSITION_SLEEPING) {
    act("You feel very sleepy ..... zzzzzz", 0, victim, nullptr, nullptr,
      TO_CHAR);
    act("$n go to sleep.", 1, victim, nullptr, nullptr, TO_ROOM);
    GET_POS(victim) = POSITION_SLEEPING;
  }
}

void cast_sleep(signed char level, struct char_data* ch, const char* /*arg*/,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_sleep(level, ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_sleep(level, ch, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_sleep(level, ch, tar_ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj != nullptr) {
        return;
      }
      spell_sleep(level, ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_sleep(level, ch, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in sleep!");
      break;
  }
}

static void spell_strength(signed char level, struct char_data* ch,
  struct char_data* victim) {
  struct affected_type af{};

  assert(victim);

  if (affected_by_spell(victim, SPELL_STRENGTH) == 0) {
    act("You feel stronger.", 0, victim, nullptr, nullptr, TO_CHAR);
    act("$n seems stronger!\n\r", 0, victim, nullptr, nullptr, TO_ROOM);
    af.type = SPELL_STRENGTH;
    af.duration = 2 * level;
    if (IS_NPC(victim)) {
      if (level >= CREATOR) {
        af.modifier = 25 - GET_STR(victim);
      } else {
        af.modifier = number(1, 6);
      }
    } else {
      if (HasClass(ch, CLASS_WARRIOR) != 0) {
        af.modifier = number(1, 8);
      } else if ((HasClass(ch, CLASS_CLERIC) != 0) ||
                 (HasClass(ch, CLASS_THIEF) != 0)) {
        af.modifier = number(1, 6);
      } else {
        af.modifier = number(1, 4);
      }
    }
    af.location = APPLY_STR;
    af.bitvector = 0;
    affect_to_char(victim, &af);
  } else {
    act("Nothing seems to happen.", 0, ch, nullptr, nullptr, TO_CHAR);
  }
}

void cast_strength(signed char level, struct char_data* ch, const char* /*arg*/,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
      spell_strength(level, ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_strength(level, ch, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_strength(level, ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_strength(level, ch, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in strength!");
      break;
  }
}

void cast_ventriloquate(signed char /*level*/, struct char_data* ch,
  const char* arg, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  struct char_data* tmp_ch = nullptr;
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];

  if (type != SPELL_TYPE_SPELL) {
    vlog("Attempt to ventriloquate by non-cast-spell.");
    return;
  }
  for (; (*arg != 0) && (*arg == ' '); arg++) {
    ;
  }
  if (tar_obj != nullptr) {
    sprintf(buf1, "The %s says '%s'\n\r", fname(tar_obj->name), arg);
    sprintf(buf2, "Someone makes it sound like the %s says '%s'.\n\r",
      fname(tar_obj->name), arg);
  } else {
    sprintf(buf1, "%s says '%s'\n\r", GET_NAME(tar_ch), arg);
    sprintf(buf2, "Someone makes it sound like %s says '%s'\n\r",
      GET_NAME(tar_ch), arg);
  }

  sprintf(buf3, "Someone says, '%s'\n\r", arg);

  for (tmp_ch = real_roomp(ch->in_room)->people; tmp_ch != nullptr;
    tmp_ch = tmp_ch->next_in_room) {
    if ((tmp_ch != ch) && (tmp_ch != tar_ch)) {
      if (saves_spell(tmp_ch, SAVING_SPELL) != 0) {
        send_to_char(buf2, tmp_ch);
      } else {
        send_to_char(buf1, tmp_ch);
      }
    } else {
      if (tmp_ch == tar_ch) {
        send_to_char(buf3, tar_ch);
      }
    }
  }
}

static void spell_word_of_recall(struct char_data* ch,
  struct char_data* victim) {
  int location = 0;
  struct room_data* rp = nullptr;

  assert(victim);

  if (IS_NPC(victim)) {
    return;
  }

  rp = real_roomp(ch->in_room);

  if (IS_SET(rp->room_flags, ARENA)) {
    send_to_char("You can't recall from the arena!\n\r", ch);
    return;
  }

  /*  loc_nr = GET_HOME(ch); */

  if (ch->player.hometown != 0) {
    location = ch->player.hometown;
  } else {
    location = 3001;
  }

  if (real_roomp(location) == nullptr) {
    send_to_char("You are completely lost.\n\r", victim);
    return;
  }

  /* a location has been found. */

  act("$n disappears.", 1, victim, nullptr, nullptr, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, location);
  act("$n appears in the middle of the room.", 1, victim, nullptr, nullptr,
    TO_ROOM);
  do_look(victim, "", 15);
  GET_MOVE(victim) -= 100;
  GET_MOVE(victim) = MAX(0, GET_MOVE(victim));
  update_pos(victim);
}

void cast_word_of_recall(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_word_of_recall(ch, ch);
      break;
    case SPELL_TYPE_POTION:
      spell_word_of_recall(ch, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_word_of_recall(ch, tar_ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj != nullptr) {
        return;
      }
      spell_word_of_recall(ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_word_of_recall(ch, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in word of recall!");
      break;
  }
}

static void raw_summon(struct char_data* v, struct char_data* c) {
  short int target = 0;
  struct char_data* tmp = nullptr;
  struct obj_data* o = nullptr;
  struct obj_data* n = nullptr;
  int j = 0;
  int i = 0;

  if (IS_NPC(v) && (!IS_SET(v->specials.act, ACT_POLYSELF)) &&
      (GetMaxLevel(v) > GetMaxLevel(c) + 3)) {
    act("$N struggles, and all of $S items are destroyed!", 1, c, nullptr, v,
      TO_CHAR);
    /* remove objects from victim */
    for (j = 0; j < MAX_WEAR; j++) {
      if (v->equipment[j] != nullptr) {
        o = unequip_char(v, j);
        extract_obj(o);
      }
    }
    for (o = v->carrying; o != nullptr; o = n) {
      n = o->next_content;
      obj_from_char(o);
      extract_obj(o);
    }
    AddHated(v, c);
  } else {
    WAIT_STATE(c, PULSE_VIOLENCE * 6);
  }

  act("$n disappears suddenly.", 1, v, nullptr, nullptr, TO_ROOM);
  target = c->in_room;
  char_from_room(v);
  char_to_room(v, target);

  act("$n arrives suddenly.", 1, v, nullptr, nullptr, TO_ROOM);
  act("$n has summoned you!", 0, c, nullptr, v, TO_VICT);
  do_look(v, "", 15);

  for (tmp = real_roomp(v->in_room)->people; tmp != nullptr;
    tmp = tmp->next_in_room) {
    if (IS_NPC(tmp) && !(IS_SET(tmp->specials.act, ACT_POLYSELF)) &&
        ((IS_SET(tmp->specials.act, ACT_AGGRESSIVE) ||
          (IS_SET(tmp->specials.act, ACT_META_AGG))))) {
      act("$n growls at you", 1, tmp, nullptr, c, TO_VICT);
      act("$n growls at $N", 1, tmp, nullptr, c, TO_NOTVICT);
      i = number(0, 6);
      if (i == 0) {
        if (CAN_SEE(tmp, c) != 0) {
          hit(tmp, c, TYPE_UNDEFINED);
        }
      }
    }
  }
}

void spell_summon(signed char /*level*/, struct char_data* ch,
  struct char_data* victim, struct obj_data* /*obj*/) {
  assert(ch && victim);

  if (victim->in_room == NOWHERE) {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  if (GetMaxLevel(victim) > GET_LEVEL(ch, BestMagicClass(ch))) {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  if (!IS_IMMORTAL(ch)) {
    if (IS_NPC(victim)) {
      send_to_char(
        "You hear a booming voice echo 'due to abuse, that is impossible.'\n\r",
        ch);
      return;
    }
    if (IS_SET(real_roomp(victim->in_room)->room_flags, NO_SUM)) {
      send_to_char(
        "You cannot penetrate the magical defenses of that area.\n\r", ch);
      return;
    }

    if (IS_SET(victim->specials.act, ACT_AGGRESSIVE)) {
      send_to_char(
        "Sorry, due to problems, you can no longer summon aggressives.\n\r",
        ch);
      return;
    }
    if (IS_SET(real_roomp(victim->in_room)->room_flags, ARENA)) {
      send_to_char("That person is fighting in the arena. NO SUMMONING!\n\r",
        ch);
      return;
    }
  }
  raw_summon(victim, ch);
}

void cast_summon(signed char level, struct char_data* ch, const char* /*arg*/,
  int type, struct char_data* tar_ch, struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_summon(level, ch, tar_ch, nullptr);
      break;
    default:
      vlog("Serious screw-up in summon!");
      break;
  }
}

static void fail_charm(struct char_data* victim, struct char_data* ch) {
  if (IS_NPC(victim)) {
    if (victim->specials.fighting == nullptr) {
      set_fighting(victim, ch);
    }
  } else {
    send_to_char("You feel charmed, but the feeling fades.\n\r", victim);
  }
}

static void spell_charm_person(struct char_data* ch, struct char_data* victim) {
  struct affected_type af{};

  assert(ch && victim);

  if (victim == ch) {
    send_to_char("You like yourself even better!\n\r", ch);
    return;
  }

  if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
    if (circle_follow(victim, ch) != 0) {
      send_to_char("Sorry, following in circles can not be allowed.\n\r", ch);
      return;
    }

    if (IsPerson(victim) == 0) {
      send_to_char("Umm,  that's not a person....\n\r", ch);
      return;
    }

    if (GetMaxLevel(victim) > GetMaxLevel(ch) + 3) {
      fail_charm(victim, ch);
      return;
    }

    if ((IsImmune(victim, IMM_CHARM) != 0) || ((WeaponImmune(victim)) != 0)) {
      fail_charm(victim, ch);
      return;
    }
    if (IsResist(victim, IMM_CHARM) != 0) {
      if (saves_spell(victim, SAVING_PARA) != 0) {
        fail_charm(victim, ch);
        return;
      }

      if (saves_spell(victim, SAVING_PARA) != 0) {
        fail_charm(victim, ch);
        return;
      }
    } else {
      if (IsSusc(victim, IMM_CHARM) == 0) {
        if (saves_spell(victim, SAVING_PARA) != 0) {
          fail_charm(victim, ch);
          return;
        }
      }
    }

    if (victim->master != nullptr) {
      stop_follower(victim);
    }

    add_follower(victim, ch);

    af.type = SPELL_CHARM_PERSON;

    if (GET_INT(victim)) {
      af.duration = 24 * 18 / GET_INT(victim);
    } else {
      af.duration = 24 * 18;
    }

    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);

    act("Isn't $n just such a nice fellow?", 0, ch, nullptr, victim, TO_VICT);
  }
}

void cast_charm_person(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_charm_person(ch, tar_ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_ch == nullptr) {
        return;
      }
      spell_charm_person(ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (in_group(tar_ch, ch) == 0) {
          spell_charm_person(ch, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in charm person!");
      break;
  }
}

static void spell_charm_monster(struct char_data* ch,
  struct char_data* victim) {
  struct affected_type af{};

  assert(ch && victim);

  if (victim == ch) {
    send_to_char("You like yourself even better!\n\r", ch);
    return;
  }

  if (GetMaxLevel(victim) > GetMaxLevel(ch) + 3) {
    fail_charm(victim, ch);
    return;
  }

  if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
    if (circle_follow(victim, ch) != 0) {
      send_to_char("Sorry, following in circles can not be allowed.\n\r", ch);
      return;
    }
    if ((IsImmune(victim, IMM_CHARM) != 0) || ((WeaponImmune(victim)) != 0)) {
      fail_charm(victim, ch);
      return;
    }
    if (IsResist(victim, IMM_CHARM) != 0) {
      if (saves_spell(victim, SAVING_PARA) != 0) {
        fail_charm(victim, ch);
        return;
      }

      if (saves_spell(victim, SAVING_PARA) != 0) {
        fail_charm(victim, ch);
        return;
      }
    } else {
      if (IsSusc(victim, IMM_CHARM) == 0) {
        if (saves_spell(victim, SAVING_PARA) != 0) {
          fail_charm(victim, ch);
          return;
        }
      }
    }

    if (victim->master != nullptr) {
      stop_follower(victim);
    }

    add_follower(victim, ch);

    af.type = SPELL_CHARM_PERSON;

    if (GET_INT(victim)) {
      af.duration = 24 * 18 / GET_INT(victim);
    } else {
      af.duration = 24 * 18;
    }

    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);

    act("Isn't $n just such a nice fellow?", 0, ch, nullptr, victim, TO_VICT);
  }
}

void cast_charm_monster(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_charm_monster(ch, tar_ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_ch == nullptr) {
        return;
      }
      spell_charm_monster(ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (in_group(tar_ch, ch) == 0) {
          spell_charm_monster(ch, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in charm monster!");
      break;
  }
}

static void spell_control_undead(struct char_data* ch,
  struct char_data* victim) {
  struct affected_type af{};

  assert(ch && victim);

  if (victim == ch) {
    send_to_char("You like yourself even better!\n\r", ch);
    return;
  }

  if ((GET_RACE(victim) = RACE_UNDEAD) == 0) {
    return;
  }

  if (GetMaxLevel(victim) > GetMaxLevel(ch) + 3) {
    fail_charm(victim, ch);
    return;
  }

  if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
    if (circle_follow(victim, ch) != 0) {
      send_to_char("Sorry, following in circles can not be allowed.\n\r", ch);
      return;
    }
    if ((IsImmune(victim, IMM_CHARM) != 0) || ((WeaponImmune(victim)) != 0)) {
      fail_charm(victim, ch);
      return;
    }
    if (IsResist(victim, IMM_CHARM) != 0) {
      if (saves_spell(victim, SAVING_PARA) != 0) {
        fail_charm(victim, ch);
        return;
      }

      if (saves_spell(victim, SAVING_PARA) != 0) {
        fail_charm(victim, ch);
        return;
      }
    } else {
      if (IsSusc(victim, IMM_CHARM) == 0) {
        if (saves_spell(victim, SAVING_PARA) != 0) {
          fail_charm(victim, ch);
          return;
        }
      }
    }

    if (victim->master != nullptr) {
      stop_follower(victim);
    }

    add_follower(victim, ch);

    af.type = SPELL_CHARM_PERSON;

    if (GET_INT(victim)) {
      af.duration = 24 * 18 / GET_INT(victim);
    } else {
      af.duration = 24 * 18;
    }

    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);

    act("Isn't $n just such a nice fellow?", 0, ch, nullptr, victim, TO_VICT);
  }
}

void cast_control_undead(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_control_undead(ch, tar_ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_ch == nullptr) {
        return;
      }
      spell_control_undead(ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (in_group(tar_ch, ch) == 0) {
          spell_control_undead(ch, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in control undead!");
      break;
  }
}

static void spell_sense_life(signed char level, struct char_data* ch,
  struct char_data* victim) {
  struct affected_type af{};

  assert(victim);

  if (affected_by_spell(victim, SPELL_SENSE_LIFE) == 0) {
    send_to_char("Your feel your awareness improve.\n\r", ch);

    af.type = SPELL_SENSE_LIFE;
    af.duration = 5 * level;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_SENSE_LIFE;
    affect_to_char(victim, &af);
  }
}

void cast_sense_life(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_sense_life(level, ch, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_sense_life(level, ch, tar_ch);
      [[fallthrough]];
    case SPELL_TYPE_POTION:
      spell_sense_life(level, ch, ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_sense_life(level, ch, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in sense life!");
      break;
  }
}

static void spell_identify(struct char_data* ch, struct char_data* victim,
  struct obj_data* obj) {
  char buf[256];
  char buf2[256];
  int i = 0;
  char found = 0;
  float av_dam = NAN;

  assert(ch && (obj || victim));

  if (obj != nullptr) {
    send_to_char("You feel informed:\n\r", ch);

    sprintf(buf, "Object '%s', Item type: ", obj->name);
    sprinttype(GET_ITEM_TYPE(obj), item_types, buf2);
    strcat(buf, buf2);
    strcat(buf, "\n\r");
    send_to_char(buf, ch);

    if (obj->obj_flags.bitvector != 0) {
      send_to_char("Item will give you following abilities:  ", ch);
      sprintbit(obj->obj_flags.bitvector, affected_bits, buf);
      strcat(buf, "\n\r");
      send_to_char(buf, ch);
    }

    send_to_char("Item is: ", ch);
    sprintbit(obj->obj_flags.extra_flags, extra_bits, buf);
    strcat(buf, "\n\r");
    send_to_char(buf, ch);

    sprintf(buf, "Item is made out of %s.    ",
      material_types[obj->obj_flags.material_points]);
    send_to_char(buf, ch);

    send_to_char("Item will be ", ch);
    if ((obj->obj_flags.decay_time == -1) ||
        (obj->obj_flags.decay_time > 800)) {
      send_to_char("around a long time.\n\r", ch);
    } else if (obj->obj_flags.decay_time < 800) {
      send_to_char("gone one day.\n\r", ch);
    } else if (obj->obj_flags.decay_time < 400) {
      send_to_char("gone before you know it.\n\r", ch);
    } else if (obj->obj_flags.decay_time < 200) {
      send_to_char("gone soon.\n\r", ch);
    } else if (obj->obj_flags.decay_time < 100) {
      send_to_char("gone in no time.\n\r", ch);
    }

    sprintf(buf, "Volume: %d, Weight: %d, Value: %d, Rent cost: %d  %s\n\r",
      obj->obj_flags.volume, obj->obj_flags.weight, obj->obj_flags.cost,
      obj->obj_flags.cost_per_day,
      obj->obj_flags.cost_per_day > LIM_ITEM_COST_MIN ? "[RARE]" : " ");
    send_to_char(buf, ch);

    switch (GET_ITEM_TYPE(obj)) {
      case ITEM_SCROLL:
      case ITEM_POTION:
        sprintf(buf, "Level %d spells of:\n\r", obj->obj_flags.value[0]);
        send_to_char(buf, ch);
        if (obj->obj_flags.value[1] >= 1) {
          sprinttype(obj->obj_flags.value[1] - 1, spells, buf);
          strcat(buf, "\n\r");
          send_to_char(buf, ch);
        }
        if (obj->obj_flags.value[2] >= 1) {
          sprinttype(obj->obj_flags.value[2] - 1, spells, buf);
          strcat(buf, "\n\r");
          send_to_char(buf, ch);
        }
        if (obj->obj_flags.value[3] >= 1) {
          sprinttype(obj->obj_flags.value[3] - 1, spells, buf);
          strcat(buf, "\n\r");
          send_to_char(buf, ch);
        }
        break;

      case ITEM_WAND:
      case ITEM_STAFF:
        sprintf(buf, "Has %d charges, with %d charges left.\n\r",
          obj->obj_flags.value[1], obj->obj_flags.value[2]);
        send_to_char(buf, ch);

        sprintf(buf, "Level %d spell of:\n\r", obj->obj_flags.value[0]);
        send_to_char(buf, ch);

        if (obj->obj_flags.value[3] >= 1) {
          sprinttype(obj->obj_flags.value[3] - 1, spells, buf);
          strcat(buf, "\n\r");
          send_to_char(buf, ch);
        }
        break;

      case ITEM_WEAPON:
        av_dam =
          obj->obj_flags.value[1] * (obj->obj_flags.value[2] / 2.0 + 0.5);
        sprintf(buf, "The weapon does %s\n\r", GET_WEAPON_DAMAGE(av_dam));
        send_to_char(buf, ch);
        break;

      case ITEM_FIREWEAPON:
        sprintf(buf, "Bullet number is %d\n\r", obj->obj_flags.value[0]);
        send_to_char(buf, ch);
        sprintf(buf, "Damage Dice is %dD%d\n\r", obj->obj_flags.value[1],
          obj->obj_flags.value[2]);
        break;

      case ITEM_ARMOR:
        sprintf(buf, "AC-apply is %d\n\r", obj->obj_flags.value[0]);
        send_to_char(buf, ch);
        break;
    }

    found = 0;

    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if ((obj->affected[i].location != APPLY_NONE) &&
          (obj->affected[i].modifier != 0)) {
        if (found == 0) {
          send_to_char("Can affect you as :\n\r", ch);
          found = 1;
        }

        sprinttype(obj->affected[i].location, apply_types, buf2);
        sprintf(buf, "    Affects : %s By %lu\n\r", buf2,
          obj->affected[i].modifier);
        send_to_char(buf, ch);
      }
    }

  } else { /* victim */

    if (!IS_NPC(victim)) {
      sprintf(buf, "%d Years,  %d Months,  %d Days,  %d Hours old.\n\r",
        age(victim).year, age(victim).month, age(victim).day,
        age(victim).hours);
      send_to_char(buf, ch);

      sprintf(buf, "Height %dcm  Weight %dpounds \n\r", GET_HEIGHT(victim),
        GET_WEIGHT(victim));
      send_to_char(buf, ch);

      sprintf(buf, "%s is %s\n\r", GET_NAME(victim),
        ac_for_score(GET_AC(victim)));
      send_to_char(buf, ch);

      /*
   sprintf(buf,"Str %d/%d,  Int %d,  Wis %d,  Dex %d,  Con %d\n\r",
   GET_STR(victim), GET_ADD(victim),
   GET_INT(victim),
   GET_WIS(victim),
   GET_DEX(victim),
   GET_CON(victim) );
   send_to_char(buf,ch);
   */

    } else {
      send_to_char("You learn nothing new.\n\r", ch);
    }
  }
}

void cast_identify(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SCROLL:
      spell_identify(ch, tar_ch, tar_obj);
      break;
    default:
      vlog("Serious screw-up in identify!");
      break;
  }
}

#define MAX_BREATHS 3

struct breath_potion {
    int vnum;
    int spell[MAX_BREATHS];
} breath_potions[] = {
  {3970, {201, 0}},
  {3971, {202, 0}},
  {3972, {203, 0}},
  {3973, {204, 0}},
  {3974, {205, 0}},
  {0},
};

void cast_dragon_breath(signed char level, struct char_data* ch,
  const char* /*arg*/, int /*type*/, struct char_data* /*tar_ch*/,
  struct obj_data* potion) {
  struct breath_potion* scan = nullptr;
  int i = 0;
  struct affected_type af{};

  for (scan = breath_potions;
    (scan->vnum != 0) && scan->vnum != obj_index[potion->item_number].vnum;
    scan++) {
    ;
  }
  if (scan->vnum == 0) {
    char buf[MAX_STRING_LENGTH];
    send_to_char("Hey, this potion isn't in my list!\n\r", ch);
    sprintf(buf, "unlisted breath potion %s %d", potion->short_description,
      obj_index[potion->item_number].vnum);
    vlog(buf);
    return;
  }

  for (i = 0; i < MAX_BREATHS && (scan->spell[i] != 0); i++) {
    if (affected_by_spell(ch, scan->spell[i]) == 0) {
      af.type = scan->spell[i];
      af.duration = 1 + dice(1, 2);
      if (GET_CON(ch) < 4) {
        send_to_char(
          "You are too weak to stomach the potion and spew it all over the "
          "floor.\n\r",
          ch);
        act("$n gags and pukes glowing goop all over the floor.", 0, ch,
          nullptr, ch, TO_NOTVICT);
        break;
      }
      if (level > MIN(GET_CON(ch) - 1, GetMaxLevel(ch))) {
        send_to_char(
          "!GACK! You are too weak to handle the full power of the potion.\n\r",
          ch);
        act("$n gags and flops around on the floor a bit.", 0, ch, nullptr, ch,
          TO_NOTVICT);
        level = MIN(GET_CON(ch) - 1, GetMaxLevel(ch));
      }
      af.modifier = -level;
      af.location = APPLY_CON;
      af.bitvector = 0;
      affect_to_char(ch, &af);
      send_to_char("You feel powerful forces build within your stomach...\n\r",
        ch);
    }
  }
}

void cast_knock(signed char /*level*/, struct char_data* ch, const char* arg,
  int type, struct char_data* /*tar_ch*/, struct obj_data* /*tar_obj*/) {
  int door = 0;
  int other_room = 0;
  char dir[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char otype[MAX_INPUT_LENGTH];
  struct room_direction_data* back = nullptr;
  struct obj_data* obj = nullptr;
  struct char_data* victim = nullptr;

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND: {
      argument_interpreter(arg, otype, dir);

      if (otype[0] == 0) {
        send_to_char("Knock on what?\n\r", ch);
        return;
      }

      if (generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj) !=
          0) {
        if (obj->obj_flags.type_flag != ITEM_CONTAINER) {
          sprintf(buf, " %s is not a container.\n\r ", obj->name);
        } else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) {
          sprintf(buf, " Silly! %s isn't even closed!\n\r ", obj->name);
        } else if (obj->obj_flags.value[2] < 0) {
          sprintf(buf, "%s doesn't have a lock...\n\r", obj->name);
        } else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED)) {
          sprintf(buf, "Hehe.. %s wasn't even locked.\n\r", ch);
        } else if (IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF)) {
          sprintf(buf, "%s resists your magic.\n\r", obj->name);
        } else {
          REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
          sprintf(buf, "<Click>\n\r");
          act("$n magically opens $p", 0, ch, obj, nullptr, TO_ROOM);
        }
        send_to_char(buf, ch);
        return;
      }
      if ((door = find_door(ch, otype, dir)) >= 0) {
        if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR)) {
          send_to_char("That's absurd.\n\r", ch);
        } else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
          send_to_char("You realize that the door is already open.\n\r", ch);
        } else if (EXIT(ch, door)->key < 0) {
          send_to_char("You can't seem to spot any lock to pick.\n\r", ch);
        } else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED)) {
          send_to_char("Oh.. it wasn't locked at all.\n\r", ch);
        } else if (IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF)) {
          send_to_char("You seem to be unable to knock this...\n\r", ch);
        } else {
          REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
          if (EXIT(ch, door)->keyword) {
            act("$n magically opens the lock of the $F.", 0, ch, nullptr,
              EXIT(ch, door)->keyword, TO_ROOM);
          } else {
            act("$n magically opens the lock.", 1, ch, nullptr, nullptr,
              TO_ROOM);
          }
          send_to_char("The lock quickly yields to your skills.\n\r", ch);
          if ((other_room = EXIT(ch, door)->to_room) != NOWHERE) {
            if ((back = real_roomp(other_room)->dir_option[rev_dir[door]]) !=
                nullptr) {
              if (back->to_room == ch->in_room) {
                REMOVE_BIT(back->exit_info, EX_LOCKED);
              }
            }
          }
        }
      }
    } break;
    default:
      vlog("serious error in Knock.");
      break;
  }
}

static void spell_know_alignment(struct char_data* ch,
  struct char_data* victim) {
  int ap = 0;
  char buf[200];
  char name[100];

  assert(victim && ch);

  if (IS_NPC(victim)) {
    strcpy(name, victim->player.short_descr);
  } else {
    strcpy(name, GET_NAME(victim));
  }

  ap = GET_ALIGNMENT(victim);

  if (ap > 700) {
    sprintf(buf, "%s has an aura as white as the driven snow.\n\r", name);
  } else if (ap > 350) {
    sprintf(buf, "%s is of excellent moral character.\n\r", name);
  } else if (ap > 100) {
    sprintf(buf, "%s is often kind and thoughtful.\n\r", name);
  } else if (ap > 25) {
    sprintf(buf, "%s isn't a bad sort...\n\r", name);
  } else if (ap > -25) {
    sprintf(buf, "%s doesn't seem to have a firm moral commitment\n\r", name);
  } else if (ap > -100) {
    sprintf(buf, "%s isn't the worst you've come across\n\r", name);
  } else if (ap > -350) {
    sprintf(buf, "%s could be a little nicer, but who couldn't?\n\r", name);
  } else if (ap > -700) {
    sprintf(buf, "%s probably just had a bad childhood\n\r", name);
  } else {
    sprintf(buf, "I'd rather just not say anything at all about %s\n\r", name);
  }

  send_to_char(buf, ch);
}

void cast_know_alignment(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_know_alignment(ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_know_alignment(ch, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_know_alignment(ch, tar_ch);
      break;
    default:
      vlog("Serious screw-up in know alignment!");
      break;
  }
}

static void spell_weakness(signed char level, struct char_data* ch,
  struct char_data* victim) {
  struct affected_type af{};
  float modifier = NAN;

  assert(ch && victim);

  if (affected_by_spell(victim, SPELL_WEAKNESS) == 0) {
    if (saves_spell(victim, SAVING_SPELL) == 0) {
      modifier = (77.0 - level) / 100.0;
      act("You feel weaker.", 0, victim, nullptr, nullptr, TO_VICT);
      act("$n seems weaker.", 0, victim, nullptr, nullptr, TO_ROOM);

      af.type = SPELL_WEAKNESS;
      af.duration = (int)level / 2;
      af.modifier = 0 - (victim->abilities.str * modifier);
      if (victim->abilities.str_add != 0) {
        af.modifier -= 2;
      }
      af.location = APPLY_STR;
      af.bitvector = 0;

      affect_to_char(victim, &af);
    }
  }
}

void cast_weakness(signed char level, struct char_data* ch, const char* /*arg*/,
  int type, struct char_data* tar_ch, struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
      spell_weakness(level, ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_weakness(level, ch, ch);
      break;
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
    default:
      vlog("Serious screw-up in weakness!");
      break;
  }
}

void spell_dispel_magic(signed char level, struct char_data* ch,
  struct char_data* victim, struct obj_data* /*obj*/) {
  int yes = 0;

  assert(ch && victim);

  /* gets rid of infravision, invisibility, detect, etc */

  if (GetMaxLevel(victim) > GetMaxLevel(ch)) {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  if (GetMaxLevel(victim) <= GetMaxLevel(ch)) {
    yes = 1;
  } else {
    yes = 0;
  }

  if (affected_by_spell(victim, SPELL_INVISIBLE) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_INVISIBLE);
      send_to_char("You feel exposed.\n\r", victim);
    }
  }
  if (affected_by_spell(victim, SPELL_DETECT_INVISIBLE) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_DETECT_INVISIBLE);
      send_to_char("You feel less perceptive.\n\r", victim);
    }
  }
  if (affected_by_spell(victim, SPELL_DETECT_EVIL) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_DETECT_EVIL);
      send_to_char("You feel less morally alert.\n\r", victim);
    }
  }
  if (affected_by_spell(victim, SPELL_DETECT_MAGIC) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_DETECT_MAGIC);
      send_to_char("You stop noticing the magic in your life.\n\r", victim);
    }
  }
  if (affected_by_spell(victim, SPELL_SILENCE) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_SILENCE);
      send_to_char("You can speak again.\n\r", victim);
    }
  }
  if (affected_by_spell(victim, SPELL_SENSE_LIFE) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_SENSE_LIFE);
      send_to_char("You feel less in touch with living things.\n\r", victim);
    }
  }
  if (affected_by_spell(victim, SPELL_SANCTUARY) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_SANCTUARY);
      send_to_char("You don't feel so invulnerable anymore.\n\r", victim);
      act("The white glow around $n's body fades.", 0, victim, nullptr, nullptr,
        TO_ROOM);
    }
    /*
     *  aggressive Act.
     */
    if ((victim->attackers < 6) && (victim->specials.fighting == nullptr) &&
        (IS_NPC(victim))) {
      set_fighting(victim, ch);
    }
  }
  if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      REMOVE_BIT(victim->specials.affected_by, AFF_SANCTUARY);
      send_to_char("You don't feel so invulnerable anymore.\n\r", victim);
      act("The white glow around $n's body fades.", 0, victim, nullptr, nullptr,
        TO_ROOM);
    }
    /*
     *  aggressive Act.
     */
    if ((victim->attackers < 6) && (victim->specials.fighting == nullptr) &&
        (IS_NPC(victim))) {
      set_fighting(victim, ch);
    }
  }
  if (affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_PROTECT_FROM_EVIL);
      send_to_char("You feel less morally protected.\n\r", victim);
    }
  }
  if (affected_by_spell(victim, SPELL_INFRAVISION) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_INFRAVISION);
      send_to_char("Your sight grows dimmer.\n\r", victim);
    }
  }
  if (affected_by_spell(victim, SPELL_SLEEP) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_SLEEP);
      send_to_char("You don't feel so tired.\n\r", victim);
    }
  }
  if (affected_by_spell(victim, SPELL_CHARM_PERSON) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_CHARM_PERSON);
      send_to_char("You feel less enthused about your master.\n\r", victim);
    }
  }
  if (affected_by_spell(victim, SPELL_WEAKNESS) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_WEAKNESS);
      send_to_char("You don't feel so weak.\n\r", victim);
    }
  }
  if (affected_by_spell(victim, SPELL_STRENGTH) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_STRENGTH);
      send_to_char("You don't feel so strong.\n\r", victim);
    }
  }

  if (affected_by_spell(victim, SPELL_ARMOR) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_ARMOR);
      send_to_char("You don't feel so well protected.\n\r", victim);
    }
  }
  if (affected_by_spell(victim, SPELL_DETECT_POISON) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_DETECT_POISON);
      send_to_char("You don't feel so sensitive to fumes.\n\r", victim);
    }
  }

  if (affected_by_spell(victim, SPELL_BLESS) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_BLESS);
      send_to_char("You don't feel so blessed.\n\r", victim);
    }
  }

  if (affected_by_spell(victim, SPELL_FLY) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_FLY);
      send_to_char("You don't feel lighter than air anymore.\n\r", victim);
    }
  }

  if (affected_by_spell(victim, SPELL_WATER_BREATH) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_WATER_BREATH);
      send_to_char("You don't feel so fishy anymore.\n\r", victim);
    }
  }

  if (affected_by_spell(victim, SPELL_FIRE_BREATH) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_FIRE_BREATH);
      send_to_char("You don't feel so fiery anymore.\n\r", victim);
    }
  }
  if (affected_by_spell(victim, SPELL_LIGHTNING_BREATH) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_LIGHTNING_BREATH);
      send_to_char("You don't feel so electric anymore.\n\r", victim);
    }
  }
  if (affected_by_spell(victim, SPELL_GAS_BREATH) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_GAS_BREATH);
      send_to_char("You don't have gas anymore.\n\r", victim);
    }
  }
  if (affected_by_spell(victim, SPELL_FROST_BREATH) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_FROST_BREATH);
      send_to_char("You don't feel so frosty anymore.\n\r", victim);
    }
  }
  if (affected_by_spell(victim, SPELL_FIRESHIELD) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_FIRESHIELD);
      send_to_char("You don't feel so firey anymore.\n\r", victim);
      act("The red glow around $n's body fades.", 1, ch, nullptr, nullptr,
        TO_ROOM);
    }
  }
  if (affected_by_spell(victim, SPELL_FAERIE_FIRE) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_FAERIE_FIRE);
      send_to_char("You don't feel so pink anymore.\n\r", victim);
      act("The pink glow around $n's body fades.", 1, ch, nullptr, nullptr,
        TO_ROOM);
    }
  }
  if (affected_by_spell(victim, SPELL_MINOR_TRACK) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_MINOR_TRACK);
      send_to_char("You lose the trail.\n\r", victim);
    }
  }

  if (affected_by_spell(victim, SPELL_MAJOR_TRACK) != 0) {
    if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
      affect_from_char(victim, SPELL_MAJOR_TRACK);
      send_to_char("You lose the trail.\n\r", victim);
    }
  }

  if (affected_by_spell(victim, SPELL_WEB) != 0) {
    affect_from_char(victim, SPELL_WEB);
    send_to_char("You don't feel so sticky anymore.\n\r", victim);
  }

  if (level == IMPLEMENTOR) {
    if (affected_by_spell(victim, SPELL_BLINDNESS) != 0) {
      if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
        affect_from_char(victim, SPELL_BLINDNESS);
        send_to_char("Your vision returns.\n\r", victim);
      }
    }
    if (affected_by_spell(victim, SPELL_PARALYSIS) != 0) {
      if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
        affect_from_char(victim, SPELL_PARALYSIS);
        send_to_char("You feel freedom of movement.\n\r", victim);
      }
    }
    if (affected_by_spell(victim, SPELL_POISON) != 0) {
      if ((yes != 0) || (saves_spell(victim, SAVING_SPELL) == 0)) {
        affect_from_char(victim, SPELL_POISON);
      }
    }
  }
}

void cast_dispel_magic(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
      spell_dispel_magic(level, ch, tar_ch, tar_obj);
      break;
    case SPELL_TYPE_POTION:
      spell_dispel_magic(level, ch, ch, nullptr);
      break;
    case SPELL_TYPE_SCROLL:
      /*
               if(tar_obj) {
              spell_dispel_magic(level, ch, 0, tar_obj);
              return;
            }
      */
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_dispel_magic(level, ch, tar_ch, nullptr);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_dispel_magic(level, ch, tar_ch, nullptr);
        }
      }
      break;
    default:
      vlog("Serious screw-up in dispel magic");
      break;
  }
}

void spell_animate_dead(signed char level, struct char_data* ch,
  struct char_data* /*victim*/, struct obj_data* corpse) {
  struct char_data* mob = nullptr;
  struct obj_data* obj_object = nullptr;
  struct obj_data* next_obj = nullptr;
  char buf[MAX_STRING_LENGTH];
  static constexpr int r_num = 100; /* virtual # for zombie */
  int k = 0;
  /*
   some sort of check for corpse hood
  */
  if (!IS_CORPSE(corpse)) {
    send_to_char("The magic fails abruptly!\n\r", ch);
    return;
  }

  mob = read_mobile(r_num, VIRTUAL);
  char_to_room(mob, ch->in_room);

  act("With mystic power, $n animates a corpse.", 1, ch, nullptr, nullptr,
    TO_ROOM);
  act("$N slowly rises from the ground.", 0, ch, nullptr, mob, TO_ROOM);

  /*
    zombie should be charmed and follower ch
  */

  SET_BIT(mob->specials.affected_by, AFF_CHARM);
  GET_EXP(mob) = 100 * GET_LEVEL(ch, BestMagicClass(ch));
  add_follower(mob, ch);
  IS_CARRYING_W(mob) = 0;
  IS_CARRYING_N(mob) = 0;

  /*
          mob->killer = obj->killer;
  */
  /*
    take all from corpse, and give to zombie
  */

  for (obj_object = corpse->contains; obj_object != nullptr;
    obj_object = next_obj) {
    next_obj = obj_object->next_content;
    obj_from_obj(obj_object);
    obj_to_char(obj_object, mob);
  }

  /*
     set up descriptions and such
  */
  sprintf(buf, "%s is here, slowly animating\n\r", corpse->short_description);
  mob->player.long_descr = strdup(buf);

  /*
    set up hitpoints
  */

  mob->points.max_hit = dice((level + 1), 8);
  mob->points.hit = (mob->points.max_hit / 2);

  for (k = MAGE_LEVEL_IND; k <= RANGER_LEVEL_IND; k++) {
    mob->player.level[k] = ch->player.level[k];
  }
  mob->player.sex = 0;

  GET_RACE(mob) = RACE_UNDEAD;
  mob->player.char_class = ch->player.char_class;

  /*
    get rid of corpse
  */
  extract_obj(corpse);
}

void cast_animate_dead(signed char level, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*tar_ch*/,
  struct obj_data* tar_obj) {
  struct obj_data* i = nullptr;

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
      if (tar_obj != nullptr) {
        if (IS_CORPSE(tar_obj)) {
          spell_animate_dead(level, ch, nullptr, tar_obj);
        } else {
          send_to_char("That's not a corpse!\n\r", ch);
          return;
        }
      } else {
        send_to_char("That isn't a corpse!\n\r", ch);
        return;
      }
      break;
    case SPELL_TYPE_POTION:
      send_to_char("Your body revolts against the magic liquid.\n\r", ch);
      ch->points.hit = 0;
      break;
    case SPELL_TYPE_STAFF:
      for (i = real_roomp(ch->in_room)->contents; i != nullptr;
        i = i->next_content) {
        if (IS_CORPSE(i)) {
          spell_animate_dead(level, ch, nullptr, i);
        }
      }
      break;
    default:
      vlog("Serious screw-up in animate_dead!");
      break;
  }
}

static void spell_succor(struct char_data* ch) {
  struct obj_data* o = nullptr;

  o = read_object(3052, VIRTUAL);
  obj_to_char(o, ch);

  act("$n waves $s hand, and creates $p", 1, ch, o, nullptr, TO_ROOM);
  act("You wave your hand and create $p.", 1, ch, o, nullptr, TO_CHAR);
}

void cast_succor(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*tar_ch*/,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_succor(ch);
  }
}

static void spell_well_of_knowledge(struct char_data* ch) {
  struct obj_data* o = nullptr;

  o = read_object(3050, VIRTUAL);
  obj_to_char(o, ch);

  act("$n waves $s hand, and creates $p", 1, ch, o, nullptr, TO_ROOM);
  act("You wave your hand and create $p.", 1, ch, o, nullptr, TO_CHAR);
}

void cast_well_of_knowledge(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*victim*/,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_well_of_knowledge(ch);
  }
}

static void paralyze_fail(struct char_data* victim, struct char_data* ch) {
  send_to_char("You feel frozen for a moment, but then you recover\n\r",
    victim);
  if (IS_NPC(victim)) {
    if (victim->specials.fighting == nullptr) {
      set_fighting(victim, ch);
    }
  }
}

static void spell_paralyze(struct char_data* ch, struct char_data* victim) {
  struct affected_type af{};

  assert(victim);

  if (!IS_AFFECTED(victim, AFF_PARALYSIS)) {
    if (IsImmune(victim, IMM_HOLD) != 0) {
      paralyze_fail(victim, ch);
      return;
    }
    if (IsResist(victim, IMM_HOLD) != 0) {
      if (saves_spell(victim, SAVING_PARA) != 0) {
        paralyze_fail(victim, ch);
        return;
      }
      if (saves_spell(victim, SAVING_PARA) != 0) {
        paralyze_fail(victim, ch);
        return;
      }
    } else if (IsSusc(victim, IMM_HOLD) == 0) {
      if (saves_spell(victim, SAVING_PARA) != 0) {
        paralyze_fail(victim, ch);
        return;
      }
    }

    af.type = SPELL_PARALYSIS;
    af.duration = 1;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_PARALYSIS;
    affect_join(victim, &af, 0, 0);

    act("Your limbs freeze in place", 0, victim, nullptr, nullptr, TO_CHAR);
    act("$n is paralyzed!", 1, victim, nullptr, nullptr, TO_ROOM);
    GET_POS(victim) = POSITION_STUNNED;

  } else {
    send_to_char("Someone tries to paralyze you AGAIN!\n\r", victim);
  }
}

void cast_paralyze(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_paralyze(ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_paralyze(ch, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_paralyze(ch, tar_ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj != nullptr) {
        return;
      }
      spell_paralyze(ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (tar_ch != ch) {
          spell_paralyze(ch, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in paralyze");
      break;
  }
}

static void spell_fear(struct char_data* ch, struct char_data* victim) {
  assert(victim && ch);

  if (GetMaxLevel(ch) >= GetMaxLevel(victim) - 2) {
    if (saves_spell(victim, SAVING_SPELL) == 0) {
      /*
              af.type      = SPELL_FEAR;
              af.duration  = 4+level;
              af.modifier  = 0;
              af.location  = APPLY_NONE;
         af.bitvector = 0;

              affect_join(victim, &af, false, false);
      */
      do_flee(victim, "", 0);

    } else {
      send_to_char("You feel afraid, but the effect fades.\n\r", victim);
      return;
    }
  }
}

void cast_fear(signed char /*level*/, struct char_data* ch, const char* /*arg*/,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_fear(ch, tar_ch);
      break;
    case SPELL_TYPE_POTION:
      spell_fear(ch, ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_fear(ch, tar_ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_fear(ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (in_group(tar_ch, ch) == 0) {
          spell_fear(ch, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in fear");
      break;
  }
}

static void spell_turn(signed char level, struct char_data* ch,
  struct char_data* victim) {
  int diff = 0;
  int i = 0;

  assert(ch && victim);
  assert((level >= 1) && (level <= ABS_MAX_LVL));

  if (IsUndead(victim) != 0) {
    diff = level - GetTotLevel(victim);
    if (diff <= 0) {
      act("You are powerless to affect $N", 1, ch, nullptr, victim, TO_CHAR);
      return;
    }
    for (i = 1; i <= diff; i++) {
      if (saves_spell(victim, SAVING_SPELL) == 0) {
        act("$n forces $N from this room.", 1, ch, nullptr, victim, TO_NOTVICT);
        act("You force $N from this room.", 1, ch, nullptr, victim, TO_CHAR);
        act("$n forces you from this room.", 1, ch, nullptr, victim, TO_VICT);
        do_flee(victim, "", 0);
        break;
      }
    }
    if (i < diff) {
      act("You laugh at $n.", 1, ch, nullptr, victim, TO_VICT);
      act("$N laughs at $n.", 1, ch, nullptr, victim, TO_NOTVICT);
      act("$N laughs at you.", 1, ch, nullptr, victim, TO_CHAR);
    }

  } else {
    act("$n just tried to turn you, what a moron!", 1, ch, nullptr, victim,
      TO_VICT);
    act("$N thinks $n is really strange.", 1, ch, nullptr, victim, TO_NOTVICT);
    act("Um... $N isn't undead...", 1, ch, nullptr, victim, TO_CHAR);
  }
}

void cast_turn(signed char level, struct char_data* ch, const char* /*arg*/,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj) {
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_turn(level, ch, tar_ch);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_turn(level, ch, tar_ch);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj != nullptr) {
        return;
      }
      if (tar_ch == nullptr) {
        tar_ch = ch;
      }
      spell_turn(level, ch, tar_ch);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = real_roomp(ch->in_room)->people; tar_ch != nullptr;
        tar_ch = tar_ch->next_in_room) {
        if (in_group(tar_ch, ch) == 0) {
          spell_turn(level, ch, tar_ch);
        }
      }
      break;
    default:
      vlog("Serious screw-up in turn");
      break;
  }
}

static void spell_faerie_fog(struct char_data* ch) {
  struct char_data* tmpv = nullptr;

  assert(ch);

  act("$n snaps $s fingers, and a cloud of purple smoke billows forth", 1, ch,
    nullptr, nullptr, TO_ROOM);
  act("You snap your fingers, and a cloud of purple smoke billows forth", 1, ch,
    nullptr, nullptr, TO_CHAR);

  for (tmpv = real_roomp(ch->in_room)->people; tmpv != nullptr;
    tmpv = tmpv->next_in_room) {
    if ((ch->in_room == tmpv->in_room) && (ch != tmpv)) {
      if (IS_IMMORTAL(tmpv)) {
        break;
      }
      if (in_group(ch, tmpv) == 0) {
        if (IS_AFFECTED(tmpv, AFF_INVISIBLE)) {
          if (saves_spell(tmpv, SAVING_SPELL) != 0) {
            REMOVE_BIT(tmpv->specials.affected_by, AFF_INVISIBLE);
            act("$n is briefly revealed, but dissapears again.", 1, tmpv,
              nullptr, nullptr, TO_ROOM);
            act("You are briefly revealed, but dissapear again.", 1, tmpv,
              nullptr, nullptr, TO_CHAR);
            SET_BIT(tmpv->specials.affected_by, AFF_INVISIBLE);
          } else {
            REMOVE_BIT(tmpv->specials.affected_by, AFF_INVISIBLE);
            act("$n is revealed!", 1, tmpv, nullptr, nullptr, TO_ROOM);
            act("You are revealed!", 1, tmpv, nullptr, nullptr, TO_CHAR);
          }
        }
      }
    }
  }
}

void cast_faerie_fog(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*victim*/,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
      spell_faerie_fog(ch);
      break;
    default:
      vlog("Serious screw-up in faerie fog!");
      break;
  }
}

static void spell_poly_self(struct char_data* ch, struct char_data* mob) {
  char* buf = nullptr;

  /*
   *  Check to make sure that there is no snooping going on.
   */
  if ((ch->desc == nullptr) || (ch->desc->snoop.snooping != nullptr)) {
    send_to_char("Godly interference prevents the spell from working.", ch);
    extract_char(mob);
    return;
  }

  if (ch->desc->snoop.snoop_by != nullptr) { /* force the snooper to stop */
    do_snoop(ch->desc->snoop.snoop_by, GET_NAME(ch->desc->snoop.snoop_by), 0);
  }

  /*
   * Put mobile in correct room
   */

  char_to_room(mob, ch->in_room);

  SwitchStuff(ch, mob);

  /*
   *  move char to storage
   */

  if ((IS_SET(ch->specials.act, PLR_ANSI)) ||
      (IS_SET(ch->specials.act, PLR_VT100))) {
    do_terminal(ch, "none", 0);
  }

  act("$n's flesh melts and flows into the shape of $N", 1, ch, nullptr, mob,
    TO_ROOM);

  act("Your flesh melts and flows into the shape of $N", 1, ch, nullptr, mob,
    TO_CHAR);

  char_from_room(ch);
  char_to_room(ch, 3);

  /*
    stop following whoever you are following..
   */
  if (ch->master != nullptr) {
    stop_follower(ch);
  }

  /*
   *  switch caster into mobile
   */

  ch->desc->character = mob;
  ch->desc->original = ch;

  mob->desc = ch->desc;
  ch->desc = nullptr;

  SET_BIT(mob->specials.act, ACT_POLYSELF);
  SET_BIT(mob->specials.act, ACT_NICE_THIEF);
  SET_BIT(mob->specials.act, ACT_SENTINEL);
  REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
  REMOVE_BIT(mob->specials.act, ACT_SCAVENGER);

  GET_MANA(mob) = MIN((GET_MANA(mob) - 15), 85);
  WAIT_STATE(mob, PULSE_VIOLENCE * 2);

  /* do some fiddling with the strings */
  buf = (char*)malloc(strlen(GET_NAME(mob)) + strlen(GET_NAME(ch)) + 2);
  sprintf(buf, "%s %s", GET_NAME(ch), GET_NAME(mob));

#if TITAN
#else
  /* this code crashes ardent titans */
  if (GET_NAME(mob)) {
    free(GET_NAME(mob));
  }
#endif

  GET_NAME(mob) = buf;
  buf =
    (char*)malloc(strlen(mob->player.short_descr) + strlen(GET_NAME(ch)) + 2);
  sprintf(buf, "%s %s", GET_NAME(ch), mob->player.short_descr);

#if TITAN
  if (mob->player.short_descr)
    free(mob->player.short_descr);
#endif
  mob->player.short_descr = buf;

  buf = (char*)malloc(strlen(mob->player.short_descr) + 12);
  sprintf(buf, "%s is here\n\r", mob->player.short_descr);

#if TITAN
#else
  if (mob->player.long_descr != nullptr) {
    free(mob->player.long_descr);
  }
#endif

  mob->player.long_descr = buf;
}

void cast_poly_self(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* /*tar_ch*/, struct obj_data* /*tar_obj*/) {
  char buffer[40];
  int mobn = 0;
  int x = LAST_POLY_MOB;
  int found = 0;
  struct char_data* mob = nullptr;

  one_argument(arg, buffer);

  if (IS_NPC(ch)) {
    send_to_char("You don't really want to do that.\n\r", ch);
    return;
  }

  switch (type) {
    case SPELL_TYPE_SPELL: {
      while (found == 0) {
        if (poly_list[x].level > level) {
          x--;
        } else {
          if (str_cmp(poly_list[x].name, buffer) == 0) {
            mobn = poly_list[x].number;
            found = 1;
          } else {
            x--;
          }
          if (x < 0) {
            break;
          }
        }
      }

      if (found == 0) {
        send_to_char("Couldn't find any of those\n\r", ch);
        return;
      }
      mob = read_mobile(mobn, VIRTUAL);
      if (mob != nullptr) {
        spell_poly_self(ch, mob);
      } else {
        send_to_char("You couldn't summon an image of that creature\n\r", ch);
      }
      return;

    } break;

    default: {
      vlog("Problem in poly_self");
    } break;
  }
}

#define LONG_SWORD 3022
#define SHIELD 3042
#define RAFT 3060
#define BAG 3032
#define WATER_BARREL 6013
#define BREAD 3010

static void spell_minor_create(struct char_data* ch, struct obj_data* obj) {
  assert(ch && obj);

  act("$n claps $s hands together.", 1, ch, nullptr, nullptr, TO_ROOM);
  act("You clap your hands together.", 1, ch, nullptr, nullptr, TO_CHAR);
  act("In a flash of light, $p appears.", 1, ch, obj, nullptr, TO_ROOM);
  act("In a flash of light, $p appears.", 1, ch, obj, nullptr, TO_CHAR);

  obj_to_room(obj, ch->in_room);
}

void cast_minor_creation(signed char /*level*/, struct char_data* ch,
  const char* arg, int type, struct char_data* /*tar_ch*/,
  struct obj_data* /*tar_obj*/) {
  char buffer[40];
  int obj = 0;
  struct obj_data* o = nullptr;

  one_argument(arg, buffer);

  if (str_cmp(buffer, "sword") == 0) {
    obj = LONG_SWORD;
  } else if (str_cmp(buffer, "shield") == 0) {
    obj = SHIELD;
  } else if (str_cmp(buffer, "raft") == 0) {
    obj = RAFT;
  } else if (str_cmp(buffer, "bag") == 0) {
    obj = BAG;
  } else if (str_cmp(buffer, "barrel") == 0) {
    obj = WATER_BARREL;
  } else if (str_cmp(buffer, "bread") == 0) {
    obj = BREAD;
  } else {
    send_to_char("There is nothing of that available\n\r", ch);
    return;
  }

  o = read_object(obj, VIRTUAL);
  if (o == nullptr) {
    send_to_char("There is nothing of that available\n\r", ch);
    return;
  }

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
      spell_minor_create(ch, o);
      break;
    default:
      vlog("serious screw-up in minor_create.");
      break;
  }
}

#define FIRE_ELEMENTAL 10
#define WATER_ELEMENTAL 11
#define AIR_ELEMENTAL 13
#define EARTH_ELEMENTAL 12

#define RED_STONE 5233
#define PALE_BLUE_STONE 5230
#define GREY_STONE 5239
#define CLEAR_STONE 5243

static void spell_conjure_elemental(struct char_data* ch,
  struct char_data* victim, struct obj_data* obj) {
  struct affected_type af{};

  /*
   *   victim, in this case, is the elemental
   *   object could be the sacrificial object
   */

  assert(ch && victim && obj);

  /*
  ** objects:
  **     fire  : red stone
  **     water : pale blue stone
  **     earth : grey stone
  **     air   : clear stone
  */

  act("$n gestures, and a cloud of smoke appears", 1, ch, nullptr, nullptr,
    TO_ROOM);
  act("$n gestures, and a cloud of smoke appears", 1, ch, nullptr, nullptr,
    TO_CHAR);
  act("$p explodes with a loud BANG!", 1, ch, obj, nullptr, TO_ROOM);
  act("$p explodes with a loud BANG!", 1, ch, obj, nullptr, TO_CHAR);
  obj_from_char(obj);
  extract_obj(obj);
  char_to_room(victim, ch->in_room);
  act("Out of the smoke, $N emerges", 1, ch, nullptr, victim, TO_NOTVICT);

  /* charm them for a while */
  if (victim->master != nullptr) {
    stop_follower(victim);
  }

  add_follower(victim, ch);

  af.type = SPELL_CHARM_PERSON;
  af.duration = 48;
  af.modifier = 0;
  af.location = 0;
  af.bitvector = AFF_CHARM;

  affect_to_char(victim, &af);
}

void cast_conjure_elemental(signed char /*level*/, struct char_data* ch,
  const char* arg, int type, struct char_data* /*tar_ch*/,
  struct obj_data* /*tar_obj*/) {
  char buffer[40];
  int mob = 0;
  int obj = 0;
  struct obj_data* sac = nullptr;
  struct char_data* el = nullptr;

  one_argument(arg, buffer);

  if (str_cmp(buffer, "fire") == 0) {
    mob = FIRE_ELEMENTAL;
    obj = RED_STONE;
  } else if (str_cmp(buffer, "water") == 0) {
    mob = WATER_ELEMENTAL;
    obj = PALE_BLUE_STONE;
  } else if (str_cmp(buffer, "air") == 0) {
    mob = AIR_ELEMENTAL;
    obj = CLEAR_STONE;
  } else if (str_cmp(buffer, "earth") == 0) {
    mob = EARTH_ELEMENTAL;
    obj = GREY_STONE;
  } else {
    send_to_char("There are no elementals of that type available\n\r", ch);
    return;
  }
  if (ch->equipment[HOLD] == nullptr) {
    send_to_char(" You must be holding the correct stone\n\r", ch);
    return;
  }

  sac = unequip_char(ch, HOLD);
  if (sac != nullptr) {
    obj_to_char(sac, ch);
    if (ObjVnum(sac) != obj) {
      send_to_char("You must have the correct item to sacrifice.\n\r", ch);
      return;
    }
    el = read_mobile(mob, VIRTUAL);
    if (el == nullptr) {
      send_to_char("There are no elementals of that type available\n\r", ch);
      return;
    }
  } else {
    send_to_char("You must be holding the correct item to sacrifice.\n\r", ch);
    return;
  }

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
      spell_conjure_elemental(ch, el, sac);
      break;
    default:
      vlog("serious screw-up in conjure_elemental.");
      break;
  }
}

#define DEMON_TYPE_I 20
#define DEMON_TYPE_II 21
#define DEMON_TYPE_III 22
#define DEMON_TYPE_IV 23
#define DEMON_TYPE_V 24
#define DEMON_TYPE_VI 25

#define SWORD_ANCIENTS 25000
#define SHADOWSHIV 25014
#define FIRE_SWORD 25015
#define SILVER_TRIDENT 25016
#define JEWELLED_DAGGER 25019
#define SWORD_SHARPNESS 25017

static void spell_cacaodemon(struct char_data* ch, struct char_data* victim,
  struct obj_data* obj) {
  struct affected_type af{};

  assert(ch && victim && obj);

  act("$n gestures, and a black cloud of smoke appears", 1, ch, nullptr,
    nullptr, TO_ROOM);
  act("$n gestures, and a black cloud of smoke appears", 1, ch, nullptr,
    nullptr, TO_CHAR);
  act("$p bursts into flame and disintegrates!", 1, ch, obj, nullptr, TO_ROOM);
  act("$p bursts into flame and disintegrates!", 1, ch, obj, nullptr, TO_CHAR);
  obj_from_char(obj);
  extract_obj(obj);
  char_to_room(victim, ch->in_room);

  act("With an evil laugh, $N emerges from the smoke", 1, ch, nullptr, victim,
    TO_NOTVICT);

  /* charm them for a while */
  if (victim->master != nullptr) {
    stop_follower(victim);
  }

  add_follower(victim, ch);

  af.type = SPELL_CHARM_PERSON;
  af.duration = 48;
  af.modifier = 0;
  af.location = 0;
  af.bitvector = AFF_CHARM;

  affect_to_char(victim, &af);

  if (IS_SET(victim->specials.act, ACT_AGGRESSIVE)) {
    REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
  }

  if (!IS_SET(victim->specials.act, ACT_SENTINEL)) {
    SET_BIT(victim->specials.act, ACT_SENTINEL);
  }
}

void cast_cacaodemon(signed char /*level*/, struct char_data* ch,
  const char* arg, int type, struct char_data* /*tar_ch*/,
  struct obj_data* /*tar_obj*/) {
  char buffer[40];
  int mob = 0;
  int obj = 0;
  struct obj_data* sac = nullptr;
  struct char_data* el = nullptr;

  one_argument(arg, buffer);

  if (str_cmp(buffer, "one") == 0) {
    mob = DEMON_TYPE_I;
    obj = SWORD_SHARPNESS;
  } else if (str_cmp(buffer, "two") == 0) {
    mob = DEMON_TYPE_II;
    obj = JEWELLED_DAGGER;
  } else if (str_cmp(buffer, "three") == 0) {
    mob = DEMON_TYPE_III;
    obj = SILVER_TRIDENT;
  } else if (str_cmp(buffer, "four") == 0) {
    mob = DEMON_TYPE_IV;
    obj = FIRE_SWORD;
  } else if (str_cmp(buffer, "five") == 0) {
    mob = DEMON_TYPE_V;
    obj = SHADOWSHIV;
  } else if (str_cmp(buffer, "six") == 0) {
    mob = DEMON_TYPE_VI;
    obj = SWORD_ANCIENTS;
  } else {
    send_to_char("There are no demons of that type available\n\r", ch);
    return;
  }
  if (ch->equipment[WIELD] == nullptr) {
    send_to_char(" You must be wielding the correct item\n\r", ch);
    return;
  }

  if (obj_index[ch->equipment[WIELD]->item_number].vnum != obj) {
    send_to_char(" You must be wielding the correct item\n\r", ch);
    return;
  }

  sac = unequip_char(ch, WIELD);
  if (sac != nullptr) {
    obj_to_char(sac, ch);
    if (ObjVnum(sac) != obj) {
      send_to_char("You must have the correct item to sacrifice.\n\r", ch);
      return;
    }
    el = read_mobile(mob, VIRTUAL);
    if (el == nullptr) {
      send_to_char("There are no demons of that type available\n\r", ch);
      return;
    }
  } else {
    send_to_char("You must be holding the correct item to sacrifice.\n\r", ch);
    return;
  }

  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
      spell_cacaodemon(ch, el, sac);
      break;
    default:
      vlog("serious screw-up in conjure_elemental.");
      break;
  }
}

static void spell_create_monster(signed char level, struct char_data* ch) {
  struct affected_type af{};
  struct char_data* mob = nullptr;
  int rnum = 0;

  /* load in a monster of the correct type, determined by
     level of the spell */

  /* really simple to start out with */

  if (level <= 5) {
    rnum = number(1, 10) + 200;
    mob = read_mobile(rnum, VIRTUAL);
  } else if (level <= 7) {
    rnum = number(1, 10) + 210;
    mob = read_mobile(rnum, VIRTUAL);
  } else if (level <= 9) {
    rnum = number(1, 10) + 220;
    mob = read_mobile(rnum, VIRTUAL);
  } else if (level <= 11) {
    rnum = number(1, 10) + 230;
    mob = read_mobile(rnum, VIRTUAL);
  } else if (level <= 13) {
    rnum = number(1, 10) + 240;
    mob = read_mobile(rnum, VIRTUAL);
  } else if (level <= 15) {
    rnum = 251;
    mob = read_mobile(rnum, VIRTUAL);
  } else {
    rnum = 261;
    mob = read_mobile(rnum, VIRTUAL);
  }

  char_to_room(mob, ch->in_room);

  act("$n waves $s hand, and $N appears!", 1, ch, nullptr, mob, TO_ROOM);
  act("You wave your hand, and $N appears!", 1, ch, nullptr, mob, TO_CHAR);

  /* charm them for a while */
  if (mob->master != nullptr) {
    stop_follower(mob);
  }

  add_follower(mob, ch);

  af.type = SPELL_CHARM_PERSON;

  if (GET_INT(mob)) {
    af.duration = 24 * 18 / GET_INT(mob);
  } else {
    af.duration = 24 * 18;
  }

  af.modifier = 0;
  af.location = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(mob, &af);

  /*
    adjust the bits...
  */

  /*
   get rid of aggressive, add sentinel
  */

  if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
    REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
  }
  if (!IS_SET(mob->specials.act, ACT_SENTINEL)) {
    SET_BIT(mob->specials.act, ACT_SENTINEL);
  }
}

void cast_mon_sum1(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*tar_ch*/,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_create_monster(5, ch);
      break;
    default:
      vlog("Serious screw-up in monster_summoning_1");
      break;
  }
}

void cast_mon_sum2(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*tar_ch*/,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_create_monster(7, ch);
      break;
    default:
      vlog("Serious screw-up in monster_summoning_1");
      break;
  }
}

void cast_mon_sum3(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*tar_ch*/,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_create_monster(9, ch);
      break;
    default:
      vlog("Serious screw-up in monster_summoning_1");
      break;
  }
}

void cast_mon_sum4(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*tar_ch*/,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_create_monster(11, ch);
      break;
    default:
      vlog("Serious screw-up in monster_summoning_1");
      break;
  }
}

void cast_mon_sum5(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*tar_ch*/,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_create_monster(13, ch);
      break;
    default:
      vlog("Serious screw-up in monster_summoning_1");
      break;
  }
}

void cast_mon_sum6(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*tar_ch*/,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_create_monster(15, ch);
      break;
    default:
      vlog("Serious screw-up in monster_summoning_1");
      break;
  }
}

void cast_mon_sum7(signed char /*level*/, struct char_data* ch,
  const char* /*arg*/, int type, struct char_data* /*tar_ch*/,
  struct obj_data* /*tar_obj*/) {
  switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
      spell_create_monster(17, ch);
      break;
    default:
      vlog("Serious screw-up in monster_summoning_1");
      break;
  }
}
