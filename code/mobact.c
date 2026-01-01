#include "accessors.h"
#include "bit_ops.h"
#include "character_flags.h"
#include "comm.h"
#include "commands.h"
#include "constants.h"
#include "db.h"
#include "handler.h"
#include "hash.h"
#include "multiclass.h"
#include "opinion.h"
#include "room_flags.h"
#include "structs.h"
#include "trap.h"
#include "utils.h"

static void mobile_guardian(struct char_data* ch) {
  struct char_data* targ = nullptr;
  int i;
  int found = 0;

  if (ch->in_room > -1) {
    if ((!ch->master) || (!IS_AFFECTED(ch, AFF_CHARM))) {
      return;
    }
    if (ch->master->specials.fighting) { /**/
      for (i = 0; i < 10 && !found; i++) {
        targ = FindAnAttacker(ch->master);
        if (targ) {
          found = 1;
        }
      }

      if (!found) {
        return;
      }

      if (!SameRace(targ, ch)) {
        if (IsHumanoid(ch)) {
          act("$n screams 'I must protect my master!'", 0, ch, nullptr, nullptr,
            TO_ROOM);
        } else {
          act("$n growls angrily!", 0, ch, nullptr, nullptr, TO_ROOM);
        }
        if (CAN_SEE(ch, targ)) {
          hit(ch, targ, 0);
        }
      }
    }
  }
}

void mobile_wander(struct char_data* ch) {
  int door = 0;
  struct room_direction_data* exitp;
  struct room_data* rp = nullptr;

  if (!((GET_POS(ch) == POSITION_STANDING) && ((door = number(0, 15)) <= 5) &&
        exit_ok(exitp = EXIT(ch, door), &rp) &&
        (!IS_SET(rp->room_flags, NO_MOB) || IS_POLICE(ch)) &&
        !IS_SET(rp->room_flags, DEATH))) {
    return;
  }

  if (IsHumanoid(ch) ? CAN_GO_HUMAN(ch, door) : CAN_GO(ch, door)) {
    if (ch->specials.last_direction == door) {
      ch->specials.last_direction = -1;
    } else {
      if (!IS_SET(ch->specials.act, ACT_STAY_ZONE) ||
          (rp->zone == real_roomp(ch->in_room)->zone)) {
        ch->specials.last_direction = door;
        go_direction(ch, door);
      }
    }
  }
}

static void mob_hunt(struct char_data* ch) {
  int res, k;

#if NOTRACK
  return; /* too much CPU useage for some machines.  */
#endif

  if (ch->persist <= 0) {
    res = choose_exit_in_zone(ch->in_room, ch->old_room, 2000);
    if (res > -1) {
      go_direction(ch, res);
    } else {
      if (ch->specials.hunting) {
        if (ch->specials.hunting->in_room == ch->in_room) {
          if (Hates(ch, ch->specials.hunting) &&
              (!IS_AFFECTED(ch->specials.hunting, AFF_HIDE))) {
            if (check_peaceful(ch,
                  "You'd love to tear your quarry to bits, but you just "
                  "CAN'T\n\r")) {
              act("$n fumes at $N", 1, ch, nullptr, ch->specials.hunting,
                TO_ROOM);
            } else {
              if (IsHumanoid(ch)) {
                act("$n screams 'Time to die, $N'", 1, ch, nullptr,
                  ch->specials.hunting, TO_ROOM);
              } else if (IsAnimal(ch)) {
                act("$n growls.", 1, ch, nullptr, nullptr, TO_ROOM);
              }
              hit(ch, ch->specials.hunting, 0);
              return;
            }
          }
        }
      }
      REMOVE_BIT(ch->specials.act, ACT_HUNTING);
      ch->specials.hunting = nullptr;
      ch->hunt_dist = 0;
    }
  } else if (ch->specials.hunting) {
    if (ch->hunt_dist <= 50) {
      ch->hunt_dist = 100;
    }
    for (k = 1; k <= 1 && ch->specials.hunting; k++) {
      ch->persist -= 1;
      res = dir_track(ch, ch->specials.hunting);
      if (res != -1) {
        go_direction(ch, res);
      } else {
        ch->persist = 0;
        ch->specials.hunting = nullptr;
        ch->hunt_dist = 0;
      }
    }
  } else {
    ch->persist = 0;
  }
}

