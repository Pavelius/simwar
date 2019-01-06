#include "main.h"

static name_info gender_data[] = {{"male"},
{"female"},
};
assert_enum(gender, Female);
bsreq gender_type[] = {
	BSREQ(name_info, id, text_type),
	BSREQ(name_info, name, text_type),
	BSREQ(name_info, nameof, text_type),
{}};
BSMETA(gender);