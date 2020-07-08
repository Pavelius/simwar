#include "main.h"

static namei gender_data[] = {{"male"},
{"female"},
};
assert_enum(gender, Female);
bsreq gender_type[] = {
	BSREQ(namei, id, text_type),
	BSREQ(namei, name, text_type),
	BSREQ(namei, nameof, text_type),
{}};
BSMETA(gender);