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
	BSREQ(game_info, default_action, action_type),
	BSREQ(game_info, map, text_type),
{}};
game_info		game;
bsdata			game_manager("game", game, game_type);

void game_info::clear() {
	memset(this, 0, sizeof(*this));
}

static const char* requisits[] = {"name", "nameof", "nameact", "text", 0};
static bsdata::requisit required_reqisits[] = {{"name", true},
{"nameact", true},
{"order", false, {1, 5}},
{"bonus_tactic", true},
{"nation", true, {}, province_info::metadata},
};

bool game_info::read(const char* name) {
	char temp[260];
	auto result = true;
	const auto url_errors = "errors.txt";
	game.clear();
	if(true) {
		bslog log(url_errors);
		bsdata::read(zprint(temp, "script/%1.txt", name), &log);
		if(log.iserrors())
			result = false;
		bsdata::readl(zprint(temp, "script/%1_%2.txt", name, "ru"), requisits);
		if(!log.check(required_reqisits, sizeof(required_reqisits)/ sizeof(required_reqisits[0])))
			result = false;
	}
	if(result) {
		if(!draw::initializemap())
			return false;
	}
	if(result)
		io::file::remove(url_errors);
	return result;
}