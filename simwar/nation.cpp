#include "main.h"

bsreq nation_type[] = {
	BSREQ(nationi, id, text_type),
	BSREQ(nationi, name, text_type),
	BSREQ(nationi, text, text_type),
{}};
adat<nationi, 16> nation_data;
BSMETA(nation);
