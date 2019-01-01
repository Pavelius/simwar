#include "main.h"

bsreq combat_type[] = {
	BSREQ(unit_info, attack, number_type),
	BSREQ(unit_info, defend, number_type),
	BSREQ(unit_info, raid, number_type),
	BSREQ(unit_info, sword, number_type),
	BSREQ(unit_info, shield, number_type),
{}};

int combat_info::get(const char* id) const {
	return bsval((void*)this, combat_type).get(id).get();
}