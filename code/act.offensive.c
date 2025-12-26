/* ************************************************************************
 *  file: act.offensive.c , Implementation of commands.    Part of DIKUMUD *
 *  Usage : Offensive commands.                                            *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************* */

#include <stdio.h>
#include <sys/param.h>

#include "comm.h"
#include "constants.h"
#include "db.h"
#include "games.h"
#include "handler.h"
#include "interpreter.h"
#include "limits.h"
#include "multiclass.h"
#include "opinion.h"
#include "spec_procs.h"
#include "spells.h"
#include "structs.h"
#include "utils.h"

void do_hit(struct char_data* ch, char* argument, int cmd) {
  char arg[80];
  struct char_data* victim;
  int ch_level;
  int vict_level;

  if (check_blackjack(ch)) {
    do_bj_hit(ch);
    return;
  }
  if (GET_POS(ch) < POSITION_FIGHTING) {
    send_to_char("Try standing up first.\n\r", ch);
    return;
  }

  if (check_peaceful(ch,
        "You feel too peaceful to contemplate violence.\n\r")) {
    return;
  }

  only_argument(argument, arg);

  if (*arg) {
    victim = get_char_room_vis(ch, arg);
    if (victim) {
      if (victim == ch) {
        send_to_char("You hit yourself..OUCH!.\n\r", ch);
        act("$n hits $mself, and says OUCH!", FALSE, ch, 0, victim, TO_ROOM);
      } else {
        if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
          act("$N is just such a good friend, you simply can't hit $M.", FALSE,
            ch, 0, victim, TO_CHAR);
          return;
        }

        if ((GET_POS(ch) == POSITION_STANDING) &&
            (victim != ch->specials.fighting)) {
          hit(ch, victim, TYPE_UNDEFINED);
          WAIT_STATE(ch, PULSE_VIOLENCE + 2);

        } else {
          if (victim != ch->specials.fighting) {
            if (ch->skills && ch->skills[SKILL_SWITCH_OPP].learned) {
              if (number(1, 101) < ch->skills[SKILL_SWITCH_OPP].learned) {
                stop_fighting(ch);
                if (victim->attackers < 5) {
                  set_fighting(ch, victim);
                } else {
                  send_to_char("There's no room to switch!\n\r", ch);
                }
                send_to_char("You switch opponents\n\r", ch);
                act("$n switches targets", FALSE, ch, 0, 0, TO_ROOM);
                WAIT_STATE(ch, PULSE_VIOLENCE + 2);
              } else {
                send_to_char(
                  "You try to switch opponents, but you become confused!\n\r",
                  ch);
                stop_fighting(ch);
                LearnFromMistake(ch, SKILL_SWITCH_OPP, 0, 95);
                WAIT_STATE(ch, PULSE_VIOLENCE * 2);
              }
            } else {
              send_to_char("You do the best you can!\n\r", ch);
            }
          } else {
            send_to_char("You do the best you can!\n\r", ch);
          }
        }
      }
    } else {
      send_to_char("They aren't here.\n\r", ch);
    }
  } else {
    send_to_char("Hit who?\n\r", ch);
  }
}

void do_kill(struct char_data* ch, char* argument, int cmd) {
  static char arg[MAX_INPUT_LENGTH];
  struct char_data* victim;

  if (check_peaceful(ch, "You feel to peaceful to contemplate violence!\n\r")) {
    return;
  }

  if ((GetMaxLevel(ch) < SILLYLORD) || IS_NPC(ch)) {
    do_hit(ch, argument, 0);
    return;
  }

  only_argument(argument, arg);

  if (!*arg) {
    send_to_char("Kill who?\n\r", ch);
  } else {
    if (!(victim = get_char_room_vis(ch, arg))) {
      send_to_char("They aren't here.\n\r", ch);
    } else if (ch == victim) {
      send_to_char("Your mother would be so sad.. :(\n\r", ch);
    } else {
      act("You chop $M to pieces! Ah! The blood!", FALSE, ch, 0, victim,
        TO_CHAR);
      act("$N chops you to pieces!", FALSE, victim, 0, ch, TO_CHAR);
      act("$n brutally slays $N", FALSE, ch, 0, victim, TO_NOTVICT);
      raw_kill(victim);
    }
  }
}

