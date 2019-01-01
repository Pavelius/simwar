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

bool hasvalue(const std::initializer_list<const char*>& values, const char* value) {
	for(auto p : values) {
		if(strcmp(p, value) == 0)
			return true;
	}
	return false;
}

bool bsdata::parser::check_required(const std::initializer_list<const char*> requisits) {
	auto result = true;
	for(auto pb = bsdata::first; pb; pb = pb->next) {
		for(unsigned index = 0; index < pb->getcount(); index++) {
			auto p = pb->get(index);
			bsval bv = {p, pb->fields};
			for(auto pf = pb->fields; *pf; pf++) {
				if(pf->type == text_type) {
					if(!hasvalue(requisits, pf->id))
						continue;
					auto value = pf->get(pf->ptr(p));
					if(!value) {
						auto id = bv.getid();
						errornp(ErrorNotFilled1p, "localization", 0, 0, id);
						result = false;
					}
				}
			}
		}
	}
	return result;
}