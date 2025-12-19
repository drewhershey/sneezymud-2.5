/* ************************************************************************
 *  file: Interpreter.h , Command interpreter module.      Part of DIKUMUD *
 *  Usage: Procedures interpreting user command                            *
 ************************************************************************* */

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdio.h>

#include "structs.h"

#define MAX_CMD_LIST 400

extern const char* const path[];
extern const char* const command[];
extern int WizLock;
extern int Silence;

struct command_info {
    void (*command_pointer)(struct char_data* ch, char* argument, int cmd);
    signed char minimum_position;
    signed char minimum_level;
};

extern struct command_info cmd_info[MAX_CMD_LIST];

int ReadObjs(FILE* fl, struct obj_file_u* st);
void ZeroRent(char* n);
void command_interpreter(struct char_data* ch, char* argument);
int search_block(char* arg, const char* const* list, char exact);
int old_search_block(char* argument, int begin, int length,
  const char* const* list, int mode);
void argument_interpreter(char* argument, char* first_arg, char* second_arg);
char* one_argument(char* argument, char* first_arg);
void only_argument(char* argument, char* dest);
int fill_word(char* argument);
void half_chop(char* string, char* arg1, char* arg2);
void nanny(struct descriptor_data* d, char* arg);
int is_abbrev(const char* arg1, const char* arg2);
char* crypt(const char*, const char*);
int special(struct char_data* ch, int cmd, char* arg);
int is_number(char* str);
int parse_name(char* arg, char* name);
int find_name(char* name);
void set_title(struct char_data* ch);
void init_char(struct char_data* ch);
void store_to_char(struct char_file_u* st, struct char_data* ch);
int create_entry(char* name);

