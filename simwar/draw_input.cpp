#include "main.h"
#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

enum ui_command_s {
	NoCommand, ChooseLeft, ChooseRight, ChooseList,
};
struct cmdid {
	callback_proc		proc;
	int					param;
	void clear() { memset(this, 0, sizeof(*this)); }
};
struct focusable_element {
	int				id;
	rect			rc;
	operator bool() const { return id != 0; }
};
static focusable_element	elements[96];
static focusable_element*	render_control;
static int				current_focus;
static cmdid			current_execute;
static bool				keep_hot;
static hotinfo			keep_hot_value;
static bool				break_modal;
static int				break_result;
static point			camera;
static point			camera_drag;
static rect				last_board;
static point			tooltips_point;
static short			tooltips_width;
static char				tooltips_text[4096];
static surface			map_image;
static sprite*			sprite_shields;
static rect				hilite_rect;
const int				map_normal = 1000;
static int				map_scale = map_normal;
static const province_info*	current_province;
static control*			current_hilite;
static control*			current_focus_control;
static control*			current_execute_control;
extern rect				sys_static_area;
int						distance(point p1, point p2);
static amap<const char*, surface> bitmap_previews;

static bsreq gui_type[] = {
	BSREQ(gui_info, opacity, number_type),
	BSREQ(gui_info, opacity_disabled, number_type),
	BSREQ(gui_info, opacity_hilighted, number_type),
	BSREQ(gui_info, opacity_hilighted_province, number_type),
	BSREQ(gui_info, border, number_type),
	BSREQ(gui_info, control_border, number_type),
	BSREQ(gui_info, button_width, number_type),
	BSREQ(gui_info, button_border, number_type),
	BSREQ(gui_info, window_width, number_type),
	BSREQ(gui_info, hero_window_width, number_type),
	BSREQ(gui_info, tips_width, number_type),
	BSREQ(gui_info, hero_width, number_type),
	BSREQ(gui_info, padding, number_type),
{}};
gui_info gui;
bsdata gui_manger("gui", gui, gui_type);

void gui_info::initialize() {
	memset(this, 0, sizeof(*this));
	opacity = 220;
	opacity_disabled = 50;
	border = 8;
	padding = 4;
	window_width = 400;
	tips_width = 200;
	hero_width = 64;
	button_width = 64;
	button_border = 4;
	hero_window_width = 200;
	opacity_hilighted = 200;
	opacity_hilighted_province = 64;
}

static void set_focus_callback() {
	auto id = getnext(draw::getfocus(), hot.param);
	if(id)
		setfocus(id, true);
}

static void setfocus_callback() {
	current_focus = hot.param;
}

static focusable_element* getby(int id) {
	if(!id)
		return 0;
	for(auto& e : elements) {
		if(!e)
			return 0;
		if(e.id == id)
			return &e;
	}
	return 0;
}

static focusable_element* getfirst() {
	for(auto& e : elements) {
		if(!e)
			return 0;
		return &e;
	}
	return 0;
}

static focusable_element* getlast() {
	auto p = elements;
	for(auto& e : elements) {
		if(!e)
			break;
		p = &e;
	}
	return p;
}

void draw::addelement(int id, const rect& rc) {
	if(!render_control
		|| render_control >= elements + sizeof(elements) / sizeof(elements[0]) - 1)
		render_control = elements;
	render_control[0].id = id;
	render_control[0].rc = rc;
	render_control[1].id = 0;
	render_control++;
}

int draw::getnext(int id, int key) {
	if(!key)
		return id;
	auto pc = getby(id);
	if(!pc)
		pc = getfirst();
	if(!pc)
		return 0;
	auto pe = pc;
	auto pl = getlast();
	int inc = 1;
	if(key == KeyLeft || key == KeyUp || key == (KeyTab | Shift))
		inc = -1;
	while(true) {
		pc += inc;
		if(pc > pl)
			pc = elements;
		else if(pc < elements)
			pc = pl;
		if(pe == pc)
			return pe->id;
		switch(key) {
		case KeyRight:
			if(pe->rc.y1 >= pc->rc.y1
				&& pe->rc.y1 <= pc->rc.y2
				&& pe->rc.x1 < pc->rc.x1)
				return pc->id;
			break;
		case KeyLeft:
			if(pe->rc.y1 >= pc->rc.y1
				&& pe->rc.y1 <= pc->rc.y2
				&& pe->rc.x1 > pc->rc.x1)
				return pc->id;
			break;
		case KeyDown:
			if(pc->rc.y1 >= pe->rc.y2)
				return pc->id;
			break;
		case KeyUp:
			if(pc->rc.y2 <= pe->rc.y1)
				return pc->id;
			break;
		default:
			return pc->id;
		}
	}
}

void draw::setfocus(int id, bool instant) {
	if(instant)
		current_focus = id;
	else if(current_focus != id)
		execute(setfocus_callback, id);
}

int draw::getfocus() {
	return current_focus;
}

void draw::execute(void(*proc)(), int param) {
	current_execute.proc = proc;
	current_execute.param = param;
}

void draw::execute(const hotinfo& value) {
	keep_hot = true;
	keep_hot_value = value;
	hot.key = InputUpdate;
}

void draw::breakmodal(int result) {
	break_modal = true;
	break_result = result;
}

void draw::buttoncancel() {
	breakmodal(0);
}

void draw::buttonok() {
	breakmodal(1);
}

int draw::getresult() {
	return break_result;
}

