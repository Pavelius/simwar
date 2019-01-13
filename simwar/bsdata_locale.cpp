#include "bsdata.h"
#include "crt.h"

static bool test_true(const char* id, const char** requisits, void* object, const bsreq* type) {
	return true;
}

static bool test_req(const char* id, const char** requisits, void* object, const bsreq* type) {
	if(!requisits[0] || !requisits[1])
		return false;
	char name[128];
	auto index = 0;
	for(unsigned i = 1; requisits[i]; i++) {
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

static void set_req_object(const char* id, const char** requisits, const char** strings, void* object, const bsreq* type) {
	char name[128];
	auto index = 0;
	for(unsigned i = 0; requisits[i]; i++) {
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

static void set_all_bsdata(const char* id, const char** requisits, const char** strings, void* object, const bsreq* type) {
	for(auto pb = bsdata::first; pb; pb = pb->next) {
		auto pfid = pb->fields->find("id", text_type);
		if(!pfid)
			continue;
		for(unsigned i = 0; requisits[i]; i++) {
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

static bool read_localization(const char* url, const char** requisits, void* object, const bsreq* type, bslocal_proc proc, bslocal_test multi_strings) {
	if(!requisits || requisits[0] == 0)
		return false;
	auto pb = (const char*)loadt(url);
	if(!pb)
		return false;
	auto requisits_count = zlen(requisits);
	const int maximum_strings = 32;
	auto p = pb;
	while(*p) {
		char name[128]; name[0] = 0;
		auto index = -1;
		if(isnum(p[0]))
			p = psnum(p, index);
		else
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
		if(multi_strings(name, requisits, object, type)) {
			while(pt[0]) {
				if(pt[0] == '.' && requisits_count > 1) {
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
		proc(name, requisits, strings, object, type);
	}
	delete pb;
	return true;
}

bool bsdata::readl(const char* url, const char** requisits) {
	return read_localization(url, requisits, 0, 0, set_all_bsdata, test_true);
}

bool bsdata::readl(const char* url, const char** requisits, void* object, const bsreq* type) {
	return read_localization(url, requisits, object, type, set_req_object, test_req);
}

static bool inlist(const char* id, const char** list) {
	for(auto pb = list; *pb; pb++) {
		if(strcmp(id, *pb) == 0)
			return true;
	}
	return false;
}

bool bsdata::readl(const char* url, const char* locale_name, parser& errors, const char** prefix, const char** requisits_names, const char** skip_name) {
	char file[260];
	if(!locale_name || locale_name[0] == 0)
		return false;
	const char* error_url = "reading localizations";
	for(auto pb = first; pb; pb = pb->next) {
		if(!pb->isconst())
			continue;
		if(inlist(pb->id, skip_name))
			continue;
		zprint(file, "%1/%3/%2.txt", url, pb->id, locale_name);
		if(pb->getcount() == 1) {
			if(!readl(file, prefix, pb->get(0), pb->fields))
				errors.add(ErrorFile1pNotFound, error_url, 0, 0, file);
			else
				errors.check(file, {pb->get(0), pb->fields});
		} else {
			if(!readl(file, requisits_names))
				errors.add(ErrorFile1pNotFound, error_url, 0, 0, file);
		}
	}
	return errors.getcount() == 0;
}