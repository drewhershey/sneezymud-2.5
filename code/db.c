/**************************************************************************
*  file: db.c , Database module.                          Part of DIKUMUD *
*  Usage: Loading/Saving chars, booting world, resetting etc.             *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "limits.h"
#include "race.h"
#include "mail.h"
#include "opinion.h"
#include "hash.h"

#define NEW_ZONE_SYSTEM

/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */

int top_of_world = 0;                 /* ref to the top element of world */
#if HASH
struct hash_header	room_db;
#else
struct room_data        *room_db[WORLD_SIZE];
#endif

struct obj_data  *object_list = 0;    /* the global linked list of obj's */
struct char_data *character_list = 0; /* global l-list of chars          */

struct zone_data *zone_table;         /* table of reset data             */
int top_of_zone_table = 0;
struct message_list fight_messages[MAX_MESSAGES]; /* fighting messages   */
struct player_index_element *player_table = 0; /* index to player file   */
int top_of_p_table = 0;               /* ref to top of table             */
int top_of_p_file = 0;
long total_bc = 0;
long room_count=0;
long mob_count=0;
int obj_count=0;
long total_mbc=0;
int no_mail=0;
long total_obc=0;

/*
**  distributed monster stuff
*/
int mob_tick_count=0;

char motd[MAX_STRING_LENGTH];         /* the messages of today           */
char ansi[MAX_STRING_LENGTH];

FILE *mob_f,                          /* file containing mob prototypes  */
     *obj_f;                          /* obj prototypes                  */

struct index_data *mob_index;         /* index table for mobile file     */
struct index_data *obj_index;         /* index table for object file     */

int top_of_mobt = 0;                  /* top of mobile index table       */
int top_of_objt = 0;                  /* top of object index table       */

struct time_info_data time_info;	/* the infomation about the time   */
struct weather_data weather_info;	/* the infomation about the weather */




/* local procedures */
void boot_zones(void);
void setup_dir(FILE *fl, int room, int dir);
void allocate_room(int new_top);
void boot_world(void);
struct index_data *generate_indices(FILE *fl, int *top);
void build_player_index(void);
void char_to_store(struct char_data *ch, struct char_file_u *st);
void store_to_char(struct char_file_u *st, struct char_data *ch);
int is_empty(int zone_nr);
void reset_zone(int zone);
int file_to_string(char *name, char *buf);
void renum_zone_table(void);
void reset_time(void);
void clear_char(struct char_data *ch);
struct obj_data *unequip_char_for_save(struct char_data *ch, int pos);


/* external refs */
extern struct descriptor_data *descriptor_list;
void load_messages(void);
void weather_and_time ( int mode );
void assign_command_pointers ( void );
void assign_spell_pointers ( void );
void vlog(char *str);
int dice(int number, int size);
int number(int from, int to);
void boot_social_messages(void);
void boot_pose_messages(void);
void change_char_file(void); /* In reception.c */
void update_obj_file(void);
int DetermineExp( struct char_data *mob, int exp_flags);
void SetRacialStuff( struct char_data *mob);
int ClassSpecificStuff( struct char_data *ch);

/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */


/* body of the booting system */
void boot_db(void)
{
	int i;
	extern int no_specials;

	vlog("Boot db -- BEGIN.");

	vlog("Resetting the game time:");
	reset_time();

	vlog("Reading atlas, and motd.");
        file_to_string(ANSI_FILE, ansi);
	file_to_string(MOTD_FILE, motd);

	vlog("Opening mobile, object and help files.");
	if (!(mob_f = fopen(MOB_FILE, "r")))	{
		perror("boot");
		exit(0);
	}

	if (!(obj_f = fopen(OBJ_FILE, "r")))	{
		perror("boot");
		exit(0);
	}

	vlog("Loading zone table.");
	boot_zones();

	vlog("Loading rooms.");
	boot_world();

	vlog("Generating index tables for mobile and object files.");
	mob_index = generate_indices(mob_f, &top_of_mobt);
	obj_index = generate_indices(obj_f, &top_of_objt);
			
	vlog("Renumbering zone table.");
	renum_zone_table();

	vlog("Generating player index.");
	build_player_index();

	vlog("Loading fight messages.");
	load_messages();

	vlog("Loading social messages.");
	boot_social_messages();

  vlog("Loading pose messages.");
	boot_pose_messages();

	vlog("Assigning function pointers:");
	if (!no_specials)	{
		vlog("   Mobiles.");
		assign_mobiles();
		vlog("   Objects.");
		assign_objects();
		vlog("   Room.");
		assign_rooms();
	}

	vlog("   Commands.");	
	assign_command_pointers();
	vlog("   Spells.");
	assign_spell_pointers();

	vlog("Updating characters with saved items:");
	update_obj_file();
 
        vlog(" Booting mail system.");
        if (!scan_file())
        {
               vlog("   Mail system error -- mail system disabled!");
               no_mail = 1;
        }


#if LIMITED_ITEMS
        PrintLimitedItems();
#endif

	for (i = 0; i <= top_of_zone_table; i++)	{
	  char	*s;
	  int	d,e;
	  s = zone_table[i].name;
	  d = (i ? (zone_table[i - 1].top + 1) : 0);
	  e = zone_table[i].top;
	   fprintf(stderr, "Performing boot-time reset of %s (rooms %d-%d).\n",
#if 1
		   s, d, e);
#else
	       	zone_table[i].name,
	       	(i ? (zone_table[i - 1].top + 1) : 0),
	       	zone_table[i].top);
#endif
		reset_zone(i);
	}

	reset_q.head = reset_q.tail = 0;

	vlog("Boot db -- DONE.");
}


/* reset the time in the game from file */
void reset_time(void)
{
	char buf[80];
        extern unsigned char moontype;
	long beginning_of_time = 650336715;



	struct time_info_data mud_time_passed(time_t t2, time_t t1);

	time_info = mud_time_passed(time(0), beginning_of_time);

        moontype = time_info.day;

	switch(time_info.hours){
		case 0 :
		case 1 :
		case 2 :
		case 3 :
		case 4 : 
		{
			weather_info.sunlight = SUN_DARK;
			break;
		}
		case 5 :
		case 6 :
		{
			weather_info.sunlight = SUN_RISE;
			break;
		}
		case 7 :
		case 8 :
		case 9 :
		case 10 :
		case 11 :
		case 12 :
		case 13 :
		case 14 :
		case 15 :
		case 16 :
		case 17 :
		case 18 :
		{
			weather_info.sunlight = SUN_LIGHT;
			break;
		}
		case 19 :
		case 20 :
		{
			weather_info.sunlight = SUN_SET;
			break;
		}
		case 21 :
		case 22 :
		case 23 :
		default :
		{
			weather_info.sunlight = SUN_DARK;
			break;
		}
	}

	sprintf(buf,"   Current Gametime: %dH %dD %dM %dY.",
	        time_info.hours, time_info.day,
	        time_info.month, time_info.year);
	vlog(buf);

	weather_info.pressure = 960;
	if ((time_info.month>=7)&&(time_info.month<=12))
		weather_info.pressure += dice(1,50);
	else
		weather_info.pressure += dice(1,80);

	weather_info.change = 0;

	if (weather_info.pressure<=980) {
           if ((time_info.month>=3) && (time_info.month<=14))
		weather_info.sky = SKY_LIGHTNING;
           else
                weather_info.sky = SKY_LIGHTNING;
	} else if (weather_info.pressure<=1000) {
           if ((time_info.month>=3) && (time_info.month<=14))
		weather_info.sky = SKY_RAINING;
           else
		weather_info.sky = SKY_RAINING;
	} else if (weather_info.pressure<=1020) {
		weather_info.sky = SKY_CLOUDY;
	} else {
            weather_info.sky = SKY_CLOUDLESS;
	}
}



/* update the time file */
void update_time(void)
{
	FILE *f1;
	extern struct time_info_data time_info;
	long current_time;

	return;

	if (!(f1 = fopen(TIME_FILE, "w")))
	{
		perror("update time");
		exit(0);
	}

	current_time = time(0);
	vlog("Time update.");

	fprintf(f1, "#\n");

	fprintf(f1, "%d\n", current_time);
	fprintf(f1, "%d\n", time_info.hours);
	fprintf(f1, "%d\n", time_info.day);
	fprintf(f1, "%d\n", time_info.month);
	fprintf(f1, "%d\n", time_info.year);

	fclose(f1);
}



/* generate index table for the player file */
void build_player_index(void)
{
  int nr = -1, i;
  int pc = 0;
  struct char_file_u dummy;
  FILE *fl;

  if (!(fl = fopen(PLAYER_FILE, "rb+")))	{
    perror("build player index");
    exit(0);
  }
  
  for (; !feof(fl);)	{
    fread(&dummy, sizeof(struct char_file_u), 1, fl);
    if (!feof(fl))   /* new record */      	{
      /* Create new entry in the list */
      if (nr == -1) {
	CREATE(player_table, 
	       struct player_index_element, 3000);
	nr = 0;
      }	else {
	if (nr >= pc) {
	   if (!(player_table = (struct player_index_element *)
	      realloc(player_table, (++nr + 50) *
		      sizeof(struct player_index_element))))
	     {
	       perror("generate index");
	       exit(0);
	     }
             pc += 50;
	 } else {
	   nr++;
         }
      }
      
      player_table[nr].nr = nr;
      
      CREATE(player_table[nr].name, char,
	     strlen(dummy.name) + 1);
      for (i = 0; *(player_table[nr].name + i) = 
	   LOWER(*(dummy.name + i)); i++);
    }
  }
  
  fclose(fl);
  
  top_of_p_table = nr;
  
  top_of_p_file = top_of_p_table;
}
	





