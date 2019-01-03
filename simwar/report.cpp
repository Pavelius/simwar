#include "main.h"

adat<report_info, 2048>	report_data;

report_info* report_info::add(player_info* player, province_info* province, hero_info* hero, const char* text) {
	auto p = report_data.add();
	p->turn = game.turn;
	p->hero = hero;
	p->player = player;
	p->province = province;
	p->text = szdup(text);
	return p;
}

bool report_info::is(const player_info* player) const {
	return this->player == player;
}

void report_info::set(player_info* player) {
	this->player = player;
}