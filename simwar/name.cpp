#include "main.h"

int name_info::compare(const void* p1, const void* p2) {
	auto e1 = *((name_info**)p1);
	auto e2 = *((name_info**)p2);
	return strcmp(e1->name, e2->name);
}

int name_info::fix(stringcreator* sb, const char* name, int value) {
	if(sb && value) {
		auto p = sb->get();
		if(p > sb->begin() && p[-1] != '\"')
			sb->add("\n");
		sb->add("[%3%+1i]\t%+2", value, name, (value >= 0) ? "+" : "-");
	}
	return value;
}

int name_info::getnum(const void* object, const bsreq* type, const char* id) {
	auto pf = type->find(id);
	if(!pf)
		return 0;
	return pf->get(pf->ptr(object));
}