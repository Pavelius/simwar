#include "main.h"

const int ideal_safety = 2;

void player_info::computer_move() {
	hero_info* source[hero_max];
	auto hero_count = hero_info::select(source, lenghtof(source), this);
	for(unsigned i = 0; i < hero_count; i++)
		source[i]->make_move();
}

bool hero_info::choose_units_computer(const action_info* action, const province_info* province, unit_set& s1, unit_set& s2, cost_info& cost) const {
	cost_info total = cost;
	zshuffle(s1.data, s1.getcount());
	for(unsigned i = 0; i < s1.count; i++) {
		total += s1.data[i]->cost;
		if(total > player->cost)
			break;
		cost = total;
		s2.add(s1.data[i]);
	}
	return s2.getcount() > 0;
}

bool hero_info::choose_troops_computer(const action_info* action, const province_info* province, army& s1, army& s2, army& a3, int minimal_count, cost_info& cost, int safety) const {
	auto defend = a3.getstrenght(0);
	cost_info total = cost;
	zshuffle(s1.data, s1.getcount());
	for(unsigned i = 0; i < s1.count; i++) {
		auto attack = s2.get(Attack, 0);
		if(attack >= defend + safety)
			break;
		total.gold += action->cost_per_unit;
		if(total > player->cost)
			return false;
		cost = total;
		s2.add(s1.data[i]);
	}
	auto attack = s2.get(Attack, 0);
	return attack >= defend;
}

static int getminimaldefence(const player_info* player) {
	adat<province_info*> provincies;
	provincies.count = province_info::select(provincies.data, provincies.getmaximum());
	auto minimal_defence = -1;
	for(auto p : provincies) {
		if(p->getstatus(player) == NoFriendlyProvince)
			continue;
		auto d = p->getstrenght();
		if(minimal_defence ==-1 || minimal_defence > d)
			minimal_defence = d;
	}
	return minimal_defence;
}

static int getweight(const hero_info* hero, const action_info* action) {
	const int ideal_provincies = 10;
	const int ideal_gold = 30;
	const int ideal_income = 5;
	const int ideal_income_maximum = 15;
	const int higly_likely = 20;
	auto result = 10;
	auto player = hero->getplayer();
	auto good_mode = (action->getprovince() == NoFriendlyProvince) ? -1 : 1;
	if(player) {
		auto attack_strenght = player->getstrenght();
		auto gold = player->cost.gold;
		auto income = player->getincome(0);
		auto provincies = player->getfriendlyprovinces();
		auto troops = player->gettroopscount();
		auto ideal_troops = provincies;
		if(ideal_troops < 10)
			ideal_troops = 10;
		if(action->get(Attack) > 0)
			result += (ideal_provincies - player->getfriendlyprovinces());
		if(action->get(Raid) > 0)
			result += (ideal_gold - gold) * 2 + (ideal_income - income);
		if(action->get(Economy) > 0)
			result += (ideal_income - income) * 2;
		if(action->get(Support) > 0)
			result -= player->getsupport() / 4;
		if(action->get(Recruit) > 0) {
			auto minimal_strenght = getminimaldefence(player) + ideal_safety;
			result += (ideal_troops - troops) * 3;
			if(minimal_strenght > attack_strenght)
				result += higly_likely;
		}
	}
	result += action->cost.gold / 2;
	result += action->get(Support);
	result += action->get(Economy) * 2 * good_mode;
	return result;
}

bool hero_info::choose_troops(const action_info* action, const province_info* province, army& a1, army& a2, army& a3, int minimal_count, cost_info& cost) const {
	if(player->iscomputer())
		return choose_troops_computer(action, province, a1, a2, a3, minimal_count, cost, ideal_safety);
	return choose_troops_human(action, province, a1, a2, a3, minimal_count, cost);
}

const action_info* hero_info::choose_action() const {
	choiseset ai;
	for(auto& e : action_data) {
		if(!e)
			continue;
		if(!isallow(&e))
			continue;
		if(player) {
			if(!player->isallow(&e))
				continue;
		}
		if(e.isplaceable()) {
			if(!choose_province(&e, false))
				continue;
		}
		auto p = ai.add((int)&e, e.getname());
		p->weight = getweight(this, &e);
	}
	ai.sort();
	auto interactive = player && !player->iscomputer();
	return (action_info*)ai.choose(interactive, this, true, 0);
}