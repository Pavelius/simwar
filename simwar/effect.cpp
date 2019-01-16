#include "main.h"

bsreq effect_type[] = {
	BSREQ(effect_info, test, ability_type),
	BSREQ(effect_info, text, text_type),
	BSREQ(effect_info, ability, number_type),
	BSREQ(effect_info, units, unit_type),
{}};

bool effect_info::isvalid(const hero_info& e) const {
	if(e.get(test) < 1)
		return false;
	return true;
}

void effect_info::operator+=(const effect_info& e) {
	for(auto i = Good; i <= LastAbility; i = (ability_s)(i + 1))
		set(i, e.get(i));
	for(auto v : e.units) {
		for(int i = 0; i < sizeof(units) / sizeof(units[0]) && e.units[i]; i++) {
			if(!units[i]) {
				units[i] = e.units[i];
				break;
			}
		}
	}
}

void effect_info::apply(player_info* player) {
	player->cost.gold += get(Gold);
}

void effect_info::apply(hero_info* hero) {
	hero->setwound(hero->get(Wounds) + get(Wounds));
	hero->setwound(hero->get(Wait) + get(Wait));
}