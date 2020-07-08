#include "main.h"

bsreq playeri::metadata[] = {
	BSREQ(playeri, id, text_type),
	BSREQ(playeri, name, text_type),
	BSREQ(playeri, nameof, text_type),
	BSREQ(playeri, text, text_type),
	BSREQ(playeri, type, player_ai_type),
	BSREQ(playeri, cost, number_type),
{}};
adat<playeri, player_max> player_data;
bsdata player_manager("player", player_data, playeri::metadata);
static adat<playeri*, player_max> players;

void playeri::post(const heroi* hero, const provincei* province, const char* text) const {
	reporti::add(this, province, hero, text);
}

extern bsdata calendar_manager;

bool playeri::isallow(const actioni* action) const {
	if(action->cost > cost)
		return false;
	return true;
}

unsigned playeri::remove_restricted(actioni** source, unsigned count) const {
	auto ps = source;
	auto pe = source + count;
	for(auto pb = source; pb < pe; pb++) {
		auto p = *pb;
		if(!isallow(p))
			continue;
		*ps++ = p;
	}
	return ps - source;
}

void playeri::getcalendar(stringcreator& sb) {
	auto count = calendar_manager.getcount();
	if(!count)
		return;
	auto calendar_index = game.turn % count;
	auto calendar = (calendari*)calendar_manager.get(calendar_index);
	auto year = game.year + game.turn / count;
	sb.add("%+1 %2i года, %3", calendar->getname(), year, calendar->season->getname(), calendar->season->getnameof());
}

int	playeri::getindex() const {
	return player_data.indexof(this);
}

void playeri::getinfo(stringcreator& sb) const {
	char tips[512];
	stringcreator ti(tips, zendof(tips));
	getcalendar(sb);
	auto income = getincome(&ti);
	sb.adds(":gold:%1i[%4\"%3\"%+2i]", cost.gold, income, tips, (income >= 0) ? "+" : "-");
	sb.adds(":flag_grey:%1i", cost.fame);
}

int playeri::getincome(stringcreator* ti) const {
	auto result = 0, r = 0;
	for(auto& e : province_data) {
		if(!e)
			continue;
		if(e.getplayer() != this)
			continue;
		r += e.getincome();
	}
	result += fix(ti, msg.income_province, r); r = 0;
	for(auto& e : troop_data) {
		if(!e)
			continue;
		if(e.getplayer() != this)
			continue;
		r += e.getincome();
	}
	result += fix(ti, msg.income_units, r); r = 0;
	for(auto& e : hero_data) {
		if(!e)
			continue;
		if(e.getplayer() != this)
			continue;
		r += e.getincome();
	}
	result += fix(ti, msg.income_heroes, r); r = 0;
	return result;
}

int	playeri::getstrenght() const {
	auto result = 0;
	for(auto& e : troop_data) {
		if(!e)
			continue;
		if(e.getplayer()!=this)
			continue;
		result += e.get(Attack);
	}
	return result;
}

int	playeri::getsupport() const {
	auto result = 0;
	for(auto& e : province_data) {
		if(!e)
			continue;
		result += e.getsupport(this);
	}
	return result;
}

unsigned playeri::gettroops(troopi** source, unsigned maximum_count, const provincei* province, const playeri* player, const playeri* player_move) {
	auto ps = source;
	auto pe = ps + maximum_count;
	for(auto& e : troop_data) {
		if(!e)
			continue;
		if(player && e.getplayer() != player)
			continue;
		if(province && e.getprovince(player_move) != province)
			continue;
		if(ps < pe)
			*ps++ = &e;
	}
	return ps - source;
}

provincei* playeri::getbestprovince() const {
	provincei* elements[player_max*2];
	auto count = provincei::select(elements, lenghtof(elements), this);
	if(!count)
		return 0;
	return elements[0];
}

void playeri::gain_profit() {
	for(auto& e : player_data) {
		if(!e)
			continue;
		e.cost.gold += e.getincome();
	}
}