void do_backstab(struct char_data* ch, char* argument, int cmd) {
  struct char_data* victim;
  char name[256];
  signed char percent;
  signed char base = 0;

  if (check_peaceful(ch, "Naughty, naughty.  None of that here.\n\r")) {
    return;
  }

  only_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("Backstab who?\n\r", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("How can you sneak up on yourself?\n\r", ch);
    return;
  }

  if (!ch->equipment[WIELD]) {
    send_to_char("You need to wield a weapon, to make it a succes.\n\r", ch);
    return;
  }

  if (ch->attackers) {
    send_to_char("There's no way to reach that back while you're fighting!\n\r",
      ch);
    return;
  }

  if (victim->attackers >= 3) {
    send_to_char("You can't get close enough to them to backstab!\n\r", ch);
    return;
  }

  if (ch->equipment[WIELD]->obj_flags.value[3] != 11 &&
      ch->equipment[WIELD]->obj_flags.value[3] != 1 &&
      ch->equipment[WIELD]->obj_flags.value[3] != 10) {
    send_to_char(
      "Only piercing or stabbing weapons can be used for backstabbing.\n\r",
      ch);
    return;
  }

  if (ch->specials.fighting) {
    send_to_char("You're too busy to backstab\n\r", ch);
    return;
  }

  if (victim->specials.fighting) {
    base = 0;
  } else {
    base = 4;
  }

  setKillerFlag(ch, victim);

  percent = number(1, 101); /* 101% is a complete failure */

  if (ch->skills && ch->skills[SKILL_BACKSTAB].learned) {
    if (percent > ch->skills[SKILL_BACKSTAB].learned) {
      if (AWAKE(victim)) {
        damage(ch, victim, 0, SKILL_BACKSTAB);
        AddHated(victim, ch);
      } else {
        base += 2;
        GET_HITROLL(ch) += base;
        hit(ch, victim, SKILL_BACKSTAB);
        GET_HITROLL(ch) -= base;
        AddHated(victim, ch);
      }
    } else {
      GET_HITROLL(ch) += base;
      hit(ch, victim, SKILL_BACKSTAB);
      GET_HITROLL(ch) -= base;
      AddHated(victim, ch);
    }
  } else {
    damage(ch, victim, 0, SKILL_BACKSTAB);
    AddHated(victim, ch);
  }
  WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
}

static int check_no_order(struct char_data* ch, char* msg) {
  struct room_data* rp;

  rp = real_roomp(ch->in_room);
  if (rp && rp->room_flags & NO_ORDER) {
    send_to_char(msg, ch);
    return 1;
  }
  return 0;
}

void do_order(struct char_data* ch, char* argument, int cmd) {
  char name[100];
  char message[256];
  char buf[256];
  char found = FALSE;
  int org_room;
  struct char_data* victim;
  struct follow_type* k;

  if (apply_soundproof(ch)) {
    return;
  }

  if (check_no_order(ch, "Sorry this is one of Brut's no order rooms.\n\r")) {
    return;
  }

  half_chop(argument, name, message);

  if (!*name || !*message) {
    send_to_char("Order who to do what?\n\r", ch);
  } else if (!(victim = get_char_room_vis(ch, name)) &&
             str_cmp("follower", name) && str_cmp("followers", name)) {
    send_to_char("That person isn't here.\n\r", ch);
  } else if (ch == victim) {
    send_to_char("You obviously suffer from Multiple Personality Disorder.\n\r",
      ch);

  } else {
    if (IS_AFFECTED(ch, AFF_CHARM)) {
      send_to_char("Your superior would not aprove of you giving orders.\n\r",
        ch);
      return;
    }

    if (victim) {
      if (check_soundproof(victim)) {
        return;
      }
      sprintf(buf, "$N orders you to '%s'", message);
      act(buf, FALSE, victim, 0, ch, TO_CHAR);
      act("$n gives $N an order.", FALSE, ch, 0, victim, TO_ROOM);

      if ((victim->master != ch) || !IS_AFFECTED(victim, AFF_CHARM)) {
        act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
      } else {
        send_to_char("Ok.\n\r", ch);
        command_interpreter(victim, message);
      }
    } else { /* This is order "followers" */
      sprintf(buf, "$n issues the order '%s'.", message);
      act(buf, FALSE, ch, 0, victim, TO_ROOM);

      org_room = ch->in_room;

      for (k = ch->followers; k; k = k->next) {
        if (org_room == k->follower->in_room) {
          if (IS_AFFECTED(k->follower, AFF_CHARM)) {
            found = TRUE;
            command_interpreter(k->follower, message);
          }
        }
      }
      if (found) {
        send_to_char("Ok.\n\r", ch);
      } else {
        send_to_char("Nobody here is a loyal subject of yours!\n\r", ch);
      }
    }
  }
}

