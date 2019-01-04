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
	if(!this->player)
		return NeutralProvince;
	return NoFriendlyProvince;
}

int province_info::getdefend() const {
	return landscape->defend + level;
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

unsigned province_info::select(province_info** source, unsigned maximum, const player_info* player, province_flag_s state) {
	auto ps = source;
	auto pe = ps + maximum;
	for(auto& e : province_data) {
		if(!e)
			continue;
		switch(state) {
		case NoFriendlyProvince:
			if(e.getstatus(player) == FriendlyProvince)
				continue;
			break;
		case FriendlyProvince:
			if(e.getstatus(player) != FriendlyProvince)
				continue;
			break;
		case NeutralProvince:
			if(e.getstatus(player) != NeutralProvince)
				continue;
			break;
		default:
			break;
		}
		if(ps < pe)
			*ps++ = &e;
	}
	return ps - source;
}

unsigned province_info::remove_hero_present(aref<province_info*> source, const player_info* player) {
	auto ps = source.data;
	for(auto p : source) {
		if(p->gethero(player))
			continue;
		*ps++ = p;
	}
	return ps - source.data;
}

char* province_info::getinfo(char* result, const char* result_maximum, bool show_landscape) const {
	result[0] = 0;
	if(show_landscape)
		szprint(zend(result), result_maximum, "%1 ", landscape->name);
	szprint(zend(result), result_maximum, ":gold:%1i :house:%2i :shield_grey:%3i",
		getincome(), getlevel(), getdefend());
	return result;
}

void province_info::build(unit_info* unit, int turns) {
	if(turns < 1)
		turns = 1;
	auto p = build_data.add();
	p->province = this;
	p->unit = unit;
	p->wait = turns;
}

void province_info::add(unit_info* unit) {
	troop_info::add(this, unit);
}

void province_info::change_support() {
	for(auto& e : province_data) {
		if(!e)
			continue;
		auto player = e.player;
		auto player_index = player_data.indexof(player);
		for(auto i = 0; i < sizeof(e.support) / sizeof(e.support[0]); i++) {
			auto value = e.support[i];
			if(player_index == i) {
				if(value < game.support_maximum)
					value++;
			} else if(value > 0) {
				if(value > game.support_minimum)
					value--;
			} else if(value < 0)
				value++;
			e.support[i] = value;
		}
	}
}