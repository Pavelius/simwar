#include "main.h"

bsreq effect_type[] = {
	BSREQ(effecti, test, ability_type),
	BSREQ(effecti, text, text_type),
	BSREQ(effecti, ability, number_type),
	BSREQ(effecti, units, unit_type),
{}};

bool effecti::isvalid(const heroi& e) const {
	if(e.get(test) < 1)
		return false;
	return true;
}

void effecti::operator+=(const effecti& e) {
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

void effecti::apply(string& sb, playeri* player, provincei* province, heroi* hero) const {
	if(player) {
		player->cost.gold += get(Gold);
		player->cost.fame += get(Fame);
	}
	if(province) {
		province->addeconomy(get(Gold));
		province->addsupport(province->getplayer(), get(Support));
		if(get(Attack)) {
			province->addinvader((uniti**)units, sizeof(units) / sizeof(units[0]), 0);
			province->battle(sb, 0, player, 0, false);
		}
		if(get(Recruit)) {
			for(auto u : units) {
				if(!u)
					break;
				province->add(u);
			}
		}
	}
	if(hero) {
		hero->setwound(hero->get(Wounds) + get(Wounds));
		hero->setwait(hero->get(Wait) + get(Wait));
		hero->setloyalty(hero->get(Loyalty) + get(Loyalty));
	}
}