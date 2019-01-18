#include "main.h"

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

bool hero_info::choose_troops_computer(const action_info* action, const province_info* province, army& s1, army& s2, army& a3, int minimal_count, cost_info& cost) const {
	const int safety = 2;
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

static const action_info* get_action(adat<action_info*> actions, ability_s id) {
	for(auto p : actions) {
		if(p->get(id) > 0)
			return p;
	}
	return 0;
}

static bool most_needed(const player_info* player, const hero_info* hero, const action_info* action) {
	const int ideal_provincies = 10;
	const int ideal_gold = 30;
	const int ideal_income = 5;
	const int ideal_income_maximum = 15;
	const int ideal_troops = 10;
	if(action->get(Attack) > 0) {
		auto provincies = player->getfriendlyprovinces();
		if(provincies < ideal_provincies)
			return true;
	}
	if(action->get(Raid) > 0) {
		auto gold = player->cost.gold;
		auto income = player->getincome(0);
		if(income < ideal_income && gold < ideal_gold)
			return true;
	}
	if(action->get(Recruit) > 0) {
		auto troops = player->gettroopscount();
		if(troops < ideal_troops)
			return true;
	}
	if(action->get(Economy) > 0) {
		auto income = player->getincome();
		if(income < ideal_income_maximum)
			return true;
	}
	if(action->get(Support) > 0) {
		auto support = player->getsupport();
		if(support < -1 * player->getfriendlyprovinces())
			return true;
	}
	return false;
}

const action_info* hero_info::choose_action_computer(adat<action_info*>& actions) const {
	auto player = getplayer();
	zshuffle(actions.data, actions.count);
	for(auto a : actions) {
		if(most_needed(player, this, a))
			return a;
	}
	return (action_info*)actions.data[rand() % actions.count];
}