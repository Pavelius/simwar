#include "bsdata.h"
#include "crt.h"

void bsdata::parser::errornp(bsparse_error_s id, const char* url, int line, int column, ...) {
	error(id, url, line, column, xva_start(column));
}

bool bsdata::parser::check_required(const char* url, bsval source) {
	auto result = true;
	for(auto pf = source.type; *pf; pf++) {
		if(pf->type == text_type) {
			auto value = pf->get(pf->ptr(source.data));
			if(!value) {
				errornp(ErrorNotFilled1p, url, 0, 0, pf->id);
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

bool bsdata::parser::check_required(const bsdata::requisit* requisits, unsigned requisits_count) {
	auto result = true;
	for(auto pb = bsdata::first; pb; pb = pb->next) {
		for(unsigned index = 0; index < pb->getcount(); index++) {
			auto p = pb->get(index);
			bsval bv = {p, pb->fields};
			auto id = bv.getid();
			for(auto pf = pb->fields; *pf; pf++) {
				auto pr = findr(requisits, requisits_count, pf->id);
				if(!pr)
					continue;
				auto value = pf->get(pf->ptr(p));
				if(pr->required && !value) {
					errornp(ErrorNotFilled1p, "localization", 0, 0, id);
					result = false;
				}
				if(pr->values[0] != pr->values[1]) {
					if(value<pr->values[0] || value>pr->values[1]) {
						errornp(ErrorValue1pIn2pRecord3pMustBeIn4pAnd5p, "localization", 0, 0, pf->id, pb->id, id, pr->values[0], pr->values[1]);
						result = false;
					}
				}
			}
		}
	}
	return result;
}