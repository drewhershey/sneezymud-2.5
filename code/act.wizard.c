#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <time.h>

#include <algorithm>

#include "accessors.h"
#include "bit_ops.h"
#include "board.h"
#include "character_flags.h"
#include "comm.h"
#include "commands.h"
#include "constants.h"
#include "db.h"
#include "game_constants.h"
#include "handler.h"
#include "hash.h"
#include "heap.h"
#include "interpreter.h"
#include "limits.h"
#include "memory_macros.h"
#include "multiclass.h"
#include "object_flags.h"
#include "race.h"
#include "room_flags.h"
#include "spell_ids.h"
#include "spell_info.h"
#include "structs.h"
#include "utils.h"

static void create_one_room(int loc_nr) {
  struct room_data* rp = allocate_room(loc_nr);
  if (rp == nullptr) {
    return;
  }

  if (top_of_zone_table >= 0) {
    int zone = 0;

    for (; zone >= 0 && zone <= top_of_zone_table; zone++) {
      if (rp->number <= zone_table[zone].top) {
        break;
      }
    }

    if (zone > top_of_zone_table) {
      fprintf(stderr, "room_data %d is outside of any zone.\n", rp->number);
      --zone;
    }

    rp->zone = (short)zone;
  }

  char buf[10];
  sprintf(buf, "%d", loc_nr);
  rp->name = strdup(buf);
  rp->description = strdup("Empty\n");
}

void do_demote(struct char_data* ch, const char* argument, int /*cmd*/) {
  char person[MAX_STRING_LENGTH];
  struct char_data* victim = nullptr;
  if (IS_NPC(ch)) {
    return;
  }

  one_argument(argument, person);

  if (*person == 0) {
    send_to_char("Syntax: demote person\n\r", ch);
    return;
  }

  if ((victim = get_char_vis(ch, person)) == nullptr) {
    send_to_char("No one by that name here..\n\r", ch);
    return;
  }

  ch->player.time.birth -= SECS_PER_MUD_YEAR;
}

static struct StrHeap* init_heap() {
  struct StrHeap* heap = nullptr;

  heap = (struct StrHeap*)malloc(sizeof(struct StrHeap));
  heap->str = nullptr;
  /*
     Heap->str = (struct StrHeapList *)malloc(sizeof(struct StrHeapList));
     Heap->str[0].string=0;
     Heap->str[0].total=0;
  */
  heap->uniq = 0;
  return (heap);
}

void do_imptest(struct char_data* ch, const char* arg, int /*cmd*/) {
  struct char_data* i = nullptr;
  struct StrHeap* h = nullptr;
  int x = 0;

  if (strcmp(arg, " test test test") != 0) { /* don't use this command on the */
    return;                                  /* regular game */
  }

  if (GetMaxLevel(ch) < IMPLEMENTOR) {
    return;
  }

  h = init_heap();

  for (i = character_list; (i != nullptr) && x++ < 100; i = i->next) {
    StringHeap(i->player.short_descr, h);
  }

  DisplayStringHeap(h, ch, TO_CHAR, 1);
}

void do_passwd(struct char_data* ch, const char* argument, int /*cmdnum*/) {
  int player_i = 0;
  int pos = 0;
  char name[30];
  char npasswd[20];
  char pass[20];
  struct char_file_u tmp_store{};
  FILE* fl = nullptr;

  /*
   *  sets the specified user's password.
   */

  /*
   *  get user's name:
   */
  argument = one_argument(argument, name);
  argument = one_argument(argument, npasswd);

  /*
   *   Look up character
   */

  if ((player_i = load_char(name, &tmp_store)) > -1) {
    /*
     *  encrypt new password.
     */

    if ((*npasswd == 0) || strlen(npasswd) > 10) {
      send_to_char("Illegal password\n\r", ch);
      return;
    }

    strncpy(pass, crypt(npasswd, tmp_store.name), 10);

    /*
     *  put new password in place of old password
     */

    *(pass + 10) = '\0';
    strcpy(tmp_store.pwd, pass);

    /*
     *   save char to file
     */

    pos = player_table[player_i].nr;

    if ((fl = fopen(PLAYER_FILE, "r+")) == nullptr) {
      perror("do_passwd");
      exit(1);
    }

    fseek(fl, pos * sizeof(struct char_file_u), 0);

    fwrite(&tmp_store, sizeof(struct char_file_u), 1, fl);

    fclose(fl);

    return;
  }
  send_to_char("I don't recognize that name\n\r", ch);
}

/* Bamfin and bamfout - courtesy of DM from Epic */
static void dsearch(char* string, char* tmp) {
  char* c = nullptr;
  char buf[255];
  char buf2[255];
  char buf3[255];
  int i = 0;
  int j = 0;

  i = 0;
  while (i == 0) {
    if (strchr(string, '~') == nullptr) {
      i = 1;
      strcpy(tmp, string);
    } else {
      c = strchr(string, '~');
      j = c - string;
      switch (string[j + 1]) {
        case 'N':
          strcpy(buf2, "$n");
          break;
        case 'H':
          strcpy(buf2, "$s");
          break;
        default:
          strcpy(buf2, "");
          break;
      }
      strcpy(buf, string);
      buf[j] = '\0';
      strcpy(buf3, (string + j + 2));
      sprintf(tmp, "%s%s%s", buf, buf2, buf3);
      sprintf(string, "%s", tmp);
    }
  }
}

