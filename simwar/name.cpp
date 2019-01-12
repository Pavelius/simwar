#include "main.h"

int name_info::compare(const void* p1, const void* p2) {
	auto e1 = *((name_info**)p1);
	auto e2 = *((name_info**)p2);
	return strcmp(e1->name, e2->name);
}

int name_info::fix(tip_info* ti, const char* name, int value) {
	if(ti && value) {
		char temp[260]; temp[0] = 0;
		if(name) {
			zcpy(temp, name);
			szupper(temp, 1);
		}
		szprint(zend(ti->result), ti->result_max, "[%3%+1i]\t%2\n", value, temp, (value>=0) ? "+" : "-");
	}
	return value;
}

int name_info::getnum(const void* object, const bsreq* type, const char* id) {
	auto pf = type->find(id);
	if(!pf)
		return 0;
	return pf->get(pf->ptr(object));
}