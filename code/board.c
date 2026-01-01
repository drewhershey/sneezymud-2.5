#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "accessors.h"
#include "board.h"
#include "comm.h"
#include "db.h"
#include "game_constants.h"
#include "handler.h"
#include "interpreter.h"
#include "multiclass.h"
#include "spec_procs.h"
#include "structs.h"
#include "utils.h"

struct char_data* board_kludge_char;
struct Board* board_list;

void InitBoards(void) {
  /*
   **  this is called at the very beginning, like shopkeepers
   */
  board_list = nullptr;
}

void InitABoard(struct obj_data* obj) {
  struct Board* new_board = nullptr;
  struct Board* tmp = nullptr;
  int i = 0;

  if (board_list != nullptr) {
    /*
     **  try to match a board with an existing board in the game
     */
    for (tmp = board_list; tmp != nullptr; tmp = tmp->next) {
      if (tmp->Rnum == obj->item_number) {
        /*
         **  board has been matched, load and ignore it.
         */
        board_load_board(tmp);
        return;
      }
    }
  }

  new_board = (struct Board*)malloc(sizeof(*new_board));
  if (new_board == nullptr) {
    perror("InitABoard(malloc)");
    exit(0);
  }

  for (i = 0; i < MAX_MSGS; ++i) {
    new_board->head[i] = nullptr;
  }
  for (i = 0; i < MAX_MSGS; ++i) {
    new_board->msgs[i] = nullptr;
  }

  new_board->msg_num = 0;
  new_board->Rnum = obj->item_number;

  sprintf(new_board->filename, "%d.messages", obj_index[obj->item_number].vnum);

  board_load_board(new_board);

  /*
   **  add our new_board board to the beginning of the list
   */

  tmp = board_list;
  new_board->next = tmp;
  board_list = new_board;
}

void OpenBoardFile(struct Board* b) {
  b->file = fopen(b->filename, "r+");

  if (b->file == nullptr) {
    perror("OpenBoardFile(fopen)");
    exit(0);
  }
}

struct Board* FindBoardInRoom(int room) {
  struct obj_data* o = nullptr;
  struct Board* nb = nullptr;

  if (real_roomp(room) == nullptr) {
    return (nullptr);
  }

  for (o = real_roomp(room)->contents; o != nullptr; o = o->next_content) {
    if (obj_index[o->item_number].func.obj_f == board) {
      for (nb = board_list; nb != nullptr; nb = nb->next) {
        if (nb->Rnum == o->item_number) {
          return (nb);
        }
      }
      return (nullptr);
    }
  }
  return (nullptr);
}

int board(struct char_data* ch, int cmd, const char* arg, struct obj_data* me) {
  struct Board* nb = nullptr;

  if (ch == nullptr) {
    return 0;
  }

  nb = FindBoardInRoom(ch->in_room);

  if (nb == nullptr) {
    return 0;
  }

  if (ch->desc == nullptr) {
    return 0;
  }

  switch (cmd) {
    case 15: /* look */
      return (board_show_board(ch, arg, nb));
    case 149: /* write */
      board_write_msg(ch, arg, nb);
      return 1;
    case 63: /* read */
      return (board_display_msg(ch, arg, nb));
    case 66: /* remove */
      return (board_remove_msg(ch, arg, nb));
    default:
      return 0;
  }
}

