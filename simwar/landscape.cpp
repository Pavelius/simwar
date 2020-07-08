#include "main.h"

bsreq landscape_type[] = {
	BSREQ(landscapei, id, text_type),
	BSREQ(landscapei, name, text_type),
	BSREQ(landscapei, text, text_type),
	BSREQ(landscapei, ability, number_type),
	BSREQ(landscapei, income, number_type),
{}};
adat<landscapei, 32> landscape_data;
BSMETA(landscape);

int landscapei::getincome(stringcreator* ti) const {
	return income;
}