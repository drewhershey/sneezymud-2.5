#include <stdio.h>
#include <string.h>

#include "accessors.h"
#include "bit_ops.h"
#include "character_flags.h"
#include "comm.h"
#include "commands.h"
#include "db.h"
#include "game_constants.h"
#include "handler.h"
#include "interpreter.h"
#include "multiclass.h"
#include "object_flags.h"
#include "room_flags.h"
#include "spell_ids.h"
#include "structs.h"
#include "utils.h"

void do_say(struct char_data* ch, const char* argument, int cmd) {
  int i = 0;
  char buf[MAX_INPUT_LENGTH + 40] = "\0\0\0\0";

  if (apply_soundproof(ch) != 0) {
    return;
  }

  for (i = 0; *(argument + i) == ' '; i++) {
    ;
  }

  if (IS_AFFECTED(ch, AFF_SILENT)) {
    send_to_char("You can't make a sound!\n\r", ch);
    act("$n waves $s hands and points silently toward his $s mouth.", 0, ch,
      nullptr, nullptr, TO_ROOM);
    return;
  }

  if (*(argument + i) == 0) {
    send_to_char("Yes, but WHAT do you want to say?\n\r", ch);
  } else {
    sprintf(buf, "$n says '%s'", argument + i);
    act(buf, 0, ch, nullptr, nullptr, TO_ROOM);
    sprintf(buf, "You say '%s'\n\r", argument + i);
    send_to_char(buf, ch);
  }
}

void do_shout(struct char_data* ch, const char* argument, int cmd) {
  char buf1[MAX_INPUT_LENGTH + 40];
  struct descriptor_data* i = nullptr;

  if (IS_AFFECTED(ch, AFF_SILENT)) {
    send_to_char("You can't make a sound!\n\r", ch);
    act("$n waves $s hands and points silently toward his $s mouth.", 0, ch,
      nullptr, nullptr, TO_ROOM);
    return;
  }

  if (!IS_NPC(ch) && IS_SET(ch->specials.act, PLR_NOSHOUT)) {
    send_to_char("You can't shout!!\n\r", ch);
    return;
  }

  if (!IS_NPC(ch) && (Silence == 1) && (!IS_IMMORTAL(ch))) {
    send_to_char("Shouting has been banned.\n\r", ch);
    send_to_char("It will return when the lag is better.\n\r", ch);
    return;
  }

  if (apply_soundproof(ch) != 0) {
    return;
  }

  for (; *argument == ' '; argument++) {
    ;
  }

  if ((ch->master != nullptr) && IS_AFFECTED(ch, AFF_CHARM)) {
    send_to_char("I don't think so :-)", ch->master);
    return;
  }

  if ((*argument) == 0) {
    send_to_char("Shout? Yes! Fine! Shout we must, but WHAT??\n\r", ch);
  } else {
    sprintf(buf1, "You shout '%s'\n\r", argument);
    send_to_char(buf1, ch);
    sprintf(buf1, "$n shouts '%s'", argument);

    for (i = descriptor_list; i != nullptr; i = i->next) {
      if (i->character != ch && (i->connected == 0) &&
          (IS_NPC(i->character) ||
            (!IS_SET(i->character->specials.act, PLR_NOSHOUT))) &&
          (check_soundproof(i->character) == 0)) {
        act(buf1, 0, ch, nullptr, i->character, TO_VICT);
      }
    }
  }
}

void do_grouptell(struct char_data* ch, const char* argument, int cmd) {
  static char buf1[MAX_INPUT_LENGTH];
  struct follow_type* f = nullptr;
  struct char_data* k = nullptr;

  if (IS_AFFECTED(ch, AFF_SILENT)) {
    send_to_char("You can't make a sound!\n\r", ch);
    act("$n waves $s hands and points silently toward his $s mouth.", 0, ch,
      nullptr, nullptr, TO_ROOM);
    return;
  }

  if ((k = ch->master) == nullptr) {
    k = ch;
  }
  for (; *argument == ' '; argument++) {
    ;
  }

  if ((*argument) == 0) {
    send_to_char(
      "Grouptell is a good command, but you need to tell your group "
      "SOMEthing!\n\r",
      ch);
  } else {
    sprintf(buf1, "You tell your group: %s\n\r", argument);
    send_to_char(buf1, ch);

    sprintf(buf1, "$n: %s", argument);

    act(buf1, 0, ch, nullptr, k, TO_VICT);

    for (f = k->followers; f != nullptr; f = f->next) {
      if IS_AFFECTED (f->follower, AFF_GROUP)
        act(buf1, 0, ch, nullptr, f->follower, TO_VICT);
    }
  }
}

