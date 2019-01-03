#include "collection.h"
#include "crt.h"
#include "draw_control.h"
#include "stringcreator.h"

#pragma once

enum province_flag_s {
	AnyProvince,
	NoFriendlyProvince, FriendlyProvince, NeutralProvince,
};

const int player_max = 8;
const int province_max = 128;

bsreq action_type[];
bsreq character_type[];
bsreq landscape_type[];
bsreq msg_type[];
bsreq point_type[];
bsreq tactic_type[];
bsreq trait_type[];
bsreq unit_type[];

extern bsdata tactic_manager;

struct hero_info;
struct player_info;
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
struct prof_info {
	char						cost;
	char						income;
	char						support;
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
	static int					fix(tip_info* ti, const char* name, int value);
	int							fix(tip_info* ti, int value) const { return fix(ti, name, value); }
};
struct action_info : name_info, combat_info, prof_info {
	const char*					nameact;
	char						recruit, support, profit, placeable;
	char						order;
	province_flag_s				getprovince() const;
};
struct character_info : combat_info  {
	char						diplomacy;
	char						nobility;
	char						will;
	int							get(const char* id) const;
};
struct landscape_info : name_info, combat_info, prof_info {
	int							getincome(tip_info* ti) const;
};
struct province_info : name_info {
	bool						battle(char* result, const char* result_max, player_info* attacker_player, player_info* defender_player, bool raid);
	void						createwave();
	hero_info*					gethero(const player_info* player) const;
	player_info*				getplayer() const { return player; }
	point						getposition() const { return position; }
	int							getincome(tip_info* ti = 0) const;
	province_flag_s				getstatus(const player_info* player) const;
	int							getsupport(const player_info* player) const;
	static bsreq				metadata[];
	void						render_neighbors(const rect& rc) const;
	static void					update_status(const player_info* player);
private:
	player_info * player;
	landscape_info*				landscape;
	char						level;
	point						position;
	short						support[player_max];
	province_info*				neighbors[8];
};
struct report_info {
	constexpr report_info() : hero(0), player(0), province(0), text(0) {}
	static report_info*			add(player_info* player, province_info* province, hero_info* hero, const char* text);
	const province_info*		getprovince() const { return province; }
	const char*					get() const { return text; }
	bool						is(const player_info* player) const;
	void						set(player_info* player);
private:
	hero_info*					hero;
	player_info*				player;
	province_info*				province;
	const char*					text;
};
struct trait_info : name_info, character_info {
};
struct tactic_info : name_info, combat_info {
	constexpr tactic_info(const char* id) : name_info(id), combat_info() {}
};
struct hero_info : name_info {
	void						before_turn();
	int							get(const char* id) const;
	const char*					getavatar() const { return avatar; }
	action_info*				getaction() const { return action; }
	int							getbonus(const char* id) const;
	int							getex(const char* id) const { return get(id) + getbonus(id); }
	int							getincome() const;
	player_info*				getplayer() const { return player; }
	province_info*				getprovince() const { return province; }
	tactic_info*				gettactic() const { return tactic; }
	static bsreq				metadata[];
	bool						isready() const { return true; }
	void						resolve();
	void						setaction(action_info* value) { action = value; }
	void						setprovince(province_info* value) { province = value; }
	trait_info*					traits[4];
private:
	action_info*				action;
	const char*					avatar;
	const char*					nameof;
	player_info*				player;
	province_info*				province;
	tactic_info*				tactic;
};
struct unit_info : name_info, combat_info, prof_info {
	const char*					nameof;
};
struct troop_info {
	explicit operator bool() const { return type != 0; }
	static int					compare(const void* p1, const void* p2);
	int							fix(tip_info* ti, int value) const { return type->fix(ti, type->name, value); }
	int							get(const char* id) const { return type->get(id); }
	int							getbonus(const char* id) const { return 0; }
	int							getincome() const { return type->income; }
	const char*					getname() const { return type->name; }
	const char*					getnameof() const { return type->nameof; }
	player_info*				getplayer() const { return player; }
	static const char*			getpresent(char* result, const char* result_maximum, troop_info** source, unsigned count);
	province_info*				getprovince() const { return province; }
	province_info*				getprovince(const player_info* player) const;
	province_info*				getmove() const { return move; }
	static bsreq				metadata[];
	void						setmove(province_info* value) { move = value; }
	static void					sort(troop_info** source, unsigned count);
private:
	unit_info*					type;
	province_info*				province;
	province_info*				move;
	player_info*				player;
};
struct player_info : name_info {
	static void					after_turn();
	static void					before_turn();
	static unsigned				getactions(hero_info** source, unsigned maximum_count, int order);
	province_info*				getbestprovince() const;
	int							getgold() const { return gold; }
	int							getincome(tip_info* ti = 0) const;
	int							getinfluence() const { return influence; }
	const char*					getnameof() const { return nameof; }
	static unsigned				getheroes(hero_info** source, unsigned maximum_count, const province_info* province = 0, const player_info* player = 0);
	static unsigned				getprovinces(province_info** source, unsigned maximum, const player_info* player = 0, province_flag_s state = AnyProvince);
	int							getsupport(tip_info* ti = 0) const;
	static unsigned				gettroops(troop_info** source, unsigned maximum_count, const province_info* province = 0, const player_info* player = 0, const player_info* player_move = 0);
	void						makemove();
	static void					maketurn();
	static bsreq				metadata[];
	static void					resolve_actions();
private:
	const char*					nameof;
	int							influence;
	int							gold;
};
struct game_info {
	action_info*				default_action;
	char						income_per_level, casualties;
	const char*					map;
};
struct msg_info {
	const char*	attacking_force;
	const char*	defending_force;
	const char* casualties;
	const char* winner;
	const char* lead;
	const char* attack;
	const char* defend;
	const char* raid;
	const char* sword;
	const char* shield;
	const char* diplomacy;
	const char* cruelty;
	const char* nobility;
	const char* will;
	const char* total_strenght;
	const char *predict_fail, *predict_partial, *predict_success;
	const char* income;
	const char* income_province;
	const char* income_units;
	const char* income_heroes;
	const char* cost;
	const char* squads;
	const char* title;
	const char* exit;
	const char* loadgame;
	const char* newgame;
	const char* savegame;
	const char* accept;
	const char* cancel;
	const char* yes;
	const char* no;
};
struct gui_info {
	unsigned char				border;
	unsigned char				opacity, opacity_disabled, opacity_hilighted, opacity_hilighted_province;
	short						button_width, window_width, tips_width, hero_width, hero_window_width, control_border;
	short						padding;
};
struct army : adat<troop_info*, 32> {
	hero_info*					general;
	player_info*				player;
	province_info*				province;
	const tactic_info*			tactic;
	bool						attack;
	bool						raid;
	constexpr army() : general(0), player(0), tactic(0), province(0), attack(false), raid(false) {}
	army(player_info* player, hero_info* general, bool attack, bool raid);
	void						fill(const player_info* player, const province_info* province);
	int							get(const char* id, tip_info* ti, bool include_number = true) const;
	int							getstrenght(tip_info* ti, bool include_number = true) const;
};
namespace draw {
void							addaccept(char* result, const char* result_max);
void							addbutton(char* result, const char* result_max, const char* name);
void							avatar(int x, int y, const char* id);
inline int						button(int x, int y, int width, const char* label, const runable& e, unsigned key = 0) { return 0; }
int								buttonw(int x, int y, int width, const char* label, const runable& e, unsigned key = 0);
action_info*					getaction(player_info* player, hero_info* hero);
color							getcolor(province_flag_s id);
province_info*					getprovince(player_info* player, hero_info* hero, action_info* action, aref<province_info*> selection, color selection_color);
areas							hilite(rect rc);
bool							initializemap();
bool							move(const player_info* player, hero_info* hero, const action_info* action, const province_info* province, army& s1, army& s2, const army& a3);
void							report(const char* format);
areas							window(rect rc, bool disabled = false, bool hilight = false, int border = 0);
int								window(int x, int y, int width, const char* string);
int								windowb(int x, int y, int width, const char* string, const runable& e, int border = 0, unsigned key = 0);
}
extern adat<action_info, 32>	action_data;
extern game_info				game;
extern gui_info					gui;
extern adat<hero_info, 128>		hero_data;
extern msg_info					msg;
extern adat<landscape_info, 32> landscape_data;
extern adat<player_info, player_max> player_data;
extern adat<province_info, province_max> province_data;
extern adat<report_info, 2048>	report_data;
extern tactic_info				tactic_data[];
extern adat<troop_info, 256>	troop_data;
extern adat<unit_info, 64>		unit_data;