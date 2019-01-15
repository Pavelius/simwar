#include "main.h"

bsreq unit_type[] = {
	BSREQ(unit_info, id, text_type),
	BSREQ(unit_info, name, text_type),
	BSREQ(unit_info, text, text_type),
	BSREQ(unit_info, nameof, text_type),
	BSREQ(unit_info, ability, number_type),
	BSREQ(unit_info, cost, cost_type),
	BSREQ(unit_info, income, number_type),
	BSREQ(unit_info, ability, number_type),
	BSREQ(unit_info, nation, nation_type),
	BSREQ(unit_info, mourning, number_type),
	BSREQ(unit_info, landscape, landscape_type),
{}};
adat<unit_info, 64> unit_data;
BSMETA(unit);

void unit_set::fill(const player_info* player, const province_info* province, const hero_info* hero, const action_info* action) {
	auto player_cost = player->cost;
	auto level = 0;
	const landscape_info* landscape = 0;
	nation_info* nation = 0;
	if(province) {
		level = province->getlevel();
		nation = province->getnation();
		landscape = province->getlandscape();
	}
	for(auto& e : unit_data) {
		if(!e)
			continue;
		if(e.cost > player_cost)
			continue;
		if(e.get(Level) > level)
			continue;
		if(e.nation && e.nation != nation)
			continue;
		if(!e.is(landscape))
			continue;
		auto count = e.get(Recruit);
		if(!count)
			count = 1;
		for(int i=0; i<count; i++)
			add(&e);
	}
}

bool unit_info::is(const landscape_info* landscape) const {
	if(this->landscape[0] == 0 || !landscape)
		return true;
	for(auto p : this->landscape) {
		if(p == landscape)
			return true;
	}
	return false;
}

const unit_info* unit_info::getfirst(const nation_info* nation, const landscape_info* landscape, int level) {
	for(auto& e : unit_data) {
		if(!e)
			continue;
		if(e.get(Level) != level)
			continue;
		if(e.nation != nation)
			continue;
		if(!e.is(landscape))
			continue;
		return &e;
	}
	return 0;
}

cost_info unit_set::getcost() const {
	cost_info result;
	for(auto& e : *this)
		result += e->cost;
	return result;
}