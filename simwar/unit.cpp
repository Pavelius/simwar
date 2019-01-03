#include "main.h"

bsreq unit_type[] = {
	BSREQ(unit_info, id, text_type),
	BSREQ(unit_info, name, text_type),
	BSREQ(unit_info, text, text_type),
	BSREQ(unit_info, nameof, text_type),
	BSREQ(unit_info, attack, number_type),
	BSREQ(unit_info, defend, number_type),
	BSREQ(unit_info, raid, number_type),
	BSREQ(unit_info, cruelty, number_type),
	BSREQ(unit_info, shield, number_type),
	BSREQ(unit_info, gold, number_type),
	BSREQ(unit_info, income, number_type),
	BSREQ(unit_info, level, number_type),
	BSREQ(unit_info, recruit_count, number_type),
	BSREQ(unit_info, recruit_time, number_type),
	BSREQ(unit_info, recruit_landscape, landscape_type),
{}};
adat<unit_info, 64> unit_data;
BSMETA(unit);

void unit_set::fill(const player_info* player, const province_info* province, const hero_info* hero, const action_info* action) {
	auto player_cost = player->getcost();
	auto level = 0;
	if(province)
		level = province->geteconomy();
	for(auto& e : unit_data) {
		if(!e)
			continue;
		if(e > player_cost)
			continue;
		if(e.level > level)
			continue;
		auto count = e.recruit_count;
		if(!count)
			count = 1;
		for(int i=0; i<count; i++)
			add(&e);
	}
}

int unit_info::get(const char* id) const {
	return name_info::getnum(this, unit_type, id);
}

cost_info unit_set::getcost() const {
	cost_info result;
	for(auto& e : *this)
		result += *e;
	return result;
}