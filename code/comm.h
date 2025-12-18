/* ************************************************************************
 *  file: comm.h , Communication module.                   Part of DIKUMUD *
 *  Usage: Prototypes for the common functions in comm.c                   *
 ************************************************************************* */

#ifndef COMM_H
#define COMM_H

#include <sys/time.h>

#include "structs.h"

#if SITELOCK
extern char hostlist[MAX_BAN_HOSTS][30];
extern int numberhosts;
#endif

extern int Shutdown;
extern int rebootmud;
extern long Uptime;
extern int slow_death;
extern int no_specials;
extern struct descriptor_data *descriptor_list, *next_to_process;
extern int tics;

void send_to_all(const char* messg);
void send_to_char(const char* messg, struct char_data* ch);
void send_to_except(const char* messg, struct char_data* ch);
void send_to_room(const char* messg, int room);
void send_to_room_except(const char* messg, int room, struct char_data* ch);
void send_to_room_except_two(const char* messg, int room, struct char_data* ch1,
  struct char_data* ch2);
void send_to_outdoor(const char* messg);
void perform_to_all(const char* messg, struct char_data* ch);
void perform_complex(struct char_data* ch1, struct char_data* ch2,
  struct obj_data* obj1, struct obj_data* obj2, const char* mess,
  byte mess_type, bool hide);
int get_from_q(struct txt_q* queue, char* dest);
/* write_to_q is in comm.h for the macro */
int run_the_game(int port);
int game_loop(int s);
int init_socket(int port);
int new_connection(int s);
int new_descriptor(int s);
int process_output(struct descriptor_data* t);
int process_input(struct descriptor_data* t);
void close_sockets(int s);
void close_socket(struct descriptor_data* d);
struct timeval timediff(struct timeval* a, struct timeval* b);
void flush_queues(struct descriptor_data* d);
void nonblock(int s);
void save_all(void);
struct char_data* make_char(char* name, struct descriptor_data* desc);
void boot_db(void);
void zone_update(void);
void affect_update(int pulse); /* In spell_parser.c */
void free_char(struct char_data* ch);
void string_add(struct descriptor_data* d, char* str);
void perform_violence(int pulse);
void stop_fighting(struct char_data* ch);
void show_string(struct descriptor_data* d, char* input);
void gr(int s);
void station(void);
void down_river(int pulse);
void Teleport(int pulse);
void MakeSound(int pulse);
void TeleportPulseStuff(int pulse);
void RiverPulseStuff(int pulse);
char* find_ex_description(char* word, struct extra_descr_data* list);
extern int load(void);
void coma(int s);
void check_reboot(void);
void signal_setup(void);
extern int workhours(void);

void act(const char* str, int hide_invisible, struct char_data* ch,
  struct obj_data* obj, void* vict_obj, int type);

#define TO_ROOM 0
#define TO_VICT 1
#define TO_NOTVICT 2
#define TO_CHAR 3

int write_to_descriptor(int desc, const char* txt);
void write_to_q(const char* txt, struct txt_q* queue);
#define SEND_TO_Q(messg, desc) write_to_q((messg), &(desc)->output)

#define ANSI_NORMAL "\033[0m"
#define ANSI_BLACK "\033[30m"
#define ANSI_RED "\033[31m"
#define ANSI_RED2 "\033[4m"
#define ANSI_GREEN "\033[32m"
#define ANSI_ORANGE "\033[33m"
#define ANSI_BLUE "\033[34m"
#define ANSI_VIOLET "\033[35m"
#define ANSI_CYAN "\033[36m"
#define ANSI_WHITE "\033[37m"

#define ANSI_BK_ON_BK "\033[8m"  /* black on black text */
#define ANSI_BK_ON_WH "\033[7m"  /* black on white text */
#define ANSI_WH_ON_BL "\033[44m" /* white on blue text */
#define ANSI_WH_ON_CY "\033[46m" /* white on cyan text */
#define ANSI_WH_ON_GR "\033[42m" /* white on green text */
#define ANSI_WH_ON_OR "\033[43m" /* white on orange text */
#define ANSI_WH_ON_PR "\033[45m" /* white on purple text */
#define ANSI_WH_ON_RD "\033[41m" /* white on red text */

#define VT_GRAPHBR "�"             /* test for bar graph */
#define VT_INITSEQ "\033[1;24r"    /* fixes up margins */
#define VT_CURSPOS "\033[%d;%dH"   /* respositions cursor */
#define VT_CURSRIG "\033[%dC"      /* moves cursor right */
#define VT_CURSLEF "\033[%dD"      /* moves cursor left */
#define VT_CLENSEQ "\033[r\033[2J" /* clears and resets screen */
#define VT_INVERTT "\033[0;1;7m"   /* inverted text */
#define VT_BOLDTEX "\033[0;1m"     /* bold text */
#define VT_NORMALT "\033[0m"       /* normal text */
#define VT_MARGSET "\033[%d;%dr"   /* sets margins */
#define VT_CURSAVE "\0337"         /* saves cursor position */
#define VT_CURREST "\0338"         /* restores cursor position */

#endif