static void mob_scavenge(struct char_data* ch) {
  struct obj_data* best_obj = nullptr;
  struct obj_data* obj = nullptr;
  int max;

  if ((real_roomp(ch->in_room))->contents && !number(0, 5)) {
    for (max = 1, best_obj = nullptr, obj = (real_roomp(ch->in_room))->contents;
      obj; obj = obj->next_content) {
      if (CAN_GET_OBJ(ch, obj)) {
        if (obj->obj_flags.cost > max) {
          best_obj = obj;
          max = obj->obj_flags.cost;
        }
      }
    } /* for */

    if (best_obj) {
      if (CheckForAnyTrap(ch, best_obj)) {
        return;
      }

      obj_from_room(best_obj);
      obj_to_char(best_obj, ch);
      act("$n gets $p.", 0, ch, best_obj, nullptr, TO_ROOM);
    }
  }
}

/* check to see if a mob is a friend */
static int mob_friend(struct char_data* ch, struct char_data* f) {
  if (SameRace(ch, f)) {
    if (IS_GOOD(ch)) {
      if (IS_GOOD(f)) {
        return 1;
      }
      return 0;
    }
    if (IS_NPC(f)) {
      return 1;
    }

  } else {
    return 0;
  }
  return 0;
}

static int assist_friend(struct char_data* ch) {
  struct char_data* damsel;
  struct char_data* targ;
  struct char_data* tmp_ch;
  struct char_data* next;
  int t;
  int found;

  damsel = nullptr;
  targ = nullptr;

  if (check_peaceful(ch, "")) {
    return 0;
  }

  if (ch->in_room < 0) {
    char_to_room(ch, 0);
    return 0;
  }

  /*
    find the people who are fighting
    */

  for (tmp_ch = (real_roomp(ch->in_room))->people; tmp_ch; tmp_ch = next) {
    next = tmp_ch->next_in_room;
    if (CAN_SEE(ch, tmp_ch)) {
      if (!IS_SET(ch->specials.act, ACT_WIMPY)) {
        if (mob_friend(ch, tmp_ch)) {
          if (tmp_ch->specials.fighting) {
            damsel = tmp_ch;
          }
        }
      }
    }
  }

  if (damsel) {
    /*
      check if the people in the room are fighting.
      */
    found = 0;
    for (t = 1; t <= 8 && !found; t++) {
      targ = FindAnAttacker(damsel);
      if (targ) {
        if (targ->specials.fighting) {
          found = 1;
        }
      }
    }
    if (targ) {
      if (targ->in_room == ch->in_room) {
        if (!IS_AFFECTED(ch, AFF_CHARM) || ch->master != targ) {
          hit(ch, targ, 0);
        }
      }
    }
  }
  return 0;
}

