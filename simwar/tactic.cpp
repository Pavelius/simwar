#include "main.h"

tactic_info tactic_data[] = {{"lighting_attack"},
{"careful_defence"},
{"messy_fighting"},
};
bsreq tactic_type[] = {
	BSREQ(tactic_info, id, text_type),
	BSREQ(tactic_info, name, text_type),
	BSREQ(tactic_info, text, text_type),
	BSREQ(tactic_info, attack, number_type),
	BSREQ(tactic_info, defend, number_type),
	BSREQ(tactic_info, raid, number_type),
	BSREQ(tactic_info, cruelty, number_type),
	BSREQ(tactic_info, shield, number_type),
{}};
BSMETA(tactic);