#include "main.h"

struct string_id {
	const char*		id;
	const char*		url;
};
struct string_id_proc {
	const char*		id;
	void			(string::*proc)();
};
static string_id change_gender[] = {{"�", ""},
{"��", ""},
{"���", "��"},
{"��", "��"},
{"��", "���"},
{"���", "����"},
{"���", "��"},
{"��", "���"},
};
static string_id change_properties[] ={{"�����", "hero.name"},
{"�����", "hero.nameof"},
{"hero", "hero.name"},
{"province", "province.name"},
{"player", "player.name"},
};
static string_id_proc change_proc[] = {{"cost", &string::addcost},
{"strenght", &string::addstrenght},
{"player_income", &string::addplayerincome},
};
bsreq string::metadata[] = {
	BSREQ(string, hero, hero_info::metadata),
	BSREQ(string, player, player_info::metadata),
	BSREQ(string, province, province_info::metadata),
{}};

string::string() : stringcreator(buffer, buffer + sizeof(buffer) / sizeof(buffer[0])),
army(0), hero(0), player(0), province(0) {}

void string::addcost() {
	add(":gold:%1i", cost.gold);
}

void string::addstrenght() {
	if(army)
		army->getstrenght(this);
}

void string::addplayerincome() {
	if(player)
		player->getincome(this);
}

void string::addidentifier(const char* identifier) {
	auto gender = Male;
	if(hero)
		gender = hero->getgender();
	for(auto& e : change_gender) {
		if(strcmp(e.id, identifier) == 0) {
			if(gender == Female)
				add(e.id);
			else
				add(e.url);
			return;
		}
	}
	for(auto& e : change_properties) {
		if(strcmp(e.id, identifier) == 0) {
			bsval bv = {this, string::metadata};
			bv.get(e.url);
			if(!bv) {
				stringcreator::addidentifier(e.url);
				return;
			}
			if(bv.type->type == text_type) {
				auto p = (const char*)bv.get();
				if(p)
					add(p);
			} else if(bv.type->type == number_type)
				add("%1i", bv.get());
			return;
		}
	}
	for(auto& e : change_proc) {
		if(strcmp(e.id, identifier) == 0) {
			(this->*e.proc)();
			return;
		}
	}
	stringcreator::addidentifier(identifier);
}