/* generate index table for object or monster file */
struct index_data *generate_indices(FILE *fl, int *top)
{
  int i = 0;
  long bc=1500;
  struct index_data *index;
  char buf[82];
  
  rewind(fl);
  
  for (;;)	{
    if (fgets(buf, sizeof(buf), fl)) 	{
      if (*buf == '#')       	{
	if (!i)						 /* first cell */
	  CREATE(index, struct index_data, bc);
	else
	  if (i >= bc) {
	    if (!(index = (struct index_data*) 
		realloc(index, (i + 50) * sizeof(struct index_data)))) {
	      perror("load indices");
	      exit(0);
	    } 
            bc += 50;
          }
	sscanf(buf, "#%d", &index[i].virtual);
	index[i].pos = ftell(fl);
	index[i].number = 0;
	index[i].func = 0;
	index[i].name = (index[i].virtual<99999)?fread_string(fl):"omega";
	i++;
      } else {
	if (*buf == '$')	/* EOF */
	  break;
      }
    }    else      {
	fprintf(stderr,"generate indices");
	exit(0);
    }
  }
  *top = i - 2;
  return(index);
}

void cleanout_room(struct room_data *rp)
{
  int	i;
  struct extra_descr_data *exptr, *nptr;

  free(rp->name);
  free(rp->description);
  for (i=0; i<6; i++)
    if (rp->dir_option[i]) {
      free(rp->dir_option[i]->general_description);
      free(rp->dir_option[i]->keyword);
      free (rp->dir_option[i]);
      rp->dir_option[i] = NULL;
    }

  for (exptr=rp->ex_description; exptr; exptr = nptr) {
    nptr = exptr->next;
    free(exptr->keyword);
    free(exptr->description);
    free(exptr);
  }
}

void completely_cleanout_room(struct room_data *rp)
{
  struct char_data	*ch;
  struct obj_data	*obj;
  
  while (rp->people) {
    ch = rp->people;
    act("The hand of god sweeps across the land and you are swept into the Void.", FALSE, NULL, NULL, NULL, TO_VICT);
    char_from_room(ch);
    char_to_room(ch, 0);	/* send character to the void */
  }
  
  while (rp->contents) {
    obj = rp->contents;
    obj_from_room(obj);
    obj_to_room(obj, 0);	/* send item to the void */
  }
  
  cleanout_room(rp);
}
    
void load_one_room(FILE *fl, struct room_data *rp)
{
  char chk[50];
  int   bc=0;
  int	tmp;

  struct extra_descr_data *new_descr;

  bc = sizeof(struct room_data);
  
  rp->name = fread_string(fl);
  if (rp->name && *rp->name)
     bc += strlen(rp->name);
  rp->description = fread_string(fl);
  if (rp->description && *rp->description)
     bc += strlen(rp->description);
  
  if (top_of_zone_table >= 0) {
    int	zone;
    fscanf(fl, " %*d ");
    
    /* OBS: Assumes ordering of input rooms */
    
    for (zone=0;
	 rp->number > zone_table[zone].top && zone<=top_of_zone_table;
	 zone++)
      ;
    if (zone > top_of_zone_table) {
      fprintf(stderr, "Room %d is outside of any zone.\n", rp->number);
      exit(0);
    }
    rp->zone = zone;
  }
  fscanf(fl, " %d ", &tmp);
  rp->room_flags = tmp;
  fscanf(fl, " %d ", &tmp);
  rp->sector_type = tmp;
  
  if (tmp == -1) { 
    fscanf(fl, " %d", &tmp);
    rp->tele_time = tmp;
    fscanf(fl, " %d", &tmp);			  
    rp->tele_targ = tmp;
    fscanf(fl, " %d", &tmp);
    rp->tele_look = tmp;
    fscanf(fl, " %d", &tmp);
    rp->sector_type = tmp;
  } else {
    rp->tele_time = 0;
    rp->tele_targ = 0;
    rp->tele_look = 0;
  }
  
  if (tmp == 7)  { /* river */
    /* read direction and rate of flow */
    fscanf(fl, " %d ", &tmp);
    rp->river_speed = tmp;
    fscanf(fl, " %d ", &tmp);
    rp->river_dir = tmp;
  } 

  if (rp->room_flags & TUNNEL) {  /* read in mobile limit on tunnel */
    fscanf(fl, " %d ", &tmp);
    rp->moblim = tmp;
  }
  
  rp->funct = 0;
  rp->light = 0; /* Zero light sources */
  
  for (tmp = 0; tmp <= 5; tmp++)
    rp->dir_option[tmp] = 0;
  
  rp->ex_description = 0;
  
  while (1==fscanf(fl, " %s \n", chk)) {
    static char	buf[MAX_INPUT_LENGTH];
    switch (*chk) {
    case 'D':
      setup_dir(fl, rp->number, atoi(chk + 1));
      bc += sizeof(struct room_direction_data);
/*      bc += strlen(rp->dir_option[atoi(chk + 1)]->general_description);
      bc += strlen(rp->dir_option[atoi(chk + 1)]->keyword);
*/
      break;
    case 'E': /* extra description field */

      CREATE(new_descr,struct extra_descr_data,1);
      bc += sizeof(struct extra_descr_data);

      new_descr->keyword = fread_string(fl);
      if (new_descr->keyword && *new_descr->keyword)
         bc += strlen(new_descr->keyword);
      else
	fprintf(stderr, "No keyword in room %d\n", rp->number);

      new_descr->description = fread_string(fl);
      if (new_descr->description && *new_descr->description)
         bc += strlen(new_descr->description);
      else
	fprintf(stderr, "No desc in room %d\n", rp->number);

      new_descr->next = rp->ex_description;
      rp->ex_description = new_descr;
      break;
    case 'S':	/* end of current room */

#if BYTE_COUNT
  if (bc >= 1000)
     fprintf(stderr, "Byte count for this room[%d]: %d\n",rp->number,  bc);
#endif
      total_bc += bc;
      room_count++;
      return;
    default:
      sprintf(buf,"unknown auxiliary code `%s' in room load of #%d",
	      chk, rp->number);
      vlog(buf);
      break;
    }
  }
}
  
  

/* load the rooms */
void boot_world(void)
{
  FILE *fl;
  int virtual_nr, j;
  char buf[50];
  struct room_data	*rp;

#if HASH
  init_hash_table(&room_db, sizeof(struct room_data), 2048);
#else
  init_world(room_db);
#endif
  character_list = 0;
  object_list = 0;

  if (!(fl = fopen(WORLD_FILE, "r")))	{
    perror("fopen");
    vlog("World file not found");
    exit(0);
  }
  
  while (1==fscanf(fl, " #%d\n", &virtual_nr)) {
    allocate_room(virtual_nr);
    rp = real_roomp(virtual_nr);
    bzero(rp, sizeof(*rp));
    rp->number = virtual_nr;
    load_one_room(fl, rp);
  }
  
  fclose(fl);
}





void allocate_room(int room_number)
{
  if (room_number>top_of_world)
    top_of_world = room_number;
#if HASH
  hash_find_or_create(&room_db, room_number);
#else
  room_find_or_create(room_db, room_number);
#endif
}






/* read direction data */
void setup_dir(FILE *fl, int room, int dir)
{
	int tmp;
	struct room_data	*rp, dummy;

	rp = real_roomp(room);

        if (!rp) {
	  rp = &dummy;            /* this is a quick fix to make the game */
	  dummy.number = room;   /* stop crashing   */
	}

	CREATE(rp->dir_option[dir], 
		struct room_direction_data, 1);

	rp->dir_option[dir]->general_description =
		fread_string(fl);
	rp->dir_option[dir]->keyword = fread_string(fl);

	fscanf(fl, " %d ", &tmp);
	if (tmp == 1)
       	   rp->dir_option[dir]->exit_info = EX_ISDOOR;
	else if (tmp == 2)
       	   rp->dir_option[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF;
	else if (tmp == 3)
	   rp->dir_option[dir]->exit_info = EX_ISDOOR | EX_SECRET;
	else if (tmp == 4)
	   rp->dir_option[dir]->exit_info = EX_ISDOOR | EX_SECRET |
	                                    EX_PICKPROOF;
	else
       	   rp->dir_option[dir]->exit_info = 0;
 
	fscanf(fl, " %d ", &tmp);
	rp->dir_option[dir]->key = tmp;

	fscanf(fl, " %d ", &tmp);
	rp->dir_option[dir]->to_room = tmp;
}


#define LOG_ZONE_ERROR(ch, type, zone, cmd) {\
	sprintf(buf, "error in zone %s cmd %d (%c) resolving %s number", \
		zone_table[zone].name, cmd, ch, type); \
	vlog(buf); \
	}