bool control::ishilited() const {
	return current_hilite == this;
}

bool control::isfocused() const {
	return current_focus_control == this;
}

void control::view(const rect& rc) {
	if(isfocusable()) {
		addelement((int)this, rc);
		if(!getfocus())
			setfocus((int)this, true);
	}
	if(areb(rc))
		current_hilite = this;
	if((control*)getfocus() == this)
		current_focus_control = this;
	if(show_border)
		rectb(rc, colors::border);
}

static bool control_focus() {
	if(current_hilite) {
		switch(hot.key & CommandMask) {
		case MouseLeft:
		case MouseRight:
		case MouseLeftDBL:
			current_hilite->mouseinput(hot.key, hot.mouse);
			return true;
		case MouseWheelDown:
			current_hilite->mousewheel(hot.key, hot.mouse, 1);
			return true;
		case MouseWheelUp:
			current_hilite->mousewheel(hot.key, hot.mouse, -1);
			return true;
		}
	}
	if(current_focus_control) {
		if(current_focus_control->keyinput(hot.key))
			return true;
	}
	int id;
	switch(hot.key) {
	case 0:
		exit(0);
		return true;
	case KeyTab:
	case KeyTab | Shift:
	case KeyTab | Ctrl:
	case KeyTab | Ctrl | Shift:
		id = getnext(draw::getfocus(), hot.key);
		if(id)
			setfocus(id, true);
		return true;
	}
	return false;
}

static void before_render() {
	hot.cursor = CursorArrow;
	hot.hilite.clear();
	render_control = elements;
	current_execute.clear();
	current_hilite = 0;
	current_focus_control = 0;
	if(hot.mouse.x < 0 || hot.mouse.y < 0)
		sys_static_area.clear();
	else
		sys_static_area = {0, 0, draw::getwidth(), draw::getheight()};
}

bool draw::ismodal() {
	before_render();
	if(!break_modal)
		return true;
	break_modal = false;
	return false;
}

static areas hilite(rect rc) {
	auto border = gui.border;
	rc.offset(-border, -border);
	color c = colors::form;
	auto rs = draw::area(rc);
	if(rs == AreaHilited) {
		auto op = gui.opacity_hilighted_province;
		draw::rectf(rc, c, op);
		draw::rectb(rc, c);
	}
	return rs;
}

static areas window(rect rc, bool disabled = false, bool hilight = true, int border = 0) {
	if(border == 0)
		border = gui.border;
	rc.offset(-border, -border);
	color c = colors::form;
	color b = colors::form;
	auto rs = draw::area(rc);
	auto op = gui.opacity;
	if(disabled)
		op = op / 2;
	else if(hilight && (rs == AreaHilited || rs == AreaHilitedPressed))
		op = gui.opacity_hilighted;
	draw::rectf(rc, c, op);
	draw::rectb(rc, b);
	return rs;
}

static int render_text(int x, int y, int width, const char* string) {
	draw::link[0] = 0;
	auto result = textf(x, y, width, string);
	if(draw::link[0])
		tooltips(x, y, width, draw::link);
	return result;
}

static int windowf(int x, int y, int width, const char* string) {
	rect rc = {x, y, x + width, y};
	draw::state push;
	draw::font = metrics::font;
	auto height = textf(rc, string);
	rc.x2 = rc.x1 + width;
	window(rc, false);
	render_text(x, y, rc.width(), string);
	return height + gui.border * 2 + gui.padding;
}

static int window(int x, int y, int width, const char* string, int right_width = 0) {
	auto right_side = (right_width != 0);
	rect rc = {x, y, x + width, y};
	draw::state push;
	draw::font = metrics::font;
	auto height = textf(rc, string);
	if(right_side) {
		auto w1 = rc.width();
		x = x + right_width - w1;
		rc.x1 = x;
		rc.x2 = rc.x1 + w1;
	}
	window(rc, false, false);
	render_text(x, y, rc.width(), string);
	return height + gui.border * 2 + gui.padding;
}

static int windowb(int x, int y, int width, const char* string, const runable& e, int border = 0, unsigned key = 0, const char* tips = 0) {
	draw::state push;
	draw::font = metrics::font;
	rect rc = {x, y, x + width, y + draw::texth()};
	auto ra = window(rc, e.isdisabled(), true, border);
	draw::text(rc, string, AlignCenterCenter);
	if((ra == AreaHilited || ra == AreaHilitedPressed) && tips)
		tooltips(x, y, rc.width(), tips);
	if(!e.isdisabled()
		&& ((ra == AreaHilitedPressed && hot.key == MouseLeft)
			|| (key && key == hot.key)))
		e.execute();
	return rc.height() + gui.border * 2;
}

static void avatar(int x, int y, const char* id) {
	static amap<const char*, surface> avatars;
	auto p = avatars.find(id);
	if(!p) {
		p = avatars.add(id, surface());
		p->value.resize(gui.hero_width, gui.hero_width, 32, true);
		surface e(id, 0);
		if(e)
			blit(p->value, 0, 0, p->value.width, p->value.height, 0, e, 0, 0, e.width, e.height);
	}
	blit(*draw::canvas, x, y, gui.hero_width, gui.hero_width, 0, p->value, 0, 0);
	rectb({x, y, x + gui.hero_width, y + gui.hero_width}, colors::border);
}

