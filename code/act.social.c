#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "accessors.h"
#include "character_flags.h"
#include "comm.h"
#include "db.h"
#include "game_constants.h"
#include "handler.h"
#include "interpreter.h"
#include "memory_macros.h"
#include "multiclass.h"
#include "utils.h"

struct char_data;

struct social_messg {
    int act_nr;
    int hide;
    int min_victim_position; /* Position of victim */

    /* No argument was supplied */
    char* char_no_arg;
    char* others_no_arg;

    /* An argument was there, and a victim was found */
    char* char_found; /* if nullptr, read no further, ignore args */
    char* others_found;
    char* vict_found;

    /* An argument was there, but no victim was found */
    char* not_found;

    /* The victim turned out to be the character */
    char* char_auto;
    char* others_auto;
}* soc_mess_list = nullptr;

struct pose_type {
    int level;          /* minimum level for poser */
    char* poser_msg[4]; /* message to poser        */
    char* room_msg[4];  /* message to room         */
} pose_messages[MAX_MESSAGES];

static int list_top = -1;

static char* fread_action(FILE* fl) {
  char buf[MAX_STRING_LENGTH];
  char* rslt = nullptr;

  for (;;) {
    fgets(buf, MAX_STRING_LENGTH, fl);
    if (feof(fl) != 0) {
      vlog("Fread_action - unexpected EOF.");
      exit(0);
    }

    if (*buf == '#') {
      return (nullptr);
    }
    *(buf + strlen(buf) - 1) = '\0';
    CREATE(rslt, char, strlen(buf) + 1);
    strcpy(rslt, buf);
    return (rslt);
  }
}

void boot_social_messages(void) {
  FILE* fl = nullptr;
  int tmp = -1;
  int hide = 0;
  int min_pos = 0;

  if ((fl = fopen(SOCMESS_FILE, "r")) == nullptr) {
    perror("boot_social_messages");
    exit(0);
  }

  for (;;) {
    fscanf(fl, " %d ", &tmp);
    if (tmp < 0) {
      break;
    }
    fscanf(fl, " %d ", &hide);
    fscanf(fl, " %d \n", &min_pos);

    /* alloc a new cell */
    if (soc_mess_list == nullptr) {
      CREATE(soc_mess_list, struct social_messg, 1);
      list_top = 0;
    } else {
      RECREATE(soc_mess_list, struct social_messg,
        (unsigned long)(++list_top + 1));
    }

    /* read the stuff */
    soc_mess_list[list_top].act_nr = tmp;
    soc_mess_list[list_top].hide = hide;
    soc_mess_list[list_top].min_victim_position = min_pos;

    soc_mess_list[list_top].char_no_arg = fread_action(fl);
    soc_mess_list[list_top].others_no_arg = fread_action(fl);

    soc_mess_list[list_top].char_found = fread_action(fl);

    /* if no char_found, the rest is to be ignored */
    if (soc_mess_list[list_top].char_found == nullptr) {
      continue;
    }

    soc_mess_list[list_top].others_found = fread_action(fl);
    soc_mess_list[list_top].vict_found = fread_action(fl);

    soc_mess_list[list_top].not_found = fread_action(fl);

    soc_mess_list[list_top].char_auto = fread_action(fl);

    soc_mess_list[list_top].others_auto = fread_action(fl);
  }
  fclose(fl);
}

static int find_action(int cmd) {
  int bot = 0;
  int top = 0;
  int mid = 0;

  bot = 0;
  top = list_top;

  if (top < 0) {
    return (-1);
  }

  for (;;) {
    mid = (bot + top) / 2;

    if (soc_mess_list[mid].act_nr == cmd) {
      return (mid);
    }
    if (bot == top) {
      return (-1);
    }

    if (soc_mess_list[mid].act_nr > cmd) {
      top = --mid;
    } else {
      bot = ++mid;
    }
  }
}

