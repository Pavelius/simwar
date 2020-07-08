#include "main.h"

bsreq troopi::metadata[] = {
	BSREQ(troopi, type, unit_type),
	BSREQ(troopi, province, provincei::metadata),
	BSREQ(troopi, move, provincei::metadata),
	BSREQ(troopi, home, provincei::metadata),
{}};
adat<troopi, 256> troop_data;
bsdata troop_manager("troop", troop_data, troopi::metadata);

playeri* troopi::getplayer() const {
	if(!province)
		return 0;
	return province->getplayer();
}

int troopi::compare(const void* p1, const void* p2) {
	auto e1 = *((troopi**)p1);
	auto e2 = *((troopi**)p2);
	auto s1 = e1->getsort();
	auto s2 = e2->getsort();
	if(s1 < s2)
		return -1;
	else if(s1 > s2)
		return 1;
	return strcmp(e1->type->name, e2->type->name);
}

void troopi::sort(troopi** source, unsigned count) {
	qsort(source, count, sizeof(source[0]), compare);
}

void troopi::getpresent(stringcreator& sb, troopi** objects, unsigned count, const char* addition_text) {
	int count_in_row = 1;
	for(unsigned i = 0; i < count; i++) {
		if(i < count - 1 && compare(objects + i, objects + i + 1) == 0) {
			count_in_row++;
			continue;
		}
		if(count_in_row == 1)
			sb.addn("%+1", objects[i]->getname());
		else
			sb.addn(msg.squads, count_in_row, objects[i]->getnameof());
		if(addition_text)
			sb.adds("(%1)", addition_text);
		count_in_row = 1;
	}
}

provincei* troopi::getprovince(const playeri* player) const {
	if(move && getplayer() == player)
		return move;
	return province;
}

unsigned troopi::remove_moved(troopi** source, unsigned count) {
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

unsigned troopi::remove_restricted(troopi** source, unsigned count, const provincei* province) {
	auto ps = source;
	auto pe = source + count;
	if(!province)
		return 0;
	auto landscape = province->getlandscape();
	for(auto pb = source; pb < pe; pb++) {
		auto p = *pb;
		if(!p->type->is(landscape))
			continue;
		*ps++ = p;
	}
	return ps - source;
}

unsigned troopi::remove(troopi** source, unsigned count, const provincei* province) {
	auto ps = source;
	auto pe = source + count;
	for(auto pb = source; pb < pe; pb++) {
		auto p = *pb;
		if(p->province==province)
			continue;
		*ps++ = p;
	}
	return ps - source;
}

troopi* troopi::add(provincei* province, const uniti* type) {
	auto p = troop_data.add();
	memset(p, 0, sizeof(*p));
	p->province = province;
	p->type = type;
	p->home = province;
	return p;
}

void troopi::clear() {
	memset(this, 0, sizeof(*this));
}

unsigned troopi::select(troopi** result, unsigned result_maximum, const playeri* player) {
	auto ps = result;
	auto pe = ps + result_maximum;
	for(auto& e : troop_data) {
		if(!e)
			continue;
		if(e.getplayer() != player)
			continue;
		if(ps < pe)
			*ps++ = &e;
	}
	return ps - result;
}

unsigned troopi::select(troopi** result, unsigned result_maximum, const provincei* province) {
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

unsigned troopi::select_move(troopi** result, unsigned result_maximum, const provincei* province, const playeri* player) {
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

void troopi::kill(playeri* player) {
	if(home)
		home->addsupport(getplayer(), -type->mourning);
	clear();
}

void troopi::retreat(const provincei* province, const playeri* player) {
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

void troopi::arrival(const provincei* province, const playeri* player) {
	for(auto& e : troop_data) {
		if(!e)
			continue;
		if(!e.move || e.move != province)
			continue;
		if(e.getplayer() != player)
			continue;
		e.province = e.move;
		e.move = 0;
	}
}