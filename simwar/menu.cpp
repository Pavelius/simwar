#include "main.h"

static void exit_game(const menu_info* p) {
	exit(0);
}

static void load_game(const menu_info* p) {
	if(!game.readmap("silentseas"))
		return;
	player_info::playgame();
}

static void new_game(const menu_info* p) {
	game_header games[64];
	auto count = game_header::select(games, lenghtof(games));
	auto choosen_map = game_header::choose(games, count);
}

static void map_editor(const menu_info* p) {
	game.readmap("silentseas");
	game.editor("editor.map");
}

bsreq menu_type[] = {
	BSREQ(menu_info, id, text_type),
	BSREQ(menu_info, name, text_type),
{}};
static menu_info menu_data[] = {
{"new_game", "main_menu", new_game},
{"load_game", "main_menu", load_game},
{"exit_game", "main_menu", exit_game},
};
BSMETA(menu);

void menu_info::select(answer_info& ai, const char* parent) {
	for(auto& e : menu_data) {
		if(strcmp(e.parent, parent)==0)
			ai.add((int)&e, e.name);
	}
}

const menu_info* menu_info::choose(const char* parent, bool cancel_button) {
	answer_info ai;
	select(ai, parent);
	//ai.sort();
	return (menu_info*)ai.choose(cancel_button);
}

void menu_info::choose_block(const char* parent) {
	while(parent) {
		auto p = choose(parent, false);
		if(!p)
			break;
		if(p->proc)
			p->proc(p);
		parent = p->id;
	}
}