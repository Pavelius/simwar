#include "main.h"

army::army(player_info* player, province_info* province, hero_info* general, bool attack, bool raid) :
	player(player), general(general), tactic(0), attack(attack), province(province), raid(raid) {
}

void army::fill(const player_info* player, const province_info* province) {
	for(auto& e : troop_data) {
		if(!e)
			continue;
		if(player && e.getplayer() != player)
			continue;
		if(province && e.getprovince() != province)
			continue;
		if(raid && e.get("raid")<=0)
			continue;
		add(&e);
	}
}

int army::getstrenght(tip_info* ti, bool include_number) const {
	return get(attack ? "attack" : "defend", ti, include_number);
}

int army::get(const char* id, tip_info* ti, bool include_number) const {
	auto r = 0;
	if(ti && include_number)
		zcpy(ti->result, "[\"");
	if(general) {
		r += general->fix(ti, general->get(id) + general->getbonus(id));
		if(attack && raid)
			r += general->fix(ti, general->get("raid") + general->getbonus("raid"));
	}
	for(auto p : *this) {
		r += p->fix(ti, p->get(id) + p->getbonus(id));
		if(attack && raid)
			r += p->fix(ti, general->get("raid") + general->getbonus("raid"));
	}
	if(!attack) {
		if(province)
			r += province->fix(ti, province->getdefend());
	}
	if(tactic) {
		r += tactic->fix(ti, tactic->get(id));
		if(raid)
			r += tactic->fix(ti, tactic->get("raid"));
	}
	if(ti && include_number)
		szprint(zend(ti->result), ti->result_max, "\"%1i]", r);
	return r;
}