void do_move(struct char_data* ch, char* argument, int cmd);
void do_look(struct char_data* ch, char* argument, int cmd);
void do_read(struct char_data* ch, char* argument, int cmd);
void do_say(struct char_data* ch, char* argument, int cmd);
void do_exit(struct char_data* ch, char* argument, int cmd);
void do_snoop(struct char_data* ch, char* argument, int cmd);
void do_insult(struct char_data* ch, char* argument, int cmd);
void do_quit(struct char_data* ch, char* argument, int cmd);
void do_qui(struct char_data* ch, char* argument, int cmd);
void do_help(struct char_data* ch, char* argument, int cmd);
void do_who(struct char_data* ch, char* argument, int cmd);
void do_whozone(struct char_data* ch, char* argument, int cmd);
void do_emote(struct char_data* ch, char* argument, int cmd);
void do_echo(struct char_data* ch, char* argument, int cmd);
void do_trans(struct char_data* ch, char* argument, int cmd);
void do_kill(struct char_data* ch, char* argument, int cmd);
void do_stand(struct char_data* ch, char* argument, int cmd);
void do_sit(struct char_data* ch, char* argument, int cmd);
void do_rest(struct char_data* ch, char* argument, int cmd);
void do_sleep(struct char_data* ch, char* argument, int cmd);
void do_wake(struct char_data* ch, char* argument, int cmd);
void do_force(struct char_data* ch, char* argument, int cmd);
void do_get(struct char_data* ch, char* argument, int cmd);
void do_drop(struct char_data* ch, char* argument, int cmd);
void do_news(struct char_data* ch, char* argument, int cmd);
void do_wiznews(struct char_data* ch, char* argument, int cmd); /* SG */
void do_atlas(struct char_data* ch, char* argument, int cmd);
void do_monitor(struct char_data* ch, char* argument, int cmd);
void do_score(struct char_data* ch, char* argument, int cmd);
void do_loglist(struct char_data* ch, char* argument, int cmd);
void do_checklog(struct char_data* ch, char* argument, int cmd);
void do_deathcheck(struct char_data* ch, char* argument, int cmd);
void do_inventory(struct char_data* ch, char* argument, int cmd);
void do_equipment(struct char_data* ch, char* argument, int cmd);
void do_shout(struct char_data* ch, char* argument, int cmd);
void do_not_here(struct char_data* ch, char* argument, int cmd);
void do_tell(struct char_data* ch, char* argument, int cmd);
void do_wear(struct char_data* ch, char* argument, int cmd);
void do_wield(struct char_data* ch, char* argument, int cmd);
void do_grab(struct char_data* ch, char* argument, int cmd);
void do_remove(struct char_data* ch, char* argument, int cmd);
void do_put(struct char_data* ch, char* argument, int cmd);
void do_shutdown(struct char_data* ch, char* argument, int cmd);
void do_save(struct char_data* ch, char* argument, int cmd);
void do_hit(struct char_data* ch, char* argument, int cmd);
void do_string(struct char_data* ch, char* arg, int cmd);
void do_give(struct char_data* ch, char* arg, int cmd);
void do_stat(struct char_data* ch, char* arg, int cmd);
void do_guard(struct char_data* ch, char* arg, int cmd);
void do_time(struct char_data* ch, char* arg, int cmd);
void do_weather(struct char_data* ch, char* arg, int cmd);
void do_load(struct char_data* ch, char* arg, int cmd);
void do_purge(struct char_data* ch, char* arg, int cmd);
void do_shutdow(struct char_data* ch, char* arg, int cmd);
void do_idea(struct char_data* ch, char* arg, int cmd);
void do_typo(struct char_data* ch, char* arg, int cmd);
void do_bug(struct char_data* ch, char* arg, int cmd);
void do_whisper(struct char_data* ch, char* arg, int cmd);
void do_cast(struct char_data* ch, char* arg, int cmd);
void do_at(struct char_data* ch, char* arg, int cmd);
void do_goto(struct char_data* ch, char* arg, int cmd);
void do_ask(struct char_data* ch, char* arg, int cmd);
void do_drink(struct char_data* ch, char* arg, int cmd);
void do_eat(struct char_data* ch, char* arg, int cmd);
void do_pour(struct char_data* ch, char* arg, int cmd);
void do_sip(struct char_data* ch, char* arg, int cmd);
void do_taste(struct char_data* ch, char* arg, int cmd);
void do_order(struct char_data* ch, char* arg, int cmd);
void do_follow(struct char_data* ch, char* arg, int cmd);
void do_rent(struct char_data* ch, char* arg, int cmd);
void do_bload(struct char_data* ch, char* arg, int cmd);
void do_advance(struct char_data* ch, char* arg, int cmd);
void do_close(struct char_data* ch, char* arg, int cmd);
void do_open(struct char_data* ch, char* arg, int cmd);
void do_lock(struct char_data* ch, char* arg, int cmd);
void do_unlock(struct char_data* ch, char* arg, int cmd);
void do_exits(struct char_data* ch, char* arg, int cmd);
void do_enter(struct char_data* ch, char* arg, int cmd);
void do_leave(struct char_data* ch, char* arg, int cmd);
void do_write(struct char_data* ch, char* arg, int cmd);
void do_flee(struct char_data* ch, char* arg, int cmd);
void do_sneak(struct char_data* ch, char* arg, int cmd);
void do_hide(struct char_data* ch, char* arg, int cmd);
void do_backstab(struct char_data* ch, char* arg, int cmd);
void do_pick(struct char_data* ch, char* arg, int cmd);
void do_steal(struct char_data* ch, char* arg, int cmd);
void do_bash(struct char_data* ch, char* arg, int cmd);
void do_rescue(struct char_data* ch, char* arg, int cmd);
void do_kick(struct char_data* ch, char* arg, int cmd);
void do_examine(struct char_data* ch, char* arg, int cmd);
void do_info(struct char_data* ch, char* arg, int cmd);
void do_users(struct char_data* ch, char* arg, int cmd);
void do_where(struct char_data* ch, char* arg, int cmd);
void do_levels(struct char_data* ch, char* arg, int cmd);
void do_reroll(struct char_data* ch, char* arg, int cmd);
void do_pray(struct char_data* ch, char* arg, int cmd);
void do_brief(struct char_data* ch, char* arg, int cmd);
void do_cls(struct char_data* ch, char* arg, int cmd);
void do_bamfin(struct char_data* ch, char* arg, int cmd);
void do_bamfout(struct char_data* ch, char* arg, int cmd);
void do_terminal(struct char_data* ch, char* arg, int cmd);
void do_prompt(struct char_data* ch, char* arg, int cmd);
void do_glance(struct char_data* ch, char* arg, int cmd);
void do_wizlist(struct char_data* ch, char* arg, int cmd);
void do_consider(struct char_data* ch, char* arg, int cmd);
void do_group(struct char_data* ch, char* arg, int cmd);
void do_restore(struct char_data* ch, char* arg, int cmd);
void do_return(struct char_data* ch, char* argument, int cmd);
void do_switch(struct char_data* ch, char* argument, int cmd);
void do_quaff(struct char_data* ch, char* argument, int cmd);
void do_recite(struct char_data* ch, char* argument, int cmd);
void do_use(struct char_data* ch, char* argument, int cmd);
void do_pose(struct char_data* ch, char* argument, int cmd);
void do_noshout(struct char_data* ch, char* argument, int cmd);
void do_plr_noshout(struct char_data* ch, char* argument, int cmd);
void do_wizhelp(struct char_data* ch, char* argument, int cmd);
void do_credits(struct char_data* ch, char* argument, int cmd);
void do_compact(struct char_data* ch, char* argument, int cmd);
void do_wimpy(struct char_data* ch, char* argument, int cmd);    /* jdb -8-16 */
void do_commune(struct char_data* ch, char* argument, int cmd);  /* jdb - 9-1 */
void do_nohassle(struct char_data* ch, char* argument, int cmd); /* jdb 9-6 */
void do_system(struct char_data* ch, char* argument, int cmd);   /* jdb 9-16 */
void do_pull(struct char_data* ch, char* argument, int cmd);     /* jdb 9-16 */
void do_stealth(struct char_data* ch, char* argument, int cmd);  /* jdb 9-17 */
void do_edit(struct char_data* ch, char* arg, int cmd);          /* jdb 9-29 */
void do_set(struct char_data* ch, char* arg, int cmd);           /* jdb 9-29 */
void do_rsave(struct char_data* ch, char* arg, int cmd);         /* jdb 10-5 */
void do_rload(struct char_data* ch, char* arg, int cmd);         /* jdb 10-5 */
void do_wizlock(struct char_data* ch, char* arg, int cmd);       /* jdb 10-15 */
void do_highfive(struct char_data* ch, char* arg, int cmd);      /* jdb 10-30 */
void do_title(struct char_data* ch, char* arg, int cmd);         /* jdb 11-3 */
void do_uptime(struct char_data* ch, char* arg, int cmd);        /* jdb 12-3 */
void do_instazone(struct char_data* ch, char* arg, int cmd);     /* jdb 12-3 */
void do_disarm(struct char_data* ch, char* arg, int cmd);        /* jdb 12-3 */
void do_junk(struct char_data* ch, char* arg, int cmd);          /* jdb 12-17 */
void do_gain(struct char_data* ch, char* arg, int cmd);          /* jdb 1-19 */
void do_passwd(struct char_data* ch, char* arg, int cmd);        /* jdb 2-6 */
void do_fill(struct char_data* ch, char* arg, int cmd);          /* jdb 2-9 */
void do_imptest(struct char_data* ch, char* arg, int cmd);       /* jdb 2-13 */
void do_silence(struct char_data* ch, char* arg, int cmd);       /* smg 4-26 */
void do_teams(struct char_data* ch, char* arg, int cmd);         /* smg 5-26 */
void do_auth(struct char_data* ch, char* arg, int cmd);          /* jdb 3-1 */
void do_shoot(struct char_data* ch, char* arg, int cmd);         /* jdb 3-8 */
void do_swim(struct char_data* ch, char* arg, int cmd);          /* jdb 8-4 */
void do_reload(struct char_data* ch, char* arg, int cmd);        /* jhh 7-24 */
void do_oset(struct char_data* ch, char* arg, int cmd);          /* jfr2 10-15*/
void do_bet(struct char_data* ch, char* arg, int cmd);           /* jhh 8-22 */
void do_stay(struct char_data* ch, char* arg, int cmd);          /* jhh 8-22 */
void do_peek(struct char_data* ch, char* arg, int cmd);          /* jhh 8-22 */
void do_color(struct char_data* ch, char* arg, int cmd);         /*jfr 8-28 */
void do_search(struct char_data* ch, char* arg, int cmd); /* jfr2 1-16-93*/
void do_send(struct char_data* ch, char* arg, int cmd);   /* jfr2 12-30 */
void do_spy(struct char_data* ch, char* arg, int cmd);    /*jfr2 1-23-93*/
void do_sign(struct char_data* ch, char* arg, int cmd);
void do_play(struct char_data* ch, char* arg, int cmd); /*jfr2 2-11-93 */
void do_flag(struct char_data* ch, char* arg, int cmd); /*jfr2 1-30-93 */
void do_link(struct char_data* ch, char* arg, int cmd); /* jfr2 1-24-93 */
void do_doorbash(struct char_data* ch, char* arg, int cmd);
void do_springleap(struct char_data* ch, char* arg, int cmd);
void do_lay_hands(struct char_data* ch, char* arg, int cmd);
void do_quivering_palm(struct char_data* ch, char* arg, int cmd);
void do_feign_death(struct char_data* ch, char* arg, int cmd);
void do_first_aid(struct char_data* ch, char* arg, int cmd);
void do_channel(struct char_data* ch, char* arg, int cmd);    /*jfr2 12-30 */
void do_headbutt(struct char_data* ch, char* arg, int cmd);   /*jfr2 9-21 */
void do_log(struct char_data* ch, char* arg, int cmd);        /*jfr2 12/29/92 */
void do_subterfuge(struct char_data* ch, char* arg, int cmd); /* jfr2 10-3 */
void do_throw(struct char_data* ch, char* arg, int cmd);      /* jfr2 10-4 */
void do_scribe(struct char_data* ch, char* arg, int cmd);     /* jfr2 10-4 */
void do_brew(struct char_data* ch, char* arg, int cmd);       /* jfr2 10-15*/
void do_grapple(struct char_data* ch, char* arg, int cmd);    /*jfr2 10-16*/