static int windowh(const hero_info* hero, const char* format, const char* format_param) {
	string sb;
	draw::state push;
	draw::font = metrics::font;
	auto width = gui.window_width;
	auto avatar_width = 0;
	if(hero)
		avatar_width = gui.hero_width + gui.padding;
	width -= avatar_width;
	rect rc = {0, 0, width, 0};
	auto h = textf(rc, format);
	auto w = rc.width() + avatar_width;
	auto x = getwidth() - w - gui.border - gui.padding;
	auto y = gui.padding + gui.border;
	if(hero) {
		if(h < gui.hero_width)
			h = gui.hero_width;
	}
	auto hittest = window({x, y, x + w, y + h}, false, false);
	auto x1 = x;
	auto y1 = y;
	if(hero) {
		avatar(x, y, hero->getavatar());
		if(hittest == AreaHilited || hittest == AreaHilitedPressed) {
			sb.clear();
			hero->getinfo(sb);
			tooltips(x, y, width, sb);
		}
		x += avatar_width;
		sb.clear();
		sb.set(hero);
	}
	sb.addv(format, format_param);
	draw::link[0] = 0;
	auto result = textf(x, y, width, sb);
	if(draw::link[0])
		tooltips(x1, y1, w, draw::link);
	return y + h + gui.border * 2;
}

static int render_player(int x, int y, const player_info* player) {
	if(!player)
		return 0;
	string sb;
	sb.add("###%+1\n", player->getname());
	player->getinfo(sb);
	return window(x, y, gui.window_width, sb);
}

static void choose_current_province() {
	current_province = (province_info*)hot.param;
}

static point getscreen(const rect& rc, point pt) {
	auto x = pt.x - camera.x + rc.x1 + rc.width() / 2;
	auto y = pt.y - camera.y + rc.y1 + rc.height() / 2;
	return {(short)x, (short)y};
}

static point getmappos(const rect& rc, point pt) {
	auto x = pt.x + camera.x - rc.x1 - rc.width() / 2;
	auto y = pt.y + camera.y - rc.y1 - rc.height() / 2;
	return {(short)x, (short)y};
}

void province_info::render_neighbors(const rect& rc) const {
	draw::state push;
	draw::fore = colors::red;
	point center = getscreen(rc, getposition());
	for(auto p : neighbors) {
		if(!p)
			break;
		auto pt = getscreen(rc, p->getposition());
		line(center.x, center.y, pt.x, pt.y);
	}
}

static void render_shield(int x, int y, const province_info* province, const player_info* player) {
	auto province_player = province->getplayer();
	if(!province_player)
		return;
	auto old_stroke = fore;
	auto value = province->getsupport(player);
	fore = colors::gray;
	if(value > 0)
		fore = colors::yellow;
	else if(value < 0)
		fore = colors::red;
	auto index = province_player->getindex();
	stroke(x, y, sprite_shields, index, 0, 2);
	image(x, y, sprite_shields, index, 0);
	fore = old_stroke;
}

static void render_province(rect rc, point mouse, const player_info* player, callback_proc proc, aref<province_info*> visible, aref<province_info*> selection, color selection_color, bool set_current_province) {
	if(!draw::font)
		return;
	draw::state push;
	const int max_size = 128;
	string sb;
	for(auto province : visible) {
		if(!province)
			continue;
		auto pt = getscreen(rc, province->getposition());
		if((rc.x1 > pt.x + max_size) || (rc.y1 > pt.y + max_size) || (rc.x2 < pt.x - max_size) || (rc.y2 < pt.y - max_size))
			continue;
		draw::font = metrics::h1;
		fore = colors::black;
		fore_stroke = colors::white.mix(selection_color);
		auto text_width = draw::textw(province->getname());
		army defenders;
		defenders.province = province;
		defenders.count = troop_info::select(defenders.data, defenders.getmaximum(), province);
		rect rc = {pt.x - text_width / 2, pt.y - draw::texth() / 2, pt.x + text_width / 2, pt.y + draw::texth() / 2};
		auto status = province->getstatus(player);
		auto inlist = selection.is(province);
		if(inlist)
			fore_stroke = colors::white.mix(selection_color);
		else
			fore_stroke = colors::white;
		auto a = AreaNormal;
		if(set_current_province || inlist)
			a = draw::area(rc);
		if(a == AreaHilited || a == AreaHilitedPressed) {
			if(inlist) {
				fore_stroke = selection_color;
				hot.cursor = CursorHand;
				if(hot.key == MouseLeft && hot.pressed) {
					if(proc)
						draw::execute(proc, (int)province);
				}
			} else {
				if(hot.key == MouseLeft && hot.pressed)
					draw::execute(choose_current_province, (int)province);
			}
		}
		render_shield(pt.x - text_width / 2 - 20, pt.y, province, player);
		draw::text(pt.x - text_width / 2, pt.y - draw::texth() / 2, province->getname(), -1, TextStroke);
		pt.y += draw::texth() / 2;
		draw::font = metrics::font;
		auto hero = province->gethero(player);
		if(status == FriendlyProvince)
			defenders.general = hero;
		sb.clear();
		sb.add("%+1 ", province->getnation()->getname());
		province->getinfo(sb, false, &defenders);
		auto w = textfw(sb);
		textf(pt.x - w / 2, pt.y, w, sb, 0, 0, 0, 0, 0, TextStroke);
		pt.y += texth();
		if(defenders) {
			sb.clear();
			troop_info::sort(defenders.data, defenders.getcount());
			troop_info::getpresent(sb, defenders.data, defenders.getcount(), 0);
			rect rc = {0, 0, 200, 0}; draw::textw(rc, sb);
			pt.y += draw::text({pt.x - rc.width() / 2, pt.y, pt.x + rc.width() / 2 + 1, pt.y + rc.height()}, sb, AlignCenter);
		}
		if(hero) {
			sb.clear();
			auto action = hero->getaction();
			if(action)
				sb.add("%1 %2", hero->getname(), action->nameof);
			else
				sb.add("%1", hero->getname());
			text(pt.x - textw(sb) / 2, pt.y, sb);
			pt.y += texth();
		}
		defenders.count = troop_info::select_move(defenders.data, defenders.getmaximum(), province, player);
		if(defenders) {
			sb.clear();
			troop_info::sort(defenders.data, defenders.getcount());
			troop_info::getpresent(sb, defenders.data, defenders.count, msg.moved);
			rect rc = {0, 0, 200, 0}; draw::textw(rc, sb);
			pt.y += draw::text({pt.x - rc.width() / 2, pt.y, pt.x + rc.width() / 2 + 1, pt.y + rc.height()}, sb, AlignCenter);
		}
		if(status == FriendlyProvince) {
			sb.clear();
			fore = fore.mix(colors::blue);
			build_info* build_array[16];
			auto count = build_info::select(build_array, lenghtof(build_array), province);
			if(count) {
				build_info::sort(build_array, count);
				build_info::getpresent(sb, build_array, count);
				rect rc = {0, 0, 200, 0}; draw::textw(rc, sb);
				pt.y += draw::text({pt.x - rc.width() / 2, pt.y, pt.x + rc.width() / 2 + 1, pt.y + rc.height()}, sb, AlignCenter);
			}
		}
	}
}