void do_flee(struct char_data* ch, char* argument, int cmd) {
  struct obj_data* weapon;
  int i;
  int lev_check;
  int attempt;
  int loose = 0;
  int die;
  int percent;
  int losedie;

  if (IS_AFFECTED(ch, AFF_PARALYSIS)) {
    return;
  }

  if (affected_by_spell(ch, SPELL_WEB)) {
    if (!saves_spell(ch, SAVING_PARA)) {
      WAIT_STATE(ch, PULSE_VIOLENCE);
      send_to_char("You are ensared in webs, you cannot move!\n\r", ch);
      act("$n struggles against the webs that hold $m", FALSE, ch, 0, 0,
        TO_ROOM);
      return;
    }
    send_to_char("You pull free from the sticky webbing!\n\r", ch);
    act("$n manages to pull free from the sticky webbing!", FALSE, ch, 0, 0,
      TO_ROOM);
    GET_MOVE(ch) -= 50;
  }

  if (ch->specials.fighting) {
    lev_check = (GetMaxLevel(ch->specials.fighting) - GetMaxLevel(ch));
    if (number(1, 100) < lev_check) {
      WAIT_STATE(ch, PULSE_VIOLENCE);
      act("$N grabs you by the collar and stops you from fleeing!", FALSE, ch,
        0, ch->specials.fighting, TO_CHAR);
      act("You grab $N by the collar and stop them from fleeing!", FALSE,
        ch->specials.fighting, 0, ch, TO_CHAR);
      act("$N grabs $n by the collar and stops $m from fleeing!", FALSE, ch, 0,
        ch->specials.fighting, TO_NOTVICT);
      return;
    }
  }

  if (GET_POS(ch) <= POSITION_SITTING) {
    GET_MOVE(ch) -= 10;
    act("$n scrambles madly to $s feet!", TRUE, ch, 0, 0, TO_ROOM);
    act("Panic-stricken, you scramble to your feet.", TRUE, ch, 0, 0, TO_CHAR);
    GET_POS(ch) = POSITION_STANDING;
    WAIT_STATE(ch, PULSE_VIOLENCE);
    return;
  }

  if (!(ch->specials.fighting)) {
    for (i = 0; i < 6; i++) {
      attempt = number(0, 5); /* Select a random direction */
      if (CAN_GO(ch, attempt) &&
          !IS_SET(real_roomp(EXIT(ch, attempt)->to_room)->room_flags, DEATH)) {
        act("$n panics, and attempts to flee.", TRUE, ch, 0, 0, TO_ROOM);
        if ((die = MoveOne(ch, attempt)) == 1) {
          /* The escape has succeded */
          send_to_char("You flee head over heels.\n\r", ch);
          return;
        }
        if (!die) {
          act("$n tries to flee, but is too exhausted!", TRUE, ch, 0, 0,
            TO_ROOM);
        }
        return;
      }
    } /* for */
    /* No exits was found */
    send_to_char("PANIC! You couldn't escape!\n\r", ch);
    return;
  }

  for (i = 0; i < 6; i++) {
    attempt = number(0, 5); /* Select a random direction */
    if (CAN_GO(ch, attempt) &&
        !IS_SET(real_roomp(EXIT(ch, attempt)->to_room)->room_flags, DEATH)) {
      int panic;
      int j;

      if (!ch->skills || (number(1, 101) > ch->skills[SKILL_RETREAT].learned)) {
        act("$n panics, and attempts to flee.", TRUE, ch, 0, 0, TO_ROOM);
        panic = TRUE;
        LearnFromMistake(ch, SKILL_RETREAT, 0, 90);
      } else {
        act("$n skillfully retreats from battle", TRUE, ch, 0, 0, TO_ROOM);
        panic = FALSE;
      }

      if (IS_PC(ch)) {
        if (ch->equipment[WIELD]) {
          if (number(1, 3) == 1) {
            send_to_char("In your haste to flee, you drop your weapon.\n\r",
              ch);
            obj_to_room(unequip_char(ch, WIELD), ch->in_room);
          }
        }
      }

      if ((die = MoveOne(ch, attempt)) == 1) {
        if (GetMaxLevel(ch) > 3) {
          if (panic || !HasClass(ch, CLASS_WARRIOR)) {
            loose = 2 * GetMaxLevel(ch);
            loose -= 2 * GetMaxLevel(ch->specials.fighting);
            loose *= GetMaxLevel(ch);
          }
        } else {
          loose = 0;
        }
        if (loose < 0) {
          loose = 1;
        }

        if (IS_NPC(ch) &&
            !(IS_SET(ch->specials.act, ACT_POLYSELF) && !ch->desc &&
              !(IS_SET(ch->specials.act, ACT_AGGRESSIVE)))) {
          AddFeared(ch, ch->specials.fighting);
        } else {
          percent = 100 * (float)GET_HIT(ch->specials.fighting) /
                    (float)GET_MAX_HIT(ch->specials.fighting);
          if (number(1, 101) < percent) {
            if ((Hates(ch->specials.fighting, ch)) ||
                (IS_GOOD(ch) && (IS_EVIL(ch->specials.fighting))) ||
                (IS_EVIL(ch) && (IS_GOOD(ch->specials.fighting)))) {
              SetHunting(ch->specials.fighting, ch);
            }
          }
        }

        if (IS_PC(ch) && panic) {
          if (HasClass(ch, CLASS_MONK) || !HasClass(ch, CLASS_WARRIOR)) {
            GET_EXP(ch) -= loose;
          }
        }

        if (panic) {
          send_to_char("You flee head over heels.\n\r", ch);
        } else {
          send_to_char("You retreat skillfully\n\r", ch);
        }
        if (ch->specials.fighting->specials.fighting == ch) {
          stop_fighting(ch->specials.fighting);
        }
        if (ch->specials.fighting) {
          stop_fighting(ch);
        }
        return;
      }
      if (!die) {
        act("$n tries to flee, but is too exhausted!", TRUE, ch, 0, 0, TO_ROOM);
      }
      return;
    }
  } /* for */

  /* No exits were found */
  send_to_char("PANIC! You couldn't escape!\n\r", ch);
}

