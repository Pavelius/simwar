#include "main.h"

bsreq action_type[] = {
	BSREQ(action_info, id, text_type),
	BSREQ(action_info, name, text_type),
	BSREQ(action_info, nameof, text_type),
	BSREQ(action_info, text, text_type),
	BSREQ(action_info, attack, number_type),
	BSREQ(action_info, raid, number_type),
	BSREQ(action_info, defend, number_type),
	BSREQ(action_info, economy, number_type),
	BSREQ(action_info, recruit, number_type),
	BSREQ(action_info, support, number_type),
	BSREQ(action_info, movement, number_type),
	BSREQ(action_info, order, number_type),
	BSREQ(action_info, cost, number_type),
	BSREQ(action_info, trophies, number_type),
	BSREQ(action_info, wait, number_type),
	BSREQ(action_info, good, number_type),
{}};
adat<action_info, 32> action_data; BSMETA(action);

province_flag_s action_info::getprovince() const {
	if(attack > 0 || raid > 0)
		return NoFriendlyProvince;
	else if(defend || recruit || economy || movement)
		return FriendlyProvince;
	return AnyProvince;
}

bool action_info::isplaceable() const {
	return attack > 0 || raid > 0 || defend > 0
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