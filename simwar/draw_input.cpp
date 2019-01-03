#include "main.h"

using namespace draw;
using namespace draw::controls;

struct focusable_element {
	int				id;
	rect			rc;
	operator bool() const { return id != 0; }
};
static focusable_element	elements[96];
static focusable_element*	render_control;
static int				current_focus;
static callback_proc	current_execute;
static bool				keep_hot;
static hotinfo			keep_hot_value;
static bool				break_modal;
static int				break_result;
static point			camera;
static point			camera_drag;
static rect				last_board;
static point			last_mouse;
static point			tooltips_point;
static short			tooltips_width;
static char				tooltips_text[4096];
static surface			map;
static rect				hilite_rect;
const int				map_normal = 1000;
static int				map_scale = map_normal;
static player_info*		current_player;
static province_info*	current_province;
static control*			current_hilite;
static control*			current_focus_control;
static control*			current_execute_control;
extern rect				sys_static_area;
int						distance(point p1, point p2);

static bsreq gui_type[] = {
	BSREQ(gui_info, opacity, number_type),
	BSREQ(gui_info, opacity_disabled, number_type),
	BSREQ(gui_info, opacity_hilighted, number_type),
	BSREQ(gui_info, opacity_hilighted_province, number_type),
	BSREQ(gui_info, border, number_type),
	BSREQ(gui_info, control_border, number_type),
	BSREQ(gui_info, button_width, number_type),
	BSREQ(gui_info, window_width, number_type),
	BSREQ(gui_info, hero_window_width, number_type),
	BSREQ(gui_info, tips_width, number_type),
	BSREQ(gui_info, hero_width, number_type),
	BSREQ(gui_info, padding, number_type),
{}};
gui_info gui;
bsdata gui_manger("gui", gui, gui_type);

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
	current_execute = proc;
	hot.key = 0;
	hot.param = param;
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
	current_execute = 0;
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

static void render_power(int x, int y, const player_info* owner, const province_info* province) {
	draw::circlef(x, y, 4 * 4, colors::green, 128);
	draw::circle(x, y, 4 * 4, colors::green);
}

static int render_player(int x, int y, const player_info* player) {
	char temp[8192]; temp[0] = 0;
	char tips[1024]; tips[0] = 0;
	tip_info ti(tips);
	stringcreator sc;
	stringbuilder sb(sc, temp);
	sb.add("###%1\n", player->getname());
	auto value = player->getincome(&ti);
	sb.add(":gold:%1i[%4\"%3\"%+2i]", player->getgold(), value, tips, (value>=0) ? "+" : "-");
	value = player->getinfluence();
	sb.add(" :house:%1i", value);
	return window(x, y, gui.window_width, temp);
}

static void choose_current_province() {
	current_province = (province_info*)hot.param;
}

