#include "collection.h"
#include "crt.h"
#include "draw_control.h"
#include "msg.h"
#include "stringcreator.h"

#pragma once

enum province_flag_s {
	AnyProvince,
	NoFriendlyProvince, FriendlyProvince, NeutralProvince,
};
enum player_ai_s : unsigned char {
	NoPlayer,
	PlayerHuman, PlayerComputer,
};

const int player_max = 8;
const int hero_max = player_max * 5;
const int province_max = player_max * 16;

bsreq action_type[];
bsreq character_type[];
bsreq landscape_type[];
bsreq msg_type[];
bsreq point_type[];
bsreq player_ai_type[];
bsreq tactic_type[];
bsreq trait_type[];
bsreq unit_type[];

struct army;
struct hero_info;
struct player_info;
struct troop_info;
struct unit_info;
struct unit_set;

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
	char						cruelty, shield;
	int							get(const char* id) const;
};
struct cost_info {
	short						gold, income;
	char						fame;
	constexpr cost_info() : gold(0), income(0), fame(0) {}
	constexpr void operator+=(const cost_info& e) { gold += e.gold; fame += e.fame; }
	constexpr void operator+=(const int value) { gold += value; }
	constexpr void operator-=(const cost_info& e) { gold -= e.gold; fame -= e.fame; }
	constexpr bool operator>(const cost_info& e) { return gold > e.gold || fame > e.fame; }
	void						clear();
	char*						get(char* result, const char* result_maximum) const;
};
struct name_info {
	const char*					id;
	const char*					name;
	const char*					text;
	constexpr name_info() : id(0), name(0), text(0) {}
	constexpr name_info(const char* id) : id(id), name(0), text(0) {}
	explicit operator bool() const { return id != 0; }
	const char*					getid() const { return id; }
	const char*					getname() const { return name; }
	static int					getnum(const void* object, const bsreq* type, const char* id);
	static int					fix(tip_info* ti, const char* name, int value);
	int							fix(tip_info* ti, int value) const { return fix(ti, name, value); }
};
struct action_info : name_info, combat_info, cost_info {
	const char*					nameact;
	char						recruit, support, rule, hire, movement;
	char						order;
	char						wait;
	//
	static int					compare(const void* p1, const void* p2);
	bool						isplaceable() const;
	province_flag_s				getprovince() const;
	static void					sort(action_info** source, unsigned count);
};
struct character_info : combat_info {
	char						diplomacy;
	char						nobility;
	char						will;
	int							get(const char* id) const;
};
struct landscape_info : name_info, combat_info, cost_info {
	int							getincome(tip_info* ti) const;
};
struct province_info : name_info {
	void						add(unit_info* unit);
	void						addeconomy(int value);
	void						addsupport(const player_info* player, int value);
	bool						battle(char* result, const char* result_max, player_info* attacker_player, player_info* defender_player, action_info* action, bool raid);
	void						build(unit_info* unit, int wait = 1);
	static void					change_support();
	void						createwave();
	int							getdefend() const;
	int							geteconomy() const { return economy; }
	hero_info*					gethero(const player_info* player) const;
	player_info*				getplayer() const { return player; }
	point						getposition() const { return position; }
	int							getincome(tip_info* ti = 0) const;
	char*						getinfo(char* result, const char* result_maximum, bool show_landscape, const army* support_units = 0) const;
	landscape_info*				getlandscape() const { return landscape; }
	int							getlevel() const { return level; }
	province_info*				getneighbors(const player_info* player) const;
	province_flag_s				getstatus(const player_info* player) const;
	int							getsupport(const player_info* player) const;
	const char*					getsupport(char* result, const char* result_maximum) const;
	static bsreq				metadata[];
	static unsigned				remove_hero_present(aref<province_info*> source, const player_info* player);
	void						render_neighbors(const rect& rc) const;
	void						retreat(const player_info* player);
	static unsigned				select(province_info** source, unsigned maximum, const player_info* player = 0, province_flag_s state = AnyProvince);
	void						setplayer(player_info* value) { player = value; }
	void						setsupport(const player_info* player, int value);
private:
	char						level;
	player_info*				player;
	landscape_info*				landscape;
	point						position;
	int							support[player_max];
	int							economy;
	province_info*				neighbors[8];
};
struct report_info {
	constexpr report_info() : hero(0), player(0), province(0), text(0), turn(0) {}
	static report_info*			add(player_info* player, province_info* province, hero_info* hero, const char* text);
	const province_info*		getprovince() const { return province; }
	const char*					get() const { return text; }
	int							getturn() const { return turn; }
	bool						is(const player_info* player) const;
	void						set(player_info* player);
private:
	hero_info * hero;
	player_info*				player;
	province_info*				province;
	const char*					text;
	int							turn;
};
struct trait_info : name_info, character_info {};
struct tactic_info : name_info, combat_info {
	constexpr tactic_info(const char* id) : name_info(id), combat_info() {}
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
	int							getstrenght(tip_info* ti, bool include_number = true) const;
};
struct hero_info : name_info {
	cost_info					pay;
	trait_info*					traits[2];
	//
	void						cancelaction();
	int							get(const char* id) const;
	int							getattack() const { return get("attack"); }
	const action_info*			getaction() const { return action; }
	const char*					getavatar() const { return avatar; }
	const tactic_info*			getbesttactic() const { return best_tactic; }
	int							getbonus(const char* id) const;
	int							getcruelty() const { return get("cruelty"); }
	int							getdefend() const { return get("defend"); }
	int							getdiplomacy() const { return get("diplomacy"); }
	int							getex(const char* id) const { return get(id) + getbonus(id); }
	int							getincome() const;
	int							getnobility() const { return get("nobility"); }
	player_info*				getplayer() const { return player; }
	province_info*				getprovince() const { return province; }
	int							getraid() const { return get("raid"); }
	const tactic_info*			gettactic() const { return tactic; }
	int							getwait() const { return wait; }
	static bsreq				metadata[];
	bool						isallow(const action_info* action) const;
	bool						isready() const { return (wait == 0); }
	static void					refresh_heroes();
	unsigned					remove_this(hero_info** source, unsigned count) const;
	void						resolve();
	unsigned					select(action_info** source, unsigned maximum) const;
	static unsigned				select(hero_info** source, unsigned maximum_count, const province_info* province = 0, const player_info* player = 0);
	static unsigned				select(hero_info** source, unsigned maximum_count, const player_info* player, const action_info* action, const hero_info* exclude = 0);
	void						setaction(action_info* value) { action = value; }
	void						setaction(action_info* action, province_info* province, const cost_info& cost, const army& logistic, const unit_set& production);
	void						setprovince(province_info* value) { province = value; }
	void						setwait(int v) { wait = v; }
private:
	action_info * action;
	const char*					avatar;
	const char*					nameof;
	player_info*				player;
	province_info*				province;
	const tactic_info*			tactic;
	const tactic_info*			best_tactic;
	char						wait;
};
struct unit_info : name_info, combat_info, cost_info {
	const char*					nameof;
	char						level;
	char						recruit_count, recruit_time;
	landscape_info*				recruit_landscape;
	static int					compare(const void* p1, const void* p2);
	int							get(const char* id) const;
};
struct troop_info {
	explicit operator bool() const { return type != 0; }
	static troop_info*			add(province_info* province, unit_info* type);
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
	static const char*			getpresent(char* result, const char* result_maximum, troop_info** source, unsigned count, const char* addition_text);
	province_info*				getprovince() const { return province; }
	province_info*				getprovince(const player_info* player) const;
	int							getsort() const { return type->attack + type->defend; }
	void						kill(player_info* player);
	static bsreq				metadata[];
	static unsigned				remove_moved(troop_info** source, unsigned count);
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
struct player_info : name_info, cost_info {
	explicit operator bool() const { return type != NoPlayer; }
	operator cost_info&() { return *static_cast<cost_info*>(this); }
	void						add(province_info* province, hero_info* hero, const char* text);
	static void					gain_profit();
	static unsigned				getactions(hero_info** source, unsigned maximum_count, int order);
	province_info*				getbestprovince() const;
	cost_info					getcost() const { return *static_cast<const cost_info*>(this); }
	int							getincome(tip_info* ti = 0) const;
	const char*					getnameof() const { return nameof; }
	int							getsupport(tip_info* ti = 0) const;
	static unsigned				gettroops(troop_info** source, unsigned maximum_count, const province_info* province = 0, const player_info* player = 0, const player_info* player_move = 0);
	void						makemove();
	static bsreq				metadata[];
	static void					playgame();
	static void					resolve_actions();
private:
	const char*					nameof;
	player_ai_s					type;
};
struct build_info {
	unit_info*					unit;
	province_info*				province;
	char						wait;
	explicit operator bool() const { return unit != 0; }
	static void					build_units();
	void						clear();
	static int					compare(const void* p1, const void* p2);
	static const char*			getpresent(char* result, const char* result_maximum, build_info** objects, unsigned count);
	static unsigned				select(build_info** source, unsigned maximum, const province_info* province);
	static void					sort(build_info** source, unsigned count);
};
struct game_info {
	const char*					map;
	action_info*				default_action;
	char						income_per_level, casualties;
	char						support_maximum, support_minimum, support_attack, support_defend, support_change;
	char						economy_minimum, economy_maximum;
	int							turn;
	void						clear();
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
namespace draw {
void							addaccept(char* result, const char* result_max);
void							addbutton(char* result, const char* result_max, const char* name);
void							avatar(int x, int y, const char* id);
int								button(int x, int y, int width, const char* label, const runable& e, unsigned key = 0);
int								buttonw(int x, int y, int width, const char* label, const runable& e, unsigned key = 0, const char* tips = 0);
bool							conquer(const player_info* player, hero_info* hero, const action_info* action, const province_info* province, army& s1, army& s2, const army& a3);
action_info*					getaction(player_info* player, hero_info* hero);
color							getcolor(province_flag_s id);
province_info*					getprovince(player_info* player, hero_info* hero, action_info* action, aref<province_info*> selection, color selection_color);
areas							hilite(rect rc);
bool							initializemap();
bool							recruit(const player_info* player, hero_info* hero, const action_info* action, const province_info* province, unit_set& s1, unit_set& s2, cost_info& pay);
void							report(const char* format);
areas							window(rect rc, bool disabled = false, bool hilight = false, int border = 0);
int								window(int x, int y, int width, const char* string);
int								windowb(int x, int y, int width, const char* string, const runable& e, int border = 0, unsigned key = 0, const char* tips = 0);
}
extern adat<action_info, 32>	action_data;
extern adat<build_info, 256>	build_data;
extern game_info				game;
extern gui_info					gui;
extern adat<hero_info, hero_max> hero_data;
extern msg_info					msg;
extern adat<landscape_info, 32> landscape_data;
extern adat<player_info, player_max> player_data;
extern adat<province_info, province_max> province_data;
extern unsigned char			province_order[province_max];
extern adat<report_info, 2048>	report_data;
extern tactic_info				tactic_data[];
extern adat<troop_info, 256>	troop_data;
extern adat<unit_info, 64>		unit_data;