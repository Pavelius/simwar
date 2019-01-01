#include "main.h"

int name_info::fix(tip_info* ti, const char* name, int value) {
	if(ti && value) {
		char temp[260]; temp[0] = 0;
		if(name) {
			zcpy(temp, name);
			szupper(temp, 1);
		}
		szprint(zend(ti->result), ti->result_max, ti->text, value, temp);
		if(ti->separator)
			zcat(ti->result, ti->separator);
	}
	return value;
}