#include "main.h"

void army::fill(const player_info* player, const province_info* province) {
	for(auto& e : troop_data) {
		if(!e)
			continue;
		if(player && e.getplayer() != player)
			continue;
		if(province && e.getprovince() != province)
			continue;
		add(&e);
	}
}