/* ************************************************************************
 *  file: utils.h, Utility module.                         Part of DIKUMUD *
 *  Usage: Utility macros                                                  *
 ************************************************************************* */

#ifndef UTILS_H
#define UTILS_H

#define _GNU_SOURCE
#include <features.h>

#include <stdio.h>
#include <stdlib.h>

#include "limits.h"
#include "structs.h"

#if defined(DEBUG) && DEBUG
#define free(obj)                                \
  fprintf(stderr, "freeing %d\n", sizeof(*obj)); \
  free(obj)
#endif

int CAN_SEE(struct char_data* s, struct char_data* o);

#define TRUE 1

#define FALSE 0

#define LOWER(c) (((c) >= 'A' && (c) <= 'Z') ? ((c) + ('a' - 'A')) : (c))

#define UPPER(c) (((c) >= 'a' && (c) <= 'z') ? ((c) + ('A' - 'a')) : (c))

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r')

#define IF_STR(st) ((st) ? (st) : "\0")

#define CAP(st) (*(st) = UPPER(*(st)), st)

#define CREATE(result, type, number)                  \
  do {                                                \
    (result) = (type*)calloc((number), sizeof(type)); \
    if (!(result)) {                                  \
      perror("malloc failure");                       \
      abort();                                        \
    }                                                 \
  } while (0)

#define RECREATE(result, type, number)                         \
  do {                                                         \
    void* _temp;                                               \
    _temp = realloc((void*)(result), sizeof(type) * (number)); \
    if (!(_temp)) {                                            \
      perror("realloc failure");                               \
      abort();                                                 \
    }                                                          \
    (result) = (type*)_temp;                                   \
  } while (0)

#define IS_SET(flag, bit) ((flag) & (bit))

#define BANISHED(ch) (((ch)->specials.act) & (PLR_BANISHED))

#define IS_KILLER(ch) (((ch)->specials.act) & (PLR_KILLER))

#define IS_OUTLAW(ch) (((ch)->specials.act) & (PLR_OUTLAW))

#define SWITCH(a, b) \
  {                  \
    (a) ^= (b);      \
    (b) ^= (a);      \
    (a) ^= (b);      \
  }

#define IS_AFFECTED(ch, skill) (IS_SET((ch)->specials.affected_by, (skill)))

#define IS_DARK(room) \
  (!real_roomp(room)->light && IS_SET(real_roomp(room)->room_flags, DARK))

#define IS_LIGHT(room) \
  (real_roomp(room)->light || !IS_SET(real_roomp(room)->room_flags, DARK))

#define SET_BIT(var, bit) ((var) = (var) | (bit))

#define REMOVE_BIT(var, bit) ((var) = (var) & ~(bit))

#define RM_FLAGS(i) ((real_roomp(i)) ? real_roomp(i)->room_flags : 0)

#define GET_LEVEL(ch, i) ((ch)->player.level[(i)])

#define GET_WIMPY(ch) ((ch)->wimpy)

#define GET_CLASS_TITLE(ch, class, lev)                            \
  ((ch)->player.sex                                                \
      ? (((ch)->player.sex == 1) ? titles[(class)][(lev)].title_m  \
                                 : titles[(class)][(lev)].title_f) \
      : titles[(class)][(lev)].title_m)

#define GET_REQ(i)                                                        \
  ((i) < 2                                                                \
      ? "Awful"                                                           \
      : ((i) < 4                                                          \
            ? "Bad"                                                       \
            : ((i) < 7                                                    \
                  ? "Poor"                                                \
                  : ((i) < 10                                             \
                        ? "Average"                                       \
                        : ((i) < 14 ? "Fair"                              \
                                    : ((i) < 20 ? "Good"                  \
                                                : ((i) < 24 ? "Very good" \
                                                            : "Superb")))))))

#define GET_WEAPON_DAMAGE(i)                                              \
  ((i) < 2                                                                \
      ? "Very low damage"                                                 \
      : ((i) < 5 ? "Low damage"                                           \
                 : ((i) < 10 ? "Moderate damage"                          \
                             : ((i) < 15 ? "A good bit of damage"         \
                                         : ((i) < 20 ? "Very nice damage" \
                                                     : "Superb Damage")))))