void do_bash(struct char_data* ch, char* argument, int cmd) {
  struct char_data* victim;
  char name[256];
  signed char percent;

  if (!ch->skills) {
    return;
  }

  if (check_peaceful(ch,
        "You feel too peaceful to contemplate violence.\n\r")) {
    return;
  }

  only_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("Bash who?\n\r", ch);
      return;
    }
  }

  if (victim == ch) {
    send_to_char("Aren't we funny today...\n\r", ch);
    return;
  }

  if (!ch->skills) {
    if (GET_POS(victim) > POSITION_DEAD) {
      damage(ch, victim, 0, SKILL_BASH);
      GET_POS(ch) = POSITION_SITTING;
    }
  }

  if (ch->attackers > 3) {
    send_to_char("There's no room to bash!\n\r", ch);
    return;
  }

  if (victim->attackers >= 6) {
    send_to_char("You can't get close enough to them to bash!\n\r", ch);
    return;
  }

  setKillerFlag(ch, victim);

  percent = number(1, 101); /* 101% is a complete failure */

  /* some modifications to account for dexterity, and level */
  percent -= dex_app[GET_DEX(ch)].reaction * 10;
  percent += dex_app[GET_DEX(victim)].reaction * 10;
  if (GetMaxLevel(victim) > 12) {
    percent += ((GetMaxLevel(victim) - 10) * 5);
  }
  if (percent > ch->skills[SKILL_BASH].learned) {
    if (GET_POS(victim) > POSITION_DEAD) {
      damage(ch, victim, 0, SKILL_BASH);
      GET_POS(ch) = POSITION_SITTING;
    }
  } else {
    if (GET_POS(victim) > POSITION_DEAD) {
      damage(ch, victim, 1, SKILL_BASH);
      GET_POS(victim) = POSITION_SITTING;
      WAIT_STATE(victim, PULSE_VIOLENCE * 2);
    }
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

void do_rescue(struct char_data* ch, char* argument, int cmd) {
  struct char_data* victim;
  struct char_data* tmp_ch;
  int percent;
  char victim_name[240];

  if (!ch->skills) {
    send_to_char("You fail the rescue.\n\r", ch);
    return;
  }

  if (check_peaceful(ch, "No one should need rescuing here.\n\r")) {
    return;
  }

  only_argument(argument, victim_name);

  if (!(victim = get_char_room_vis(ch, victim_name))) {
    send_to_char("Who do you want to rescue?\n\r", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("What about fleeing instead?\n\r", ch);
    return;
  }

  if (IS_NPC(victim)) {
    return;
  }

  if (ch->specials.fighting == victim) {
    send_to_char("How can you rescue someone you are trying to kill?\n\r", ch);
    return;
  }

  if (victim->attackers >= 3) {
    send_to_char("You can't get close enough to them to rescue!\n\r", ch);
    return;
  }

  for (tmp_ch = real_roomp(ch->in_room)->people;
    tmp_ch && (tmp_ch->specials.fighting != victim);
    tmp_ch = tmp_ch->next_in_room) {
    ;
  }

  if (!tmp_ch) {
    act("But nobody is fighting $M?", FALSE, ch, 0, victim, TO_CHAR);
    return;
  }

  if (!HasClass(ch, CLASS_WARRIOR) && (!HasClass(ch, CLASS_PALADIN)) &&
      (!HasClass(ch, CLASS_RANGER))) {
    send_to_char("But only true warriors can do this!", ch);
  } else {
    percent = number(1, 101); /* 101% is a complete failure */

    if (percent > ch->skills[SKILL_RESCUE].learned) {
      send_to_char("You fail the rescue.\n\r", ch);
      return;
    }

    send_to_char("Banzai! To the rescue...\n\r", ch);
    act("You are rescued by $N, you are confused!", FALSE, victim, 0, ch,
      TO_CHAR);
    act("$n heroically rescues $N.", FALSE, ch, 0, victim, TO_NOTVICT);

    if (victim->specials.fighting == tmp_ch) {
      stop_fighting(victim);
    }
    if (tmp_ch->specials.fighting) {
      stop_fighting(tmp_ch);
    }
    if (ch->specials.fighting) {
      stop_fighting(ch);
    }

    set_fighting(ch, tmp_ch);
    set_fighting(tmp_ch, ch);

    WAIT_STATE(victim, 2 * PULSE_VIOLENCE);
  }
}

void do_assist(struct char_data* ch, char* argument, int cmd) {
  struct char_data* victim;
  struct char_data* tmp_ch;
  char victim_name[240];

  if (check_peaceful(ch, "Noone should need assistance here.\n\r")) {
    return;
  }

  only_argument(argument, victim_name);

  if (!(victim = get_char_room_vis(ch, victim_name))) {
    send_to_char("Who do you want to assist?\n\r", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("Oh, by all means, help yourself...\n\r", ch);
    return;
  }

  if (IS_NPC(victim)) {
    send_to_char("You can't assist monsters sorry.\n\r", ch);
    return;
  }

  if (ch->specials.fighting == victim) {
    send_to_char("That would be counterproductive?\n\r", ch);
    return;
  }

  if (ch->specials.fighting) {
    send_to_char("You have your hands full right now\n\r", ch);
    return;
  }

  if (victim->attackers >= 6) {
    send_to_char("You can't get close enough to them to assist!\n\r", ch);
    return;
  }

  tmp_ch = victim->specials.fighting;
  /*	for (tmp_ch=real_roomp(ch->in_room)->people; tmp_ch &&
  (tmp_ch->specials.fighting != victim); tmp_ch=tmp_ch->next_in_room)  ;
  */
  if (!tmp_ch) {
    act("But he's not fighting anyone.", FALSE, ch, 0, victim, TO_CHAR);
    return;
  }

  hit(ch, tmp_ch, TYPE_UNDEFINED);

  WAIT_STATE(victim, PULSE_VIOLENCE + 2); /* same as hit */
}

void do_kick(struct char_data* ch, char* argument, int cmd) {
  struct char_data* victim;
  char name[256];
  signed char percent;
  int dam;

  if (!ch->skills) {
    return;
  }

  if (check_peaceful(ch,
        "You feel too peaceful to contemplate violence.\n\r")) {
    return;
  }

  only_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("Kick who?\n\r", ch);
      return;
    }
  }

  if (victim == ch) {
    send_to_char("Aren't we funny today...\n\r", ch);
    return;
  }

  if (ch->attackers > 2) {
    send_to_char("There's no room to kick!\n\r", ch);
    return;
  }

  if (victim->attackers >= 3) {
    send_to_char("You can't get close enough to them to kick!\n\r", ch);
    return;
  }

  setKillerFlag(ch, victim);

  percent = ((10 - (GET_AC(victim) / 10)) << 1) +
            number(1, 101); /* 101% is a complete failure */

  if (percent > ch->skills[SKILL_KICK].learned) {
    if (GET_POS(victim) > POSITION_DEAD) {
      damage(ch, victim, 0, SKILL_KICK);
    }
    LearnFromMistake(ch, SKILL_KICK, 0, 90);
  } else {
    if (GET_POS(victim) > POSITION_DEAD) {
      dam = GET_LEVEL(ch, BestFightingClass(ch));
      damage(ch, victim, dam, SKILL_KICK);
    }
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}

void do_wimpy(struct char_data* ch, char* arg, int cmd) {
  char buff[MAX_STRING_LENGTH];

  if (IS_NPC(ch)) {
    return;
  }

  if (IS_SET(ch->specials.act, PLR_WIMPY)) {
    send_to_char("You are no longer a wimp\n\r", ch);
    REMOVE_BIT(ch->specials.act, PLR_WIMPY);
    return;
  }
  SET_BIT(ch->specials.act, PLR_WIMPY);

  sprintf(buff, "Your min hit point before fleeing is %d\n\r",
    (hit_limit(ch) / 5));

  send_to_char("You are now a wimp!!\n\r", ch);
  send_to_char(buff, ch);
}

const funcp bweapons[] = {cast_geyser, cast_fire_breath, cast_gas_breath,
  cast_frost_breath, cast_acid_breath, cast_lightning_breath};

void do_breath(struct char_data* ch, char* argument, int cmd) {
  struct char_data* victim;
  char buf[MAX_STRING_LENGTH];
  char name[MAX_STRING_LENGTH];
  int count;
  int manacost;
  funcp weapon;

  if (check_peaceful(ch, "That wouldn't be nice at all.\n\r")) {
    return;
  }

  only_argument(argument, name);

  for (count = FIRST_BREATH_WEAPON;
    count <= LAST_BREATH_WEAPON && !affected_by_spell(ch, count); count++) {
    ;
  }

  if (count > LAST_BREATH_WEAPON) {
    struct breather* scan;

    for (scan = breath_monsters;
      scan->vnum >= 0 && scan->vnum != mob_index[ch->nr].virtual; scan++) {
      ;
    }

    if (scan->vnum < 0) {
      send_to_char("You don't have a breath weapon, potatohead.\n\r", ch);
      return;
    }

    for (count = 0; scan->breaths[count]; count++) {
      ;
    }

    if (count < 1) {
      sprintf(buf, "monster %s has no breath weapons", ch->player.short_descr);
      vlog(buf);
      send_to_char("Hey, why don't you have any breath weapons!?\n\r", ch);
      return;
    }

    weapon = scan->breaths[dice(1, count) - 1];
    manacost = scan->cost;
    if (GET_MANA(ch) <= -3 * manacost) {
      weapon = nullptr;
    }
  } else {
    manacost = 0;
    weapon = bweapons[count - FIRST_BREATH_WEAPON];
    affect_from_char(ch, count);
  }

  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("Breath on who?\n\r", ch);
      return;
    }
  }

  breath_weapon(ch, victim, manacost, weapon);

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

static int bow_missile_damage(struct char_data* ch, struct char_data* victim,
  int olddam, int attacktype) {
  int dam = 0;
  struct obj_data* bow;

  if (!DamDetailsOk(ch, victim, dam, attacktype)) {
    return (FALSE);
  }

  bow = ch->equipment[HOLD];
  dam = GET_DAMROLL(ch);
  dam += dice(bow->obj_flags.value[1], bow->obj_flags.value[2]);

  dam = MAX(0, dam);

  dam = SkipImmortals(victim, dam);

  SetVictFighting(ch, victim);

  dam = DamageTrivia(ch, victim, dam, attacktype);

  if (DoDamage(ch, victim, dam, attacktype)) {
    return (TRUE);
  }

  DamageMessages(ch, victim, dam, SPEC_BOW);

  if (DamageEpilog(ch, victim)) {
    return (TRUE);
  }

  return (FALSE);
}

static void bow_hit(struct char_data* ch, struct char_data* victim, int type) {
  root_hit(ch, victim, type, bow_missile_damage);
}

static void fire(struct char_data* ch, struct char_data* victim) {
  struct obj_data* bow;
  int tohit = 0;
  int todam = 0;

  bow = ch->equipment[HOLD];

  if (!bow || bow->obj_flags.type_flag != ITEM_BOW) {
    send_to_char("You must be holding a bow to fire one!\n\r", ch);
    return;
  }
  if (bow->obj_flags.value[3] >= 1) {
    bow->obj_flags.value[3]--;
    bow_hit(ch, victim, SPEC_BOW);
  } else {
    send_to_char("Your bow has no arrow. It twangs as you try to shoot it!\n\r",
      ch);
  }
}

void do_fire(struct char_data* ch, char* argument, int cmd) {
  char arg[80];
  struct char_data* victim;

  if (check_peaceful(ch,
        "You feel too peaceful to contemplate violence.\n\r")) {
    return;
  }

  only_argument(argument, arg);

  if (*arg) {
    victim = get_char_room_vis(ch, arg);
    if (victim) {
      if (victim == ch) {
        send_to_char("Your mother would be SO sad!\n\r", ch);
        return;
      }
      if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
        act("$N is just such a good friends, you simply can't fire at $M.",
          FALSE, ch, 0, victim, TO_CHAR);
        return;
      }
      fire(ch, victim);
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);

    } else {
      send_to_char("They aren't here.\n\r", ch);
    }
  } else {
    send_to_char("Fire at who?\n\r", ch);
  }
}

