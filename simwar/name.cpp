#include "main.h"

int name_info::fix(tip_info* ti, const char* name, int value) {
	if(ti && value) {
		char temp[260]; temp[0] = 0;
		if(name) {
			zcpy(temp, name);
			szupper(temp, 1);
		}
		szprint(zend(ti->result), ti->result_max, ti->text, value, temp, (value>=0) ? "+" : "-");
		if(ti->separator)
			zcat(ti->result, ti->separator);
	}
	return value;
}

int name_info::getnum(const void* object, const bsreq* type, const char* id) {
	auto pf = type->find(id);
	if(!pf)
		return 0;
	return pf->get(pf->ptr(object));
}