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
	if(strcmp(identifier, "�") == 0)
		gms(gender, result, result_max, "", identifier);
	else if(strcmp(identifier, "��") == 0)
		gms(gender, result, result_max, "", identifier);
	else if(strcmp(identifier, "���") == 0)
		gms(gender, result, result_max, "��", identifier);
	else if(strcmp(identifier, "��") == 0)
		gms(gender, result, result_max, "��", identifier);
	else if(strcmp(identifier, "��") == 0)
		gms(gender, result, result_max, "���", identifier);
	else if(strcmp(identifier, "���") == 0)
		gms(gender, result, result_max, "����", identifier);
	else if(strcmp(identifier, "���") == 0)
		gms(gender, result, result_max, "��", identifier);
	else if(strcmp(identifier, "��") == 0)
		gms(gender, result, result_max, "���", identifier);
	else if(strcmp(identifier, "cost") == 0) {
		print(result, result_max, ":gold:%1i", cost.gold);
	} else {
		zcat(result, "[-");
		zcat(result, identifier);
		zcat(result, "]");
	}
}