/*
  depth first seach procedure donated by WhiteGold
  */

void do_track(struct char_data* ch, char* arg, int cmd); /* jdb 10-9 */

/*
  These 3 were donated by sequent
  */

void do_attribute(struct char_data* ch, char* arg, int cmd); /* jdb 11-6 */
void do_world(struct char_data* ch, char* arg, int cmd);     /* jdb 11-6 */
void do_spells(struct char_data* ch, char* arg, int cmd);    /* jdb 11-6 */

void do_action(struct char_data* ch, char* arg, int cmd);
void do_practice(struct char_data* ch, char* arg, int cmd);

/* Hammor commands */
void do_assist(struct char_data* ch, char* arg, int cmd);
void do_fire(struct char_data* ch, char* arg, int cmd);
void do_show(struct char_data* ch, char* arg, int cmd);
void do_bodyslam(struct char_data* ch, char* arg, int cmd);
void do_invis(struct char_data* ch, char* arg, int cmd);
void do_grouptell(struct char_data* ch, char* arg, int cmd);

/* Brutius commands */

void do_report(struct char_data* ch, char* arg, int cmd);
void do_demote(struct char_data* ch, char* arg, int cmd);
void do_split(struct char_data* ch, char* arg, int cmd);
void do_command(struct char_data* ch, char* arg, int cmd);
void do_deathstroke(struct char_data* ch, char* arg, int cmd);