void mobile_activity(struct char_data* ch) {
  struct char_data* tmp_ch;
  int k;

  /* Examine call for special procedure */

  /* some status checking for errors */
  if ((ch->in_room < 0) || !room_find(room_db, ch->in_room)) {
    vlog("Char not in correct room.  moving to 50 ");
    char_from_room(ch);
    char_to_room(ch, 50);
  }

  if (IS_SET(ch->specials.act, ACT_SPEC) && !no_specials) {
    if (!mob_index[ch->nr].func.mob_f) {
      vlog("Attempting to call a non-existing MOB func. (mobact.c)");
      vlog(ch->player.name);
      REMOVE_BIT(ch->specials.act, ACT_SPEC);
    } else {
      if ((*mob_index[ch->nr].func.mob_f)(ch, 0, "")) {
        return;
      }
    }
  }

  /* check to see if the monster is possessed */

  if (AWAKE(ch) && (!ch->specials.fighting) && (!ch->desc) &&
      (!IS_SET(ch->specials.act, ACT_POLYSELF))) {
    assist_friend(ch);

    if (IS_SET(ch->specials.act, ACT_SCAVENGER)) {
      mob_scavenge(ch);
    } /* Scavenger */

    if (IS_SET(ch->specials.act, ACT_HUNTING)) {
      mob_hunt(ch);
    } else if ((!IS_SET(ch->specials.act, ACT_SENTINEL))) {
      mobile_wander(ch);
    }

    if (GET_HIT(ch) > (GET_MAX_HIT(ch) / 2)) {
      if (IS_SET(ch->specials.act, ACT_HATEFUL)) {
        tmp_ch = FindAHatee(ch);
        if (tmp_ch) {
          if (check_peaceful(ch,
                "You ask your mortal enemy to step outside to settle "
                "matters.\n\r")) {
            act(
              "$n growls '$N, would you care to step outside where we can "
              "settle this?'",
              1, ch, nullptr, tmp_ch, TO_ROOM);
          } else {
            if (IsHumanoid(ch)) {
              act("$n screams 'I'm gonna kill you!'", 1, ch, nullptr, nullptr,
                TO_ROOM);
            } else if (IsAnimal(ch)) {
              act("$n growls", 1, ch, nullptr, nullptr, TO_ROOM);
            }
            hit(ch, tmp_ch, 0);
          }
        }
      }
      if (!ch->specials.fighting) {
        if (IS_SET(ch->specials.act, ACT_AFRAID)) {
          if ((tmp_ch = FindAFearee(ch)) != nullptr) {
            do_flee(ch, "", 0);
          }
        }
      }
    } else {
      if (IS_SET(ch->specials.act, ACT_AFRAID)) {
        if ((tmp_ch = FindAFearee(ch)) != nullptr) {
          do_flee(ch, "", 0);
        } else {
          if (IS_SET(ch->specials.act, ACT_HATEFUL)) {
            tmp_ch = FindAHatee(ch);
            if (tmp_ch) {
              if (check_peaceful(ch,
                    "You ask your mortal enemy to step outside to settle "
                    "matters.\n\r")) {
                act(
                  "$n growls '$N, would you care to step outside where we can "
                  "settle this?'",
                  1, ch, nullptr, tmp_ch, TO_ROOM);
              } else {
                if (IsHumanoid(ch)) {
                  act("$n screams 'I'm gonna get you!'", 1, ch, nullptr,
                    nullptr, TO_ROOM);
                } else if (IsAnimal(ch)) {
                  act("$n growls", 1, ch, nullptr, nullptr, TO_ROOM);
                }
                hit(ch, tmp_ch, 0);
              }
            }
          }
        }
      }
    }
    if (IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
      for (k = 0; k <= 5; k++) {
        tmp_ch = FindVictim(ch);
        if (tmp_ch) {
          if (check_peaceful(ch,
                "You can't seem to exercise your violent tendencies.\n\r")) {
            act("$n growls impotently", 1, ch, nullptr, nullptr, TO_ROOM);
            return;
          }
          hit(ch, tmp_ch, 0);
          k = 10;
        }
      }
    }
    if (IS_SET(ch->specials.act, ACT_META_AGG)) {
      for (k = 0; k <= 5; k++) {
        tmp_ch = FindMetaVictim(ch);
        if (tmp_ch) {
          if (check_peaceful(ch,
                "You can't seem to exercise your violent tendencies.\n\r")) {
            act("$n growls impotently", 1, ch, nullptr, nullptr, TO_ROOM);
            return;
          }
          hit(ch, tmp_ch, 0);
          k = 10;
        }
      }
    }
    if (IS_SET(ch->specials.act, ACT_GUARDIAN)) {
      mobile_guardian(ch);
    }

  } /* If AWAKE(ch)   */
}

int SameRace(struct char_data* ch1, struct char_data* ch2) {
  if ((!ch1) || (!ch2)) {
    return 0;
  }

  if (ch1 == ch2) {
    return 1;
  }

  if (IS_NPC(ch1) && (IS_NPC(ch2))) {
    if (mob_index[ch1->nr].vnum == mob_index[ch2->nr].vnum) {
      return 1;
    }
  }

  if (in_group(ch1, ch2)) {
    return 1;
  }

  if (GET_RACE(ch1) == GET_RACE(ch2)) {
    return 1;
  }

  return 0;
}
