#include "main.h"

bsreq point_type[] = {
	BSREQ(point, x, number_type),
	BSREQ(point, y, number_type),
{}};
bsreq province_info::metadata[] = {
	BSREQ(province_info, id, text_type),
	BSREQ(province_info, name, text_type),
	BSREQ(province_info, text, text_type),
	BSREQ(province_info, player, player_info::metadata),
	BSREQ(province_info, landscape, landscape_type),
	BSREQ(province_info, level, number_type),
	BSREQ(province_info, position, point_type),
	BSREQ(province_info, neighbors, metadata),
{}};
adat<province_info, province_max> province_data;
bsdata province_manager("province", province_data, province_info::metadata);
static unsigned short	province_movement[province_max];

province_flag_s province_info::getstatus(const player_info* player) const {
	if(this->player == player)
		return FriendlyProvince;
	return HostileProvince;
}

int province_info::getincome(tip_info* ti) const {
	auto result = 0;
	if(landscape)
		result += landscape->getincome(ti);
	result += level * game.income_per_level;
	return result;
}

static void clear_movement() {
	memset(province_movement, 0, sizeof(province_movement));
}

void province_info::createwave() {
	clear_movement();
}

hero_info* province_info::gethero(const player_info* player) const {
	for(auto& e : hero_data) {
		if(!e)
			continue;
		if(e.getplayer() != player)
			continue;
		if(e.getprovince() != this)
			continue;
		return &e;
	}
	return 0;
}