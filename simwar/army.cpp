#include "main.h"

army::army(playeri* player, provincei* province, heroi* general, bool attack, bool raid) :
	player(player), general(general), tactic(0), attack(attack), province(province), raid(raid) {}

void army::fill(const playeri* player, const provincei* province) {
	for(auto& e : troop_data) {
		if(!e)
			continue;
		// ����� ������ ���� �������� � ��� �����
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

int army::getstrenght(stringcreator* sb) const {
	return get(attack ? Attack : Defend, sb);
}

int army::get(ability_s id, stringcreator* sb) const {
	auto r = 0;
	if(sb)
		sb->adds("[\"");
	if(general) {
		auto value = general->get(id);
		if(raid)
			value += general->get(Raid);
		r += general->fix(sb, value);
		if(province) {
			if(general->getorigin() == province->getlandscape())
				r += namei::fix(sb, msg.battle_magic, general->get(Magic));
		}
		if(!attack) {
			auto action = general->getaction();
			r += action->fix(sb, action->get(Defend));
		}
	}
	for(auto p : *this) {
		auto value = p->get(id);
		if(raid)
			value += p->get(Raid);
		r += p->fix(sb, value);
	}
	if(province) {
		if(!attack) {
			r += province->fix(sb, province->getdefend());
			if(game.support_defend) {
				auto value = province->getsupport(player) / game.support_defend;
				char temp[256]; zprint(temp, "%1 %2", namei::getname(Support), province->getname());
				r += namei::fix(sb, temp, value);
			}
		} else {
			if(game.support_attack) {
				auto value = province->getsupport(player) / game.support_attack;
				char temp[256]; zprint(temp, "%1 %2", namei::getname(Support), province->getname());
				r += namei::fix(sb, temp, value);
			}
		}
	}
	if(tactic) {
		auto value = tactic->get(id);
		if(raid)
			value += tactic->get(Raid);
		r += tactic->fix(sb, value);
	}
	if(sb)
		sb->adds("\"%1i]", r);
	return r;
}