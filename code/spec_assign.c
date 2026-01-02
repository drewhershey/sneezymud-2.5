#include <stdio.h>

#include "board.h"
#include "db.h"
#include "game_constants.h"
#include "spec_procs.h"
#include "structs.h"
#include "utils.h"

struct special_proc_entry {
    int vnum;
    int (*proc)(struct char_data*, int, const char*);
};

/* ********************************************************************
 *  Assignments                                                        *
 ******************************************************************** */

static const struct special_proc_entry mob_specials[] = {
  {.vnum = 1, .proc = puff},
  {.vnum = 2, .proc = Ringwraith},
  {.vnum = 3, .proc = tormentor},
  {.vnum = 4, .proc = Inquisitor},
  {.vnum = 6, .proc = tormentor},
  {.vnum = 14, .proc = bounty_hunter},
  {.vnum = 15, .proc = bounty_hunter},
  {.vnum = 16, .proc = bounty_hunter},
  {.vnum = 17, .proc = bounty_hunter},
  {.vnum = 18, .proc = bounty_hunter},
  {.vnum = 19, .proc = bounty_hunter},
  {.vnum = 25, .proc = magic_user},
  {.vnum = 30, .proc = MageGuildMaster},
  {.vnum = 31, .proc = ClericGuildMaster},
  {.vnum = 32, .proc = ThiefGuildMaster},
  {.vnum = 33, .proc = WarriorGuildMaster},
  {.vnum = 34, .proc = MageGuildMaster},
  {.vnum = 35, .proc = ClericGuildMaster},
  {.vnum = 50, .proc = i_am_police},
  {.vnum = 51, .proc = i_am_police},
  {.vnum = 52, .proc = i_am_police},
  {.vnum = 53, .proc = i_am_police},
  {.vnum = 36, .proc = ThiefGuildMaster},
  {.vnum = 37, .proc = WarriorGuildMaster},
  {.vnum = 29902, .proc = AntiGuildMaster},
  {.vnum = 29904, .proc = RangerGuildMaster},
  {.vnum = 29905, .proc = PaladinGuildMaster},
  {.vnum = 199, .proc = AGGRESSIVE},
  {.vnum = 200, .proc = AGGRESSIVE},
  {.vnum = 1699, .proc = postmaster},
  /*
  **  D&D standard
  */

  {.vnum = 210, .proc = snake},        /* spider */
  {.vnum = 211, .proc = fighter},      /* gnoll  */
  {.vnum = 220, .proc = fighter},      /* fighter */
  {.vnum = 221, .proc = fighter},      /* bugbear */
  {.vnum = 223, .proc = ghoul},        /* ghoul */
  {.vnum = 226, .proc = fighter},      /* ogre */
  {.vnum = 236, .proc = ghoul},        /* ghast */
  {.vnum = 227, .proc = snake},        /* spider */
  {.vnum = 230, .proc = BreathWeapon}, /* baby black */
  {.vnum = 232, .proc = blink},        /* blink dog */
  {.vnum = 233, .proc = BreathWeapon}, /* baby blue */
  {.vnum = 234, .proc = cleric},       /* cleric */
  {.vnum = 239, .proc = shadow},       /* shadow    */
  {.vnum = 240, .proc = snake},        /* toad      */
  {.vnum = 243, .proc = BreathWeapon}, /* teenage white */
  {.vnum = 247, .proc = fighter},      /* minotaur */
  {.vnum = 251, .proc = CarrionCrawler},
  {.vnum = 261, .proc = fighter},
  {.vnum = 271, .proc = regenerator},
  {.vnum = 248, .proc = snake},      /* snake       */
  {.vnum = 249, .proc = snake},      /* snake       */
  {.vnum = 250, .proc = snake},      /* snake       */
  {.vnum = 257, .proc = magic_user}, /* magic_user  */

  {.vnum = 650, .proc = monk},
  {.vnum = 651, .proc = monk},
  {.vnum = 652, .proc = monk},
  {.vnum = 653, .proc = monk},
  {.vnum = 654, .proc = monk},
  {.vnum = 655, .proc = monk},
  {.vnum = 656, .proc = monk},
  {.vnum = 657, .proc = monk},
  {.vnum = 658, .proc = monk},
  {.vnum = 659, .proc = monk},
  {.vnum = 660, .proc = monk},
  {.vnum = 661, .proc = monk},
  {.vnum = 662, .proc = monk},
  {.vnum = 663, .proc = monk},
  {.vnum = 664, .proc = monk},
  {.vnum = 665, .proc = monk},
  {.vnum = 666, .proc = monk},
  {.vnum = 667, .proc = monk},
  {.vnum = 668, .proc = monk},
  {.vnum = 669, .proc = monk},
  {.vnum = 670, .proc = monk},
  {.vnum = 671, .proc = monk},
  {.vnum = 672, .proc = monk},
  {.vnum = 673, .proc = monk},
  {.vnum = 674, .proc = monk},
  {.vnum = 675, .proc = monk},
  {.vnum = 676, .proc = monk},
  {.vnum = 677, .proc = monk},
  {.vnum = 678, .proc = monk},
  {.vnum = 679, .proc = monk},
  {.vnum = 680, .proc = monk},
  {.vnum = 681, .proc = monk},
  {.vnum = 682, .proc = monk},
  {.vnum = 683, .proc = monk},
  {.vnum = 684, .proc = monk},
  {.vnum = 685, .proc = monk},
  {.vnum = 686, .proc = monk},
  {.vnum = 687, .proc = monk},
  {.vnum = 688, .proc = monk},
  {.vnum = 689, .proc = monk},
  {.vnum = 690, .proc = monk},
  {.vnum = 691, .proc = monk_master},

  /*
  **  Abyss part II
  */
  {.vnum = 25126, .proc = magic_user},   /* Vascar */
  {.vnum = 25127, .proc = cleric},       /* ralthar */
  {.vnum = 25128, .proc = fighter},      /*draco */
  {.vnum = 25131, .proc = Demon},        /*Balrog */
  {.vnum = 25134, .proc = BreathWeapon}, /*rainbow d. */
  {.vnum = 25147, .proc = fighter},
  {.vnum = 25148, .proc = magic_user},
  {.vnum = 25149, .proc = thief},
  {.vnum = 25150, .proc = cleric},
  {.vnum = 25154, .proc = snake}, /*wyvern*/
  {.vnum = 25156, .proc = janitor},
  {.vnum = 25158, .proc = magic_user},
  {.vnum = 25159, .proc = magic_user},
  {.vnum = 25160, .proc = magic_user},
  {.vnum = 25161, .proc = fighter},
  {.vnum = 25162, .proc = magic_user},
  {.vnum = 25164, .proc = cleric},
  {.vnum = 25165, .proc = cleric},
  {.vnum = 25166, .proc = magic_user},
  {.vnum = 25168, .proc = magic_user},
  /*
  **   shire
  */
  {.vnum = 1000, .proc = magic_user},
  {.vnum = 1010, .proc = fighter},
  {.vnum = 1011, .proc = fighter},
  {.vnum = 1012, .proc = fighter},
  {.vnum = 1014, .proc = fighter},
  {.vnum = 1015, .proc = fighter},
  {.vnum = 1016, .proc = fighter},
  {.vnum = 1017, .proc = fighter},
  {.vnum = 1001, .proc = fighter},
  {.vnum = 1023, .proc = fighter},
  {.vnum = 1031, .proc = receptionist},
  {.vnum = 1701, .proc = monk},

  {.vnum = 1702, .proc = monk},
  {.vnum = 1703, .proc = monk},
  {.vnum = 1704, .proc = monk},
  {.vnum = 1705, .proc = monk},
  {.vnum = 1706, .proc = monk},
  {.vnum = 1707, .proc = monk},
  {.vnum = 1708, .proc = monk},
  {.vnum = 1709, .proc = monk},
  {.vnum = 1710, .proc = monk},
  {.vnum = 1711, .proc = monk},
  {.vnum = 1712, .proc = monk},
  {.vnum = 1713, .proc = monk},
  {.vnum = 1714, .proc = monk},
  {.vnum = 1715, .proc = monk},
  {.vnum = 1716, .proc = monk},
  {.vnum = 1717, .proc = monk},
  {.vnum = 1718, .proc = monk},
  /*
  **  cacaodemons
      { 20, fighter},
      { 21, fighter},
      { 22, fighter},
      { 23, fighter},
  */

  /*
  **  G1
  */
  {.vnum = 9213, .proc = CarrionCrawler},
  {.vnum = 9208, .proc = cleric},
  {.vnum = 9217, .proc = BreathWeapon},
  /*
  **  chessboard
  */
  {.vnum = 1401, .proc = fighter},
  {.vnum = 1404, .proc = fighter},
  {.vnum = 1406, .proc = fighter},
  {.vnum = 1457, .proc = fighter},
  {.vnum = 1460, .proc = fighter},
  {.vnum = 1462, .proc = fighter},
  {.vnum = 1499, .proc = sisyphus},
  {.vnum = 1471, .proc = paramedics},
  {.vnum = 1470, .proc = jabberwocky},
  {.vnum = 1472, .proc = flame},
  {.vnum = 1437, .proc = banana},
  {.vnum = 1495, .proc = delivery_elf},
  {.vnum = 1493, .proc = delivery_beast},

  /*
  The Undead Temple
  */
  {.vnum = 28801, .proc = cleric},
  {.vnum = 28802, .proc = fighter},
  {.vnum = 28806, .proc = AGGRESSIVE},
  {.vnum = 28808, .proc = cleric},
  {.vnum = 28809, .proc = fighter},
  {.vnum = 28811, .proc = fighter},
  {.vnum = 28813, .proc = arch_vampire},

  /*
  Batopr
  */
  {.vnum = 28701, .proc = bouncer},
  {.vnum = 28705, .proc = magic_user},
  {.vnum = 28708, .proc = cleric},
  {.vnum = 28711, .proc = fighter},
  {.vnum = 28722, .proc = toilet_thing},
  {.vnum = 28724, .proc = dishboy},
  {.vnum = 28729, .proc = game_wizard},
  {.vnum = 28742, .proc = fighter},
  {.vnum = 28743, .proc = gilbert},
  {.vnum = 28744, .proc = fighter},
  {.vnum = 28745, .proc = magic_user},
  {.vnum = 28746, .proc = fighter},
  {.vnum = 28748, .proc = magic_user},
  {.vnum = 11335, .proc = fighter},
  {.vnum = 11340, .proc = magic_user},
  {.vnum = 3008, .proc = thief},
  {.vnum = 3009, .proc = thief},

  /*
  Evils area
  */

  {.vnum = 28401, .proc = web_slinger},
  {.vnum = 28402, .proc = magic_user},
  {.vnum = 28403, .proc = magic_user},
  {.vnum = 28404, .proc = snake},
  {.vnum = 28405, .proc = snake},
  {.vnum = 28448, .proc = dragon},
  {.vnum = 28449, .proc = AbbarachDragon},

  /*
  Marvel world
  */
  {.vnum = 29001, .proc = fighter},
  {.vnum = 29002, .proc = magic_user},
  {.vnum = 29003, .proc = mega_regenerator},
  {.vnum = 29004, .proc = web_slinger},
  {.vnum = 29005, .proc = magneto},
  {.vnum = 29006, .proc = fighter},
  {.vnum = 29008, .proc = sheriff},
  {.vnum = 29010, .proc = prof_x},
  {.vnum = 29011, .proc = magic_user},
  {.vnum = 29012, .proc = fighter},
  {.vnum = 29013, .proc = zombie_hater},
  {.vnum = 29014, .proc = storm},
  {.vnum = 29017, .proc = elektro},
  {.vnum = 29021, .proc = fighter},
  {.vnum = 29022, .proc = juggernaut},
  {.vnum = 29024, .proc = fighter},
  {.vnum = 29025, .proc = fighter},
  {.vnum = 29026, .proc = fighter},
  {.vnum = 29027, .proc = fighter},
  {.vnum = 29028, .proc = bow_shooter},
  {.vnum = 29029, .proc = fighter},
  {.vnum = 29030, .proc = fighter},
  {.vnum = 29031, .proc = iceman},
  {.vnum = 29033, .proc = nightcrawler},
  {.vnum = 29039, .proc = janitor},
  {.vnum = 29040, .proc = replicant},
  {.vnum = 29044, .proc = fido},
  {.vnum = 29954, .proc = dragon},

  /*
  **  New Thalos
  */
  {.vnum = 3600, .proc = MageGuildMaster},
  {.vnum = 3601, .proc = ClericGuildMaster},
  {.vnum = 3602, .proc = WarriorGuildMaster},
  {.vnum = 3603, .proc = ThiefGuildMaster},
  {.vnum = 3604, .proc = receptionist_for_outlaws},
#if 0
  { 3606, BattleOfEvermoreSinger },
#endif
  {.vnum = 3619, .proc = fighter},
  {.vnum = 3620, .proc = fighter},
  {.vnum = 3632, .proc = fighter},
  {.vnum = 3634, .proc = fighter},
  {.vnum = 3636, .proc = fighter},
  {.vnum = 3639, .proc = fighter},    /* caramon */
  {.vnum = 3641, .proc = cleric},     /* curley g. */
  {.vnum = 3640, .proc = magic_user}, /* raist */
  {.vnum = 3656, .proc = NewThalosGuildGuard},
  {.vnum = 3657, .proc = NewThalosGuildGuard},
  {.vnum = 3658, .proc = NewThalosGuildGuard},
  {.vnum = 3659, .proc = NewThalosGuildGuard},
  {.vnum = 3661, .proc = SultanGuard},    /* wandering */
  {.vnum = 3662, .proc = SultanGuard},    /* not */
  {.vnum = 3682, .proc = SultanGuard},    /* royal */
  {.vnum = 3670, .proc = BreathWeapon},   /* Cryohydra */
  {.vnum = 3674, .proc = BreathWeapon},   /* Behir */
  {.vnum = 3675, .proc = BreathWeapon},   /* Chimera */
  {.vnum = 3676, .proc = BreathWeapon},   /* Couatl */
  {.vnum = 3681, .proc = cleric},         /* High priest */
  {.vnum = 3689, .proc = NewThalosMayor}, /* Guess */
  {.vnum = 3644, .proc = fido},
  {.vnum = 3635, .proc = thief},
  /*
  **  Skexie
  */
  {.vnum = 15813, .proc = magic_user},
  {.vnum = 15815, .proc = magic_user},
  {.vnum = 15820, .proc = magic_user},
  {.vnum = 15821, .proc = vampire},
  {.vnum = 15844, .proc = cleric},
  {.vnum = 15847, .proc = fighter},
  {.vnum = 15831, .proc = fighter},
  {.vnum = 15832, .proc = fighter},
  {.vnum = 15822, .proc = fighter},
  {.vnum = 15819, .proc = fighter},
  {.vnum = 15805, .proc = fighter},
  /*
  **  Challenge
  */
  {.vnum = 15858, .proc = BreathWeapon},
  {.vnum = 15861, .proc = magic_user},
  {.vnum = 15862, .proc = magic_user},
  {.vnum = 15863, .proc = fighter},
  {.vnum = 15864, .proc = sisyphus},

  /*
  **   Zombie's
  */

  {.vnum = 23001, .proc = fighter}, /* Bob */

  /*
  **  abyss
  */
  {.vnum = 25000, .proc = magic_user}, /* Demi-lich  */
  {.vnum = 25001, .proc = Keftab},
  {.vnum = 25009, .proc = BreathWeapon},    /* hydra */
  {.vnum = 25002, .proc = vampire},         /* Crimson */
  {.vnum = 25003, .proc = StormGiant},      /* MistDaemon */
  {.vnum = 25006, .proc = StormGiant},      /* Storm giant */
  {.vnum = 25014, .proc = StormGiant},      /* DeathKnight */
  {.vnum = 25009, .proc = BreathWeapon},    /* hydra */
  {.vnum = 25017, .proc = AbyssGateKeeper}, /* Abyss Gate Keeper */
  {.vnum = 25013, .proc = fighter},         /* kalas */

  /*
  **  Paladin's guild
  */
  {.vnum = 25100, .proc = PaladinGuildGuard},
  {.vnum = 25101, .proc = PaladinGuildGuard},

  /*
  **  Abyss Fire Giants
  */
  {.vnum = 25500, .proc = fighter},
  {.vnum = 25501, .proc = fighter},
  {.vnum = 25502, .proc = fighter},
  {.vnum = 25505, .proc = fighter},
  {.vnum = 25504, .proc = BreathWeapon},
  {.vnum = 25503, .proc = cleric},

  /*
  **  Temple Labrynth
  */

  {.vnum = 10900, .proc = temple_labrynth_liar},
  {.vnum = 10901, .proc = temple_labrynth_liar},
  {.vnum = 10902, .proc = temple_labrynth_sentry},
  /*
  **  Gypsy Village
  */

  {.vnum = 16106, .proc = fido},
  {.vnum = 16107, .proc = CaravanGuildGuard},
  {.vnum = 16108, .proc = CaravanGuildGuard},
  {.vnum = 16109, .proc = CaravanGuildGuard},
  {.vnum = 16110, .proc = CaravanGuildGuard},
  {.vnum = 16111, .proc = WarriorGuildMaster},
  {.vnum = 16112, .proc = MageGuildMaster},
  {.vnum = 16113, .proc = ThiefGuildMaster},
  {.vnum = 16114, .proc = ClericGuildMaster},
  {.vnum = 16122, .proc = receptionist},
  {.vnum = 16105, .proc = StatTeller},

  /*
  **  Draagdim
  */

  {.vnum = 2500, .proc = NudgeNudge}, /* jailer */
  /*
  **  mordilnia
  */
  {.vnum = 18200, .proc = magic_user},
  {.vnum = 18205, .proc = receptionist},
  {.vnum = 18206, .proc = MageGuildMaster},
  {.vnum = 18207, .proc = ClericGuildMaster},
  {.vnum = 18208, .proc = ThiefGuildMaster},
  {.vnum = 18209, .proc = WarriorGuildMaster},
  {.vnum = 18210, .proc = MordGuildGuard}, /*18266 3*/
  {.vnum = 18211, .proc = MordGuildGuard}, /*18276 1*/
  {.vnum = 18212, .proc = MordGuildGuard}, /*18272 0*/
  {.vnum = 18213, .proc = MordGuildGuard}, /*18256 2*/
  {.vnum = 18215, .proc = MordGuard},
  {.vnum = 18216, .proc = janitor},
  {.vnum = 18217, .proc = fido},
  {.vnum = 18221, .proc = fighter},
  {.vnum = 18222, .proc = MordGuard},
  {.vnum = 18223, .proc = MordGuard},

  /*
  **  Graecia:
  */
  {.vnum = 13779, .proc = magic_user},
  {.vnum = 13784, .proc = magic_user},
  {.vnum = 13785, .proc = magic_user},
  {.vnum = 13787, .proc = magic_user},
  {.vnum = 13789, .proc = magic_user},
  {.vnum = 13791, .proc = magic_user},
  {.vnum = 13793, .proc = magic_user},
  {.vnum = 13795, .proc = magic_user},
  {.vnum = 13797, .proc = magic_user},

#if 0
/*
**  Eastern Path
*/
  {160, },
  {160, },
  {160, },
  {160, },
  {160, },
  {160, },
  {160, },
  {160, },
  {160, },
  {160, },
/*
**   Ravenloft
*/
  {161, },
  {161, },
  {161, },
  {161, },
  {161, },
  {161, },
  {161, },
  {161, },
  {161, },
  {161, },
  {161, },
#endif
  /*
  **  Bay Isle
  */
  {.vnum = 16610, .proc = Demon},
  {.vnum = 16620, .proc = BreathWeapon},
  {.vnum = 16640, .proc = cleric},
  {.vnum = 16650, .proc = cleric},

#if 0
  {16630, PortalGuard_X},
#endif

  /*
  **  King's Mountain
  */
  {.vnum = 16700, .proc = BreathWeapon},
  {.vnum = 16702, .proc = shadow},
  {.vnum = 16703, .proc = magic_user},
  {.vnum = 16709, .proc = vampire},
  {.vnum = 16710, .proc = Devil},
  {.vnum = 16711, .proc = Devil},
  {.vnum = 16712, .proc = Devil},
  {.vnum = 16713, .proc = ghoul},
  {.vnum = 16714, .proc = ghoul},
  {.vnum = 16715, .proc = wraith},
  {.vnum = 16717, .proc = fighter},
  {.vnum = 16720, .proc = Devil},
  {.vnum = 16721, .proc = Devil},
  {.vnum = 16724, .proc = Devil},
  {.vnum = 16725, .proc = magic_user},
  {.vnum = 16726, .proc = cleric},
  {.vnum = 16727, .proc = Devil},
  {.vnum = 16728, .proc = Devil},
  {.vnum = 16730, .proc = Devil},
  {.vnum = 16731, .proc = Devil},
  {.vnum = 16732, .proc = Demon},
  {.vnum = 16733, .proc = Demon},
  {.vnum = 16734, .proc = Demon},
  {.vnum = 16735, .proc = Demon},
  {.vnum = 16736, .proc = cleric},
  {.vnum = 16738, .proc = BreathWeapon},
  /*
  **  Mages Tower
  */
  {.vnum = 1500, .proc = shadow},
  {.vnum = 1504, .proc = magic_user},
  {.vnum = 1506, .proc = magic_user},
  {.vnum = 1507, .proc = magic_user},
  {.vnum = 1508, .proc = magic_user},
  {.vnum = 1510, .proc = magic_user},
  {.vnum = 1514, .proc = magic_user},
  {.vnum = 1515, .proc = magic_user},
  {.vnum = 1516, .proc = magic_user},
  {.vnum = 1517, .proc = magic_user},
  {.vnum = 1518, .proc = magic_user},
  {.vnum = 1520, .proc = magic_user},
  {.vnum = 1521, .proc = magic_user},
  {.vnum = 1522, .proc = magic_user},
  {.vnum = 1523, .proc = magic_user},
  {.vnum = 1524, .proc = magic_user},
  {.vnum = 1525, .proc = magic_user},
  {.vnum = 1526, .proc = magic_user},
  {.vnum = 1527, .proc = magic_user},
  {.vnum = 1528, .proc = magic_user},
  {.vnum = 1529, .proc = magic_user},
  {.vnum = 1530, .proc = magic_user},
  {.vnum = 1531, .proc = magic_user},
  {.vnum = 1532, .proc = magic_user},
  {.vnum = 1533, .proc = magic_user},
  {.vnum = 1534, .proc = magic_user},
  {.vnum = 1537, .proc = magic_user},
  {.vnum = 1538, .proc = magic_user},
  {.vnum = 1540, .proc = magic_user},
  {.vnum = 1541, .proc = magic_user},
  {.vnum = 1548, .proc = magic_user},
  {.vnum = 1549, .proc = magic_user},
  {.vnum = 1552, .proc = magic_user},
  {.vnum = 1553, .proc = magic_user},
  {.vnum = 1554, .proc = magic_user},
  {.vnum = 1556, .proc = magic_user},
  {.vnum = 1557, .proc = magic_user},
  {.vnum = 1559, .proc = magic_user},
  {.vnum = 1560, .proc = magic_user},
  {.vnum = 1562, .proc = magic_user},
  {.vnum = 1564, .proc = magic_user},
  {.vnum = 1565, .proc = magic_user},
  /*
  **  Marvel World
  */

  /*
  **  Forest of Rhowyn
  */

  {.vnum = 13901, .proc = ThrowerMob},

#if 0
/*
**  Quikland
*/
  {62, },
  {62, },
  {62, },
  {62, },
  {62, },
  {62, },
  {62, },
  {62, },
  {62, },
  {62, },

/*
**  Lycanthropia
*/
  {169, },
  {169, },
  {169, },
  {169, },
  {169, },
  {169, },
  {169, },
  {169, },
  {169, },
  {169, },
#endif
  /*
  **  Main City
  */

  {.vnum = 29898, .proc = craps_table_man},
  {.vnum = 29901, .proc = aunt_bee},
  {.vnum = 29899, .proc = sheriff},
  {.vnum = 3000, .proc = magic_user},
  {.vnum = 3060, .proc = cityguard},
  {.vnum = 3067, .proc = cityguard},
  {.vnum = 3061, .proc = janitor},
  {.vnum = 3063, .proc = fighter},
  {.vnum = 3062, .proc = fido},
  {.vnum = 3066, .proc = fido},
  {.vnum = 3005, .proc = receptionist},
  {.vnum = 3020, .proc = MageGuildMaster},
  {.vnum = 3021, .proc = ClericGuildMaster},
  {.vnum = 3022, .proc = ThiefGuildMaster},
  {.vnum = 3023, .proc = WarriorGuildMaster},

  {.vnum = 3007, .proc = MidgaardCitizen}, /* Sailor */
  {.vnum = 3024, .proc = guild_guard},
  {.vnum = 3025, .proc = guild_guard},
  {.vnum = 3026, .proc = guild_guard},
  {.vnum = 3027, .proc = guild_guard},
  {.vnum = 29950, .proc = guild_guard},
  {.vnum = 29951, .proc = guild_guard},
  {.vnum = 29952, .proc = guild_guard},
  {.vnum = 29953, .proc = guild_guard},
  {.vnum = 3070, .proc = RepairGuy},
  {.vnum = 3071, .proc = RepairGuy},
  {.vnum = 3069, .proc = cityguard}, /* post guard */
  {.vnum = 3068, .proc = new_ninja_master},
  {.vnum = 3073, .proc = loremaster},
  {.vnum = 3074, .proc = hunter},

  /*
  **  Lower city
  */
  {.vnum = 3143, .proc = mayor},
  /*
  **   Hammor's Stuff
  */
  {.vnum = 3900, .proc = eric_johnson},
  {.vnum = 3901, .proc = andy_wilcox},
  {.vnum = 3950, .proc = zombie_master},
  {.vnum = 3952, .proc = BreathWeapon},

  /*
  **  MORIA
  */
  {.vnum = 4000, .proc = snake},
  {.vnum = 4001, .proc = snake},
  {.vnum = 4053, .proc = snake},

  {.vnum = 4103, .proc = thief},
  {.vnum = 4100, .proc = magic_user},
  {.vnum = 4101, .proc = regenerator},
  {.vnum = 4102, .proc = snake},

  /*
  **  Pyramid
  */

  {.vnum = 5308, .proc = RustMonster},
  {.vnum = 5303, .proc = vampire},

  /*
  **  Arctica
  */
  {.vnum = 6801, .proc = BreathWeapon},
  {.vnum = 6802, .proc = BreathWeapon},
  {.vnum = 6815, .proc = magic_user},
  {.vnum = 6821, .proc = snake},
  {.vnum = 6824, .proc = BreathWeapon},
  {.vnum = 6825, .proc = thief},

  /*
  ** SEWERS
  */
  {.vnum = 7009, .proc = fighter},
  {.vnum = 7006, .proc = snake},
  {.vnum = 7008, .proc = snake},
  {.vnum = 7042, .proc = magic_user},   /* naga       */
  {.vnum = 7040, .proc = BreathWeapon}, /* Red    */
  {.vnum = 7041, .proc = magic_user},   /* sea hag    */
  {.vnum = 7200, .proc = magic_user},   /* mindflayer */
  {.vnum = 7201, .proc = magic_user},   /* senior     */
  {.vnum = 7202, .proc = magic_user},   /* junior     */

  /*
  ** FOREST
  */

  {.vnum = 6111, .proc = magic_user}, /* tree */
  {.vnum = 6113, .proc = snake},
  {.vnum = 6114, .proc = snake},
  {.vnum = 6112, .proc = BreathWeapon}, /* green */
  {.vnum = 6910, .proc = magic_user},

  /*
  **  Great Eastern Desert
  */
  {.vnum = 5000, .proc = thief},        /* rag. dervish */
  {.vnum = 5002, .proc = snake},        /* coral snake */
  {.vnum = 5003, .proc = snake},        /* scorpion    */
  {.vnum = 5004, .proc = snake},        /* purple worm  */
  {.vnum = 5014, .proc = cleric},       /* myconoid */
  {.vnum = 5005, .proc = BreathWeapon}, /* brass */

  /*
  **  Drow (edition 1)
  */
  {.vnum = 5010, .proc = magic_user}, /* dracolich */
  {.vnum = 5104, .proc = cleric},
  {.vnum = 5103, .proc = magic_user}, /* drow mage */
  {.vnum = 5107, .proc = cleric},     /* drow mat. mot */
  {.vnum = 5108, .proc = magic_user}, /* drow mat. mot */
  {.vnum = 5109, .proc = cleric},     /* yochlol */

  /*
  **   Thalos
  */
  {.vnum = 5200, .proc = magic_user}, /* beholder    */

  /*
  **  Zoo
  */
  {.vnum = 9021, .proc = snake}, /* Gila Monster */

  /*
  **   Gonge area
  */
  {.vnum = 23012, .proc = magic_user},
  {.vnum = 23013, .proc = cleric},
  {.vnum = 23014, .proc = cleric},
  {.vnum = 23016, .proc = magic_user},
  {.vnum = 23017, .proc = magic_user},
  {.vnum = 23018, .proc = magic_user},
  /*
  **  Castle Python
  */

  {.vnum = 11016, .proc = receptionist},
  {.vnum = 11017, .proc = NudgeNudge},

  /*
  **  miscellaneous
  */
  {.vnum = 9061, .proc = vampire}, /* vampiress  */

  /*
  **  White Plume Mountain
  */

  {.vnum = 17004, .proc = magic_user},  /* gnyosphinx   */
  {.vnum = 17017, .proc = magic_user},  /* ogre magi   */
  {.vnum = 17014, .proc = ghoul},       /* ghoul  */
  {.vnum = 17009, .proc = geyser},      /* geyser  */
  {.vnum = 17011, .proc = vampire},     /* vampire Amelia  */
  {.vnum = 17002, .proc = wraith},      /* wight*/
  {.vnum = 17005, .proc = shadow},      /* shadow */
  {.vnum = 17010, .proc = green_slime}, /* green slime */

  /*
  **  Arachnos
  */
  {.vnum = 20001, .proc = snake},      /* Young (large) spider */
  {.vnum = 20003, .proc = snake},      /* wolf (giant) spider  */
  {.vnum = 20005, .proc = snake},      /* queen wasp      */
  {.vnum = 20006, .proc = snake},      /* drone spider    */
  {.vnum = 20010, .proc = snake},      /* bird spider     */
  {.vnum = 20009, .proc = magic_user}, /* quasit         */
  {.vnum = 20014, .proc = magic_user}, /* Arachnos        */
  {.vnum = 20015, .proc = magic_user}, /* Ki Rin          */

  {.vnum = 20002, .proc = BreathWeapon}, /* Yevaud */
  {.vnum = 20017, .proc = BreathWeapon}, /* Elder  */
  {.vnum = 20016, .proc = BreathWeapon}, /* Baby   */

#if 0
/*
**   The Darklands
*/

  { 24050, cleric },
  { 24052, magic_user2 },
  { 24053, magic_user2 },
  { 24054, magic_user2 },
  { 24055, magic_user2 },
  { 24056, magic_user2 },
  { 24057, magic_user2 },
  { 24058, magic_user2 },
  { 24059, magic_user2 },
#endif

  /*
  **   Abbarach
  */
  {.vnum = 27001, .proc = magic_user},
  {.vnum = 27002, .proc = magic_user},
  {.vnum = 27003, .proc = magic_user},
  {.vnum = 27004, .proc = magic_user},
  {.vnum = 27005, .proc = magic_user},
  {.vnum = 27006, .proc = Tytan},
  {.vnum = 27007, .proc = replicant},
  {.vnum = 27016, .proc = BreathWeapon},
  {.vnum = 27014, .proc = magic_user},
  {.vnum = 27017, .proc = magic_user},
  {.vnum = 27018, .proc = magic_user},
  {.vnum = 27019, .proc = magic_user},
};

