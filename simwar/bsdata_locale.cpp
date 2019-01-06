#include "bsdata.h"
#include "crt.h"

typedef void(*bslocal_proc)(const char* id, const char** requisits, unsigned requisits_count, const char** strings, void* object, const bsreq* type);
typedef bool(*bslocal_test)(const char* id, const char** requisits, unsigned requisits_count, void* object, const bsreq* type);

static bool test_true(const char* id, const char** requisits, unsigned requisits_count, void* object, const bsreq* type) {
	return true;
}

static bool test_req(const char* id, const char** requisits, unsigned requisits_count, void* object, const bsreq* type) {
	if(requisits_count < 2)
		return false;
	char name[128];
	auto index = 0;
	for(unsigned i = 1; i < requisits_count; i++) {
		zprint(name, "%1%2", id, requisits[i]);
		auto pf = type->find(name, text_type);
		if(!pf)
			continue;
		return true;
	}
	return false;
}

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

static void set_req_object(const char* id, const char** requisits, unsigned requisits_count, const char** strings, void* object, const bsreq* type) {
	char name[128];
	auto index = 0;
	for(unsigned i = 0; i < requisits_count; i++) {
		zprint(name, "%1%2", id, requisits[i]);
		auto pf = type->find(name, text_type);
		if(!pf)
			continue;
		if(!strings[index] || strings[index][0] == 0)
			break;
		type->set(pf->ptr(object), (int)szdup(strings[index]));
		index++;
	}
}

static void set_all_bsdata(const char* id, const char** requisits, unsigned requisits_count, const char** strings, void* object, const bsreq* type) {
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
				if(strcmp(j_id, id) != 0)
					continue;
				pf->set(pf->ptr(pv), (int)string_value);
			}
		}
	}
}

static bool read_localization(const char* url, const char** requisits, unsigned requisits_count, void* object, bsreq* type, bslocal_proc proc, bslocal_test multi_strings) {
	if(!requisits_count)
		return false;
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
		if(multi_strings(name, requisits, requisits_count, object, type)) {
			while(pt[0]) {
				if(pt[0] == '.' && requisits_count>1) {
					pt[0] = 0;
					pt = zskipsp(pt + 1);
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
		}
		proc(name, requisits, requisits_count, strings, object, type);
	}
	delete pb;
	return true;
}

bool bsdata::readl(const char* url, const char** requisits, unsigned requisits_count) {
	return read_localization(url, requisits, requisits_count, 0, 0, set_all_bsdata, test_true);
}

bool bsdata::readl(const char* url, const char** requisits, unsigned requisits_count, void* object, bsreq* type) {
	return read_localization(url, requisits, requisits_count, object, type, set_req_object, test_req);
}