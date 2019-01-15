#include "main.h"

name_info ability_data[] = {{"good"},
{"nobility"},
//
{"loyalty"},
{"wounds"},
{"gold"},
{"fame"},
//
{"economy"},
{"support"},
{"level"},
//
{"recruit"},
{"movement"},
//
{"attack"},
{"defend"},
{"raid"},
{"magic"},
{"sword"},
{"shield"},
};
assert_enum(ability, LastAbility);
bsreq ability_type[] = {
	BSREQ(hero_info, id, text_type),
	BSREQ(hero_info, name, text_type),
	BSREQ(hero_info, nameof, text_type),
	BSREQ(hero_info, text, text_type),
{}};
BSMETA(ability);

const char* name_info::getname(ability_s id, int value) {
	if(value < 0 && ability_data[id].nameof)
		return ability_data[id].nameof;
	return ability_data[id].name;
}

const char* name_info::getnameint(ability_s id, int value) {
	if(value == 1)
		return ability_data[id].name;
	else if(value >= 5)
		return ability_data[id].text;
	return ability_data[id].nameof;
}