static point getscreen(const rect& rc, point pt) {
	auto x = pt.x - camera.x + rc.x1;
	auto y = pt.y - camera.y + rc.y1;
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

static void render_province(rect rc, point mouse, const player_info* player, callback_proc proc, aref<province_info*> selection, color selection_color) {
	unsigned count;
	char temp[1024];
	if(!draw::font)
		return;
	draw::state push;
	draw::fore = colors::black;
	for(auto& e : province_data) {
		if(!e)
			continue;
		draw::font = metrics::h1;
		fore_stroke = colors::white.mix(selection_color);
		zprint(temp, e.getname());
		auto text_width = draw::textw(temp);
		hero_info* hero_array[16];
		count = player_info::getheroes(hero_array, lenghtof(hero_array), &e, player);
		auto pt = getscreen(rc, e.getposition());
		rect rc = {pt.x - text_width / 2, pt.y - draw::texth() / 2, pt.x + text_width / 2, pt.y + draw::texth() / 2};
		auto a = draw::area(rc);
		auto inlist = selection.is(&e);
		if(inlist)
			fore_stroke = colors::white.mix(selection_color);
		else
			fore_stroke = colors::white;
		if(a == AreaHilited || a == AreaHilitedPressed) {
			if(inlist) {
				fore_stroke = selection_color;
				hot.cursor = CursorHand;
				if(hot.key == MouseLeft) {
					if(proc)
						draw::execute(proc, (int)&e);
				}
			} else {
				if(hot.key == MouseLeft)
					draw::execute(choose_current_province, (int)&e);
			}
		}
		draw::text(pt.x - text_width / 2, pt.y - draw::texth() / 2, temp, -1, TextStroke);
		pt.y += draw::texth() / 2;
		draw::font = metrics::font;
		if(count) {
			auto hero = hero_array[0];
			auto action = hero->getaction();
			if(action)
				zprint(temp, "%1 %2", hero->getname(), action->nameact);
			else
				zprint(temp, "%1", hero->getname());
			text(pt.x - textw(temp) / 2, pt.y, temp);
			pt.y += texth();
		}
		troop_info* troop_array[8];
		count = player_info::gettroops(troop_array, lenghtof(troop_array), &e, 0, player);
		if(count) {
			troop_info::sort(troop_array, count);
			troop_info::getpresent(temp, zendof(temp), troop_array, count);
			rect rc = {0, 0, 200, 0}; draw::textw(rc, temp);
			pt.y += draw::text({pt.x - rc.width() / 2, pt.y, pt.x + rc.width() / 2 + 1, pt.y + rc.height()}, temp, AlignCenter);
		}
	}
}

static void render_board(const rect& rco, const player_info* player, callback_proc proc, aref<province_info*> selection, color selection_color) {
	auto rc = rco;
	draw::state push;
	draw::area(rc); // Drag and drop analize this result
	last_board = rc;
	last_mouse.x = (short)(hot.mouse.x - rc.x1 + camera.x);
	last_mouse.y = (short)(hot.mouse.y - rc.y1 + camera.y);
	int w = rc.width();
	int h = rc.height();
	int x1 = camera.x;
	int y1 = camera.y;
	int x2 = x1 + w;
	int y2 = y1 + h;
	if(x1 < 0) {
		rc.x1 -= x1;
		x1 = 0;
	}
	if(x2 > map.width) {
		rc.x2 -= x2 - map.width;
		x2 = map.width;
	}
	if(y1 < 0) {
		rc.y1 -= y1;
		y1 = 0;
	}
	if(y2 > map.height) {
		rc.y2 -= y2 - map.height;
		y2 = map.height;
	}
	if(rc.x1 != last_board.x1 || rc.y1 != last_board.y1 || rc.y2 != last_board.y2 || rc.x2 != last_board.x2)
		draw::rectf(last_board, colors::gray);
	if(rc.width() > 0 && rc.height() > 0)
		blit(*draw::canvas, rc.x1, rc.y1, rc.width(), rc.height(), 0, map, x1, y1);
	if(current_province)
		current_province->render_neighbors(rco);
	if(player)
		render_province(last_board, last_mouse, player, proc, selection, selection_color);
}

static void render_board() {
	render_board({0, 0, draw::getwidth(), draw::getheight()}, current_player, 0, {}, colors::blue);
}

static int render_hero(int x, int y, int width, hero_info* e, const char* error_text, callback_proc proc = 0) {
	char temp[2048]; temp[0] = 0;
	draw::state push;
	draw::font = metrics::font;
	auto pa = e->getavatar();
	int height = gui.hero_width;
	szprint(zend(temp), zendof(temp), "###%1\n", e->getname());
	for(auto p : e->traits) {
		if(!p)
			continue;
		zcat(temp, p->getname());
		zcat(temp, "\n");
	}
	auto owner = e->getplayer();
	rect rc = {x, y, x + width, y + height};
	areas hittest = window(rc, error_text!=0, true);
	//if(owner)
	//	draw::shield(x + drw.hero_width - 20, y + 18, owner->getimage());
	int x1 = x;
	if(pa) {
		int y1 = y;
		avatar(x, y1, pa);
		rectb({x, y1, x + gui.hero_width, y1 + gui.hero_width}, colors::border);
		x1 += gui.hero_width + gui.padding;
	}
	draw::textf(x1, y - 3, rc.x2 - x1, temp);
	if(hittest == AreaHilited || hittest == AreaHilitedPressed) {
		temp[0] = 0;
		// Ability block
		auto ps = zend(temp);
		auto ph = hero_info::metadata;
		for(auto ppf = character_type; *ppf; ppf++) {
			auto pf = msg_type->find(ppf->id);
			if(!pf)
				continue;
			auto pn = (const char*)pf->get(pf->ptr(&msg));
			if(!pn)
				continue;
			auto value = e->get(ppf->id);
			if(!value)
				continue;
			if(ps[0])
				zcat(temp, "\n");
			szprint(zend(temp), zendof(temp), "%+2i %1", pn, value);
		}
		if(error_text) {
			if(temp[0])
				szprint(zend(temp), zendof(temp), "\n");
			szprint(zend(temp), zendof(temp), "[-%1]", error_text);
		}
		tooltips(x, y, width, temp);
		if(hittest == AreaHilitedPressed && hot.key == MouseLeft && proc)
			execute(proc, (int)e);
	}
	return height + gui.border * 2;
}

static int render_hero(int x, int y, hero_info* e) {
	return render_hero(x, y, gui.hero_window_width, e, false, 0);
}

static int render_heroes(int x, int y, const player_info* player, callback_proc proc) {
	auto y0 = y;
	if(!player)
		return 0;
	for(auto& e : hero_data) {
		if(!e || e.getplayer() != player)
			continue;
		char temp[260]; temp[0] = 0;
		const char* error_text = 0;
		if(!e.isready()) {
			zprint(temp, msg.hero_wait, e.getwait());
			error_text = temp;
		}
		y += render_hero(x, y, gui.hero_window_width, &e, error_text, proc);
		y += gui.padding;
	}
	return y - y0;
}

static int render_province(int x, int y, const province_info* province) {
	char temp[4096];
	if(!province)
		return 0;
	stringcreator sc;
	stringbuilder sb(sc, temp);
	sb.addh(province->getname());
	sb.addn("%1 :gold:%2i :house:%3i :shield_grey:%4i", province->getlandscape()->name,
		province->getincome(), province->getlevel(), province->getdefend());
	if(province->text)
		sb.addn(province->text);
	if(!temp[0])
		return 0;
	draw::state state;
	draw::fore = colors::text;
	return draw::window(x, y, gui.window_width, temp) + gui.border * 2 + gui.padding;
}

static int render_left_side(bool show_current_province = true) {
	auto x = gui.border + gui.border;
	auto y = gui.padding + gui.border;
	if(current_player)
		y += render_player(x, y, current_player);
	if(show_current_province && current_province)
		y += render_province(x, y, current_province);
	return y;
}

static int render_right_side(callback_proc proc = 0) {
	auto x = getwidth() - gui.hero_window_width - gui.border - gui.padding;
	auto y = gui.padding + gui.border;
	return y + render_heroes(x, y, current_player, proc);
}

static void mouse_map_info() {
	auto pt = hot.mouse + camera;
	char temp[512]; zprint(temp, "���������� �����: %1i, %2i.", pt.x, pt.y);
	addaccept(temp, zendof(temp));
	report(temp);
}

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
		if(hot.pressed) {
			if(last_board == hot.hilite) {
				draw::drag::begin(last_board);
				camera_drag = camera;
			}
		}
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
	draw::blit(*draw::canvas, x, y + dy - p->value.height - 1, w, p->value.height, ImageTransparent, p->value, 0, 0);
}

