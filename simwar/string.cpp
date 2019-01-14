#include "main.h"

struct string_id_gender {
	const char*		she;
	const char*		he;
};
struct string_id_proc {
	const char*		id;
	void			(string::*proc)();
};
static string_id_gender change_gender[] = {{"а", ""},
{"ла", ""},
{"ась", "ся"},
{"ая", "ый"},
{"ей", "ему"},
{"нее", "него"},
{"она", "он"},
{"ее", "его"},
};
static string_id_proc change_proc[] = {{"cost", &string::addcost},
{"strenght", &string::addstrenght},
};

void string::addcost() {
	add(":gold:%1i", cost.gold);
}

void string::addstrenght() {
	if(army)
		army->getstrenght(this);
}

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
	for(auto& e : change_proc) {
		if(strcmp(e.id, identifier) == 0) {
			(this->*e.proc)();
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