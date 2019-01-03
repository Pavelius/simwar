#include "main.h"

bsreq hero_info::metadata[] = {
	BSREQ(hero_info, id, text_type),
	BSREQ(hero_info, name, text_type),
	BSREQ(hero_info, nameof, text_type),
	BSREQ(hero_info, text, text_type),
	BSREQ(hero_info, avatar, text_type),
	BSREQ(hero_info, tactic, tactic_type),
	BSREQ(hero_info, traits, trait_type),
	BSREQ(hero_info, player, player_info::metadata),
	BSREQ(hero_info, province, province_info::metadata),
{}};
adat<hero_info, 128> hero_data;
bsdata hero_manager("hero", hero_data, hero_info::metadata);

void hero_info::before_turn() {
	tactic = 0;
	province = 0;
	action = game.default_action;
}

int hero_info::get(const char* id) const {
	auto r = 0;
	for(auto p : traits) {
		if(!p)
			continue;
		r += p->get(id);
	}
	return r;
}

int hero_info::getbonus(const char* id) const {
	return 0;
}

int	hero_info::getincome() const {
	auto result = 0;
	for(auto p : traits) {
		if(p)
			result--;
	}
	result += getex("nobility");
	return result;
}

void hero_info::resolve() {
	char temp[8192]; temp[0] = 0;
	if(action->attack || action->raid) {
		auto israid = (action->raid > 0);
		auto enemy = province->getplayer();
		if(enemy!=player)
			province->battle(temp, zendof(temp), player, enemy, israid);
	}
}