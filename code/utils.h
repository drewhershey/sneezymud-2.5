#pragma once

#include <stdio.h>
#include <time.h>

struct char_data;
struct obj_data;
struct descriptor_data;
struct obj_cost;
struct obj_file_u;
struct room_direction_data;
struct room_data;

#if defined(DEBUG) && DEBUG
#define free(obj)                                \
  fprintf(stderr, "freeing %d\n", sizeof(*obj)); \
  free(obj)
#endif

/* Logging functions */
void vlog(const char* str);
void vlogf(const char* errorMsg, ...);

/* Random number generation */
int number(int from, int to);

#define NUMBER(from, to)                                   \
  ({                                                       \
    decltype(from) _result;                                \
    if ((to) - (from) + 1)                                 \
      _result = ((rand() % ((to) - (from) + 1)) + (from)); \
    else                                                   \
      _result = (from);                                    \
    _result;                                               \
  })

int dice(int number, int size);

#define DICE(number, size)               \
  ({                                     \
    decltype(number) _sum = 0;           \
    decltype(number) _r;                 \
    assert((size) >= 0);                 \
    if ((size) == 0)                     \
      _sum;                              \
    else                                 \
      for (_r = 1; _r <= (number); _r++) \
        _sum += ((rand() % (size)) + 1); \
    _sum;                                \
  })

/* Damage and combat utilities */
int SkipImmortals(struct char_data* v, int amnt);
int PreProcDam(struct char_data* ch, int type, int dam);
int IsUndead(struct char_data* ch);
void update_pos(struct char_data* victim);
void die(struct char_data* ch);
void do_look(struct char_data* ch, const char* argument, int cmd);
void death_cry(struct char_data* ch);
void zero_rent(struct char_data* ch);
int apply_soundproof(struct char_data* ch);
int check_soundproof(struct char_data* ch);
int HasHands(struct char_data* ch);
void RestoreChar(struct char_data* ch);
void RemAllAffects(struct char_data* ch);
void sprintbit(unsigned long vektor, const char* const* names, char* result);
int CAN_SEE_FOR_WHO(struct char_data* s, struct char_data* o);
void do_at(struct char_data* ch, const char* argument, int cmd);
int start_page_file(struct descriptor_data* d, const char* fpath,
  const char* errormsg);
int str_cmp(const char* arg1, const char* arg2);

/* Race type checks */
int IsAnimal(struct char_data* ch);
int IsVeggie(struct char_data* ch);
int IsHumanoid(struct char_data* ch);
int IsDiabolic(struct char_data* ch);
int IsLycanthrope(struct char_data* ch);
int IsReptile(struct char_data* ch);
int IsGiantish(struct char_data* ch);
int IsExtraPlanar(struct char_data* ch);
int IsPerson(struct char_data* ch);
int IsOther(struct char_data* ch);

/* Utility calculations */
int GetApprox(int num, int perc);
int CalcThaco(struct char_data* ch);
void* Mymalloc(long size);

/* Character utilities */
void SpaceForSkills(struct char_data* ch);
void error_log(const char* str);
void slog(const char* str);

/* Weather and time */
void weather_and_time(int mode);
void another_hour(int mode);
void weather_change();
void GetMonth(int month);
void ChangeWeather(int change);
void night_watchman();

/* Object utilities */
void SwitchStuff(struct char_data* giver, struct char_data* taker);
int ObjLevelCheck(struct obj_data* obj, struct char_data* ch);

/* Combat functions */
void hit(struct char_data* ch, struct char_data* victim, int type);
[[nodiscard]] struct char_data* FindVictim(struct char_data* ch);
[[nodiscard]] struct char_data* FindMetaVictim(struct char_data* ch);
[[nodiscard]] struct char_data* FindAnAttacker(struct char_data* ch);
int SameRace(struct char_data* ch1, struct char_data* ch2);
char in_group(struct char_data* ch1, struct char_data* ch2);
int dir_track(struct char_data* ch, struct char_data* vict);
[[nodiscard]] struct char_data* FindAnyVictim(struct char_data* ch);

/* Mobile AI */
void mobile_activity(struct char_data* ch);
void MakeNoise(int room, const char* local_snd, const char* distant_snd);
void weight_change_object(struct obj_data* obj, int weight);
int damage(struct char_data* ch, struct char_data* victim, int damage,
  int weapontype);
struct time_info_data age(struct char_data* ch);
void set_fighting(struct char_data* ch, struct char_data* vict);
int WeaponCheck(struct char_data* ch, struct char_data* v, int type, int dam);
void add_follower(struct char_data* ch, struct char_data* leader);

/* Immunity and resistance */
int IsImmune(struct char_data* ch, int bit);
int WeaponImmune(struct char_data* ch);
int IsResist(struct char_data* ch, int bit);
int IsSusc(struct char_data* ch, int bit);

