#pragma once

struct char_data;

/* Movement commands */
void do_move(struct char_data* ch, const char* argument, int cmd);
void do_enter(struct char_data* ch, const char* arg, int cmd);
void do_leave(struct char_data* ch, const char* arg, int cmd);
void do_exits(struct char_data* ch, const char* arg, int cmd);
void do_flee(struct char_data* ch, const char* arg, int cmd);

/* Position commands */
void do_stand(struct char_data* ch, const char* argument, int cmd);
void do_sit(struct char_data* ch, const char* argument, int cmd);
void do_rest(struct char_data* ch, const char* argument, int cmd);
void do_sleep(struct char_data* ch, const char* argument, int cmd);
void do_wake(struct char_data* ch, const char* argument, int cmd);

/* Information commands */
void do_look(struct char_data* ch, const char* argument, int cmd);
void do_read(struct char_data* ch, const char* argument, int cmd);
void do_examine(struct char_data* ch, const char* arg, int cmd);
void do_glance(struct char_data* ch, const char* arg, int cmd);
void do_peek(struct char_data* ch, const char* arg, int cmd);
void do_score(struct char_data* ch, const char* argument, int cmd);
void do_inventory(struct char_data* ch, const char* argument, int cmd);
void do_equipment(struct char_data* ch, const char* argument, int cmd);
void do_who(struct char_data* ch, const char* argument, int cmd);
void do_whozone(struct char_data* ch, const char* argument, int cmd);
void do_where(struct char_data* ch, const char* arg, int cmd);
void do_time(struct char_data* ch, const char* arg, int cmd);
void do_weather(struct char_data* ch, const char* arg, int cmd);
void do_levels(struct char_data* ch, const char* arg, int cmd);
void do_consider(struct char_data* ch, const char* arg, int cmd);
void do_attribute(struct char_data* ch, const char* arg, int cmd);
void do_spells(struct char_data* ch, const char* arg, int cmd);
void do_world(struct char_data* ch, const char* arg, int cmd);
void do_report(struct char_data* ch, const char* arg, int cmd);

/* Communication commands */
void do_say(struct char_data* ch, const char* argument, int cmd);
void do_shout(struct char_data* ch, const char* argument, int cmd);
void do_tell(struct char_data* ch, const char* argument, int cmd);
void do_whisper(struct char_data* ch, const char* arg, int cmd);
void do_ask(struct char_data* ch, const char* arg, int cmd);
void do_emote(struct char_data* ch, const char* argument, int cmd);
void do_insult(struct char_data* ch, const char* argument, int cmd);
void do_commune(struct char_data* ch, const char* argument, int cmd);
void do_sign(struct char_data* ch, const char* arg, int cmd);
void do_grouptell(struct char_data* ch, const char* arg, int cmd);

/* Object manipulation */
void do_get(struct char_data* ch, const char* argument, int cmd);
void do_drop(struct char_data* ch, const char* argument, int cmd);
void do_put(struct char_data* ch, const char* argument, int cmd);
void do_give(struct char_data* ch, const char* arg, int cmd);
void do_junk(struct char_data* ch, const char* arg, int cmd);
void do_fill(struct char_data* ch, const char* arg, int cmd);

/* Equipment commands */
void do_wear(struct char_data* ch, const char* argument, int cmd);
void do_wield(struct char_data* ch, const char* argument, int cmd);
void do_grab(struct char_data* ch, const char* argument, int cmd);
void do_remove(struct char_data* ch, const char* argument, int cmd);

/* Consumable commands */
void do_drink(struct char_data* ch, const char* arg, int cmd);
void do_eat(struct char_data* ch, const char* arg, int cmd);
void do_pour(struct char_data* ch, const char* arg, int cmd);
void do_sip(struct char_data* ch, const char* arg, int cmd);
void do_taste(struct char_data* ch, const char* arg, int cmd);
void do_quaff(struct char_data* ch, const char* argument, int cmd);
void do_recite(struct char_data* ch, const char* argument, int cmd);
void do_use(struct char_data* ch, const char* argument, int cmd);