void do_bamfin(struct char_data* ch, const char* arg, int /*cmd*/) {
  char buf[255];
  int len = 0;

  for (; *arg == ' '; arg++) {
    ; /* pass all those spaces */
  }

  if (*arg == 0) {
    send_to_char("Bamfin <bamf definition>\n\r", ch);
    send_to_char(" Additional arguments can include ~N for where you\n\r", ch);
    send_to_char(" Want your name (if you want your name).  If you use\n\r",
      ch);
    send_to_char(" ~H, it puts in either his or her depending on your\n\r", ch);
    send_to_char(" sex.  If you use the keyword 'def' for your bamf,\n\r", ch);
    send_to_char(" it turns on the default bamf.  \n\r", ch);
    return;
  }

  if (strcmp(arg, "def") == 0) {
    REMOVE_BIT(ch->poof.pmask, BIT_POOF_IN);
    free(ch->poof.poofin);
    ch->poof.poofin = nullptr;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  len = strlen(arg);

  if (len > 150) {
    send_to_char("String too long.  Truncated to:\n\r", ch);
    len = 150;
    (void)snprintf(buf, sizeof(buf), "%.*s\n\r", len, arg);
    send_to_char(buf, ch);
  }

  if ((ch->poof.poofin != nullptr) && len >= strlen(ch->poof.poofin)) {
    free(ch->poof.poofin);
    ch->poof.poofin = (char*)malloc(len + 1);
  } else {
    if (ch->poof.poofin == nullptr) {
      ch->poof.poofin = (char*)malloc(len + 1);
    }
  }

  strcpy(buf, arg);
  dsearch(buf, ch->poof.poofin);
  SET_BIT(ch->poof.pmask, BIT_POOF_IN);
  send_to_char("Ok.\n\r", ch);
}

void do_bamfout(struct char_data* ch, const char* arg, int /*cmd*/) {
  char buf[255];
  int len = 0;

  if (*arg == 0) {
    send_to_char("Bamfout <bamf definition>\n\r", ch);
    send_to_char("  Additional arguments can include ~N for where you\n\r", ch);
    send_to_char("  Want your name (if you want your name). If you use\n\r",
      ch);
    send_to_char("  ~H, it puts in either his or her depending on your\n\r",
      ch);
    send_to_char("  sex.  If you use the keyword 'def' for your bamf,\n\r", ch);
    send_to_char("  it turns on the default bamf.  \n\r", ch);
    return;
  }
  arg++;

  if (strcmp(arg, "def") == 0) {
    REMOVE_BIT(ch->poof.pmask, BIT_POOF_OUT);
    free(ch->poof.poofout);
    ch->poof.poofout = nullptr;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  len = strlen(arg);

  if (len > 150) {
    send_to_char("String too long.  Truncated to:\n\r", ch);
    len = 150;
    (void)snprintf(buf, sizeof(buf), "%.*s\n\r", len, arg);
    send_to_char(buf, ch);
  }

  if ((ch->poof.poofout != nullptr) && len >= strlen(ch->poof.poofout)) {
    free(ch->poof.poofout);
    ch->poof.poofout = (char*)malloc(len + 1);
  } else if (ch->poof.poofout == nullptr) {
    ch->poof.poofout = (char*)malloc(len + 1);
  }

  strcpy(buf, arg);
  dsearch(buf, ch->poof.poofout);
  SET_BIT(ch->poof.pmask, BIT_POOF_OUT);
  send_to_char("Ok.\n\r", ch);
}

static FILE* make_zone_file(struct char_data* c) {
  char buf[256];
  FILE* fp = nullptr;

  sprintf(buf, "zone/%s.zon", GET_NAME(c));

  if ((fp = fopen(buf, "w")) != nullptr) {
    return (fp);
  }
  return (nullptr);
}

static int mob_vnum(struct char_data* c) {
  if (IS_NPC(c)) {
    return (mob_index[c->nr].vnum);
  }
  return (0);
}

static void rec_zwrite_obj(FILE* fp, struct obj_data* o) {
  struct obj_data* t = nullptr;

  if (ITEM_TYPE(o) == ITEM_CONTAINER) {
    for (t = o->contains; t != nullptr; t = t->next_content) {
      Zwrite(fp, 'P', 1, ObjVnum(t), obj_index[t->item_number].number,
        ObjVnum(o), t->short_description);
      rec_zwrite_obj(fp, t);
    }
  } else {
    return;
  }
}

void do_instazone(struct char_data* ch, const char* argument, int /*cmdnum*/) {
  char cmd = 0;
  char c = 0;
  char buf[80];
  int i = 0;
  int start_room = 0;
  int end_room = 0;
  int j = 0;
  int arg1 = 0;
  int arg2 = 0;
  int arg3 = 0;
  struct char_data* p = nullptr;
  struct obj_data* o = nullptr;
  struct room_data* room = nullptr;
  FILE* fp = nullptr;

  if (IS_NPC(ch)) {
    return;
  }

  /*
   *   read in parameters (room #s)
   */
  start_room = -1;
  end_room = -1;
  sscanf(argument, "%d%c%d", &start_room, &c, &end_room);

  if ((start_room == -1) || (end_room == -1)) {
    send_to_char("Instazone <start_room> <end_room>\n\r", ch);
    return;
  }

  fp = make_zone_file(ch);

  if (fp == nullptr) {
    send_to_char("Couldn't make file.. try again later\n\r", ch);
    return;
  }

  for (i = start_room; i <= end_room; i++) {
    room = real_roomp(i);
    if (room != nullptr) {
      /*
       *  first write out monsters
       */
      for (p = room->people; p != nullptr; p = p->next_in_room) {
        if (IS_NPC(p)) {
          cmd = 'M';
          arg1 = mob_vnum(p);
          arg2 = mob_index[p->nr].number;
          arg3 = i;
          Zwrite(fp, cmd, 0, arg1, arg2, arg3, p->player.short_descr);
          for (j = 0; j < MAX_WEAR; j++) {
            if (p->equipment[j] != nullptr) {
              if (p->equipment[j]->item_number >= 0) {
                cmd = 'E';
                arg1 = ObjVnum(p->equipment[j]);
                arg2 = obj_index[p->equipment[j]->item_number].number;
                arg3 = j;
                strcpy(buf, p->equipment[j]->short_description);
                Zwrite(fp, cmd, 1, arg1, arg2, arg3, buf);
                rec_zwrite_obj(fp, p->equipment[j]);
              }
            }
          }
          for (o = p->carrying; o != nullptr; o = o->next_content) {
            if (o->item_number >= 0) {
              cmd = 'G';
              arg1 = ObjVnum(o);
              arg2 = obj_index[o->item_number].number;
              arg3 = 0;
              strcpy(buf, o->short_description);
              Zwrite(fp, cmd, 1, arg1, arg2, arg3, buf);
              rec_zwrite_obj(fp, o);
            }
          }
        }
      }
      /*
       *  write out objects in rooms
       */
      for (o = room->contents; o != nullptr; o = o->next_content) {
        if (o->item_number >= 0) {
          cmd = 'O';
          arg1 = ObjVnum(o);
          arg2 = obj_index[o->item_number].number;
          arg3 = i;
          strcpy(buf, o->short_description);
          Zwrite(fp, cmd, 0, arg1, arg2, arg3, buf);
          rec_zwrite_obj(fp, o);
        }
      }
      /*
       *  lastly.. doors
       */

      for (j = 0; j < 6; j++) {
        /*
         *  if there is an door type exit, write it.
         */
        if (room->dir_option[j] != nullptr) { /* is a door */
          if (room->dir_option[j]->exit_info != 0) {
            cmd = 'D';
            arg1 = i;
            arg2 = j;
            arg3 = 0;
            if (IS_SET(room->dir_option[j]->exit_info, EX_CLOSED)) {
              arg3 = 1;
            }
            if (IS_SET(room->dir_option[j]->exit_info, EX_LOCKED)) {
              arg3 = 2;
            }
            Zwrite(fp, cmd, 0, arg1, arg2, arg3, room->name);
          }
        }
      }
    }
  }
  fclose(fp);
}

void do_highfive(struct char_data* ch, const char* argument, int /*cmd*/) {
  char buf[80];
  char mess[120];
  struct char_data* tch = nullptr;

  if (argument != nullptr) {
    only_argument(argument, buf);
    if ((tch = get_char_room_vis(ch, buf)) != nullptr) {
      if ((GetMaxLevel(tch) >= DEMIGOD) && (!IS_NPC(tch)) &&
          (GetMaxLevel(ch) >= DEMIGOD) && (!IS_NPC(ch))) {
        sprintf(mess, "Time stops for a moment as %s and %s high five.\n\r",
          ch->player.name, tch->player.name);
        send_to_all(mess);
      } else {
        act("$n gives you a high five", 1, ch, nullptr, tch, TO_VICT);
        act("You give a hearty high five to $N", 1, ch, nullptr, tch, TO_CHAR);
        act("$n and $N do a high five.", 1, ch, nullptr, tch, TO_NOTVICT);
      }
    } else {
      sprintf(buf, "I don't see anyone here like that.\n\r");
      send_to_char(buf, ch);
    }
  }
}

void do_silence(struct char_data* ch, const char* /*argument*/, int /*cmd*/) {
  char buf[255];
  if ((GetMaxLevel(ch) < DEMIGOD) || (IS_NPC(ch))) {
    send_to_char("You cannot Silence.\n\r", ch);
    return;
  }

  if (Silence == 0) {
    Silence = 1;
    send_to_char("You have now silenced shouting.\n\r", ch);
    sprintf(buf, "%s has stopped characters from shouting.", ch->player.name);
    vlog(buf);
  } else {
    Silence = 0;
    send_to_char("You have now unsilenced players.\n\r", ch);
    sprintf(buf, "%s has allowed characters to shout.", ch->player.name);
    vlog(buf);
  }
}

void do_wizlock(struct char_data* ch, const char* /*argument*/, int /*cmd*/) {
#if SITELOCK
  char* test;
  int a, length, b;
  char buf[255];
#endif

  if ((GetMaxLevel(ch) < DEMIGOD) || (IS_NPC(ch))) {
    send_to_char("You cannot WizLock.\n\r", ch);
    return;
  }

#if SITELOCK
  /* 4 commands */
  /* all,  add (place),  list,  rem  (place) */

  for (test = argument; *test && isspace(*test); test++)
    ;
  if (!*test)
    return;
  argument = test;

  /*
  **  get first piece..
  */

  argument = one_argument(argument, buf);
  if (!*buf) {
    send_to_char("Wizlock {all | off | add <host> | rem <host> | list}\n\r",
      ch);
    return;
  }

  if (str_cmp(buf, "all") == 0) {
    if (WizLock) {
      send_to_char("Its already on!\n\r", ch);
    } else {
      send_to_char("WizLock is now on\n\r", ch);
      vlog("WizLock is now on.");
      WizLock = true;
    }

  } else if (str_cmp(buf, "off") == 0) {
    if (!WizLock) {
      send_to_char("Its already off!\n\r", ch);
    } else {
      send_to_char("WizLock is now off\n\r", ch);
      vlog("WizLock is now off.");
      WizLock = false;
    }

  } else if (str_cmp(buf, "add") == 0) {
    argument = one_argument(argument, buf);
    if (!*buf) {
      send_to_char("Wizlock add <host_name>\n\r", ch);
      return;
    }
    length = strlen(buf);
    if ((length <= 3) || (length >= 30)) {
      send_to_char("Host is too long or short, please try again\n\r", ch);
      return;
    }
    for (a = 0; a <= numberhosts - 1; a++) {
      if (strncmp(hostlist[a], buf, length) == 0) {
        send_to_char("Host is already in database\n\r", ch);
        return;
      }
    }
    strcpy(hostlist[numberhosts], buf);
    sprintf(buf, "%s has added host %s to the access denied list.",
      GET_NAME(ch), hostlist[numberhosts]);
    vlog(buf);
    numberhosts++;
    return;

  } else if (str_cmp(buf, "rem") == 0) {
    if (numberhosts <= 0) {
      send_to_char("Host list is empty.\n\r", ch);
      return;
    }

    for (test = argument; *test && isspace(*test); test++)
      ;
    if (!*test) {
      send_to_char("Wizlock rem <host_name>\n\r", ch);
      return;
    }
    argument = test;

    argument = one_argument(argument, buf);

    if (!*buf) {
      send_to_char("Wizlock rem <host_name>\n\r", ch);
      return;
    }

    length = strlen(buf);
    if ((length <= 3) || (length >= 30)) {
      send_to_char("Host length is bad, please try again\n\r", ch);
      return;
    }

    for (a = 0; a <= numberhosts - 1; a++) {
      if (strncmp(hostlist[a], buf, length) == 0) {
        char tempstring[255];

        for (b = a; b <= numberhosts; b++)
          strcpy(hostlist[b], hostlist[b + 1]);
        sprintf(tempstring,
          "%s has removed host %s from the access denied list.", GET_NAME(ch),
          buf);
        vlog(tempstring);
        numberhosts--;
        return;
      }
    }
    send_to_char("Host is not in database\n\r", ch);
    return;

  } else if (str_cmp(buf, "list") == 0) {
    if (numberhosts <= 0) {
      send_to_char("Host list is empty.\n\r", ch);
      return;
    }
    for (a = 0; a <= numberhosts - 1; a++) {
      sprintf(buf, "Host: %s\n", hostlist[a]);
      send_to_char(buf, ch);
    }
    return;

  } else {
    send_to_char("Wizlock {all | add <host> | rem <host> | list}\n\r", ch);
    return;
  }

#else
  if (WizLock != 0) {
    send_to_char("WizLock is now off\n\r", ch);
    vlog("Wizlock is now off.");
    WizLock = 0;
  } else {
    send_to_char("WizLock is now on\n\r", ch);
    vlog("WizLock is now on.");
    WizLock = 1;
  }
#endif
}

static int room_enter(struct room_data* rb[], int key, struct room_data* rm) {
  if (key < 0 || key >= WORLD_SIZE) {
    return (0);
  }

  struct room_data* temp = room_find(rb, key);

  if (temp != nullptr) {
    return (0);
  }

  rb[key] = rm;
  return (1);
}

static void room_load(struct char_data* ch, int start, int end) {
  FILE* fp = nullptr;
  int vnum = 0;
  int found = 0;
  int x = 0;
  char buf[80];
  struct room_data* rp = nullptr;
  struct room_data dummy = {};

  sprintf(buf, "areas/%s", ch->player.name);

  if ((fp = fopen(buf, "r")) == nullptr) {
    send_to_char("You don't appear to have an area...\n\r", ch);
    return;
  }

  send_to_char("Searching and loading rooms\n\r", ch);

  while ((found == 0) && ((x = feof(fp)) != 1)) {
    fscanf(fp, "#%d\n", &vnum);
    if ((vnum >= start) && (vnum <= end)) {
      if (vnum == end) {
        found = 1;
      }

      if ((rp = real_roomp(vnum)) == nullptr) { /* empty room */
        rp = (struct room_data*)malloc(sizeof(struct room_data));
        memset(rp, 0, sizeof(struct room_data));
        room_enter(room_db, vnum, rp);
        send_to_char("+", ch);
      } else {
        if (rp->people != nullptr) {
          act("$n reaches down and scrambles reality.", 0, ch, nullptr,
            rp->people, TO_ROOM);
        }
        cleanout_room(rp);
        send_to_char("-", ch);
      }

      rp->number = vnum;
      load_one_room(fp, rp);

    } else {
      send_to_char(".", ch);
      /*  read w/out loading */
      dummy.number = vnum;
      load_one_room(fp, &dummy);
      cleanout_room(&dummy);
    }
  }
  fclose(fp);

  if (found == 0) {
    send_to_char(
      "\n\rThe room number(s) that you specified could not all be found.\n\r",
      ch);
  } else {
    send_to_char("\n\rDone.\n\r", ch);
  }
}

void do_rload(struct char_data* ch, const char* argument, int /*cmd*/) {
  char i = 0;
  int start = -1;
  int end = -2;

  if (IS_NPC(ch)) {
    return;
  }
  if (GetMaxLevel(ch) < IMMORTAL) {
    return;
  }

  for (i = 0; *(argument + i) == ' '; i++) {
    ;
  }
  if (*(argument + i) == 0) {
    send_to_char("Load? Fine!  Load we must, But what?\n\r", ch);
    return;
  }
  sscanf(argument, "%d %d", &start, &end);

  if ((start <= end) && (start != -1) && (end != -2)) {
    room_load(ch, start, end);
  }
}

static void room_save(struct char_data* ch, int start, int end) {
  char temp[2048];
  char dots[500];
  char buf[255];
  int rstart = 0;
  int rend = 0;
  int i = 0;
  int j = 0;
  int k = 0;
  int x = 0;
  struct extra_descr_data* exptr = nullptr;
  FILE* fp = nullptr;
  struct room_data* rp = nullptr;
  struct room_direction_data* rdd = nullptr;

  sprintf(buf, "areas/%s", ch->player.name);

  if ((fp = fopen(buf, "w")) == nullptr) {
    send_to_char("Can't write to disk now..try later \n\r", ch);
    return;
  }

  rstart = start;
  rend = end;

  if (((rstart <= -1) || (rend <= -1)) ||
      ((rstart > 40000) || (rend > 40000))) {
    send_to_char("I don't know those room #s.  make sure they are all\n\r", ch);
    send_to_char("contiguous.\n\r", ch);
    fclose(fp);
    return;
  }

  send_to_char("Saving\n", ch);
  strcpy(dots, "\0");

  for (i = rstart; i <= rend; i++) {
    rp = real_roomp(i);
    if (rp == nullptr) {
      continue;
    }

    strcat(dots, ".");

    /*
       strip ^Ms from description
    */
    x = 0;

    if (rp->description == nullptr) {
      CREATE(rp->description, char, 128);
      strcpy(rp->description, "Empty");
    }

    for (k = 0; k <= strlen(rp->description); k++) {
      if (rp->description[k] != 13) {
        temp[x++] = rp->description[k];
      }
    }
    temp[x] = '\0';

    fprintf(fp, "#%d\n%s~\n%s~\n", rp->number, rp->name, temp);
    if (rp->tele_targ == 0) {
      fprintf(fp, "%d %ld %d", rp->zone, rp->room_flags, rp->sector_type);
    } else {
      fprintf(fp, "%d %ld -1 %d %d %d %d", rp->zone, rp->room_flags,
        rp->tele_time, rp->tele_targ, rp->tele_look, rp->sector_type);
    }
    if (rp->sector_type == SECT_WATER_NOSWIM) {
      fprintf(fp, " %d %d", rp->river_speed, rp->river_dir);
    }

    if ((rp->room_flags & TUNNEL) != 0) {
      fprintf(fp, " %d ", (int)rp->moblim);
    }

    fprintf(fp, "\n");

    for (j = 0; j < 6; j++) {
      rdd = rp->dir_option[j];
      if (rdd != nullptr) {
        fprintf(fp, "D%d\n", j);

        if (rdd->general_description != nullptr) {
          if (strlen(rdd->general_description) > 0) {
            x = 0;
          }

          for (k = 0; k <= strlen(rdd->general_description); k++) {
            if (rdd->general_description[k] != 13) {
              temp[x++] = rdd->general_description[k];
            }
          }
          temp[x] = '\0';

          fprintf(fp, "%s~\n", temp);
        } else {
          fprintf(fp, "~\n");
        }

        if (rdd->keyword != nullptr) {
          if (strlen(rdd->keyword) > 0) {
            fprintf(fp, "%s~\n", rdd->keyword);
          }
        } else {
          fprintf(fp, "~\n");
        }

        if (IS_SET(rdd->exit_info, EX_PICKPROOF)) {
          if (IS_SET(rdd->exit_info, EX_SECRET)) {
            fprintf(fp, "4");
          } else {
            fprintf(fp, "2");
          }
        } else if (IS_SET(rdd->exit_info, EX_ISDOOR)) {
          if (IS_SET(rdd->exit_info, EX_SECRET)) {
            fprintf(fp, "3");
          } else {
            fprintf(fp, "1");
          }
        } else {
          fprintf(fp, "0");
        }

        fprintf(fp, " %d ", rdd->key);

        fprintf(fp, "%d\n", rdd->to_room);
      }
    }

    /*
      extra descriptions..
    */

    for (exptr = rp->ex_description; exptr != nullptr; exptr = exptr->next) {
      x = 0;

      if (exptr->description != nullptr) {
        for (k = 0; k <= strlen(exptr->description); k++) {
          if (exptr->description[k] != 13) {
            temp[x++] = exptr->description[k];
          }
        }
        temp[x] = '\0';

        fprintf(fp, "E\n%s~\n%s~\n", exptr->keyword, temp);
      }
    }

    fprintf(fp, "S\n");
  }

  fclose(fp);
  send_to_char(dots, ch);
  send_to_char("\n\rDone\n\r", ch);
}

void do_rsave(struct char_data* ch, const char* argument, int /*cmd*/) {
  char i = 0;
  int start = -1;
  int end = -2;

  if (IS_NPC(ch)) {
    return;
  }
  if (GetMaxLevel(ch) < IMMORTAL) {
    return;
  }

  for (i = 0; *(argument + i) == ' '; i++) {
    ;
  }
  if (*(argument + i) == 0) {
    send_to_char("Save? Fine!  Save we must, But what?\n\r", ch);
    return;
  }
  sscanf(argument, "%d %d", &start, &end);

  if ((start <= end) && (start != -1) && (end != -2)) {
    room_save(ch, start, end);
  }
}

void do_emote(struct char_data* ch, const char* argument, int cmd) {
  int i = 0;
  char buf[MAX_INPUT_LENGTH];

  if (IS_NPC(ch) && (cmd != 0)) {
    return;
  }

  if (check_soundproof(ch) != 0) {
    return;
  }

  for (i = 0; *(argument + i) == ' '; i++) {
    ;
  }

  if (*(argument + i) == 0) {
    send_to_char("Yes.. But what?\n\r", ch);
  } else {
    sprintf(buf, "$n %s", argument + i);
    act(buf, 0, ch, nullptr, nullptr, TO_ROOM);
    act(buf, 0, ch, nullptr, nullptr, TO_CHAR);
  }
}

void do_flag(struct char_data* ch, const char* argument, int /*cmd*/) {
  char buf[80];
  char buf2[80];
  struct char_data* victim = nullptr;

  half_chop(argument, buf, buf2);

  if (*buf == 0) {
    send_to_char("Flag who?\n\r", ch);
  } else {
    if ((victim = get_char_vis_world(ch, buf, nullptr)) == nullptr) {
      send_to_char("Noone by that name on!\n\r", ch);
    } else if (GetMaxLevel(victim) > GetMaxLevel(ch)) {
      send_to_char("Sorry you can't flag someone higher than you.\n\r", ch);
      return;
    } else if (!IS_PC(victim)) {
      send_to_char("Flag an NPC?\n\r", ch);
      return;
    } else if (is_abbrev(buf2, "killer") != 0) {
      if (IS_SET(victim->specials.act, PLR_KILLER)) {
        REMOVE_BIT(victim->specials.act, PLR_KILLER);
        act("You just removed $N's killer flag", 0, ch, nullptr, victim,
          TO_CHAR);
      } else {
        SET_BIT(victim->specials.act, PLR_KILLER);
        act("You just set $N's killer flag", 0, ch, nullptr, victim, TO_CHAR);
      }
    } else if (is_abbrev(buf2, "outlaw") != 0) {
      if (IS_SET(victim->specials.act, PLR_OUTLAW)) {
        REMOVE_BIT(victim->specials.act, PLR_OUTLAW);
        act("You just removed $N's outlaw flag", 0, ch, nullptr, victim,
          TO_CHAR);
      } else {
        SET_BIT(victim->specials.act, PLR_OUTLAW);
        act("You just set $N's outlaw flag.", 0, ch, nullptr, victim, TO_CHAR);
      }
    } else if (is_abbrev(buf2, "banished") != 0) {
      if (IS_SET(victim->specials.act, PLR_BANISHED)) {
        REMOVE_BIT(victim->specials.act, PLR_BANISHED);
        act("You just removed $N's banished flag", 0, ch, nullptr, victim,
          TO_CHAR);
      } else {
        SET_BIT(victim->specials.act, PLR_BANISHED);
        act("You just set $N's banished flag.", 0, ch, nullptr, victim,
          TO_CHAR);
      }
    } else {
      send_to_char("Syntax: flag <player> <flag> (killer,outlaw,banished)\n\r",
        ch);
      return;
    }
  }
}

void do_echo(struct char_data* ch, const char* argument, int /*cmd*/) {
  int i = 0;
  char buf[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)) {
    return;
  }

  for (i = 0; *(argument + i) == ' '; i++) {
    ;
  }

  if (IS_IMMORTAL(ch)) {
    sprintf(buf, "%s\n\r", argument + i);
    send_to_room_except(buf, ch->in_room, ch);
    send_to_char("Ok.\n\r", ch);
  }
}

void do_system(struct char_data* ch, const char* argument, int /*cmd*/) {
  int i = 0;
  char buf[256];

  if (IS_NPC(ch)) {
    return;
  }

  for (i = 0; *(argument + i) == ' '; i++) {
    ;
  }

  if (*(argument + i) == 0) {
    send_to_char("That must be a mistake...\n\r", ch);
  } else {
    sprintf(buf, "\n\r%s\n\r", argument + i);
    send_to_all(buf);
  }
}

void do_trans(struct char_data* ch, const char* argument, int /*cmd*/) {
  struct descriptor_data* i = nullptr;
  struct char_data* victim = nullptr;
  char buf[100];
  short int target = 0;

  if (IS_NPC(ch)) {
    return;
  }

  only_argument(argument, buf);
  if (*buf == 0) {
    send_to_char("Who do you wich to transfer?\n\r", ch);
  } else if (str_cmp("all", buf) != 0) {
    if ((victim = get_char_vis_world(ch, buf, nullptr)) == nullptr) {
      send_to_char("No-one by that name around.\n\r", ch);
    } else {
      act("$n disappears in a cloud of mushrooms.", 0, victim, nullptr, nullptr,
        TO_ROOM);
      target = ch->in_room;
      char_from_room(victim);
      char_to_room(victim, target);
      act("$n arrives from a puff of smoke.", 0, victim, nullptr, nullptr,
        TO_ROOM);
      act("$n has transferred you!", 0, ch, nullptr, victim, TO_VICT);
      do_look(victim, "", 15);
      send_to_char("Ok.\n\r", ch);
    }
  } else { /* Trans All */
    for (i = descriptor_list; i != nullptr; i = i->next) {
      if (i->character != ch && (i->connected == 0)) {
        victim = i->character;
        act("$n disappears in a cloud of mushrooms.", 0, victim, nullptr,
          nullptr, TO_ROOM);
        target = ch->in_room;
        char_from_room(victim);
        char_to_room(victim, target);
        act("$n arrives from a puff of smoke.", 0, victim, nullptr, nullptr,
          TO_ROOM);
        act("$n has transferred you!", 0, ch, nullptr, victim, TO_VICT);
        do_look(victim, "", 15);
      }
    }

    send_to_char("Ok.\n\r", ch);
  }
}

void do_at(struct char_data* ch, const char* argument, int /*cmd*/) {
  char command[MAX_INPUT_LENGTH];
  char loc_str[MAX_INPUT_LENGTH];
  int loc_nr = 0;
  int location = 0;
  int original_loc = 0;
  struct char_data* target_mob = nullptr;
  struct obj_data* target_obj = nullptr;

  if (IS_NPC(ch)) {
    return;
  }

  half_chop(argument, loc_str, command);
  if (*loc_str == 0) {
    send_to_char("You must supply a room number or a name.\n\r", ch);
    return;
  }

  if (isdigit(*loc_str) != 0) {
    loc_nr = atoi(loc_str);
    if (nullptr == real_roomp(loc_nr)) {
      send_to_char("No room exists with that number.\n\r", ch);
      return;
    }
    location = loc_nr;
  } else if ((target_mob = get_char_vis(ch, loc_str)) != nullptr) {
    location = target_mob->in_room;
  } else if ((target_obj = get_obj_vis_world(ch, loc_str, nullptr)) !=
             nullptr) {
    if (target_obj->in_room != NOWHERE) {
      location = target_obj->in_room;
    } else {
      send_to_char("The object is not available.\n\r", ch);
      return;
    }
  } else {
    send_to_char("No such creature or object around.\n\r", ch);
    return;
  }

  /* a location has been found. */

  original_loc = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, location);
  command_interpreter(ch, command);

  /* check if the guy's still there */
  for (target_mob = real_roomp(location)->people; target_mob != nullptr;
    target_mob = target_mob->next_in_room) {
    if (ch == target_mob) {
      char_from_room(ch);
      char_to_room(ch, original_loc);
    }
  }
}