#define HSHR(ch) \
  ((ch)->player.sex ? (((ch)->player.sex == 1) ? "his" : "her") : "its")

#define HSSH(ch) \
  ((ch)->player.sex ? (((ch)->player.sex == 1) ? "he" : "she") : "it")

#define HMHR(ch) \
  ((ch)->player.sex ? (((ch)->player.sex == 1) ? "him" : "her") : "it")

#define ANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")

#define SANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")

#define IS_NPC(ch) (IS_SET((ch)->specials.act, ACT_ISNPC))

#define IS_PC(ch) (!IS_NPC((ch)) || IS_SET((ch)->specials.act, ACT_POLYSELF))

#define IS_MOB(ch) (IS_SET((ch)->specials.act, ACT_ISNPC) && ((ch)->nr > -1))

#define GET_POS(ch) ((ch)->specials.position)

#define GET_COND(ch, i) ((ch)->specials.conditions[(i)])

#define GET_NAME(ch) ((ch)->player.name)

#define GET_TITLE(ch) ((ch)->player.title)

#define GET_CLASS(ch) ((ch)->player.class)

#define GET_HOME(ch) ((ch)->player.hometown)

#define GET_AGE(ch) (age(ch).year)

#define GET_POINT(ch) ((ch)->point_roll)

#define GET_STR(ch) ((ch)->tmpabilities.str)

#define GET_ADD(ch) ((ch)->tmpabilities.str_add)

#define GET_DEX(ch) ((ch)->tmpabilities.dex)

#define GET_RDEX(ch) ((ch)->abilities.dex)

#define GET_RSTR(ch) ((ch)->abilities.str)

#define GET_RINT(ch) ((ch)->abilities.intel)

#define GET_RCON(ch) ((ch)->abilities.con)

#define GET_RWIS(ch) ((ch)->abilities.wis)

#define GET_INT(ch) ((ch)->tmpabilities.intel)

#define GET_WIS(ch) ((ch)->tmpabilities.wis)

#define GET_CON(ch) ((ch)->tmpabilities.con)

#define STRENGTH_APPLY_INDEX(ch)                             \
  (((GET_ADD(ch) == 0) || (GET_STR(ch) != 18)) ? GET_STR(ch) \
    : (GET_ADD(ch) <= 50)                                    \
      ? 26                                                   \
      : ((GET_ADD(ch) <= 75)                                 \
            ? 27                                             \
            : ((GET_ADD(ch) <= 90) ? 28 : ((GET_ADD(ch) <= 99) ? 29 : 30))))

#define GET_AC(ch) ((ch)->points.armor)

#define GET_HIT(ch) ((ch)->points.hit)

static inline int GET_MAX_HIT(struct char_data* ch) { return (hit_limit(ch)); }

#define GET_PERC_HIT(ch) \
  (((float)((ch)->points.hit) / ((float)(hit_limit(ch)))) * 100)

#define GET_MOVE(ch) ((ch)->points.move)

static inline int GET_MAX_MOVE(struct char_data* ch) {
  return (move_limit(ch));
}

#define GET_MANA(ch) ((ch)->points.mana)

static inline int GET_MAX_MANA(struct char_data* ch) {
  return (mana_limit(ch));
}

#define GET_GOLD(ch) ((ch)->points.gold)

#define GET_BANK(ch) ((ch)->points.bankgold)

#define GET_EXP(ch) ((ch)->points.exp)

#define GET_HEIGHT(ch) ((ch)->player.height)

#define GET_WEIGHT(ch) ((ch)->player.weight)

#define GET_SEX(ch) ((ch)->player.sex)

#define GET_RACE(ch) ((ch)->race)

#define GET_HITROLL(ch) ((ch)->points.hitroll)

#define GET_DAMROLL(ch) ((ch)->points.damroll)

#define AWAKE(ch) \
  (GET_POS(ch) > POSITION_SLEEPING && !IS_AFFECTED(ch, AFF_PARALYSIS))

