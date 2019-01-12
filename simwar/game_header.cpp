#include "io.h"
#include "main.h"

struct game_header_parser : bsdata::parser {

	game_info		game;
	player_info		players[player_max];
	unsigned		players_count;
	bsdata			bases[2];

	virtual void error(bsparse_error_s id, const char* url, int line, int column, const char* format_param) override {}
	
	virtual bsdata*	findbase(const bsreq* type) const override {
		for(auto& e : bases) {
			if(e.fields == type)
				return (bsdata*)&e;
		}
		return 0;
	}
	
	virtual bsdata*	findbase(const char* id) const override {
		for(auto& e : bases) {
			if(strcmp(e.id, id)==0)
				return (bsdata*)&e;
		}
		return 0;
	}

	game_header_parser() : bases{{"game", game_type, &game, sizeof(game), 1},
	{"player", player_info::metadata, players, sizeof(players[0]), lenghtof(players), players_count}},
	players_count(0) {
		game.initialize();
		memset(players, 0, sizeof(players));
	}

};

unsigned game_header::select(game_header* source, unsigned maximum) {
	auto ps = source;
	auto pe = ps + maximum;
	const char* url = "maps";
	for(io::file::find e("maps"); e; e.next()) {
		if(e.name()[0] == '.')
			continue;
		if(!szpmatch(e.name(), "*.map"))
			continue;
		if(ps >= pe)
			break;
		char temp[260]; szfnamewe(temp, e.name());
		ps->id = szdup(temp);
		game_header_parser header;
		bsdata::read(zprint(temp, "%1/%2.map", url, ps->id), header);
		ps->map = header.game.map;
		ps->players_count = header.players_count;
		ps++;
	}
	return ps - source;
}