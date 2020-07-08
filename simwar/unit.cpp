#include "main.h"

bsreq unit_type[] = {
	BSREQ(uniti, id, text_type),
	BSREQ(uniti, name, text_type),
	BSREQ(uniti, text, text_type),
	BSREQ(uniti, nameof, text_type),
	BSREQ(uniti, ability, number_type),
	BSREQ(uniti, cost, cost_type),
	BSREQ(uniti, income, number_type),
	BSREQ(uniti, ability, number_type),
	BSREQ(uniti, nation, nation_type),
	BSREQ(uniti, mourning, number_type),
	BSREQ(uniti, landscape, landscape_type),
{}};
adat<uniti, 64> unit_data;
BSMETA(unit);

void unit_set::fill(const playeri* player, const provincei* province, const heroi* hero, const actioni* action) {
	auto player_cost = player->cost;
	auto level = 0;
	const landscapei* landscape = 0;
	nationi* nation = 0;
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

bool uniti::is(const landscapei* landscape) const {
	if(this->landscape[0] == 0 || !landscape)
		return true;
	for(auto p : this->landscape) {
		if(p == landscape)
			return true;
	}
	return false;
}

const uniti* uniti::getfirst(const nationi* nation, const landscapei* landscape, int level) {
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

costi unit_set::getcost() const {
	costi result;
	for(auto& e : *this)
		result += e->cost;
	return result;
}