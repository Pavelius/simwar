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

void effect_info::apply(player_info* player) const {
	if(!player)
		return;
	player->cost.gold += get(Gold);
	player->cost.fame += get(Fame);
}

void effect_info::apply(province_info* province) const {
	if(!province)
		return;
	province->addeconomy(get(Gold));
	province->addsupport(province->getplayer(), get(Gold));
	if(get(Recruit)) {
		for(auto u : units) {
			if(!u)
				break;
			province->add(u);
		}
	}
}

void effect_info::apply(hero_info* hero) const {
	if(!hero)
		return;
	hero->setwound(hero->get(Wounds) + get(Wounds));
	hero->setwait(hero->get(Wait) + get(Wait));
	hero->setloyalty(hero->get(Loyalty) + get(Loyalty));
}