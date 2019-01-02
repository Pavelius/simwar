#include "main.h"

bsreq player_info::metadata[] = {
	BSREQ(player_info, id, text_type),
	BSREQ(player_info, name, text_type),
	BSREQ(player_info, nameof, text_type),
	BSREQ(player_info, text, text_type),
	BSREQ(player_info, gold, number_type),
	BSREQ(player_info, influence, number_type),
{}};
adat<player_info, player_max> player_data;
bsdata player_manager("player", player_data, player_info::metadata);

int player_info::getincome(tip_info* ti) const {
	auto result = 0, r = 0;
	for(auto& e : province_data) {
		if(!e)
			continue;
		if(e.getplayer() != this)
			continue;
		r += e.getincome();
	}
	result += fix(ti, msg.income_province, r); r = 0;
	for(auto& e : troop_data) {
		if(!e)
			continue;
		if(e.getplayer() != this)
			continue;
		r += e.getincome();
	}
	result += fix(ti, msg.income_units, r); r = 0;
	for(auto& e : hero_data) {
		if(!e)
			continue;
		if(e.getplayer() != this)
			continue;
		r += e.getincome();
	}
	result += fix(ti, msg.income_heroes, r); r = 0;
	return result;
}

int	player_info::getsupport(tip_info* ti) const {
	auto result = 0;
	for(auto& e : hero_data) {
		if(!e)
			continue;
		if(e.getplayer() != this)
			continue;
	}
	return result;
}

unsigned player_info::getheroes(hero_info** source, unsigned maximum_count, const province_info* province, const player_info* player) {
	auto ps = source;
	auto pe = ps + maximum_count;
	for(auto& e : hero_data) {
		if(!e)
			continue;
		if(player && e.getplayer() != player)
			continue;
		if(province && e.getprovince() != province)
			continue;
		if(ps<pe)
			*ps++ = &e;
	}
	return ps - source;
}

unsigned player_info::gettroops(troop_info** source, unsigned maximum_count, const province_info* province, const player_info* player, const player_info* player_move) {
	auto ps = source;
	auto pe = ps + maximum_count;
	for(auto& e : troop_data) {
		if(!e)
			continue;
		if(player && e.getplayer() != player)
			continue;
		if(province && e.getprovince(player_move) != province)
			continue;
		if(ps<pe)
			*ps++ = &e;
	}
	return ps - source;
}

unsigned player_info::getprovinces(province_info** source, unsigned maximum, const player_info* player, province_flag_s state) {
	auto ps = source;
	auto pe = ps + maximum;
	for(auto& e : province_data) {
		if(!e)
			continue;
		if(player && e.getplayer() != player)
			continue;
		if(ps<pe)
			*ps++ = &e;
	}
	return ps - source;
}

province_info* player_info::getbestprovince() const {
	province_info* elements[province_max];
	auto count = getprovinces(elements, sizeof(elements) / sizeof(elements[0]), this);
	if(!count)
		return 0;
	return elements[0];
}

void player_info::before_turn() {
	for(auto& e : hero_data) {
		if(!e)
			continue;
		e.before_turn();
	}
}

void player_info::after_turn() {
	for(auto& e : player_data) {
		if(!e)
			continue;
		e.gold += e.getincome();
	}
}

void player_info::maketurn() {
	before_turn();
	for(auto& e : player_data) {
		if(!e)
			continue;
		e.makemove();
	}
	after_turn();
}