#define WAIT_STATE(ch, cycle) (((ch)->desc) ? (ch)->desc->wait = (cycle) : 0)

/* Object And Carry related macros */

#define CAN_SEE_OBJ(sub, obj)                                   \
  (((!IS_NPC(sub)) && (GetMaxLevel(sub) > LOW_IMMORTAL)) ||     \
    (((!IS_SET((obj)->obj_flags.extra_flags, ITEM_INVISIBLE) || \
        IS_AFFECTED((sub), AFF_DETECT_INVISIBLE)) &&            \
       !IS_AFFECTED((sub), AFF_BLIND)) &&                       \
      (IS_LIGHT((sub)->in_room))))

#define GET_ITEM_TYPE(obj) ((obj)->obj_flags.type_flag)

#define CAN_WEAR(obj, part) (IS_SET((obj)->obj_flags.wear_flags, part))

#define GET_OBJ_WEIGHT(obj) ((obj)->obj_flags.weight)

#define GET_OBJ_VOLUME(obj) ((obj)->obj_flags.volume)

#define CAN_CARRY_W(ch) (str_app[STRENGTH_APPLY_INDEX(ch)].carry_w)

#define CAN_CARRY_N(ch)                                           \
  (((ch)->equipment[WIELD] && (ch)->equipment[WEAR_SHIELD])       \
      ? (dex_app_skill[GET_DEX(ch)].volume) / 3                   \
      : (((ch)->equipment[WIELD] || (ch)->equipment[WEAR_SHIELD]) \
            ? (dex_app_skill[GET_DEX(ch)].volume) / 2             \
            : dex_app_skill[GET_DEX(ch)].volume))

#define IS_CARRYING_W(ch) ((ch)->specials.carry_weight)

#define IS_CARRYING_N(ch) ((ch)->specials.carry_items)

#define CAN_CARRY_OBJ(ch, obj)                                                \
  (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&          \
    ((IS_CARRYING_N(ch) +                                                     \
       (GET_OBJ_VOLUME(obj) / vol_mult[(obj)->obj_flags.material_points])) <= \
      CAN_CARRY_N(ch)))

#define CAN_GET_OBJ(ch, obj)                                   \
  (CAN_WEAR((obj), ITEM_TAKE) && CAN_CARRY_OBJ((ch), (obj)) && \
    CAN_SEE_OBJ((ch), (obj)))

#define IS_OBJ_STAT(obj, stat) (IS_SET((obj)->obj_flags.extra_flags, stat))

/* char name/short_desc(for mobs) or someone?  */

#define PERS(ch, vict)                                               \
  (CAN_SEE(vict, ch)                                                 \
      ? (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr) \
      : "someone")

#define OBJS(obj, vict) \
  (CAN_SEE_OBJ((vict), (obj)) ? (obj)->short_description : "something")

#define OBJN(obj, vict) \
  (CAN_SEE_OBJ((vict), (obj)) ? fname((obj)->name) : "something")

#define OUTSIDE(ch) (!IS_SET(real_roomp((ch)->in_room)->room_flags, INDOORS))

#define IS_IMMORTAL(ch) (!IS_NPC(ch) && (GetMaxLevel(ch) >= 52))

#define IS_POLICE(ch)                        \
  ((mob_index[(ch)->nr].virtual == 3060) ||  \
    (mob_index[(ch)->nr].virtual == 3069) || \
    (mob_index[(ch)->nr].virtual == 14) ||   \
    (mob_index[(ch)->nr].virtual == 15) ||   \
    (mob_index[(ch)->nr].virtual == 16) ||   \
    (mob_index[(ch)->nr].virtual == 17) ||   \
    (mob_index[(ch)->nr].virtual == 18) ||   \
    (mob_index[(ch)->nr].virtual == 19) ||   \
    (mob_index[(ch)->nr].virtual == 3067))

#define IS_CORPSE(obj) \
  (GET_ITEM_TYPE((obj)) == ITEM_CONTAINER && isname("corpse", (obj)->name))

#define EXIT(ch, door) (real_roomp((ch)->in_room)->dir_option[door])

