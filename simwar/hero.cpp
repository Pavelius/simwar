#include "main.h"

bsreq hero_info::metadata[] = {
	BSREQ(hero_info, id, text_type),
	BSREQ(hero_info, name, text_type),
	BSREQ(hero_info, nameof, text_type),
	BSREQ(hero_info, text, text_type),
	BSREQ(hero_info, avatar, text_type),
	BSREQ(hero_info, tactic, tactic_type),
	BSREQ(hero_info, traits, trait_type),
	BSREQ(hero_info, player, player_info::metadata),
	BSREQ(hero_info, province, province_info::metadata),
	BSREQ(hero_info, wait, number_type),
{}};
adat<hero_info, 128> hero_data;
bsdata hero_manager("hero", hero_data, hero_info::metadata);

void hero_info::refresh_heroes() {
	for(auto& e : hero_data) {
		if(!e)
			continue;
		if(e.wait > 0)
			e.wait--;
		if(e.wait == 0) {
			e.tactic = 0;
			e.province = 0;
			e.action = game.default_action;
		}
	}
}

int hero_info::get(const char* id) const {
	auto r = 0;
	assert(trait_type->find(id)!=0);
	for(auto p : traits) {
		if(!p)
			continue;
		r += p->get(id);
	}
	return r;
}

int hero_info::getbonus(const char* id) const {
	return 0;
}

int	hero_info::getincome() const {
	auto result = 0;
	for(auto p : traits) {
		if(p)
			result--;
	}
	result += getex("nobility");
	return result;
}

void hero_info::setaction(action_info* action, province_info* province, const cost_info& cost, const army& logistic, const unit_set& production) {
	cancelaction();
	setaction(action);
	setprovince(province);
	pay = cost;
	pay += *action;
	if(player)
		*player -= pay;
	for(auto p : logistic)
		p->setmove(province);
	for(auto p : production)
		province->build(p, p->recruit_time);
}

void hero_info::resolve() {
	char temp[8192]; temp[0] = 0;
	if(action->attack || action->raid) {
		auto israid = (action->raid > 0);
		auto enemy = province->getplayer();
		if(enemy != player) {
			auto iswin = province->battle(temp, zendof(temp), player, enemy, action, israid);
			player->add(province, this, temp);
			enemy->add(province, province->gethero(enemy), temp);
		}
	}
	if(action->support)
		province->addsupport(player, action->support + get("diplomacy"));
	if(action->profit)
		province->addsupport(player, action->support + get("diplomacy"));
}

unsigned hero_info::select(hero_info** source, unsigned maximum_count, const province_info* province, const player_info* player) {
	auto ps = source;
	auto pe = ps + maximum_count;
	for(auto& e : hero_data) {
		if(!e)
			continue;
		if(player && e.getplayer() != player)
			continue;
		if(province && e.getprovince() != province)
			continue;
		if(ps < pe)
			*ps++ = &e;
	}
	return ps - source;
}

void hero_info::cancelaction() {
	if(!action)
		return;
	for(auto& e : troop_data) {
		if(!e)
			continue;
		if(e.getplayer() == player && e.getmove() == province)
			e.setmove(0);
	}
	for(auto& e : build_data) {
		if(!e)
			continue;
		if(e.province==province)
			e.clear();
	}
	if(player)
		*player += pay;
	pay.clear();
}