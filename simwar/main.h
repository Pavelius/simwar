#include "bsdata.h"
#include "bslog.h"
#include "collection.h"
#include "crt.h"
#include "msg.h"
#include "point.h"
#include "stringcreator.h"

#pragma once

const int player_max = 8;
const int hero_max_per_player = 4;
const int hero_max = player_max * hero_max_per_player;
const int province_max = player_max * 16;

enum province_flag_s {
	AnyProvince,
	NoFriendlyProvince, FriendlyProvince, NeutralProvince,
};
enum player_ai_s : unsigned char {
	NoPlayer,
	PlayerHuman, PlayerComputer,
};
enum gender_s : unsigned char {
	Male, Female
};
enum ability_s : unsigned char {
	Good, Nobility,
	Loyalty, Wounds, Wait,
	Gold, Fame,
	Economy, Support, Level,
	Recruit, Movement,
	Attack, Defend, Raid, Magic, Sword, Shield,
	LastAbility = Shield
};
enum event_type_s : unsigned char {
	GlobalEvent, PlayerEvent, ProvinceEvent, HeroEvent,
};

bsreq ability_type[];
bsreq action_type[];
bsreq calendar_type[];
bsreq character_type[];
bsreq cost_type[];
bsreq effect_type[];
bsreq game_type[];
bsreq gender_type[];
bsreq landscape_type[];
bsreq msg_type[];
bsreq nation_type[];
bsreq point_type[];
bsreq player_ai_type[];
bsreq season_type[];
bsreq tactic_type[];
bsreq trait_type[];
bsreq unit_type[];

struct army;
struct heroi;
struct playeri;
struct provincei;
struct troopi;
struct uniti;
struct unit_set;

