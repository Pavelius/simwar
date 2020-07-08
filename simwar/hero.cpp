#include "main.h"

bsreq heroi::metadata[] = {
	BSREQ(heroi, id, text_type),
	BSREQ(heroi, name, text_type),
	BSREQ(heroi, nameof, text_type),
	BSREQ(heroi, text, text_type),
	BSREQ(heroi, ability, number_type),
	BSREQ(heroi, gender, gender_type),
	BSREQ(heroi, avatar, text_type),
	BSREQ(heroi, tactic, tactic_type),
	BSREQ(heroi, best_tactic, tactic_type),
	BSREQ(heroi, origin, landscape_type),
	BSREQ(heroi, traits, trait_type),
	BSREQ(heroi, player, playeri::metadata),
	BSREQ(heroi, province, provincei::metadata),
{}};
adat<heroi, hero_max> hero_data;
bsdata hero_manager("hero", hero_data, heroi::metadata);

void heroi::clear_actions() {
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

void heroi::refresh_heroes() {
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

int	heroi::getincome() const {
	auto result = 0;
	for(auto p : traits) {
		if(p)
			result--;
	}
	result += get(Nobility);
	return result;
}

bool heroi::isallow(const actioni* action) const {
	if(action->getraid() > 0)
		return (getattack() + getraid()) > 0;
	return true;
}

void heroi::setaction(const actioni* action, provincei* province, const tactici* tactic, const costi& cost, const army& logistic, const unit_set& production) {
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

void heroi::resolve() {
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

unsigned heroi::select(heroi** source, unsigned maximum_count) {
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

unsigned heroi::select(heroi** source, unsigned maximum_count, const playeri* player) {
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

void heroi::cancelaction() {
	if(!action)
		return;
	troopi::retreat(province, player);
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

unsigned heroi::remove_hired(heroi** source, unsigned count) {
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

unsigned heroi::remove_this(heroi** source, unsigned count) const {
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

void heroi::check_leave() {
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

void heroi::setplayer(playeri* player) {
	this->player = player;
	if(player) {
		setloyalty(game.loyalty_base);
		ability[Loyalty] += game.loyalty_noble_modifier*get(Good);
	} else
		ability[Loyalty] = 0;
}

void heroi::initialize() {
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

void heroi::getinfo(stringcreator& sb) const {
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

void heroi::getstate(stringcreator& sb) const {
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

void heroi::getbrief(stringcreator& sb) const {
	sb.addn("###%1", getname());
	for(auto p : traits) {
		if(!p)
			continue;
		sb.addn(p->getname());
	}
	sb.addn("%1: %2i", getname(Loyalty), getloyalty());
}

void heroi::neutral_hero_actions() {
	auto default_patrol = actioni::getaction(Defend);
	if(!default_patrol)
		return;
	provincei* source[province_max];
	auto count = provincei::select_friendly(source, lenghtof(source), 0);
	if(!count)
		return;
	zshuffle(source, count);
	playeri* player = 0;
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

void heroi::setloyalty(int value) {
	if(!player)
		ability[Loyalty] = 0;
	else
		ability[Loyalty] = value;
}

provincei* heroi::choose_province(const actioni* action, bool run) const {
	auto choose_mode = action->getprovince();
	adat<provincei*> provinces;
	provinces.count = provincei::select(provinces.data, provinces.getmaximum(), getplayer());
	provinces.count = provincei::remove_mode(provinces, player, choose_mode);
	provinces.count = provincei::remove_hero_present(provinces, player);
	if(!provinces.count)
		return 0;
	if(player->iscomputer()) {
		if(action->getprovince() == NoFriendlyProvince) {
			auto ps = provinces.data;
			for(auto p : provinces) {
				auto raid = action->get(Raid) > 0;
				army troops_move(
					const_cast<playeri*>(player),
					const_cast<provincei*>(p),
					const_cast<heroi*>(this),
					true, raid);
				auto cost = player->cost;
				if(!choose_attack(action, p, troops_move, cost, raid))
					continue;
				*ps++ = p;
			}
			provinces.count = ps - provinces.data;
		}
		if(!provinces.count)
			return 0;
		return provinces[rand() % provinces.count];
	}
	if(!run)
		return provinces[rand() % provinces.count];
	return const_cast<provincei*>(choose_province(action, provinces, choose_mode));
}

void heroi::make_move() {
	auto player = getplayer();
	auto action = choose_action();
	if(!action)
		return;
	costi cost = action->cost;
	const tactici* tactic = 0;
	provincei* province = 0;
	if(action->isplaceable()) {
		province = choose_province(action, true);
		if(!province)
			return;
	}
	auto raid = action->getraid() > 0;
	army troops_move(const_cast<playeri*>(player), const_cast<provincei*>(province), this, true, raid);
	unit_set units_product;
	if(action->get(Raid) || action->get(Attack)) {
		if(!choose_attack(action, province, troops_move, cost, raid))
			return;
	}
	if(action->get(Movement)) {
		army a1(const_cast<playeri*>(player), province, this, false, false);
		army a3;
		a1.fill(player, 0);
		a1.count = troopi::remove(a1.data, a1.count, province);
		a1.count = troopi::remove_moved(a1.data, a1.count);
		a1.count = troopi::remove_restricted(a1.data, a1.count, province);
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

bool heroi::choose_attack(const actioni* action, const provincei* province, army& troops, costi& cost, bool raid) const {
	army a1(getplayer(), const_cast<provincei*>(province), const_cast<heroi*>(this), true, raid);
	army a3(0, const_cast<provincei*>(province), 0, false, raid);
	a1.fill(getplayer(), 0);
	a1.count = troopi::remove_moved(a1.data, a1.count);
	a1.count = troopi::remove_restricted(a1.data, a1.count, province);
	a3.fill(province->getplayer(), province);
	return choose_troops(action, province, a1, troops, a3, 0, cost);
}

bool heroi::choose_units(const actioni* action, const provincei* province, unit_set& a1, unit_set& a2, costi& cost) const {
	if(player->iscomputer())
		return choose_units_computer(action, province, a1, a2, cost);
	return choose_units_human(action, province, a1, a2, cost);
}