void renum_zone_table(void)
{
  int zone, comm;
  struct reset_com *cmd;
  char	buf[256];
  
  for (zone = 0; zone <= top_of_zone_table; zone++)
    for (comm = 0; zone_table[zone].cmd[comm].command != 'S'; comm++)
      switch((cmd = zone_table[zone].cmd +comm)->command) {
      case 'M':
	cmd->arg1 = real_mobile(cmd->arg1);
	if (cmd->arg1<0)
	  LOG_ZONE_ERROR('M', "mobile", zone, comm);
	if(cmd->arg3<0)
	  LOG_ZONE_ERROR('M', "room", zone, comm);
	break;
      case 'C':
	cmd->arg1 = real_mobile(cmd->arg1);
	if (cmd->arg1<0)
	  LOG_ZONE_ERROR('C', "mobile", zone, comm);
	/*cmd->arg3 = real_room(cmd->arg3);*/
	if(cmd->arg3<0)
	  LOG_ZONE_ERROR('C', "room", zone, comm);
	break;
      case 'O':
	cmd->arg1 = real_object(cmd->arg1);
	if(cmd->arg1<0)
	  LOG_ZONE_ERROR('O', "object", zone, comm);
	if (cmd->arg3 != NOWHERE) {
	  /*cmd->arg3 = real_room(cmd->arg3);*/
	  if(cmd->arg3<0)
	    LOG_ZONE_ERROR('O', "room", zone, comm);
	}
	break;
      case 'G':
	cmd->arg1 = real_object(cmd->arg1);
	if(cmd->arg1<0)
	  LOG_ZONE_ERROR('G', "object", zone, comm);
	break;
      case 'E':
	cmd->arg1 = real_object(cmd->arg1);
	if(cmd->arg1<0)
	  LOG_ZONE_ERROR('E', "object", zone, comm);
	break;
      case 'P':
	cmd->arg1 = real_object(cmd->arg1);
	if(cmd->arg1<0)
	  LOG_ZONE_ERROR('P', "object", zone, comm);
	cmd->arg3 = real_object(cmd->arg3);
	if(cmd->arg3<0)
	  LOG_ZONE_ERROR('P', "object", zone, comm);
	break;					
      case 'D':
	/*cmd->arg1 = real_room(cmd->arg1);*/
	if(cmd->arg1<0)
	  LOG_ZONE_ERROR('D', "room", zone, comm);
	break;
      }
}


/* load the zone table and command tables */
void boot_zones(void)
{
  FILE *fl;
  int zon = 0, cmd_no = 0, expand, tmp, bc=100, cc = 20;
  char *check, buf[81];
  
  if (!(fl = fopen(ZONE_FILE, "r")))	{
    perror("boot_zones");
    exit(0);
  }
  
  for (;;)	{
    fscanf(fl, " #%*d\n");
    check = fread_string(fl);
    
    if (*check == '$')
      break;		/* end of file */
    
    /* alloc a new zone */
    
    if (!zon)
      CREATE(zone_table, struct zone_data, bc);
    else
      if (zon >= bc) {
        if (!(zone_table = (struct zone_data *) realloc(zone_table,
						      (zon + 10) * sizeof(struct zone_data))))
	{
	  perror("boot_zones realloc");
	  exit(0);
	}
        bc += 10;
      }
    zone_table[zon].name = check;
    fscanf(fl, " %d ", &zone_table[zon].top);
    fscanf(fl, " %d ", &zone_table[zon].lifespan);
    fscanf(fl, " %d ", &zone_table[zon].reset_mode);
    
    /* read the command table */
    
    cmd_no = 0;
    
    for (expand = 1;;)     	{
      if (expand)
	if (!cmd_no)
	  CREATE(zone_table[zon].cmd, struct reset_com, cc);
	else
	  if (cmd_no >= cc) {
  	    if (!(zone_table[zon].cmd =
		(struct reset_com *) realloc(zone_table[zon].cmd, 
				   (cmd_no + 5) * sizeof(struct reset_com))))
	      {
	        perror("reset command load");
	        exit(0);
	      }
	      cc += 5;
          }
      
      expand = 1;
      
      fscanf(fl, " "); /* skip blanks */
      fscanf(fl, "%c", 
	     &zone_table[zon].cmd[cmd_no].command);
      
      if (zone_table[zon].cmd[cmd_no].command == 'S')
	break;
      
      if (zone_table[zon].cmd[cmd_no].command == '*')	{
	expand = 0;
	fgets(buf, 80, fl); /* skip command */
	continue;
      }
      
      fscanf(fl, " %d %d %d", 
	     &tmp,
	     &zone_table[zon].cmd[cmd_no].arg1,
	     &zone_table[zon].cmd[cmd_no].arg2);
      
      zone_table[zon].cmd[cmd_no].if_flag = tmp;
      
      if (zone_table[zon].cmd[cmd_no].command == 'M' ||
	  zone_table[zon].cmd[cmd_no].command == 'O' ||
	  zone_table[zon].cmd[cmd_no].command == 'C' ||
	  zone_table[zon].cmd[cmd_no].command == 'E' ||
	  zone_table[zon].cmd[cmd_no].command == 'P' ||
	  zone_table[zon].cmd[cmd_no].command == 'D')
	fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);
      
      fgets(buf, 80, fl);	/* read comment */

#if LIMITED_ITEMS
      /*
      **  check for mininum limits on items  (adjust them up or down)
      **  Clearly, this is not implemented yet.
      */
#endif
      cmd_no++;
    }
    zon++;
  }
  top_of_zone_table = --zon;
  free(check);
  fclose(fl);
}


/*************************************************************************
*  procedures for resetting, both play-time and boot-time	 	 *
*********************************************************************** */


/* read a mobile from MOB_FILE */
struct char_data *read_mobile(int nr, int type)
{
  int i;
  long tmp, tmp2, tmp3, bc=0;
  struct char_data *mob;
  char buf[100];
  char letter;

  extern int mob_tick_count;
  extern long mob_count;
  
  i = nr;
  if (type == VIRTUAL)
    if ((nr = real_mobile(nr)) < 0)	{
      sprintf(buf, "Mobile (V) %d does not exist in database.", i);
      return(0);
    }
  
  fseek(mob_f, mob_index[nr].pos, 0);
  
  CREATE(mob, struct char_data, 1);
  bc = sizeof(struct char_data);
  clear_char(mob);
  
  /***** String data *** */
  
  mob->player.name = fread_string(mob_f);
  if (*mob->player.name)
    bc += strlen(mob->player.name);
  mob->player.short_descr = fread_string(mob_f);
  if (*mob->player.short_descr)
    bc += strlen(mob->player.short_descr);
  mob->player.long_descr = fread_string(mob_f);
  if (*mob->player.long_descr)
    bc += strlen(mob->player.long_descr);
  mob->player.description = fread_string(mob_f);
  if (mob->player.description && *mob->player.description)
    bc += strlen(mob->player.description);
  mob->player.title = 0;
  
  /* *** Numeric data *** */
  
  mob->mult_att = 1.0;
  
  fscanf(mob_f, "%d ", &tmp);
  mob->specials.act = tmp;
  SET_BIT(mob->specials.act, ACT_ISNPC);
  
  fscanf(mob_f, " %d ", &tmp);
  mob->specials.affected_by = tmp;
  
  fscanf(mob_f, " %d ", &tmp);
  mob->specials.alignment = tmp;

  mob->player.class = CLASS_WARRIOR;
  
  fscanf(mob_f, " %c ", &letter);
  
