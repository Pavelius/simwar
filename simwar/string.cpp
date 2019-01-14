#include "main.h"

struct gender_change {
	const char*		she;
	const char*		he;
};
static gender_change change_gender[] = {{"а", ""},
{"ла", ""},
{"ась", "ся"},
{"ая", "ый"},
{"ей", "ему"},
{"нее", "него"},
{"она", "он"},
{"ее", "его"},
};

void string::addidentifier(const char* identifier) {
	for(auto& e : change_gender) {
		if(strcmp(e.she, identifier) == 0) {
			if(gender == Female)
				add(e.she);
			else
				add(e.he);
			return;
		}
	}
	//if(strcmp(identifier, "cost") == 0)
	//	print(result, result_max, ":gold:%1i", cost.gold);
	//else if(strcmp(identifier, "strenght") == 0) {
	//	if(army) {
	//		stringcreator sb(*this, result, result_max);
	//		army->getstrenght(&sb);
	//	}
	//} else
	addidentifier(identifier);
}