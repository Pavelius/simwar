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

string::string() : stringcreator(buffer, buffer + sizeof(buffer) / sizeof(buffer[0])), army(0) {}

void string::addcost() {
	add(":gold:%1i", cost.gold);
}

void string::addstrenght() {
	if(army)
		army->getstrenght(this);
}

void string::addidentifier(const char* identifier) {
	auto gender = Male;
	if(hero)
		gender = hero->getgender();
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
	addidentifier(identifier);
}

void string::set(const hero_info* value) {
	hero = value;
}

void string::set(const player_info* value) {
	player = value;
}
