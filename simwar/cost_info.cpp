#include "main.h"

bsreq cost_type[] = {
	BSREQ(cost_info, gold, number_type),
	BSREQ(cost_info, fame, number_type),
{}};

void cost_info::getinfo(stringcreator& sb) const {
	sb.addicon("gold", gold);
	if(fame)
		sb.addicon("flag_grey", fame);
}

void cost_info::clear() {
	memset(this, 0, sizeof(*this));
}