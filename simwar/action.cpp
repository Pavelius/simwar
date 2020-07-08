#include "main.h"

bsreq action_type[] = {
	BSREQ(actioni, ability, number_type),
	BSREQ(actioni, id, text_type),
	BSREQ(actioni, name, text_type),
	BSREQ(actioni, nameof, text_type),
	BSREQ(actioni, text, text_type),
	BSREQ(actioni, order, number_type),
	BSREQ(actioni, cost, cost_type),
	BSREQ(actioni, cost_per_unit, number_type),
{}};
adat<actioni, 32> action_data; BSMETA(action);

province_flag_s actioni::getprovince() const {
	if(getattack() > 0 || getraid() > 0)
		return NoFriendlyProvince;
	else if(getdefend() || get(Recruit) || get(Economy) || get(Movement))
		return FriendlyProvince;
	return AnyProvince;
}

bool actioni::isplaceable() const {
	return getattack() > 0 || getraid() > 0 || getdefend() > 0
		|| get(Recruit) || get(Support) || get(Economy) || get(Movement);
}

int actioni::compare(const void* p1, const void* p2) {
	auto e1 = *((actioni**)p1);
	auto e2 = *((actioni**)p2);
	return strcmp(e1->name, e2->name);
}

void actioni::sort(actioni** source, unsigned count) {
	qsort(source, count, sizeof(source[0]), compare);
}

unsigned actioni::select(actioni** source, unsigned count, ability_s id) {
	auto ps = source;
	auto pe = ps + count;
	for(auto& e : action_data) {
		if(!e)
			continue;
		if(e.get(id)==0)
			continue;
		if(ps < pe)
			*ps++ = &e;
		else
			break;
	}
	return ps - source;
}

const actioni* actioni::getaction(ability_s id) {
	actioni* source[1];
	auto count = select(source, lenghtof(source), id);
	if(!count)
		return 0;
	return source[0];
}