#include "bslog.h"
#include "main.h"

bsreq game_type[] = {
	BSREQ(game_info, income_per_level, number_type),
	BSREQ(game_info, casualties, number_type),
	BSREQ(game_info, support_maximum, number_type),
	BSREQ(game_info, support_minimum, number_type),
	BSREQ(game_info, economy_maximum, number_type),
	BSREQ(game_info, economy_minimum, number_type),
	BSREQ(game_info, support_attack, number_type),
	BSREQ(game_info, support_defend, number_type),
	BSREQ(game_info, loyalty_base, number_type),
	BSREQ(game_info, loyalty_maximum, number_type),
	BSREQ(game_info, loyalty_noble_modifier, number_type),
	BSREQ(game_info, desert_base, number_type),
	BSREQ(game_info, year, number_type),
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
{"home", true},
{"origin", true},
};
bsreq ability_requisits[LastAbility + 2];

struct game_log : bslog {

	game_log(const char* name) : bslog(name) {}

	virtual const bsreq* getrequisit(const bsreq* fields, const char* buffer) const override {
		auto result = parser::getrequisit(fields, buffer);
		if(result)
			return result;
		auto pbase = fields->find("ability");
		if(!pbase)
			return 0;
		if(!ability_requisits[0].id) {
			for(auto i = 0; i <= LastAbility; i++) {
				ability_requisits[i].id = ability_data[i].id;
				ability_requisits[i].type = number_type;
				ability_requisits[i].count = 1;
				ability_requisits[i].size = 1;
				ability_requisits[i].offset = pbase->offset + i;
			}
		}
		return ability_requisits->find(buffer);
	}

};

bool game_info::readmap(const char* name) {
	char temp[260];
	auto result = true;
	const auto url_errors = "errors.txt";
	game.clear();
	io::file::remove(url_errors);
	if(true) {
		game_log errors(url_errors);
		bsdata::read(zprint(temp, "maps/%1.map", name), errors);
		bsdata::readl(zprint(temp, "maps/%1_%2.txt", name, "ru"), key_requisits);
		errors.check(required_reqisits, lenghtof(required_reqisits));
		if(result) {
			if(!initializemap())
				return false;
			initialize();
		}
	}
	return !io::file::exist(url_errors);
}

void game_info::initialize() {
	if(!loyalty_noble_modifier)
		loyalty_noble_modifier = 2;
	if(!hire_turns)
		hire_turns = 1;
	if(!hire_turns_range[0])
		hire_turns_range[0] = 3;
	if(!hire_turns_range[1])
		hire_turns_range[1] = 7;
	if(hire_turns_range[0] > hire_turns_range[1])
		hire_turns_range[0] = hire_turns_range[1] - 1;
	province_info::initialize();
	hero_info::initialize();
}