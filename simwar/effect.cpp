#include "main.h"

bsreq effect_type[] = {
	BSREQ(effect_info, test, ability_type),
	BSREQ(effect_info, text, text_type),
	BSREQ(effect_info, ability, number_type),
	BSREQ(effect_info, units, unit_type),
{}};

bool effect_info::isvalid(const hero_info& e) const {
	if(e.get(test) < 1)
		return false;
	return true;
}