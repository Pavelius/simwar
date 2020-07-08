#include "main.h"

adat<tactici, 16> tactic_data;
bsreq tactic_type[] = {
	BSREQ(tactici, id, text_type),
	BSREQ(tactici, name, text_type),
	BSREQ(tactici, text, text_type),
	BSREQ(tactici, ability, number_type),
{}};
BSMETA(tactic);