/* Combat commands */
void do_hit(struct char_data* ch, const char* argument, int cmd);
void do_kill(struct char_data* ch, const char* argument, int cmd);
void do_bash(struct char_data* ch, const char* arg, int cmd);
void do_kick(struct char_data* ch, const char* arg, int cmd);
void do_backstab(struct char_data* ch, const char* arg, int cmd);
void do_rescue(struct char_data* ch, const char* arg, int cmd);
void do_disarm(struct char_data* ch, const char* arg, int cmd);
void do_assist(struct char_data* ch, const char* arg, int cmd);
void do_headbutt(struct char_data* ch, const char* arg, int cmd);
void do_grapple(struct char_data* ch, const char* arg, int cmd);
void do_bodyslam(struct char_data* ch, const char* arg, int cmd);
void do_deathstroke(struct char_data* ch, const char* arg, int cmd);
void do_springleap(struct char_data* ch, const char* arg, int cmd);
void do_quivering_palm(struct char_data* ch, const char* arg, int cmd);
void do_doorbash(struct char_data* ch, const char* arg, int cmd);
void do_throw(struct char_data* ch, const char* arg, int cmd);

/* Ranged combat */
void do_shoot(struct char_data* ch, const char* arg, int cmd);
void do_reload(struct char_data* ch, const char* arg, int cmd);
void do_fire(struct char_data* ch, const char* arg, int cmd);

/* Thief skills */
void do_sneak(struct char_data* ch, const char* arg, int cmd);
void do_hide(struct char_data* ch, const char* arg, int cmd);
void do_steal(struct char_data* ch, const char* arg, int cmd);
void do_pick(struct char_data* ch, const char* arg, int cmd);
void do_subterfuge(struct char_data* ch, const char* arg, int cmd);
void do_spy(struct char_data* ch, const char* arg, int cmd);
void do_search(struct char_data* ch, const char* arg, int cmd);

/* Special class abilities */
void do_lay_hands(struct char_data* ch, const char* arg, int cmd);
void do_feign_death(struct char_data* ch, const char* arg, int cmd);
void do_first_aid(struct char_data* ch, const char* arg, int cmd);
void do_track(struct char_data* ch, const char* arg, int cmd);

/* Magic commands */
void do_cast(struct char_data* ch, const char* arg, int cmd);
void do_practice(struct char_data* ch, const char* arg, int cmd);
void do_scribe(struct char_data* ch, const char* arg, int cmd);
void do_brew(struct char_data* ch, const char* arg, int cmd);
void do_channel(struct char_data* ch, const char* arg, int cmd);
void do_pray(struct char_data* ch, const char* arg, int cmd);

/* Door manipulation */
void do_open(struct char_data* ch, const char* arg, int cmd);
void do_close(struct char_data* ch, const char* arg, int cmd);
void do_lock(struct char_data* ch, const char* arg, int cmd);
void do_unlock(struct char_data* ch, const char* arg, int cmd);

/* Group commands */
void do_group(struct char_data* ch, const char* arg, int cmd);
void do_follow(struct char_data* ch, const char* arg, int cmd);
void do_order(struct char_data* ch, const char* arg, int cmd);
void do_split(struct char_data* ch, const char* arg, int cmd);

/* Misc actions */
void do_guard(struct char_data* ch, const char* arg, int cmd);
void do_swim(struct char_data* ch, const char* arg, int cmd);
void do_pull(struct char_data* ch, const char* argument, int cmd);
void do_write(struct char_data* ch, const char* arg, int cmd);
void do_pose(struct char_data* ch, const char* argument, int cmd);
void do_highfive(struct char_data* ch, const char* arg, int cmd);
void do_action(struct char_data* ch, const char* arg, int cmd);
void do_play(struct char_data* ch, const char* arg, int cmd);
void do_bet(struct char_data* ch, const char* arg, int cmd);
void do_stay(struct char_data* ch, const char* arg, int cmd);
void do_gain(struct char_data* ch, const char* arg, int cmd);

/* Player settings */
void do_brief(struct char_data* ch, const char* arg, int cmd);
void do_cls(struct char_data* ch, const char* arg, int cmd);
void do_terminal(struct char_data* ch, const char* arg, int cmd);
void do_prompt(struct char_data* ch, const char* arg, int cmd);
void do_compact(struct char_data* ch, const char* argument, int cmd);
void do_wimpy(struct char_data* ch, const char* argument, int cmd);
void do_title(struct char_data* ch, const char* arg, int cmd);
void do_color(struct char_data* ch, const char* arg, int cmd);
void do_plr_noshout(struct char_data* ch, const char* argument, int cmd);
void do_bamfin(struct char_data* ch, const char* arg, int cmd);
void do_bamfout(struct char_data* ch, const char* arg, int cmd);
void do_passwd(struct char_data* ch, const char* arg, int cmd);