static const size_t num_mob_specials =
  sizeof(mob_specials) / sizeof(mob_specials[0]);

/* assign special procedures to mobiles */
void assign_mobiles() {
  char buf[MAX_STRING_LENGTH];

  for (auto mob_special : mob_specials) {
    const int rnum = real_mobile(mob_special.vnum);

    if (rnum < 0) {
      sprintf(buf, "mobile_assign: Mobile %d not found in database.",
        mob_special.vnum);
      vlog(buf);
    } else {
      mob_index[rnum].func.mob_f = mob_special.proc;
    }
  }

  boot_the_shops();
  assign_the_shopkeepers();
}

struct obj_proc_entry {
    int vnum;
    int (*proc)(struct char_data*, int, const char*, struct obj_data*);
};

static const struct obj_proc_entry obj_specials[] = {
  {.vnum = 3095, .proc = board},
  {.vnum = 3097, .proc = board},
  {.vnum = 3098, .proc = board},
  {.vnum = 3099, .proc = board},
  {.vnum = 25102, .proc = board},
  {.vnum = 29992, .proc = jive_box},
  {.vnum = 21122, .proc = nodrop},
  {.vnum = 21130, .proc = soap},
  {.vnum = 7215, .proc = warMaker},
  {.vnum = 16754, .proc = orbOfDestruction},
};