void do_goto(struct char_data* ch, const char* argument, int /*cmd*/) {
  char buf[MAX_INPUT_LENGTH];
  int loc_nr = 0;
  int location = 0;
  int i = 0;
  struct char_data* target_mob = nullptr;
  struct char_data* pers = nullptr;
  struct char_data* v = nullptr;
  struct obj_data* target_obj = nullptr;

  if (IS_NPC(ch)) {
    return;
  }

  only_argument(argument, buf);
  if (*buf == 0) {
    send_to_char("You must supply a room number or a name.\n\r", ch);
    return;
  }

  if ((isdigit(*buf) != 0) && nullptr == strchr(buf, '.')) {
    loc_nr = atoi(buf);
    if (nullptr == real_roomp(loc_nr)) {
      if (GetMaxLevel(ch) < 51 || loc_nr < 0) {
        send_to_char("No room exists with that number.\n\r", ch);
        return;
      }
      if (loc_nr < WORLD_SIZE) {
        send_to_char("You form order out of chaos.\n\r", ch);
        create_one_room(loc_nr);
      } else {
        send_to_char("Sorry, that room # is too large.\n\r", ch);
        return;
      }
    }
    location = loc_nr;
  } else if ((target_mob = get_char_vis_world(ch, buf, nullptr)) != nullptr) {
    location = target_mob->in_room;
  } else if ((target_obj = get_obj_vis_world(ch, buf, nullptr)) != nullptr) {
    if (target_obj->in_room != NOWHERE) {
      location = target_obj->in_room;
    } else {
      send_to_char("The object is not available.\n\r", ch);
      send_to_char("Try where #.object to nail its room number.\n\r", ch);
      return;
    }
  } else {
    send_to_char("No such creature or object around.\n\r", ch);
    return;
  }

  /* a location has been found. */

  if (real_roomp(location) == nullptr) {
    vlog("Massive error in do_goto. Everyone Off NOW.");
    return;
  }

  if (IS_SET(real_roomp(location)->room_flags, PRIVATE)) {
    for (i = 0, pers = real_roomp(location)->people; pers != nullptr;
      pers = pers->next_in_room, i++) {
      ;
    }
    if (i > 1) {
      send_to_char("There's a private conversation going on in that room.\n\r",
        ch);
      return;
    }
  }

  if (IS_SET(ch->specials.act, PLR_STEALTH)) {
    for (v = real_roomp(ch->in_room)->people; v != nullptr;
      v = v->next_in_room) {
      if ((ch != v) && (GetMaxLevel(v) >= LOW_IMMORTAL)) {
        if (!IS_SET(ch->poof.pmask, BIT_POOF_OUT) ||
            (ch->poof.poofout == nullptr)) {
          act("$n disappears in a cloud of mushrooms.", 0, ch, nullptr, v,
            TO_VICT);
        } else {
          act(ch->poof.poofout, 0, ch, nullptr, v, TO_VICT);
        }
      }
    }
  } else {
    if (!IS_SET(ch->poof.pmask, BIT_POOF_OUT) ||
        (ch->poof.poofout == nullptr)) {
      act("$n disappears in a cloud of mushrooms.", 0, ch, nullptr, nullptr,
        TO_ROOM);
    } else if (*ch->poof.poofout != '!') {
      act(ch->poof.poofout, 0, ch, nullptr, nullptr, TO_ROOM);
    } else {
      command_interpreter(ch, (ch->poof.poofout + 1));
    }
  }

  if (ch->specials.fighting != nullptr) {
    stop_fighting(ch);
  }
  char_from_room(ch);
  char_to_room(ch, location);

  if (IS_SET(ch->specials.act, PLR_STEALTH)) {
    for (v = real_roomp(ch->in_room)->people; v != nullptr;
      v = v->next_in_room) {
      if ((ch != v) && (GetMaxLevel(v) >= LOW_IMMORTAL)) {
        if (!IS_SET(ch->poof.pmask, BIT_POOF_IN) ||
            (ch->poof.poofin == nullptr)) {
          act("$n appears with an explosion of rose-petals.", 0, ch, nullptr, v,
            TO_VICT);
        } else {
          act(ch->poof.poofin, 0, ch, nullptr, v, TO_VICT);
        }
      }
    }
  } else {
    if (!IS_SET(ch->poof.pmask, BIT_POOF_IN) || (ch->poof.poofin == nullptr)) {
      act("$n appears with an explosion of rose-petals.", 0, ch, nullptr,
        nullptr, TO_ROOM);
    } else if (*ch->poof.poofin != '!') {
      act(ch->poof.poofin, 0, ch, nullptr, nullptr, TO_ROOM);
    } else {
      command_interpreter(ch, (ch->poof.poofin + 1));
    }
  }
  do_look(ch, "", 15);
}

static const char* const sector_types[] = {"Inside", "City", "Field", "Forest",
  "Hills", "Mountains", "Water Swim", "Water NoSwim", "Air", "Underwater",
  "\n"};

static const char* const room_bits[] = {"DARK", "DEATH", "NO_MOB", "INDOORS",
  "PEACEFUL", "NOSTEAL", "NO_SUM", "NO_MAGIC", "TUNNEL", "PRIVATE", "SILENCE",
  "NO_ORDER", "ANARCHY", "HAVE_TO_WALK", "ARENA", "NO-HEAL", "\n"};