int exit_ok(struct room_direction_data*, struct room_data**);

#define CAN_GO(ch, door)                                    \
  (EXIT(ch, door) && real_roomp(EXIT(ch, door)->to_room) && \
    !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))

#define CAN_GO_HUMAN(ch, door)                              \
  (EXIT(ch, door) && real_roomp(EXIT(ch, door)->to_room) && \
    !IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))

#define GET_ALIGNMENT(ch) ((ch)->specials.alignment)

#define IS_GOOD(ch) (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch) (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))

#define ITEM_TYPE(obj) ((int)(obj)->obj_flags.type_flag)

extern unsigned char moontype;

void vlog(const char* str);
void vlogf(const char* errorMsg, ...);

int number(int from, int to);

#define NUMBER(from, to)                                   \
  ({                                                       \
    __typeof__(from) _result;                              \
    if ((to) - (from) + 1)                                 \
      _result = ((rand() % ((to) - (from) + 1)) + (from)); \
    else                                                   \
      _result = (from);                                    \
    _result;                                               \
  })

int SkipImmortals(struct char_data* v, int amnt);
int PreProcDam(struct char_data* ch, int type, int dam);
int IsUndead(struct char_data* ch);
void update_pos(struct char_data* victim);
void die(struct char_data* ch);
void do_look(struct char_data* ch, char* argument, int cmd);
void death_cry(struct char_data* ch);
void zero_rent(struct char_data* ch);
int apply_soundproof(struct char_data* ch);
int check_soundproof(struct char_data* ch);
int HasHands(struct char_data* ch);
void RestoreChar(struct char_data* ch);
void RemAllAffects(struct char_data* ch);
void sprintbit(unsigned long vektor, const char* const* names, char* result);
int CAN_SEE_FOR_WHO(struct char_data* s, struct char_data* o);
void do_at(struct char_data* ch, char* argument, int cmd);
int start_page_file(struct descriptor_data* d, const char* fpath,
  const char* errormsg);
int str_cmp(const char* arg1, const char* arg2);
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
int GetApprox(int num, int perc);
int CalcThaco(struct char_data* ch);
void* Mymalloc(long size);
int dice(int number, int size);

#define DICE(number, size)               \
  ({                                     \
    __typeof__(number) _sum = 0;         \
    __typeof__(number) _r;               \
    assert((size) >= 0);                 \
    if ((size) == 0)                     \
      _sum;                              \
    else                                 \
      for (_r = 1; _r <= (number); _r++) \
        _sum += ((rand() % (size)) + 1); \
    _sum;                                \
  })

void SpaceForSkills(struct char_data* ch);
void error_log(const char* str);
void slog(const char* str);
void weather_and_time(int mode);
void another_hour(int mode);
void weather_change(void);
void GetMonth(int month);
void ChangeWeather(int change);
void night_watchman(void);
void SwitchStuff(struct char_data* giver, struct char_data* taker);
int ObjLevelCheck(struct obj_data* obj, struct char_data* ch);
void hit(struct char_data* ch, struct char_data* victim, int type);
struct char_data* FindVictim(struct char_data* ch);
struct char_data* FindMetaVictim(struct char_data* ch);
struct char_data* FindAnAttacker(struct char_data* ch);
int SameRace(struct char_data* ch1, struct char_data* ch2);
char in_group(struct char_data* ch1, struct char_data* ch2);
int dir_track(struct char_data* ch, struct char_data* vict);
struct char_data* FindAnyVictim(struct char_data* ch);
void mobile_activity(struct char_data* ch);
void MakeNoise(int room, const char* local_snd, const char* distant_snd);
void weight_change_object(struct obj_data* obj, int weight);
int damage(struct char_data* ch, struct char_data* victim, int damage,
  int weapontype);
