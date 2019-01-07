#include "main.h"

bsreq season_type[] = {
	BSREQ(season_info, id, text_type),
	BSREQ(season_info, name, text_type),
	BSREQ(season_info, text, text_type),
{}};
adat<season_info, 64>		season_data;
BSMETA(season);