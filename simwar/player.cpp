#include "main.h"

bsreq player_info::metadata[] = {
	BSREQ(player_info, id, text_type),
	BSREQ(player_info, name, text_type),
	BSREQ(player_info, nameof, text_type),
	BSREQ(player_info, text, text_type),
	BSREQ(player_info, gold, number_type),
	BSREQ(player_info, fame, number_type),
{}};
adat<player_info, player_max>	player_data;
bsdata			player_manager("player", player_data, player_info::metadata);

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
	auto sp1 = pl1->getcost().fame;
	auto sp2 = pl2->getcost().fame;
	if(sp1 < sp2)
		return -1;
	else if(sp1 > sp2)
		return 1;
	return 0;
}

void player_info::resolve_actions() {
	hero_info* heroes[64];
	build_info::build_units();
	for(auto i = 0; i < 10; i++) {
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

void player_info::add(province_info* province, hero_info* hero, const char* text) {
	report_info::add(this, province, hero, text);
}

static void create_province_order() {
	for(unsigned i = 0; i < province_data.count; i++)
		province_order[i] = i;
	zshuffle(province_order, province_data.count);
}

void player_info::playgame() {
	create_province_order();
	while(true) {
		hero_info::refresh_heroes();
		for(auto& e : player_data) {
			if(!e)
				continue;
			e.makemove();
		}
		game.turn++;
		resolve_actions();
		gain_profit();
		province_info::change_support();
	}
}