void do_commune(struct char_data* ch, const char* argument, int cmd) {
  static char buf1[MAX_INPUT_LENGTH];
  struct descriptor_data* i = nullptr;

  for (; *argument == ' '; argument++) {
    ;
  }

  if ((*argument) == 0) {
    send_to_char("Communing among the gods is fine, but WHAT?\n\r", ch);
  } else {
    sprintf(buf1, "You tell the gods: %s\n\r", argument);
    send_to_char(buf1, ch);

    for (i = descriptor_list; i != nullptr; i = i->next) {
      if (i->character != ch && (i->connected == 0) && !IS_NPC(i->character) &&
          (GetMaxLevel(i->character) >= 52)) {
        if (IS_SET(i->character->specials.act, PLR_COLOR)) {
          sprintf(buf1, "%s$n: %s%s%s", ANSI_VIOLET, ANSI_CYAN, argument,
            ANSI_NORMAL);
        } else {
          sprintf(buf1, "$n: %s", argument);
        }
        act(buf1, 0, ch, nullptr, i->character, TO_VICT);
      }
    }
  }
}

static const char* random_word(void) {
  static const char* const words[] = {
    "argle", "bargle", "glop", "glyph", "hussamah",                    /* 5 */
    "rodina", "mustafah", "angina", "the", "fribble",                  /* 10 */
    "fnort", "frobozz", "zarp", "ripple", "yrk",                       /* 15 */
    "yid", "yerf", "oork", "grapple", "red",                           /* 20 */
    "blue", "you", "me", "ftagn", "hastur",                            /* 25 */
    "brob", "gnort", "lram", "truck", "kill",                          /* 30 */
    "cthulhu", "huzzah", "acetacytacylic", "hydrooxypropyl", "summah", /* 35 */
    "hummah", "cookies", "stan", "will", "wadapatang",                 /* 40 */
    "pterodactyl", "frob", "yuma", "gumma", "lo-pan",                  /* 45 */
    "sushi", "yaya", "yoyodine", "your", "mother"                      /* 50 */
  };
  static constexpr size_t max_words = sizeof(words) / sizeof(words[0]);

  return words[number(0, (int)(max_words - 1))];
}

void do_sign(struct char_data* ch, const char* argument, int cmd) {
  int i = 0;
  char buf[MAX_INPUT_LENGTH + 40];
  char buf2[MAX_INPUT_LENGTH];
  char* p = nullptr;
  int diff = 0;
  struct char_data* t = nullptr;
  struct room_data* rp = nullptr;

  for (i = 0; *(argument + i) == ' '; i++) {
    ;
  }

  if (*(argument + i) == 0) {
    send_to_char("Yes, but WHAT do you want to sign?\n\r", ch);
  } else {
    rp = real_roomp(ch->in_room);
    if (rp == nullptr) {
      return;
    }

    if (HasHands(ch) == 0) {
      send_to_char("Yeah right... WHAT HANDS!!!!!!!!\n\r", ch);
      return;
    }

    strcpy(buf, argument + i);
    buf2[0] = '\0';
    /*
      work through the argument, word by word.  if you fail your
      skill roll, the word comes out garbled.
      */
    p = strtok(buf, " "); /* first word */

    diff = strlen(buf);

    while (p != nullptr) {
      if ((ch->skills != nullptr) &&
          number(1, 75 + strlen(p)) < ch->skills[SKILL_SIGN].learned) {
        strcat(buf2, p);
      } else {
        strcat(buf2, random_word());
      }
      strcat(buf2, " ");
      diff -= 1;
      p = strtok(nullptr, " "); /* next word */
    }
    /*
      if a recipient fails a roll, a word comes out garbled.
      */

    /*
      buf2 is now the "corrected" string.
      */

    sprintf(buf, "$n signs '%s'", buf2);

    for (t = rp->people; t != nullptr; t = t->next_in_room) {
      if (t != ch) {
        if ((t->skills != nullptr) &&
            number(1, diff) < t->skills[SKILL_SIGN].learned) {
          act(buf, 0, ch, nullptr, t, TO_VICT);
        } else {
          act("$n makes funny motions with $s hands", 0, ch, nullptr, t,
            TO_VICT);
        }
      }
    }

    sprintf(buf, "You sign '%s'\n\r", argument + i);
    send_to_char(buf, ch);
  }
}

