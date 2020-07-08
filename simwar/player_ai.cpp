#include "main.h"

static namei player_ai_data[] = {{"none"},
{"human"},
{"computer"},
};
assert_enum(player_ai, PlayerComputer);
bsreq player_ai_type[] = {
	BSREQ(playeri, id, text_type),
	BSREQ(playeri, name, text_type),
	BSREQ(playeri, text, text_type),
{}};
BSMETA(player_ai);