struct costi {
	short						gold, fame;
	constexpr costi() : gold(0), fame(0) {}
	constexpr explicit operator bool() const { return gold || fame; }
	void operator+=(const costi& e) { gold += e.gold; fame += e.fame; }
	void operator+=(const int value) { gold += value; }
	void operator-=(const costi& e) { gold -= e.gold; fame -= e.fame; }
	bool operator>(const costi& e) const { return gold > e.gold; }
	void						clear();
	void						getinfo(stringcreator& sb) const;
};
struct string : stringcreator {
	string();
	void						addidentifier(const char* identifier) override;
	void						addcost();
	void						addplayerincome();
	void						addstrenght();
	static bsreq				metadata[];
	void						set(const heroi* value) { hero = value; }
	void						set(const playeri* value) { player = value; }
	void						set(const provincei* value) { province = value; }
	void						set(const costi& value) { cost = value; }
private:
	costi						cost;
	const struct army*			army;
	const playeri*				player;
	const heroi*				hero;
	const provincei*			province;
	char						buffer[8192];
};
struct namei {
	const char*					id;
	const char*					name;
	const char*					nameof;
	const char*					text;
	constexpr namei() : id(0), name(0), nameof(), text(0) {}
	constexpr namei(const char* id) : id(id), name(0), nameof(), text(0) {}
	explicit operator bool() const { return id != 0; }
	int							compare(const void* p1, const void* p2);
	const char*					getid() const { return id; }
	const char*					getname() const { return name; }
	static const char*			getname(ability_s id, int value = 0);
	static const char*			getnameint(ability_s id, int value);
	const char*					getnameof() const { return nameof; }
	static int					getnum(const void* object, const bsreq* type, const char* id);
	static int					fix(stringcreator* ti, const char* name, int value);
	int							fix(stringcreator* ti, int value) const { return fix(ti, name, value); }
};
struct objecti : namei {
	char						ability[LastAbility + 1];
	void						add(ability_s id, char value) { ability[id] += value; }
	int							get(ability_s id) const { return ability[id]; }
	static bool					ismatch(int v1, int v2);
	bool						isvalid(const playeri& e) const;
	bool						isvalid(const provincei& e) const;
	void						set(ability_s id, char value) { ability[id] = value; }
};
struct nationi : namei {};
struct actioni : objecti {
	costi						cost;
	char						cost_per_unit;
	char						order;
	//
	static int					compare(const void* p1, const void* p2);
	bool						isplaceable() const;
	static const actioni*		getaction(ability_s id);
	int							getattack() const { return get(Attack); }
	int							getdefend() const { return get(Defend); }
	int							getraid() const { return get(Raid); }
	province_flag_s				getprovince() const;
	static unsigned				select(actioni** source, unsigned count, ability_s id);
	static void					sort(actioni** source, unsigned count);
};
struct seasoni : namei {};
struct calendari : namei {
	const seasoni*				season;
};
struct landscapei : objecti {
	char						income;
	int							getincome(stringcreator* ti) const;
};
struct provincei : objecti {
	void						add(const uniti* unit);
	void						addeconomy(int value) { seteconomy(geteconomy() + value); }
	troopi*						addinvader(uniti* type, playeri* player);
	void						addinvader(uniti** units, unsigned count, playeri* player);
	void						addsupport(const playeri* player, int value) { setsupport(player, getsupport(player) + value); }
	void						addsupportex(const playeri* player, int value, int minimal_value, int maximal_value);
	void						arrival(const playeri* player);
	bool						battle(string& sb, playeri* attacker_player, playeri* defender_player, const actioni* action, bool raid);
	void						build(uniti* unit, int wait = 1);
	static void					change_support();
	void						createwave();
	int							getdefend() const;
	int							geteconomy() const { return get(Economy); }
	heroi*						gethero(const playeri* player) const;
	playeri*					getplayer() const { return player; }
	point						getposition() const { return position; }
	int							getstrenght() const;
	int							getincome(stringcreator* ti = 0) const;
	int							getindex() const;
	void						getinfo(stringcreator& sb, bool show_landscape, const army* support_units = 0) const;
	landscapei*					getlandscape() const { return landscape; }
	int							getlevel() const { return get(Level); }
	nationi*					getnation() const { return nation; }
	provincei*					getneighbors(const playeri* player) const;
	int							getmovecost() const;
	province_flag_s				getstatus(const playeri* player) const;
	int							getsupport(const playeri* player) const;
	void						getsupport(stringcreator& sb) const;
	static void					initialize();
	void						loose(string& sb, playeri* attacker_player, playeri* defender_player, const actioni* action, bool raid, heroi* attacker_general, heroi* defender_general, int trophies);
	static bsreq				metadata[];
	static unsigned				remove_hero_present(aref<provincei*> source, const playeri* player);
	static unsigned				remove_mode(aref<provincei*> source, const playeri* player, province_flag_s state);
	void						render_neighbors(const rect& rc) const;
	void						retreat(const playeri* player);
	static unsigned				select(provincei** source, unsigned maximum);
	static unsigned				select(provincei** source, unsigned maximum, const playeri* player);
	static unsigned				select_friendly(provincei** source, unsigned maximum, const playeri* player);
	void						seteconomy(int value);
	void						setlandscape(landscapei* value) { landscape = value; }
	void						setnation(nationi* value) { nation = value; }
	void						setplayer(playeri* value) { player = value; }
	void						setsupport(const playeri* player, int value);
	void						win(string& sb, playeri* attacker_player, playeri* defender_player, const actioni* action, bool raid, heroi* attacker_general, heroi* defender_general, int trophies);
private:
	playeri*					player;
	landscapei*					landscape;
	nationi*					nation;
	point						position;
	int							support[player_max];
	provincei*					neighbors[8];
};
struct reporti {
	constexpr reporti() : hero(0), player(0), province(0), text(0), turn(0) {}
	static reporti*				add(const playeri* player, const provincei* province, const heroi* hero, const char* text);
	const heroi*				gethero() const { return hero; }
	const playeri*				getplayer() const { return player; }
	const provincei*			getprovince() const { return province; }
	const char*					get() const { return text; }
	int							getturn() const { return turn; }
	bool						is(const playeri* player) const;
	void						set(playeri* player);
private:
	const char*					text;
	int							turn;
	const heroi*				hero;
	const playeri*				player;
	const provincei*			province;
};
struct traiti : objecti {};
struct tactici : objecti {};
struct army : adat<troopi*, 32> {
	heroi*						general;
	playeri*					player;
	provincei*					province;
	const tactici*				tactic;
	bool						attack;
	bool						raid;
	constexpr army() : general(0), player(0), tactic(0), province(0), attack(false), raid(false) {}
	army(playeri* player, provincei* province, heroi* general, bool attack, bool raid);
	void						fill(const playeri* player, const provincei* province);
	int							get(ability_s id, stringcreator* sb) const;
	int							getraid() const { return get(Raid); }
	int							getstrenght(stringcreator* sb) const;
	int							getshield() const { return getcasualty(Shield); }
	int							getsword() const { return getcasualty(Sword); }
private:
	int							get(ability_s id) const;
	int							getcasualty(ability_s id) const;
};
struct choiseset : string {
	struct element {
		int						param;
		int						weight;
		const char*				text;
		const char*				getname() const { return text; }
	};
	typedef void(*tips_type)(stringcreator& sb, const element& e);
	adat<element, 8>			elements;
	constexpr explicit operator bool() const { return elements.count != 0; }
	element*					add(int param, const char* format, ...);
	element*					addv(int param, const char* format, const char* format_param);
	int							choose(bool interactive, bool cancel_button) const;
	int							choose(bool interactive, const heroi* hero, bool cancel_button, choiseset::tips_type getinfo) const;
	const element*				getrandom() const;
	int							getrandomparam() const;
	int							getweight() const;
	void						sort();
};
struct heroi : objecti {
	costi					pay;
	//
	void						cancelaction();
	void						check_leave();
	const actioni*				choose_action() const;
	bool						choose_attack(const actioni* action, const provincei* province, army& troops, costi& cost, bool raid) const;
	provincei*					choose_province(const actioni* action, bool run) const;
	const provincei*			choose_province(const actioni* action, aref<provincei*> source, province_flag_s mode) const;
	const tactici*				choose_tactic() const;
	bool						choose_troops(const actioni* action, const provincei* province, army& a1, army& a2, army& a3, int minimal_count, costi& cost) const;
	bool						choose_troops_human(const actioni* action, const provincei* province, army& a1, army& a2, army& a3, int minimal_count, costi& cost) const;
	bool						choose_troops_computer(const actioni* action, const provincei* province, army& a1, army& a2, army& a3, int minimal_count, costi& cost, int test_safety) const;
	bool						choose_units(const actioni* action, const provincei* province, unit_set& a1, unit_set& a2, costi& cost) const;
	bool						choose_units_human(const actioni* action, const provincei* province, unit_set& a1, unit_set& a2, costi& cost) const;
	bool						choose_units_computer(const actioni* action, const provincei* province, unit_set& s1, unit_set& s2, costi& cost) const;
	static void					clear_actions();
	int							getattack() const { return get(Attack); }
	const actioni*				getaction() const { return action; }
	const char*					getavatar() const { return avatar; }
	const tactici*				getbesttactic() const { return best_tactic; }
	void						getbrief(stringcreator& sb) const;
	void						getinfo(stringcreator& sb) const;
	gender_s					getgender() const { return gender; }
	int							getdefend() const { return get(Defend); }
	int							getincome() const;
	int							getloyalty() const { return get(Loyalty); }
	const landscapei*			getorigin() const { return origin; }
	playeri*					getplayer() const { return player; }
	provincei*					getprovince() const { return province; }
	int							getraid() const { return get(Raid); }
	int							getshield() const { return get(Shield); }
	void						getstate(stringcreator& sb) const;
	int							getsword() const { return get(Sword); }
	const tactici*				gettactic() const { return tactic; }
	int							getwait() const { return get(Wait); }
	int							getwound() const { return get(Wounds); }
	static void					initialize();
	bool						isallow(const actioni* action) const;
	bool						isready() const { return (get(Wait) == 0) && (get(Wounds) == 0); }
	void						make_move();
	static bsreq				metadata[];
	static void					neutral_hero_actions();
	static void					refresh_heroes();
	unsigned					remove_this(heroi** source, unsigned count) const;
	static unsigned				remove_hired(heroi** source, unsigned count);
	void						resolve();
	static unsigned				select(heroi** source, unsigned maximum_count);
	static unsigned				select(heroi** source, unsigned maximum_count, const playeri* player);
	void						setaction(const actioni* value) { action = value; }
	void						setaction(const actioni* action, provincei* province, const tactici* tactic, const costi& cost, const army& logistic, const unit_set& production);
	void						setloyalty(int value);
	void						setplayer(playeri* player);
	void						setprovince(provincei* value) { province = value; }
	void						settactic(const tactici* value) { tactic = value; }
	void						setwait(int v) { set(Wait, v); }
	void						setwound(int v) { set(Wounds, v); }
private:
	char						experience;
	gender_s					gender;
	const actioni*				action;
	const char*					avatar;
	playeri*					player;
	provincei*					province;
	const tactici*				tactic;
	const tactici*				best_tactic;
	const landscapei*			origin;
	traiti*						traits[2];
};
struct uniti : objecti {
	costi						cost;
	nationi*					nation;
	char						income;
	char						mourning;
	landscapei*					landscape[4];
	//
	static const uniti*			getfirst(const nationi* nation, const landscapei* landscape, int level);
	bool						is(const landscapei* landscape) const;
};
struct troopi {
	explicit operator bool() const { return type != 0; }
	static troopi*				add(provincei* province, const uniti* type);
	static void					arrival(const provincei* province, const playeri* player);
	void						clear();
	static int					compare(const void* p1, const void* p2);
	int							fix(stringcreator* sb, int value) const { return type->fix(sb, type->name, value); }
	int							get(ability_s id) const { return type->get(id); }
	int							getbonus(const char* id) const { return 0; }
	int							getincome() const { return type->income; }
	provincei*					gethome() const { return home; }
	provincei*					getmove() const { return move; }
	const char*					getname() const { return type->name; }
	const char*					getnameof() const { return type->nameof; }
	playeri*					getplayer() const;
	static void					getpresent(stringcreator& sb, troopi** source, unsigned count, const char* addition_text);
	provincei*					getprovince() const { return province; }
	provincei*					getprovince(const playeri* player) const;
	int							getshield() const { return get(Shield); }
	int							getsort() const { return type->get(Attack) + type->get(Defend); }
	int							getsword() const { return get(Sword); }
	void						kill(playeri* player);
	static bsreq				metadata[];
	static unsigned				remove(troopi** source, unsigned count, const provincei* province);
	static unsigned				remove_moved(troopi** source, unsigned count);
	static unsigned				remove_restricted(troopi** source, unsigned count, const provincei* province);
	static void					retreat(const provincei* province, const playeri* player);
	static unsigned				select(troopi** result, unsigned result_maximum, const provincei* province);
	static unsigned				select(troopi** result, unsigned result_maximum, const playeri* player);
	static unsigned				select_move(troopi** result, unsigned result_maximum, const provincei* province, const playeri* player);
	void						sethome(provincei* value) { home = value; }
	void						setmove(provincei* value) { move = value; }
	void						setprovince(provincei* value) { province = value; }
	static void					sort(troopi** source, unsigned count);
private:
	const uniti*				type;
	provincei*					province;
	provincei*					home;
	provincei*					move;
};
struct playeri : namei {
	explicit operator bool() const { return type != NoPlayer; }
	static void					check_heroes();
	void						check_hire();
	int							choose(const heroi* hero, choiseset& source, const char* format, ...) const;
	costi						cost;
	static int					compare_fame(const void* p1, const void* p2);
	static int					compare_hire_bet(const void* p1, const void* p2);
	void						computer_move();
	static void					create_order();
	static void					desert_units();
	static void					gain_profit();
	static unsigned				getactions(heroi** source, unsigned maximum_count, int order);
	provincei*					getbestprovince() const;
	static void					getcalendar(stringcreator& sb);
	int							getherocount() const;
	int							getincome(stringcreator* sb = 0) const;
	int							getindex() const;
	void						getinfo(stringcreator& sb) const;
	const char*					getnameof() const { return nameof; }
	int							getfriendlyprovinces() const;
	int							getsupport() const;
	int							getstrenght() const;
	static unsigned				gettroops(troopi** source, unsigned maximum_count, const provincei* province = 0, const playeri* player = 0, const playeri* player_move = 0);
	int							gettroopscount() const;
	static void					hire_heroes();
	bool						iscomputer() const { return !this || type == PlayerComputer; }
	bool						isallow(const actioni* action) const;
	static bool					isallowgame();
	void						make_move();
	static bsreq				metadata[];
	static void					playgame();
	void						post(const heroi* hero, const provincei* province, const char* text) const;
	static void					random_events();
	unsigned					remove_restricted(actioni** source, unsigned maximum) const;
	static void					resolve_actions();
	void						show_reports() const;
	static void					suggest_heroes();
private:
	player_ai_s					type;
	int							hire_gold;
};
struct buildi {
	uniti*						unit;
	provincei*					province;
	char						wait;
	explicit operator bool() const { return unit != 0; }
	static void					build_units();
	void						clear();
	static int					compare(const void* p1, const void* p2);
	static void					getpresent(stringcreator& sb, buildi** objects, unsigned count);
	static unsigned				select(buildi** source, unsigned maximum, const provincei* province);
	static void					sort(buildi** source, unsigned count);
};
struct gamei {
	const char*					map;
	actioni*					default_action;
	char						income_per_level, casualties;
	char						support_maximum, support_minimum, support_attack, support_defend;
	short						desert_base;
	char						economy_minimum, economy_maximum;
	char						loyalty_maximum, loyalty_base, loyalty_noble_modifier;
	char						hire_turns, hire_turns_range[2];
	heroi*						hire_hero;
	int							turn;
	int							year;
	//
	void						clear();
	void						initialize();
	bool						initializemap();
	bool						readmap(const char* name);
};
struct game_header {
	const char*					id;
	const char*					name;
	const char*					map;
	unsigned					players_count;
	static unsigned				select(game_header* source, unsigned maximum);
	static game_header*			choose(game_header* source, unsigned count);
};
struct guii {
	unsigned char				border, button_border;
	unsigned char				opacity, opacity_disabled, opacity_hilighted, opacity_hilighted_province;
	short						button_width, window_width, tips_width, hero_width, hero_window_width, control_border;
	short						padding;
	void						initialize();
};
struct unit_set : adat<uniti*, 32> {
	void						fill(const playeri* player, const provincei* province, const heroi* hero, const actioni* action);
	costi						getcost() const;
};
struct menui {
	typedef void(*callback)(const menui* p);
	const char*					id;
	const char*					parent;
	callback					proc;
	const char*					name;
	static const menui*			choose(const char* parent, bool cancel_button);
	static void					choose_block(const char* parent);
	static void					select(choiseset& ai, const char* parent);
};
struct effecti : objecti {
	void operator+=(const effecti& e);
	ability_s					test;
	uniti*						units[4];
	void						apply(string& sb, playeri* player, provincei* province, heroi* hero) const;
	bool						isvalid(const heroi& e) const;
};
struct eventi : objecti {
	landscapei*					landscape;
	effecti						effects[3]; // 0 - Провал, 1 и 2 - выигрыш
	char						count;
	static const eventi*		getnext();
	static void					initialize();
	bool						isvalid(const provincei& e) const;
	void						play(provincei* province, heroi* hero) const;
	static void					random(provincei* province, heroi* hero);
	unsigned					select(provincei** source, unsigned maximum) const;
};
struct statistici {
	int							provinces;
	int							gold;
	int							income;
	int							support;
	int							troops;
	statistici() { clear(); }
	void						clear();
	void						fill(const playeri* player);
};
extern namei					ability_data[];
extern adat<actioni, 32>		action_data;
extern adat<buildi>				build_data;
extern adat<eventi, 64>			event_data;
extern gamei					game;
extern guii						gui;
extern adat<heroi, hero_max>	hero_data;
extern const char*				key_requisits[];
extern const bsdata_strings		key_ranges[];
extern adat<landscapei, 32>		landscape_data;
extern msgi						msg;
extern adat<nationi, 16>		nation_data;
extern adat<playeri, player_max> player_data;
extern adat<provincei, province_max> province_data;
extern unsigned char			province_order[province_max];
extern adat<reporti, 2048>		report_data;
extern adat<tactici, 16>		tactic_data;
extern adat<troopi, 256>		troop_data;
extern adat<uniti, 64>			unit_data;