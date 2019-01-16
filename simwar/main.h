#include "bsdata.h"
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
struct hero_info;
struct player_info;
struct province_info;
struct troop_info;
struct unit_info;
struct unit_set;

struct cost_info {
	short						gold, fame;
	constexpr cost_info() : gold(0), fame(0) {}
	constexpr explicit operator bool() const { return gold || fame; }
	void operator+=(const cost_info& e) { gold += e.gold; fame += e.fame; }
	void operator+=(const int value) { gold += value; }
	void operator-=(const cost_info& e) { gold -= e.gold; fame -= e.fame; }
	bool operator>(const cost_info& e) const { return gold > e.gold; }
	void						clear();
	void						getinfo(stringcreator& sb) const;
};
struct string : stringcreator {
	string();
	void						addidentifier(const char* identifier) override;
	void						addcost();
	void						addplayer();
	void						addplayerincome();
	void						addstrenght();
	static bsreq				metadata[];
	void						set(const hero_info* value) { hero = value; }
	void						set(const player_info* value) { player = value; }
	void						set(const province_info* value) { province = value; }
	void						set(const cost_info& value) { cost = value; }
private:
	cost_info					cost;
	const struct army*			army;
	const player_info*			player;
	const hero_info*			hero;
	const province_info*		province;
	char						buffer[8192];
};
struct name_info {
	const char*					id;
	const char*					name;
	const char*					nameof;
	const char*					text;
	constexpr name_info() : id(0), name(0), nameof(), text(0) {}
	constexpr name_info(const char* id) : id(id), name(0), nameof(), text(0) {}
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
struct object_info : name_info {
	char						ability[LastAbility + 1];
	int							get(ability_s id) const { return ability[id]; }
	static bool					ismatch(int v1, int v2);
	bool						isvalid(const hero_info& e) const;
	bool						isvalid(const player_info& e) const;
	bool						isvalid(const province_info& e) const;
	void						set(ability_s id, char value) { ability[id] = value; }
};
struct nation_info : name_info {};
struct action_info : object_info {
	cost_info					cost;
	char						cost_per_unit;
	char						order;
	//
	static int					compare(const void* p1, const void* p2);
	bool						isplaceable() const;
	static const action_info*	getaction(ability_s id);
	int							getattack() const { return get(Attack); }
	int							getdefend() const { return get(Defend); }
	int							getraid() const { return get(Raid); }
	province_flag_s				getprovince() const;
	static unsigned				select(action_info** source, unsigned count, ability_s id);
	static void					sort(action_info** source, unsigned count);
};
struct season_info : name_info {};
struct calendar_info : name_info {
	const season_info*			season;
};
struct landscape_info : object_info {
	char						income;
	int							getincome(stringcreator* ti) const;
};
struct province_info : object_info {
	void						add(const unit_info* unit);
	void						addeconomy(int value) { seteconomy(geteconomy() + value); }
	void						addsupport(const player_info* player, int value) { setsupport(player, getsupport(player) + value); }
	void						addsupportex(const player_info* player, int value, int minimal_value, int maximal_value);
	void						arrival(const player_info* player);
	bool						battle(string& sb, player_info* attacker_player, player_info* defender_player, const action_info* action, bool raid);
	void						build(unit_info* unit, int wait = 1);
	static void					change_support();
	void						createwave();
	int							getdefend() const;
	int							geteconomy() const { return get(Economy); }
	hero_info*					gethero(const player_info* player) const;
	player_info*				getplayer() const { return player; }
	point						getposition() const { return position; }
	int							getincome(stringcreator* ti = 0) const;
	int							getindex() const;
	void						getinfo(stringcreator& sb, bool show_landscape, const army* support_units = 0) const;
	landscape_info*				getlandscape() const { return landscape; }
	int							getlevel() const { return get(Level); }
	nation_info*				getnation() const { return nation; }
	province_info*				getneighbors(const player_info* player) const;
	int							getmovecost() const;
	province_flag_s				getstatus(const player_info* player) const;
	int							getsupport(const player_info* player) const;
	void						getsupport(stringcreator& sb) const;
	static void					initialize();
	static bsreq				metadata[];
	static unsigned				remove_hero_present(aref<province_info*> source, const player_info* player);
	static unsigned				remove_mode(aref<province_info*> source, const player_info* player, province_flag_s state);
	void						render_neighbors(const rect& rc) const;
	void						retreat(const player_info* player);
	static unsigned				select(province_info** source, unsigned maximum);
	static unsigned				select(province_info** source, unsigned maximum, const player_info* player);
	static unsigned				select_friendly(province_info** source, unsigned maximum, const player_info* player);
	void						seteconomy(int value);
	void						setlandscape(landscape_info* value) { landscape = value; }
	void						setnation(nation_info* value) { nation = value; }
	void						setplayer(player_info* value) { player = value; }
	void						setsupport(const player_info* player, int value);
private:
	player_info*				player;
	landscape_info*				landscape;
	nation_info*				nation;
	point						position;
	int							support[player_max];
	province_info*				neighbors[8];
};
struct report_info {
	constexpr report_info() : hero(0), player(0), province(0), text(0), turn(0) {}
	static report_info*			add(const player_info* player, const province_info* province, const hero_info* hero, const char* text);
	const hero_info*			gethero() const { return hero; }
	const player_info*			getplayer() const { return player; }
	const province_info*		getprovince() const { return province; }
	const char*					get() const { return text; }
	int							getturn() const { return turn; }
	bool						is(const player_info* player) const;
	void						set(player_info* player);
private:
	const char*					text;
	int							turn;
	const hero_info*			hero;
	const player_info*			player;
	const province_info*		province;
};
struct trait_info : object_info {};
struct tactic_info : object_info {};
struct army : adat<troop_info*, 32> {
	hero_info*					general;
	player_info*				player;
	province_info*				province;
	const tactic_info*			tactic;
	bool						attack;
	bool						raid;
	constexpr army() : general(0), player(0), tactic(0), province(0), attack(false), raid(false) {}
	army(player_info* player, province_info* province, hero_info* general, bool attack, bool raid);
	void						fill(const player_info* player, const province_info* province);
	int							get(ability_s id, stringcreator* sb) const;
	int							getraid() const { return get(Raid); }
	int							getstrenght(stringcreator* sb) const;
	int							getshield() const { return getcasualty(Shield); }
	int							getsword() const { return getcasualty(Sword); }
private:
	int							get(ability_s id) const;
	int							getcasualty(ability_s id) const;
};
struct answer_info : string {
	struct element {
		int						param;
		const char*				text;
		const char*				getname() const { return text; }
	};
	typedef void(*tips_type)(stringcreator& sb, const element& e);
	adat<element, 8>			elements;
	constexpr explicit operator bool() const { return elements.count != 0; }
	void						add(int param, const char* format, ...);
	void						addv(int param, const char* format, const char* format_param);
	int							choose(bool cancel_button = false) const;
	int							choose(const hero_info* hero, bool cancel_button = true, answer_info::tips_type getinfo = 0) const;
	void						sort();
};
struct hero_info : object_info {
	cost_info					pay;
	//
	void						cancelaction();
	void						check_leave();
	const action_info*			choose_action() const;
	const province_info*		choose_province(const action_info* action, aref<province_info*> source, province_flag_s mode) const;
	const tactic_info*			choose_tactic() const;
	bool						choose_troops(const action_info* action, const province_info* province, army& a1, army& a2, army& a3, int minimal_count, cost_info& cost) const;
	bool						choose_units(const action_info* action, const province_info* province, unit_set& a1, unit_set& a2, cost_info& cost) const;
	int							getattack() const { return get(Attack); }
	const action_info*			getaction() const { return action; }
	const char*					getavatar() const { return avatar; }
	const tactic_info*			getbesttactic() const { return best_tactic; }
	void						getbrief(stringcreator& sb) const;
	void						getinfo(stringcreator& sb) const;
	gender_s					getgender() const { return gender; }
	int							getdefend() const { return get(Defend); }
	int							getincome() const;
	int							getloyalty() const { return get(Loyalty); }
	const landscape_info*		getorigin() const { return origin; }
	player_info*				getplayer() const { return player; }
	province_info*				getprovince() const { return province; }
	int							getraid() const { return get(Raid); }
	int							getshield() const { return get(Shield); }
	void						getstate(stringcreator& sb) const;
	int							getsword() const { return get(Sword); }
	const tactic_info*			gettactic() const { return tactic; }
	int							getwait() const { return get(Wait); }
	int							getwound() const { return get(Wounds); }
	static void					initialize();
	bool						isallow(const action_info* action) const;
	bool						isready() const { return (get(Wait) == 0) && (get(Wounds) == 0); }
	void						make_move();
	static bsreq				metadata[];
	static void					neutral_hero_actions();
	static void					refresh_heroes();
	unsigned					remove_this(hero_info** source, unsigned count) const;
	static unsigned				remove_hired(hero_info** source, unsigned count);
	void						resolve();
	static unsigned				select(hero_info** source, unsigned maximum_count);
	static unsigned				select(hero_info** source, unsigned maximum_count, const player_info* player);
	void						setaction(const action_info* value) { action = value; }
	void						setaction(const action_info* action, province_info* province, const tactic_info* tactic, const cost_info& cost, const army& logistic, const unit_set& production);
	void						setloyalty(int value);
	void						setplayer(player_info* player);
	void						setprovince(province_info* value) { province = value; }
	void						settactic(const tactic_info* value) { tactic = value; }
	void						setwait(int v) { set(Wait, v); }
	void						setwound(int v) { set(Wounds, v); }
private:
	char						experience;
	gender_s					gender;
	const action_info*			action;
	const char*					avatar;
	player_info*				player;
	province_info*				province;
	const tactic_info*			tactic;
	const tactic_info*			best_tactic;
	const landscape_info*		origin;
	trait_info*					traits[2];
};
struct unit_info : object_info {
	cost_info					cost;
	nation_info*				nation;
	char						income;
	char						mourning;
	landscape_info*				landscape[4];
	//
	static const unit_info*		getfirst(const nation_info* nation, const landscape_info* landscape, int level);
	bool						is(const landscape_info* landscape) const;
};
struct troop_info {
	explicit operator bool() const { return type != 0; }
	static troop_info*			add(province_info* province, const unit_info* type);
	static void					arrival(const province_info* province, const player_info* player);
	void						clear();
	static int					compare(const void* p1, const void* p2);
	int							fix(stringcreator* sb, int value) const { return type->fix(sb, type->name, value); }
	int							get(ability_s id) const { return type->get(id); }
	int							getbonus(const char* id) const { return 0; }
	int							getincome() const { return type->income; }
	province_info*				gethome() const { return home; }
	province_info*				getmove() const { return move; }
	const char*					getname() const { return type->name; }
	const char*					getnameof() const { return type->nameof; }
	player_info*				getplayer() const { return province->getplayer(); }
	static void					getpresent(stringcreator& sb, troop_info** source, unsigned count, const char* addition_text);
	province_info*				getprovince() const { return province; }
	province_info*				getprovince(const player_info* player) const;
	int							getshield() const { return get(Shield); }
	int							getsort() const { return type->get(Attack) + type->get(Defend); }
	int							getsword() const { return get(Sword); }
	void						kill(player_info* player);
	static bsreq				metadata[];
	static unsigned				remove(troop_info** source, unsigned count, const province_info* province);
	static unsigned				remove_moved(troop_info** source, unsigned count);
	static unsigned				remove_restricted(troop_info** source, unsigned count, const province_info* province);
	static void					retreat(const province_info* province, const player_info* player);
	static unsigned				select(troop_info** result, unsigned result_maximum, const province_info* province);
	static unsigned				select(troop_info** result, unsigned result_maximum, const player_info* player);
	static unsigned				select_move(troop_info** result, unsigned result_maximum, const province_info* province, const player_info* player);
	void						sethome(province_info* value) { home = value; }
	void						setmove(province_info* value) { move = value; }
	void						setprovince(province_info* value) { province = value; }
	static void					sort(troop_info** source, unsigned count);
private:
	const unit_info*			type;
	province_info*				province;
	province_info*				home;
	province_info*				move;
};
struct player_info : name_info {
	explicit operator bool() const { return type != NoPlayer; }
	static void					check_heroes();
	void						check_hire();
	int							choose(const hero_info* hero, answer_info& source, const char* format, ...) const;
	cost_info					cost;
	static int					compare_fame(const void* p1, const void* p2);
	static int					compare_hire_bet(const void* p1, const void* p2);
	void						computer_move();
	static void					create_order();
	static void					desert_units();
	static void					gain_profit();
	static unsigned				getactions(hero_info** source, unsigned maximum_count, int order);
	province_info*				getbestprovince() const;
	static void					getcalendar(stringcreator& sb);
	int							getherocount() const;
	int							getincome(stringcreator* sb = 0) const;
	int							getindex() const;
	void						getinfo(stringcreator& sb) const;
	const char*					getnameof() const { return nameof; }
	int							getsupport(stringcreator* ti = 0) const;
	static unsigned				gettroops(troop_info** source, unsigned maximum_count, const province_info* province = 0, const player_info* player = 0, const player_info* player_move = 0);
	static void					hire_heroes();
	bool						iscomputer() const { return !this || type == PlayerComputer; }
	bool						isallow(const action_info* action) const;
	static bool					isallowgame();
	void						make_move();
	static bsreq				metadata[];
	static void					playgame();
	void						post(const hero_info* hero, const province_info* province, const char* text) const;
	static void					random_events();
	unsigned					remove_restricted(action_info** source, unsigned maximum) const;
	static void					resolve_actions();
	void						show_reports() const;
	static void					suggest_heroes();
private:
	player_ai_s					type;
	int							hire_gold;
};
struct build_info {
	unit_info*					unit;
	province_info*				province;
	char						wait;
	explicit operator bool() const { return unit != 0; }
	static void					build_units();
	void						clear();
	static int					compare(const void* p1, const void* p2);
	static void					getpresent(stringcreator& sb, build_info** objects, unsigned count);
	static unsigned				select(build_info** source, unsigned maximum, const province_info* province);
	static void					sort(build_info** source, unsigned count);
};
struct game_info {
	const char*					map;
	action_info*				default_action;
	char						income_per_level, casualties;
	char						support_maximum, support_minimum, support_attack, support_defend;
	short						desert_base;
	char						economy_minimum, economy_maximum;
	char						loyalty_maximum, loyalty_base, loyalty_noble_modifier;
	char						hire_turns, hire_turns_range[2];
	hero_info*					hire_hero;
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
struct gui_info {
	unsigned char				border, button_border;
	unsigned char				opacity, opacity_disabled, opacity_hilighted, opacity_hilighted_province;
	short						button_width, window_width, tips_width, hero_width, hero_window_width, control_border;
	short						padding;
	void						initialize();
};
struct unit_set : adat<unit_info*, 32> {
	void						fill(const player_info* player, const province_info* province, const hero_info* hero, const action_info* action);
	cost_info					getcost() const;
};
struct menu_info {
	typedef void(*callback)(const menu_info* p);
	const char*					id;
	const char*					parent;
	callback					proc;
	const char*					name;
	static const menu_info*		choose(const char* parent, bool cancel_button);
	static void					choose_block(const char* parent);
	static void					select(answer_info& ai, const char* parent);
};
struct effect_info : object_info {
	void operator+=(const effect_info& e);
	ability_s					test;
	unit_info*					units[4];
	void						apply(hero_info* hero) const;
	void						apply(player_info* player) const;
	void						apply(province_info* province) const;
	bool						isvalid(const hero_info& e) const;
};
struct event_info : object_info {
	landscape_info*				landscape;
	effect_info					effects[3]; // 0 - Провал, 1 и 2 - выигрыш
	char						count;
	static const event_info*	getnext();
	static void					initialize();
	bool						isvalid(const province_info& e) const;
	void						play(province_info* province, hero_info* hero) const;
	static void					random(province_info* province, hero_info* hero);
	unsigned					select(province_info** source, unsigned maximum) const;
};
extern name_info				ability_data[];
extern adat<action_info, 32>	action_data;
extern adat<build_info, 256>	build_data;
extern adat<event_info, 64>		event_data;
extern game_info				game;
extern gui_info					gui;
extern adat<hero_info, hero_max> hero_data;
extern const char*				key_requisits[];
extern const bsdata_strings		key_ranges[];
extern adat<landscape_info, 32> landscape_data;
extern msg_info					msg;
extern adat<nation_info, 16>	nation_data;
extern adat<player_info, player_max> player_data;
extern adat<province_info, province_max> province_data;
extern unsigned char			province_order[province_max];
extern adat<report_info, 2048>	report_data;
extern adat<tactic_info, 16>	tactic_data;
extern adat<troop_info, 256>	troop_data;
extern adat<unit_info, 64>		unit_data;