  if (letter == 'S') {
    
    fscanf(mob_f, "\n");
    
    fscanf(mob_f, " %d ", &tmp);
    GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

    if (GET_LEVEL(mob, WARRIOR_LEVEL_IND) < 50) {
      mob->abilities.str   = 15;
      mob->abilities.intel = 15;
      mob->abilities.wis   = 15;
      mob->abilities.dex   = 15;
      mob->abilities.con   = 15; 
    } else {
      mob->abilities.str     = 18;
      mob->abilities.str_add = 100;
      mob->abilities.wis     = 18;
      mob->abilities.con     = 18;
      mob->abilities.dex     = 18; 
      mob->abilities.intel   = 18;
    }
 
    fscanf(mob_f, " %d ", &tmp);
    mob->points.hitroll = 20-tmp;
    
    fscanf(mob_f, " %d ", &tmp);
    mob->points.armor = 10*tmp;
    
    fscanf(mob_f, " %dd%d+%d ", &tmp, &tmp2, &tmp3);
    mob->points.max_hit = dice(tmp, tmp2)+tmp3;
    mob->points.hit = mob->points.max_hit;
    
    fscanf(mob_f, " %dd%d+%d \n", &tmp, &tmp2, &tmp3);
    mob->points.damroll = tmp3;
    mob->specials.damnodice = tmp;
    mob->specials.damsizedice = tmp2;
    
    mob->points.mana = 10;
    mob->points.max_mana = 10;   
    
    
    mob->points.move = 50;
    mob->points.max_move = 50;
    
    fscanf(mob_f, " %d ", &tmp);
    if (tmp == -1) {
      fscanf(mob_f, " %d ", &tmp);
      mob->points.gold = tmp;
      fscanf(mob_f, " %d ", &tmp);
      GET_EXP(mob) = tmp;
      fscanf(mob_f, " %d \n", &tmp);
      GET_RACE(mob) = tmp;
    } else {
      mob->points.gold = tmp;
      fscanf(mob_f, " %d \n", &tmp);
      GET_EXP(mob) = tmp;
    }
    fscanf(mob_f, " %d ", &tmp);
    mob->specials.position = tmp;
    
    fscanf(mob_f, " %d ", &tmp);
    mob->specials.default_pos = tmp;
    
    fscanf(mob_f, " %d ", &tmp);
    if (tmp < 3) {
      mob->player.sex = tmp;
      mob->immune = 0;
      mob->M_immune = 0;
      mob->susc = 0;
    } else if (tmp < 6) {
      mob->player.sex = (tmp-3);
      fscanf(mob_f, " %d ", &tmp);
      mob->immune = tmp;
      fscanf(mob_f, " %d ", &tmp);
      mob->M_immune = tmp;
      fscanf(mob_f, " %d ", &tmp);
      mob->susc = tmp;
    } else {
      mob->player.sex = 0;
      mob->immune = 0;
      mob->M_immune = 0;
      mob->susc = 0;
    }
    
    fscanf(mob_f,"\n");
    
    mob->player.class = 0;
    
    mob->player.time.birth = time(0);
    mob->player.time.played	= 0;
    mob->player.time.logon  = time(0);
    mob->player.weight = 200;
    mob->player.height = 198;
    
    for (i = 0; i < 3; i++)
      GET_COND(mob, i) = -1;
    
    for (i = 0; i < 5; i++)
      mob->specials.apply_saving_throw[i] =
            20-(GET_LEVEL(mob, WARRIOR_LEVEL_IND)/2);

    
  } else if ((letter == 'A') || (letter == 'N') || (letter == 'B') ||
	     (letter == 'L')) {
    
    if ((letter == 'A') || (letter == 'B') || (letter == 'L')) {
      fscanf(mob_f, " %d ", &tmp);
      mob->mult_att = (float)tmp;
      /*
      **  read in types:
      */
      /*
	for (i=0;i<mob->mult_att && i < 10; i++) {
	   fscanf(mob_f, " %d ", &tmp);
	   mob->att_type[i] = tmp;
	}
      */
    }
    
    fscanf(mob_f, "\n");
    
    fscanf(mob_f, " %d ", &tmp);
    GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

    if (GET_LEVEL(mob, WARRIOR_LEVEL_IND) < 50) {
    
      mob->abilities.con = 15;
      mob->abilities.dex = 15;
      mob->abilities.str = 15;
      mob->abilities.intel = 15;
      mob->abilities.wis = 15;
    } else {
      mob->abilities.con = 18;
      mob->abilities.dex = 18;
      mob->abilities.str = 18;
      mob->abilities.str_add = 100;
      mob->abilities.intel = 18;
      mob->abilities.wis = 18;
    }
    
    fscanf(mob_f, " %d ", &tmp);
    mob->points.hitroll = 20-tmp;
    
    fscanf(mob_f, " %d ", &tmp);
    mob->points.armor = 10*tmp;
    fscanf(mob_f, " %d ", &tmp);
    mob->points.max_hit = dice(GET_LEVEL(mob, WARRIOR_LEVEL_IND), 8)+tmp;
    mob->points.hit = mob->points.max_hit;
    
    fscanf(mob_f, " %dd%d+%d \n", &tmp, &tmp2, &tmp3);
    mob->points.damroll = tmp3;
    mob->specials.damnodice = tmp;
    mob->specials.damsizedice = tmp2;
    
    mob->points.mana = 10;
    mob->points.max_mana = 10;   
    
    
    mob->points.move = 50;
    mob->points.max_move = 50;
    
    fscanf(mob_f, " %d ", &tmp);
    
    if (tmp == -1) {
      fscanf(mob_f, " %d ", &tmp);
      mob->points.gold = tmp;
      fscanf(mob_f, " %d ", &tmp);
      GET_EXP(mob) = (DetermineExp(mob, tmp)+mob->points.gold);
      fscanf(mob_f, " %d ", &tmp);
      GET_RACE(mob) = tmp;
      
    } else {
      mob->points.gold = tmp;
      
      /*
	this is where the new exp will come into play
	*/
      fscanf(mob_f, " %d \n", &tmp);
      GET_EXP(mob) = (DetermineExp(mob, tmp)+mob->points.gold);
    }
    fscanf(mob_f, " %d ", &tmp);
    mob->specials.position = tmp;
    
    fscanf(mob_f, " %d ", &tmp);
    mob->specials.default_pos = tmp;
    
    fscanf(mob_f, " %d \n", &tmp);
    if (tmp < 3) {
      mob->player.sex = tmp;
      mob->immune = 0;
      mob->M_immune = 0;
      mob->susc = 0;
    } else if (tmp < 6) {
      mob->player.sex = (tmp-3);
      fscanf(mob_f, " %d ", &tmp);
      mob->immune = tmp;
      fscanf(mob_f, " %d ", &tmp);
      mob->M_immune = tmp;
      fscanf(mob_f, " %d ", &tmp);
      mob->susc = tmp;
    } else {
      mob->player.sex = 0;
      mob->immune = 0;
      mob->M_immune = 0;
      mob->susc = 0;
    }

    /*
     *   read in the sound string for a mobile
     */
    if (letter == 'L') {
       mob->player.sounds = fread_string(mob_f);
       if (mob->player.sounds && *mob->player.sounds)
           bc += strlen(mob->player.sounds);

       mob->player.distant_snds = fread_string(mob_f);
       if (mob->player.distant_snds && *mob->player.distant_snds)
           bc += strlen(mob->player.distant_snds);
    } else {
      mob->player.sounds = 0;
      mob->player.distant_snds = 0;
    }

    mob->player.class = 0;
    
    mob->player.time.birth = time(0);
    mob->player.time.played	= 0;
    mob->player.time.logon  = time(0);
    mob->player.weight = 200;
    mob->player.height = 198;
    
    for (i = 0; i < 3; i++)
      GET_COND(mob, i) = -1;
    
    for (i = 0; i < 5; i++)
      mob->specials.apply_saving_throw[i] = 
        20-(GET_LEVEL(mob, WARRIOR_LEVEL_IND)/2);
    
  } else {  /* The old monsters are down below here */
    
    fscanf(mob_f, "\n");
    
    fscanf(mob_f, " %d ", &tmp);
    mob->abilities.str = tmp;
    
    fscanf(mob_f, " %d ", &tmp);
    mob->abilities.intel = tmp; 
    
    fscanf(mob_f, " %d ", &tmp);
    mob->abilities.wis = tmp;
    
    fscanf(mob_f, " %d ", &tmp);
    mob->abilities.dex = tmp;
    
    fscanf(mob_f, " %d \n", &tmp);
    mob->abilities.con = tmp;

    fscanf(mob_f, " %d ", &tmp);
    fscanf(mob_f, " %d ", &tmp2);
    
    mob->points.max_hit = number(tmp, tmp2);
    mob->points.hit = mob->points.max_hit;
    
    fscanf(mob_f, " %d ", &tmp);
    mob->points.armor = 10*tmp;
    
    fscanf(mob_f, " %d ", &tmp);
    mob->points.mana = tmp;
    mob->points.max_mana = tmp;
    
    fscanf(mob_f, " %d ", &tmp);
    mob->points.move = tmp;		
    mob->points.max_move = tmp;
    
    fscanf(mob_f, " %d ", &tmp);
    mob->points.gold = tmp;
    
    fscanf(mob_f, " %d \n", &tmp);
    GET_EXP(mob) = tmp;
    
    fscanf(mob_f, " %d ", &tmp);
    mob->specials.position = tmp;
    
    fscanf(mob_f, " %d ", &tmp);
    mob->specials.default_pos = tmp;
    
    fscanf(mob_f, " %d ", &tmp);
    mob->player.sex = tmp;
    
    fscanf(mob_f, " %d ", &tmp);
    mob->player.class = tmp;
    
    fscanf(mob_f, " %d ", &tmp);
    GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;
    
    fscanf(mob_f, " %d ", &tmp);
    mob->player.time.birth = time(0);
    mob->player.time.played	= 0;
    mob->player.time.logon  = time(0);
    
    fscanf(mob_f, " %d ", &tmp);
    mob->player.weight = tmp;
    
    fscanf(mob_f, " %d \n", &tmp);
    mob->player.height = tmp;
    
    for (i = 0; i < 3; i++)
      {
	fscanf(mob_f, " %d ", &tmp);
	GET_COND(mob, i) = tmp;
      }
    fscanf(mob_f, " \n ");
    
    for (i = 0; i < 5; i++)
      {
	fscanf(mob_f, " %d ", &tmp);
	mob->specials.apply_saving_throw[i] = tmp;
      }
    
    fscanf(mob_f, " \n ");
    
    /* Set the damage as some standard 1d4 */
    mob->points.damroll = 0;
    mob->specials.damnodice = 1;
    mob->specials.damsizedice = 6;
    
    /* Calculate THAC0 as a formular of Level */
    mob->points.hitroll = MAX(1, GET_LEVEL(mob,WARRIOR_LEVEL_IND)-3);
  }
  
  mob->tmpabilities = mob->abilities;
  
  for (i = 0; i < MAX_WEAR; i++) /* Initialisering Ok */
    mob->equipment[i] = 0;
  
  mob->nr = nr;
  
  mob->desc = 0;
  
  if (!IS_SET(mob->specials.act, ACT_ISNPC))
    SET_BIT(mob->specials.act, ACT_ISNPC);
  
  
  /* insert in list */
  
  mob->next = character_list;
  character_list = mob;

