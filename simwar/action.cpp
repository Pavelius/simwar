#include "main.h"

bsreq action_type[] = {
	BSREQ(action_info, id, text_type),
	BSREQ(action_info, name, text_type),
	BSREQ(action_info, nameof, text_type),
	BSREQ(action_info, text, text_type),
	BSREQ(action_info, ability, number_type),
	BSREQ(action_info, economy, number_type),
	BSREQ(action_info, recruit, number_type),
	BSREQ(action_info, support, number_type),
	BSREQ(action_info, movement, number_type),
	BSREQ(action_info, order, number_type),
	BSREQ(action_info, cost, cost_type),
	BSREQ(action_info, cost_per_unit, number_type),
	BSREQ(action_info, trophies, cost_type),
	BSREQ(action_info, wait, number_type),
{}};
adat<action_info, 32> action_data; BSMETA(action);

province_flag_s action_info::getprovince() const {
	if(getattack() > 0 || getraid() > 0)
		return NoFriendlyProvince;
	else if(getdefend() || recruit || economy || movement)
		return FriendlyProvince;
	return AnyProvince;
}

bool action_info::isplaceable() const {
	return getattack() > 0 || getraid() > 0 || getdefend() > 0
		|| recruit || support || economy|| movement;
}

int action_info::compare(const void* p1, const void* p2) {
	auto e1 = *((action_info**)p1);
	auto e2 = *((action_info**)p2);
	return strcmp(e1->name, e2->name);
}

void action_info::sort(action_info** source, unsigned count) {
	qsort(source, count, sizeof(source[0]), compare);
}

unsigned action_info::select(action_info** source, unsigned count, char attack, char defend, char raid) {
	auto ps = source;
	auto pe = ps + count;
	for(auto& e : action_data) {
		if(!e)
			continue;
		if(e.get(Attack) < attack)
			continue;
		if(e.get(Defend) < defend)
			continue;
		if(e.get(Raid) < raid)
			continue;
		if(ps<pe)
			*ps++ = &e;
	}
	return ps - source;
}

const action_info* action_info::getaction(char attack, char defend, char raid) {
	action_info* source[1];
	auto count = select(source, lenghtof(source), attack, defend, raid);
	if(!count)
		return 0;
	return source[0];
}