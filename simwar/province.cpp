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
unsigned char			province_order[province_max];
bsdata					province_manager("province", province_data, province_info::metadata);
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

const char* province_info::getsupport(char* result, const char* result_maximum) const {
	for(unsigned i = 0; i < player_data.count; i++) {
		auto& e = player_data.data[i];
		if(!e)
			continue;
		if(support[i] == 0)
			continue;
		szprint(zend(result), result_maximum, " %1: [%3%2i]", e.getname(), support[i], (support[i] >= 0) ? "+" : "-");
	}
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
		auto player = e.getplayer();
		auto player_index = player_data.indexof(player);
		for(auto i = 0; i < sizeof(support) / sizeof(support[0]); i++) {
			auto value = e.support[i];
			if(player_index != i) {
				if(value > 0)
					value--;
			}
			e.support[i] = value;
		}
	}
}

int	province_info::getsupport(const player_info* player) const {
	auto player_index = player_data.indexof(player);
	if(player_index == -1)
		return 0;
	return support[player_index];
}

void province_info::setsupport(const player_info* player, int value) {
	auto player_index = player_data.indexof(player);
	if(player_index == -1)
		return;
	if(value > game.support_maximum)
		value = game.support_maximum;
	if(value < game.support_minimum)
		value = game.support_minimum;
	support[player_index] = value;
}

province_info* province_info::getneighbors(const player_info* player) const {
	province_info* province_array[sizeof(neighbors) / sizeof(neighbors[0])];
	auto ps = province_array;
	for(auto p : neighbors) {
		if(!p)
			break;
		if(!(*p))
			continue;
		if(p->player == player)
			*ps++ = p;
	}
	auto count = ps - province_array;
	if(!count)
		return 0;
	return province_array[rand() % count];
}

void province_info::retreat(const player_info* player) {
	troop_info* objects[64];
	auto count = troop_info::select(objects, sizeof(objects) / sizeof(objects[0]), this, player);
	if(!count)
		return;
	// Вначале отступают те, кто перемещался туда, откуда пришли
	for(unsigned i = 0; i < count; i++) {
		auto p = objects[i];
		if(p->getmove()==this) {
			auto province = p->getprovince();
			p->setmove(0);
			if(province->player != player)
				p->setprovince(this);
		}
	}
	// Потом, остатки отступают на соседнюю свою
	auto province = getneighbors(player);
	if(province) {
		for(unsigned i = 0; i < count; i++) {
			auto p = objects[i];
			if(p->getprovince() == this)
				p->setprovince(province);
		}
	}
	// Остальные в итоге умирают
	for(unsigned i = 0; i < count; i++) {
		auto p = objects[i];
		if(p->getprovince()==this)
			p->clear();
	}
}