void do_stat(struct char_data* ch, const char* argument, int /*cmd*/) {
  struct affected_type* aff = nullptr;
  char arg1[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct room_data* rm = nullptr;
  struct char_data* k = nullptr;
  struct obj_data* j = nullptr;
  struct obj_data* j2 = nullptr;
  struct extra_descr_data* desc = nullptr;
  struct follow_type* fol = nullptr;
  int i = 0;
  int obj_vnum = 0;
  int i2 = 0;
  int count = 0;
  char found = 0;

  if (IS_NPC(ch)) {
    return;
  }

  only_argument(argument, arg1);

  /* no argument */
  if (*arg1 == 0) {
    send_to_char("Stats on who or what?\n\r", ch);
    return;
  } /* stats on room */
  if (str_cmp("room", arg1) == 0) {
    rm = real_roomp(ch->in_room);
    sprintf(buf,
      "room_data name: %s, Of zone : %d. V-Number : %d, R-number : %d\n\r",
      rm->name, rm->zone, rm->number, ch->in_room);
    send_to_char(buf, ch);

    sprinttype(rm->sector_type, sector_types, buf2);
    sprintf(buf, "Sector type : %s ", buf2);
    send_to_char(buf, ch);

    strcpy(buf, "Special procedure : ");
    strcat(buf, ((rm->funct) != nullptr) ? "Exists\n\r" : "No\n\r");
    send_to_char(buf, ch);

    send_to_char("room_data flags: ", ch);
    sprintbit(rm->room_flags, room_bits, buf);
    strcat(buf, "\n\r");
    send_to_char(buf, ch);

    send_to_char("Description:\n\r", ch);
    send_to_char(rm->description, ch);

    strcpy(buf, "Extra description keywords(s): ");
    if (rm->ex_description != nullptr) {
      strcat(buf, "\n\r");
      for (desc = rm->ex_description; desc != nullptr; desc = desc->next) {
        strcat(buf, desc->keyword);
        strcat(buf, "\n\r");
      }
      strcat(buf, "\n\r");
      send_to_char(buf, ch);
    } else {
      strcat(buf, "None\n\r");
      send_to_char(buf, ch);
    }

    strcpy(buf, "------- Chars present -------\n\r");
    for (k = rm->people; k != nullptr; k = k->next_in_room) {
      strcat(buf, GET_NAME(k));
      strcat(buf,
        (!IS_NPC(k) ? "(PC)\n\r" : (!IS_MOB(k) ? "(NPC)\n\r" : "(MOB)\n\r")));
    }
    strcat(buf, "\n\r");
    send_to_char(buf, ch);

    strcpy(buf, "--------- Contents ---------\n\r");
    for (j = rm->contents; j != nullptr; j = j->next_content) {
      strcat(buf, j->name);
      strcat(buf, "\n\r");
    }
    strcat(buf, "\n\r");
    send_to_char(buf, ch);

    static const char* const exit_bits[] = {"IS-DOOR", "CLOSED", "LOCKED",
      "SECRET", "RSLOCKED", "PICKPROOF", "\n"};

    send_to_char("------- Exits defined -------\n\r", ch);
    for (i = 0; i <= 5; i++) {
      if (rm->dir_option[i] != nullptr) {
        sprintf(buf, "Direction %s . Keyword : %s\n\r", dirs[i],
          rm->dir_option[i]->keyword);
        send_to_char(buf, ch);
        strcpy(buf, "Description:\n\r  ");
        if (rm->dir_option[i]->general_description != nullptr) {
          strcat(buf, rm->dir_option[i]->general_description);
        } else {
          strcat(buf, "UNDEFINED\n\r");
        }
        send_to_char(buf, ch);
        sprintbit(rm->dir_option[i]->exit_info, exit_bits, buf2);
        sprintf(buf,
          "Exit flag: %s \n\rKey no: %d\n\rTo room (R-Number): %d\n\r", buf2,
          rm->dir_option[i]->key, rm->dir_option[i]->to_room);
        send_to_char(buf, ch);
      }
    }
    return;
  }

  count = 1;

  /* mobile in world */
  if ((k = get_char_vis_world(ch, arg1, &count)) != nullptr) {
    switch (k->player.sex) {
      case SEX_NEUTRAL:
        strcpy(buf, "NEUTRAL-SEX");
        break;
      case SEX_MALE:
        strcpy(buf, "MALE");
        break;
      case SEX_FEMALE:
        strcpy(buf, "FEMALE");
        break;
      default:
        strcpy(buf, "ILLEGAL-SEX!!");
        break;
    }

    sprintf(buf2, " %s - Name : %s [R-Number%d], In room [%d]\n\r",
      (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")), GET_NAME(k), k->nr,
      k->in_room);
    strcat(buf, buf2);
    send_to_char(buf, ch);
    if (IS_MOB(k)) {
      sprintf(buf, "V-Number [%d]\n\r", mob_index[k->nr].vnum);
      send_to_char(buf, ch);
    }

    strcpy(buf, "Short description: ");
    strcat(buf,
      ((k->player.short_descr != nullptr) ? k->player.short_descr : "None"));
    strcat(buf, "\n\r");
    send_to_char(buf, ch);

    strcpy(buf, "Title: ");
    strcat(buf, ((k->player.title != nullptr) ? k->player.title : "None"));
    strcat(buf, "\n\r");
    send_to_char(buf, ch);

    send_to_char("Long description: ", ch);
    if (k->player.long_descr != nullptr) {
      send_to_char(k->player.long_descr, ch);
    } else {
      send_to_char("None", ch);
    }
    send_to_char("\n\r", ch);

    if (IS_NPC(k)) {
      static const char* const npc_class_types[] = {"Normal", "Undead", "\n"};

      strcpy(buf, "Monster Class: ");
      sprinttype(k->player.char_class, npc_class_types, buf2);
    } else {
      static const char* const pc_class_types[] = {"Magic User", "Cleric",
        "Warrior", "Thief", "Antipaladin", "Paladin", "Monk", "Ranger", "\n"};

      strcpy(buf, "Class: ");
      sprintbit(k->player.char_class, pc_class_types, buf2);
    }
    strcat(buf, buf2);

    if ((IS_NPC(k)) || (((HasClass(k, CLASS_MAGIC_USER)) != 0) ||
                         ((HasClass(k, CLASS_CLERIC)) != 0) ||
                         ((HasClass(k, CLASS_THIEF)) != 0) ||
                         ((HasClass(k, CLASS_WARRIOR)) != 0))) {
      sprintf(buf2, "   Level [%d/%d/%d/%d] Alignment[%d]\n\r",
        k->player.level[0], k->player.level[1], k->player.level[2],
        k->player.level[3], GET_ALIGNMENT(k));
    } else if (HasClass(k, CLASS_ANTIPALADIN) != 0) {
      sprintf(buf2, "   Level [ANTI lev %d] Alignment[%d]\n\r",
        k->player.level[4], GET_ALIGNMENT(k));
    } else if (HasClass(k, CLASS_PALADIN) != 0) {
      sprintf(buf2, "   Level [PAL  lev %d] Alignment[%d]\n\r",
        k->player.level[5], GET_ALIGNMENT(k));
    } else if (HasClass(k, CLASS_RANGER) != 0) {
      sprintf(buf2, "   Level [RANGER lev %d] Alignment[%d]\n\r",
        k->player.level[7], GET_ALIGNMENT(k));
    }

    strcat(buf, buf2);
    send_to_char(buf, ch);

    sprintf(buf, "Birth : [%ld]secs, Logon[%ld]secs, Played[%d]secs\n\r",
      k->player.time.birth, k->player.time.logon, k->player.time.played);

    send_to_char(buf, ch);

    sprintf(buf, "Age: [%d] Years,  [%d] Months,  [%d] Days,  [%d] Hours\n\r",
      age(k).year, age(k).month, age(k).day, age(k).hours);
    send_to_char(buf, ch);

    if (IS_PC(ch)) {
      sprintf(buf, "Practices : %d\n\r", ch->specials.spells_to_learn);
      send_to_char(buf, ch);
    }

    sprintf(buf, "Height [%d]cm  Weight [%d]pounds \n\r", GET_HEIGHT(k),
      GET_WEIGHT(k));
    send_to_char(buf, ch);

    sprintf(buf, "Str:[%d/%d]  Int:[%d]  Wis:[%d]  Dex:[%d]  Con:[%d]\n\r",
      GET_STR(k), GET_ADD(k), GET_INT(k), GET_WIS(k), GET_DEX(k), GET_CON(k));
    send_to_char(buf, ch);

    sprintf(buf,
      "Mana p.:[%d/%d+%d]  Hit p.:[%d/%d+%d]  Move p.:[%d/%d+%d]\n\r",
      GET_MANA(k), mana_limit(k), mana_gain(k), GET_HIT(k), hit_limit(k),
      hit_gain(k), GET_MOVE(k), move_limit(k), move_gain(k));
    send_to_char(buf, ch);

    sprintf(buf,
      "AC:[%d/10], Coins: [%d], Exp: [%d], Hitroll: [%d], Damroll: [%d]\n\r",
      GET_AC(k), GET_GOLD(k), GET_EXP(k), k->points.hitroll, k->points.damroll);
    send_to_char(buf, ch);

    static const char* const position_types[] = {"Dead", "Mortally wounded",
      "Incapacitated", "Stunned", "Sleeping", "Resting", "Sitting", "Fighting",
      "Standing", "\n"};

    sprinttype(GET_POS(k), position_types, buf2);
    sprintf(buf, "Position: %s, Fighting: %s", buf2,
      (((k->specials.fighting) != nullptr) ? GET_NAME(k->specials.fighting)
                                           : "Nobody"));

    if (k->desc != nullptr) {
      static const char* const connected_types[] = {"Playing", "Get name",
        "Confirm name", "Read Password", "Get new password",
        "Confirm new password", "Get sex", "Read messages of today",
        "Read Menu", "Get extra description", "Get class", "\n"};

      sprinttype(k->desc->connected, connected_types, buf2);
      strcat(buf, ", Connected: ");
      strcat(buf, buf2);
    }
    strcat(buf, "\n\r");
    send_to_char(buf, ch);

    strcpy(buf, "Default position: ");
    sprinttype((k->specials.default_pos), position_types, buf2);
    strcat(buf, buf2);
    if (IS_NPC(k)) {
      static const char* const action_bits[] = {"SPEC", "SENTINEL", "SCAVENGER",
        "ISNPC", "NICE-THIEF", "AGGRESSIVE", "STAY-ZONE", "WIMPY", "ANNOYING",
        "HATEFUL", "AFRAID", "IMMORTAL", "HUNTING", "DEADLY", "POLYMORPHED",
        "META_AGGRESSIVE", "GUARDING", "\n"};

      strcat(buf, ",NPC flags: ");
      sprintbit(k->specials.act, action_bits, buf2);
    } else {
      static const char* const player_bits[] = {"BRIEF", "COMPACT", "WIMPY",
        "DONTSET", "NOHASSLE", "STEALTH", "HUNTING", "MAILING", "LOGGED",
        "KILLER", "VT100", "COLOR", "OUTLAW", "ANSI", "NOSHOUT", "BANISHED",
        "GHOST", "\n"};

      strcat(buf, "\n\rFlags (Specials Act): ");
      sprintbit(k->specials.act, player_bits, buf2);
    }

    strcat(buf, buf2);

    sprintf(buf2, ",Timer [%d] \n\r", k->specials.timer);
    strcat(buf, buf2);
    send_to_char(buf, ch);

    if (IS_MOB(k)) {
      strcpy(buf, "\n\rMobile Special procedure : ");
      strcat(buf,
        ((mob_index[k->nr].func.mob_f != nullptr) ? "Exists\n\r" : "None\n\r"));
      send_to_char(buf, ch);
    }

    if (IS_NPC(k)) {
      sprintf(buf, "NPC Bare Hand Damage %dd%d.\n\r", k->specials.damnodice,
        k->specials.damsizedice);
      send_to_char(buf, ch);
    }

    sprintf(buf, "Carried weight: %d   Carried volume: %d\n\r",
      IS_CARRYING_W(k), IS_CARRYING_N(k));
    send_to_char(buf, ch);

    for (i = 0, j = k->carrying; j != nullptr; j = j->next_content, i++) {
      ;
    }
    sprintf(buf, "Items in inventory: %d, ", i);

    for (i = 0, i2 = 0; i < MAX_WEAR; i++) {
      if (k->equipment[i] != nullptr) {
        i2++;
      }
    }
    sprintf(buf2, "Items in equipment: %d\n\r", i2);
    strcat(buf, buf2);
    send_to_char(buf, ch);

    sprintf(buf, "Apply saving throws: [%d] [%d] [%d] [%d] [%d]\n\r",
      k->specials.apply_saving_throw[0], k->specials.apply_saving_throw[1],
      k->specials.apply_saving_throw[2], k->specials.apply_saving_throw[3],
      k->specials.apply_saving_throw[4]);
    send_to_char(buf, ch);

    sprintf(buf, "Thirst: %d, Hunger: %d, Drunk: %d\n\r",
      k->specials.conditions[THIRST], k->specials.conditions[FULL],
      k->specials.conditions[DRUNK]);
    send_to_char(buf, ch);

    sprintf(buf, "Master is '%s'\n\r",
      (((k->master) != nullptr) ? GET_NAME(k->master) : "NOBODY"));
    send_to_char(buf, ch);
    send_to_char("Followers are:\n\r", ch);
    for (fol = k->followers; fol != nullptr; fol = fol->next) {
      act("    $N", 0, ch, nullptr, fol->follower, TO_CHAR);
    }

    static const char* const immunity_names[] = {"FIRE", "COLD", "ELECTRICITY",
      "ENERGY", "BLUNT", "PIERCE", "SLASH", "ACID", "POISON", "DRAIN", "SLEEP",
      "CHARM", "HOLD", "NON-MAGIC", "+1", "+2", "+3", "+4", "\n"};

    /* immunities */
    send_to_char("Immune to:", ch);
    sprintbit(k->M_immune, immunity_names, buf);
    strcat(buf, "\n\r");
    send_to_char(buf, ch);
    /* resistances */
    send_to_char("Resistant to:", ch);
    sprintbit(k->immune, immunity_names, buf);
    strcat(buf, "\n\r");
    send_to_char(buf, ch);
    /* Susceptible */
    send_to_char("Susceptible to:", ch);
    sprintbit(k->susc, immunity_names, buf);
    strcat(buf, "\n\r");
    send_to_char(buf, ch);
    /*  race, action pointer */
    send_to_char("Race: ", ch);
    sprinttype((k->race), RaceName, buf2);
    send_to_char(buf2, ch);
    sprintf(buf, "  Action pointer: %p\n\r", (void*)k->act_ptr);
    send_to_char(buf, ch);

    if (IS_SET(k->specials.act, PLR_ANSI)) {
      send_to_char("Terminal type: ANSI\n\r", ch);
    } else if (IS_SET(k->specials.act, PLR_VT100)) {
      send_to_char("Terminal type: VT100\n\r", ch);
    } else {
      send_to_char("Terminal type : NONE\n\r", ch);
    }

    if (k->desc != nullptr) {
      sprintf(buf, "Screensize : %d", k->desc->screen_size);
      send_to_char(buf, ch);
      sprintf(buf, "   Prompt : %s\n\r", k->desc->prompt);
      send_to_char(buf, ch);
    }

    /* Showing the bitvector */
    sprintbit(k->specials.affected_by, affected_bits, buf);
    send_to_char("Affected by: ", ch);
    strcat(buf, "\n\r");
    send_to_char(buf, ch);

    /* Routine to show what spells a char is affected by */
    if (k->affected != nullptr) {
      send_to_char("\n\rAffecting Spells:\n\r--------------\n\r", ch);
      for (aff = k->affected; aff != nullptr; aff = aff->next) {
        sprintf(buf, "Spell : '%s'\n\r", spells[aff->type - 1]);
        send_to_char(buf, ch);
        sprintf(buf, "     Modifies %s by %d points\n\r",
          apply_types[aff->location], aff->modifier);
        send_to_char(buf, ch);
        sprintf(buf, "     Expires in %3d hours, Bits set ", aff->duration);
        send_to_char(buf, ch);
        sprintbit(aff->bitvector, affected_bits, buf);
        strcat(buf, "\n\r");
        send_to_char(buf, ch);
      }
    }
    return;
  }
  /* stat on object */
  if ((j = get_obj_vis_world(ch, arg1, &count)) != nullptr) {
    obj_vnum = (j->item_number >= 0) ? obj_index[j->item_number].vnum : 0;
    sprintf(buf,
      "Object name: [%s], R-number: [%d], V-number: [%d] Item type: ", j->name,
      j->item_number, obj_vnum);
    sprinttype(GET_ITEM_TYPE(j), item_types, buf2);
    strcat(buf, buf2);
    strcat(buf, "\n\r");
    send_to_char(buf, ch);
    sprintf(buf, "Short description: %s\n\rLong description:\n\r%s\n\r",
      (((j->short_description) != nullptr) ? j->short_description : "None"),
      (((j->description) != nullptr) ? j->description : "None"));
    send_to_char(buf, ch);
    if (j->ex_description != nullptr) {
      strcpy(buf, "Extra description keyword(s):\n\r----------\n\r");
      for (desc = j->ex_description; desc != nullptr; desc = desc->next) {
        strcat(buf, desc->keyword);
        strcat(buf, "\n\r");
      }
      strcat(buf, "----------\n\r");
      send_to_char(buf, ch);
    } else {
      strcpy(buf, "Extra description keyword(s): None\n\r");
      send_to_char(buf, ch);
    }

    static const char* const wear_bits[] = {"TAKE", "FINGER", "NECK", "BODY",
      "HEAD", "LEGS", "FEET", "HANDS", "ARMS", "SHIELD", "ABOUT", "WAIST",
      "WRIST", "WIELD", "HOLD", "THROW", "LIGHT-SOURCE", "EARRING", "FACE-GEAR",
      "\n"};

    send_to_char("Can be worn on :", ch);
    sprintbit(j->obj_flags.wear_flags, wear_bits, buf);
    strcat(buf, "\n\r");
    send_to_char(buf, ch);

    send_to_char("Set char bits  :", ch);
    sprintbit(j->obj_flags.bitvector, affected_bits, buf);
    strcat(buf, "\n\r");
    send_to_char(buf, ch);

    send_to_char("Extra flags: ", ch);
    sprintbit(j->obj_flags.extra_flags, extra_bits, buf);
    strcat(buf, "\n\r");
    send_to_char(buf, ch);

    sprintf(buf, "Volume: %d, Weight: %d, Value: %d, Cost/day: %d\n\r",
      j->obj_flags.volume, j->obj_flags.weight, j->obj_flags.cost,
      j->obj_flags.cost_per_day);
    send_to_char(buf, ch);

    sprintf(buf, "Decay :%d, Max Struct :%d, Struct Left %d\n\r",
      j->obj_flags.decay_time, j->obj_flags.max_struct_points,
      j->obj_flags.struct_points);
    send_to_char(buf, ch);

    sprintf(buf, "Material Type : %s\n\r",
      material_types[j->obj_flags.material_points]);
    send_to_char(buf, ch);

    strcpy(buf, "In room: ");
    if (j->in_room == NOWHERE) {
      strcat(buf, "Nowhere");
    } else {
      sprintf(buf2, "%d", j->in_room);
      strcat(buf, buf2);
    }
    strcat(buf, " ,In object: ");
    strcat(buf, ((j->in_obj == nullptr) ? "None" : fname(j->in_obj->name)));

    /*
strcat(buf," ,Carried by:");
if (j->carried_by) {
if (GET_NAME(j->carried_by)) {
if (strlen(GET_NAME(j->carried_by)) > 0) {
strcat(buf, (!j->carried_by) ? "Nobody" : GET_NAME(j->carried_by));
} else {
strcat(buf, "NonExistantPlayer");
}
} else {
strcat(buf, "NonExistantPlayer");
}
} else {
strcat(buf, "Nobody");
}
strcat(buf,"\n\r");
send_to_char(buf, ch);
*/
    switch (j->obj_flags.type_flag) {
      case ITEM_LIGHT:
        sprintf(buf, "Colour : [%d]\n\rType : [%d]\n\rHours : [%d]",
          j->obj_flags.value[0], j->obj_flags.value[1], j->obj_flags.value[2]);
        break;
      case ITEM_SCROLL:
        sprintf(buf, "Spells : %d, %d, %d, %d", j->obj_flags.value[0],
          j->obj_flags.value[1], j->obj_flags.value[2], j->obj_flags.value[3]);
        break;
      case ITEM_WAND:
        sprintf(buf, "Spell : %d\n\rMana : %d", j->obj_flags.value[0],
          j->obj_flags.value[1]);
        break;
      case ITEM_STAFF:
        sprintf(buf, "Spell : %d\n\rMana : %d", j->obj_flags.value[0],
          j->obj_flags.value[1]);
        break;
      case ITEM_WEAPON:
        sprintf(buf, "Tohit : %d\n\rTodam : %dD%d\n\rType : %d",
          j->obj_flags.value[0], j->obj_flags.value[1], j->obj_flags.value[2],
          j->obj_flags.value[3]);
        break;
      case ITEM_FIREWEAPON:
        sprintf(buf, "Bullet # : %d\n\rTodam : %dD%d\n\rShots left : %d",
          j->obj_flags.value[0], j->obj_flags.value[1], j->obj_flags.value[2],
          j->obj_flags.value[3]);
        break;
      case ITEM_MISSILE:
        sprintf(buf, "Tohit : %d\n\rTodam : %d\n\rType : %d",
          j->obj_flags.value[0], j->obj_flags.value[1], j->obj_flags.value[3]);
        break;
      case ITEM_ARMOR:
        sprintf(buf, "AC-apply : [%d]\n\rFull Strength : [%d]",
          j->obj_flags.value[0], j->obj_flags.value[1]);

        break;
      case ITEM_POTION:
        sprintf(buf, "Spells : %d, %d, %d, %d", j->obj_flags.value[0],
          j->obj_flags.value[1], j->obj_flags.value[2], j->obj_flags.value[3]);
        break;
      case ITEM_TRAP:
        sprintf(buf, "level: %d, att type: %d, damage class: %d, charges: %d",
          j->obj_flags.value[0], j->obj_flags.value[1], j->obj_flags.value[2],
          j->obj_flags.value[3]);
        break;
      case ITEM_CONTAINER:
        sprintf(buf, "Max-contains : %d\n\rLocktype : %d\n\rVolume : %d",
          j->obj_flags.value[0], j->obj_flags.value[1], j->obj_flags.value[3]);
        break;
      case ITEM_DRINKCON:
        sprinttype(j->obj_flags.value[2], drinks, buf2);
        sprintf(buf,
          "Max-contains : %d\n\rContains : %d\n\rPoisoned : %d\n\rLiquid : "
          "%s",
          j->obj_flags.value[0], j->obj_flags.value[1], j->obj_flags.value[3],
          buf2);
        break;
      case ITEM_NOTE:
        sprintf(buf, "Tounge : %d", j->obj_flags.value[0]);
        break;
      case ITEM_KEY:
        sprintf(buf, "Keytype : %d", j->obj_flags.value[0]);
        break;
      case ITEM_FOOD:
        sprintf(buf, "Makes full : %d\n\rPoisoned : %d", j->obj_flags.value[0],
          j->obj_flags.value[3]);
        break;
      case ITEM_ARROW:
        sprintf(buf, "Damage done : %dD%d", j->obj_flags.value[1],
          j->obj_flags.value[2]);
        break;
      default:
        sprintf(buf, "Values 0-3 : [%d] [%d] [%d] [%d]", j->obj_flags.value[0],
          j->obj_flags.value[1], j->obj_flags.value[2], j->obj_flags.value[3]);
        break;
    }
    send_to_char(buf, ch);

    strcpy(buf, "\n\rEquipment Status: ");
    if (j->carried_by == nullptr) {
      strcat(buf, "NONE");
    } else {
      found = 0;
      for (i = 0; i < MAX_WEAR; i++) {
        if (j->carried_by->equipment[i] == j) {
          static const char* const equipment_types[] = {"Special",
            "Worn on right finger", "Worn on left finger",
            "First worn around Neck", "Second worn around Neck", "Worn on body",
            "Worn on head", "Worn on legs", "Worn on feet", "Worn on hands",
            "Worn on arms", "Worn as shield", "Worn about body",
            "Worn around waist", "Worn around right wrist",
            "Worn around left wrist", "Wielded", "Held", "\n"};

          sprinttype(i, equipment_types, buf2);
          strcat(buf, buf2);
          found = 1;
        }
      }
      if (found == 0) {
        strcat(buf, "Inventory");
      }
    }
    send_to_char(buf, ch);

    strcpy(buf, "\n\rSpecial procedure : ");
    if (j->item_number >= 0) {
      strcat(buf,
        ((obj_index[j->item_number].func.obj_f != nullptr) ? "exists\n\r"
                                                           : "No\n\r"));
    } else {
      strcat(buf, "No\n\r");
    }
    send_to_char(buf, ch);

    strcpy(buf, "Contains :\n\r");
    found = 0;
    for (j2 = j->contains; j2 != nullptr; j2 = j2->next_content) {
      strcat(buf, fname(j2->name));
      strcat(buf, "\n\r");
      found = 1;
    }
    if (found == 0) {
      strcpy(buf, "Contains : Nothing\n\r");
    }
    send_to_char(buf, ch);

    send_to_char("Can affect char :\n\r", ch);
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      sprinttype(j->affected[i].location, apply_types, buf2);
      sprintf(buf, "    Affects : %s By %lu\n\r", buf2,
        j->affected[i].modifier);
      send_to_char(buf, ch);
    }
    return;
  }
  send_to_char("No mobile or object by that name in the world\n\r", ch);
}

void do_set(struct char_data* ch, const char* argument, int /*cmd*/) {
  char field[20];
  char name[20];
  char parmstr[50];
  struct char_data* mob = nullptr;
  int parm = 0;
  int parm2 = 0;
  char buf[256];

  if ((GetMaxLevel(ch) < SILLYLORD) || (IS_NPC(ch))) {
    return;
  }

  argument = one_argument(argument, field);
  argument = one_argument(argument, name);
  argument = one_argument(argument, parmstr);

  if ((mob = get_char_vis(ch, name)) == nullptr) {
    send_to_char("I don't see that here \n\r", ch);
    return;
  }

  if (strcmp(field, "align") == 0) {
    sscanf(parmstr, "%d", &parm);
    GET_ALIGNMENT(mob) = parm;
  } else if (strcmp(field, "class") == 0) {
    sscanf(parmstr, "%d", &parm);
    /*
    ** this will do almost nothing. (hopefully);
    */
    mob->player.char_class = parm;
  } else if (strcmp(field, "exp") == 0) {
    sscanf(parmstr, "%d", &parm);
    GET_EXP(mob) = parm;
  } else if (strcmp(field, "lev") == 0) {
    parm2 = 0; /* mage */
    sscanf(parmstr, "%d", &parm);
    argument = one_argument(argument, parmstr);
    sscanf(parmstr, "%d", &parm2);
    if (!IS_NPC(mob)) {
      if ((GetMaxLevel(mob) >= GetMaxLevel(ch)) && (ch != mob)) {
        send_to_char(GET_NAME(ch), mob);
        send_to_char(" just tried to change your level.\n\r", mob);
        return;
      }
    } else {
      if (parm2 < RANGER_LEVEL_IND) {
        GET_LEVEL(mob, parm2) = parm;
      }
      return;
    }

    if (parm < 0) {
      send_to_char("bug fix. :-)\n\r", ch);
      return;
    }

    if (parm < GetMaxLevel(ch) || (strcmp(GET_NAME(ch), "Loki") == 0)) {
      if (GetMaxLevel(ch) >= IMPLEMENTOR) {
        if (parm2 < RANGER_LEVEL_IND) {
          GET_LEVEL(mob, parm2) = parm;
        }
      } else {
        if (parm > DEMIGOD) {
          send_to_char("Sorry, you can't advance past 54th level\n", ch);
          return;
        }
        if (parm2 < RANGER_LEVEL_IND) {
          GET_LEVEL(mob, parm2) = parm;
        }
      }
    }
  } else if (strcmp(field, "sex") == 0) {
    if (is_number(parmstr) != 0) {
      sscanf(parmstr, "%d", &parm);
      GET_SEX(mob) = parm;
    } else {
      send_to_char("argument must be a number\n\r", ch);
    }
  } else if (strcmp(field, "race") == 0) {
    if (is_number(parmstr) != 0) {
      sscanf(parmstr, "%d", &parm);
      GET_RACE(mob) = parm;
    } else {
      send_to_char("argument must be a number\n\r", ch);
    }
  } else if (strcmp(field, "hunger") == 0) {
    sscanf(parmstr, "%d", &parm);
    GET_COND(mob, FULL) = parm;
  } else if (strcmp(field, "thirst") == 0) {
    sscanf(parmstr, "%d", &parm);
    GET_COND(mob, THIRST) = parm;
  } else if (strcmp(field, "hit") == 0) {
    sscanf(parmstr, "%d", &parm);
    GET_HIT(mob) = parm;
  } else if (strcmp(field, "mhit") == 0) {
    sscanf(parmstr, "%d", &parm);
    mob->points.max_hit = parm;
  } else if (strcmp(field, "board") == 0) {
    board_kludge_char = ch;
  } else if (strcmp(field, "tohit") == 0) {
    sscanf(parmstr, "%d", &parm);
    GET_HITROLL(mob) = parm;
  } else if (strcmp(field, "todam") == 0) {
    sscanf(parmstr, "%d", &parm);
    GET_DAMROLL(mob) = parm;
  } else if (strcmp(field, "ac") == 0) {
    sscanf(parmstr, "%d", &parm);
    GET_AC(mob) = parm;
  } else if (strcmp(field, "bank") == 0) {
    sscanf(parmstr, "%d", &parm);
    GET_BANK(mob) = parm;
  } else if (strcmp(field, "gold") == 0) {
    sscanf(parmstr, "%d", &parm);
    GET_GOLD(mob) = parm;
  } else if (strcmp(field, "prac") == 0) {
    sscanf(parmstr, "%d", &parm);
    mob->specials.spells_to_learn = parm;
  } else if (strcmp(field, "age") == 0) {
    sscanf(parmstr, "%d", &parm);
    mob->player.time.birth -= SECS_PER_MUD_YEAR * parm;

  } else if (strcmp(field, "str") == 0) {
    sscanf(parmstr, "%d", &parm);
    mob->abilities.str = parm;
    mob->tmpabilities.str = parm;
  } else if (strcmp(field, "add") == 0) {
    sscanf(parmstr, "%d", &parm);
    mob->abilities.str_add = parm;
    mob->tmpabilities.str_add = parm;
  } else if (strcmp(field, "saves") == 0) {
    parm = 0;
    parm2 = 0;
    sscanf(parmstr, "%d %d", &parm, &parm2);
    mob->specials.apply_saving_throw[parm] = parm2;

  } else if (strcmp(field, "skills") == 0) {
    parm = 0;
    parm2 = 0;
    sscanf(parmstr, "%d %d", &parm, &parm2);
    if (mob->skills != nullptr) {
      mob->skills[parm].learned = parm2;
      sprintf(buf, "You just set skill %d to value %d\n\r", parm, parm2);
      send_to_char(buf, ch);
    }

  } else if (strcmp(field, "stadd") == 0) {
    sscanf(parmstr, "%d", &parm);
    mob->abilities.str_add = parm;
    mob->tmpabilities.str_add = parm;

  } else if (strcmp(field, "int") == 0) {
    sscanf(parmstr, "%d", &parm);
    mob->abilities.intel = parm;
    mob->tmpabilities.intel = parm;

  } else if (strcmp(field, "wis") == 0) {
    sscanf(parmstr, "%d", &parm);
    mob->abilities.wis = parm;
    mob->tmpabilities.wis = parm;

  } else if (strcmp(field, "dex") == 0) {
    sscanf(parmstr, "%d", &parm);
    mob->abilities.dex = parm;
    mob->tmpabilities.dex = parm;

  } else if (strcmp(field, "con") == 0) {
    sscanf(parmstr, "%d", &parm);
    mob->abilities.con = parm;
    mob->tmpabilities.con = parm;

  } else if (strcmp(field, "mana") == 0) {
    sscanf(parmstr, "%d", &parm);
    mob->points.mana = parm;
    mob->points.max_mana = parm;

  } else {
    send_to_char(" Only align allowed \n\r", ch);
    return;
  }
}

void do_shutdow(struct char_data* ch, const char* /*argument*/, int /*cmd*/) {
  send_to_char("If you want to shut something down - say so!\n\r", ch);
}

void do_shutdown(struct char_data* ch, const char* argument, int /*cmd*/) {
  char buf[100];
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)) {
    return;
  }

  one_argument(argument, arg);

  if (*arg == 0) {
    sprintf(buf, "Shutdown by %s.", GET_NAME(ch));
    send_to_all(buf);
    vlog(buf);
    Shutdown = 1;
  } else if (str_cmp(arg, "reboot") == 0) {
    sprintf(buf, "Reboot by %s.", GET_NAME(ch));
    send_to_all(buf);
    vlog(buf);
    Shutdown = rebootmud = 1;
  } else {
    send_to_char("Go shut down someone your own size.\n\r", ch);
  }
}

