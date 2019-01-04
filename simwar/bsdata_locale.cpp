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
		char name[256];
		p = psidn(p, name, zendof(name));
		if(p[0] != ':')
			break;
		auto bv = findbyid(name);
		p = zskipsp(p + 1);
		char value[8192];
		p = read_string(p, value, zendof(value));
		if(bv) {
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
			// ���������� ����, ��� ����������
			for(unsigned i = 0; i < requisits_count; i++) {
				auto pf = bv.type->find(requisits[i], text_type);
				if(!pf)
					continue;
				auto pv = strings[i];
				if(!pv)
					continue;
				pf->set(pf->ptr(bv.data), (int)szdup(pv));
			}
		}
	}
	delete pb;
	return true;
}