int gun_missile_damage(struct char_data* ch, struct char_data* victim,
  int olddam, int attacktype) {
  int dam = 0;
  struct obj_data* gun;

  if (!DamDetailsOk(ch, victim, dam, attacktype)) {
    return (FALSE);
  }

  gun = ch->equipment[HOLD];
  dam = GET_DAMROLL(ch);
  if (gun->obj_flags.value[2] > 0) {
    dam += dice(gun->obj_flags.value[1], gun->obj_flags.value[2]);
  } else {
    act("$p jams and refuses to fire.", TRUE, ch, gun, 0, TO_CHAR);
    act("$p jams on $n", TRUE, ch, gun, 0, TO_ROOM);
    return (FALSE);
  }

  if (GET_POS(victim) < POSITION_FIGHTING) {
    dam *= 1 + (POSITION_FIGHTING - GET_POS(victim)) / 3;
  }

  dam = MAX(0, dam);

  dam = SkipImmortals(victim, dam);

  SetVictFighting(ch, victim);
  /*
     if (!SetCharFighting(ch, victim)) return(FALSE);
  */

  dam = DamageTrivia(ch, victim, dam, attacktype);

  if (DoDamage(ch, victim, dam, attacktype)) {
    return (TRUE);
  }

  DamageMessages(ch, victim, dam, SPEC_SHOOT);

  if (DamageEpilog(ch, victim)) {
    return (TRUE);
  }

  return (FALSE); /* not dead */
}

