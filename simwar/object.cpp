#include "main.h"

static ability_s province_conditions[] = {Economy, Level};
static ability_s hero_conditions[] = {Attack, Defend, Raid, Magic, Good, Nobility};

bool object_info::ismatch(int v1, int v2) {
	if(v1 == 0)
		return true;
	else if(v1 < 0)
		return v2 <= v1;
	else
		return v2 >= v1;
}

bool object_info::isvalid(const player_info& e) const {
	if(!ismatch(get(Gold), e.cost.gold))
		return false;
	if(!ismatch(get(Fame), e.cost.fame))
		return false;
	return true;
}

bool object_info::isvalid(const province_info& e) const {
	auto player = e.getplayer();
	if(!player)
		return false;
	if(!isvalid(*player))
		return false;
	if(!ismatch(get(Support), e.getsupport(player)))
		return false;
	for(auto i : province_conditions) {
		if(!ismatch(get(i), e.get(i)))
			return false;
	}
	return true;
}