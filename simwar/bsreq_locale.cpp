#include "bsreq.h"
#include "crt.h"

static const char* read_string(const char* p, char* ps, const char* pe) {
	while(p[0] && p[0]!='\n' && p[0] != '\r') {
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

bool bsreq::readl(const char* url, void* object) const {
	auto pb = (const char*)loadt(url);
	if(!pb)
		return false;
	auto p = pb;
	while(*p) {
		char name[128];
		p = psidn(p, name, zendof(name));
		if(p[0] != ':')
			break;
		auto pf = find(name);
		p = zskipsp(p + 1);
		char value[4096];
		p = read_string(p, value, zendof(value));
		if(pf) {
			if(pf->type==text_type)
				set(pf->ptr(object), (int)szdup(value));
		}
	}
	delete pb;
	return true;
}