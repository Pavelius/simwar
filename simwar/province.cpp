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
	BSREQ(province_info, nation, nation_type),
{}};
adat<province_info, province_max> province_data;
unsigned char			province_order[province_max];
bsdata					province_manager("province", province_data, province_info::metadata);
static unsigned short	path_cost[province_max];
static unsigned char	path_stack[province_max * 4];
static unsigned short	path_push;
static unsigned short	path_pop;

province_flag_s province_info::getstatus(const player_info* player) const {
	if(this->player == player)
		return FriendlyProvince;
	if(!this->player)
		return NeutralProvince;
	return NoFriendlyProvince;
}

int province_info::getdefend() const {
	return landscape->get(Defend) + level;
}

int province_info::getincome(stringbuilder* ti) const {
	auto result = 0;
	if(landscape)
		result += landscape->getincome(ti);
	result += level * game.income_per_level;
	result += geteconomy();
	return result;
}

static void clear_movement() {
	memset(path_cost, 0, sizeof(path_cost));
}

int	province_info::getindex() const {
	return province_data.data - this;
}

int	province_info::getmovecost() const {
	auto result = getindex();
	if(result == -1)
		return -1;
	return path_cost[result];
}

static bool has(province_info** source, province_info** source_end, const province_info* province) {
	for(auto p = source; p < source_end; p++) {
		if(*p == province)
			return true;
	}
	return false;
}

unsigned province_info::select_friendly(province_info** source, unsigned maximum, const player_info* player) {
	auto ps = source;
	auto pe = ps + maximum;
	for(auto& e : province_data) {
		if(!e)
			continue;
		if(e.getstatus(player) != FriendlyProvince)
			continue;
		if(ps < pe)
			*ps++ = &e;
	}
	return ps - source;
}

unsigned province_info::select(province_info** source, unsigned maximum, const player_info* player) {
	auto count = select_friendly(source, maximum, player);
	if(!count)
		return 0;
	auto ps = source + count;
	auto pe = source + maximum;
	for(unsigned i = 0; i < count; i++) {
		for(auto p : source[i]->neighbors) {
			if(!p)
				continue;
			if(has(source, ps, p))
				continue;
			if(ps < pe)
				*ps++ = p;
		}
	}
	return ps - source;
}

void province_info::createwave() {
	clear_movement();
	path_stack[path_push++] = getindex();
	path_cost[getindex()] = 1;
	while(path_push != path_pop) {
		auto p = province_data.data + path_stack[path_pop++];
		auto n = p->getindex();
		auto w = path_cost[n] + 1;
		if(w >= 250)
			continue;
		for(auto pn : p->neighbors) {
			auto i = pn->getindex();
			if(!path_cost[i] || path_cost[i] > w) {
				path_cost[i] = w;
				path_stack[path_push++] = i;
				if(path_push >= sizeof(path_stack) / sizeof(path_stack[0]))
					return;
			}
		}
	}
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

unsigned province_info::remove_mode(aref<province_info*> source, const player_info* player, province_flag_s state) {
	auto ps = source.data;
	for(auto p : source) {
		switch(state) {
		case NoFriendlyProvince:
			if(p->getstatus(player) == FriendlyProvince)
				continue;
			break;
		case FriendlyProvince:
			if(p->getstatus(player) != FriendlyProvince)
				continue;
			break;
		case NeutralProvince:
			if(p->getstatus(player) != NeutralProvince)
				continue;
			break;
		default:
			break;
		}
		*ps++ = p;
	}
	return ps - source.data;
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

void province_info::getinfo(stringbuilder& sb, bool show_landscape, const army* defenders) const {
	if(show_landscape)
		sb.adds(landscape->name);
	sb.adds(":gold:%1i", getincome());
	sb.adds(":house:%1i", getlevel());
	auto value = getdefend();
	if(defenders)
		value = defenders->get(Defend, 0);
	sb.adds(":shield_grey:%1i", value);
}

void province_info::getsupport(stringbuilder& sb) const {
	for(auto& e : player_data) {
		if(!e)
			continue;
		auto value = getsupport(&e);
		if(value == 0)
			continue;
		sb.adds("%+1:[%2i]", e.getname(), value);
	}
}

void province_info::build(unit_info* unit, int turns) {
	if(turns < 1)
		turns = 1;
	auto p = build_data.add();
	p->province = this;
	p->unit = unit;
	p->wait = turns;
}

void province_info::add(const unit_info* unit) {
	troop_info::add(this, unit);
}

void province_info::change_support() {
	auto support_maximum = game.support_maximum;
	auto support_minimum = game.support_minimum;
	for(auto& e : province_data) {
		if(!e)
			continue;
		auto player = e.getplayer();
		auto player_index = player->getindex();
		for(auto i = 0; i < sizeof(support) / sizeof(support[0]); i++) {
			auto value = e.support[i];
			if(player_index == i) {
				if(value < support_maximum)
					value++;
			} else {
				if(value > 0)
					value--;
				else if(value < 0)
					value++;
			}
			e.support[i] = value;
		}
	}
}

int	province_info::getsupport(const player_info* player) const {
	auto player_index = player->getindex();
	if(player_index == -1)
		return 0;
	return support[player_index];
}

void province_info::setsupport(const player_info* player, int value) {
	auto player_index = player->getindex();
	if(player_index == -1)
		return;
	if(value > game.support_maximum)
		value = game.support_maximum;
	if(value < game.support_minimum)
		value = game.support_minimum;
	support[player_index] = value;
}

void province_info::addsupportex(const player_info* player, int value, int minimal_value, int maximal_value) {
	auto player_index = player->getindex();
	for(unsigned i = 0; i < sizeof(support) / sizeof(support[0]); i++) {
		if(i == player_index)
			continue;
		auto v = support[i];
		if(v < minimal_value || v > maximal_value)
			continue;
		v += value;
		if(v < minimal_value)
			v = minimal_value;
		if(v > maximal_value)
			v = maximal_value;
		setsupport(player_data.data + i, v);
	}
}

void province_info::seteconomy(int value) {
	economy += value;
	if(economy > game.economy_maximum)
		economy = game.economy_maximum;
	if(economy < game.economy_minimum)
		economy = game.economy_minimum;
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

void province_info::arrival(const player_info* player) {
	troop_info::arrival(this, player);
}

void province_info::retreat(const player_info* player) {
	troop_info* source[64];
	if(getplayer() != player) {
		// Если игрок атаковал, оступают все нападающие.
		// Ели в этот момент захватили их провинцию, они сдаются победителю.
		troop_info::retreat(this, player);
		return;
	}
	auto count = troop_info::select(source, sizeof(source) / sizeof(source[0]), this);
	if(!count)
		return;
	// Потом, остатки отступают на соседнюю свою
	auto province = getneighbors(player);
	if(province) {
		for(unsigned i = 0; i < count; i++)
			source[i]->setprovince(province);
		return;
	}
	// Остальные в итоге умирают
	for(unsigned i = 0; i < count; i++) {
		auto p = source[i];
		if(p->getprovince() == this)
			p->clear();
	}
}

void province_info::initialize() {
	for(auto& e : province_data) {
		if(!e)
			continue;
		if(e.player) {
			if(e.getsupport(e.player) == 0)
				e.setsupport(e.player, 10);
		} else {
			army a1; a1.fill(0, &e);
			if(a1.getcount())
				continue;
			for(auto i = 0; i <= e.level; i++) {
				auto pu = unit_info::getfirst(e.nation, e.landscape, 0);
				if(!pu)
					continue;
				e.add(pu);
			}
		}
	}
}