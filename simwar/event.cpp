#include "main.h"

bsreq event_type[] = {
	BSREQ(event_info, id, text_type),
	BSREQ(event_info, name, text_type),
	BSREQ(event_info, text, text_type),
	BSREQ(event_info, landscape, landscape_type),
	BSREQ(event_info, effects, effect_type),
{}};
adat<event_info, 64> event_data;
BSMETA(event);