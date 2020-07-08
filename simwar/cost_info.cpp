#include "main.h"

bsreq cost_type[] = {
	BSREQ(costi, gold, number_type),
	BSREQ(costi, fame, number_type),
{}};

void costi::getinfo(stringcreator& sb) const {
	sb.addicon("gold", gold);
	if(fame)
		sb.addicon("flag_grey", fame);
}

void costi::clear() {
	memset(this, 0, sizeof(*this));
}