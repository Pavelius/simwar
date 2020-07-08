#include "main.h"

bsreq calendar_type[] = {
	BSREQ(calendari, id, text_type),
	BSREQ(calendari, name, text_type),
	BSREQ(calendari, text, text_type),
	BSREQ(calendari, season, season_type),
{}};
adat<calendari, 64>		calendar_data;
BSMETA(calendar);