void do_send(struct char_data* ch, const char* argument, int cmd) {
  char buf1[MAX_INPUT_LENGTH + 40];
  struct descriptor_data* i = nullptr;
  struct obj_data* radio = nullptr;
  struct obj_data* radio2 = nullptr;

  if (!IS_NPC(ch) &&
      (IS_SET(ch->specials.act, PLR_NOSHOUT) || IS_AFFECTED(ch, AFF_SILENT))) {
    send_to_char("You can't send any messages at the moment!!\n\r", ch);
    return;
  }

  radio = ch->equipment[WEAR_RADIO];

  if (radio == nullptr) {
    send_to_char("You need to be holding a radio to send a message.\n\r", ch);
    return;
  }
  if (apply_soundproof(ch) != 0) {
    return;
  }

  for (; *argument == ' '; argument++) {
    ;
  }

  if ((ch->master != nullptr) && IS_AFFECTED(ch, AFF_CHARM)) {
    send_to_char("I don't think so :-)", ch->master);
    return;
  }

  if ((*argument) == 0) {
    send_to_char("What exactly did you want your message to be?\n\r", ch);
  } else {
    sprintf(buf1, "Your message is : %s\n\r", argument);
    send_to_char(buf1, ch);
    sprintf(buf1, "$n [Channel %d] : %s", radio->obj_flags.value[3], argument);

    for (i = descriptor_list; i != nullptr; i = i->next) {
      if ((i->character != nullptr) && (i->character != ch) &&
          (i->connected == CON_PLYNG) &&
          (check_soundproof(i->character) == 0) &&
          (i->character->in_room != NOWHERE)) {
        radio2 = i->character->equipment[WEAR_RADIO];

        if (((radio2) != nullptr) &&
            (radio->obj_flags.value[3] == radio2->obj_flags.value[3])) {
          act(buf1, 0, ch, nullptr, i->character, TO_VICT);
        }
      }
    }
  }
}

void do_tell(struct char_data* ch, const char* argument, int cmd) {
  struct char_data* vict = nullptr;
  char name[100];
  char message[MAX_INPUT_LENGTH + 20];
  char buf[MAX_INPUT_LENGTH + 20];

  if (IS_AFFECTED(ch, AFF_SILENT)) {
    send_to_char("You can't make a sound!\n\r", ch);
    act("$n waves $s hands and points silently toward his $s mouth.", 0, ch,
      nullptr, nullptr, TO_ROOM);
    return;
  }

  if (apply_soundproof(ch) != 0) {
    return;
  }

  half_chop(argument, name, message);

  if ((*name == 0) || (*message == 0)) {
    send_to_char("Who do you wish to tell what??\n\r", ch);
    return;
  }
  if ((vict = get_char_vis(ch, name)) == nullptr) {
    send_to_char("No-one by that name here..\n\r", ch);
    return;
  }
  if (ch == vict) {
    send_to_char("You try to tell yourself something.\n\r", ch);
    return;
  }
  if (GET_POS(vict) == POSITION_SLEEPING) {
    act("$E is asleep, shhh.", 0, ch, nullptr, vict, TO_CHAR);
    return;
  }
  if (IS_NPC(vict) && ((vict->desc) == nullptr)) {
    send_to_char("No-one by that name here..\n\r", ch);
    return;
  }
  if (vict->desc == nullptr) {
    send_to_char("They can't hear you", ch);
    return;
  }

  if (check_soundproof(vict) != 0) {
    send_to_char("Your words dont reach them, must be in a silent zone.\n\r",
      ch);
    return;
  }

  if (IS_SET(vict->specials.act, PLR_COLOR)) {
    sprintf(buf, "%s%s%s tells you %s'%s'%s\n\r", ANSI_VIOLET,
      (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), ANSI_NORMAL,
      ANSI_CYAN, message, ANSI_NORMAL);
    send_to_char(buf, vict);
  } else if (IS_SET(vict->specials.act, PLR_VT100)) {
    sprintf(buf, "%s%s%s tells you '%s'\n\r", VT_BOLDTEX,
      (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), ANSI_NORMAL,
      message);
    send_to_char(buf, vict);
  } else {
    sprintf(buf, "%s tells you '%s'\n\r",
      (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), message);
    send_to_char(buf, vict);
  }

  sprintf(buf, "You tell %s '%s'\n\r",
    (IS_NPC(vict) ? vict->player.short_descr : GET_NAME(vict)), message);
  send_to_char(buf, ch);
}

