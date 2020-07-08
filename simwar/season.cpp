#include "main.h"

bsreq season_type[] = {
	BSREQ(seasoni, id, text_type),
	BSREQ(seasoni, name, text_type),
	BSREQ(seasoni, text, text_type),
{}};
adat<seasoni, 64>		season_data;
BSMETA(season);