void do_action(struct char_data* ch, const char* argument, int cmd) {
  int act_nr = 0;
  char buf[MAX_INPUT_LENGTH];
  struct social_messg* action = nullptr;
  struct char_data* vict = nullptr;

  if ((act_nr = find_action(cmd)) < 0) {
    send_to_char("That action is not supported.\n\r", ch);
    return;
  }

  action = &soc_mess_list[act_nr];

  if (action->char_found != nullptr) {
    only_argument(argument, buf);
  } else {
    *buf = '\0';
  }

  if (*buf == 0) {
    send_to_char(action->char_no_arg, ch);
    send_to_char("\n\r", ch);
    act(action->others_no_arg, action->hide, ch, nullptr, nullptr, TO_ROOM);
    return;
  }

  if ((vict = get_char_room_vis(ch, buf)) == nullptr) {
    send_to_char(action->not_found, ch);
    send_to_char("\n\r", ch);
  } else if (vict == ch) {
    send_to_char(action->char_auto, ch);
    send_to_char("\n\r", ch);
    act(action->others_auto, action->hide, ch, nullptr, nullptr, TO_ROOM);
  } else {
    if (GET_POS(vict) < action->min_victim_position) {
      act("$N is not in a proper position for that.", 0, ch, nullptr, vict,
        TO_CHAR);
    } else {
      act(action->char_found, 0, ch, nullptr, vict, TO_CHAR);

      act(action->others_found, action->hide, ch, nullptr, vict, TO_NOTVICT);

      act(action->vict_found, action->hide, ch, nullptr, vict, TO_VICT);
    }
  }
}

void do_insult(struct char_data* ch, const char* argument, int /*cmd*/) {
  static char buf[100];
  static char arg[MAX_STRING_LENGTH];
  struct char_data* victim = nullptr;

  only_argument(argument, arg);

  if (*arg != 0) {
    if ((victim = get_char_room_vis(ch, arg)) == nullptr) {
      send_to_char("Can't hear you!\n\r", ch);
    } else {
      if (victim != ch) {
        sprintf(buf, "You insult %s.\n\r", GET_NAME(victim));
        send_to_char(buf, ch);

        switch (rand() % 3) {
          case 0: {
            if (GET_SEX(ch) == SEX_MALE) {
              if (GET_SEX(victim) == SEX_MALE) {
                act("$n accuses you of fighting like a woman!", 0, ch, nullptr,
                  victim, TO_VICT);
              } else {
                act("$n says that women can't fight.", 0, ch, nullptr, victim,
                  TO_VICT);
              }
            } else { /* Ch == Woman */
              if (GET_SEX(victim) == SEX_MALE) {
                act("$n accuses you of having the smallest.... (brain?)", 0, ch,
                  nullptr, victim, TO_VICT);
              } else {
                act(
                  "$n tells you that you'd loose a beautycontest against a "
                  "troll.",
                  0, ch, nullptr, victim, TO_VICT);
              }
            }
          } break;
          case 1: {
            act("$n calls your mother a bitch!", 0, ch, nullptr, victim,
              TO_VICT);
          } break;
          default: {
            act("$n tells you to get lost!", 0, ch, nullptr, victim, TO_VICT);
          } break;
        } /* end switch */

        act("$n insults $N.", 1, ch, nullptr, victim, TO_NOTVICT);
      } else { /* ch == victim */
        send_to_char("You feel insulted.\n\r", ch);
      }
    }
  } else {
    send_to_char("Sure you don't want to insult everybody.\n\r", ch);
  }
}

void boot_pose_messages(void) {
  FILE* fl = nullptr;
  signed char counter = 0;
  signed char char_class = 0;

  if ((fl = fopen(POSEMESS_FILE, "r")) == nullptr) {
    perror("boot_pose_messages");
    exit(0);
  }

  for (counter = 0;; counter++) {
    fscanf(fl, " %d ", &pose_messages[counter].level);
    if (pose_messages[counter].level < 0) {
      break;
    }
    for (char_class = 0; char_class < 4; char_class++) {
      pose_messages[counter].poser_msg[char_class] = fread_action(fl);
      pose_messages[counter].room_msg[char_class] = fread_action(fl);
    }
  }

  fclose(fl);
}

void do_pose(struct char_data* ch, const char* /*argument*/, int /*cmd*/) {
  signed char to_pose = 0;
  signed char counter = 0;

  send_to_char("Sorry Buggy command.\n\r", ch);
  return;

  if ((GetMaxLevel(ch) < pose_messages[0].level) || IS_NPC(ch)) {
    send_to_char("You can't do that.\n\r", ch);
    return;
  }

  for (counter = 0; (pose_messages[counter].level < GetMaxLevel(ch)) &&
                    (pose_messages[counter].level > 0);
    counter++) {
    ;
  }
  counter--;

  to_pose = number(0, counter);
  /*
  **  find highest level, use that.
  */
}