void do_snoop(struct char_data* ch, const char* argument, int /*cmd*/) {
  static char arg[MAX_STRING_LENGTH];
  struct char_data* victim = nullptr;

  if (ch->desc == nullptr) {
    return;
  }

  if (IS_NPC(ch)) {
    return;
  }

  only_argument(argument, arg);

  if (*arg == 0) {
    send_to_char("Snoop who ?\n\r", ch);
    return;
  }

  if ((victim = get_char_vis(ch, arg)) == nullptr) {
    send_to_char("No such person around.\n\r", ch);
    return;
  }

  if (victim->desc == nullptr) {
    send_to_char("There's no link.. nothing to snoop.\n\r", ch);
    return;
  }
  if (victim == ch) {
    send_to_char("Ok, you just snoop yourself.\n\r", ch);
    if (ch->desc->snoop.snooping != nullptr) {
      if (ch->desc->snoop.snooping->desc != nullptr) {
        ch->desc->snoop.snooping->desc->snoop.snoop_by = nullptr;
      } else {
        char buf[MAX_STRING_LENGTH];
        sprintf(buf, "caught %s snooping %s who didn't have a descriptor!",
          ch->player.name, ch->desc->snoop.snooping->player.name);
        vlog(buf);
      }
      ch->desc->snoop.snooping = nullptr;
    }
    return;
  }

  if (victim->desc->snoop.snoop_by != nullptr) {
    send_to_char("Busy already. \n\r", ch);
    return;
  }

  if (GetMaxLevel(victim) >= GetMaxLevel(ch)) {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  send_to_char("Ok. \n\r", ch);

  if (ch->desc->snoop.snooping != nullptr) {
    if (ch->desc->snoop.snooping->desc != nullptr) {
      ch->desc->snoop.snooping->desc->snoop.snoop_by = nullptr;
    }
  }

  ch->desc->snoop.snooping = victim;
  victim->desc->snoop.snoop_by = ch;
}

void do_switch(struct char_data* ch, const char* argument, int /*cmd*/) {
  static char arg[80];
  struct char_data* victim = nullptr;

  if (IS_NPC(ch)) {
    return;
  }

  only_argument(argument, arg);

  if (*arg == 0) {
    send_to_char("Switch with who?\n\r", ch);
  } else {
    if ((victim = get_char(arg)) == nullptr) {
      send_to_char("They aren't here.\n\r", ch);
    } else {
      if (ch == victim) {
        send_to_char("He he he... We are jolly funny today, eh?\n\r", ch);
        return;
      }

      if ((ch->desc == nullptr) || (ch->desc->snoop.snoop_by != nullptr) ||
          (ch->desc->snoop.snooping != nullptr)) {
        send_to_char("Mixing snoop & switch is bad for your health.\n\r", ch);
        return;
      }

      if ((victim->desc != nullptr) || (!IS_NPC(victim))) {
        send_to_char("You can't do that, the body is already in use!\n\r", ch);
      } else {
        send_to_char("Ok.\n\r", ch);

        ch->desc->character = victim;
        ch->desc->original = ch;

        victim->desc = ch->desc;
        ch->desc = nullptr;
      }
    }
  }
}

void do_return(struct char_data* ch, const char* /*argument*/, int cmd) {
  struct char_data* mob = nullptr;
  struct char_data* per = nullptr;

  if (ch->desc == nullptr) {
    return;
  }

  if (ch->desc->original == nullptr) {
    send_to_char("Arglebargle, glop-glyf!?!\n\r", ch);
    return;
  }
  send_to_char("You return to your original body.\n\r", ch);

  if (IS_SET(ch->specials.act, ACT_POLYSELF) && (cmd != 0)) {
    mob = ch;
    per = ch->desc->original;

    act("$n turns liquid, and reforms as $N", 1, mob, nullptr, per, TO_ROOM);

    char_from_room(per);
    char_to_room(per, mob->in_room);

    SwitchStuff(mob, per);
  }

  ch->desc->character = ch->desc->original;
  ch->desc->original = nullptr;

  ch->desc->character->desc = ch->desc;
  ch->desc = nullptr;

  if (IS_SET(ch->specials.act, ACT_POLYSELF) && (cmd != 0)) {
    extract_char(mob);
  }
}

void do_force(struct char_data* ch, const char* argument, int cmd) {
  struct descriptor_data* i = nullptr;
  struct char_data* vict = nullptr;
  char name[100];
  char to_force[100];
  char buf[100];

  if (IS_NPC(ch) && (cmd != 0)) {
    return;
  }

  half_chop(argument, name, to_force);

  if ((*name == 0) || (*to_force == 0)) {
    send_to_char("Who do you wish to force to do what?\n\r", ch);
  } else if (str_cmp("all", name) != 0) {
    if ((vict = get_char_vis(ch, name)) == nullptr) {
      send_to_char("No-one by that name here..\n\r", ch);
    } else {
      if ((GetMaxLevel(ch) <= GetMaxLevel(vict)) && (!IS_NPC(vict))) {
        send_to_char("Oh no you don't!!\n\r", ch);
      } else {
        sprintf(buf, "$n has forced you to '%s'.", to_force);
        act(buf, 0, ch, nullptr, vict, TO_VICT);
        send_to_char("Ok.\n\r", ch);
        command_interpreter(vict, to_force);
      }
    }
  } else { /* force all */
    for (i = descriptor_list; i != nullptr; i = i->next) {
      if (i->character != ch && (i->connected == 0) &&
          i->character != board_kludge_char) {
        vict = i->character;
        if ((GetMaxLevel(ch) <= GetMaxLevel(vict)) && (!IS_NPC(vict))) {
          send_to_char("Oh no you don't!!\n\r", ch);
        } else {
          sprintf(buf, "$n has forced you to '%s'.", to_force);
          act(buf, 0, ch, nullptr, vict, TO_VICT);
          command_interpreter(vict, to_force);
        }
      }
    }
    send_to_char("Ok.\n\r", ch);
  }
}

void do_load(struct char_data* ch, const char* argument, int /*cmd*/) {
  struct char_data* mob = nullptr;
  struct obj_data* obj = nullptr;
  char type[100];
  char num[100];
  int number = 0;

  if (IS_NPC(ch)) {
    return;
  }

  argument = one_argument(argument, type);

  only_argument(argument, num);
  if (isdigit(*num) != 0) {
    number = atoi(num);
  } else {
    number = -1;
  }

  if (is_abbrev(type, "mobile") != 0) {
    if (!IS_IMMORTAL(ch)) {
      return;
    }
    if (number < 0) {
      for (number = 0; number <= top_of_mobt; number++) {
        if (isname(num, mob_index[number].name) != 0) {
          break;
        }
      }
      if (number > top_of_mobt) {
        number = -1;
      }
    } else {
      number = real_mobile(number);
    }
    if (number < 0 || number > top_of_mobt) {
      send_to_char("There is no such monster.\n\r", ch);
      return;
    }
    mob = read_mobile(number, REAL);
    char_to_room(mob, ch->in_room);

    act("$n makes a quaint, magical gesture with one hand.", 1, ch, nullptr,
      nullptr, TO_ROOM);
    act("$n has summoned $N from the ether!", 0, ch, nullptr, mob, TO_ROOM);
    act("You bring forth $N from the the cosmic ether.", 0, ch, nullptr, mob,
      TO_CHAR);

  } else if (is_abbrev(type, "object") != 0) {
    if (!IS_IMMORTAL(ch)) {
      return;
    }
    if (number < 0) {
      for (number = 0; number <= top_of_objt; number++) {
        if (isname(num, obj_index[number].name) != 0) {
          break;
        }
      }
      if (number > top_of_objt) {
        number = -1;
      }
    } else {
      number = real_object(number);
    }
    if (number < 0 || number > top_of_objt) {
      send_to_char("There is no such object.\n\r", ch);
      return;
    }

    if (GetMaxLevel(ch) < BRUTIUS) {
      switch (obj_index[number].vnum) {
        case 5021:
          send_to_char(
            "no.  No more bows!  And don't kill the worm either!\n\r", ch);
          return;
        case 5112:
          send_to_char(
            "no, no more Ruby rings!  And don't kill for it either!\n\r", ch);
          return;
        case 9002:
          send_to_char(
            "These dice are unloadable by anyone but Brutius. More than one "
            "pair in the game could wreck havok!\n\r",
            ch);
          return;
        case 233:
        case 15831:
        case 25001:
        case 29001:
        case 29002:
        case 29003:
        case 29004:
        case 29005:
        case 29006:
        case 29007:
        case 29994:
        case 30012:
        case 30013:
        case 30014:
        case 30015:
        case 30016:
        case 30017:
        case 30018:
          send_to_char("When monkeys fly out of your butt.\n\r", ch);
          return;
        case 21113:
        case 21117:
        case 21120:
        case 21121:
        case 21122:
          send_to_char("You can't load this item, sorry.\n\r", ch);
          return;
      }
    }

    obj = read_object(number, REAL);
    obj_to_char(obj, ch);
    act("$n makes a strange magical gesture.", 1, ch, nullptr, nullptr,
      TO_ROOM);
    act("$n has created $p!", 0, ch, obj, nullptr, TO_ROOM);
    act("You now have $p.", 0, ch, obj, nullptr, TO_CHAR);
  } else if (is_abbrev(type, "room") != 0) {
    int start = 0;
    int end = 0;

    if (GetMaxLevel(ch) < CREATOR) {
      return;
    }

    switch (sscanf(num, "%d %d", &start, &end)) {
      case 2: /* we got both numbers */
        room_load(ch, start, end);
        break;
      case 1: /* we only got one, load it */
        room_load(ch, start, start);
        break;
      default:
        send_to_char("Load? Fine!  Load we must, But what?\n\r", ch);
        break;
    }
  } else {
    send_to_char("Usage: load (object|mobile) (number|name)\n\r", ch);
    send_to_char("       load room start [end]\n\r", ch);
  }
}

static void completely_cleanout_room(struct room_data* rp) {
  struct char_data* ch = nullptr;
  struct obj_data* obj = nullptr;

  while (rp->people != nullptr) {
    ch = rp->people;
    act(
      "The hand of god sweeps across the land and you are swept into the "
      "Void.",
      0, nullptr, nullptr, nullptr, TO_VICT);
    char_from_room(ch);
    char_to_room(ch, 0); /* send character to the void */
  }

  while (rp->contents != nullptr) {
    obj = rp->contents;
    obj_from_room(obj);
    obj_to_room(obj, 0); /* send item to the void */
  }

  cleanout_room(rp);
}

static void purge_one_room(int rnum, struct room_data* rp, const int* range) {
  struct char_data* ch = nullptr;
  struct obj_data* obj = nullptr;

  if (rnum == 0 || /* purge the void?  I think not */
      rnum < range[0] || rnum > range[1]) {
    return;
  }

  while (rp->people != nullptr) {
    ch = rp->people;
    send_to_char(
      "A god strikes the heavens making the ground around you erupt into a",
      ch);
    send_to_char(
      "fluid fountain boiling into the ether.  All that's left is the Void.",
      ch);
    char_from_room(ch);
    char_to_room(ch, 0); /* send character to the void */
    do_look(ch, "", 15);
    act("$n tumbles into the Void.", 1, ch, nullptr, nullptr, TO_ROOM);
  }

  while (rp->contents != nullptr) {
    obj = rp->contents;
    obj_from_room(obj);
    obj_to_room(obj, 0); /* send item to the void */
  }

  completely_cleanout_room(rp); /* clear out the pointers */
  room_remove(room_db, rnum);
  room_count--;
}

void do_link(struct char_data* ch, const char* argument, int /*cmd*/) {
  struct descriptor_data* d = nullptr;
  struct descriptor_data* next_d = nullptr;
  int done = 0;
  char name[100];

  if (IS_NPC(ch)) {
    return;
  }

  argument = one_argument(argument, name);

  if (*name == 0) {
    for (d = descriptor_list; d != nullptr; d = next_d) {
      next_d = d->next;
      if ((d->character == nullptr) || (d->character->player.name == nullptr)) {
        close_socket(d);
      }
    }
  } else {
    for (d = descriptor_list; (d != nullptr) && (done == 0); d = d->next) {
      if (d->character != nullptr) {
        if (GET_NAME(d->character) &&
            (str_cmp(GET_NAME(d->character), name) == 0)) {
          done = 1;
          break;
        }
      }
    }

    if (d != nullptr) {
      close_socket(d);
    }
  }
}

/* clean a room of all mobiles and objects */
void do_purge(struct char_data* ch, const char* argument, int /*cmd*/) {
  struct char_data* vict = nullptr;
  struct char_data* next_v = nullptr;
  struct obj_data* obj = nullptr;
  struct obj_data* next_o = nullptr;

  char name[100];

  if (IS_NPC(ch)) {
    return;
  }

  only_argument(argument, name);

  if (*name != 0) { /* argument supplied. destroy single object or char */
    if (strcmp(name, "links") == 0 && GetMaxLevel(ch) >= IMPLEMENTOR) {
      struct descriptor_data* d = nullptr;
      struct descriptor_data* next_d = nullptr;

      for (d = descriptor_list; d != nullptr; d = next_d) {
        next_d = d->next;
        if ((d->character != nullptr) && d->character->specials.timer > 10) {
          close_socket(d);
        }
      }
      return;
    }

    if ((vict = get_char_room_vis(ch, name)) != nullptr) {
      if ((!IS_NPC(vict) || IS_SET(vict->specials.act, ACT_POLYSELF)) &&
          (GetMaxLevel(ch) < 53)) {
        send_to_char("I can't let you do that.\n\r", ch);
        return;
      }

      act("$n disintegrates $N.", 0, ch, nullptr, vict, TO_NOTVICT);

      if (IS_NPC(vict) || (!IS_SET(ch->specials.act, ACT_POLYSELF))) {
        extract_char(vict);
      } else {
        if (vict->desc != nullptr) {
          close_socket(vict->desc);
          vict->desc = nullptr;
          extract_char(vict);
        } else {
          extract_char(vict);
        }
      }
    } else if ((obj = get_obj_in_list_vis(ch, name,
                  real_roomp(ch->in_room)->contents)) != nullptr) {
      act("$n destroys $p.", 0, ch, obj, nullptr, TO_ROOM);
      extract_obj(obj);
    } else {
      argument = one_argument(argument, name);
      if (0 == str_cmp("room", name)) {
        int range[2];
        int i = 0;
        struct room_data* rp = nullptr;
        if (GetMaxLevel(ch) < IMPLEMENTOR) {
          send_to_char("I'm sorry, Dave.  I can't let you do that.\n\r", ch);
          return;
        }
        argument = one_argument(argument, name);
        if (isdigit(*name) == 0) {
          send_to_char("purge room start [end]", ch);
          return;
        }
        range[0] = atoi(name);
        argument = one_argument(argument, name);
        if (isdigit(*name) != 0) {
          range[1] = atoi(name);
        } else {
          range[1] = range[0];
        }

        if (range[0] == 0 || range[1] == 0) {
          send_to_char("usage: purge room start [end]\n\r", ch);
          return;
        }
        if (range[0] >= WORLD_SIZE || range[1] >= WORLD_SIZE) {
          send_to_char("only purging to WORLD_SIZE\n\r", ch);
          return;
        }
        for (i = range[0]; i <= range[1]; i++) {
          if ((rp = real_roomp(i)) != nullptr) {
            purge_one_room(i, rp, range);
          }
        }
      } else {
        send_to_char("I don't see that here.\n\r", ch);
        return;
      }
    }

    send_to_char("Ok.\n\r", ch);
  } else { /* no argument. clean out the room */
    if (GetMaxLevel(ch) < DEMIGOD) {
      return;
    }
    if (IS_NPC(ch)) {
      send_to_char("You would only kill yourself..\n\r", ch);
      return;
    }

    act(
      "$n gestures... You are surrounded by thousands of tiny scrubbing "
      "bubbles!",
      0, ch, nullptr, nullptr, TO_ROOM);
    send_to_room("The world seems a little cleaner.\n\r", ch->in_room);

    for (vict = real_roomp(ch->in_room)->people; vict != nullptr;
      vict = next_v) {
      next_v = vict->next_in_room;
      if (IS_NPC(vict) && (!IS_SET(vict->specials.act, ACT_POLYSELF))) {
        extract_char(vict);
      }
    }

    for (obj = real_roomp(ch->in_room)->contents; obj != nullptr;
      obj = next_o) {
      next_o = obj->next_content;
      extract_obj(obj);
    }
  }
}

/* Give pointers to the five abilities */
static void roll_abilities(struct char_data* ch) {
  int i = 0;
  int j = 0;
  int k = 0;
  int temp = 0;
  unsigned char table[MAX_STAT];
  unsigned char rools[4];

  for (i = 0; i < MAX_STAT; table[i++] = 0) {
    ;
  }

  for (i = 0; i < MAX_STAT; i++) {
    for (j = 0; j < 4; j++) {
      rools[j] = number(1, 6);
    }

    temp = rools[0] + rools[1] + rools[2] + rools[3] -
           MIN(rools[0], MIN(rools[1], MIN(rools[2], rools[3])));

    for (k = 0; k < MAX_STAT; k++)
      if (table[k] < temp)
        SWITCH(temp, table[k]);
  }

  for (i = 0; i < MAX_STAT; i++) {
    switch (ch->desc->stat[i]) {
      case 's':
        ch->abilities.str = table[i];
        break;
      case 'i':
        ch->abilities.intel = table[i];
        break;
      case 'd':
        ch->abilities.dex = table[i];
        break;
      case 'w':
        ch->abilities.wis = table[i];
        break;
      case 'o':
        ch->abilities.con = table[i];
        break;
      default:
        break;
    }
  }

  if (ch->abilities.str == 0) {
    ch->abilities.str = 11;
  }
  if (ch->abilities.intel == 0) {
    ch->abilities.intel = 11;
  }
  if (ch->abilities.dex == 0) {
    ch->abilities.dex = 11;
  }
  if (ch->abilities.wis == 0) {
    ch->abilities.wis = 11;
  }
  if (ch->abilities.con == 0) {
    ch->abilities.con = 11;
  }

  ch->abilities.str_add = 0;

  if (GetMaxLevel(ch) < 2) {
    ch->points.max_hit = HowManyClasses(ch) * 10;

    if (HasClass(ch, CLASS_MAGIC_USER) != 0) {
      ch->points.max_hit += number(1, 4);
    }
    if (HasClass(ch, CLASS_CLERIC) != 0) {
      ch->points.max_hit += number(1, 8);
    }
    if (HasClass(ch, CLASS_WARRIOR) != 0) {
      ch->points.max_hit += number(1, 10);
      if (ch->abilities.str == 18) {
        ch->abilities.str_add = number(0, 100);
      }
    }
    if (HasClass(ch, CLASS_THIEF) != 0) {
      ch->points.max_hit += number(1, 6);
    }
    if (HasClass(ch, CLASS_MONK) != 0) {
      ch->points.max_hit += number(1, 6);
    }

    ch->points.max_hit /= HowManyClasses(ch);
  }

  if (GET_RACE(ch) == RACE_ELVEN) {
    ch->abilities.dex++;
    ch->abilities.con--;
  } else if (GET_RACE(ch) == RACE_DWARF) {
    ch->abilities.con++;
    ch->abilities.dex--;
  } else if (GET_RACE(ch) == RACE_GNOME) {
    ch->abilities.wis++;
    ch->abilities.dex--;
  } else if (GET_RACE(ch) == RACE_OGRE) {
    ch->abilities.str++;
    ch->abilities.str++;
    ch->abilities.wis--;
    ch->abilities.intel--;
  } else if (GET_RACE(ch) == RACE_HOBBIT) {
    ch->abilities.dex++;
    ch->abilities.wis--;
  }
  ch->tmpabilities = ch->abilities;
}

void do_reroll(struct char_data* ch, const char* /*argument*/, int /*cmd*/) {
  if (IS_NPC(ch)) {
    return;
  }
}

static void start_levels(struct char_data* ch) {
  if (IS_SET(ch->player.char_class, CLASS_MAGIC_USER)) {
    advance_level(ch, MAGE_LEVEL_IND);
  }
  if (IS_SET(ch->player.char_class, CLASS_CLERIC)) {
    advance_level(ch, CLERIC_LEVEL_IND);
  }
  if (IS_SET(ch->player.char_class, CLASS_WARRIOR)) {
    advance_level(ch, WARRIOR_LEVEL_IND);
  }
  if (IS_SET(ch->player.char_class, CLASS_THIEF)) {
    advance_level(ch, THIEF_LEVEL_IND);
  }
  if (IS_SET(ch->player.char_class, CLASS_ANTIPALADIN)) {
    advance_level(ch, ANTIPALADIN_LEVEL_IND);
  }
  if (IS_SET(ch->player.char_class, CLASS_RANGER)) {
    advance_level(ch, RANGER_LEVEL_IND);
  }
  if (IS_SET(ch->player.char_class, CLASS_MONK)) {
    advance_level(ch, MONK_LEVEL_IND);
  }
  if (IS_SET(ch->player.char_class, CLASS_PALADIN)) {
    advance_level(ch, PALADIN_LEVEL_IND);
  }
}

void do_start(struct char_data* ch) {
  int r_num = 0;
  struct obj_data* obj = nullptr;

  send_to_char("Welcome to SneezyMUD.  Enjoy the game...\n\r", ch);
  *(ch->player.title) = '0';

  start_levels(ch);

  GET_EXP(ch) = 1;

  set_title(ch);

  roll_abilities(ch);
  ch->points.max_hit = 20; /* These are BASE numbers   */

  /*
     outfit char with valueless items
  */
  if ((r_num = real_object(12)) >= 0) {
    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch); /* bread   */
    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch); /* bread   */
  }

  if ((r_num = real_object(13)) >= 0) {
    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch); /* water   */

    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch); /* water   */
  }

  if ((r_num = real_object(29999)) >= 0) {
    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch); /*radio*/
  }

  if (IS_SET(ch->player.char_class, CLASS_THIEF)) {
    ch->skills[SKILL_DETECT_SECRET].learned = 5;
    ch->skills[SKILL_SNEAK].learned = 10;
    ch->skills[SKILL_HIDE].learned = 5;
    ch->skills[SKILL_STEAL].learned = 15;
    ch->skills[SKILL_BACKSTAB].learned = 10;
    ch->skills[SKILL_PICK_LOCK].learned = 10;
  }

  if (IS_SET(ch->player.char_class, CLASS_ANTIPALADIN)) {
    GET_ALIGNMENT(ch) = -1000;
  }
  if (IS_SET(ch->player.char_class, CLASS_PALADIN)) {
    GET_ALIGNMENT(ch) = 1000;
  }

  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);
  ch->points.max_move += GET_CON(ch) + number(1, 20) - 9;
  GET_MOVE(ch) = GET_MAX_MOVE(ch);

  GET_COND(ch, THIRST) = 24;
  GET_COND(ch, FULL) = 24;
  GET_COND(ch, DRUNK) = 0;

  ch->player.time.played = 0;
  ch->player.time.logon = time(nullptr);
}

