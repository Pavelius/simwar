#include "main.h"

bsreq combat_type[] = {
	BSREQ(combat_info, attack, number_type),
	BSREQ(combat_info, defend, number_type),
	BSREQ(combat_info, raid, number_type),
	BSREQ(combat_info, cruelty, number_type),
	BSREQ(combat_info, shield, number_type),
{}};

int combat_info::get(const char* id) const {
	return bsval((void*)this, combat_type).get(id).get();
}