/* Game information */
void do_news(struct char_data* ch, const char* argument, int cmd);
void do_wiznews(struct char_data* ch, const char* argument, int cmd);
void do_atlas(struct char_data* ch, const char* argument, int cmd);
void do_help(struct char_data* ch, const char* argument, int cmd);
void do_info(struct char_data* ch, const char* arg, int cmd);
void do_credits(struct char_data* ch, const char* argument, int cmd);
void do_wizlist(struct char_data* ch, const char* arg, int cmd);
void do_wizhelp(struct char_data* ch, const char* argument, int cmd);

/* Session commands */
void do_quit(struct char_data* ch, const char* argument, int cmd);
void do_qui(struct char_data* ch, const char* argument, int cmd);
void do_save(struct char_data* ch, const char* argument, int cmd);
void do_rent(struct char_data* ch, const char* arg, int cmd);
void do_exit(struct char_data* ch, const char* argument, int cmd);

/* Wizard commands - observation */
void do_snoop(struct char_data* ch, const char* argument, int cmd);
void do_stat(struct char_data* ch, const char* arg, int cmd);
void do_users(struct char_data* ch, const char* arg, int cmd);
void do_loglist(struct char_data* ch, const char* argument, int cmd);
void do_checklog(struct char_data* ch, const char* argument, int cmd);
void do_deathcheck(struct char_data* ch, const char* argument, int cmd);
void do_monitor(struct char_data* ch, const char* argument, int cmd);
void do_show(struct char_data* ch, const char* arg, int cmd);
void do_log(struct char_data* ch, const char* arg, int cmd);
void do_uptime(struct char_data* ch, const char* arg, int cmd);

/* Wizard commands - world manipulation */
void do_at(struct char_data* ch, const char* arg, int cmd);
void do_goto(struct char_data* ch, const char* arg, int cmd);
void do_trans(struct char_data* ch, const char* argument, int cmd);
void do_force(struct char_data* ch, const char* argument, int cmd);
void do_send(struct char_data* ch, const char* arg, int cmd);
void do_echo(struct char_data* ch, const char* argument, int cmd);
void do_load(struct char_data* ch, const char* arg, int cmd);
void do_purge(struct char_data* ch, const char* arg, int cmd);
void do_return(struct char_data* ch, const char* argument, int cmd);
void do_switch(struct char_data* ch, const char* argument, int cmd);
void do_invis(struct char_data* ch, const char* arg, int cmd);

/* Wizard commands - player management */
void do_advance(struct char_data* ch, const char* arg, int cmd);
void do_demote(struct char_data* ch, const char* arg, int cmd);
void do_reroll(struct char_data* ch, const char* arg, int cmd);
void do_restore(struct char_data* ch, const char* arg, int cmd);
void do_noshout(struct char_data* ch, const char* argument, int cmd);
void do_nohassle(struct char_data* ch, const char* argument, int cmd);
void do_stealth(struct char_data* ch, const char* argument, int cmd);
void do_set(struct char_data* ch, const char* arg, int cmd);
void do_flag(struct char_data* ch, const char* arg, int cmd);
void do_auth(struct char_data* ch, const char* arg, int cmd);

/* Wizard commands - world building */
void do_rload(struct char_data* ch, const char* arg, int cmd);
void do_rsave(struct char_data* ch, const char* arg, int cmd);
void do_edit(struct char_data* ch, const char* arg, int cmd);
void do_link(struct char_data* ch, const char* arg, int cmd);
void do_instazone(struct char_data* ch, const char* arg, int cmd);
void do_string(struct char_data* ch, const char* arg, int cmd);
void do_oset(struct char_data* ch, const char* arg, int cmd);
void do_bload(struct char_data* ch, const char* arg, int cmd);

/* Wizard commands - server control */
void do_shutdown(struct char_data* ch, const char* argument, int cmd);
void do_shutdow(struct char_data* ch, const char* arg, int cmd);
void do_wizlock(struct char_data* ch, const char* arg, int cmd);
void do_silence(struct char_data* ch, const char* arg, int cmd);
void do_system(struct char_data* ch, const char* argument, int cmd);
void do_imptest(struct char_data* ch, const char* arg, int cmd);
void do_teams(struct char_data* ch, const char* arg, int cmd);
void do_command(struct char_data* ch, const char* arg, int cmd);

/* Reporting commands */
void do_idea(struct char_data* ch, const char* arg, int cmd);
void do_typo(struct char_data* ch, const char* arg, int cmd);
void do_bug(struct char_data* ch, const char* arg, int cmd);

/* Placeholder */
void do_not_here(struct char_data* ch, const char* argument, int cmd);