static void gain_exp_regardless(struct char_data* ch, int gain,
  int char_class) {
  int i = 0;
  char is_altered = 0;

  save_char(ch, AUTO_RENT);
  if (!IS_NPC(ch)) {
    if (gain > 0) {
      GET_EXP(ch) += gain;

      for (i = 0;
        (i < ABS_MAX_LVL) && (titles[char_class][i].exp <= GET_EXP(ch)); i++) {
        if (i > GET_LEVEL(ch, char_class)) {
          send_to_char("You raise a level\n\r", ch);
          GET_LEVEL(ch, char_class) = i;
          advance_level(ch, char_class);
          is_altered = 1;
        }
      }
    }
    if (gain < 0) {
      GET_EXP(ch) += gain;
    }
    GET_EXP(ch) = std::max(GET_EXP(ch), 0);
  }
  if (is_altered != 0) {
    set_title(ch);
  }
}

void do_advance(struct char_data* ch, const char* argument, int /*cmd*/) {
  struct char_data* victim = nullptr;
  char name[100];
  char level[100];
  char char_class_arg[100];
  int adv = 0;
  int newlevel = 0;
  int lin_class = 0;

  if (IS_NPC(ch)) {
    return;
  }

  argument = one_argument(argument, name);

  if (*name != 0) {
    if ((victim = get_char_room_vis(ch, name)) == nullptr) {
      send_to_char("That player is not here.\n\r", ch);
      return;
    }
  } else {
    send_to_char("Advance who?\n\r", ch);
    return;
  }

  if (IS_NPC(victim)) {
    send_to_char("NO! Not on NPC's.\n\r", ch);
    return;
  }

  argument = one_argument(argument, char_class_arg);

  if (*char_class_arg == 0) {
    send_to_char("Supply a class: M C W T\n\r", ch);
    return;
  }

  switch (*char_class_arg) {
    case 'M':
    case 'm':
      lin_class = MAGE_LEVEL_IND;
      break;

    case 'T':
    case 't':
      lin_class = THIEF_LEVEL_IND;
      break;

    case 'W':
    case 'w':
    case 'F':
    case 'f':
      lin_class = WARRIOR_LEVEL_IND;
      break;

    case 'C':
    case 'c':
    case 'P':
    case 'p':
      lin_class = CLERIC_LEVEL_IND;
      break;

    default:
      send_to_char("Must supply a class (M C W T)\n\r", ch);
      return;
      break;
  }

  argument = one_argument(argument, level);

  if (GET_LEVEL(victim, lin_class) == 0) {
    adv = 1;
  } else if (*level == 0) {
    send_to_char("You must supply a level number.\n\r", ch);
    return;
  } else {
    if (isdigit(*level) == 0) {
      send_to_char("Third argument must be a positive integer.\n\r", ch);
      return;
    }
    if ((newlevel = atoi(level)) < GET_LEVEL(victim, lin_class)) {
      send_to_char("Can't dimish a players status (yet).\n\r", ch);
      return;
    }
    adv = newlevel - GET_LEVEL(victim, lin_class);
  }

  if (((adv + GET_LEVEL(victim, lin_class)) > 1) &&
      (GetMaxLevel(ch) < IMPLEMENTOR)) {
    send_to_char("Thou art not godly enough.\n\r", ch);
    return;
  }

  if ((adv + GET_LEVEL(victim, lin_class)) > IMPLEMENTOR) {
    send_to_char("Implementor is the highest possible level.\n\r", ch);
    return;
  }

  if (((adv + GET_LEVEL(victim, lin_class)) < 1) &&
      ((adv + GET_LEVEL(victim, lin_class)) != 1)) {
    send_to_char("1 is the lowest possible level.\n\r", ch);
    return;
  }

  send_to_char("You feel generous.\n\r", ch);
  act(
    "$n makes some strange gestures.\n\rA strange feeling comes upon you,"
    "\n\rLike a giant hand, light comes down from\n\rabove, grabbing your "
    "body, that begins\n\rto pulse with coloured lights from inside.\n\rYo"
    "ur head seems to be filled with daemons\n\rfrom another plane as your"
    " body dissolves\n\rinto the elements of time and space itself.\n\rSudde"
    "nly a silent explosion of light snaps\n\ryou back to reality. You fee"
    "l slightly\n\rdifferent.",
    0, ch, nullptr, victim, TO_VICT);

  if (GET_LEVEL(victim, lin_class) == 0) {
    do_start(victim);
  } else {
    if (GET_LEVEL(victim, lin_class) < IMPLEMENTOR) {
      gain_exp_regardless(victim,
        (titles[lin_class][GET_LEVEL(victim, lin_class) + adv].exp) -
          GET_EXP(victim),
        lin_class);

      send_to_char("Character is now advanced.\n\r", ch);
    } else {
      send_to_char("Some idiot just tried to advance your level.\n\r", victim);
      send_to_char("IMPOSSIBLE! IDIOTIC!\n\r", ch);
    }
  }
}