void board_write_msg(struct char_data* ch, const char* arg, struct Board* b) {
  static char buf[100];
  long ot = 0;
  char* otmstr = nullptr;

  if (b->msg_num > MAX_MSGS - 1) {
    send_to_char("The board is full already.\n\r", ch);
    return;
  }

  if (board_kludge_char != nullptr) {
    send_to_char(
      "Sorry, but someone has stolen the pen.. wait a few minutes.\n\r", ch);
    return;
  }

  /* skip blanks */

  for (; isspace(*arg) != 0; arg++) {
    ;
  }

  if (*arg == 0) {
    send_to_char("We must have a headline!\n\r", ch);
    return;
  }

  board_kludge_char = ch;

  ot = time(nullptr);
  otmstr = asctime(localtime(&ot));
  *(otmstr + strlen(otmstr) - 1) = '\0';
  sprintf(buf, "%s", otmstr);
  b->head[b->msg_num] =
    (char*)malloc(strlen(buf) + strlen(arg) + strlen(GET_NAME(ch)) + 8);

  /* +8 is for a space and '()' around the character name. */

  if (b->head[b->msg_num] == nullptr) {
    error_log("Malloc for board header failed.\n\r");
    send_to_char("The board is malfunctioning - sorry.\n\r", ch);
    return;
  }

  sprintf(b->head[b->msg_num], "[%s] %s (%s)", buf, arg, GET_NAME(ch));
  b->msgs[b->msg_num] = nullptr;

  send_to_char("Write your message. Terminate with an @.\n\r\n\r", ch);
  act("$n starts to write a message.", 1, ch, nullptr, nullptr, TO_ROOM);

  ch->desc->str = &b->msgs[b->msg_num];
  ch->desc->max_str = MAX_MESSAGE_LENGTH;

  b->msg_num++;
}

int board_remove_msg(struct char_data* ch, const char* arg, struct Board* b) {
  int ind = 0;
  int msg = 0;
  char buf[256];
  char number[MAX_INPUT_LENGTH];

  one_argument(arg, number);

  if ((*number == 0) || (isdigit(*number) == 0)) {
    return (0);
  }
  if ((msg = atoi(number)) == 0) {
    return (0);
  }
  if (b->msg_num == 0) {
    send_to_char("The board is empty!\n\r", ch);
    return (1);
  }
  if (msg < 1 || msg > b->msg_num) {
    send_to_char("That message exists only in your imagination..\n\r", ch);
    return (1);
  }

  if (GetMaxLevel(ch) < 51) {
    send_to_char("Due to misuse of the REMOVE command, only 51st level\n\r",
      ch);
    send_to_char("and above can remove messages.\n\r", ch);
    return 1;
  }

  ind = msg;
  free(b->head[--ind]);
  if ((b->msgs[ind] != nullptr) && (*b->msgs[ind] != 0)) {
    free(b->msgs[ind]);
  }
  for (; ind < b->msg_num - 1; ind++) {
    b->head[ind] = b->head[ind + 1];
    b->msgs[ind] = b->msgs[ind + 1];
  }
  b->msg_num--;
  send_to_char("Message removed.\n\r", ch);
  sprintf(buf, "$n just removed message %d.", msg);
  act(buf, 0, ch, nullptr, nullptr, TO_ROOM);
  board_save_board(b);

  return (1);
}

void board_save_board(struct Board* b) {
  int ind = 0;
  int len = 0;

  if (b == nullptr) {
    return;
  }

  if (b->msg_num == 0) {
    error_log("No messages to save.\n\r");
    return;
  }

  OpenBoardFile(b);

  fwrite(&b->msg_num, sizeof(int), 1, b->file);
  for (ind = 0; ind < b->msg_num; ind++) {
    len = strlen(b->head[ind]) + 1;
    fwrite(&len, sizeof(int), 1, b->file);
    fwrite(b->head[ind], sizeof(char), len, b->file);
    if (b->msgs[ind] == nullptr) {
      if ((b->msgs[ind] = (char*)Mymalloc(50)) != nullptr) {
        strcpy(b->msgs[ind], "Generic Message");
      } else {
        exit(1);
      }
    }
    len = strlen(b->msgs[ind]) + 1;
    fwrite(&len, sizeof(int), 1, b->file);
    fwrite(b->msgs[ind], sizeof(char), len, b->file);
  }
  fclose(b->file);
  board_fix_long_desc(b);
}

