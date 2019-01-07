#include "main.h"

bsreq hero_info::metadata[] = {
	BSREQ(hero_info, id, text_type),
	BSREQ(hero_info, name, text_type),
	BSREQ(hero_info, nameof, text_type),
	BSREQ(hero_info, text, text_type),
	BSREQ(hero_info, gender, gender_type),
	BSREQ(hero_info, avatar, text_type),
	BSREQ(hero_info, tactic, tactic_type),
	BSREQ(hero_info, best_tactic, tactic_type),
	BSREQ(hero_info, traits, trait_type),
	BSREQ(hero_info, player, player_info::metadata),
	BSREQ(hero_info, province, province_info::metadata),
	BSREQ(hero_info, wait, number_type),
{}};
adat<hero_info, hero_max> hero_data;
bsdata hero_manager("hero", hero_data, hero_info::metadata);

void hero_info::refresh_heroes() {
	for(auto& e : hero_data) {
		if(!e)
			continue;
		if(e.wait > 0)
			e.wait--;
		if(e.wound > 0)
			e.wound--;
		if(e.wait == 0) {
			e.tactic = 0;
			e.province = 0;
			e.action = game.default_action;
		}
	}
}

int hero_info::get(const char* id) const {
	auto r = 0;
	assert(trait_type->find(id) != 0);
	for(auto p : traits) {
		if(!p)
			continue;
		r += p->get(id);
	}
	return r;
}

int hero_info::getbonus(const char* id) const {
	return 0;
}

int	hero_info::getincome() const {
	auto result = 0;
	for(auto p : traits) {
		if(p)
			result--;
	}
	result += getnobility();
	return result;
}

bool hero_info::isallow(const action_info* action) const {
	if(action->raid > 0)
		return (getattack() + getraid()) > 0;
	if(action->support > 0)
		return (action->support + getdiplomacy()) > 0;
	return true;
}

void hero_info::setaction(const action_info* action, province_info* province, const tactic_info* tactic, const cost_info& cost, const army& logistic, const unit_set& production) {
	cancelaction();
	setaction(action);
	setprovince(province);
	settactic(tactic);
	pay = cost;
	pay += *action;
	if(player)
		*player -= pay;
	for(auto p : logistic)
		p->setmove(province);
	for(auto p : production)
		province->build(p, p->recruit_time);
}

void hero_info::resolve() {
	string sb;
	// ƒалее идут действи€, которые действуют на провинцию
	if(!province)
		return;
	if(action->attack || action->raid) {
		auto israid = (action->raid > 0);
		auto enemy = province->getplayer();
		if(enemy != player) {
			province->battle(sb, player, enemy, action, israid);
			player->post(this, province, sb);
			enemy->post(province->gethero(enemy), province, sb);
		}
	}
	if(action->support)
		province->addsupport(player, action->support + getdiplomacy());
	province->addsupport(player, getgood());
	province->addeconomy(action->economy);
	if(action->defend)
		province->addsupportex(player, -action->defend - imax(0, getdefend()), 0, game.support_maximum);
	// ƒоброе или злое действие вли€ет на ло€льность геро€
	setloyalty(getloyalty() + action->good*getgood());
}

unsigned hero_info::select(hero_info** source, unsigned maximum_count, const player_info* player) {
	auto ps = source;
	auto pe = ps + maximum_count;
	for(auto& e : hero_data) {
		if(!e)
			continue;
		if(!e.isready())
			continue;
		if(e.getplayer() != player)
			continue;
		if(ps < pe)
			*ps++ = &e;
	}
	return ps - source;
}

void hero_info::cancelaction() {
	if(!action)
		return;
	troop_info::retreat(province, player);
	for(auto& e : build_data) {
		if(!e)
			continue;
		if(e.province == province)
			e.clear();
	}
	if(player)
		*player += pay;
	pay.clear();
	tactic = 0;
}

unsigned hero_info::remove_hired(hero_info** source, unsigned count) {
	auto ps = source;
	auto pe = source + count;
	for(auto pb = source; pb < pe; pb++) {
		auto p = *pb;
		if(p->player)
			continue;
		*ps++ = p;
	}
	return ps - source;
}

unsigned hero_info::remove_this(hero_info** source, unsigned count) const {
	auto ps = source;
	auto pe = source + count;
	for(auto pb = source; pb < pe; pb++) {
		auto p = *pb;
		if(p == this)
			continue;
		*ps++ = p;
	}
	return ps - source;
}

unsigned hero_info::select(action_info** source, unsigned maximum_count) const {
	auto ps = source;
	auto pe = ps + maximum_count;
	for(auto& e : action_data) {
		if(!e)
			continue;
		if(!isallow(&e))
			continue;
		if(ps < pe)
			*ps++ = &e;
	}
	return ps - source;
}

void hero_info::check_leave() {
	if(!player)
		return;
	if(loyalty <= 0) {
		string sb;
		sb.gender = getgender();
		sb.add(msg.hero_desert, getname());
		player->post(this, 0, sb);
		player = 0;
	}
}

void hero_info::setplayer(player_info* player) {
	this->player = player;
	loyalty = game.loyalty_base + game.loyalty_noble_modifier*getgood();
}

void hero_info::initialize() {
	for(auto& e : hero_data) {
		if(!e)
			continue;
		if(!e.player)
			continue;
		e.setplayer(e.player);
	}
}

int	hero_info::gethirecost(const player_info* player) const {
	return game.hire_cost;
}

void hero_info::getinfo(stringbuilder& sb) const {
	auto ph = metadata;
	for(auto ppf = character_type; *ppf; ppf++) {
		auto pf = msg_type->find(ppf->id);
		if(!pf)
			continue;
		auto pn = (const char*)pf->get(pf->ptr(&msg));
		if(!pn)
			continue;
		auto value = get(ppf->id);
		if(!value)
			continue;
		if(value < 0) {
			char name[64]; ;
			auto pf1 = msg_type->find(zprint(name, "%1_negative", pf->id));
			if(pf1) {
				pn = (const char*)pf1->get(pf1->ptr(&msg));
				value = -value;
			}
		}
		sb.addn("%+2i %1", pn, value);
	}
}

void hero_info::getstate(stringbuilder& sb) const {
	if(action) {
		sb.addn("[+");
		sb.add("%+1", action->getnameof());
		if(province) {
			sb.add(" %1 ", msg.word_in);
			sb.add(province->getname());
		}
		if(tactic)
			sb.adds("(%1)", tactic->getname());
		sb.add("]");
	}
	auto value = getwait();
	if(value > 0) {
		sb.addn("[-");
		sb.addn(msg.hero_wait, value);
		sb.add("]");
	}
	value = getwound();
	if(value > 0) {
		sb.addn("[-");
		sb.add(msg.hero_wound, value);
		sb.add("]");
	}
}

void hero_info::getbrief(stringbuilder& sb) const {
	sb.addn("###%1", getname());
	for(auto p : traits) {
		if(!p)
			continue;
		sb.addn(p->getname());
	}
	sb.addn("%1: %2i", msg.loyalty, getloyalty());
}