static void missile_hit(struct char_data* ch, struct char_data* victim,
  int type) {
  root_hit(ch, victim, type, gun_missile_damage);
}

static void shoot(struct char_data* ch, struct char_data* victim) {
  struct obj_data* gun;
  int tohit = 0;
  int todam = 0;

  gun = ch->equipment[HOLD];

  if (!gun || gun->obj_flags.type_flag != ITEM_FIREWEAPON) {
    send_to_char("You need to be holding a gun.\n\r", ch);
    return;
  } /*
     **  for guns:  value[0] = arror type
     **             value[1] = rolls
     **             value[2] = dice max
     **             value[3] = current shots
     **
     **   fire the weapon.
     */
  if (gun->obj_flags.value[3] >= 1) {
    gun->obj_flags.value[3]--;
    missile_hit(ch, victim, SPEC_SHOOT);
  } else {
    send_to_char("Click!  It seems to be empty.\n\r", ch);
    act("Click!  $n tries to fire an empty weapon.", FALSE, ch, 0, 0, TO_ROOM);
  }
}

void do_shoot(struct char_data* ch, char* argument, int cmd) {
  char arg[80];
  struct char_data* victim;

  if (check_peaceful(ch,
        "You feel too peaceful to contemplate violence.\n\r")) {
    return;
  }

  only_argument(argument, arg);

  if (*arg) {
    victim = get_char_room_vis(ch, arg);
    if (victim) {
      if (victim == ch) {
        send_to_char("You can't shoot things at yourself!", ch);
        return;
      }
      if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
        act("$N is just such a good friend, you simply can't shoot at $M.",
          FALSE, ch, 0, victim, TO_CHAR);
        return;
      }

      /*** Let me shoot during fighting, atleast for testing
      if (ch->specials.fighting) {
        send_to_char("You're at too close range to fire a weapon!\n\r", ch);
        return;
      }
      ******************************************************/
      shoot(ch, victim);
      WAIT_STATE(ch, PULSE_VIOLENCE);

    } else {
      send_to_char("They aren't here.\n\r", ch);
    }
  } else {
    send_to_char("Shoot who?\n\r", ch);
  }
}

