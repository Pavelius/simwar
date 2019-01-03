#include "main.h"

bsreq unit_type[] = {
	BSREQ(unit_info, id, text_type),
	BSREQ(unit_info, name, text_type),
	BSREQ(unit_info, text, text_type),
	BSREQ(unit_info, nameof, text_type),
	BSREQ(unit_info, attack, number_type),
	BSREQ(unit_info, defend, number_type),
	BSREQ(unit_info, raid, number_type),
	BSREQ(unit_info, cruelty, number_type),
	BSREQ(unit_info, shield, number_type),
	BSREQ(unit_info, cost, number_type),
	BSREQ(unit_info, income, number_type),
{}};
adat<unit_info, 64> unit_data;
BSMETA(unit);