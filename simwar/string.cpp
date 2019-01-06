#include "main.h"

static void gms(gender_s gender, char* result, const char* result_max, const char* text_male, const char* text_female) {
	stringcreator sc;
	if(gender == Female) {
		if(text_female)
			sc.printv(result, result_max, text_female, 0);
	} else {
		if(text_male)
			sc.printv(result, result_max, text_male, 0);
	}
}

void string::parseidentifier(char* result, const char* result_max, const char* identifier) {
	if(strcmp(identifier, "а") == 0)
		gms(gender, result, result_max, "", identifier);
	else if(strcmp(identifier, "ла") == 0)
		gms(gender, result, result_max, "", identifier);
	else if(strcmp(identifier, "ась") == 0)
		gms(gender, result, result_max, "ся", identifier);
	else if(strcmp(identifier, "ая") == 0)
		gms(gender, result, result_max, "ый", identifier);
	else if(strcmp(identifier, "ей") == 0)
		gms(gender, result, result_max, "ему", identifier);
	else if(strcmp(identifier, "нее") == 0)
		gms(gender, result, result_max, "него", identifier);
	else if(strcmp(identifier, "она") == 0)
		gms(gender, result, result_max, "он", identifier);
	else if(strcmp(identifier, "ее") == 0)
		gms(gender, result, result_max, "его", identifier);
	else {
		zcat(result, "[-");
		zcat(result, identifier);
		zcat(result, "]");
	}
}

void string::set(hero_info* v) {
	hero = v;
	if(hero)
		gender = hero->getgender();
}

void string::clear() {
	stringbuilder::clear();
	gender = Male;
	hero = 0;
	province = 0;
	player = 0;
}

void string::post() const {
	report_info::add(player, province, hero, buffer);
}