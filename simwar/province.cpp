#include "main.h"

bsreq point_type[] = {
	BSREQ(point, x, number_type),
	BSREQ(point, y, number_type),
{}};
bsreq province_type[] = {
	BSREQ(province_info, id, text_type),
	BSREQ(province_info, name, text_type),
	BSREQ(province_info, text, text_type),
	BSREQ(province_info, player, player_info::metadata),
	BSREQ(province_info, landscape, landscape_type),
	BSREQ(province_info, level, number_type),
	BSREQ(province_info, position, point_type),
{}};
adat<province_info, province_max> province_data; BSMETA(province);

int province_info::getincome(tip_info* ti) const {
	auto result = 0;
	if(landscape)
		result += landscape->getprofit(ti);
	result += level * game.income_per_level;
	return result;
}