static void render_board(const rect& rco, const player_info* player, callback_proc proc, aref<province_info*> visible, aref<province_info*> selection, color selection_color, bool set_current_province, bool show_lines) {
	auto rc = rco;
	draw::state push;
	draw::area(rc); // Drag and drop analize this result
	last_board = rc;
	int w = rc.width();
	int h = rc.height();
	int x1 = camera.x - w / 2;
	int y1 = camera.y - h / 2;
	int x2 = x1 + w;
	int y2 = y1 + h;
	point last_mouse = {
		(short)(hot.mouse.x - rc.x1 - rc.width() / 2 + camera.x),
		(short)(hot.mouse.y - rc.y1 - rc.height() / 2 + camera.y)};
	if(x1 < 0) {
		rc.x1 -= x1;
		x1 = 0;
	}
	if(x2 > map_image.width) {
		rc.x2 -= x2 - map_image.width;
		x2 = map_image.width;
	}
	if(y1 < 0) {
		rc.y1 -= y1;
		y1 = 0;
	}
	if(y2 > map_image.height) {
		rc.y2 -= y2 - map_image.height;
		y2 = map_image.height;
	}
	if(rc.x1 != last_board.x1 || rc.y1 != last_board.y1 || rc.y2 != last_board.y2 || rc.x2 != last_board.x2)
		draw::rectf(last_board, colors::gray);
	if(rc.width() > 0 && rc.height() > 0)
		blit(*draw::canvas, rc.x1, rc.y1, rc.width(), rc.height(), 0, map_image, x1, y1);
	if(current_province && show_lines)
		current_province->render_neighbors(rco);
	if(player)
		render_province(last_board, last_mouse, player, proc, visible, selection, selection_color, set_current_province);
}

static int render_hero(int x, int y, int width, const hero_info* hero, callback_proc proc = 0, bool show_state = true) {
	if(!hero)
		return 0;
	draw::state push;
	draw::font = metrics::font;
	auto pa = hero->getavatar();
	int height = gui.hero_width;
	rect rc = {x, y, x + width, y + height};
	areas hittest = window(rc, show_state && !hero->isready(), proc != 0);
	int x1 = x;
	if(pa) {
		auto y1 = y;
		avatar(x, y1, pa);
		rectb({x, y1, x + gui.hero_width, y1 + gui.hero_width}, colors::border);
		x1 += gui.hero_width + gui.padding;
	}
	string sb;
	hero->getbrief(sb);
	draw::textf(x1, y - 3, rc.x2 - x1, sb);
	if(hittest == AreaHilited || hittest == AreaHilitedPressed) {
		sb.clear();
		hero->getinfo(sb);
		if(show_state)
			hero->getstate(sb);
		tooltips(x, y, width, sb);
		if(hittest == AreaHilitedPressed && hot.key == MouseLeft && proc)
			execute(proc, (int)hero);
	}
	return height + gui.border * 2;
}

static int render_hero(int x, int y, const hero_info* e) {
	return render_hero(x, y, gui.hero_window_width, e, 0, 0);
}

static int render_province(int x, int y, const province_info* province) {
	if(!province)
		return 0;
	string sb;
	sb.add("###%1\n", province->getname());
	province->getinfo(sb, true);
	province->getsupport(sb);
	if(province->text)
		sb.addn(province->text);
	draw::state state;
	draw::fore = colors::text;
	return window(x, y, gui.window_width, sb) + gui.border * 2 + gui.padding;
}

static void render_board(const player_info* player, callback_proc proc, aref<province_info*> selection, color selection_color, bool set_current_province, bool show_lines) {
	rect rc = {0, 0, draw::getwidth(), draw::getheight()};
	province_info* source[province_max];
	auto count = province_info::select(source, lenghtof(source), player);
	render_board(rc, player, proc, {source, count}, selection, selection_color, set_current_province, show_lines);
}