/* Movement */
int go_direction(struct char_data* ch, int dir);
int choose_exit_in_zone(int in_room, int tgt_room, int depth);
int check_peaceful(struct char_data* ch, const char* msg);
void raw_open_door(struct char_data* ch, int dir);
int MobCountInRoom(struct char_data* list);
int DisplayMove(struct char_data* ch, int dir, int was_in, int total);
void open_door(struct char_data* ch, int dir);
char circle_follow(struct char_data* ch, struct char_data* victim);
int RecGetObjRoom(struct obj_data* obj);
int MissileDamage(struct char_data* ch, struct char_data* victim, int dam,
  int attacktype);
void stop_follower(struct char_data* ch);
void do_start(struct char_data* ch);
int AntiGuildMaster(struct char_data* ch, int cmd, const char* arg);
int getabunch(const char* name, char* newname);
int choose_exit_global(int in_room, int tgt_room, int depth);
void LearnFromMistake(struct char_data* ch, int sknum, int silent, int max);
int fighter(struct char_data* ch, int cmd, const char* arg);
int RecCompObjNum(struct obj_data* o, int obj_num);
int find_door(struct char_data* ch, char* type, char* dir);
void ThrowChar(struct char_data* ch, struct char_data* v, int dir);
int SetVictFighting(struct char_data* ch, struct char_data* v);
int SetCharFighting(struct char_data* ch, struct char_data* v);
void CallForGuard(struct char_data* ch, struct char_data* vict, int lev,
  int area);
void SetHunting(struct char_data* ch, struct char_data* tch);
void make_corpse(struct char_data* ch);
int utility_irritable(struct char_data* ch, int cmd, const char* arg,
  int (*func)(struct char_data*, int, const char*));
void mobile_wander(struct char_data* ch);
int is_target_room_p(int room, int tgt_room);
void group_gain(struct char_data* ch, struct char_data* victim);
void change_alignment(struct char_data* ch, struct char_data* victim);
char getall(char* name, char* newname);
void Zwrite(FILE* fp, char cmd, int tf, int arg1, int arg2, int arg3,
  char* desc);
int ObjVnum(struct obj_data* o);
char* lower(char* s);
char recep_offer(struct char_data* ch, struct char_data* receptionist,
  struct obj_cost* cost);
struct char_data* char_holding(struct obj_data* obj);
int track(struct char_data* ch, struct char_data* vict);
const char* DescMoves(float a);
const char* ac_for_score(int a);
void name_from_drinkcon(struct obj_data* obj);
void name_to_drinkcon(struct obj_data* obj, int type);
void save_obj(struct char_data* ch, struct obj_cost* cost, int do_delete);
void obj_to_store(struct obj_data* obj, struct obj_file_u* st,
  struct char_data* ch, int do_delete);
void update_file(struct char_data* ch, struct obj_file_u* st, int save);
int ValidMove(struct char_data* ch, int cmd);
void raw_kill(struct char_data* ch);
void setKillerFlag(struct char_data* ch, struct char_data* victim);
int MoveOne(struct char_data* ch, int dir);
void root_hit(struct char_data* ch, struct char_data* victim, int type,
  int (*dam_func)(struct char_data*, struct char_data*, int, int));
int DamDetailsOk(struct char_data* ch, struct char_data* v, int dam, int type);
int DamageTrivia(struct char_data* ch, struct char_data* v, int dam, int type);
int DoDamage(struct char_data* ch, struct char_data* v, int dam, int type);
int DamageMessages(struct char_data* ch, struct char_data* v, int dam,
  int attacktype);
int DamageEpilog(struct char_data* ch, struct char_data* victim);
int HitOrMiss(struct char_data* ch, struct char_data* victim, int calc_thaco);
void sprinttype(int type, const char* const* names, char* result);
struct time_info_data mud_time_passed(time_t t2, time_t t1);

int exit_ok(struct room_direction_data*, struct room_data**);

/* Path finding types and functions */
struct hunting_data {
    char* name;
    struct char_data** victim;
};

using mob_in_room_pred_t = int (*)(int room, struct hunting_data* data);
using obj_on_ground_pred_t = int (*)(int room, char* data);
using is_target_room_pred_t = int (*)(int room, int target_room);

enum FindPathType {
  FIND_MOB_IN_ROOM,
  FIND_OBJECT_ON_GROUND,
  FIND_TARGET_ROOM,
};

union find_path_fn {
    mob_in_room_pred_t mob_in_room_fn;
    obj_on_ground_pred_t obj_on_ground_fn;
    is_target_room_pred_t is_target_room_fn;
};

union find_path_fn_data {
    struct hunting_data* data;
    const char* obj_name;
    int target_room;
};

struct find_path_data {
    enum FindPathType type;
    union find_path_fn fn;
    union find_path_fn_data fn_data;
};

int find_path(int in_room, const struct find_path_data* data, int depth,
  int in_zone);
