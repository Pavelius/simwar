#include "bsdata.h"
#include "crt.h"

typedef bool(*bslocal_test)(const char* id, const char** requisits, const bsdata_strings* tables, void* object, const bsreq* type, int index);
typedef void(*bslocal_proc)(const char* id, const char** requisits, const bsdata_strings* tables, const char** strings, void* object, const bsreq* type, int index);

const int maximum_strings = 32;

static bool test_true(const char* id, const char** requisits, const bsdata_strings* tables, void* object, const bsreq* type, int index) {
	if(index != -1)
		return false;
	return true;
}

static bool test_req(const char* id, const char** requisits, const bsdata_strings* tables, void* object, const bsreq* type, int index) {
	if(index != -1)
		return false;
	if(!requisits[0] || !requisits[1])
		return false;
	char name[128];
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

static void set_req_object(const char* id, const char** requisits, const bsdata_strings* tables, const char** strings, void* object, const bsreq* type, int index) {
	char name[128];
	auto ni = 0;
	for(unsigned i = 0; requisits[i]; i++) {
		zprint(name, "%1%2", id, requisits[i]);
		auto pf = type->find(name, text_type);
		if(!pf)
			continue;
		if(!strings[ni] || strings[ni][0] == 0)
			break;
		type->set(pf->ptr(object), (int)szdup(strings[ni]));
		ni++;
	}
}

static void set_all_bsdata(const char* id, const char** requisits, const bsdata_strings* tables, const char** strings, void* object, const bsreq* type, int index) {
	for(auto pb = bsdata::first; pb; pb = pb->next) {
		auto pfid = pb->fields->find("id", text_type);
		if(!pfid)
			continue;
		if(index == -1) {
			for(unsigned i = 0; requisits[i]; i++) {
				if(!strings[i])
					continue;
				auto pf = pb->fields->find(requisits[i], text_type);
				if(!pf)
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
		} else {
			if(!tables)
				continue;
			for(auto pr = tables; *pr; pr++) {
				if(!pr->requisits)
					continue;
				if(pr->isrange() && (index<pr->range[0] || index > pr->range[1]))
					continue;
				auto pf = pb->fields->find(pr->id);
				if(!pf)
					continue;
				if(pf->reference)
					continue;
				unsigned ib = index - pr->range[0];
				if(ib >= pf->count)
					continue;
				auto pft = pf->type;
				if(pft == number_type)
					continue;
				auto pid = pb->fields->getkey();
				if(!pid)
					continue;
				auto pv = pb->find(pid, id);
				if(!pv)
					continue;
				pv = (char*)pf->ptr(pv) + ib * pf->size;
				for(unsigned i = 0; pr->requisits[i] && i<maximum_strings; i++) {
					if(!strings[i])
						continue;
					auto pf = pft->find(pr->requisits[i], text_type);
					if(!pf)
						continue;
					auto string_value = szdup(strings[i]);
					pf->set(pf->ptr(pv), (int)string_value);
				}
			}
		}
	}
}

static bool read_localization(const char* url, const char** requisits, const bsdata_strings* tables, void* object, const bsreq* type, bslocal_proc proc, bslocal_test multi_strings) {
	if(!requisits || requisits[0] == 0)
		return false;
	auto pb = (const char*)loadt(url);
	if(!pb)
		return false;
	auto requisits_count = zlen(requisits);
	auto p = pb;
	char name[128];
	while(*p) {
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
		auto ms = false;
		if(pt[0] == ':') {
			ms = false;
			pt = zskipsp(pt + 1);
		} else
			ms = multi_strings(name, requisits, tables, object, type, index);
		strings[0] = pt;
		if(ms) {
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
		proc(name, requisits, tables, strings, object, type, index);
	}
	delete pb;
	return true;
}

bool bsdata::readl(const char* url, const char** requisits, const bsdata_strings* tables) {
	return read_localization(url, requisits, tables, 0, 0, set_all_bsdata, test_true);
}

bool bsdata::readl(const char* url, const char** requisits, void* object, const bsreq* type) {
	return read_localization(url, requisits, 0, object, type, set_req_object, test_req);
}

static bool inlist(const char* id, const char** list) {
	for(auto pb = list; *pb; pb++) {
		if(strcmp(id, *pb) == 0)
			return true;
	}
	return false;
}

bool bsdata::readl(const char* url, const char* locale_name, parser& errors, const char** prefix, const char** requisits_names, const bsdata_strings* tables, const char** skip_name) {
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
			if(!readl(file, requisits_names, tables))
				errors.add(ErrorFile1pNotFound, error_url, 0, 0, file);
		}
	}
	return errors.getcount() == 0;
}