struct char_data;
struct obj_data;

struct char_data* FindMobInRoomWithFunction(int room,
  int (*func)(struct char_data*, int, const char*));
int breath_weapon(struct char_data* ch, struct char_data* target, int mana_cost,
  void (*func)(signed char, struct char_data*, int, struct char_data*));
char* how_good(int percent);

/* Used by spell_parser.c affect_update() for breath weapon effect ticks */
using bweapon_fn = void (*)(signed char, struct char_data*, const char*, int,
  struct char_data*, struct obj_data*);

/* Indexed by (spell_id - FIRST_BREATH_WEAPON) */
extern const bweapon_fn bweapons[];

void do_breath(struct char_data* ch, const char* argument, int cmd);

// Object Procs
extern int board(struct char_data* ch, int cmd, const char* arg,
  struct obj_data* me);
extern int nodrop(struct char_data* ch, int cmd, const char* arg,
  struct obj_data* me);
extern int soap(struct char_data* ch, int cmd, const char* arg,
  struct obj_data* me);
extern int vorpal(struct char_data* victim, int cmd, const char* arg,
  struct obj_data* me);
extern int jive_box(struct char_data* ch, int cmd, const char* arg,
  struct obj_data* me);
extern int warMaker(struct char_data* ch, int cmd, const char* arg,
  struct obj_data* o);
extern int orbOfDestruction(struct char_data* ch, int cmd, const char* arg,
  struct obj_data* o);

// room_data Procs
extern int dump(struct char_data* ch, int cmd, const char* arg);
extern int train_station(struct char_data* ch, int cmd, const char* arg);
extern int pet_shops(struct char_data* ch, int cmd, const char* arg);
extern int bank(struct char_data* ch, int cmd, const char* arg);
extern int House(struct char_data* ch, int cmd, const char* arg);
extern int mirror_room(struct char_data* ch, int cmd, const char* arg);
extern int Magic_Fountain(struct char_data* ch, int cmd, const char* arg);
extern int board_room_entrance(struct char_data* ch, int cmd, const char* arg);
extern int hospital_entrance(struct char_data* ch, int cmd, const char* arg);
extern int hospital(struct char_data* ch, int cmd, const char* arg);
extern int Fountain(struct char_data* ch, int cmd, const char* arg);
extern int Donation(struct char_data* ch, int cmd, const char* arg);
extern int monk_challenge_prep_room(struct char_data* ch, int cmd,
  const char* arg);
extern int monk_challenge_room(struct char_data* ch, int cmd, const char* arg);
extern int metahospital(struct char_data* ch, int cmd, const char* arg);
extern int no_order(struct char_data* ch, int cmd, const char* arg);
extern int mag_room(struct char_data* ch, int cmd, const char* arg);

// Mob Procs
extern int cityguard(struct char_data* ch, int cmd, const char* arg);
extern int craps_table_man(struct char_data* ch, int cmd, const char* arg);
extern int aunt_bee(struct char_data* ch, int cmd, const char* arg);
extern int sheriff(struct char_data* ch, int cmd, const char* arg);
extern int bow_shooter(struct char_data* ch, int cmd, const char* arg);
extern int magneto(struct char_data* ch, int cmd, const char* arg);
extern int ThalosGuildGuard(struct char_data* ch, int cmd, const char* arg);
extern int SultanGuard(struct char_data* ch, int cmd, const char* arg);
extern int NewThalosCitzen(struct char_data* ch, int cmd, const char* arg);
extern int NewThalosMayor(struct char_data* ch, int cmd, const char* arg);
extern int MordGuard(struct char_data* ch, int cmd, const char* arg);
extern int MordGuildGuard(struct char_data* ch, int cmd, const char* arg);
extern int CaravanGuildGuard(struct char_data* ch, int cmd, const char* arg);
extern int StatTeller(struct char_data* ch, int cmd, const char* arg);
extern int ThrowerMob(struct char_data* ch, int cmd, const char* arg);
extern int Demon(struct char_data* ch, int cmd, const char* arg);
extern int Devil(struct char_data* ch, int cmd, const char* arg);
extern int Inquisitor(struct char_data* ch, int cmd, const char* arg);
extern int temple_labrynth_liar(struct char_data* ch, int cmd, const char* arg);
extern int AbyssGateKeeper(struct char_data* ch, int cmd, const char* arg);
extern int postmaster(struct char_data* ch, int cmd, const char* arg);
extern int temple_labrynth_sentry(struct char_data* ch, int cmd,
  const char* arg);