void do_wiznews(struct char_data* ch, const char* /*argument*/, int cmd) {
  if ((cmd == 0) || (ch->desc == nullptr) || ((ch->desc->connected) != 0)) {
    return;
  }

  start_page_file(ch->desc, WIZNEWS_FILE, "No news for the immorts!\n\r");
}

void do_restore(struct char_data* ch, const char* argument, int cmd) {
  struct char_data* victim = nullptr;
  char buf[100];
  int i = 0;

  if (cmd == 0) {
    return;
  }

  only_argument(argument, buf);
  if (*buf == 0) {
    send_to_char("Who do you wish to restore?\n\r", ch);
  } else if ((victim = get_char(buf)) == nullptr) {
    send_to_char("No-one by that name in the world.\n\r", ch);
  } else {
    GET_MANA(victim) = GET_MAX_MANA(victim);
    GET_HIT(victim) = GET_MAX_HIT(victim);
    GET_MOVE(victim) = GET_MAX_MOVE(victim);

    if (GetMaxLevel(victim) >= 51) {
      victim->specials.conditions[DRUNK] = -1;
      victim->specials.conditions[FULL] = -1;
      victim->specials.conditions[THIRST] = -1;
    }

    if (IS_NPC(victim)) {
      return;
    }

    if (GetMaxLevel(victim) >= CREATOR) {
      for (i = 0; i < MAX_SKILLS; i++) {
        victim->skills[i].learned = 100;
        victim->skills[i].recognise = 1;
      }

      if (GetMaxLevel(victim) >= GOD) {
        victim->abilities.str_add = 100;
        victim->abilities.intel = 25;
        victim->abilities.wis = 25;
        victim->abilities.dex = 25;
        victim->abilities.str = 25;
        victim->abilities.con = 25;
      }
      victim->tmpabilities = victim->abilities;
    }
    update_pos(victim);
    send_to_char("Done.\n\r", ch);
    act("You have been fully healed by $N!", 0, victim, nullptr, ch, TO_CHAR);
  }
}

