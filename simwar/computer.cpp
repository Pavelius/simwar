#include "main.h"

class move_info {
	enum priority_s : unsigned char {
		EnemyPlayer,
		LastPriority = EnemyPlayer,
	};
	struct statistic {
		int					provinces;
		int					gold;
		int					income;
		int					support;
		int					troops;
		statistic() { clear(); }
		void clear() { memset(this, 0, sizeof(*this)); }
	};
	struct need {
		province_info*		province;
		const action_info*	action;
		int					cost;
	};
	char					priority[LastPriority + 1];
	player_info*			player;
	const action_info*		action_attack;
	const action_info*		action_raid;
	const action_info*		action_defend;
	adat<province_info*>	provincies;
	statistic				current, ideal;
	adat<troop_info*>		troops;
	adat<need, 1024>		needs;

	void set(const action_info** result, const action_info* action, ability_s id) {
		if(*result)
			return;
		if(action->get(id) > 0)
			*result = action;
	}

	void fill_actions() {
		action_attack = 0;
		action_raid = 0;
		action_defend = 0;
		for(auto& e : action_data) {
			if(!e)
				continue;
			set(&action_attack, &e, Attack);
			set(&action_raid, &e, Raid);
			set(&action_defend, &e, Defend);
		}
	}

	int get(priority_s id) const {
		return priority[id];
	}

	void add_need(const action_info* action, int cost) {
		auto state = action->getprovince();
		for(auto p : provincies) {
			if(p->getstatus(player) != state)
				continue;
			auto pn = needs.add();
			pn->province = p;
			pn->action = action;
			pn->cost = cost;
			if(p->getplayer() && p->getplayer() != player)
				pn->cost += get(EnemyPlayer);
		}
	}

	int getfriendlyprovinces() const {
		auto result = 0;
		for(auto p : provincies) {
			if(p->getstatus(player) == FriendlyProvince)
				result++;
		}
		return result;
	}

	int gettroopscount() const {
		return troops.getcount();
	}

	int getsupport() const {
		auto result = 0;
		for(auto p : provincies)
			result += p->getsupport(player);
		return result;
	}

	void update_current(statistic& e) {
		e.clear();
		e.gold = player->cost.gold;
		e.income = player->getincome(0);
		e.provinces = getfriendlyprovinces();
		e.troops = gettroopscount();
		e.support = getsupport();
	}

	void update_provincies() {
		provincies.count = province_info::select(provincies.data, provincies.getmaximum(), player);
	}

	void update_troops() {
		troops.count = troop_info::select(troops.data, troops.getmaximum(), player);
	}

	void move() {
		update_provincies();
		update_troops();
		update_current(current);
		add_need(action_attack, 1);
		add_need(action_raid, 1);
		add_need(action_defend, 1);
	}

public:

	move_info(player_info* player) {
		memset(this, 0, sizeof(*this));
		this->player = player;
	}

	void execute() {
		fill_actions();
		move();
	}

};

//void player_info::computer_move() {
//	move_info e(this);
//	e.execute();
//}

void player_info::computer_move() {
	hero_info* source[hero_max];
	auto hero_count = hero_info::select(source, lenghtof(source), this);
	for(unsigned i = 0; i < hero_count; i++)
		source[i]->make_move();
}