areas draw::hilite(rect rc) {
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

areas draw::window(rect rc, bool disabled, bool hilight, int border) {
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

int draw::window(int x, int y, int width, const char* string) {
	rect rc = {x, y, x + width, y};
	draw::state push;
	draw::font = metrics::font;
	auto height = textf(rc, string);
	window(rc, false);
	link[0] = 0; textf(x, y, rc.width(), string);
	if(link[0])
		tooltips(x, y, rc.width(), link);
	return height + gui.border * 2 + gui.padding;
}

int draw::windowb(int x, int y, int width, const char* string, const runable& e, int border, unsigned key, const char* tips) {
	draw::state push;
	draw::font = metrics::font;
	rect rc = {x, y, x + width, y + draw::texth()};
	auto ra = window(rc, e.isdisabled(), true, border);
	draw::text(rc, string, AlignCenterCenter);
	if((ra==AreaHilited || ra==AreaHilitedPressed) && tips)
		tooltips(x, y, rc.width(), tips);
	if(!e.isdisabled()
		&& ((ra == AreaHilitedPressed && hot.key == MouseLeft)
			|| (key && key == hot.key)))
		e.execute();
	return rc.height() + gui.border * 2;
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
		draw::window(rc);
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

bool draw::initializemap() {
	if(!game.map || !game.map[0])
		return false;
	char temp[260];
	if(!map.read(szurl(temp, "maps", game.map)))
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
			szprint(result, result_maximum, ":gold:%1i", source.data[line]->get("cost"));
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

	void row(const rect& rc, int index) override {
		char temp[260]; temp[0] = 0;
		rowhilite(rc, index);
		auto p = getname(temp, zendof(temp), index, 0);
		if(p)
			draw::textc(rc.x1 + 4, rc.y1 + 4, rc.width() - 4 * 2, p);
		p = getname(temp, zendof(temp), index, 1);
		if(p) {
			auto w = textfw(p);
			textf(rc.x2 - 4 - w, rc.y1 + 4, rc.width() - 4 * 2, p);
		}
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

void draw::report(const char* format) {
	while(ismodal()) {
		render_board();
		render_right_side();
		auto x = gui.border * 2;
		auto y = render_left_side(false);
		draw::window(x, y, gui.window_width, format);
		domodal();
		control_standart();
	}
}

static void breakparam() {
	breakmodal(hot.param);
}

action_info* draw::getaction(player_info* player, hero_info* hero) {
	while(ismodal()) {
		render_board();
		auto x = getwidth() - gui.hero_window_width - gui.border - gui.padding;
		auto y = gui.padding + gui.border;
		y += render_hero(x, y, hero) + 1;
		for(auto& e : action_data)
			y += windowb(x, y, gui.hero_window_width, e.getname(), cmd(breakparam, (int)&e), gui.border) + 1;
		y += windowb(x, y, gui.hero_window_width, msg.cancel, cmd(breakparam, 0), 0, KeyEscape) + 1;
		domodal();
		control_standart();
	}
	return (action_info*)getresult();
}

province_info* draw::getprovince(player_info* player, hero_info* hero, action_info* action, aref<province_info*> selection, color selection_color) {
	while(ismodal()) {
		rect rc = {0, 0, draw::getwidth(), draw::getheight()};
		render_board(rc, player, breakparam, selection, selection_color);
		auto x = getwidth() - gui.hero_window_width - gui.border - gui.padding;
		auto y = gui.padding + gui.border;
		y += render_hero(x, y, hero) + 1;
		y += windowb(x, y, gui.hero_window_width, action->getname(), cmd(), gui.border) + 1;
		y += windowb(x, y, gui.hero_window_width, msg.cancel, cmd(breakparam, 0), 0, KeyEscape) + 1;
		domodal();
		control_standart();
	}
	return (province_info*)getresult();
}

bool draw::recruit(const player_info* player, hero_info* hero, const action_info* action, const province_info* province, unit_set& s1, unit_set& s2) {
	unit_list u1(s1); u1.id = 10;
	unit_list u2(s2); u2.id = 11;
	auto th = texth() * 3 + 2;
	while(ismodal()) {
		render_board();
		auto gold = s2.get("cost");
		auto x = getwidth() - gui.hero_window_width - gui.border - gui.padding;
		auto y = gui.padding + gui.border;
		y += render_hero(x, y, hero) + 1;
		y += windowb(x, y, gui.hero_window_width, action->getname(), cmd(), gui.border) + 1;
		auto isdisabled = false;
		const char* error_info = 0;
		if(gold > player->getgold()) {
			isdisabled = true;
			error_info = msg.not_enought_gold;
		}
		y += windowb(x, y, gui.hero_window_width, msg.accept, cmd(breakparam, 1, isdisabled), 0, KeyEnter, error_info) + 1;
		y += windowb(x, y, gui.hero_window_width, msg.cancel, cmd(breakparam, 0), 0, KeyEscape) + 1;
		x = gui.border * 2;
		y = gui.padding + gui.border;
		auto x1 = x;
		auto w = gui.window_width / 2;
		auto pw = gui.padding / 2;
		y += render_player(x, y, player);
		auto y2 = getheight() - gui.padding * 2 - gui.border;
		auto y3 = y2 - th;
		draw::window({x, y, x + gui.window_width, y2});
		u1.view({x, y, x + w - pw, y3});
		x = x + w + gui.padding;
		u2.view({x, y, x + w - pw, y3});
		x = x1; y = y3 + 2;
		char temp[1024]; zprint(temp, msg.total_cost, gold);
		y += textf(x, y, gui.window_width, temp);
		domodal();
		control_standart();
		if(hot.key == u1.id) {
			auto p1 = u1.getcurrent();
			u1.source.remove(u1.current);
			u2.source.add(p1);
		} else if(hot.key == u2.id) {
			auto p1 = u2.getcurrent();
			u2.source.remove(u2.current);
			u1.source.add(p1);
		}
	}
	return getresult() != 0;
}

bool draw::move(const player_info* player, hero_info* hero, const action_info* action, const province_info* province, army& s1, army& s2, const army& a3) {
	army_list u1(s1); u1.id = 10;
	army_list u2(s2); u2.id = 11;
	auto th = texth() * 3 + 2;
	while(ismodal()) {
		render_board();
		auto x = getwidth() - gui.hero_window_width - gui.border - gui.padding;
		auto y = gui.padding + gui.border;
		y += render_hero(x, y, hero) + 1;
		y += windowb(x, y, gui.hero_window_width, action->getname(), cmd(), gui.border) + 1;
		y += windowb(x, y, gui.hero_window_width, msg.accept, cmd(breakparam, 1), 0, KeyEnter) + 1;
		y += windowb(x, y, gui.hero_window_width, msg.cancel, cmd(breakparam, 0), 0, KeyEscape) + 1;
		x = gui.border * 2;
		y = gui.padding + gui.border;
		auto x1 = x;
		auto w = gui.window_width / 2;
		auto pw = gui.padding / 2;
		y += render_player(x, y, player);
		auto y2 = getheight() - gui.padding * 2 - gui.border;
		auto y3 = y2 - th;
		draw::window({x, y, x + gui.window_width, y2});
		u1.view({x, y, x + w - pw, y3});
		x = x + w + gui.padding;
		u2.view({x, y, x + w - pw, y3});
		x = x1; y = y3 + 2;
		auto attacker_strenght = s2.getstrenght(0);
		char temp[4096]; zprint(temp, msg.total_strenght, attacker_strenght);
		auto defender_strenght = a3.getstrenght(0);
		szprint(zend(temp), zendof(temp), " ");
		if(attacker_strenght < defender_strenght)
			szprint(zend(temp), zendof(temp), msg.predict_fail);
		else if(attacker_strenght <= defender_strenght + 1)
			szprint(zend(temp), zendof(temp), msg.predict_partial);
		else
			szprint(zend(temp), zendof(temp), msg.predict_success);
		y += textf(x, y, gui.window_width, temp);
		domodal();
		control_standart();
		if(hot.key == u1.id) {
			auto p1 = u1.getcurrent();
			u1.source.remove(u1.current);
			u2.source.add(p1);
		} else if(hot.key == u2.id) {
			auto p1 = u2.getcurrent();
			u2.source.remove(u2.current);
			u1.source.add(p1);
		}
	}
	return getresult() != 0;
}

static void choose_action() {
	hero_info* hero = (hero_info*)hot.param;
	if(!hero)
		return;
	auto action = getaction(current_player, hero);
	if(!action)
		return;
	province_info* province = 0;
	if(action->isplaceable()) {
		auto choose_mode = action->getprovince();
		province_info* provinces[128];
		auto count = current_player->getprovinces(provinces, sizeof(provinces) / sizeof(provinces[0]), current_player, choose_mode);
		province = getprovince(current_player, hero, action, {provinces, count}, getcolor(choose_mode));
		if(!province)
			return;
	}
	if(action->raid || action->attack) {
		auto raid = action->raid > 0;
		army a1(current_player, province, hero, true, raid); a1.fill(current_player, 0);
		army a2(current_player, province, hero, true, raid);
		army a3(0, 0, 0, false, raid); a3.fill(province->getplayer(), province);
		if(!move(current_player, hero, action, province, a1, a2, a3))
			return;
		for(auto p : a2)
			p->setmove(province);
	} else if(action->recruit) {
		unit_set a1; a1.fill(current_player, province, hero, action);
		unit_set a2;
		if(!recruit(current_player, hero, action, province, a1, a2))
			return;
	}
	hero->setaction(action);
	hero->setprovince(province);
	if(action->wait)
		hero->setwait(hero->getwait() + action->wait);
	current_province = province;
}

static void end_turn() {
	breakmodal(0);
}

void player_info::makemove() {
	current_player = this;
	current_province = province_data.data + 0;
	while(ismodal()) {
		render_board();
		render_left_side(true);
		auto x = getwidth() - gui.hero_window_width - gui.border - gui.padding;
		auto y = render_right_side(choose_action);
		y += buttonw(x, y, gui.hero_window_width, "��������� ���", cmd(end_turn));
		//current_player, current_player, 0, choose_action, current_province) + gui.padding * 3;
		domodal();
		control_standart();
	}
}

void draw::avatar(int x, int y, const char* id) {
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
}

void draw::addaccept(char* result, const char* result_max) {
	addbutton(result, result_max, "accept");
}

void draw::addbutton(char* result, const char* result_max, const char* name) {
	szprint(zend(result), result_max, "\n$(%1)", name);
}

int	draw::buttonw(int x, int y, int width, const char* label, const runable& e, unsigned key, const char* tips) {
	return windowb(x, y, width, label, e, 0, key);
}

TEXTPLUGIN(accept) {
	if(hot.key == KeyEnter)
		execute(buttonok);
	return button(x + width - gui.button_width - gui.padding * 2, y + gui.padding + 4, gui.button_width, msg.accept, cmd(buttonok));
}

TEXTPLUGIN(yesno) {
	if(hot.key == Alpha + 'Y')
		execute(buttonok);
	else if(hot.key == Alpha + 'N')
		execute(buttoncancel);
	auto height = button(x + width - gui.button_width, y, gui.button_width, msg.yes, cmd(buttonok));
	width -= gui.button_width + gui.padding;
	button(x + width - gui.button_width, y, gui.button_width, msg.no, cmd(buttoncancel));
	return height;
}

void draw::domodal() {
	if(current_execute) {
		auto proc = current_execute;
		before_render();
		proc();
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

color draw::getcolor(province_flag_s id) {
	switch(id) {
	case NoFriendlyProvince: return colors::red;
	case FriendlyProvince: return colors::green;
	default: return colors::gray;
	}
}