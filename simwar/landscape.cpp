#include "main.h"

bsreq landscape_type[] = {
	BSREQ(landscape_info, id, text_type),
	BSREQ(landscape_info, name, text_type),
	BSREQ(landscape_info, text, text_type),
	BSREQ(landscape_info, ability, number_type),
	BSREQ(landscape_info, income, number_type),
{}};
adat<landscape_info, 32> landscape_data;
BSMETA(landscape);

int landscape_info::getincome(stringbuilder* ti) const {
	return income;
}