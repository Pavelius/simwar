#include "main.h"

extern bsdata tactic_manager;

class combatside : public army {

	int strenght;
	int	casualties;

public:

	combatside(province_info* province, player_info* player, bool attack, bool raid) : army(player, province, 0, attack, raid), strenght(0), casualties(0) {
		fill(player, province);
		general = province->gethero(player);
		shuffle();
	}

	operator bool() const {
		return getcount();
	}

	void shuffle() {
		zshuffle(data, count);
	}

	char* getlead(char* result, const char* result_max) const {
		zcpy(result, getsideof());
		if(result[0])
			zcat(result, " ");
		if(general) {
			szprint(zend(result), result_max, msg.lead, general->getname());
			zcat(result, " ");
		}
		return result;
	}

	char* setstrenght(char* result, const char* result_max, const char* format, const char* province_name) {
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
		strenght = army::getstrenght(&ti);
		szprint(result, result_max, format, getlead(text_lead, zendof(text_lead)), text_tips, province_name);
		return zend(result);
	}

	int getstrenght() const {
		return strenght;
	}

	const char* getside() const {
		if(player)
			return player->getname();
		return msg.neutral_army;
	}

	const char* getsideof() const {
		if(player)
			return player->getnameof();
		return msg.neutral_army_of;
	}

	player_info* getowner() {
		return player;
	}

	void setcasualty(char* result, const char* result_max, combatside& enemy) {
		if(game.casualties)
			casualties += (enemy.strenght / game.casualties);
		if(enemy.tactic)
			casualties += enemy.tactic->cruelty;
		if(tactic)
			casualties -= tactic->shield;
		if(casualties > (int)count)
			casualties = count;
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
			if(getcount()>0) {
				if(ps[0])
					zcat(ps, ", ");
				zcat(result, data[count - 1]->getname());
				count--;
			}
		}
		if(result[0])
			zcat(result, ".");
	}

};

bool province_info::battle(char* result, const char* result_max, player_info* attacker_player, player_info* defender_player, bool raid) {
	auto p = result;
	combatside attackers(this, attacker_player, true, raid);
	combatside defenders(this, defender_player, false, raid);
	p = attackers.setstrenght(p, result_max, msg.attacking_force, getname()); zcat(p, " ");
	p = defenders.setstrenght(zend(p), result_max, msg.defending_force, getname());
	attackers.setcasualty(zend(p), result_max, defenders);
	defenders.setcasualty(zend(p), result_max, attackers);
	auto& winner = (attackers.getstrenght() > defenders.getstrenght()) ? attackers : defenders;
	zcat(result, " "); szprint(zend(result), result_max, msg.winner, winner.getside());
	attackers.applycasualty(zend(p), result_max);
	defenders.applycasualty(zend(p), result_max);
	draw::addbutton(zend(p), result_max, "accept");
	add(attacker_player, -1);
	return &winner == &attackers;
}