static void render_left_side(const player_info* player, const province_info* province, bool set_current_province = false) {
	render_board(player, 0, {}, colors::blue, set_current_province, province != 0);
	auto x = gui.border + gui.border;
	auto y = gui.padding + gui.border;
	if(player)
		y += render_player(x, y, player);
	if(province)
		y += render_province(x, y, province);
}

static void breakparam() {
	breakmodal(hot.param);
}

static void keyparam() {
	hot.key = hot.param;
	hot.param = 0;
}

static void mouse_map_info();

static bool control_board() {
	const int step = 32;
	switch(hot.key) {
	case MouseWheelUp: map_scale += 50; break;
	case MouseWheelDown: map_scale -= 50; break;
	case KeyLeft: camera.x -= step; break;
	case KeyRight: camera.x += step; break;
	case KeyUp: camera.y -= step; break;
	case KeyDown: camera.y += step; break;
	case MouseLeft:
		if(hot.pressed && last_board == hot.hilite) {
			draw::drag::begin(last_board);
			camera_drag = camera;
		} else
			return false;
		break;
	case Ctrl + Alpha + 'M': mouse_map_info(); break;
	default:
		if(draw::drag::active(last_board)) {
			hot.cursor = CursorAll;
			if(hot.mouse.x >= 0 && hot.mouse.y >= 0)
				camera = camera_drag + (draw::drag::mouse - hot.mouse);
			return true;
		}
		return false;
	}
	return true;
}

void control_standart() {
	if(control_focus())
		return;
	if(control_board())
		return;
}

static void setcamera(const province_info* province) {
	if(!province)
		return;
	current_province = province;
	camera = province->getposition();
}

static int choose_answer(const player_info* player, const province_info* province, const hero_info* hero, answer_info& source, const char* format, const char* format_param) {
	if(province)
		setcamera(province);
	while(ismodal()) {
		render_left_side(player, province);
		auto x = getwidth() - gui.hero_window_width - gui.border - gui.padding;
		auto y = windowh(hero, format, format_param) + gui.padding;
		for(unsigned i = 0; i < source.elements.count; i++)
			y += windowb(x, y, gui.hero_window_width, source.elements[i].text,
				cmd(breakparam, source.elements[i].param), 0,
				Alpha + '1' + i);
		domodal();
		control_standart();
	}
	return draw::getresult();
}

static void show_report(const player_info* player, const province_info* province, const hero_info* hero, const char* format, ...) {
	answer_info source; source.add(0, msg.accept);
	choose_answer(player, province, hero, source, format, xva_start(format));
}

static void mouse_map_info() {
	auto pt = getmappos(last_board, hot.mouse);
	char temp[512]; zprint(temp, "Координаты карты: %1i, %2i.", pt.x, pt.y);
	show_report(0, 0, 0, temp);
}

static void draw_icon(int& x, int& y, int x0, int x2, int* max_width, int& w, const char* id) {
	static amap<const char*, draw::surface> source;
	auto p = source.find(id);
	if(!p) {
		char temp[260];
		p = source.add(id, surface());
		memset(p, 0, sizeof(*p));
		p->value.read(szurl(temp, "art/icons", id, "png"));
	}
	auto dy = draw::texth();
	w = p->value.width;
	if(x + w > x2) {
		if(max_width)
			*max_width = imax(*max_width, x - x0);
		x = x0;
		y += draw::texth();
	}
	draw::blit(*draw::canvas, x, y + dy - p->value.height - 2, w, p->value.height, ImageTransparent, p->value, 0, 0);
}

void draw::tooltips(int x1, int y1, int width, const char* format, ...) {
	tooltips_point.x = x1;
	tooltips_point.y = y1;
	tooltips_width = width;
	szprint(tooltips_text, tooltips_text + sizeof(tooltips_text) - 1, format, xva_start(format));
}

static void render_tooltips() {
	if(!tooltips_text[0])
		return;
	draw::state push;
	draw::font = metrics::font;
	if(draw::font) {
		rect rc;
		rc.x1 = tooltips_point.x + tooltips_width + gui.border * 2 + gui.padding;
		rc.y1 = tooltips_point.y;
		rc.x2 = rc.x1 + gui.tips_width;
		rc.y2 = rc.y1;
		draw::textf(rc, tooltips_text);
		if(rc.x2 > getwidth() - gui.border - gui.padding) {
			auto w = rc.width();
			rc.x1 = tooltips_point.x - gui.border * 2 - gui.padding - w;
			rc.x2 = rc.x1 + w;
		}
		// Correct border
		int height = draw::getheight();
		int width = draw::getwidth();
		if(rc.y2 >= height)
			rc.move(0, height - 2 - rc.y2);
		if(rc.x2 >= width)
			rc.move(width - 2 - rc.x2, 0);
		window(rc);
		draw::fore = colors::tips::text;
		draw::textf(rc.x1, rc.y1, rc.width(), tooltips_text);
	}
	tooltips_text[0] = 0;
}