unsigned playeri::getactions(heroi** source, unsigned maximum, int order) {
	auto ps = source;
	auto pe = ps + maximum;
	for(auto& e : hero_data) {
		if(!e)
			continue;
		auto a = e.getaction();
		if(!a)
			continue;
		if(a->order != order)
			continue;
		if(ps < pe)
			*ps++ = &e;
	}
	return ps - source;
}

void playeri::resolve_actions() {
	heroi* heroes[hero_max];
	buildi::build_units();
	for(auto i = 1; i <= 5; i++) {
		// Получим всех игроков
		auto count = getactions(heroes, sizeof(heroes) / sizeof(heroes[0]), i);
		if(!count)
			continue;
		// Все ходы героев идут в порядке игроков
		for(auto p : players) {
			for(unsigned j = 0; j < count; j++) {
				auto hero = heroes[j];
				if(hero->getplayer()==p)
					hero->resolve();
			}
		}
		// Нейтральные герои делают свои ходы в последнюю очередь
		for(unsigned j = 0; j < count; j++) {
			auto hero = heroes[j];
			if(!hero->getplayer())
				hero->resolve();
		}
	}
}

static void create_province_order() {
	for(unsigned i = 0; i < province_data.count; i++)
		province_order[i] = i;
	zshuffle(province_order, province_data.count);
}

int playeri::getherocount() const {
	auto result = 0;
	for(auto& e : hero_data) {
		if(!e || e.getplayer() != this)
			continue;
		result++;
	}
	return result;
}

int playeri::gettroopscount() const {
	auto result = 0;
	for(auto& e : troop_data) {
		if(!e)
			continue;
		if(e.getplayer() == this)
			result++;
	}
	return result;
}

int playeri::getfriendlyprovinces() const {
	auto result = 0;
	for(auto& e : province_data) {
		if(!e)
			continue;
		if(e.getstatus(this) == FriendlyProvince)
			result++;
	}
	return result;
}

int playeri::compare_hire_bet(const void* p1, const void* p2) {
	auto e1 = *((playeri**)p1);
	auto e2 = *((playeri**)p2);
	if(e2->hire_gold < e1->hire_gold)
		return -1;
	if(e2->hire_gold > e1->hire_gold)
		return 1;
	if(e2->cost.fame < e1->cost.fame)
		return -1;
	if(e2->cost.fame > e1->cost.fame)
		return 1;
	if(e2->cost.gold < e1->cost.gold)
		return -1;
	if(e2->cost.gold > e1->cost.gold)
		return 1;
	return 0;
}

void playeri::check_heroes() {
	for(auto& e : hero_data) {
		if(!e)
			continue;
		e.check_leave();
	}
}

void playeri::suggest_heroes() {
	game.hire_hero = 0;
	if(game.hire_turns > 0) {
		game.hire_turns--;
		return;
	}
	game.hire_turns = imax(1, game.hire_turns_range[0] + (rand() % game.hire_turns_range[0]));
	heroi* source[hero_max];
	auto count = heroi::select(source, lenghtof(source), 0);
	if(!count)
		return;
	game.hire_hero = source[rand() % count];
}

void playeri::hire_heroes() {
	if(!game.hire_hero)
		return;
	auto hero = game.hire_hero;
	adat<playeri*, player_max> source;
	// Определим претендентов
	for(auto& e : player_data) {
		if(!e || !e.hire_gold)
			continue;
		if(e.getherocount() >= hero_max_per_player)
			continue;
		source.add(&e);
	}
	// Выбираем лучшего
	qsort(source.data, source.getcount(), sizeof(source.data[0]), compare_hire_bet);
	// Выполним действие
	for(unsigned i = 0; i < source.count; i++) {
		auto player = source.data[i];
		string sb;
		sb.set(hero);
		if(i == 0) {
			sb.add(msg.hero_hire_success, hero->getname(), player->getname(), player->hire_gold);
			player->post(hero, 0, sb);
			hero->setplayer(const_cast<playeri*>(player));
		} else {
			player->cost.gold += player->hire_gold;
			sb.add(msg.hero_hire_fail, hero->getname(), source.data[0]->getname(), player->hire_gold);
			player->post(hero, 0, sb);
		}
		player->hire_gold = 0;
	}
}