#define MENU \
  "\n\rWelcome to SneezyMUD\n\r\n\
0) Leave SneezyMUD.\n\r\
1) Enter the game at Midgaard\n\r\
2) Enter description.\n\r\
3) Read the background story\n\r\
4) Change password.\n\r\
5) Enter somewhere else\n\r\
6) Delete this character\n\r\n\r\
   Make your choice: "

#define CLASSHELP \
  "\n\rClasses: (Thief,Cleric,Mage,Warrior,Antipaladin,Paladin,Ranger,Monk)\n\r\
Advantages of being single classed (one class only):\n\r\n\r\
1) Skills for Single Class people can be more specialized.\n\r\
2) Some very powerful spells are available that aren't to others.\n\r\
3) Single classed people gain experience faster than multi-classes\n\r\
Diadvantages of being single Classes:\n\r\n\r\
1) Lack of the ability to go alone(this isnt really a bad thing)\n\r\
2) Fewer hit points than multi-classed people.\n\r\
3) The lack of ability to wear certain items.\n\r\
Advantages for multi-classed people:\n\r\n\r\
1) More hit points than single classed people.\n\r\
2) Ability to go alone(This can be bad at times)\n\r\
3) Ability to wear more items.\n\r\
Disadvantages for multi-classing:\n\r\n\r\
1) Takes longer to gain experience.\n\r\
2) Cant be totally specialized in most skills.\n\r"

#define RACEHELP \
  "\n\rRaces:  (Dwarven, Elven, Human, Hobbit, Ogre, Gnome)\n\r\
Dwarves:  Shorter. Less Movement. More sturdy. Less wise. Infravision\n\r\
Elves:    Taller.  More Movement. Less sturdy. More dextrous.\n\r\
Humans:   Average...  \n\r\
Hobbits:  Shorter. Least movement of all races. More dextrous. Weaker.\n\r\
Ogres:    Largest of all races.Most movement of all races. Stronger. Dumber\n\r\
Gnomes:   Shorter. Less movement. More intelligent. Less wise.\n\r"

#define OLDONE \
  "\n\r           Tom Madsen, Michael Seifert, and Sebastian Hammer\n\r\
                  Hans Henrik Staerfeldt, Katja Nyboe,\n\r\
                              Created by\n\r\n\r\
                           DikuMUD I (GAMMA 0.0)\n\r\n\r"

#define GREETINGS \
  "\n\r                                 SneezyMUD 2.5\n\r\n\r\
                       Original DikuMUD concept created by:\n\r\
                Tom Madsen, Michael Seifert, and Sebastian Hammer\n\r\
                       Hans Henrik Staerfeldt, Katja Nyboe,\n\r\
             Original SneezyMUD code source provided by J. Brothers.\n\r\n\r\
                        Coding done by Brutius, Stargazer\n\r\
                             Lord of Worlds : Batopr\n\r\n\r"

#define WELC_MESSG \
  "\n\rWelcome to the land of SneezyMUD. May your visit here be... interesting.\
\n\r\n\r"

#define STORY \
  " SneezyMUD is a creation of the Public broadcasting System. \n \
  Brought to you today by the letters X and B, and the number 69\n\n\r"

#endif
