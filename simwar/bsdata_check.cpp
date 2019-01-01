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

bool bsdata::parser::check_required(const char* url) {
	auto result = true;
	for(auto pb = bsdata::first; pb; pb = pb->next) {
		for(unsigned index = 0; index < pb->getcount(); index++) {
			auto p = pb->get(index);
			bsval bv = {p, pb->fields};
			for(auto pf = pb->fields; *pf; pf++) {
				if(pf->type == text_type) {
					auto value = pf->get(pf->ptr(p));
					if(!value) {
						auto id = bv.getname();
						errornp(ErrorNotFilled1pIn2pRecord3p, url, 0, 0, pf->id, pb->id, id);
						result = false;
					}
				}
			}
		}
	}
	return result;
}