#pragma once

struct char_data;
struct obj_data;

/* Core spell functions */
char saves_spell(struct char_data* ch, short int save_type);
int can_do_verbal(struct char_data* ch);

/* Spell behavior functions */
void spell_dispel_magic(signed char level, struct char_data* ch,
  struct char_data* victim, struct obj_data* obj);
void spell_animate_dead(signed char level, struct char_data* ch,
  struct char_data* victim, struct obj_data* corpse);

/* Cast handlers - Transport/Movement */
void cast_teleport(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void spell_teleport(signed char level, struct char_data* ch,
  struct char_data* victim, struct obj_data* obj);
void spell_astral_walk(signed char level, struct char_data* ch,
  struct char_data* victim, struct obj_data* obj);
void spell_summon(signed char level, struct char_data* ch,
  struct char_data* victim, struct obj_data* obj);
void cast_astral_walk(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_summon(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_word_of_recall(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj);
void cast_portal(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_succor(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);

/* Cast handlers - Summoning */
void cast_charm_person(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_charm_monster(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj);
void cast_animate_dead(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_conjure_elemental(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj);
void cast_cacaodemon(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_clone(signed char level, struct char_data* ch, const char* arg,
  int si, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_mon_sum1(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_mon_sum2(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_mon_sum3(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_mon_sum4(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_mon_sum5(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_mon_sum6(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_mon_sum7(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_create_golem(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* victim, struct obj_data* tar_obj);
void cast_control_undead(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj);
void cast_poly_self(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);

/* Cast handlers - Damage spells */
void cast_magic_missile(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* victim,
  struct obj_data* tar_obj);
void cast_burning_hands(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj);
void cast_shocking_grasp(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* victim,
  struct obj_data* tar_obj);
void cast_chill_touch(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* victim, struct obj_data* tar_obj);
void cast_colour_spray(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* victim, struct obj_data* tar_obj);
void cast_lightning_bolt(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* victim,
  struct obj_data* tar_obj);
void cast_fireball(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_cone_of_cold(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* victim, struct obj_data* tar_obj);
void cast_acid_blast(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* victim, struct obj_data* tar_obj);
void cast_ice_storm(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* victim, struct obj_data* tar_obj);
void cast_meteor_swarm(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* victim, struct obj_data* tar_obj);
void cast_call_lightning(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* victim,
  struct obj_data* tar_obj);
void cast_earthquake(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_flamestrike(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* victim, struct obj_data* tar_obj);
void cast_disintegrate(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* victim, struct obj_data* tar_obj);

/* Cast handlers - Harm/debuff spells */
void cast_fear(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_blindness(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_curse(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_poison(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_sleep(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_paralyze(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_weakness(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_energy_drain(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* victim, struct obj_data* tar_obj);
void cast_dispel_evil(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_dispel_good(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_harm(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* victim, struct obj_data* tar_obj);
void cast_cause_light(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* victim, struct obj_data* tar_obj);
void cast_cause_serious(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* victim,
  struct obj_data* tar_obj);
void cast_cause_critic(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* victim, struct obj_data* tar_obj);
void cast_vampiric_touch(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* victim,
  struct obj_data* tar_obj);
void cast_life_leech(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_synostodweomer(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj);
void cast_silence(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_web(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);

/* Cast handlers - Healing spells */
void cast_cure_light(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_cure_serious(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_cure_critic(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_heal(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_full_heal(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_heal_spray(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_cure_blind(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_remove_curse(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_remove_poison(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj);
void cast_remove_paralysis(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj);
void cast_refresh(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_second_wind(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_resurrection(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);

/* Cast handlers - Buff spells */
void cast_armor(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_bless(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_strength(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_shield(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_stone_skin(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_sanctuary(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_fireshield(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_invisibility(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_flying(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_fly_group(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_water_breath(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_protection_from_evil(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj);
void cast_infravision(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);

/* Cast handlers - Detection spells */
void cast_detect_evil(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_detect_invisibility(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj);
void cast_detect_magic(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_detect_poison(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj);
void cast_sense_life(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_true_seeing(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_know_alignment(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj);
void cast_farlook(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_locate_object(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj);
void cast_identify(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_minor_track(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_major_track(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);

/* Cast handlers - Utility spells */
void cast_dispel_magic(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_dispel_invisible(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj);
void cast_enchant_weapon(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj);
void cast_knock(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_control_weather(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj);
void cast_ventriloquate(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj);
void cast_turn(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_well_of_knowledge(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj);
void cast_calm(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_mana(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_vitalize_mana(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj);
void cast_faerie_fire(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_faerie_fog(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);

/* Cast handlers - Creation spells */
void cast_create_food(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_create_water(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_light(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_cont_light(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);
void cast_minor_creation(signed char level, struct char_data* ch,
  const char* arg, int type, struct char_data* tar_ch,
  struct obj_data* tar_obj);
void cast_heroes_feast(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* tar_ch, struct obj_data* tar_obj);

void cast_geyser(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* victim, struct obj_data* tar_obj);
void cast_green_slime(signed char level, struct char_data* ch, const char* arg,
  int type, struct char_data* victim, struct obj_data* tar_obj);
