#include "main.h"

namei ability_data[] = {{"good"},
{"nobility"},
//
{"loyalty"},
{"wounds"},
{"wait"},
//
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
	BSREQ(heroi, id, text_type),
	BSREQ(heroi, name, text_type),
	BSREQ(heroi, nameof, text_type),
	BSREQ(heroi, text, text_type),
{}};
BSMETA(ability);

const char* namei::getname(ability_s id, int value) {
	if(value < 0 && ability_data[id].nameof)
		return ability_data[id].nameof;
	return ability_data[id].name;
}

const char* namei::getnameint(ability_s id, int value) {
	if(value == 1)
		return ability_data[id].name;
	else if(value >= 5)
		return ability_data[id].text;
	return ability_data[id].nameof;
}