void do_whisper(struct char_data* ch, const char* argument, int cmd) {
  struct char_data* vict = nullptr;
  char name[100];
  char message[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];

  if (IS_AFFECTED(ch, AFF_SILENT)) {
    send_to_char("You can't make a sound!\n\r", ch);
    act("$n waves $s hands and points silently toward his $s mouth.", 0, ch,
      nullptr, nullptr, TO_ROOM);
    return;
  }

  if (apply_soundproof(ch) != 0) {
    return;
  }

  half_chop(argument, name, message);

  if ((*name == 0) || (*message == 0)) {
    send_to_char("Who do you want to whisper to.. and what??\n\r", ch);
  } else if ((vict = get_char_room_vis(ch, name)) == nullptr) {
    send_to_char("No-one by that name here..\n\r", ch);
  } else if (vict == ch) {
    act("$n whispers quietly to $mself.", 0, ch, nullptr, nullptr, TO_ROOM);
    send_to_char(
      "You can't seem to get your mouth close enough to your ear...\n\r", ch);
  } else {
    if (check_soundproof(vict) != 0) {
      return;
    }

    sprintf(buf, "$n whispers to you, '%s'", message);
    act(buf, 0, ch, nullptr, vict, TO_VICT);
    sprintf(buf, "You whisper to %s, '%s'\n\r",
      (IS_NPC(vict) ? vict->player.name : GET_NAME(vict)), message);
    send_to_char(buf, ch);
    act("$n whispers something to $N.", 0, ch, nullptr, vict, TO_NOTVICT);
  }
}

void do_ask(struct char_data* ch, const char* argument, int cmd) {
  struct char_data* vict = nullptr;
  char name[100];
  char message[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];

  if (IS_AFFECTED(ch, AFF_SILENT)) {
    send_to_char("You can't make a sound!\n\r", ch);
    act("$n waves $s hands and points silently toward his $s mouth.", 0, ch,
      nullptr, nullptr, TO_ROOM);
    return;
  }

  if (apply_soundproof(ch) != 0) {
    return;
  }

  half_chop(argument, name, message);

  if ((*name == 0) || (*message == 0)) {
    send_to_char("Who do you want to ask something.. and what??\n\r", ch);
  } else if ((vict = get_char_room_vis(ch, name)) == nullptr) {
    send_to_char("No-one by that name here..\n\r", ch);
  } else if (vict == ch) {
    act("$n quietly asks $mself a question.", 0, ch, nullptr, nullptr, TO_ROOM);
    send_to_char("You think about it for a while...\n\r", ch);
  } else {
    if (check_soundproof(vict) != 0) {
      return;
    }

    sprintf(buf, "$n asks you '%s'", message);
    act(buf, 0, ch, nullptr, vict, TO_VICT);

    sprintf(buf, "You ask %s, '%s'\n\r",
      (IS_NPC(vict) ? vict->player.name : GET_NAME(vict)), message);
    send_to_char(buf, ch);
    act("$n asks $N a question.", 0, ch, nullptr, vict, TO_NOTVICT);
  }
}

#define MAX_NOTE_LENGTH 1000 /* arbitrary */

void do_write(struct char_data* ch, const char* argument, int cmd) {
  struct obj_data* paper = nullptr;
  struct obj_data* pen = nullptr;
  char papername[MAX_INPUT_LENGTH];
  char penname[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  argument_interpreter(argument, papername, penname);

  if (ch->desc == nullptr) {
    return;
  }

  if (*papername == 0) /* nothing was delivered */ {
    send_to_char("write (on) papername (with) penname.\n\r", ch);
    return;
  }

  if (*penname == 0) {
    send_to_char("write (on) papername (with) penname.\n\r", ch);
    return;
  }
  if ((paper = get_obj_in_list_vis(ch, papername, ch->carrying)) == nullptr) {
    sprintf(buf, "You have no %s.\n\r", papername);
    send_to_char(buf, ch);
    return;
  }
  if ((pen = get_obj_in_list_vis(ch, penname, ch->carrying)) == nullptr) {
    sprintf(buf, "You have no %s.\n\r", papername);
    send_to_char(buf, ch);
    return;
  }

  /* ok.. now let's see what kind of stuff we've found */
  if (pen->obj_flags.type_flag != ITEM_PEN) {
    act("$p is no good for writing with.", 0, ch, pen, nullptr, TO_CHAR);
  } else if (paper->obj_flags.type_flag != ITEM_NOTE) {
    act("You can't write on $p.", 0, ch, paper, nullptr, TO_CHAR);
  } else if (paper->action_description != nullptr) {
    send_to_char("There's something written on it already.\n\r", ch);
    return;
  } else {
    /* we can write - hooray! */
    send_to_char("Ok.. go ahead and write.. end the note with a @.\n\r", ch);
    act("$n begins to jot down a note.", 1, ch, nullptr, nullptr, TO_ROOM);
    ch->desc->str = &paper->action_description;
    ch->desc->max_str = MAX_NOTE_LENGTH;
  }
}