  if (mob->points.gold > GET_LEVEL(mob, WARRIOR_LEVEL_IND)*1500) {
    char buf[200];
    sprintf(buf, "%s has gold > level * 1500 (%d)", mob->player.short_descr,
	    mob->points.gold);
    vlog(buf);
  }

  SetRacialStuff(mob);


  /* set up distributed movement system */

  mob->specials.tick = mob_tick_count++;

  if (mob_tick_count == TICK_WRAP_COUNT)
    mob_tick_count=0;
  
  mob_index[nr].number++;

#if BYTE_COUNT
  fprintf(stderr,"Mobile [%d]: byte count: %d\n", mob_index[nr].virtual, bc);
#endif

  total_mbc += bc;
  mob_count++;  
  return(mob);
}


/* read an object from OBJ_FILE */
struct obj_data *read_object(int nr, int type)
{
  struct obj_data *obj;
  int tmp, i;
  long bc;
  char chk[50], buf[100];
  struct extra_descr_data *new_descr;
 
  extern long total_obc;
  
  i = nr;
  if (type == VIRTUAL) {
    nr = real_object(nr);
  }
  if (nr<0 || nr>top_of_objt) {
    sprintf(buf, "Object (V) %d does not exist in database.", i);
    return(0);
  }
  
  fseek(obj_f, obj_index[nr].pos, 0);
  
  CREATE(obj, struct obj_data, 1);
  bc = sizeof(struct obj_data);
    
  clear_object(obj);
  
  /* *** string data *** */
  
  obj->name = fread_string(obj_f);
  if (obj->name && *obj->name) {
    bc += strlen(obj->name);
  }
  obj->short_description = fread_string(obj_f);
  if (obj->short_description && *obj->short_description) {
    bc += strlen(obj->short_description);
  }
  obj->description = fread_string(obj_f);
  if (obj->description && *obj->description) {
    bc += strlen(obj->description);
  }
  obj->action_description = fread_string(obj_f);
  if (obj->action_description && *obj->action_description) {
    bc += strlen(obj->action_description);
  }
  
  /* *** numeric data *** */
  
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.type_flag = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.extra_flags = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.wear_flags = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.value[0] = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.value[1] = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.value[2] = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.value[3] = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.weight = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.cost = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.cost_per_day = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.struct_points = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.max_struct_points = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.decay_time = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.volume = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.material_points = (ubyte) tmp;
  
  /* *** extra descriptions *** */
  
  obj->ex_description = 0;
  
  while (fscanf(obj_f, " %s \n", chk), *chk == 'E')     {
    CREATE(new_descr, struct extra_descr_data, 1);
    bc += sizeof(struct extra_descr_data);
    new_descr->keyword = fread_string(obj_f);
    if (new_descr->keyword && *new_descr->keyword)
      bc += strlen(new_descr->keyword);
    new_descr->description = fread_string(obj_f);
    if (new_descr->description && *new_descr->description)
      bc += strlen(new_descr->description);
    
    new_descr->next = obj->ex_description;
    obj->ex_description = new_descr;
  }
  
  for( i = 0 ; (i < MAX_OBJ_AFFECT) && (*chk == 'A') ; i++)     {
    fscanf(obj_f, " %d ", &tmp);
    obj->affected[i].location = tmp;
    fscanf(obj_f, " %d \n", &tmp);
    obj->affected[i].modifier = tmp;
    fscanf(obj_f, " %s \n", chk);
  }
  
  for (;(i < MAX_OBJ_AFFECT);i++)       {
    obj->affected[i].location = APPLY_NONE;
    obj->affected[i].modifier = 0;
  }
  
  obj->in_room = NOWHERE;
  obj->next_content = 0;
  obj->carried_by = 0;
  obj->equipped_by = 0;
  obj->eq_pos = -1;
  obj->in_obj = 0;
  obj->contains = 0;
  obj->item_number = nr;        
  
  obj->next = object_list;
  object_list = obj;
  
  obj_index[nr].number++;
 
  if (ITEM_TYPE(obj) == ITEM_BOARD) {
    InitABoard(obj);
  }
 
  obj_count++;  
#if BYTE_COUNT
  fprintf(stderr, "Object [%d] uses %d bytes\n", obj_index[nr].virtual, bc);
#endif
  total_obc += bc;
  return (obj);  
}



#define ZO_DEAD  999

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void zone_update(void)
{
  int i;
  struct reset_q_element *update_u, *temp, *tmp2;
  extern struct reset_q_type reset_q;
  
  
  /* enqueue zones */
  
  for (i = 0; i <= top_of_zone_table; i++)	{
    if (zone_table[i].age < zone_table[i].lifespan &&
	zone_table[i].reset_mode)
      (zone_table[i].age)++;
    else
      if (zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode)	{
	/* enqueue zone */
	
	CREATE(update_u, struct reset_q_element, 1);
	
	update_u->zone_to_reset = i;
	update_u->next = 0;
	
	if (!reset_q.head)
	  reset_q.head = reset_q.tail = update_u;
	else  {
	  reset_q.tail->next = update_u;
	  reset_q.tail = update_u;
	}
	
	zone_table[i].age = ZO_DEAD;
      }
  }
  
  /* dequeue zones (if possible) and reset */
  
  for (update_u = reset_q.head; update_u; update_u = tmp2) {
    if (update_u->zone_to_reset > top_of_zone_table) {
      
      /*  this may or may not work */
      /*  may result in some lost memory, but the loss is not signifigant
	  over the short run
	  */
      update_u->zone_to_reset = 0;
      update_u->next = 0;
    }
    tmp2 = update_u->next;
    
    if (zone_table[update_u->zone_to_reset].reset_mode == 2 ||
	is_empty(update_u->zone_to_reset)) {
      reset_zone(update_u->zone_to_reset);
      /* dequeue */
      
      if (update_u == reset_q.head)
	reset_q.head = reset_q.head->next;
      else {
	for (temp = reset_q.head; temp->next != update_u; temp = temp->next)
	  ;
	
	if (!update_u->next) 
	  reset_q.tail = temp;
	
	temp->next = update_u->next;
	
      }
      
      free(update_u);
    }
  }
}




#define ZCMD zone_table[zone].cmd[cmd_no]

/* execute the reset command table of a given zone */
void reset_zone(int zone)
{
  int cmd_no, last_cmd = 1;
  char buf[256];
  struct char_data *mob;
  struct char_data *master;
  struct obj_data *obj, *obj_to;
  struct room_data	*rp;

  mob = 0;
  
  for (cmd_no = 0;;cmd_no++) {
      if (ZCMD.command == 'S')
	break;
      
      if (last_cmd || !ZCMD.if_flag)
	switch(ZCMD.command) {
	  case 'M': /* read a mobile */
	    if (mob_index[ZCMD.arg1].number < ZCMD.arg2) {
		mob = read_mobile(ZCMD.arg1, REAL);
		mob->specials.zone = zone;
		char_to_room(mob, ZCMD.arg3);
		last_cmd = 1;
	      } else
	      last_cmd = 0;
	    break;

	  case 'C': /* read a mobile.  Charm them to follow prev. */
	    if (mob_index[ZCMD.arg1].number < ZCMD.arg2) {
	        master = mob;
   		mob = read_mobile(ZCMD.arg1, REAL);
		mob->specials.zone = zone;
		char_to_room(mob, ZCMD.arg3);
		if (master) {
   		   /*
		     add the charm bit to the dude.
	   	   */
		   add_follower(mob, master);
		   SET_BIT(mob->specials.affected_by, AFF_CHARM);
		}
		last_cmd = 1;
	      }
	    else
	      last_cmd = 0;
	    break;
	    
	  case 'O': /* read an object */
	    if (obj_index[ZCMD.arg1].number < ZCMD.arg2)
	      if (ZCMD.arg3 >= 0 && ((rp = real_roomp(ZCMD.arg3)) != NULL))
		{
		    if ((obj = read_object(ZCMD.arg1, REAL)) != NULL) {
		       obj_to_room(obj, ZCMD.arg3);
		       last_cmd = 1;
		  } else
		    last_cmd = 0;
		} else if (obj = read_object(ZCMD.arg1, REAL)) {
		  sprintf(buf, "Error finding room #%d", ZCMD.arg3);
		  vlog(buf);
		  extract_obj(obj);
		  last_cmd = 1;
		}  else
		  last_cmd = 0;
	    break;
	    
	  case 'P': /* object to object */
	    if (obj_index[ZCMD.arg1].number < ZCMD.arg2)  {
	      obj = read_object(ZCMD.arg1, REAL);
	      obj_to = get_obj_num(ZCMD.arg3);
	      if (obj_to && obj) {
		obj_to_obj(obj, obj_to);
		last_cmd = 1;
	      } else {
		last_cmd = 0;
	      }
	    }
	    else
	      last_cmd = 0;
	    break;
	    
	  case 'G': /* obj_to_char */
	    if (obj_index[ZCMD.arg1].number < ZCMD.arg2 &&
		(obj = read_object(ZCMD.arg1, REAL))) {
	      obj_to_char(obj, mob);
	      last_cmd = 1;
	    } else
	      last_cmd = 0;
	    break;
	    
	  case 'H': /* hatred to char */
	    
	    if (AddHatred(mob, ZCMD.arg1, ZCMD.arg2))
	      last_cmd = 1;
	    else 
	      last_cmd = 0;
	    break;
	    
	  case 'F': /* fear to char */
	    
	    if (AddFears(mob, ZCMD.arg1, ZCMD.arg2))
	      last_cmd = 1;
	    else 
	      last_cmd = 0;
	    break;
	    
	  case 'E': /* object to equipment list */
	    if (obj_index[ZCMD.arg1].number < ZCMD.arg2 &&
		(obj = read_object(ZCMD.arg1, REAL)))
	      {		
		equip_char(mob, obj, ZCMD.arg3);
		last_cmd = 1;
	      }
	    else
	      last_cmd = 0;
	    break;
	    
	  case 'D': /* set state of door */
	    rp = real_roomp(ZCMD.arg1);
	    if (rp && rp->dir_option[ZCMD.arg2]) {
	      switch (ZCMD.arg3) {
	      case 0:
		REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
		REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
		break;
	      case 1:
		SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
		REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
		break;
	      case 2:
		SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
		SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
		break;
	      }
	      last_cmd = 1;
	    } else {
	      /* that exit doesn't exist anymore */
	    }
	    break;
	    
	  default:
	    sprintf(buf, "Undefd cmd in reset table; zone %d cmd %d.\n\r",
		    zone, cmd_no);
	    vlog(buf);
	    break;
	  }
      else
	last_cmd = 0;
      
    }
  
  zone_table[zone].age = 0;
}

