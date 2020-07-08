#include "main.h"

bsreq game_type[] = {
	BSREQ(gamei, income_per_level, number_type),
	BSREQ(gamei, casualties, number_type),
	BSREQ(gamei, support_maximum, number_type),
	BSREQ(gamei, support_minimum, number_type),
	BSREQ(gamei, economy_maximum, number_type),
	BSREQ(gamei, economy_minimum, number_type),
	BSREQ(gamei, support_attack, number_type),
	BSREQ(gamei, support_defend, number_type),
	BSREQ(gamei, loyalty_base, number_type),
	BSREQ(gamei, loyalty_maximum, number_type),
	BSREQ(gamei, loyalty_noble_modifier, number_type),
	BSREQ(gamei, desert_base, number_type),
	BSREQ(gamei, year, number_type),
	BSREQ(gamei, default_action, action_type),
	BSREQ(gamei, map, text_type),
{}};
gamei		game;
bsdata			game_manager("game", game, game_type);

void gamei::clear() {
	memset(this, 0, sizeof(*this));
}

const char* key_requisits[] = {"name", "nameof", "nameact", "text", 0};
const char* effect_requisits[] = {"text", 0};
const bsdata_strings key_ranges[] = {{"effects", {0, 2}, effect_requisits},
{}};
static bsdata::requisit required_reqisits[] = {{"name", true},
{"nameact", true},
{"order", false, {1, 5}},
{"bonus_tactic", true},
{"loyalty_maximum", true},
{"loyalty_base", true},
{"nation", true, {}, provincei::metadata},
{"home", true},
{"origin", true},
};
bsreq ability_requisits[LastAbility + 2];

struct game_log : bslog {

	game_log(const char* name) : bslog(name) {}

	const bsreq* getrequisit(const bsreq* fields, const char* buffer) const {
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

bool gamei::readmap(const char* name) {
	char temp[260];
	auto result = true;
	const auto url_errors = "errors.txt";
	game.clear();
	io::file::remove(url_errors);
	if(true) {
		auto loc = "ru";
		game_log errors(url_errors);
		bsdata::read(zprint(temp, "maps/%1.map", name), errors, loc, key_requisits, key_ranges);
		bsdata::readl(zprint(temp, "maps/%1_%2.txt", name, loc), key_requisits, key_ranges);
		errors.check(required_reqisits, lenghtof(required_reqisits));
		if(result) {
			if(!initializemap())
				return false;
			initialize();
		}
	}
	return !io::file::exist(url_errors);
}

void gamei::initialize() {
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
	provincei::initialize();
	heroi::initialize();
	eventi::initialize();
}