void board_load_board(struct Board* b) {
  int ind = 0;
  int len = 0;

  OpenBoardFile(b);
  board_reset_board(b);

  fread(&b->msg_num, sizeof(int), 1, b->file);

  if (b->msg_num < 1 || b->msg_num > MAX_MSGS || (feof(b->file) != 0)) {
    error_log("Board-message file corrupt or nonexistent.\n\r");
    fclose(b->file);
    return;
  }
  for (ind = 0; ind < b->msg_num; ind++) {
    fread(&len, sizeof(int), 1, b->file);
    b->head[ind] = (char*)Mymalloc(len + 1);
    if (b->head[ind] == nullptr) {
      error_log("Malloc for board header failed.\n\r");
      board_reset_board(b);
      fclose(b->file);
      return;
    }
    fread(b->head[ind], sizeof(char), len, b->file);
    fread(&len, sizeof(int), 1, b->file);
    b->msgs[ind] = (char*)Mymalloc(len + 1);
    if (b->msgs[ind] == nullptr) {
      error_log("Malloc for board msg failed..\n\r");
      board_reset_board(b);
      fclose(b->file);
      return;
    }
    fread(b->msgs[ind], sizeof(char), len, b->file);
  }
  fclose(b->file);
  board_fix_long_desc(b);
}

void board_reset_board(struct Board* b) {
  int ind = 0;

  for (ind = 0; ind < MAX_MSGS; ind++) {
    if (b->head[ind] != nullptr) {
      free(b->head[ind]);
    }
    if (b->msgs[ind] != nullptr) {
      free(b->msgs[ind]);
    }
    b->head[ind] = b->msgs[ind] = nullptr;
  }
  b->msg_num = 0;
  board_fix_long_desc(b);
}

void error_log(const char* str) { /* The original error-handling was MUCH */
  fputs("Board : ", stderr);      /* more competent than the current but  */
  fputs(str, stderr);
}

int board_display_msg(struct char_data* ch, const char* arg, struct Board* b) {
  char buf[512];
  char number[MAX_INPUT_LENGTH];
  char buffer[MAX_STRING_LENGTH];
  int msg = 0;

  one_argument(arg, number);
  if ((*number == 0) || (isdigit(*number) == 0)) {
    return (0);
  }
  if ((msg = atoi(number)) == 0) {
    return (0);
  }
  if (b->msg_num == 0) {
    send_to_char("The board is empty!\n\r", ch);
    return (1);
  }
  if (msg < 1 || msg > b->msg_num) {
    send_to_char("That message exists only in your imagination..\n\r", ch);
    return (1);
  }

  sprintf(buf, "$n reads message %d titled : %s.", msg, b->head[msg - 1]);
  act(buf, 1, ch, nullptr, nullptr, TO_ROOM);

  /* Bad news */

  sprintf(buffer, "Message %d : %s\n\r\n\r%s", msg, b->head[msg - 1],
    b->msgs[msg - 1]);
  page_string(ch->desc, buffer, 1);
  return (1);
}

void board_fix_long_desc(struct Board* b) {}

int board_show_board(struct char_data* ch, const char* arg, struct Board* b) {
  int i = 0;
  char buf[MAX_STRING_LENGTH];
  char tmp[MAX_INPUT_LENGTH];

  one_argument(arg, tmp);

  if ((*tmp == 0) || (isname(tmp, "board bulletin") == 0)) {
    return (0);
  }

  if (board_kludge_char != nullptr) {
    send_to_char("Sorry, but someone is writing a message\n\r", ch);
    return (0);
  }

  act("$n studies the board.", 1, ch, nullptr, nullptr, TO_ROOM);

  strcpy(buf,
    "This is a bulletin board. Usage: READ/REMOVE <messg #>, WRITE "
    "<header>\n\r");
  if (b->msg_num == 0) {
    strcat(buf, "The board is empty.\n\r");
  } else if (b->msg_num == 1) {
    sprintf(buf + strlen(buf), "There is 1 message on the board.\n\r");
  } else {
    sprintf(buf + strlen(buf), "There are %d messages on the board.\n\r",
      b->msg_num);
    for (i = 0; i < b->msg_num; i++) {
      sprintf(buf + strlen(buf), "%-2d : %s\n\r", i + 1, b->head[i]);
    }
  }
  page_string(ch->desc, buf, 1);

  return (1);
}