void do_springleap(struct char_data* ch, char* argument, int cmd) {
  struct char_data* victim;
  char name[256];
  signed char percent;

  if (!ch->skills) {
    return;
  }

  if (check_peaceful(ch,
        "You feel too peaceful to contemplate violence.\n\r")) {
    return;
  }

  if (!HasClass(ch, CLASS_MONK)) {
    send_to_char("You're no monk!\n\r", ch);
    return;
  }

  only_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("Spring-leap at who?\n\r", ch);
      return;
    }
  }

  if (GET_POS(ch) > POSITION_SITTING || !ch->specials.fighting) {
    send_to_char("You're not in position for that!\n\r", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("Aren't we funny today...\n\r", ch);
    return;
  }

  if (ch->attackers > 3) {
    send_to_char("There's no room to spring-leap!\n\r", ch);
    return;
  }

  if (victim->attackers >= 3) {
    send_to_char("You can't get close enough\n\r", ch);
    return;
  }

  percent = number(1, 101);

  act("$n does a really nifty move, and aims a leg towards $N", FALSE, ch, 0,
    victim, TO_ROOM);
  act("You leap off the ground at $N", FALSE, ch, 0, victim, TO_CHAR);
  act("$n leaps off the ground at you", FALSE, ch, 0, victim, TO_VICT);

  if (percent > ch->skills[SKILL_SPRING_LEAP].learned) {
    if (GET_POS(victim) > POSITION_DEAD) {
      damage(ch, victim, 0, SKILL_KICK);
      LearnFromMistake(ch, SKILL_SPRING_LEAP, 0, 90);
      send_to_char("You fall on your butt\n\r", ch);
      act("$n falls on $s butt", FALSE, ch, 0, 0, TO_ROOM);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    return;
  }
  if (HitOrMiss(ch, victim, CalcThaco(ch))) {
    if (GET_POS(victim) > POSITION_DEAD) {
      damage(ch, victim, GET_LEVEL(ch, BestFightingClass(ch)) >> 1, SKILL_KICK);
    }
  } else {
    damage(ch, victim, 0, SKILL_KICK);
  }
  WAIT_STATE(victim, PULSE_VIOLENCE);

  WAIT_STATE(ch, PULSE_VIOLENCE * 1);
  GET_POS(ch) = POSITION_STANDING;
  update_pos(ch);
}

void do_quivering_palm(struct char_data* ch, char* arg, int cmd) {
  struct char_data* victim;
  struct affected_type af;
  signed char percent;
  char name[256];

  if (!ch->skills) {
    return;
  }

  if (check_peaceful(ch,
        "You feel too peaceful to contemplate violence.\n\r")) {
    return;
  }

  if (!HasClass(ch, CLASS_MONK)) {
    send_to_char("You're no monk!\n\r", ch);
    return;
  }

  only_argument(arg, name);

  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("Use the fabled quivering palm on who?\n\r", ch);
      return;
    }
  }

  if (!ch->skills) {
    return;
  }

  if (ch->attackers > 3) {
    send_to_char("There's no room to use that skill!\n\r", ch);
    return;
  }

  if (victim->attackers >= 3) {
    send_to_char("You can't get close enough\n\r", ch);
    return;
  }

  if (!IsHumanoid(victim)) {
    send_to_char("You can only do this to humanoid opponents\n\r", ch);
    return;
  }

  send_to_char("You begin to work on the vibrations\n\r", ch);

  if (affected_by_spell(ch, SKILL_QUIV_PALM)) {
    send_to_char("You can only do this once per week\n\r", ch);
    return;
  }

  percent = number(1, 101);

  if (percent > ch->skills[SKILL_QUIV_PALM].learned) {
    send_to_char("The vibrations fade ineffectively\n\r", ch);
    if (GET_POS(victim) > POSITION_DEAD) {
      LearnFromMistake(ch, SKILL_QUIV_PALM, 0, 95);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    return;
  }
  if (GET_MAX_HIT(victim) > GET_MAX_HIT(ch) * 2 ||
      GetMaxLevel(victim) > GetMaxLevel(ch)) {
    damage(ch, victim, 0, SKILL_QUIV_PALM);
    return;
  }
  if (HitOrMiss(ch, victim, CalcThaco(ch))) {
    if (GET_POS(victim) > POSITION_DEAD) {
      damage(ch, victim, GET_MAX_HIT(victim) * 20, SKILL_QUIV_PALM);
    }
  }

  WAIT_STATE(ch, PULSE_VIOLENCE * 1);

  af.type = SKILL_QUIV_PALM;
  af.duration = 168;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = 0;
  affect_to_char(ch, &af);
}
