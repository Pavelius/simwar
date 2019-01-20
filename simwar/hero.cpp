#include "main.h"

bsreq hero_info::metadata[] = {
	BSREQ(hero_info, id, text_type),
	BSREQ(hero_info, name, text_type),
	BSREQ(hero_info, nameof, text_type),
	BSREQ(hero_info, text, text_type),
	BSREQ(hero_info, ability, number_type),
	BSREQ(hero_info, gender, gender_type),
	BSREQ(hero_info, avatar, text_type),
	BSREQ(hero_info, tactic, tactic_type),
	BSREQ(hero_info, best_tactic, tactic_type),
	BSREQ(hero_info, origin, landscape_type),
	BSREQ(hero_info, traits, trait_type),
	BSREQ(hero_info, player, player_info::metadata),
	BSREQ(hero_info, province, province_info::metadata),
{}};
adat<hero_info, hero_max> hero_data;
bsdata hero_manager("hero", hero_data, hero_info::metadata);

void hero_info::clear_actions() {
	for(auto& e : hero_data) {
		if(!e)
			continue;
		if(e.get(Wait) == 0) {
			e.tactic = 0;
			e.province = 0;
			e.action = game.default_action;
		}
	}
}

void hero_info::refresh_heroes() {
	for(auto& e : hero_data) {
		if(!e)
			continue;
		e.pay.clear();
		if(e.get(Wait) > 0)
			e.set(Wait, e.get(Wait) - 1);
		if(e.get(Wounds) > 0)
			e.set(Wounds, e.get(Wounds) - 1);
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
		province->build(p, p->get(Wait));
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
	} else {
		if(player) {
			player->cost.gold += action->get(Gold);
			player->cost.gold += action->get(Fame);
		}
	}
	province->addsupport(player, action->get(Support));
	province->addsupport(player, get(Good));
	province->addeconomy(action->get(Economy));
	setloyalty(getloyalty() + action->get(Good)*get(Good));
}

unsigned hero_info::select(hero_info** source, unsigned maximum_count) {
	auto ps = source;
	auto pe = ps + maximum_count;
	for(auto& e : hero_data) {
		if(!e)
			continue;
		if(!e.isready())
			continue;
		if(!e.getplayer())
			continue;
		if(ps < pe)
			*ps++ = &e;
	}
	return ps - source;
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

void hero_info::check_leave() {
	if(!player)
		return;
	if(get(Loyalty) <= 0) {
		string sb;
		sb.set(this);
		sb.add(msg.hero_desert, getname());
		player->post(this, 0, sb);
		player = 0;
	}
}

void hero_info::setplayer(player_info* player) {
	this->player = player;
	if(player) {
		setloyalty(game.loyalty_base);
		ability[Loyalty] += game.loyalty_noble_modifier*get(Good);
	} else
		ability[Loyalty] = 0;
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
	sb.addn("%1: %2i", getname(Loyalty), getloyalty());
}

void hero_info::neutral_hero_actions() {
	auto default_patrol = action_info::getaction(Defend);
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
		ability[Loyalty] = 0;
	else
		ability[Loyalty] = value;
}

province_info* hero_info::choose_province(const action_info* action) const {
	auto choose_mode = action->getprovince();
	adat<province_info*> provinces;
	provinces.count = province_info::select(provinces.data, provinces.getmaximum(), getplayer());
	provinces.count = province_info::remove_mode(provinces, player, choose_mode);
	provinces.count = province_info::remove_hero_present(provinces, player);
	if(!provinces.count)
		return 0;
	if(player->iscomputer())
		return provinces[rand() % provinces.count];
	else
		return const_cast<province_info*>(choose_province(action, provinces, choose_mode));
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
		province = choose_province(action);
		if(!province)
			return;
	}
	auto raid = action->getraid() > 0;
	army troops_move(const_cast<player_info*>(player), const_cast<province_info*>(province), this, true, raid);
	unit_set units_product;
	if(action->get(Raid) || action->get(Attack)) {
		if(!choose_attack(action, province, troops_move, cost, raid))
			return;
	}
	if(action->get(Movement)) {
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
	if(action->get(Recruit)) {
		unit_set a1; a1.fill(player, province, this, action);
		if(!choose_units(action, province, a1, units_product, cost))
			return;
	}
	setaction(action, province, tactic, cost, troops_move, units_product);
}

bool hero_info::choose_attack(const action_info* action, const province_info* province, army& troops, cost_info& cost, bool raid) const {
	army a1(getplayer(), const_cast<province_info*>(province), const_cast<hero_info*>(this), true, raid);
	army a3(0, const_cast<province_info*>(province), 0, false, raid);
	a1.fill(getplayer(), 0);
	a1.count = troop_info::remove_moved(a1.data, a1.count);
	a1.count = troop_info::remove_restricted(a1.data, a1.count, province);
	a3.fill(province->getplayer(), province);
	return choose_troops(action, province, a1, troops, a3, 0, cost);
}

bool hero_info::choose_units(const action_info* action, const province_info* province, unit_set& a1, unit_set& a2, cost_info& cost) const {
	if(player->iscomputer())
		return choose_units_computer(action, province, a1, a2, cost);
	return choose_units_human(action, province, a1, a2, cost);
}

bool hero_info::choose_troops(const action_info* action, const province_info* province, army& a1, army& a2, army& a3, int minimal_count, cost_info& cost) const {
	if(player->iscomputer())
		return choose_troops_computer(action, province, a1, a2, a3, minimal_count, cost);
	return choose_troops_human(action, province, a1, a2, a3, minimal_count, cost);
}