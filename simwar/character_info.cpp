#include "main.h"

bsreq character_type[] = {
	BSREQ(character_info, attack, number_type),
	BSREQ(character_info, defend, number_type),
	BSREQ(character_info, raid, number_type),
	BSREQ(character_info, shield, number_type),
	BSREQ(character_info, sword, number_type),
	BSREQ(character_info, good, number_type),
	BSREQ(character_info, diplomacy, number_type),
	BSREQ(character_info, nobility, number_type),
{}};

int character_info::get(const char* id) const {
	return name_info::getnum(this, character_type, id);
}