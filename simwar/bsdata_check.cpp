#include "bsdata.h"
#include "crt.h"

void bsdata::parser::add(bsparse_error_s id, const char* url, int line, int column, ...) {
	error(id, url, line, column, xva_start(column));
	count++;
}

bool bsdata::parser::check(const char* url, bsval source) {
	auto result = true;
	for(auto pf = source.type; *pf; pf++) {
		if(pf->type == text_type) {
			auto value = pf->get(pf->ptr(source.data));
			if(!value) {
				add(ErrorNotFilled1pIn2pRecord3p, url, 0, 0, "", "", pf->id);
				result = false;
			}
		}
	}
	return result;
}

const bsdata::requisit* findr(const bsdata::requisit* requisits, unsigned requisits_count, const char* id) {
	for(unsigned i = 0; i < requisits_count; i++) {
		if(strcmp(requisits[i].id, id) == 0)
			return requisits + i;
	}
	return 0;
}

bool bsdata::parser::check(const bsdata::requisit* requisits, unsigned requisits_count) {
	auto result = true;
	auto pte = requisits + requisits_count;
	for(auto pr = requisits; pr < pte; pr++) {
		for(auto pb = bsdata::first; pb; pb = pb->next) {
			if(pr->type && pr->type != pb->fields)
				continue;
			auto pf = pb->fields->find(pr->id);
			if(!pf)
				continue;
			for(unsigned index = 0; index < pb->getcount(); index++) {
				auto p = pb->get(index);
				bsval bv = {p, pb->fields};
				auto id = bv.getid();
				auto value = pf->get(pf->ptr(p));
				if(pr->required && !value) {
					add(ErrorNotFilled1pIn2pRecord3p, "localization", 0, 0, pf->id, pb->id, id);
					result = false;
				}
				if(pr->values[0] != pr->values[1]) {
					if(value<pr->values[0] || value>pr->values[1]) {
						add(ErrorValue1pIn2pRecord3pMustBeIn4pAnd5p, "localization", 0, 0, pf->id, pb->id, id, pr->values[0], pr->values[1]);
						result = false;
					}
				}
			}
		}
	}
	return result;
}