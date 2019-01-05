#include "main.h"

bsreq trait_type[] = {
	BSREQ(trait_info, id, text_type),
	BSREQ(trait_info, name, text_type),
	BSREQ(trait_info, text, text_type),
	BSREQ(trait_info, attack, number_type),
	BSREQ(trait_info, defend, number_type),
	BSREQ(trait_info, raid, number_type),
	BSREQ(trait_info, shield, number_type),
	BSREQ(trait_info, sword, number_type),
	BSREQ(trait_info, cruelty, number_type),
	BSREQ(trait_info, diplomacy, number_type),
	BSREQ(trait_info, nobility, number_type),
	BSREQ(trait_info, will, number_type),
{}};
adat<trait_info, 64> trait_data; BSMETA(trait);