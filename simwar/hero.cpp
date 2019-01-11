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
	// ����� ���� ��������, ������� ��������� �� ���������
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
	// ������ ��� ���� �������� ������ �� ���������� �����
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
	if(player)
		loyalty = game.loyalty_base + game.loyalty_noble_modifier*getgood();
	else
		loyalty = 0;
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

void hero_info::neutral_hero_actions() {
	auto default_patrol = action_info::getaction(0, 1, 0);
	if(!default_patrol)
		return;
	province_info* source[province_max];
	auto count = province_info::select(source, lenghtof(source), 0, FriendlyProvince);
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
		auto count = province_info::select(provinces, sizeof(provinces) / sizeof(provinces[0]), player, choose_mode);
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
	auto raid = action->raid > 0;
	army troops_move(const_cast<player_info*>(player), const_cast<province_info*>(province), this, true, raid);
	unit_set units_product;
	if(action->raid || action->attack) {
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
	if(action->raid || action->attack || action->defend) {
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