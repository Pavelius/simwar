#include "main.h"

bsreq hero_info::metadata[] = {
	BSREQ(hero_info, id, text_type),
	BSREQ(hero_info, name, text_type),
	BSREQ(hero_info, nameof, text_type),
	BSREQ(hero_info, text, text_type),
	BSREQ(hero_info, avatar, text_type),
	BSREQ(hero_info, tactic, tactic_type),
	BSREQ(hero_info, traits, trait_type),
	BSREQ(hero_info, player, player_info::metadata),
	BSREQ(hero_info, province, province_type),
{0}
};
adat<hero_info, 128> hero_data;
bsdata hero_manager("hero", hero_data, hero_info::metadata);

void hero_info::before_turn() {
	tactic = 0;
	province = 0;
	action = game.default_action;
}