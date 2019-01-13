#include "main.h"

bsreq trait_type[] = {
	BSREQ(trait_info, id, text_type),
	BSREQ(trait_info, name, text_type),
	BSREQ(trait_info, text, text_type),
	BSREQ(trait_info, ability, number_type),
{}};
adat<trait_info, 64> trait_data;
BSMETA(trait);