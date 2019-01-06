#include "bsdata.h"
#include "msg.h"

bsreq msg_type[] = {
	BSREQ(msg_info, attacking_force, text_type),
	BSREQ(msg_info, casualties, text_type),
	BSREQ(msg_info, defending_force, text_type),
	BSREQ(msg_info, neutral_army, text_type),
	BSREQ(msg_info, neutral_army_of, text_type),
	BSREQ(msg_info, winner, text_type),
	BSREQ(msg_info, lead, text_type),
	BSREQ(msg_info, moved, text_type),
	BSREQ(msg_info, raid_spoils, text_type),
	//
	BSREQ(msg_info, attack, text_type),
	BSREQ(msg_info, defend, text_type),
	BSREQ(msg_info, raid, text_type),
	BSREQ(msg_info, sword, text_type),
	BSREQ(msg_info, shield, text_type),
	BSREQ(msg_info, diplomacy, text_type),
	BSREQ(msg_info, loyalty, text_type),
	BSREQ(msg_info, good, text_type),
	BSREQ(msg_info, good_negative, text_type),
	BSREQ(msg_info, nobility, text_type),
	BSREQ(msg_info, nobility_negative, text_type),
	BSREQ(msg_info, will, text_type),
	BSREQ(msg_info, support, text_type),
	//
	BSREQ(msg_info, income, text_type),
	BSREQ(msg_info, income_province, text_type),
	BSREQ(msg_info, income_units, text_type),
	BSREQ(msg_info, income_heroes, text_type),
	BSREQ(msg_info, cost, text_type),
	BSREQ(msg_info, squads, text_type),
	BSREQ(msg_info, turns, text_type),
	//
	BSREQ(msg_info, hero_wait, text_type),
	BSREQ(msg_info, hero_wound, text_type),
	BSREQ(msg_info, hero_desert, text_type),
	BSREQ(msg_info, hero_choose, text_type),
	//	
	BSREQ(msg_info, total_strenght, text_type),
	BSREQ(msg_info, total, text_type),
	BSREQ(msg_info, predict_fail, text_type),
	BSREQ(msg_info, predict_partial, text_type),
	BSREQ(msg_info, predict_success, text_type),
	//
	BSREQ(msg_info, not_enought_gold, text_type),
	BSREQ(msg_info, not_choose_units, text_type),
	//
	BSREQ(msg_info, title, text_type),
	BSREQ(msg_info, exit, text_type),
	BSREQ(msg_info, end_turn, text_type),
	BSREQ(msg_info, loadgame, text_type),
	BSREQ(msg_info, newgame, text_type),
	BSREQ(msg_info, savegame, text_type),
	BSREQ(msg_info, accept, text_type),
	BSREQ(msg_info, cancel, text_type),
	BSREQ(msg_info, yes, text_type),
	BSREQ(msg_info, no, text_type),
	//
	BSREQ(msg_info, wound, text_type),
	BSREQ(msg_info, wounds, text_type),
	BSREQ(msg_info, wounds_of, text_type),
{}};
msg_info msg;
bsdata msg_manager("msg", msg, msg_type);