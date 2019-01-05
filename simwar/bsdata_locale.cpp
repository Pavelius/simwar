#include "bsdata.h"
#include "crt.h"

static const char* read_string(const char* p, char* ps, const char* pe) {
	while(p[0] && p[0] != '\n' && p[0] != '\r') {
		if(ps < pe)
			*ps++ = *p;
		p++;
	}
	ps[0] = 0;
	while(p[0] == '\n' || p[0] == '\r') {
		p = szskipcr(p);
		p = zskipsp(p);
	}
	return p;
}

bool bsdata::readl(const char* url, const char** requisits, unsigned requisits_count) {
	auto pb = (const char*)loadt(url);
	if(!pb)
		return false;
	const int maximum_strings = 32;
	if(requisits_count > maximum_strings)
		requisits_count = maximum_strings;
	auto p = pb;
	while(*p) {
		char name[128];
		p = psidn(p, name, zendof(name));
		if(p[0] != ':')
			break;
		p = zskipsp(p + 1);
		char value[8192];
		p = read_string(p, value, zendof(value));
		const char* strings[maximum_strings] = {};
		auto count = 0;
		auto pt = value;
		strings[0] = pt;
		while(pt[0]) {
			if(pt[0] == '.') {
				pt[0] = 0;
				pt = zskipsp(pt + 1);
				if(requisits_count > 1)
					strings[requisits_count - 1] = pt;
				break;
			} else if(pt[0] == '|') {
				pt[0] = 0;
				pt = zskipsp(pt + 1);
				if(count < 32)
					count++;
				strings[count] = pt;
				continue;
			}
			pt++;
		}
		// Поиск и модификация всех записей во всех таблицах с одинаковым id
		for(auto pb = bsdata::first; pb; pb = pb->next) {
			auto pfid = pb->fields->find("id", text_type);
			if(!pfid)
				continue;
			for(unsigned i = 0; i < requisits_count; i++) {
				auto pf = pb->fields->find(requisits[i], text_type);
				if(!pf)
					continue;
				if(!strings[i])
					continue;
				auto string_value = szdup(strings[i]);
				for(unsigned j = 0; j < pb->count; j++) {
					auto pv = pb->get(j);
					auto j_id = (const char*)pfid->get(pfid->ptr(pv));
					if(!j_id)
						continue;
					if(strcmp(j_id, name) != 0)
						continue;
					pf->set(pf->ptr(pv), (int)string_value);
				}
			}
		}
	}
	delete pb;
	return true;
}