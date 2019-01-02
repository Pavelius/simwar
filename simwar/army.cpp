#include "main.h"

army::army(player_info* player, hero_info* general, const char* skill) :
	player(player), general(general), tactic(0), skill(skill), raid_mode(false) {
}

void army::fill(const province_info* province) {
	fill(player, province, skill);
}

void army::fill(const player_info* player, const province_info* province, const char* skill) {
	for(auto& e : troop_data) {
		if(!e)
			continue;
		if(player && e.getplayer() != player)
			continue;
		if(province && e.getprovince() != province)
			continue;
		if(skill && e.get(skill) == 0)
			continue;
		add(&e);
	}
}

int army::get(const char* id, tip_info* ti, bool include_number) const {
	auto r = 0;
	if(ti && include_number)
		zcpy(ti->result, "[\"");
	if(general) {
		r += general->fix(ti, general->get(id) + general->getbonus(id));
		if(raid_mode)
			r += general->fix(ti, general->get("raid") + general->getbonus("raid"));
	}
	for(auto p : *this) {
		r += p->fix(ti, p->get(id) + p->getbonus(id));
		if(raid_mode)
			r += p->fix(ti, general->get("raid") + general->getbonus("raid"));
	}
	if(tactic) {
		r += tactic->fix(ti, tactic->get(id));
		if(raid_mode)
			r += tactic->fix(ti, tactic->get("raid"));
	}
	if(ti && include_number)
		szprint(zend(ti->result), ti->result_max, "\"%1i]", r);
	return r;
}