void playeri::check_hire() {
	if(!game.hire_hero)
		return;
	auto base_multiplier = 10;
	choiseset ai;
	for(auto i = 0; i < 7; i++) {
		auto value = (i + 1)*base_multiplier;
		if(cost.gold>=value)
			ai.add(value, msg.pay_gold, value);
	}
	if(ai.elements.getcount()==0)
		return;
	ai.add(0, msg.cancel);
	auto pay_value = choose(game.hire_hero, ai, msg.hero_hire, game.hire_hero->getname());
	if(!pay_value)
		return;
	hire_gold = pay_value;
	cost.gold -= hire_gold;
}

bool playeri::isallowgame() {
	for(auto& e : player_data) {
		if(!e)
			continue;
		auto province = e.getbestprovince();
		if(!province)
			return false;
	}
	return true;
}

static void neutrals_move() {
	heroi::neutral_hero_actions();
}

int playeri::compare_fame(const void* p1, const void* p2) {
	auto e1 = *((playeri**)p1);
	auto e2 = *((playeri**)p2);
	auto v1 = e1->cost.fame;
	auto v2 = e2->cost.fame;
	if(v1 < v2)
		return -1;
	else if(v1 > v2)
		return 1;
	v1 = e1->cost.gold;
	v2 = e2->cost.gold;
	if(v1 < v2)
		return -1;
	else if(v1 > v2)
		return 1;
	return 0;
}

void playeri::create_order() {
	players.clear();
	for(auto& e : player_data) {
		if(!e)
			continue;
		players.add(&e);
	}
	qsort(players.data, players.count, sizeof(players[0]), compare_fame);
}

static heroi* getmatched(aref<heroi*> source, const playeri* player) {
	for(auto p : source) {
		if(p->getplayer() == player)
			return p;
	}
	return 0;
}

void playeri::random_events() {
	adat<provincei*> provinces;
	adat<heroi*, hero_max> heroes;
	provinces.count = provincei::select(provinces.data, provinces.getmaximum());
	zshuffle(provinces.data, provinces.count);
	heroes.count = heroi::select(heroes.data, heroes.getmaximum());
	zshuffle(heroes.data, heroes.count);
	unsigned province_index = 0;
	for(int i = 0; i < 4; i++) {
		if(province_index >= provinces.count)
			province_index = 0;
		auto province = provinces.data[province_index++];
		auto player = province->getplayer();
		if(!player)
			continue;
		auto hero = getmatched(heroes, player);
		eventi::random(province, hero);
		if(hero)
			heroes.remove(heroes.indexof(hero));
	}
}

void playeri::desert_units() {
	if(!game.desert_base)
		return;
	for(auto p : players) {
		if(p->cost.gold > -game.desert_base)
			continue;
		troopi* source[32];
		auto count = troopi::select(source, lenghtof(source), p);
		if(!count)
			continue;
		zshuffle(source, count);
		unsigned desert_count = -p->cost.gold / game.desert_base;
		if(desert_count > count)
			desert_count = count;
		if(!desert_count)
			continue;
		string sb;
		sb.add(msg.troops_desert);
		auto start = sb.get();
		for(unsigned i = 0; i < desert_count; i++) {
			if(!sb.ispos(start))
				sb.add(", ");
			else
				sb.add(": ");
			sb.add("%+1", source[i]->getname());
			source[i]->kill(0);
		}
		sb.add(".");
		p->post(0, 0, sb);
	}
}

void playeri::playgame() {
	create_province_order();
	while(isallowgame()) {
		heroi::clear_actions();
		for(auto& e : player_data) {
			if(!e)
				continue;
			e.check_hire();
			e.show_reports();
			switch(e.type) {
			case PlayerHuman: e.make_move(); break;
			case PlayerComputer: e.computer_move(); break;
			}
		}
		game.turn++;
		neutrals_move();
		heroi::refresh_heroes();
		create_order();
		resolve_actions();
		random_events();
		desert_units();
		gain_profit();
		check_heroes();
		hire_heroes();
		suggest_heroes();
		provincei::change_support();
	}
}