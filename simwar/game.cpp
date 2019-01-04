#include "main.h"

static bsreq game_type[] = {
	BSREQ(game_info, income_per_level, number_type),
	BSREQ(game_info, casualties, number_type),
	BSREQ(game_info, support_maximum, number_type),
	BSREQ(game_info, support_minimum, number_type),
	BSREQ(game_info, support_multiplier, number_type),
	BSREQ(game_info, support_attack, number_type),
	BSREQ(game_info, support_defend, number_type),
	BSREQ(game_info, default_action, action_type),
	BSREQ(game_info, map, text_type),
{}};
game_info		game;
bsdata			game_manager("game", game, game_type);

void game_info::clear() {
	memset(this, 0, sizeof(*this));
}

void game_info::after_load() {
	change_support_provinces = 2;
	if(change_support_provinces > province_data.count)
		change_support_provinces = province_data.count;
}