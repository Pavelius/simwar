#include "main.h"

name_info ability_data[] = {{"attack"},
{"defend"},
{"raid"},
{"sword"},
{"shield"},
};
assert_enum(ability, Shield);
bsreq ability_type[] = {
	BSREQ(hero_info, id, text_type),
	BSREQ(hero_info, name, text_type),
	BSREQ(hero_info, nameof, text_type),
	BSREQ(hero_info, text, text_type),
{}};
BSMETA(ability);