void draw::initialize() {
	colors::active = color::create(172, 128, 0);
	colors::border = color::create(73, 73, 80);
	colors::button = color::create(0, 122, 204);
	colors::form = color::create(32, 32, 32);
	colors::window = color::create(64, 64, 64);
	colors::text = color::create(255, 255, 255);
	colors::edit = color::create(38, 79, 120);
	colors::special = color::create(255, 244, 32);
	colors::border = colors::window.mix(colors::text, 128);
	colors::tips::text = color::create(255, 255, 255);
	colors::tips::back = color::create(100, 100, 120);
	colors::tabs::back = color::create(255, 204, 0);
	colors::tabs::text = colors::black;
	colors::h1 = colors::text.mix(colors::edit, 64);
	colors::h2 = colors::text.mix(colors::edit, 96);
	colors::h3 = colors::text.mix(colors::edit, 128);
	//colors::h1 = colors::white.mix(colors::form, 128);
	//colors::h2 = colors::white.mix(colors::form, 144);
	//colors::h3 = colors::white.mix(colors::form, 196);
	draw::font = metrics::font;
	draw::fore = colors::text;
	draw::fore_stroke = colors::blue;
	set(draw_icon);
}

static bool read_sprite(sprite** result, const char* name) {
	char temp[260];
	if(*result)
		delete *result;
	*result = (sprite*)loadb(szurl(temp, "art/sprites", name, "pma"));
	return (*result) != 0;
}

bool game_info::initializemap() {
	if(!map || !map[0])
		return false;
	char temp[260];
	if(!map_image.read(szurl(temp, "maps", map)))
		return false;
	if(!read_sprite(&sprite_shields, "shields"))
		return false;
	return true;
}

struct unit_list : list {

	unit_set&	source;
	int			id;

	unit_list(unit_set& source) : source(source), id(0) {}

	const char* getname(char* result, const char* result_maximum, int line, int column) const override {
		switch(column) {
		case 0:
			szprint(result, result_maximum, source.data[line]->getname());
			szupper(result, 1);
			break;
		case 1:
			source.data[line]->cost.get(result, result_maximum);
			break;
		default:
			return "";
		}
		return result;
	}

	int getmaximum() const override {
		return source.getcount();
	}

	unit_info* getcurrent() {
		if(current < source.getcount())
			return source.data[current];
		return 0;
	}

	bool keyinput(unsigned id) override {
		switch(id) {
		case KeyEnter:
			hot.key = this->id;
			break;
		default:
			return list::keyinput(id);
		}
		return true;
	}

};

struct army_list : list {

	army&		source;
	int			id;

	army_list(army& source) : source(source), id(0) {}

	const char* getname(char* result, const char* result_maximum, int line, int column) const override {
		switch(column) {
		case 0:
			szprint(result, result_maximum, source.data[line]->getname());
			szupper(result, 1);
			break;
		default:
			return "";
		}
		return result;
	}

	int getmaximum() const override {
		return source.getcount();
	}

	troop_info* getcurrent() {
		if(current < source.getcount())
			return source.data[current];
		return 0;
	}

	bool keyinput(unsigned id) override {
		switch(id) {
		case KeyEnter:
			hot.key = this->id;
			break;
		default:
			return list::keyinput(id);
		}
		return true;
	}

};

static color get_mode_color(province_flag_s id) {
	switch(id) {
	case NoFriendlyProvince: return colors::red;
	case FriendlyProvince: return colors::green;
	default: return colors::gray;
	}
}

const province_info* hero_info::choose_province(const action_info* action, aref<province_info*> source, province_flag_s mode) const {
	while(ismodal()) {
		render_board(player, breakparam, source, get_mode_color(mode), false, false);
		auto x = gui.border + gui.border;
		auto y = gui.padding + gui.border;
		if(player)
			y += render_player(x, y, player);
		x = getwidth() - gui.hero_window_width - gui.border - gui.padding;
		y = gui.padding + gui.border;
		y += render_hero(x, y, this) + 1;
		y += windowb(x, y, gui.hero_window_width, action->getname(), cmd(), gui.border) + 1;
		y += windowb(x, y, gui.hero_window_width, msg.cancel, cmd(breakparam, 0), 0, KeyEscape) + 1;
		domodal();
		control_standart();
	}
	auto p = (province_info*)getresult();
	if(p)
		current_province = p;
	return p;
}

struct game_header_list : list {

	ui_command_s	id;
	game_header*	source;
	unsigned		count;

	constexpr game_header_list(ui_command_s id, game_header* source, unsigned count) : id(id), source(source), count(count) {}

	const char* getname(char* result, const char* result_maximum, int line, int column) const override {
		switch(column) {
		case 0:
			szprint(result, result_maximum, source[line].id);
			szupper(result, 1);
			break;
		default:
			return "";
		}
		return result;
	}

	int getmaximum() const override {
		return count;
	}

	game_header* getcurrent() {
		if(current < (int)count)
			return source + current;
		return 0;
	}

	bool keyinput(unsigned id) override {
		switch(id) {
		case KeyEnter:
			hot.key = this->id;
			break;
		default:
			return list::keyinput(id);
		}
		return true;
	}

};

static int preview(int x, int y, int width, const char* url, const char* id) {
	if(!id)
		return 0;
	char temp[260]; zprint(temp, "%1/%2", url, id);
	auto p = bitmap_previews.find(id);
	if(!p) {
		p = bitmap_previews.add(id, surface());
		surface e(temp, 0);
		if(e.width) {
			auto h1 = width * e.height / e.width;
			p->value.resize(width, h1, 32, true);
			if(e)
				blit(p->value, 0, 0, width, h1, 0, e, 0, 0, e.width, e.height);
		}
	}
	if(!p->value.height)
		return 0;
	blit(*draw::canvas, x, y, p->value.width, p->value.height, 0, p->value, 0, 0);
	rectb({x, y, x + p->value.width, y + p->value.height}, colors::border);
	return p->value.height;
}