#undef ZCMD

/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int is_empty(int zone_nr)
{
	struct descriptor_data *i;

	for (i = descriptor_list; i; i = i->next)
		if (!i->connected)
      if (real_roomp(i->character->in_room)->zone == zone_nr)
				return(0);

	return(1);
}





/*************************************************************************
*  stuff related to the save/load player system								  *
*********************************************************************** */

/* Load a char, TRUE if loaded, FALSE if not */
int load_char(char *name, struct char_file_u *char_element)
{
	FILE *fl;
	int player_i;

	int find_name(char *name);

	if ((player_i = find_name(name)) >= 0) {

		if (!(fl = fopen(PLAYER_FILE, "r"))) {
			perror("Opening player file for reading. (db.c, load_char)");
			exit(0);
		}

		fseek(fl, (long) (player_table[player_i].nr *
		sizeof(struct char_file_u)), 0);

		fread(char_element, sizeof(struct char_file_u), 1, fl);
		fclose(fl);
/*
**  Kludge for ressurection
*/
		char_element->talks[2] = TRUE;  
		return(player_i);
	} else

		return(-1);
}




/* copy data from the file structure to a char struct */	
void store_to_char(struct char_file_u *st, struct char_data *ch)
{
	int i;

	GET_SEX(ch) = st->sex;
	ch->player.class = st->class;

	for (i=MAGE_LEVEL_IND; i<= RANGER_LEVEL_IND; i++)
	  ch->player.level[i] = st->level[i];

	GET_RACE(ch)  = st->race;

	ch->player.short_descr = 0;
	ch->player.long_descr = 0;

	if (*st->title)	{
		CREATE(ch->player.title, char, strlen(st->title) + 1);
		strcpy(ch->player.title, st->title);
	}  else
		GET_TITLE(ch) = 0;

	if (*st->description)	{
		CREATE(ch->player.description, char, 
			strlen(st->description) + 1);
		strcpy(ch->player.description, st->description);
	} else
		ch->player.description = 0;

	ch->player.hometown = st->hometown;

	ch->player.time.birth = st->birth;
	ch->player.time.played = st->played;
	ch->player.time.logon  = time(0);

	for (i = 0; i <= MAX_TOUNGE - 1; i++)
		ch->player.talks[i] = st->talks[i];

	ch->player.weight = st->weight;
	ch->player.height = st->height;

	ch->abilities = st->abilities;
	ch->tmpabilities = st->abilities;
	ch->points = st->points;

        SpaceForSkills(ch);
	
	for (i = 0; i <= MAX_SKILLS - 1; i++)
		ch->skills[i] = st->skills[i];

	ch->specials.spells_to_learn = st->spells_to_learn;
	ch->specials.alignment    = st->alignment;

	ch->specials.act          = st->act;
	ch->specials.carry_weight = 0;
	ch->specials.carry_items  = 0;
	ch->points.armor          = 100;
	ch->points.hitroll        = 0;
	ch->points.damroll        = 0;

	CREATE(GET_NAME(ch), char, strlen(st->name) +1);
	strcpy(GET_NAME(ch), st->name);

	/* Not used as far as I can see (Michael) */
	for(i = 0; i <= 4; i++)
	  ch->specials.apply_saving_throw[i] = st->apply_saving_throw[i];

	for(i = 0; i <= 2; i++)
	  GET_COND(ch, i) = st->conditions[i];

	/* Add all spell effects */
	for(i=0; i < MAX_AFFECT; i++) {
		if (st->affected[i].type)
			affect_to_char(ch, &st->affected[i]);
	}
	ch->in_room = st->load_room;
	affect_total(ch);
} /* store_to_char */

	

	
/* copy vital data from a players char-structure to the file structure */
void char_to_store(struct char_data *ch, struct char_file_u *st)
{
	int i;
	struct affected_type *af;
	struct obj_data *char_eq[MAX_WEAR];

	/* Unaffect everything a character can be affected by */

	for(i=0; i<MAX_WEAR; i++) {
		if (ch->equipment[i])
			char_eq[i] = unequip_char_for_save(ch, i);
		else
			char_eq[i] = 0;
	}

	for(af = ch->affected, i = 0; i<MAX_AFFECT; i++) {
		if (af) {
			st->affected[i] = *af;
			st->affected[i].next = 0;
			/* subtract effect of the spell or the effect will be doubled */
			affect_modify( ch, st->affected[i].location,
			                   st->affected[i].modifier,
			                   st->affected[i].bitvector, FALSE);                         
			af = af->next;
		} else {
			st->affected[i].type = 0;  /* Zero signifies not used */
			st->affected[i].duration = 0;
			st->affected[i].modifier = 0;
			st->affected[i].location = 0;
			st->affected[i].bitvector = 0;
			st->affected[i].next = 0;
		}
	}

	if ((i >= MAX_AFFECT) && af && af->next)
		vlog("WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");



	ch->tmpabilities = ch->abilities;

	st->birth      = ch->player.time.birth;
	st->played     = ch->player.time.played;
	st->played    += (long) (time(0) - ch->player.time.logon);
	st->last_logon = time(0);

	ch->player.time.played = st->played;
	ch->player.time.logon = time(0);

	st->hometown = ch->player.hometown;
	st->weight   = GET_WEIGHT(ch);
	st->height   = GET_HEIGHT(ch);
	st->sex      = GET_SEX(ch);
	st->class    = ch->player.class;
	for (i=MAGE_LEVEL_IND; i<= RANGER_LEVEL_IND; i++) 
	   st->level[i]    = ch->player.level[i];
	st->race     = GET_RACE(ch);

	st->abilities = ch->abilities;
	st->points    = ch->points;
	st->alignment       = ch->specials.alignment;
	st->spells_to_learn = ch->specials.spells_to_learn;
	st->act             = ch->specials.act;

	st->points.armor   = 100;
	st->points.hitroll =  0;
	st->points.damroll =  0;

	if (GET_TITLE(ch))
		strcpy(st->title, GET_TITLE(ch));
	else
		*st->title = '\0';

	if (ch->player.description)
		strcpy(st->description, ch->player.description);
	else
		*st->description = '\0';


	for (i = 0; i <= MAX_TOUNGE - 1; i++)
		st->talks[i] = ch->player.talks[i];

	for (i = 0; i <= MAX_SKILLS - 1; i++)
		st->skills[i] = ch->skills[i];

	strcpy(st->name, GET_NAME(ch) );

	for(i = 0; i <= 4; i++)
	  st->apply_saving_throw[i] = ch->specials.apply_saving_throw[i];

	for(i = 0; i <= 2; i++)
	  st->conditions[i] = GET_COND(ch, i);

	for(af = ch->affected, i = 0; i<MAX_AFFECT; i++) {
		if (af) {
			/* Add effect of the spell or it will be lost */
			/* When saving without quitting               */
			affect_modify( ch, st->affected[i].location,
			                   st->affected[i].modifier,
			                   st->affected[i].bitvector, TRUE);
			af = af->next;
		}
	}

	for(i=0; i<MAX_WEAR; i++) {
		if (char_eq[i])
			equip_char(ch, char_eq[i], i);
	}

	affect_total(ch);
} /* Char to store */




/* create a new entry in the in-memory index table for the player file */
int create_entry(char *name)
{
	int i;

	if (top_of_p_table == -1)	{
		CREATE(player_table, struct player_index_element, 1);
		top_of_p_table = 0;
	}  else
		if (!(player_table = (struct player_index_element *) 
		  realloc(player_table, sizeof(struct player_index_element) * 
		  (++top_of_p_table + 1))))
		{
			perror("create entry");
			exit(1);
		}

	CREATE(player_table[top_of_p_table].name, char , strlen(name) + 1);

	/* copy lowercase equivalent of name to table field */
	for (i = 0; *(player_table[top_of_p_table].name + i) = 
			LOWER(*(name + i)); i++);

	player_table[top_of_p_table].nr = top_of_p_table;

	return (top_of_p_table);

	
}
		


