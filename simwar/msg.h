#pragma once

struct msg_info {
	const char*	attacking_force;
	const char*	defending_force;
	const char*	neutral_army, *neutral_army_of;
	const char* casualties;
	const char* tactic_changed;
	const char* winner;
	const char* lead;
	const char* moved;
	const char* raid_spoils;
	const char* diplomacy;
	const char* loyalty;
	const char* support;
	const char* total_strenght, *total;
	const char* predict_fail, *predict_partial, *predict_success;
	const char*	hero_wait;
	const char*	hero_wound;
	const char* hero_desert;
	const char* hero_hire;
	const char* hero_hire_fail;
	const char* hero_hire_success;
	const char* pay_gold;
	const char* income;
	const char* income_province;
	const char* income_units;
	const char* income_heroes;
	const char* not_enought_gold;
	const char* not_choose_units;
	const char* cost;
	const char* squads;
	const char* turns;
	const char* title;
	const char* troops_desert;
	const char* end_turn;
	const char* exit;
	const char* loadgame;
	const char* newgame;
	const char* savegame;
	const char* accept;
	const char* cancel;
	const char* yes;
	const char* no;
	const char* wound;
	const char* wounds;
	const char* wounds_of;
	const char* word_in;
};