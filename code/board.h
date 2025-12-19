#include <stdio.h>

#include "structs.h"

struct char_data;
struct obj_data;

#define MAX_MSGS 50             /* Max number of messages.          */
#define MAX_MESSAGE_LENGTH 2048 /* that should be enough            */

struct Board {
    char* msgs[MAX_MSGS];
    char* head[MAX_MSGS];
    int msg_num;
    char filename[40];
    FILE* file; /* file that is opened */
    int Rnum;   /* Real # of object that this board hooks to */
    struct Board* next;
};

extern struct char_data* board_kludge_char;
extern struct Board* board_list;

struct Board* FindBoardInRoom(int room);
void board_save_board(struct Board* b);
int board_show_board(struct char_data* ch, char* arg, struct Board* b);
void board_fix_long_desc(struct Board* b);
int board_display_msg(struct char_data* ch, char* arg, struct Board* b);
void board_reset_board(struct Board* b);
void board_load_board(struct Board* b);
int board_remove_msg(struct char_data* ch, char* arg, struct Board* b);
void board_write_msg(struct char_data* ch, char* arg, struct Board* b);
void OpenBoardFile(struct Board* b);
void InitABoard(struct obj_data* obj);
void InitBoards(void);
