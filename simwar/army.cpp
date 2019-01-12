#include "main.h"

army::army(player_info* player, province_info* province, hero_info* general, bool attack, bool raid) :
	player(player), general(general), tactic(0), attack(attack), province(province), raid(raid) {}

void army::fill(const player_info* player, const province_info* province) {
	for(auto& e : troop_data) {
		if(!e)
			continue;
		// Игрок должен быть заполнен и это важно
		if(e.getplayer() != player)
			continue;
		if(province && e.getprovince(player) != province)
			continue;
		if(raid && attack && e.get(Raid) <= 0)
			continue;
		add(&e);
	}
}

int army::getcasualty(ability_s id) const {
	auto result = 0;
	if(general)
		result += general->get(id) * game.casualties;
	if(tactic)
		result += tactic->get(id) * game.casualties;
	for(auto p : *this)
		result += p->get(id);
	return result;
}

int army::get(ability_s id) const {
	auto result = 0;
	if(general)
		result += general->get(id);
	if(tactic)
		result += tactic->get(id);
	for(auto p : *this)
		result += p->get(id);
	return result;
}

int army::getstrenght(tip_info* ti, bool include_number) const {
	return get(attack ? Attack : Defend, ti, include_number);
}

int army::get(ability_s id, tip_info* ti, bool include_number) const {
	auto r = 0;
	if(ti && include_number)
		zcpy(ti->result, "[\"");
	if(general) {
		auto value = general->get(id);
		if(raid)
			value += general->get(Raid);
		if(province) {
			if(general->getorigin() == province->getlandscape())
				value += general->get(Magic);
		}
		r += general->fix(ti, value);
		if(!attack) {
			auto action = general->getaction();
			r += action->fix(ti, action->get(Defend));
		}
	}
	for(auto p : *this) {
		auto value = p->get(id);
		if(raid)
			value += p->get(Raid);
		r += p->fix(ti, value);
	}
	if(province) {
		if(!attack) {
			r += province->fix(ti, province->getdefend());
			if(game.support_defend) {
				auto value = province->getsupport(player) / game.support_defend;
				char temp[256]; zprint(temp, "%1 %2", msg.support, province->getname());
				r += name_info::fix(ti, temp, value);
			}
		} else {
			if(game.support_attack) {
				auto value = province->getsupport(player) / game.support_attack;
				char temp[256]; zprint(temp, "%1 %2", msg.support, province->getname());
				r += name_info::fix(ti, temp, value);
			}
		}
	}
	if(tactic) {
		auto value = tactic->get(id);
		if(raid)
			value += tactic->get(Raid);
		r += tactic->fix(ti, value);
	}
	if(ti && include_number)
		szprint(zend(ti->result), ti->result_max, "\"%1i]", r);
	return r;
}