void do_noshout(struct char_data* ch, const char* argument, int /*cmd*/) {
  struct char_data* vict = nullptr;
  struct obj_data* dummy = nullptr;
  char buf[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)) {
    return;
  }

  only_argument(argument, buf);

  if (*buf == 0) {
    if (IS_SET(ch->specials.act, PLR_NOSHOUT)) {
      send_to_char("You can now hear shouts again.\n\r", ch);
      REMOVE_BIT(ch->specials.act, PLR_NOSHOUT);
    } else {
      send_to_char("From now on, you won't hear shouts.\n\r", ch);
      SET_BIT(ch->specials.act, PLR_NOSHOUT);
    }
  } else if (generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy) == 0) {
    send_to_char("Couldn't find any such creature.\n\r", ch);
  } else if (IS_NPC(vict)) {
    send_to_char("Can't do that to a beast.\n\r", ch);
  } else if (GetMaxLevel(vict) >= GetMaxLevel(ch)) {
    act("$E might object to that.. better not.", 0, ch, nullptr, vict, TO_CHAR);
  } else if (IS_SET(vict->specials.act, PLR_NOSHOUT) &&
             (GetMaxLevel(ch) >= SAINT)) {
    send_to_char("You can shout again.\n\r", vict);
    send_to_char("NOSHOUT removed.\n\r", ch);
    REMOVE_BIT(vict->specials.act, PLR_NOSHOUT);
  } else if (GetMaxLevel(ch) >= SAINT) {
    send_to_char("The gods take away your ability to shout!\n\r", vict);
    send_to_char("NOSHOUT set.\n\r", ch);
    SET_BIT(vict->specials.act, PLR_NOSHOUT);
  } else {
    send_to_char("Sorry, you can't do that\n\r", ch);
  }
}

void do_nohassle(struct char_data* ch, const char* argument, int /*cmd*/) {
  struct char_data* vict = nullptr;
  struct obj_data* dummy = nullptr;
  char buf[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)) {
    return;
  }

  only_argument(argument, buf);

  if (*buf == 0) {
    if (IS_SET(ch->specials.act, PLR_NOHASSLE)) {
      send_to_char("You can now be hassled again.\n\r", ch);
      REMOVE_BIT(ch->specials.act, PLR_NOHASSLE);
    } else {
      send_to_char("From now on, you won't be hassled.\n\r", ch);
      SET_BIT(ch->specials.act, PLR_NOHASSLE);
    }
  } else if (generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy) == 0) {
    send_to_char("Couldn't find any such creature.\n\r", ch);
  } else if (IS_NPC(vict)) {
    send_to_char("Can't do that to a beast.\n\r", ch);
  } else if (GetMaxLevel(vict) > GetMaxLevel(ch)) {
    act("$E might object to that.. better not.", 0, ch, nullptr, vict, TO_CHAR);
  } else {
    send_to_char("The implementor won't let you set this on mortals...\n\r",
      ch);
  }
}

void do_stealth(struct char_data* ch, const char* argument, int /*cmd*/) {
  struct char_data* vict = nullptr;
  struct obj_data* dummy = nullptr;
  char buf[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)) {
    return;
  }

  only_argument(argument, buf);

  if (*buf == 0) {
    if (IS_SET(ch->specials.act, PLR_STEALTH)) {
      send_to_char("STEALTH mode OFF.\n\r", ch);
      REMOVE_BIT(ch->specials.act, PLR_STEALTH);
    } else {
      send_to_char("STEALTH mode ON.\n\r", ch);
      SET_BIT(ch->specials.act, PLR_STEALTH);
    }
  } else if (generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy) == 0) {
    send_to_char("Couldn't find any such creature.\n\r", ch);
  } else if (IS_NPC(vict)) {
    send_to_char("Can't do that to a beast.\n\r", ch);
  } else if (GetMaxLevel(vict) > GetMaxLevel(ch)) {
    act("$E might object to that.. better not.", 0, ch, nullptr, vict, TO_CHAR);
  } else {
    send_to_char("The implementor won't let you set this on mortals...\n\r",
      ch);
  }
}

static void print_room(int rnum, struct room_data* rp,
  struct string_block* sb) {
  char buf[MAX_STRING_LENGTH];
  int dink = 0;
  int bits = 0;
  int scan = 0;

  if ((rp->sector_type < 0) || (rp->sector_type > 9)) { /* non-optimal */
    rp->sector_type = 0;
  }
  sprintf(buf, "%5d %4d %-12s %s", rp->number, rnum,
    sector_types[rp->sector_type],
    ((rp->name != nullptr) ? rp->name : "Empty"));
  strcat(buf, " [");

  dink = 0;
  for (bits = rp->room_flags, scan = 0; bits != 0; scan++) {
    if ((bits & (1 << scan)) != 0) {
      if (dink != 0) {
        strcat(buf, " ");
      }
      strcat(buf, room_bits[scan]);
      dink = 1;
      bits ^= (1 << scan);
    }
  }
  strcat(buf, "]\n\r");

  append_to_string_block(sb, buf);
}

static void print_death_room(int rnum, struct room_data* rp, void* data) {
  struct string_block* sb = (struct string_block*)data;
  if ((rp != nullptr) && ((rp->room_flags & DEATH) != 0)) {
    print_room(rnum, rp, sb);
  }
}

static void print_private_room(int rnum, struct room_data* rp, void* data) {
  struct string_block* sb = (struct string_block*)data;
  if ((rp != nullptr) && ((rp->room_flags & PRIVATE) != 0)) {
    print_room(rnum, rp, sb);
  }
}

struct show_room_zone_struct {
    int blank;
    int startblank, lastblank;
    int bottom, top;
    struct string_block* sb;
};

static void show_room_zone(int rnum, struct room_data* rp, void* data) {
  struct show_room_zone_struct* srzs = (struct show_room_zone_struct*)data;
  char buf[MAX_STRING_LENGTH];

  if ((rp == nullptr) || rp->number < srzs->bottom || rp->number > srzs->top) {
    return; /* optimize later*/
  }

  if ((srzs->blank != 0) && (srzs->lastblank + 1 != rp->number)) {
    sprintf(buf, "rooms %d-%d are blank\n\r", srzs->startblank,
      srzs->lastblank);
    append_to_string_block(srzs->sb, buf);
    srzs->blank = 0;
  }
  if (rp->name == nullptr) {
    sprintf(buf, "room %d's name is screwed!\n\r", rp->number);
    return;
  }
  if (1 == sscanf(rp->name, "%d", &srzs->lastblank) &&
      srzs->lastblank == rp->number) {
    if (srzs->blank == 0) {
      srzs->startblank = srzs->lastblank;
      srzs->blank = 1;
    }
    return;
  }
  if (srzs->blank != 0) {
    sprintf(buf, "rooms %d-%d are blank\n\r", srzs->startblank,
      srzs->lastblank);
    append_to_string_block(srzs->sb, buf);
    srzs->blank = 0;
  }

  print_room(rnum, rp, srzs->sb);
}

static void room_iterate(struct room_data* rb[],
  void (*func)(int, struct room_data*, void*), void* cdata) {
  int i = 0;
  for (i = 0; i < WORLD_SIZE; i++) {
    struct room_data* temp = nullptr;

    temp = room_find(rb, i);
    if (temp != nullptr) {
      (*func)(i, temp, cdata);
    }
  }
}

void do_show(struct char_data* ch, const char* argument, int /*cmd*/) {
  int zone = 0;
  char buf[MAX_STRING_LENGTH];
  char zonenum[MAX_INPUT_LENGTH];
  struct index_data* which_i = nullptr;
  int bottom = 0;
  int top = 0;
  int topi = 0;
  struct string_block sb{};

  if (IS_NPC(ch)) {
    return;
  }

  argument = one_argument(argument, buf);

  init_string_block(&sb);

  if (is_abbrev(buf, "zones") != 0) {
    struct zone_data* zd = nullptr;
    int bottom = 0;
    append_to_string_block(&sb,
      "# Zone   name                                lifespan age     rooms   "
      "  "
      "reset\n\r");

    for (zone = 0; zone <= top_of_zone_table; zone++) {
      const char* mode = nullptr;

      zd = zone_table + zone;
      switch (zd->reset_mode) {
        case 0:
          mode = "never";
          break;
        case 1:
          mode = "ifempty";
          break;
        case 2:
          mode = "always";
          break;
        default:
          mode = "!unknown!";
          break;
      }
      sprintf(buf, "%4d %-40s %4dm %4dm %6d-%-6d %s\n\r", zone, zd->name,
        zd->lifespan, zd->age, bottom, zd->top, mode);
      append_to_string_block(&sb, buf);
      bottom = zd->top + 1;
    }

  } else if (((is_abbrev(buf, "objects") != 0) &&
               ((which_i = obj_index, topi = top_of_objt) != 0)) ||
             ((is_abbrev(buf, "mobiles") != 0) &&
               ((which_i = mob_index, topi = top_of_mobt) != 0))) {
    int objn = 0;
    struct index_data* oi = nullptr;

    only_argument(argument, zonenum);
    zone = -1;
    if (1 == sscanf(zonenum, "%i", &zone) &&
        (zone < 0 || zone > top_of_zone_table)) {
      send_to_char("That is not a valid zone_number\n\r", ch);
      destroy_string_block(&sb);
      return;
    }
    if (zone >= 0) {
      bottom = (zone != 0) ? (zone_table[zone - 1].top + 1) : 0;
      top = zone_table[zone].top;
    }

    append_to_string_block(&sb, "VNUM  rnum count names\n\r");
    for (objn = 0; objn <= topi; objn++) {
      oi = which_i + objn;

      if ((zone >= 0 && (oi->vnum < bottom || oi->vnum > top)) ||
          (zone < 0 && (isname(zonenum, oi->name) == 0))) {
        continue; /* optimize later*/
      }

      sprintf(buf, "%5d %4d %3d  %s\n\r", oi->vnum, objn, oi->number, oi->name);
      append_to_string_block(&sb, buf);
    }

  } else if (is_abbrev(buf, "rooms") != 0) {
    only_argument(argument, zonenum);

    append_to_string_block(&sb, "VNUM  rnum type         name [BITS]\n\r");
    if (is_abbrev(zonenum, "death") != 0) {
      room_iterate(room_db, print_death_room, &sb);

    } else if (is_abbrev(zonenum, "private") != 0) {
      room_iterate(room_db, print_private_room, &sb);

    } else if (1 != sscanf(zonenum, "%i", &zone) || zone < 0 ||
               zone > top_of_zone_table) {
      append_to_string_block(&sb, "I need a zone number with this command\n\r");

    } else {
      struct show_room_zone_struct srzs{};

      srzs.bottom = (zone != 0) ? (zone_table[zone - 1].top + 1) : 0;
      srzs.top = zone_table[zone].top;

      srzs.blank = 0;
      srzs.sb = &sb;
      room_iterate(room_db, show_room_zone, &srzs);

      if (srzs.blank != 0) {
        sprintf(buf, "rooms %d-%d are blank\n\r", srzs.startblank,
          srzs.lastblank);
        append_to_string_block(&sb, buf);
        srzs.blank = 0;
      }
    }
  } else {
    append_to_string_block(&sb,
      "Usage:\n\r  show zones\n\r  show (objects|mobiles) (zone#|name)\n\r  "
      "show rooms (zone#|death|private)\n\r");
  }
  page_string_block(&sb, ch);
  destroy_string_block(&sb);
}

static int scan_number(const char* text, int* rval) {
  if (1 != sscanf(text, " %i ", rval)) {
    return 0;
  }

  return 1;
}

void do_invis(struct char_data* ch, const char* argument, int /*cmd*/) {
  char buf[MAX_INPUT_LENGTH];
  int level = 0;

  if (GetMaxLevel(ch) < 51) {
    return;
  }

  if (scan_number(argument, &level) != 0) {
    if (level < 0) {
      level = 0;
    } else if (level > GetMaxLevel(ch)) {
      level = GetMaxLevel(ch);
    }
    ch->invis_level = level;
    sprintf(buf, "Invis level set to %d.\n\r", level);
    send_to_char(buf, ch);
  } else {
    if (ch->invis_level > 0) {
      ch->invis_level = 0;
      send_to_char("You are now totally visible.\n\r", ch);
    } else {
      ch->invis_level = LOW_IMMORTAL;
      send_to_char("You are now invisible to all but gods.\n\r", ch);
    }
  }
}

/* disallow any bogus characters in automated system requests.
   this is intented to prevent 'hacking' */
static int safe_to_be_in_system(char* cp) {
  return static_cast<int>(strpbrk(cp, "\"';`") == nullptr);
}

void do_loglist(struct char_data* ch, const char* /*arg*/, int /*cmd*/) {
  char buf[256];

  if (IS_NPC(ch)) {
    return;
  }

  if (safe_to_be_in_system(GET_NAME(ch)) == 0) {
    return;
  }

  sprintf(buf, "ls -C oldlogs/* > %s.checklog", GET_NAME(ch));
  system(buf); /* create the listing */

  sprintf(buf, "%s.checklog", GET_NAME(ch));
  start_page_file(ch->desc, buf, "No logs could be found.\n\r");
}

void do_checklog(struct char_data* ch, const char* arg, int /*cmd*/) {
  char string[256];
  char file[256];
  char buf[256];
  char* s = nullptr;
  int i = 0;

  if (IS_NPC(ch)) {
    return;
  }

  for (; ((*arg != 0) && (isspace(*arg) != 0)); arg++) {
    ; /* skip whitespace */
  }

  if (*arg != '\"') {
    send_to_char("Syntax:  checklog \"string\" datestring\n\r", ch);
    return;
  }

  for (arg++; (*arg != 0) && (*arg != '\"'); arg++) {
    string[i++] = *arg;
  }
  string[i] = '\0';

  if (*arg == 0) {
    send_to_char(
      "You need to terminate your search string with another quote mark.\n\r",
      ch);
    return;
  }

  if (sscanf(++arg, "%s", file) == EOF) {
    send_to_char("You need to specify a particular log file.\n\r", ch);
    return;
  }

  s = ((file[0] == 'l') && (file[1] == 'o') && (file[2] == 'g'))
        ? (char*)file + 3
        : file;
  if ((safe_to_be_in_system(string) == 0) || (safe_to_be_in_system(s) == 0) ||
      (safe_to_be_in_system(GET_NAME(ch)) == 0)) {
    send_to_char(
      "Apostrophes, double quote marks, ` <-- thingies, and semicolons are "
      "not "
      "allowed.\n\r",
      ch);
    return;
  }

  sprintf(buf, "egrep \"%s\" oldlogs/log%s > %s.checklog", string, s,
    GET_NAME(ch));
  system(buf);

  sprintf(buf, "%s.checklog", GET_NAME(ch));
  start_page_file(ch->desc, buf, "Your search-string was not found.\n\r");
}

void do_deathcheck(struct char_data* ch, const char* arg, int /*cmd*/) {
  char file[256];
  char player[256];
  char buf[256];
  char* p = nullptr;

  if (IS_NPC(ch)) {
    return;
  }

  if (sscanf(arg, "%s %s", player, file) == EOF) {
    send_to_char("Syntax:  deathcheck playername logfile", ch);
    return;
  }
  for (p = player; *p != 0; p++) {
    if (isalpha(*p) == 0) {
      send_to_char("Please use a valid playername.", ch);
      return;
    }
  }

  sprintf(buf, "\"%s killed by \" %s", player, file);
  do_checklog(ch, buf, 314);
}
