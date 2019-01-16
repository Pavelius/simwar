#include "main.h"

bsreq event_type[] = {
	BSREQ(event_info, ability, number_type),
	BSREQ(event_info, id, text_type),
	BSREQ(event_info, name, text_type),
	BSREQ(event_info, text, text_type),
	BSREQ(event_info, landscape, landscape_type),
	BSREQ(event_info, effects, effect_type),
	BSREQ(event_info, count, number_type),
{}};
adat<event_info, 64> event_data;
BSMETA(event);
static adat<event_info*, 256> deck;
static unsigned deck_index;

void event_info::initialize() {
	deck.clear();
	for(auto& e : event_data) {
		auto count = e.count;
		if(!count)
			count = 1;
		for(auto i = 0; i < count; i++)
			deck.add(&e);
	}
	zshuffle(deck.data, deck.count);
}

bool event_info::isvalid(const province_info& e) const {
	if(landscape && e.getlandscape() != landscape)
		return false;
	return object_info::isvalid(e);
}

unsigned event_info::select(province_info** source, unsigned maximum) const {
	auto ps = source;
	auto pe = ps + maximum;
	for(auto& e : province_data) {
		if(!e)
			continue;
		if(!isvalid(e))
			continue;
		if(ps < pe)
			*ps++ = &e;
	}
	return ps - source;
}

const event_info* event_info::getnext() {
	if(!deck)
		return 0;
	if(deck_index >= deck.count)
		deck_index = 0;
	return deck.data[deck_index++];
}

void event_info::play(province_info* province, hero_info* hero) const {
	if(!province || !hero)
		return;
	auto player = hero->getplayer();
	if(!player)
		return;
	string sb;
	sb.set(hero);
	sb.set(player);
	sb.set(province);
	sb.add(name);
	auto result = 0;
	effect_info total;
	for(int i = 1; i < sizeof(effects) / sizeof(effects[0]); i++) {
		if(!effects[i].isvalid(*hero))
			break;
		total += effects[i];
		sb.adds(effects[i].text);
		result++;
	}
	if(result == 0) {
		total = effects[0];
		sb.adds(effects[0].text);
	}
	//total.apply(province);
	total.apply(player);
	total.apply(hero);
	player->post(hero, province, sb);
}

void event_info::random(province_info* province, hero_info* hero) {
	for(unsigned i = 0; i < deck.count; i++) {
		auto p = getnext();
		if(!p->isvalid(*province))
			continue;
		p->play(province, hero);
	}
}