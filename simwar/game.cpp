#include "bslog.h"
#include "main.h"

static bsreq game_type[] = {
	BSREQ(game_info, income_per_level, number_type),
	BSREQ(game_info, casualties, number_type),
	BSREQ(game_info, support_maximum, number_type),
	BSREQ(game_info, support_minimum, number_type),
	BSREQ(game_info, economy_maximum, number_type),
	BSREQ(game_info, economy_minimum, number_type),
	BSREQ(game_info, support_change, number_type),
	BSREQ(game_info, support_attack, number_type),
	BSREQ(game_info, support_defend, number_type),
	BSREQ(game_info, loyalty_base, number_type),
	BSREQ(game_info, loyalty_maximum, number_type),
	BSREQ(game_info, loyalty_noble_modifier, number_type),
	BSREQ(game_info, default_action, action_type),
	BSREQ(game_info, map, text_type),
{}};
game_info		game;
bsdata			game_manager("game", game, game_type);

void game_info::clear() {
	memset(this, 0, sizeof(*this));
}

const char* key_requisits[] = {"name", "nameof", "nameact", "text", 0};
static bsdata::requisit required_reqisits[] = {{"name", true},
{"nameact", true},
{"order", false, {1, 5}},
{"bonus_tactic", true},
{"loyalty_maximum", true},
{"loyalty_base", true},
{"nation", true, {}, province_info::metadata},
};

bool game_info::read(const char* name) {
	char temp[260];
	auto result = true;
	const auto url_errors = "errors.txt";
	game.clear();
	io::file::remove(url_errors);
	if(true) {
		bslog errors(url_errors);
		bsdata::read(zprint(temp, "script/%1.txt", name), errors);
		bsdata::readl(zprint(temp, "script/%1_%2.txt", name, "ru"), key_requisits);
		errors.check(required_reqisits, lenghtof(required_reqisits));
		if(result) {
			if(!draw::initializemap())
				return false;
			initialize();
		}
	}
	return !io::file::exist(url_errors);
}

void game_info::initialize() {
	if(!loyalty_noble_modifier)
		loyalty_noble_modifier = 1;
	hero_info::initialize();
}