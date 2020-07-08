#include "main.h"

adat<reporti, 2048>	report_data;

reporti* reporti::add(const playeri* player, const provincei* province, const heroi* hero, const char* text) {
	auto p = report_data.add();
	p->turn = game.turn;
	p->hero = hero;
	p->player = player;
	p->province = province;
	p->text = szdup(text);
	return p;
}

bool reporti::is(const playeri* player) const {
	return this->player == player;
}

void reporti::set(playeri* player) {
	this->player = player;
}