#include "main.h"

bsreq combat_type[] = {
	BSREQ(combat_info, attack, number_type),
	BSREQ(combat_info, defend, number_type),
	BSREQ(combat_info, raid, number_type),
	BSREQ(combat_info, sword, number_type),
	BSREQ(combat_info, shield, number_type),
{}};

int combat_info::get(const char* id) const {
	return name_info::getnum(this, combat_type, id);
}