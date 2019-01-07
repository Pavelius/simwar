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

void player_info::post(const hero_info* hero, const province_info* province, const char* text) const {
	report_info::add(this, province, hero, text);
}

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

int player_info::getherocount() const {
	auto result = 0;
	for(auto& e : hero_data) {
		if(!e || e.getplayer() != this)
			continue;
		result++;
	}
	return result;
}

int player_info::compare_hire_bet(const void* p1, const void* p2) {
	auto e1 = *((player_info**)p1);
	auto e2 = *((player_info**)p2);
	if(e2->hire_gold < e1->hire_gold)
		return -1;
	if(e2->hire_gold > e1->hire_gold)
		return 1;
	if(e2->fame < e1->fame)
		return -1;
	if(e2->fame > e1->fame)
		return 1;
	if(e2->gold < e1->gold)
		return -1;
	if(e2->gold > e1->gold)
		return 1;
	return 0;
}

void player_info::check_heroes() {
	for(auto& e : hero_data) {
		if(!e)
			continue;
		e.check_leave();
	}
}

void player_info::suggest_heroes() {
	game.hire_hero = 0;
	game.hire_event += xrand(5, 20);
	if(game.hire_event < 100)
		return;
	game.hire_event -= 100;
	hero_info* source[hero_max];
	auto count = hero_info::select(source, lenghtof(source), 0);
	if(!count)
		return;
	game.hire_hero = source[rand() % count];
}

void player_info::hire_heroes() {
	if(!game.hire_hero)
		return;
	auto hero = game.hire_hero;
	adat<player_info*, player_max> source;
	// ��������� ������������
	for(auto& e : player_data) {
		if(!e || !e.hire_gold)
			continue;
		if(e.getherocount() >= hero_max_per_player)
			continue;
		source.add(&e);
	}
	// �������� �������
	qsort(source.data, source.getcount(), sizeof(source.data[0]), compare_hire_bet);
	// �������� ��������
	for(unsigned i = 0; i < source.count; i++) {
		auto player = source.data[i];
		string sb(hero->getgender());
		if(i == 0) {
			sb.add(msg.hero_hire_success, hero->getname(), player->getname());
			player->post(hero, 0, sb);
			player->hire_gold = 0;
			hero->setplayer(const_cast<player_info*>(player));
		} else {
			sb.add(msg.hero_hire_fail, hero->getname(), source.data[0]->getname());
			player->post(hero, 0, sb);
			player->gold += player->hire_gold;
			player->hire_gold = 0;
		}
	}
}

void player_info::check_hire() {
	if(!game.hire_hero)
		return;
	auto base_multiplier = 10;
	answer_info aw;
	for(auto i = 0; i < 7; i++) {
		auto value = (i + 1)*base_multiplier;
		if(gold>=value)
			aw.add(value, msg.pay_gold, value);
	}
	aw.add(0, msg.cancel);
	if(aw.elements.getcount() <= 1)
		return;
	auto pay_value = choose(game.hire_hero, aw, msg.hero_hire, game.hire_hero->getname());
	if(!pay_value)
		return;
	hire_gold = pay_value;
	gold -= hire_gold;
}

void player_info::playgame() {
	create_province_order();
	game.hire_event = 80;
	while(true) {
		hero_info::refresh_heroes();
		for(auto& e : player_data) {
			if(!e)
				continue;
			e.check_hire();
			e.show_reports();
			switch(e.type) {
			case PlayerHuman:
				e.make_move();
				break;
			case PlayerComputer:
				break;
			}
		}
		// �������� ��� �����������
		game.turn++;
		resolve_actions();
		gain_profit();
		check_heroes();
		hire_heroes();
		suggest_heroes();
		province_info::change_support();
	}
}