#include "bsdata.h"
#include "msg.h"

bsreq msg_type[] = {
	BSREQ(msgi, attacking_force, text_type),
	BSREQ(msgi, casualties, text_type),
	BSREQ(msgi, tactic_changed, text_type),
	BSREQ(msgi, defending_force, text_type),
	BSREQ(msgi, battle_magic, text_type),
	BSREQ(msgi, neutral_army, text_type),
	BSREQ(msgi, neutral_army_of, text_type),
	BSREQ(msgi, winner, text_type),
	BSREQ(msgi, lead, text_type),
	BSREQ(msgi, moved, text_type),
	BSREQ(msgi, raid_spoils, text_type),
	//
	BSREQ(msgi, income, text_type),
	BSREQ(msgi, income_province, text_type),
	BSREQ(msgi, income_units, text_type),
	BSREQ(msgi, income_heroes, text_type),
	BSREQ(msgi, pay_gold, text_type),
	BSREQ(msgi, cost, text_type),
	BSREQ(msgi, squads, text_type),
	BSREQ(msgi, turns, text_type),
	//
	BSREQ(msgi, hero_wait, text_type),
	BSREQ(msgi, hero_wound, text_type),
	BSREQ(msgi, hero_desert, text_type),
	BSREQ(msgi, hero_hire, text_type),
	BSREQ(msgi, hero_hire_success, text_type),
	BSREQ(msgi, hero_hire_fail, text_type),
	//	
	BSREQ(msgi, total_strenght, text_type),
	BSREQ(msgi, total, text_type),
	BSREQ(msgi, predict_fail, text_type),
	BSREQ(msgi, predict_partial, text_type),
	BSREQ(msgi, predict_success, text_type),
	//
	BSREQ(msgi, not_enought_gold, text_type),
	BSREQ(msgi, not_choose_units, text_type),
	//
	BSREQ(msgi, title, text_type),
	BSREQ(msgi, exit, text_type),
	BSREQ(msgi, end_turn, text_type),
	BSREQ(msgi, loadgame, text_type),
	BSREQ(msgi, newgame, text_type),
	BSREQ(msgi, savegame, text_type),
	BSREQ(msgi, accept, text_type),
	BSREQ(msgi, cancel, text_type),
	BSREQ(msgi, yes, text_type),
	BSREQ(msgi, no, text_type),
	//
	BSREQ(msgi, word_in, text_type),
	BSREQ(msgi, troops_desert, text_type),	
{}};
msgi msg;
bsdata msg_manager("msg", msg, msg_type);