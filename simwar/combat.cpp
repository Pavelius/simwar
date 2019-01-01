#include "main.h"

class combatside {

	adat<troop_info*, 32>	units;
	hero_info*				general;
	tactic_info*			tactic;
	player_info*			player;
	int						strenght;
	int						casualties;

public:

	combatside(const province_info* province, player_info* player) : player(player), general(0), tactic(0), strenght(0), casualties(0) {
		// Get all units in province by one owner
		for(auto& e : troop_data) {
			if(!e)
				continue;
			if(e.getplayer() != player)
				continue;
			if(e.getprovince() != province)
				continue;
			units.add(&e);
		}
		// Get all heroes in province and choose one best matched
		for(auto& e : hero_data) {
			if(!e)
				continue;
			if(e.getplayer() != player)
				continue;
			if(e.getprovince() != province)
				continue;
			general = &e;
			break;
		}
		shuffle();
	}

	operator bool() const {
		return units.getcount();
	}

	void shuffle() {
		zshuffle(units.data, units.count);
	}

	int get(const char* id, tip_info* ti, bool include_number = true) const {
		auto r = 0;
		if(ti && include_number)
			zcpy(ti->result, "[\"");
		if(general)
			r += general->fix(ti, general->get(id) + general->getbonus(id));
		for(auto p : units)
			r += p->fix(ti, p->get(id) + p->getbonus(id));
		if(tactic)
			r += tactic->fix(ti, tactic->get(id));
		if(ti && include_number)
			szprint(zend(ti->result), ti->result_max, "\"%1i]", r);
		return r;
	}

	char* getlead(char* result, const char* result_max) const {
		zcpy(result, getsideof());
		if(result[0])
			zcat(result, " ");
		if(general)
			szprint(zend(result), result_max, msg.lead, general->getname());
		return result;
	}

	char* setstrenght(char* result, const char* result_max, const char* format, const char* id, const char* province_name) {
		char text_lead[260]; text_lead[0] = 0;
		char text_tips[2048]; text_tips[0] = 0;
		tip_info ti(text_tips, zendof(text_tips));
		if(general)
			tactic = general->gettactic();
		if(!tactic) {
			auto count = tactic_manager.getcount();
			if(count > 0)
				tactic = tactic_data + rand() % count;
			else
				tactic = tactic_data;
		}
		strenght = get(id, &ti);
		szprint(result, result_max, format, getlead(text_lead, zendof(text_lead)), text_tips, province_name);
		return zend(result);
	}

	int getstrenght() const {
		return strenght;
	}

	const char* getside() const {
		if(player)
			return player->getname();
		return "";
	}

	const char* getsideof() const {
		if(player)
			return player->getnameof();
		return "";
	}

	player_info* getowner() {
		return player;
	}

	void setcasualty(char* result, const char* result_max, combatside& enemy) {
		if(game.casualties)
			casualties += (enemy.strenght / game.casualties);
		if(enemy.tactic)
			casualties += enemy.tactic->sword;
		if(tactic)
			casualties -= tactic->shield;
		if(tactic) {
			zcat(result, " ");
			szprint(zend(result), result_max, tactic->text, getside());
		}
	}

	void applycasualty(char* result, const char* result_max) {
		result[0] = 0;
		auto ps = result;
		for(auto i = 0; i < casualties; i++) {
			if(!result[0]) {
				szprint(result, result_max, "\n%1 %2: ", msg.casualties, getsideof());
				ps = zend(result);
			}
			if(units.count>0) {
				if(ps[0])
					zcat(ps, ", ");
				zcat(result, units.data[units.count - 1]->getname());
				units.count--;
			}
		}
		if(result[0])
			zcat(result, ".");
	}

};

bool province_info::battle(char* result, const char* result_max, player_info* attacker_player, player_info* defender_player, const char* attack_skill, const char* defend_skill) {
	if(!attack_skill)
		attack_skill = "attack";
	if(!defend_skill)
		defend_skill = "defend";
	auto p = result;
	combatside attackers(this, attacker_player);
	combatside defenders(this, defender_player);
	p = attackers.setstrenght(p, result_max, msg.attacking_force, attack_skill, getname()); zcat(p, " ");
	p = defenders.setstrenght(zend(p), result_max, msg.defending_force, defend_skill, getname());
	attackers.setcasualty(zend(p), result_max, defenders);
	defenders.setcasualty(zend(p), result_max, attackers);
	auto& winner = (attackers.getstrenght() > defenders.getstrenght()) ? attackers : defenders;
	zcat(result, " "); szprint(zend(result), result_max, msg.winner, winner.getside());
	attackers.applycasualty(zend(p), result_max);
	defenders.applycasualty(zend(p), result_max);
	draw::addbutton(zend(p), result_max, "accept");
	return &winner == &attackers;
}