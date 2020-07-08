#include "main.h"

static void exit_game(const menui* p) {
	exit(0);
}

static void test_event() {
	auto p = (eventi*)bsdata::findbyid("barbarian_invasion").data;
	if(!p)
		return;
	auto province = (provincei*)bsdata::findbyid("lone_mountain").data;
	p->play(province, 0);
}

static void load_game(const menui* p) {
	if(!game.readmap("silentseas"))
		return;
	//test_event();
	playeri::playgame();
}

static void new_game(const menui* p) {
	game_header games[64];
	auto count = game_header::select(games, lenghtof(games));
	auto choosen_map = game_header::choose(games, count);
}

bsreq menu_type[] = {
	BSREQ(menui, id, text_type),
	BSREQ(menui, name, text_type),
{}};
static menui menu_data[] = {
{"new_game", "main_menu", new_game},
{"load_game", "main_menu", load_game},
{"exit_game", "main_menu", exit_game},
};
BSMETA(menu);

void menui::select(choiseset& ai, const char* parent) {
	for(auto& e : menu_data) {
		if(strcmp(e.parent, parent)==0)
			ai.add((int)&e, e.name);
	}
}

const menui* menui::choose(const char* parent, bool cancel_button) {
	choiseset ai;
	select(ai, parent);
	//ai.sort();
	return (menui*)ai.choose(true, cancel_button);
}

void menui::choose_block(const char* parent) {
	while(parent) {
		auto p = choose(parent, false);
		if(!p)
			break;
		if(p->proc)
			p->proc(p);
		parent = p->id;
	}
}