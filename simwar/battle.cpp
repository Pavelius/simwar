#include "main.h"

extern bsdata tactic_manager;

const char* numeric(int value, const char* s1, const char* s2, const char* s5) {
	if(value == 1)
		return s1;
	else if(value >= 5)
		return s5;
	return s2;
}

struct combatside : public army {

	int strenght;
	int	casualties;
	int wounds;

	combatside(province_info* province, player_info* player, bool attack, bool raid) : army(player, province, 0, attack, raid),
		strenght(0), casualties(0), wounds(0) {
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
		szprint(result, result_max, getsideof());
		if(general) {
			if(result[0])
				szprint(zend(result), result_max, " ");
			szprint(zend(result), result_max, msg.lead, general->getnameof());
		}
		return result;
	}

	void setstrenght(string& sb, const char* format, const char* province_name) {
		char text_lead[260]; text_lead[0] = 0;
		char text_tips[2048]; text_tips[0] = 0;
		tip_info ti(text_tips, zendof(text_tips));
		if(general)
			tactic = general->gettactic();
		if(!tactic) {
			auto count = tactic_manager.getcount();
			if(count > 0)
				tactic = tactic_data.data + (rand() % count);
			else
				tactic = tactic_data.data;
		}
		strenght = army::getstrenght(&ti);
		sb.add(format, getlead(text_lead, zendof(text_lead)), text_tips, province_name);
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

	void setcasualty(string& sb, combatside& enemy) {
		auto strenght_casualties = enemy.strenght + enemy.getsword() - getshield();
		if(game.casualties)
			casualties += (strenght_casualties / game.casualties);
		if(casualties > (int)count) {
			wounds = casualties - count;
			casualties = count;
		}
		if(tactic) {
			sb.add(" ");
			sb.add(tactic->text, getside());
		}
	}

	void casualtyhead(stringbuilder& sb, const char* result) {
		if(result[0])
			sb.add(", ");
		else
			sb.addn("%1 %2: ", msg.casualties, getsideof());
	}

	void applycasualty(stringbuilder& sb) {
		auto result = sb.get();
		for(auto i = 0; i < casualties; i++) {
			if(getcount() <= 0)
				break;
			casualtyhead(sb, result);
			sb.add(data[count - 1]->getname());
			data[count - 1]->kill(player);
			count--;
		}
		if(general && wounds) {
			casualtyhead(sb, result);
			sb.add("%1 (%2i %3)", general->getname(), wounds, numeric(wounds, msg.wound, msg.wounds, msg.wounds_of));
			general->setwound(general->getwound() + wounds * 2);
		}
		if(result[0])
			sb.add(".");
	}

	void addloyalty(int value) {
		if(general)
			general->setloyalty(general->getloyalty() + value);
	}

};

bool province_info::battle(string& sb, player_info* attacker_player, player_info* defender_player, const action_info* action, bool raid) {
	combatside attackers(this, attacker_player, true, raid);
	combatside defenders(this, defender_player, false, raid);
	attackers.setstrenght(sb, msg.attacking_force, getname()); sb.add(" ");
	defenders.setstrenght(sb, msg.defending_force, getname());
	attackers.setcasualty(sb, defenders);
	defenders.setcasualty(sb, attackers);
	auto& winner = (attackers.getstrenght() > defenders.getstrenght()) ? attackers : defenders;
	sb.add(" "); sb.add(msg.winner, winner.getside());
	attackers.applycasualty(sb);
	defenders.applycasualty(sb);
	auto iswin = (&winner == &attackers);
	if(attackers.player)
		attackers.player->cost.fame += defenders.casualties - attackers.casualties;
	if(defenders.player)
		defenders.player->cost.fame += attackers.casualties - defenders.casualties;
	if(iswin) {
		retreat(defender_player);
		arrival(attacker_player);
		if(!raid) {
			player = attacker_player;
			if(attackers.general)
				attacker_player->cost.fame += imax(0, attackers.general->getnobility());
		} else {
			auto trophies = action->trophies;
			if(trophies) {
				trophies.gold += defenders.casualties;
				if(attackers.general)
					trophies.gold += imax(0, attackers.general->getraid());
				if(attacker_player)
					attacker_player->cost += trophies;
				if(defender_player)
					defender_player->cost -= trophies;
				sb.addn(msg.raid_spoils, trophies.gold);
			}
		}
		defenders.addloyalty(-1);
	} else {
		retreat(attacker_player);
		if(defender_player && defenders.general)
			defender_player->cost.fame += imax(0, defenders.general->getnobility());
		attackers.addloyalty(-1);
	}
	return iswin;
}