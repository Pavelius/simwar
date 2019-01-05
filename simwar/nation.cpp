#include "main.h"

bsreq nation_type[] = {
	BSREQ(nation_info, id, text_type),
	BSREQ(nation_info, name, text_type),
	BSREQ(nation_info, text, text_type),
{}};
adat<nation_info, 16> nation_data;
BSMETA(nation);
