#include "main.h"

bsreq build_type[] = {
	BSREQ(build_info, unit, unit_type),
	BSREQ(build_info, province, province_info::metadata),
	BSREQ(build_info, wait, number_type),
{}};
adat<build_info, 256> build_data;
BSMETA(build);

int build_info::compare(const void* p1, const void* p2) {
	auto e1 = *((build_info**)p1);
	auto e2 = *((build_info**)p2);
	auto rs = strcmp(e1->unit->name, e2->unit->name);
	if(rs != 0)
		return rs;
	if(e1->wait < e2->wait)
		return -1;
	else if(e1->wait > e2->wait)
		return 1;
	return 0;
}

void build_info::sort(build_info** source, unsigned count) {
	qsort(source, count, sizeof(source[0]), compare);
}

void build_info::getpresent(stringcreator& sb, build_info** objects, unsigned count) {
	int count_in_row = 1;
	for(unsigned i = 0; i < count; i++) {
		if(i < count - 1 && compare(objects + i, objects + i + 1) == 0) {
			count_in_row++;
			continue;
		}
		if(count_in_row == 1)
			sb.addn("%+1", objects[i]->unit->getname());
		else
			sb.addn(msg.squads, count_in_row, objects[i]->unit->nameof);
		sb.adds("(%1i %2)", objects[i]->wait, msg.turns);
		count_in_row = 1;
	}
}

unsigned build_info::select(build_info** source, unsigned maximum, const province_info* province) {
	auto ps = source;
	auto pe = ps + maximum;
	for(auto& e : build_data) {
		if(!e)
			continue;
		if(e.province != province)
			continue;
		if(ps < pe)
			*ps++ = &e;
	}
	return ps - source;
}

void build_info::build_units() {
	for(auto& e : build_data) {
		if(!e)
			continue;
		if(e.wait>0)
			e.wait--;
		if(e.wait==0) {
			e.province->add(e.unit);
			e.clear();
		}
	}
}

void build_info::clear() {
	memset(this, 0, sizeof(*this));
}