/* write the vital data of a player to the player file */
void save_char(struct char_data *ch, sh_int load_room)
{
	struct char_file_u st;
	FILE *fl;
	char mode[4];
	int expand;
	struct char_data *tmp;

        if (IS_NPC(ch) && !(IS_SET(ch->specials.act, ACT_POLYSELF))) 
	  return;
	
	if (IS_NPC(ch)) {
	  if (!ch->desc) 
	    return;
	  tmp = ch->desc->original;
	  if (!tmp)
	     return;

	} else {
	  if (!ch->desc)
	    return;
	  tmp = 0;
	}

	if (expand = (ch->desc->pos > top_of_p_file))	{
		strcpy(mode, "a");
		top_of_p_file++;
	}  else
		strcpy(mode, "r+");

	if (!tmp)
	  char_to_store(ch, &st);
	else
	  char_to_store(tmp, &st);

	st.load_room = load_room;

	strcpy(st.pwd, ch->desc->pwd);

	if (!(fl = fopen(PLAYER_FILE, mode)))	{
		perror("save char");
		exit(1);
	}

	if (!expand)
		fseek(fl, ch->desc->pos * sizeof(struct char_file_u), 0);

	fwrite(&st, sizeof(struct char_file_u), 1, fl);

	fclose(fl);
}




/* for possible later use with qsort */
int compare(struct player_index_element *arg1, struct player_index_element 
	*arg2)
{
	return (str_cmp(arg1->name, arg2->name));
}




/************************************************************************
*  procs of a (more or less) general utility nature			*
********************************************************************** */


/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE *fl)
{
  char buf[MAX_STRING_LENGTH], tmp[MAX_STRING_LENGTH];
  char *rslt;
  register char *point;
  int flag;

  bzero(buf, sizeof(buf));
    
  do	{
    if (!fgets(tmp, MAX_STRING_LENGTH, fl))		{
      perror("fread_str");
      vlog("File read error.");
      return("Empty");
    }
    
    if (strlen(tmp) + strlen(buf) + 1 > MAX_STRING_LENGTH) {
      vlog("fread_string: string too large (db.c)");
      exit(0);
    } else
      strcat(buf, tmp);
    
    for (point = buf + strlen(buf) - 2; point >= buf && isspace(*point);
	 point--);		
    if (flag = (*point == '~'))
      if (*(buf + strlen(buf) - 3) == '\n')
	{
	  *(buf + strlen(buf) - 2) = '\r';
	  *(buf + strlen(buf) - 1) = '\0';
	}
      else
	*(buf + strlen(buf) -2) = '\0';
    else
      {
	*(buf + strlen(buf) + 1) = '\0';
	*(buf + strlen(buf)) = '\r';
      }
  } while (!flag);
  
  /* do the allocate boogie  */
  
  if (strlen(buf) > 0)    {
      CREATE(rslt, char, strlen(buf) + 1);
      strcpy(rslt, buf);
  }  else
    rslt = 0;
  return(rslt);
}





/* release memory allocated for a char struct */
void free_char(struct char_data *ch)
{
	struct affected_type *af;

	free(GET_NAME(ch));

  	if (ch->player.title)
		free(ch->player.title);
        if (ch->act_ptr)
                free(ch->act_ptr);
	if (ch->player.short_descr)
		free(ch->player.short_descr);
	if (ch->player.long_descr)
		free(ch->player.long_descr);
	if(ch->player.description)
		free(ch->player.description);
	if (ch->player.sounds)
	  free(ch->player.sounds);
	if (ch->player.distant_snds)
	  free(ch->player.distant_snds);

	for (af = ch->affected; af; af = af->next) 
		affect_remove(ch, af);
	if (ch->skills)
	  free(ch->skills);

	free(ch);
}







/* release memory allocated for an obj struct */
void free_obj(struct obj_data *obj)
{
	struct extra_descr_data *this, *next_one;

	free(obj->name);
	if(obj->description && *obj->description)
		free(obj->description);
	if(obj->short_description && *obj->short_description)
		free(obj->short_description);
	if(obj->action_description && *obj->action_description)
		free(obj->action_description);

	for( this = obj->ex_description ;
		(this != 0);this = next_one )
	{
		next_one = this->next;
		if(this->keyword)
			free(this->keyword);
		if(this->description)
			free(this->description);
		free(this);
	}

	free(obj);
}






/* read contents of a text file, and place in buf */
int file_to_string(char *name, char *buf)
{
	FILE *fl;
	char tmp[100];

	*buf = '\0';

	if (!(fl = fopen(name, "r")))
	{
		perror("file-to-string");
		*buf = '\0';
		return(-1);
	}

	do
	{
		fgets(tmp, 99, fl);

		if (!feof(fl))
		{
			if (strlen(buf) + strlen(tmp) + 2 > MAX_STRING_LENGTH)
			{
				vlog("fl->strng: string too big (db.c, file_to_string)");
				*buf = '\0';
				fclose(fl);
				return(-1);
			}

			strcat(buf, tmp);
			*(buf + strlen(buf) + 1) = '\0';
			*(buf + strlen(buf)) = '\r';
		}
	}
	while (!feof(fl));

	fclose(fl);

	return(0);
}


void ClearDeadBit(struct char_data *ch)
{
 
  FILE *fl;
  struct char_file_u st;
 
  fl = fopen(PLAYER_FILE, "r+");
  if (!fl) {
    perror("player file");
    exit(0);
  }
 
  fseek(fl, ch->desc->pos * sizeof(struct char_file_u), 0);
  fread(&st, sizeof(struct char_file_u), 1, fl);
  /*
   **   this is a serious kludge, and must be changed before multiple
   **   languages can be implemented
   */   
  if (st.talks[2]) {
    st.talks[2] = 0;  /* fix the 'resurrectable' bit */
    fseek(fl, ch->desc->pos * sizeof(struct char_file_u), 0);
    fwrite(&st, sizeof(struct char_file_u), 1, fl);
    ch->player.talks[2] = 0;  /* fix them both */
  }
  fclose(fl);
 
  
}


/* clear some of the the working variables of a char */
void reset_char(struct char_data *ch)
{
  char buf[100], recipient[100], *tmp;
  struct affected_type *af;
  extern struct dex_app_type dex_app[];
  
  int i, j;
  
  for (i = 0; i < MAX_WEAR; i++) /* Initializing */
    ch->equipment[i] = 0;

  spell_dispel_magic(IMPLEMENTOR,ch,ch,0);

  if (IS_SET(ch->specials.act, PLR_MAILING))
     REMOVE_BIT(ch->specials.act, PLR_MAILING);

  if (GET_RACE(ch)==RACE_OGRE) {
    if (GET_WEIGHT(ch) < 100) 
     GET_WEIGHT(ch) = (200 +number(1,10));
  }

  ch->desc->screen_size = 24;

  if (!strcmp(ch->player.name,"Brutius") ||
      !strcmp(ch->player.name,"Peel") ||
      !strcmp(ch->player.name,"Damescena") ||
      !strcmp(ch->player.name,"Dash") ||
      !strcmp(ch->player.name,"Jesus")) {
    GET_LEVEL(ch,0) = BRUTIUS;
    GET_LEVEL(ch,1) = BRUTIUS;
    GET_LEVEL(ch,2) = BRUTIUS;
    GET_LEVEL(ch,3) = BRUTIUS;
    GET_EXP(ch) = 200000000;
  }




  ch->followers = 0;
  ch->master = 0;
  ch->carrying = 0;
  ch->next = 0;
  
  ch->immune = 0;
  ch->M_immune = 0;
  ch->susc = 0;
  ch->mult_att = 1.0;

  ch->point_roll = 0;
  ch->bet_opt.craps_options = 0;
  ch->bet_opt.one_roll = 0;
  
  if (!GET_RACE(ch))
    GET_RACE(ch) = RACE_HUMAN;
  if (GET_RACE(ch) == RACE_DWARF) {
    if (!IS_AFFECTED(ch, AFF_INFRAVISION)) 
      SET_BIT(ch->specials.affected_by, AFF_INFRAVISION);
  }

  if (HasClass(ch, CLASS_PALADIN)) {
    if (!IS_AFFECTED(ch, AFF_PROTECT_EVIL))
      SET_BIT(ch->specials.affected_by, AFF_PROTECT_EVIL);
  }

  if ((ch->player.class == 3) && (GET_LEVEL(ch, THIEF_LEVEL_IND))) {
    ch->player.class = 8;
    send_to_char("Setting your class to THIEF only.\n\r", ch);
  }

  for (i=0;i<=3;i++) {
    if (GET_LEVEL(ch, i) > BRUTIUS) {
      GET_LEVEL(ch,i) = 51;
    }
  }

  ch->hunt_dist = 0;
  ch->hatefield = 0;
  ch->fearfield = 0;
  ch->hates.clist = 0;
  ch->fears.clist = 0;
  
  /* AC adjustment */
  GET_AC(ch) += dex_app[GET_DEX(ch)].defensive;
  if (GET_AC(ch) > 100)
    GET_AC(ch) = 100;

  GET_HITROLL(ch)=0;
  GET_DAMROLL(ch)=0;
  
  ch->next_fighting = 0;
  ch->next_in_room = 0;
  ch->specials.fighting = 0;
  ch->specials.position = POSITION_STANDING;
  ch->specials.default_pos = POSITION_STANDING;
  ch->specials.carry_weight = 0;
  ch->specials.carry_items = 0;
  
  if (GET_HIT(ch) <= 0)
    GET_HIT(ch) = 1;
  if (GET_MOVE(ch) <= 0)
    GET_MOVE(ch) = 1;
  if (GET_MANA(ch) <= 0)
    GET_MANA(ch) = 1;
  
  ch->points.max_mana = 0;
  ch->points.max_move = 0;
  
  if (IS_IMMORTAL(ch)) {
    GET_BANK(ch) = 0;
    GET_GOLD(ch) = 100000;
  }
  
  if (GET_BANK(ch) > GetMaxLevel(ch)*100000) {
    sprintf(buf, "%s has %d coins in bank.", GET_NAME(ch), GET_BANK(ch));
    vlog(buf);
  }
  if (GET_GOLD(ch) > GetMaxLevel(ch)*100000) {
    sprintf(buf, "%s has %d coins.", GET_NAME(ch), GET_GOLD(ch));
    vlog(buf);
  }

  /*
    Class specific Stuff
    */
  
  ClassSpecificStuff(ch);

  _parse_name(GET_NAME(ch), recipient);

   for (tmp = recipient; *tmp; tmp++)
     if (isupper(*tmp))
       *tmp = tolower(*tmp);

    if (has_mail(recipient)) {
        sprintf(buf, "You have %sMAIL%s.\n\r", VT_BOLDTEX,VT_NORMALT);
        send_to_char(buf, ch);   
    }
 
  if (HasClass(ch, CLASS_MONK)) {
    GET_AC(ch) -= MIN(200, (GET_LEVEL(ch, MONK_LEVEL_IND)*5));
    GET_HITROLL(ch) += GET_LEVEL(ch, MONK_LEVEL_IND)/10;
    ch->points.max_move += GET_LEVEL(ch, MONK_LEVEL_IND);
  }
 
  /*
    racial stuff
    */
  SetRacialStuff(ch);
 
/*
  update the affects on the character.
*/

  for(af = ch->affected; af; af=af->next)
    affect_modify(ch, af->location, (unsigned)af->modifier, af->bitvector, 
                  TRUE);
 
  if (!HasClass(ch, CLASS_MONK))
    GET_AC(ch) += dex_app[GET_DEX(ch)].defensive;
  if (GET_AC(ch) > 100)
    GET_AC(ch) = 100;

  for (i = 0;i < 5; i++)
     ch->specials.apply_saving_throw[i] = 20-(GetMaxLevel(ch)/2);
 
 
/*
  clear out the 'dead' bit on characters
*/
  if (ch->desc)
    ClearDeadBit(ch);
}