game_header* game_header::choose(game_header* source, unsigned count) {
	game_header_list u1(ChooseList, source, count);
	int x, y;
	auto height = 400;
	auto width = 600;
	auto cancel_button = true;
	while(ismodal()) {
		auto mx = getwidth();
		auto my = getheight();
		rectf({0, 0, mx, my}, colors::gray);
		auto x1 = (getwidth() - width) / 2;
		auto y1 = (getheight() - height) / 2;
		auto w1 = 200;
		window({x1, y1, x1 + width, y1 + height}, false, false);
		u1.view({x1 + w1 + gui.padding, y1, x1 + width, y1 + height});
		y = y1;
		x = x1;
		auto current = u1.getcurrent();
		if(current)
			y += preview(x1, y1, w1, "maps", current->map);
		y = y1 + height + gui.border * 2;
		x = x1 + width - gui.hero_window_width;
		if(cancel_button) {
			windowb(x, y, gui.hero_window_width, msg.cancel, cmd(buttoncancel), 0, KeyEscape);
			x -= gui.hero_window_width;
		}
		domodal();
		control_focus();
	}
	return (game_header*)getresult();
}

static void render_two_window(const player_info* player, const hero_info* hero, const action_info* action, list& u1, list& u2, const char* error_text, stringcreator& sb, const cost_info& cost) {
	if(cost) {
		char tem1[256]; cost.get(tem1, zendof(tem1));
		sb.adds("%1: %2", msg.total, tem1);
	}
	auto th = 0;
	if(sb) {
		rect rt = {0, 0, gui.window_width, 0};
		th = textf(rt, sb) + gui.padding;
	}
	render_left_side(player, 0);
	auto x = getwidth() - gui.hero_window_width - gui.border - gui.padding;
	auto y = gui.padding + gui.border;
	y += render_hero(x, y, hero) + 1;
	y += windowb(x, y, gui.hero_window_width, action->getname(), cmd(breakparam, 1, error_text != 0), 0, KeyEnter, error_text) + 1;
	y += windowb(x, y, gui.hero_window_width, msg.cancel, cmd(breakparam, 0), 0, KeyEscape) + 1;
	x = gui.border * 2;
	y = gui.padding + gui.border;
	auto x1 = x;
	auto w = gui.window_width / 2;
	auto pw = gui.padding / 2;
	y += render_player(x, y, player);
	auto y2 = getheight() - gui.padding * 2 - gui.border;
	auto y3 = y2 - th;
	window({x, y, x + gui.window_width, y2}, false, false);
	u1.view({x, y, x + w - pw, y3});
	x = x + w + gui.padding;
	u2.view({x, y, x + w - pw, y3});
	x = x1; y = y3 + gui.padding;
	y += render_text(x, y, gui.window_width, sb);
	domodal();
	control_standart();
}

bool hero_info::choose_units(const action_info* action, const province_info* province, unit_set& s1, unit_set& s2, cost_info& cost) const {
	if(player->iscomputer()) {
		zshuffle(s1.data, s1.getcount());
		cost_info total;
		for(unsigned i = 0; i < s1.count; i++) {
			total += s1.data[i]->cost;
			if(total > player->cost)
				break;
			s2.add(s1.data[i]);
		}
		return true;
	}
	unit_list u1(s1); u1.id = 10;
	unit_list u2(s2); u2.id = 11;
	auto player_cost = player->cost;
	auto start_cost = cost;
	while(ismodal()) {
		cost = s2.getcost();
		cost += start_cost;
		const char* error_info = 0;
		if(s2.getcount() == 0)
			error_info = msg.not_choose_units;
		else if(cost > player_cost)
			error_info = msg.not_enought_gold;
		string sb;
		render_two_window(player, this, action, u1, u2, error_info, sb, cost);
		if(hot.key == u1.id) {
			auto p1 = u1.getcurrent();
			if(p1) {
				u1.source.remove(u1.current);
				u2.source.add(p1);
			}
		} else if(hot.key == u2.id) {
			auto p1 = u2.getcurrent();
			if(p1) {
				u2.source.remove(u2.current);
				u1.source.add(p1);
			}
		}
	}
	return getresult() != 0;
}

bool hero_info::choose_troops(const action_info* action, const province_info* province, army& s1, army& s2, army& a3, int minimal_count, cost_info& cost) const {
	if(!s1.getcount() && minimal_count == 0)
		return true;
	if(player->iscomputer()) {
		zshuffle(s1.data, s1.getcount());
		cost_info total;
		for(unsigned i = 0; i < s1.count; i++) {
			total.gold += action->cost_per_unit;
			if(total.gold > player->cost.gold)
				break;
			a3.add(s1.data[i]);
		}
		return true;
	}
	army_list u1(s1); u1.id = 10;
	army_list u2(s2); u2.id = 11;
	string tid;
	auto th = texth() * 3 + 2;
	auto defender_strenght = a3.getstrenght(&tid);
	auto player_cost = player->cost;
	auto start_cost = cost;
	while(ismodal()) {
		const char* error_info = 0;
		string sb;
		if(s1.attack) {
			string tia;
			auto attacker_strenght = s2.getstrenght(&tia);
			sb.adds(msg.total_strenght, (const char*)tia, (const char*)tid);
			if(attacker_strenght <= defender_strenght)
				sb.adds(msg.predict_fail);
			else if(attacker_strenght <= defender_strenght + 2)
				sb.adds(msg.predict_partial);
			else
				sb.adds(msg.predict_success);
		}
		cost = start_cost;
		cost.gold += action->cost_per_unit * s2.getcount();
		if(minimal_count && s2.getcount() < minimal_count)
			error_info = msg.not_choose_units;
		else if(cost > player_cost)
			error_info = msg.not_enought_gold;
		render_two_window(player, this, action, u1, u2, error_info, sb, cost);
		if(hot.key == u1.id) {
			auto p1 = u1.getcurrent();
			if(p1) {
				u1.source.remove(u1.current);
				u2.source.add(p1);
			}
		} else if(hot.key == u2.id) {
			auto p1 = u2.getcurrent();
			if(p1) {
				u2.source.remove(u2.current);
				u1.source.add(p1);
			}
		}
	}
	if(!getresult())
		cost = start_cost;
	return getresult() != 0;
}

