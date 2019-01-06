#include "main.h"

bsreq player_info::metadata[] = {
	BSREQ(player_info, id, text_type),
	BSREQ(player_info, name, text_type),
	BSREQ(player_info, nameof, text_type),
	BSREQ(player_info, text, text_type),
	BSREQ(player_info, type, player_ai_type),
	BSREQ(player_info, gold, number_type),
	BSREQ(player_info, fame, number_type),
{}};
adat<player_info, player_max> player_data;
bsdata player_manager("player", player_data, player_info::metadata);

int	player_info::getindex() const {
	return player_data.indexof(this);
}

int player_info::getincome(tip_info* ti) const {
	auto result = 0, r = 0;
	for(auto& e : province_data) {
		if(!e)
			continue;
		if(e.getplayer() != this)
			continue;
		r += e.getincome();
	}
	result += fix(ti, msg.income_province, r); r = 0;
	for(auto& e : troop_data) {
		if(!e)
			continue;
		if(e.getplayer() != this)
			continue;
		r += e.getincome();
	}
	result += fix(ti, msg.income_units, r); r = 0;
	for(auto& e : hero_data) {
		if(!e)
			continue;
		if(e.getplayer() != this)
			continue;
		r += e.getincome();
	}
	result += fix(ti, msg.income_heroes, r); r = 0;
	return result;
}

int	player_info::getsupport(tip_info* ti) const {
	auto result = 0;
	for(auto& e : hero_data) {
		if(!e)
			continue;
		if(e.getplayer() != this)
			continue;
	}
	return result;
}

unsigned player_info::gettroops(troop_info** source, unsigned maximum_count, const province_info* province, const player_info* player, const player_info* player_move) {
	auto ps = source;
	auto pe = ps + maximum_count;
	for(auto& e : troop_data) {
		if(!e)
			continue;
		if(player && e.getplayer() != player)
			continue;
		if(province && e.getprovince(player_move) != province)
			continue;
		if(ps < pe)
			*ps++ = &e;
	}
	return ps - source;
}

province_info* player_info::getbestprovince() const {
	province_info* elements[player_max];
	auto count = province_info::select(elements, sizeof(elements) / sizeof(elements[0]), this);
	if(!count)
		return 0;
	return elements[0];
}

void player_info::gain_profit() {
	for(auto& e : player_data) {
		if(!e)
			continue;
		e.gold += e.getincome();
	}
}

unsigned player_info::getactions(hero_info** source, unsigned maximum, int order) {
	auto ps = source;
	auto pe = ps + maximum;
	for(auto& e : hero_data) {
		if(!e)
			continue;
		auto a = e.getaction();
		if(!a)
			continue;
		if(a->order != order)
			continue;
		if(ps < pe)
			*ps++ = &e;
	}
	return ps - source;
}

static int compare_heroes_by_order(const void* p1, const void* p2) {
	auto e1 = *((hero_info**)p1);
	auto e2 = *((hero_info**)p2);
	auto pl1 = e1->getplayer();
	auto pl2 = e2->getplayer();
	auto sp1 = pl1 ? pl1->getcost().fame : 0;
	auto sp2 = pl2 ? pl2->getcost().fame : 0;
	if(sp1 < sp2)
		return -1;
	else if(sp1 > sp2)
		return 1;
	return 0;
}

void player_info::resolve_actions() {
	hero_info* heroes[64];
	build_info::build_units();
	for(auto i = 1; i <= 5; i++) {
		auto count = getactions(heroes, sizeof(heroes) / sizeof(heroes[0]), i);
		if(!count)
			continue;
		qsort(heroes, count, sizeof(heroes[0]), compare_heroes_by_order);
		for(unsigned j = 0; j < count; j++) {
			auto hero = heroes[j];
			hero->resolve();
		}
	}
}

static void create_province_order() {
	for(unsigned i = 0; i < province_data.count; i++)
		province_order[i] = i;
	zshuffle(province_order, province_data.count);
}

void player_info::sethire(hero_info* hero) {
	if(hire_hero) {
		hire_hero = 0;
		gold += gethirecost(hire_hero);
	}
	if(hero) {
		hire_hero = hero;
		gold -= gethirecost(hire_hero);
	}
}

int player_info::gethirecost(const hero_info* hero) const {
	return game.hire_cost;
}

bool player_info::isallowhire() const {
	auto need_gold = gethirecost(0);
	if(gold < need_gold)
		return false;
	for(auto& e : hero_data) {
		if(!e)
			continue;
		if(e.getplayer() == 0)
			return true;
	}
	return false;
}

void player_info::hire_heroes() {
	for(auto& e : player_data) {
		if(!e)
			continue;
		if(!e.hire_hero)
			continue;
		string sb; sb.create(&e, 0, e.hire_hero);
		if(e.hire_hero->getplayer()) {
			sb.add(msg.hero_hire_fail, e.hire_hero->getname(), e.hire_hero->getplayer()->getname());
			sb.post();
			e.sethire(0);
		} else {
			sb.add(msg.hero_hire_success, e.hire_hero->getname());
			sb.post();
			e.hire_hero = 0;
		}
	}
}

void player_info::playgame() {
	create_province_order();
	while(true) {
		hero_info::refresh_heroes();
		for(auto& e : player_data) {
			if(!e)
				continue;
			switch(e.type) {
			case PlayerHuman:
				e.makemove();
				break;
			case PlayerComputer:
				break;
			}
		}
		game.turn++;
		resolve_actions();
		gain_profit();
		hero_info::desert_heroes();
		hire_heroes();
		province_info::change_support();
	}
}