/* clear ALL the working variables of a char and do NOT free any space alloc'ed*/
void clear_char(struct char_data *ch)
{
	memset(ch, '\0', sizeof(struct char_data));

	ch->in_room = NOWHERE;
	ch->specials.was_in_room = NOWHERE;
	ch->specials.position = POSITION_STANDING;
	ch->specials.default_pos = POSITION_STANDING;
	GET_AC(ch) = 100; /* Basic Armor */
        ch->player.name = NULL;
}


void clear_object(struct obj_data *obj)
{
	memset(obj, '\0', sizeof(struct obj_data));

	obj->item_number = -1;
	obj->in_room	  = NOWHERE;
	obj->eq_pos       = -1;
}




/* initialize a new character only if class is set */
void init_char(struct char_data *ch)
{
	int i;

	/* *** if this is our first player --- he be God *** */

	if (!strcmp(ch->player.name,"Brutius"))	{
		GET_EXP(ch) = 200000000;
		GET_LEVEL(ch,0) = BRUTIUS;
		GET_LEVEL(ch,1) = BRUTIUS;
		GET_LEVEL(ch,2) = BRUTIUS;
		GET_LEVEL(ch,3) = BRUTIUS;
	}

	set_title(ch);

	ch->player.short_descr = 0;
	ch->player.long_descr = 0;
	ch->player.description = 0;

	ch->player.hometown = number(1,4);

	ch->player.time.birth = time(0);
	ch->player.time.played = 0;
	ch->player.time.logon = time(0);

	for (i = 0; i < MAX_TOUNGE; i++)
	 ch->player.talks[i] = 0;

	GET_STR(ch) = 9;
	GET_INT(ch) = 9;
	GET_WIS(ch) = 9;
	GET_DEX(ch) = 9;
	GET_CON(ch) = 9;

	/* make favors for sex */
	if (GET_RACE(ch) == RACE_HUMAN) {
	   if (ch->player.sex == SEX_MALE) {
		ch->player.weight = number(120,180);
		ch->player.height = number(160,200);
	   } else {
		ch->player.weight = number(100,160);
		ch->player.height = number(150,180);
	   }
	} else if (GET_RACE(ch) == RACE_DWARF) {
	   if (ch->player.sex == SEX_MALE) {
		ch->player.weight = number(120,180);
		ch->player.height = number(100,150);
	   } else {
		ch->player.weight = number(100,160);
		ch->player.height = number(100,150);
	   }
	  
	} else if (GET_RACE(ch) == RACE_ELVEN) {
	   if (ch->player.sex == SEX_MALE) {
		ch->player.weight = number(100,150);
		ch->player.height = number(160,200);
	   } else {
		ch->player.weight = number(80,230);
		ch->player.height = number(150,180);
	   }
        } else if (GET_RACE(ch) == RACE_OGRE) {
	   if (ch->player.sex == SEX_MALE) {
		ch->player.weight = 255;
		ch->player.height = number(200,250);
	   } else {
		ch->player.weight = 255;
		ch->player.height = number(200,250);
	   }
        } else if (GET_RACE(ch) == RACE_GNOME) {
	   if (ch->player.sex == SEX_MALE) {
		ch->player.weight = number(85,100);
		ch->player.height = number(80,130);
	   } else {
		ch->player.weight = number(65,80);
		ch->player.height = number(60,110);
	   }
        } else if (GET_RACE(ch) == RACE_HOBBIT) {
	   if (ch->player.sex == SEX_MALE) {
		ch->player.weight = number(70,90);
		ch->player.height = number(60,100);
	   } else {
		ch->player.weight = number(60,80);
		ch->player.height = number(50,90);
	   }
	} else {
	   if (ch->player.sex == SEX_MALE) {
		ch->player.weight = number(120,180);
		ch->player.height = number(160,200);
	   } else {
		ch->player.weight = number(100,160);
		ch->player.height = number(150,180);
	   }
	}

	ch->points.mana = GET_MAX_MANA(ch);
	ch->points.hit = GET_MAX_HIT(ch);
	ch->points.move = GET_MAX_MOVE(ch);

	ch->points.armor = 100;

	if (!ch->skills)
	  SpaceForSkills(ch);

	for (i = 0; i <= MAX_SKILLS - 1; i++)	{
		if (GetMaxLevel(ch) <IMPLEMENTOR) {
			ch->skills[i].learned = 0;
			ch->skills[i].recognise = FALSE;
		}	else {
			ch->skills[i].learned = 100;
			ch->skills[i].recognise = FALSE;
		}
	}

	ch->specials.affected_by = 0;
	ch->specials.spells_to_learn = 0;

	for (i = 0; i < 5; i++)
		ch->specials.apply_saving_throw[i] = 0;

	for (i = 0; i < 3; i++)
		GET_COND(ch, i) = (GetMaxLevel(ch) > GOD ? -1 : 24);
}


#if 0
/* returns the real number of the room with given virtual number */
int real_room(int virtual)
{
	int bot, top, mid;

	bot = 0;
	top = top_of_world;

	/* perform binary search on world-table */
	for (;;)  {
		mid = (bot + top) / 2;

		if ((world + mid)->number == virtual)
			return(mid);
		if (bot >= top)		{
			fprintf(stderr, "Room %d does not exist in database\n", virtual);
			return(-1);
		}
		if ((world + mid)->number > virtual)
			top = mid - 1;
		else
			bot = mid + 1;
	}
}
#else
/*
**  this duplicates the code in room_find, because it is much quicker this way.
*/
struct room_data *real_roomp(int virtual)
{
#if HASH
  return hash_find(&room_db, virtual);
#else
   return((virtual<WORLD_SIZE&&virtual>-1)?room_db[virtual]:0);
#endif
}
#endif

/* returns the real number of the monster with given virtual number */
int real_mobile(int virtual)
{
	int bot, top, mid;

	bot = 0;
	top = top_of_mobt;

	/* perform binary search on mob-table */
	for (;;)
	{
		mid = (bot + top) / 2;

		if ((mob_index + mid)->virtual == virtual)
			return(mid);
		if (bot >= top)
			return(-1);
		if ((mob_index + mid)->virtual > virtual)
			top = mid - 1;
		else
			bot = mid + 1;
	}
}






/* returns the real number of the object with given virtual number */
int real_object(int virtual)
{
	int bot, top, mid;

	bot = 0;
	top = top_of_objt;

	/* perform binary search on obj-table */
	for (;;)
	{
		mid = (bot + top) / 2;

		if ((obj_index + mid)->virtual == virtual)
			return(mid);
		if (bot >= top)
			return(-1);
		if ((obj_index + mid)->virtual > virtual)
			top = mid - 1;
		else
			bot = mid + 1;
	}
}