static void end_turn() {
	breakmodal(0);
}

//void game_info::editor(const char* map_name) {
//	while(ismodal()) {
//		render_board(0, 0, {}, colors::gray, false, false);
//		auto x = getwidth() - gui.hero_window_width - gui.border - gui.padding;
//		auto y = gui.padding + gui.border;
//		y += windowb(x, y, gui.hero_window_width, msg.accept, cmd(buttonok));
//		domodal();
//		control_standart();
//		if(hot.key == MouseLeft && hot.pressed && last_board == hot.hilite) {
//			auto pt = getmappos(last_board, hot.mouse);
//			auto province = province_data.add();
//			char temp[260]; zprint(temp, "province%1i", province->getindex());
//			province->setnation(&nation_data[0]);
//			province->setlandscape(&landscape_data[0]);
//		}
//	}
//}

int answer_info::choose(const hero_info* hero, bool cancel_button, answer_info::tips_type getinfo) const {
	while(ismodal()) {
		if(hero)
			render_left_side(hero->getplayer(), 0, false);
		auto x = getwidth() - gui.hero_window_width - gui.border - gui.padding;
		auto y = gui.padding + gui.border;
		y += render_hero(x, y, gui.hero_window_width, hero);
		for(auto& e : elements)
			y += windowb(x, y, gui.hero_window_width, e.getname(), cmd(breakparam, e.param));
		if(cancel_button)
			y += windowb(x, y, gui.hero_window_width, msg.cancel, cmd(buttoncancel), 0, KeyEscape);
		domodal();
		control_standart();
	}
	return getresult();
}

int answer_info::choose(bool cancel_button) const {
	auto button_height = texth() + gui.button_border * 2 + gui.padding;
	if(!elements.getcount())
		return 0;
	auto total_height = elements.getcount() * button_height + gui.padding;
	auto window_width = gui.window_width;
	while(ismodal()) {
		auto mx = getwidth();
		auto my = getheight();
		rectf({0, 0, mx, my}, colors::gray);
		auto x = (getwidth() - window_width - gui.border * 2) / 2;
		auto y = (getheight() - total_height) / 2;
		for(auto& e : elements)
			y += windowb(x, y, window_width, e.getname(), cmd(breakparam, e.param)) + gui.padding;
		if(cancel_button)
			y += windowb(x, y, window_width, msg.cancel, cmd(buttoncancel), 0, KeyEscape);
		domodal();
	}
	return getresult();
}

const tactic_info* hero_info::choose_tactic() const {
	answer_info ai;
	for(auto& e : tactic_data) {
		if(!e)
			continue;
		ai.add((int)&e, e.getname());
	}
	if(!ai)
		return 0;
	if(player->iscomputer())
		return (tactic_info*)ai.elements.data[rand() % ai.elements.count].param;
	ai.sort();
	return (tactic_info*)ai.choose(this);
}

int	player_info::choose(const hero_info* hero, answer_info& source, const char* format, ...) const {
	if(type == PlayerComputer)
		return source.elements.data[rand() % source.elements.count].param;
	return choose_answer(this, 0, hero, source, format, xva_start(format));
}

void player_info::show_reports() const {
	if(type == PlayerComputer)
		return;
	for(auto& e : report_data) {
		if(e.getturn() != game.turn)
			continue;
		if(!e.is(this))
			continue;
		if(e.getprovince())
			current_province = e.getprovince();
		show_report(this, e.getprovince(), e.gethero(), e.get());
	}
}

static void choose_move() {
	auto hero = (hero_info*)hot.param;
	if(hero)
		hero->make_move();
}

void player_info::make_move() {
	setcamera(getbestprovince());
	while(ismodal()) {
		render_left_side(this, current_province, true);
		auto x = getwidth() - gui.hero_window_width - gui.border - gui.padding;
		auto y = gui.padding + gui.border;
		for(auto& e : hero_data) {
			if(!e || e.getplayer() != this)
				continue;
			y += render_hero(x, y, gui.hero_window_width, &e, choose_move) + gui.padding;
		}
		y += windowb(x, y, gui.hero_window_width, msg.end_turn, cmd(end_turn), 0, Ctrl + Alpha + 'E');
		domodal();
		control_standart();
	}
}

void draw::domodal() {
	if(current_execute.proc) {
		auto ev = current_execute;
		before_render();
		hot.key = InputUpdate;
		hot.param = ev.param;
		ev.proc();
		before_render();
		hot.key = InputUpdate;
		return;
	}
	render_tooltips();
	if(hot.key == InputUpdate && keep_hot) {
		keep_hot = false;
		hot = keep_hot_value;
	} else
		hot.key = draw::rawinput();
	if(!hot.key)
		exit(0);
}