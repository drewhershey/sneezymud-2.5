/*###########################################################################*/
/*## IMPORTANT!!! ALWAYS KEEP THE "NEW" CHAR FILE_U STRUCTURE IN THIS FILE ##*/
/*## --------------------------------------------------------------------- ##*/
/*## THE NEW PLAYER FILE YOU GET FROM PIF'ING, CAN BE USED WHEN YOU INSTALL##*/
/*## THE NEW STRUCTURE IN YOU MUD. REMEMBER, ALWAYS TEST BEFORE INSTALLING ##*/
/*###########################################################################*/

#ifndef C_F_U_NEW_H
#define C_F_U_NEW_H

#include <time.h>

#include "structs.h"

struct char_file_u_new {
    signed char sex;
    unsigned char class;
    signed char level[12];
    time_t birth; /* Time of birth of character     */
    int played;   /* Number of secs played in total */

    int race;
    unsigned char weight;
    unsigned char height;

    char title[80];
    short int hometown;
    char description[240];
    char talks[MAX_TOUNGE];

    short int load_room; /* Which room to place char in  */

    struct char_ability_data abilities;

    struct char_point_data points;

    struct char_skill_data skills[MAX_SKILLS];

    struct affected_type affected[MAX_AFFECT];

    /* specials */

    signed char spells_to_learn;
    int alignment;

    time_t last_logon; /* Time (in secs) of last logon */
    long act;          /* ACT Flags                    */

    long affected_by;

    /* char data */
    char name[20];
    char pwd[11];
    short int apply_saving_throw[5];
    int conditions[3];
};

#endif
