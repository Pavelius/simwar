#include "main.h"

bsreq character_type[] = {
	BSREQ(character_info, attack, number_type),
	BSREQ(character_info, defend, number_type),
	BSREQ(character_info, raid, number_type),
	BSREQ(character_info, sword, number_type),
	BSREQ(character_info, shield, number_type),
	BSREQ(character_info, cruelty, number_type),
	BSREQ(character_info, diplomacy, number_type),
	BSREQ(character_info, nobility, number_type),
	BSREQ(character_info, will, number_type),
{}};

int character_info::get(const char* id) const {
	return bsval((void*)this, character_type).get(id).get();
}