struct time_info_data age(struct char_data* ch);
void set_fighting(struct char_data* ch, struct char_data* vict);
int WeaponCheck(struct char_data* ch, struct char_data* v, int type, int dam);
void add_follower(struct char_data* ch, struct char_data* leader);
int IsImmune(struct char_data* ch, int bit);
int WeaponImmune(struct char_data* ch);
int IsResist(struct char_data* ch, int bit);
int IsSusc(struct char_data* ch, int bit);
int go_direction(struct char_data* ch, int dir);
int choose_exit_in_zone(int in_room, int tgt_room, int depth);
int check_peaceful(struct char_data* ch, const char* msg);
void raw_open_door(struct char_data* ch, int dir);
int MobCountInRoom(struct char_data* list);
int DisplayMove(struct char_data* ch, int dir, int was_in, int total);
void open_door(struct char_data* ch, int dir);
bool circle_follow(struct char_data* ch, struct char_data* victim);
int RecGetObjRoom(struct obj_data* obj);
int MissileDamage(struct char_data* ch, struct char_data* victim, int dam,
  int attacktype);
void stop_follower(struct char_data* ch);
void do_start(struct char_data* ch);
int AntiGuildMaster(struct char_data* ch, int cmd, char* arg);
int getabunch(const char* name, char* newname);
int choose_exit_global(int in_room, int tgt_room, int depth);
void LearnFromMistake(struct char_data* ch, int sknum, int silent, int max);
int fighter(struct char_data* ch, int cmd, char* arg);
int RecCompObjNum(struct obj_data* o, int obj_num);
int find_door(struct char_data* ch, char* type, char* dir);
void ThrowChar(struct char_data* ch, struct char_data* v, int dir);
int SetVictFighting(struct char_data* ch, struct char_data* v);
int SetCharFighting(struct char_data* ch, struct char_data* v);
void CallForGuard(struct char_data* ch, struct char_data* vict, int lev,
  int area);
void SetHunting(struct char_data* ch, struct char_data* tch);
void make_corpse(struct char_data* ch);
int utility_irritable(struct char_data* ch, int cmd, char* arg,
  mob_proc_t func);
void mobile_wander(struct char_data* ch);
int is_target_room_p(int room, int tgt_room);
void group_gain(struct char_data* ch, struct char_data* victim);
void change_alignment(struct char_data* ch, struct char_data* victim);
char getall(char* name, char* newname);
void Zwrite(FILE* fp, char cmd, int tf, int arg1, int arg2, int arg3,
  char* desc);
int ObjVnum(struct obj_data* o);
char* lower(char* s);
bool recep_offer(struct char_data* ch, struct char_data* receptionist,
  struct obj_cost* cost);
struct char_data* char_holding(struct obj_data* obj);
int track(struct char_data* ch, struct char_data* vict);
const char* DescMoves(float a);
const char* ac_for_score(int a);
void name_from_drinkcon(struct obj_data* obj);
void name_to_drinkcon(struct obj_data* obj, int type);
void save_obj(struct char_data* ch, struct obj_cost* cost, int delete);
void obj_to_store(struct obj_data* obj, struct obj_file_u* st,
  struct char_data* ch, int delete);
void update_file(struct char_data* ch, struct obj_file_u* st, int save);
int ValidMove(struct char_data* ch, int cmd);
void raw_kill(struct char_data* ch);
void setKillerFlag(Mob* ch, Mob* victim);
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

struct hunting_data {
    char* name;
    struct char_data** victim;
};

typedef int (*mob_in_room_pred_t)(int room, struct hunting_data* data);
typedef int (*obj_on_ground_pred_t)(int room, char* data);
typedef int (*is_target_room_pred_t)(int room, int target_room);

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
    char* obj_name;
    int target_room;
};

struct find_path_data {
    enum FindPathType type;
    union find_path_fn fn;
    union find_path_fn_data fn_data;
};

int find_path(int in_room, struct find_path_data* data, int depth, int in_zone);

typedef struct {
    void* ptr;
    size_t count;
    size_t size;
} alloc_result;

static inline alloc_result alloc_or_die(size_t count, size_t size) {
  void* ptr = calloc(count, size);
  if (!ptr) {
    perror("allocation failure");
    abort();
  }
  return (alloc_result){.ptr = ptr, .count = count, .size = size};
}

#define create(type, count) (type*)alloc_or_die(count, sizeof(type)).ptr

#endif