extern int NudgeNudge(struct char_data* ch, int cmd, const char* arg);
extern int RustMonster(struct char_data* ch, int cmd, const char* arg);
extern int PaladinGuildGuard(struct char_data* ch, int cmd, const char* arg);
extern int tormentor(struct char_data* ch, int cmd, const char* arg);
extern int receptionist(struct char_data* ch, int cmd, const char* arg);
extern int receptionist_for_outlaws(struct char_data* ch, int cmd,
  const char* arg);
extern int MageGuildMaster(struct char_data* ch, int cmd, const char* arg);
extern int ThiefGuildMaster(struct char_data* ch, int cmd, const char* arg);
extern int ClericGuildMaster(struct char_data* ch, int cmd, const char* arg);
extern int WarriorGuildMaster(struct char_data* ch, int cmd, const char* arg);
extern int PaladinGuildMaster(struct char_data* ch, int cmd, const char* arg);
extern int RangerGuildMaster(struct char_data* ch, int cmd, const char* arg);
extern int guild_guard(struct char_data* ch, int cmd, const char* arg);
extern int puff(struct char_data* ch, int cmd, const char* arg);
extern int fido(struct char_data* ch, int cmd, const char* arg);
extern int janitor(struct char_data* ch, int cmd, const char* arg);
extern int mayor(struct char_data* ch, int cmd, const char* arg);
extern int eric_johnson(struct char_data* ch, int cmd, const char* arg);
extern int andy_wilcox(struct char_data* ch, int cmd, const char* arg);
extern int zombie_master(struct char_data* ch, int cmd, const char* arg);
extern int snake(struct char_data* ch, int cmd, const char* arg);
extern int thief(struct char_data* ch, int cmd, const char* arg);
extern int monk_master(struct char_data* ch, int cmd, const char* arg);
extern int monk(struct char_data* ch, int cmd, const char* arg);
extern int magic_user(struct char_data* ch, int cmd, const char* arg);
extern int magic_user2(struct char_data* ch, int cmd, const char* arg);
extern int cleric(struct char_data* ch, int cmd, const char* arg);
extern int ghoul(struct char_data* ch, int cmd, const char* arg);
extern int vampire(struct char_data* ch, int cmd, const char* arg);
extern int arch_vampire(struct char_data* ch, int cmd, const char* arg);
extern int wraith(struct char_data* ch, int cmd, const char* arg);
extern int shadow(struct char_data* ch, int cmd, const char* arg);
extern int geyser(struct char_data* ch, int cmd, const char* arg);
extern int green_slime(struct char_data* ch, int cmd, const char* arg);
extern int BreathWeapon(struct char_data* ch, int cmd, const char* arg);
extern int dragon(struct char_data* ch, int cmd, const char* arg);
extern int DracoLich(struct char_data* ch, int cmd, const char* arg);
extern int Drow(struct char_data* ch, int cmd, const char* arg);
extern int Leader(struct char_data* ch, int cmd, const char* arg);
extern int MidgaardCitizen(struct char_data* ch, int cmd, const char* arg);
extern int NewThalosCitizen(struct char_data* ch, int cmd, const char* arg);
extern int citizen(struct char_data* ch, int cmd, const char* arg);
extern int NewThalosGuildGuard(struct char_data* ch, int cmd, const char* arg);
extern int new_ninja_master(struct char_data* ch, int cmd, const char* arg);
extern int loremaster(struct char_data* ch, int cmd, const char* arg);
extern int hunter(struct char_data* ch, int cmd, const char* arg);
extern int WizardGuard(struct char_data* ch, int cmd, const char* arg);
extern int AbbarachDragon(struct char_data* ch, int cmd, const char* arg);
extern int Tytan(struct char_data* ch, int cmd, const char* arg);
extern int replicant(struct char_data* ch, int cmd, const char* arg);
extern int nightcrawler(struct char_data* ch, int cmd, const char* arg);
extern int regenerator(struct char_data* ch, int cmd, const char* arg);
extern int mega_regenerator(struct char_data* ch, int cmd, const char* arg);
extern int web_slinger(struct char_data* ch, int cmd, const char* arg);
extern int juggernaut(struct char_data* ch, int cmd, const char* arg);
extern int storm(struct char_data* ch, int cmd, const char* arg);
extern int prof_x(struct char_data* ch, int cmd, const char* arg);
extern int elektro(struct char_data* ch, int cmd, const char* arg);
extern int iceman(struct char_data* ch, int cmd, const char* arg);
extern int blink(struct char_data* ch, int cmd, const char* arg);
extern int RepairGuy(struct char_data* ch, int cmd, const char* arg);
extern int Ringwraith(struct char_data* ch, int cmd, const char* arg);
extern int bounty_hunter(struct char_data* ch, int cmd, const char* arg);
extern int sisyphus(struct char_data* ch, int cmd, const char* arg);
extern int jabberwocky(struct char_data* ch, int cmd, const char* arg);
extern int flame(struct char_data* ch, int cmd, const char* arg);
extern int banana(struct char_data* ch, int cmd, const char* arg);
extern int paramedics(struct char_data* ch, int cmd, const char* arg);
extern int delivery_elf(struct char_data* ch, int cmd, const char* arg);
extern int delivery_beast(struct char_data* ch, int cmd, const char* arg);
extern int Keftab(struct char_data* ch, int cmd, const char* arg);
extern int StormGiant(struct char_data* ch, int cmd, const char* arg);
extern int Kraken(struct char_data* ch, int cmd, const char* arg);
extern int Manticore(struct char_data* ch, int cmd, const char* arg);
extern int i_am_police(struct char_data* ch, int cmd, const char* arg);
extern int fighter(struct char_data* ch, int cmd, const char* arg);
extern int zombie_hater(struct char_data* ch, int cmd, const char* arg);
extern int toilet_thing(struct char_data* ch, int cmd, const char* arg);
extern int gilbert(struct char_data* ch, int cmd, const char* arg);
extern int bouncer(struct char_data* ch, int cmd, const char* arg);
extern int dishboy(struct char_data* ch, int cmd, const char* arg);
extern int game_wizard(struct char_data* ch, int cmd, const char* arg);
extern int AGGRESSIVE(struct char_data* ch, int cmd, const char* arg);
extern int CarrionCrawler(struct char_data* ch, int cmd, const char* arg);
extern int guardian(struct char_data* ch, int cmd, const char* arg);
extern int lattimore(struct char_data* ch, int cmd, const char* arg);
extern int coldcaster(struct char_data* ch, int cmd, const char* arg);
extern int trapper(struct char_data* ch, int cmd, const char* arg);
extern int keystone(struct char_data* ch, int cmd, const char* arg);
extern int ghostsoldier(struct char_data* ch, int cmd, const char* arg);
extern int troguard(struct char_data* ch, int cmd, const char* arg);
extern int shaman(struct char_data* ch, int cmd, const char* arg);
extern int golgar(struct char_data* ch, int cmd, const char* arg);
extern int trogcook(struct char_data* ch, int cmd, const char* arg);
