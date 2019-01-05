#include "main.h"

bsreq troop_info::metadata[] = {
	BSREQ(troop_info, type, unit_type),
	BSREQ(troop_info, province, province_info::metadata),
	BSREQ(troop_info, move, province_info::metadata),
{}};
adat<troop_info, 256> troop_data;
bsdata troop_manager("troop", troop_data, troop_info::metadata);

int troop_info::compare(const void* p1, const void* p2) {
	auto e1 = *((troop_info**)p1);
	auto e2 = *((troop_info**)p2);
	auto s1 = e1->getsort();
	auto s2 = e2->getsort();
	if(s1 < s2)
		return -1;
	else if(s1 > s2)
		return 1;
	return strcmp(e1->type->name, e2->type->name);
}

void troop_info::sort(troop_info** source, unsigned count) {
	qsort(source, count, sizeof(source[0]), compare);
}

const char* troop_info::getpresent(char* result, const char* result_maximum, troop_info** objects, unsigned count, const char* addition_text) {
	stringcreator sc;
	auto ps = result;
	auto pe = result_maximum;
	ps[0] = 0;
	int count_in_row = 1;
	for(unsigned i = 0; i < count; i++) {
		if(i < count - 1 && compare(objects + i, objects + i + 1) == 0) {
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
		if(addition_text) {
			sc.print(ps, pe, " (%1)", addition_text);
			ps = zend(ps);
		}
		count_in_row = 1;
	}
	return result;
}

province_info* troop_info::getprovince(const player_info* player) const {
	if(move && getplayer() == player)
		return move;
	return province;
}

unsigned troop_info::remove_moved(troop_info** source, unsigned count) {
	auto ps = source;
	auto pe = source + count;
	for(auto pb = source; pb < pe; pb++) {
		auto p = *pb;
		if(p->move)
			continue;
		*ps++ = p;
	}
	return ps - source;
}

troop_info* troop_info::add(province_info* province, unit_info* type) {
	auto p = troop_data.add();
	memset(p, 0, sizeof(*p));
	p->province = province;
	p->type = type;
	return p;
}

void troop_info::clear() {
	memset(this, 0, sizeof(*this));
}

unsigned troop_info::select(troop_info** result, unsigned result_maximum, const province_info* province) {
	auto ps = result;
	auto pe = ps + result_maximum;
	for(auto& e : troop_data) {
		if(!e)
			continue;
		if(e.province != province)
			continue;
		if(ps < pe)
			*ps++ = &e;
	}
	return ps - result;
}

unsigned troop_info::select_move(troop_info** result, unsigned result_maximum, const province_info* province, const player_info* player) {
	auto ps = result;
	auto pe = ps + result_maximum;
	for(auto& e : troop_data) {
		if(!e)
			continue;
		if(e.move != province)
			continue;
		if(e.getplayer() != player)
			continue;
		if(ps < pe)
			*ps++ = &e;
	}
	return ps - result;
}

void troop_info::kill(player_info* player) {
	clear();
}

void troop_info::retreat(const province_info* province, const player_info* player) {
	for(auto& e : troop_data) {
		if(!e)
			continue;
		if(!e.move || e.move != province)
			continue;
		if(e.getplayer() != player)
			continue;
		e.move = 0;
	}
}