#include "bslog.h"
#include "main.h"

static bsreq game_type[] = {
	BSREQ(game_info, income_per_level, number_type),
	BSREQ(game_info, casualties, number_type),
	BSREQ(game_info, support_maximum, number_type),
	BSREQ(game_info, support_minimum, number_type),
	BSREQ(game_info, economy_maximum, number_type),
	BSREQ(game_info, economy_minimum, number_type),
	BSREQ(game_info, support_multiplier, number_type),
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

void game_info::after_load() {
	change_support_provinces = 2;
	if(change_support_provinces > province_data.count)
		change_support_provinces = province_data.count;
}

static const char* requisits[] = {"name", "nameof", "nameact", "text"};
static bsdata::requisit required_reqisits[] = {{"name", true},
{"nameact", true},
{"order", false, {0, 10}},
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
		bsdata::readl(zprint(temp, "script/%1_%2.txt", name, "ru"), requisits, sizeof(requisits) / sizeof(requisits[0]));
		if(!log.check_required(required_reqisits, sizeof(required_reqisits)/ sizeof(required_reqisits[0])))
			result = false;
	}
	if(result) {
		if(!draw::initializemap())
			return false;
		game.after_load();
	}
	if(result)
		io::file::remove(url_errors);
	return result;
}