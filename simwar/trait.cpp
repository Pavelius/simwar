#include "main.h"

bsreq trait_type[] = {
	BSREQ(traiti, id, text_type),
	BSREQ(traiti, name, text_type),
	BSREQ(traiti, text, text_type),
	BSREQ(traiti, ability, number_type),
{}};
adat<traiti, 64> trait_data;
BSMETA(trait);