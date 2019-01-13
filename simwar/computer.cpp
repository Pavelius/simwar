#include "main.h"

struct move_info {
	player_info*			player;
	const action_info*		action_attack;
	const action_info*		action_raid;
	adat<province_info*>	provincies;
	adat<troop_info*>		troops;
	struct need {
		province_info*		province;
		const action_info*	action;
		int					cost;
	};
	adat<need, 1024>		needs;

	void add_need(const action_info* action, int cost) {
		auto state = NoFriendlyProvince;
		for(auto p : provincies) {
			if(p->getstatus(player) != state)
				continue;
			auto pn = needs.add();
			pn->province = p;
			pn->action = action;
			pn->cost = cost;
		}
	}

};