static const size_t num_obj_specials =
  sizeof(obj_specials) / sizeof(obj_specials[0]);

/* assign special procedures to objects */
void assign_objects() {
  for (auto obj_special : obj_specials) {
    const int rnum = real_object(obj_special.vnum);

    if (rnum < 0) {
      vlogf("object_assign: Object %d not found in database.",
        obj_special.vnum);
    } else {
      obj_index[rnum].func.obj_f = obj_special.proc;
    }
  }

  InitBoards();
}

static const struct special_proc_entry room_specials[] = {
  {.vnum = 99, .proc = Donation},
  {.vnum = 666, .proc = dump},
  {.vnum = 1750, .proc = monk_challenge_prep_room},
  {.vnum = 1751, .proc = monk_challenge_room},
  {.vnum = 3030, .proc = dump},
  {.vnum = 3196, .proc = hospital},
  {.vnum = 3197, .proc = hospital_entrance},
  {.vnum = 13518, .proc = Fountain},
  {.vnum = 13547, .proc = dump},
  {.vnum = 28283, .proc = mag_room},
  {.vnum = 11014, .proc = Fountain},
  {.vnum = 23067, .proc = Fountain},
  {.vnum = 5234, .proc = Fountain},
  {.vnum = 3141, .proc = Fountain},
  {.vnum = 3606, .proc = Fountain},
  {.vnum = 3014, .proc = Fountain},
  {.vnum = 13530, .proc = pet_shops},
  {.vnum = 2999, .proc = board_room_entrance},
  {.vnum = 100, .proc = mirror_room},
  {.vnum = 11301, .proc = mirror_room},
  {.vnum = 18999, .proc = train_station},
  {.vnum = 8600, .proc = train_station},
  {.vnum = 27835, .proc = House},
  {.vnum = 27836, .proc = House},
  {.vnum = 27845, .proc = House},
  {.vnum = 29991, .proc = House},
  {.vnum = 27910, .proc = House},
  {.vnum = 27915, .proc = House},
  {.vnum = 27920, .proc = House},
  {.vnum = 27921, .proc = House},
  {.vnum = 27923, .proc = House},
  {.vnum = 27925, .proc = House},
  {.vnum = 27930, .proc = House},
  {.vnum = 27935, .proc = House},
  {.vnum = 27940, .proc = House},
  {.vnum = 27985, .proc = House},
  {.vnum = 27945, .proc = House},
  {.vnum = 27950, .proc = House},
  {.vnum = 27955, .proc = House},
  {.vnum = 27956, .proc = House},
  {.vnum = 27960, .proc = House},
  {.vnum = 27965, .proc = House},
  {.vnum = 27970, .proc = House},
  {.vnum = 27975, .proc = House},
  {.vnum = 27980, .proc = House},
  {.vnum = 27990, .proc = House},
  {.vnum = 29993, .proc = House},
  {.vnum = 29992, .proc = House},
  {.vnum = 2000, .proc = bank},
  {.vnum = 13521, .proc = bank},
  {.vnum = 3199, .proc = metahospital},
};

static const size_t num_room_specials =
  sizeof(room_specials) / sizeof(room_specials[0]);

/* assign special procedures to rooms */
void assign_rooms() {
  for (auto room_special : room_specials) {
    struct room_data* rp = real_roomp(room_special.vnum);

    if (rp != nullptr) {
      rp->funct = room_special.proc;
    } else {
      vlog("assign_rooms: unknown room");
    }
  }
}
