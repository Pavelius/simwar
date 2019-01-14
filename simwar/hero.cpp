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
	BSREQ(hero_info, origin, landscape_type),
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

int	hero_info::getincome() const {
	auto result = 0;
	for(auto p : traits) {
		if(p)
			result--;
	}
	result += get(Nobility);
	return result;
}

bool hero_info::isallow(const action_info* action) const {
	if(action->getraid() > 0)
		return (getattack() + getraid()) > 0;
	return true;
}

void hero_info::setaction(const action_info* action, province_info* province, const tactic_info* tactic, const cost_info& cost, const army& logistic, const unit_set& production) {
	cancelaction();
	setaction(action);
	setprovince(province);
	settactic(tactic);
	pay += cost;
	if(player)
		player->cost -= pay;
	for(auto p : logistic)
		p->setmove(province);
	for(auto p : production)
		province->build(p, p->recruit_time);
}

void hero_info::resolve() {
	if(!action)
		return;
	string sb;
	if(!province)
		return;
	if(action->getattack() || action->getraid()) {
		auto israid = (action->getraid() > 0);
		auto enemy = province->getplayer();
		if(enemy != player) {
			province->battle(sb, player, enemy, action, israid);
			player->post(this, province, sb);
			enemy->post(province->gethero(enemy), province, sb);
		}
	}
	province->addsupport(player, action->support);
	province->addsupport(player, get(Good));
	province->addeconomy(action->economy);
	setloyalty(getloyalty() + action->get(Good)*get(Good));
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
		player->cost += pay;
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

const action_info* hero_info::choose_action() const {
	answer_info ai;
	for(auto& e : action_data) {
		if(!e)
			continue;
		if(!isallow(&e))
			continue;
		if(player) {
			if(!player->isallow(&e))
				continue;
		}
		ai.add((int)&e, e.getname());
	}
	if(!ai)
		return 0;
	if(player->iscomputer())
		return (action_info*)ai.elements.data[rand()%ai.elements.count].param;
	ai.sort();
	return (action_info*)ai.choose(this);
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
	if(player) {
		loyalty = game.loyalty_base;
		loyalty += game.loyalty_noble_modifier*get(Good);
	} else
		loyalty = 0;
}

void hero_info::initialize() {
	for(auto& e : hero_data) {
		if(!e)
			continue;
		for(auto p : e.traits) {
			for(auto i = Attack; i <= LastAbility; i = (ability_s)(i + 1))
				e.ability[i] += p->get(i);
		}
		if(!e.player)
			continue;
		e.setplayer(e.player);
	}
}

void hero_info::getinfo(stringcreator& sb) const {
	auto ph = metadata;
	for(auto i = Attack; i<=LastAbility; i = (ability_s)(i+1)) {
		auto value = get(i);
		if(!value)
			continue;
		auto pn = ability_data[i].name;
		if(value < 0 && ability_data[i].nameof) {
			pn = ability_data[i].nameof;
			value = -value;
		}
		sb.addn("%+2i %1", pn, value);
	}
}

void hero_info::getstate(stringcreator& sb) const {
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

void hero_info::getbrief(stringcreator& sb) const {
	sb.addn("###%1", getname());
	for(auto p : traits) {
		if(!p)
			continue;
		sb.addn(p->getname());
	}
	sb.addn("%1: %2i", msg.loyalty, getloyalty());
}

void hero_info::neutral_hero_actions() {
	auto default_patrol = action_info::getaction(0, 1, 0);
	if(!default_patrol)
		return;
	province_info* source[province_max];
	auto count = province_info::select_friendly(source, lenghtof(source), 0);
	if(!count)
		return;
	zshuffle(source, count);
	player_info* player = 0;
	for(auto& e : hero_data) {
		if(!e)
			continue;
		if(!e.isready())
			continue;
		if(e.getplayer() != player)
			continue;
		if(game.hire_hero == &e)
			continue;
		if(e.action!=game.default_action)
			continue;
		if(!count)
			break;
		e.action = default_patrol;
		e.province = source[--count];
	}
}

void hero_info::setloyalty(int value) {
	if(!player)
		loyalty = 0;
	else
		loyalty = value;
}

void hero_info::make_move() {
	auto player = getplayer();
	auto action = choose_action();
	if(!action)
		return;
	cost_info cost = action->cost;
	const tactic_info* tactic = 0;
	province_info* province = 0;
	if(action->isplaceable()) {
		auto choose_mode = action->getprovince();
		province_info* provinces[128];
		auto count = province_info::select(provinces, lenghtof(provinces), player);
		count = province_info::remove_mode({provinces, count}, player, choose_mode);
		count = province_info::remove_hero_present({provinces, count}, player);
		if(!count)
			return;
		if(player->iscomputer())
			province = provinces[rand() % count];
		else
			province = const_cast<province_info*>(choose_province(action, {provinces, count}, choose_mode));
		if(!province)
			return;
	}
	auto raid = action->getraid() > 0;
	army troops_move(const_cast<player_info*>(player), const_cast<province_info*>(province), this, true, raid);
	unit_set units_product;
	if(action->get(Raid) || action->get(Attack)) {
		army a1(const_cast<player_info*>(player), const_cast<province_info*>(province), this, true, raid);
		army a3(0, const_cast<province_info*>(province), 0, false, raid);
		a1.fill(player, 0);
		a1.count = troop_info::remove_moved(a1.data, a1.count);
		a1.count = troop_info::remove_restricted(a1.data, a1.count, province);
		a3.fill(province->getplayer(), province);
		if(!choose_troops(action, province, a1, troops_move, a3, 0, cost))
			return;
	}
	if(action->movement) {
		army a1(const_cast<player_info*>(player), province, this, false, false);
		army a3;
		a1.fill(player, 0);
		a1.count = troop_info::remove(a1.data, a1.count, province);
		a1.count = troop_info::remove_moved(a1.data, a1.count);
		a1.count = troop_info::remove_restricted(a1.data, a1.count, province);
		if(!choose_troops(action, province, a1, troops_move, a3, 1, cost))
			return;
	}
	if(action->get(Raid) || action->get(Attack) || action->get(Defend)) {
		tactic = choose_tactic();
		if(!tactic)
			return;
	}
	if(action->recruit) {
		unit_set a1; a1.fill(player, province, this, action);
		if(!choose_units(action, province, a1, units_product, cost))
			return;
	}
	setaction(action, province, tactic, cost, troops_move, units_product);
}