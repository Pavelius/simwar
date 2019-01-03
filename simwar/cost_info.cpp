#include "main.h"

char* cost_info::get(char* result, const char* result_maximum) const {
	result[0] = 0;
	szprint(result, result_maximum, ":gold:%1i", gold);
	//if(income)
	//	szprint(zend(result), result_maximum, "[%2%1i]", income, (income>=0) ? "+" : "-");
	return result;
}

void cost_info::clear() {
	memset(this, 0, sizeof(*this));
}