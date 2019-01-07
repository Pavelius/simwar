#include "main.h"

adat<tactic_info, 16> tactic_data;
bsreq tactic_type[] = {
	BSREQ(tactic_info, id, text_type),
	BSREQ(tactic_info, name, text_type),
	BSREQ(tactic_info, text, text_type),
	BSREQ(tactic_info, attack, number_type),
	BSREQ(tactic_info, defend, number_type),
	BSREQ(tactic_info, raid, number_type),
	BSREQ(tactic_info, sword, number_type),
	BSREQ(tactic_info, shield, number_type),
{}};
BSMETA(tactic);