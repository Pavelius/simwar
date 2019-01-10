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

bsreq action_type[];
bsreq calendar_type[];
bsreq character_type[];
bsreq cost_type[];
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
	bool operator>(const cost_info& e) const { return gold > e.gold || fame > e.fame; }
	void						clear();
	char*						get(char* result, const char* result_maximum) const;
};
struct string : stringcreator, stringbuilder {
	gender_s					gender;
	cost_info					cost;
	string(gender_s gender = Male) : stringbuilder(*this, buffer, buffer + sizeof(buffer) / sizeof(buffer[0])), gender(gender) { buffer[0] = 0; }
	void						parseidentifier(char* result, const char* result_max, const char* identifier) override;
private:
	char						buffer[8192];
};
struct tip_info {
	char*						result;
	const char*					result_max;
	const char*					text;
	const char*					separator;
	constexpr tip_info(char* result, const char* result_max) :result(result), result_max(result_max), text("[%3%+1i]\t%2"), separator("\n") { result[0] = 0; }
	template<unsigned N> constexpr tip_info(char(&result)[N]) : tip_info(result, result + sizeof(result)) {}
};
struct combat_info {
	char						attack, defend, raid;
	char						sword, shield;
	int							get(const char* id) const;
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
	const char*					getnameof() const { return nameof; }
	static int					getnum(const void* object, const bsreq* type, const char* id);
	static int					fix(tip_info* ti, const char* name, int value);
	int							fix(tip_info* ti, int value) const { return fix(ti, name, value); }
};
struct nation_info : name_info {};
struct action_info : name_info, combat_info {
	cost_info					cost, trophies;
	char						cost_per_unit;
	char						recruit, support, economy, movement;
	char						order;
	char						good;
	char						wait;
	//
	static int					compare(const void* p1, const void* p2);
	bool						isplaceable() const;
	static const action_info*	getaction(char attack, char defend, char raid);
	province_flag_s				getprovince() const;
	static unsigned				select(action_info** source, unsigned count, char attack = 0, char defend = 0, char raid = 0);
	static void					sort(action_info** source, unsigned count);
};
struct character_info : combat_info {
	char						diplomacy;
	char						good;
	char						nobility;
	int							get(const char* id) const;
};
struct season_info : name_info {

};
struct calendar_info : name_info {
	const season_info*			season;
};
struct landscape_info : name_info, combat_info {
	char						income;
	int							getincome(tip_info* ti) const;
};
struct province_info : name_info {
	void						add(unit_info* unit);
	void						addeconomy(int value) { seteconomy(geteconomy() + value); }
	void						addsupport(const player_info* player, int value) { setsupport(player, getsupport(player) + value); }
	void						addsupportex(const player_info* player, int value, int minimal_value, int maximal_value);
	void						arrival(const player_info* player);
	bool						battle(string& sb, player_info* attacker_player, player_info* defender_player, const action_info* action, bool raid);
	void						build(unit_info* unit, int wait = 1);
	static void					change_support();
	void						createwave();
	int							getdefend() const;
	int							geteconomy() const { return economy; }
	hero_info*					gethero(const player_info* player) const;
	player_info*				getplayer() const { return player; }
	point						getposition() const { return position; }
	int							getincome(tip_info* ti = 0) const;
	int							getindex() const;
	void						getinfo(stringbuilder& sb, bool show_landscape, const army* support_units = 0) const;
	landscape_info*				getlandscape() const { return landscape; }
	int							getlevel() const { return level; }
	nation_info*				getnation() const { return nation; }
	province_info*				getneighbors(const player_info* player) const;
	int							getmovecost() const;
	province_flag_s				getstatus(const player_info* player) const;
	int							getsupport(const player_info* player) const;
	void						getsupport(stringbuilder& sb) const;
	static bsreq				metadata[];
	static unsigned				remove_hero_present(aref<province_info*> source, const player_info* player);
	void						render_neighbors(const rect& rc) const;
	void						retreat(const player_info* player);
	static unsigned				select(province_info** source, unsigned maximum, const player_info* player = 0, province_flag_s state = AnyProvince);
	static unsigned				select_visible(province_info** source, unsigned maximum, const player_info* player);
	void						seteconomy(int value);
	void						setplayer(player_info* value) { player = value; }
	void						setsupport(const player_info* player, int value);
private:
	char						level;
	player_info*				player;
	landscape_info*				landscape;
	nation_info*				nation;
	point						position;
	int							support[player_max];
	int							economy;
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
struct trait_info : name_info, character_info {};
struct tactic_info : name_info, combat_info {
};
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
	int							get(const char* id, tip_info* ti, bool include_number = true) const;
	int							getraid() const;
	int							getstrenght(tip_info* ti, bool include_number = true) const;
	int							getshield() const;
	int							getsword() const;
};
struct answer_info {
	struct element {
		int						param;
		const char*				text;
		const char*				getname() const { return text; }
	};
	typedef void(*tips_type)(stringbuilder& sb, const element& e);
	adat<element, 8>			elements;
	answer_info() { p = buffer; buffer[0] = 0; }
	void						add(int param, const char* format, ...);
	void						addv(int param, const char* format, const char* format_param);
	int							choose(const hero_info* hero, bool cancel_button = true, answer_info::tips_type getinfo = 0) const;
	void						sort();
private:
	char*						p;
	char						buffer[8196];
};
struct hero_info : name_info {
	cost_info					pay;
	//
	void						cancelaction();
	void						check_leave();
	const action_info*			choose_action() const;
	const tactic_info*			choose_tactic() const;
	int							get(const char* id) const;
	int							getattack() const { return get("attack"); }
	const action_info*			getaction() const { return action; }
	const char*					getavatar() const { return avatar; }
	const tactic_info*			getbesttactic() const { return best_tactic; }
	int							getbonus(const char* id) const;
	void						getbrief(stringbuilder& sb) const;
	void						getinfo(stringbuilder& sb) const;
	gender_s					getgender() const { return gender; }
	int							getgood() const { return get("good"); }
	int							getdefend() const { return get("defend"); }
	int							getdiplomacy() const { return get("diplomacy"); }
	int							getex(const char* id) const { return get(id) + getbonus(id); }
	int							gethirecost(const player_info* player) const;
	int							getincome() const;
	int							getloyalty() const { return loyalty; }
	int							getnobility() const { return get("nobility"); }
	player_info*				getplayer() const { return player; }
	province_info*				getprovince() const { return province; }
	int							getraid() const { return get("raid"); }
	int							getshield() const { return get("shield"); }
	void						getstate(stringbuilder& sb) const;
	int							getsword() const { return get("sword"); }
	const tactic_info*			gettactic() const { return tactic; }
	int							getwait() const { return wait; }
	int							getwound() const { return wound; }
	static bsreq				metadata[];
	static void					initialize();
	bool						isallow(const action_info* action) const;
	bool						isready() const { return (wait == 0) && (wound == 0); }
	static void					neutral_hero_actions();
	static void					refresh_heroes();
	unsigned					remove_this(hero_info** source, unsigned count) const;
	static unsigned				remove_hired(hero_info** source, unsigned count);
	void						resolve();
	static unsigned				select(hero_info** source, unsigned maximum_count, const player_info* player);
	void						setaction(const action_info* value) { action = value; }
	void						setaction(const action_info* action, province_info* province, const tactic_info* tactic, const cost_info& cost, const army& logistic, const unit_set& production);
	void						setloyalty(int value) { loyalty = value; }
	void						setplayer(player_info* player);
	void						setprovince(province_info* value) { province = value; }
	void						settactic(const tactic_info* value) { tactic = value; }
	void						setwait(int v) { wait = v; }
	void						setwound(int v) { wound = v; }
private:
	char						wait, wound, loyalty;
	gender_s					gender;
	const action_info*			action;
	const char*					avatar;
	player_info*				player;
	province_info*				province;
	const tactic_info*			tactic;
	const tactic_info*			best_tactic;
	trait_info*					traits[2];
};
struct unit_info : name_info, combat_info {
	cost_info					cost;
	nation_info*				nation;
	char						recruit_count, recruit_time;
	char						level;
	char						income;
	landscape_info*				landscape[4];
	//
	int							get(const char* id) const;
	bool						is(const landscape_info* landscape) const;
};
struct troop_info {
	explicit operator bool() const { return type != 0; }
	static troop_info*			add(province_info* province, unit_info* type);
	static void					arrival(const province_info* province, const player_info* player);
	void						clear();
	static int					compare(const void* p1, const void* p2);
	int							fix(tip_info* ti, int value) const { return type->fix(ti, type->name, value); }
	int							get(const char* id) const { return type->get(id); }
	int							getbonus(const char* id) const { return 0; }
	int							getincome() const { return type->income; }
	province_info*				getmove() const { return move; }
	const char*					getname() const { return type->name; }
	const char*					getnameof() const { return type->nameof; }
	player_info*				getplayer() const { return province->getplayer(); }
	static void					getpresent(stringbuilder& sb, troop_info** source, unsigned count, const char* addition_text);
	province_info*				getprovince() const { return province; }
	province_info*				getprovince(const player_info* player) const;
	int							getshield() const { return get("shield"); }
	int							getsort() const { return type->attack + type->defend; }
	int							getsword() const { return get("sword"); }
	void						kill(player_info* player);
	static bsreq				metadata[];
	static unsigned				remove(troop_info** source, unsigned count, const province_info* province);
	static unsigned				remove_moved(troop_info** source, unsigned count);
	static unsigned				remove_restricted(troop_info** source, unsigned count, const province_info* province);
	static void					retreat(const province_info* province, const player_info* player);
	static unsigned				select(troop_info** result, unsigned result_maximum, const province_info* province);
	static unsigned				select_move(troop_info** result, unsigned result_maximum, const province_info* province, const player_info* player);
	void						setmove(province_info* value) { move = value; }
	void						setprovince(province_info* value) { province = value; }
	static void					sort(troop_info** source, unsigned count);
private:
	unit_info * type;
	province_info*				province;
	province_info*				move;
};
struct player_info : name_info {
	explicit operator bool() const { return type != NoPlayer; }
	static void					check_heroes();
	void						check_hire();
	int							choose(const hero_info* hero, answer_info& source, const char* format, ...) const;
	cost_info					cost;
	static int					compare_hire_bet(const void* p1, const void* p2);
	static void					gain_profit();
	static unsigned				getactions(hero_info** source, unsigned maximum_count, int order);
	province_info*				getbestprovince() const;
	static void					getcalendar(stringbuilder& sb);
	int							getherocount() const;
	int							getincome(tip_info* ti = 0) const;
	int							getindex() const;
	const char*					getnameof() const { return nameof; }
	int							getsupport(tip_info* ti = 0) const;
	static unsigned				gettroops(troop_info** source, unsigned maximum_count, const province_info* province = 0, const player_info* player = 0, const player_info* player_move = 0);
	static void					hire_heroes();
	bool						isallow(const action_info* action) const;
	static bool					isallowgame();
	void						make_move();
	static bsreq				metadata[];
	static void					playgame();
	void						post(const hero_info* hero, const province_info* province, const char* text) const;
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
	static void					getpresent(stringbuilder& sb, build_info** objects, unsigned count);
	static unsigned				select(build_info** source, unsigned maximum, const province_info* province);
	static void					sort(build_info** source, unsigned count);
};
struct game_info {
	const char*					map;
	action_info*				default_action;
	char						income_per_level, casualties;
	char						support_maximum, support_minimum, support_attack, support_defend, support_change;
	char						economy_minimum, economy_maximum;
	char						loyalty_maximum, loyalty_base, loyalty_noble_modifier;
	char						hire_cost;
	char						hire_turns, hire_turns_range[2];
	hero_info*					hire_hero;
	int							turn;
	int							year;
	//
	void						clear();
	int							getyear() const;
	void						initialize();
	bool						initializemap();
	bool						read(const char* name);
};
struct gui_info {
	unsigned char				border, button_border;
	unsigned char				opacity, opacity_disabled, opacity_hilighted, opacity_hilighted_province;
	short						button_width, window_width, tips_width, hero_width, hero_window_width, control_border;
	short						padding;
};
struct unit_set : adat<unit_info*, 32> {
	void						fill(const player_info* player, const province_info* province, const hero_info* hero, const action_info* action);
	cost_info					getcost() const;
};
extern adat<action_info, 32>	action_data;
extern adat<build_info, 256>	build_data;
extern game_info				game;
extern gui_info					gui;
extern adat<hero_info, hero_max> hero_data;
extern const char*				key_requisits[];
extern adat<landscape_info, 32> landscape_data;
extern msg_info					msg;
extern adat<player_info, player_max> player_data;
extern adat<province_info, province_max> province_data;
extern unsigned char			province_order[province_max];
extern adat<report_info, 2048>	report_data;
extern adat<tactic_info, 16>	tactic_data;
extern adat<troop_info, 256>	troop_data;
extern adat<unit_info, 64>		unit_data;