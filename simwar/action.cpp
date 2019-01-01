#include "main.h"

bsreq action_type[] = {
	BSREQ(action_info, id, text_type),
	BSREQ(action_info, name, text_type),
	BSREQ(action_info, text, text_type),
	BSREQ(action_info, attack, number_type),
	BSREQ(action_info, raid, number_type),
	BSREQ(action_info, defend, number_type),
	BSREQ(action_info, profit, number_type),
	BSREQ(action_info, recruit, number_type),
	BSREQ(action_info, support, number_type),
	BSREQ(action_info, placeable, number_type),
	BSREQ(action_info, nameact, text_type),
{}};
adat<action_info, 32> action_data; BSMETA(action);

province_type_s action_info::getprovince() const {
	if(attack > 0 || raid > 0)
		return HositleProvince;
	else if(defend > 0)
		return FriendlyProvince;
	return AnyProvince;
}