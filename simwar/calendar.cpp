#include "main.h"

bsreq calendar_type[] = {
	BSREQ(calendar_info, id, text_type),
	BSREQ(calendar_info, name, text_type),
	BSREQ(calendar_info, text, text_type),
	BSREQ(calendar_info, season, season_type),
{}};
adat<calendar_info, 64>		calendar_data;
BSMETA(calendar);