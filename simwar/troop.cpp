#include "main.h"

bsreq troop_info::metadata[] = {
	BSREQ(troop_info, type, unit_type),
	BSREQ(troop_info, player, player_info::metadata),
	BSREQ(troop_info, province, province_info::metadata),
	BSREQ(troop_info, move, province_info::metadata),
{}};
adat<troop_info, 256> troop_data;
bsdata troop_manager("troop", troop_data, troop_info::metadata);

int troop_info::compare(const void* p1, const void* p2) {
	auto e1 = *((troop_info**)p1);
	auto e2 = *((troop_info**)p2);
	auto rs = strcmp(e1->getname(), e2->getname());
	if(rs != 0)
		return rs;
	if(e1->move > e2->move)
		return 1;
	else if(e1->move < e2->move)
		return -1;
	return 0;
}

void troop_info::sort(troop_info** source, unsigned count) {
	qsort(source, count, sizeof(source[0]), compare);
}

const char* troop_info::getpresent(char* result, const char* result_maximum, troop_info** objects, unsigned count) {
	stringcreator sc;
	auto ps = result;
	auto pe = result_maximum;
	ps[0] = 0;
	int count_in_row = 1;
	for(unsigned i = 0; i < count; i++) {
		if(i < count - 1 && strcmp(objects[i]->getname(), objects[i + 1]->getname()) == 0) {
			count_in_row++;
			continue;
		}
		if(ps != result) {
			sc.print(ps, pe, "\n");
			ps = zend(ps);
		}
		if(count_in_row == 1)
			sc.print(ps, pe, objects[i]->getname());
		else
			sc.print(ps, pe, msg.squads, count_in_row, objects[i]->getnameof());
		szupper(ps, 1);
		ps = zend(ps);
		count_in_row = 1;
	}
	return result;
}

province_info* troop_info::getprovince(const player_info* player) const {
	if(this->player == player && move)
		return move;
	return province;
}