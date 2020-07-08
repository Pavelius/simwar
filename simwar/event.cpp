#include "main.h"

bsreq event_type[] = {
	BSREQ(eventi, ability, number_type),
	BSREQ(eventi, id, text_type),
	BSREQ(eventi, name, text_type),
	BSREQ(eventi, text, text_type),
	BSREQ(eventi, landscape, landscape_type),
	BSREQ(eventi, effects, effect_type),
	BSREQ(eventi, count, number_type),
{}};
adat<eventi, 64> event_data;
BSMETA(event);
static adat<eventi*, 256> deck;
static unsigned deck_index;

void eventi::initialize() {
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

bool eventi::isvalid(const provincei& e) const {
	if(landscape && e.getlandscape() != landscape)
		return false;
	return objecti::isvalid(e);
}

unsigned eventi::select(provincei** source, unsigned maximum) const {
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

const eventi* eventi::getnext() {
	if(!deck)
		return 0;
	if(deck_index >= deck.count)
		deck_index = 0;
	return deck.data[deck_index++];
}

void eventi::play(provincei* province, heroi* hero) const {
	if(!province)
		return;
	auto player = province->getplayer();
	if(!player)
		return;
	string sb;
	sb.set(hero);
	sb.set(player);
	sb.set(province);
	sb.add(name);
	auto result = 0;
	if(hero) {
		for(int i = 1; i < sizeof(effects) / sizeof(effects[0]); i++) {
			if(!effects[i].isvalid(*hero))
				break;
			sb.adds(effects[i].text);
			effects[i].apply(sb, player, province, hero);
			result++;
		}
	}
	if(result == 0) {
		sb.adds(effects[0].text);
		effects[0].apply(sb, player, province, hero);
	}
	player->post(hero, province, sb);
}

void eventi::random(provincei* province, heroi* hero) {
	for(unsigned i = 0; i < deck.count; i++) {
		auto p = getnext();
		if(!p->isvalid(*province))
			continue;
		p->play(province, hero);
		break;
	}
}