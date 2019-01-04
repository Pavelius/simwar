#include "main.h"

static name_info player_ai_data[] = {{"PlayerHuman"},
{"PlayerComputer"},
};
assert_enum(player_ai, PlayerComputer);
bsreq player_ai_type[] = {
	BSREQ(player_info, id, text_type),
	BSREQ(player_info, name, text_type),
	BSREQ(player_info, text, text_type),
{}};
BSMETA(player_ai);