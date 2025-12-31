#pragma once

#include "bit_ops.h"
#include "character_flags.h"
#include "game_constants.h"
#include "limits.h"
#include "object_flags.h"
#include "room_flags.h"

/* Forward declarations */
int CAN_SEE(struct char_data* s, struct char_data* o);
struct room_data* real_roomp(int virtual);
char* fname(char* namelist);
int isname(const char* str, const char* namelist);

/* Player flag checks */
#define BANISHED(ch) (((ch)->specials.act) & (PLR_BANISHED))
#define IS_KILLER(ch) (((ch)->specials.act) & (PLR_KILLER))
#define IS_OUTLAW(ch) (((ch)->specials.act) & (PLR_OUTLAW))

/* Affect checks */
#define IS_AFFECTED(ch, skill) (IS_SET((ch)->specials.affected_by, (skill)))

/* room_data lighting checks */
#define IS_DARK(room) \
  (!real_roomp(room)->light && IS_SET(real_roomp(room)->room_flags, DARK))

#define IS_LIGHT(room) \
  (real_roomp(room)->light || !IS_SET(real_roomp(room)->room_flags, DARK))

#define RM_FLAGS(i) ((real_roomp(i)) ? real_roomp(i)->room_flags : 0)

/* Character level and class */
#define GET_LEVEL(ch, i) ((ch)->player.level[(i)])
#define GET_WIMPY(ch) ((ch)->wimpy)

#define GET_CLASS_TITLE(ch, class, lev)                            \
  ((ch)->player.sex                                                \
      ? (((ch)->player.sex == 1) ? titles[(class)][(lev)].title_m  \
                                 : titles[(class)][(lev)].title_f) \
      : titles[(class)][(lev)].title_m)

/* Requirement/quality descriptors */
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

/* Pronoun macros */
#define HSHR(ch) \
  ((ch)->player.sex ? (((ch)->player.sex == 1) ? "his" : "her") : "its")

#define HSSH(ch) \
  ((ch)->player.sex ? (((ch)->player.sex == 1) ? "he" : "she") : "it")

#define HMHR(ch) \
  ((ch)->player.sex ? (((ch)->player.sex == 1) ? "him" : "her") : "it")

/* Article macros */
#define ANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")
#define SANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")

/* NPC/PC checks */
#define IS_NPC(ch) (IS_SET((ch)->specials.act, ACT_ISNPC))
#define IS_PC(ch) (!IS_NPC((ch)) || IS_SET((ch)->specials.act, ACT_POLYSELF))
#define IS_MOB(ch) (IS_SET((ch)->specials.act, ACT_ISNPC) && ((ch)->nr > -1))

/* Position and condition */
#define GET_POS(ch) ((ch)->specials.position)
#define GET_COND(ch, i) ((ch)->specials.conditions[(i)])

/* Basic character info */
#define GET_NAME(ch) ((ch)->player.name)
#define GET_TITLE(ch) ((ch)->player.title)
#define GET_CLASS(ch) ((ch)->player.class)
#define GET_HOME(ch) ((ch)->player.hometown)
#define GET_AGE(ch) (age(ch).year)
#define GET_POINT(ch) ((ch)->point_roll)

/* Ability scores - current */
#define GET_STR(ch) ((ch)->tmpabilities.str)
#define GET_ADD(ch) ((ch)->tmpabilities.str_add)
#define GET_DEX(ch) ((ch)->tmpabilities.dex)
#define GET_INT(ch) ((ch)->tmpabilities.intel)
#define GET_WIS(ch) ((ch)->tmpabilities.wis)
#define GET_CON(ch) ((ch)->tmpabilities.con)

/* Ability scores - base/real */
#define GET_RDEX(ch) ((ch)->abilities.dex)
#define GET_RSTR(ch) ((ch)->abilities.str)
#define GET_RINT(ch) ((ch)->abilities.intel)
#define GET_RCON(ch) ((ch)->abilities.con)
#define GET_RWIS(ch) ((ch)->abilities.wis)

/* Strength apply index */
#define STRENGTH_APPLY_INDEX(ch)                             \
  (((GET_ADD(ch) == 0) || (GET_STR(ch) != 18)) ? GET_STR(ch) \
    : (GET_ADD(ch) <= 50)                                    \
      ? 26                                                   \
      : ((GET_ADD(ch) <= 75)                                 \
            ? 27                                             \
            : ((GET_ADD(ch) <= 90) ? 28 : ((GET_ADD(ch) <= 99) ? 29 : 30))))

/* Combat stats */
#define GET_AC(ch) ((ch)->points.armor)
#define GET_HIT(ch) ((ch)->points.hit)
#define GET_MOVE(ch) ((ch)->points.move)
#define GET_MANA(ch) ((ch)->points.mana)

static inline int GET_MAX_HIT(struct char_data* ch) { return (hit_limit(ch)); }
static inline int GET_MAX_MOVE(struct char_data* ch) {
  return (move_limit(ch));
}
static inline int GET_MAX_MANA(struct char_data* ch) {
  return (mana_limit(ch));
}

#define GET_PERC_HIT(ch) \
  (((float)((ch)->points.hit) / ((float)(hit_limit(ch)))) * 100)

/* Resources */
#define GET_GOLD(ch) ((ch)->points.gold)
#define GET_BANK(ch) ((ch)->points.bankgold)
#define GET_EXP(ch) ((ch)->points.exp)

/* Physical attributes */
#define GET_HEIGHT(ch) ((ch)->player.height)
#define GET_WEIGHT(ch) ((ch)->player.weight)
#define GET_SEX(ch) ((ch)->player.sex)
#define GET_RACE(ch) ((ch)->race)

/* Combat modifiers */
#define GET_HITROLL(ch) ((ch)->points.hitroll)
#define GET_DAMROLL(ch) ((ch)->points.damroll)

/* State checks */
#define AWAKE(ch) \
  (GET_POS(ch) > POSITION_SLEEPING && !IS_AFFECTED(ch, AFF_PARALYSIS))

#define WAIT_STATE(ch, cycle) (((ch)->desc) ? (ch)->desc->wait = (cycle) : 0)

/* Object visibility and access */
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

/* Carrying capacity */
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

/* Name/description visibility */
#define PERS(ch, vict)                                               \
  (CAN_SEE(vict, ch)                                                 \
      ? (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr) \
      : "someone")

#define OBJS(obj, vict) \
  (CAN_SEE_OBJ((vict), (obj)) ? (obj)->short_description : "something")

#define OBJN(obj, vict) \
  (CAN_SEE_OBJ((vict), (obj)) ? fname((obj)->name) : "something")

/* Environment checks */
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

/* Exit and movement */
#define EXIT(ch, door) (real_roomp((ch)->in_room)->dir_option[door])

#define CAN_GO(ch, door)                                    \
  (EXIT(ch, door) && real_roomp(EXIT(ch, door)->to_room) && \
    !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))

#define CAN_GO_HUMAN(ch, door)                              \
  (EXIT(ch, door) && real_roomp(EXIT(ch, door)->to_room) && \
    !IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))

/* Alignment */
#define GET_ALIGNMENT(ch) ((ch)->specials.alignment)
#define IS_GOOD(ch) (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch) (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))

/* Item type */
#define ITEM_TYPE(obj) ((int)(obj)->obj_flags.type_flag)
