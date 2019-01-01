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

bool bsdata::readl(const char* url) {
	auto pb = (const char*)loadt(url);
	if(!pb)
		return false;
	auto p = pb;
	while(*p) {
		char name[128];
		p = psidn(p, name, zendof(name));
		if(p[0] != ':')
			break;
		auto bv = findbyid(name);
		p = zskipsp(p + 1);
		char value[8192];
		p = read_string(p, value, zendof(value));
		if(bv) {
			auto pt = value;
			while(pt[0]) {
				if(pt[0] == '.') {
					pt[0] = 0;
					pt = zskipsp(pt + 1);
					break;
				}
				pt++;
			}
			auto pf = bv.type->find("name", text_type);
			if(pf)
				pf->set(pf->ptr(bv.data), (int)szdup(value));
			pf = bv.type->find("text", text_type);
			if(pf && pt && pt[0])
				pf->set(pf->ptr(bv.data), (int)szdup(pt));
		}
	}
	delete pb;
	return true;
}