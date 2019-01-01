#include "main.h"

using namespace draw;

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

static bool control_focus() {
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

static void render_province(rect rc, point mouse, const player_info* owner, callback_proc proc, province_flag_s province_state) {
	char temp[1024];
	draw::state push;
	draw::fore = colors::black;
	draw::fore_stroke = colors::white;
	if(!draw::font)
		return;
	unsigned count;
	for(auto& e : province_data) {
		if(!e)
			continue;
		draw::font = metrics::h1;
		point real_pos = e.getposition();
		point pt = {(short)(real_pos.x - rc.x1 - camera.x), (short)(real_pos.y - rc.y1 - camera.y)};
		zprint(temp, e.getname());
		auto text_width = draw::textw(temp);
		auto hilite = false;
		auto a = AreaNormal;
		hero_info* hero_array[16];
		count = player_info::getheroes(hero_array, lenghtof(hero_array), &e);
		if(proc) {
			hilite = true;
			if(count > 0)
				hilite = false;
			if(hilite) {
				if(province_state && province_state != e.getstatus(owner))
					hilite = false;
			}
			if(hilite) {
				rect rc = {pt.x - text_width / 2, pt.y - draw::texth() / 2, pt.x + text_width / 2, pt.y + draw::texth() / 2};
				a = draw::area(rc);
				if(a == AreaHilitedPressed)
					draw::execute(proc, (int)&e);
			}
		}
		//render_power(pt.x, pt.y, owner, &e);
		draw::fore_stroke = colors::white;
		if(hilite)
			draw::fore_stroke = getcolor(province_state);
		if(a == AreaHilited || a == AreaHilitedPressed) {
			draw::fore_stroke = draw::fore_stroke.mix(colors::white);
			hot.cursor = CursorHand;
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
		count = player_info::gettroops(troop_array, lenghtof(troop_array), &e, 0);
		if(count) {
			troop_info::sort(troop_array, count);
			troop_info::getpresent(temp, zendof(temp), troop_array, count);
			rect rc = {0, 0, 200, 0}; draw::textw(rc, temp);
			pt.y += draw::text({pt.x - rc.width() / 2, pt.y, pt.x + rc.width() / 2, pt.y + rc.height()}, temp, AlignCenter);
		}
	}
}

static void render_frame(rect rc, const player_info* player, callback_proc proc, province_flag_s province_state = AnyProvince) {
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
	if(player)
		render_province(last_board, last_mouse, player, proc, province_state);
}

static int render_hero(int x, int y, int width, hero_info* e, bool hilite, bool disabled, const char* disable_text, callback_proc proc = 0) {
	char temp[2048]; temp[0] = 0;
	draw::state push;
	draw::font = metrics::font;
	auto pa = e->getavatar();
	int height = gui.hero_width;
	szprint(zend(temp), zendof(temp), "###%1\n", e->getname());
	for(auto p : e->getbonuses()) {
		zcat(temp, p->getname());
		zcat(temp, "\n");
	}
	auto owner = e->getplayer();
	rect rc = {x, y, x + width, y + height};
	areas hittest = window(rc, disabled, hilite);
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
		static const char* abilities[] = {"attack", "defence", "raid", "enemy_casualties", "friendly_casualties"};
		temp[0] = 0;
		// Ability block
		auto ps = zend(temp);
		auto ph = hero_info::metadata;
		for(auto text : abilities) {
			auto pf = ph->find(text);
			if(!pf)
				continue;
			auto value = pf->get(pf->ptr(e));
			if(!value)
				continue;
			pf = msg_type->find(text);
			if(!pf)
				continue;
			auto pn = (const char*)pf->get(pf->ptr(&msg));
			if(!pn)
				continue;
			if(ps[0])
				zcat(temp, "\n");
			szprint(zend(temp), zendof(temp), "%+2i %1", pn, value);
		}
		tooltips(x, y, width, temp);
		if(hittest == AreaHilitedPressed && hot.key==MouseLeft && proc)
			execute(proc, (int)e);
	}
	return height + gui.border * 2;
}

static int render_heroes(int x, int y, const player_info* player, callback_proc proc) {
	auto y0 = y;
	if(!player)
		return 0;
	for(auto& e : hero_data) {
		if(!e || e.getplayer() != player)
			continue;
		y += render_hero(x, y, gui.hero_window_width, &e, true, !e.isready(), 0, proc);
		y += gui.padding;
	}
	return y - y0;
}

static int render_board(const player_info* province_owner, player_info* hero_owner, callback_proc choose_province, callback_proc choose_hero) {
	render_frame({0, 0, draw::getwidth(), draw::getheight()}, province_owner, choose_province);
	auto x = getwidth() - gui.hero_window_width - gui.border - gui.padding;
	auto y = gui.padding + gui.border;
	return render_heroes(x, y, hero_owner, choose_hero);
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
	auto rs = draw::area(rc);
	auto op = gui.opacity;
	if(disabled)
		op = op / 2;
	else if(hilight && !disabled && (rs == AreaHilited || rs == AreaHilitedPressed))
		op = gui.opacity_hilighted;
	draw::rectf(rc, c, op);
	draw::rectb(rc, c);
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

int draw::windowb(int x, int y, int width, const char* string, const runable& e, int border, unsigned key) {
	draw::state push;
	draw::font = metrics::font;
	rect rc = {x, y, x + width, y + draw::texth()};
	auto ra = window(rc, false, !e.isdisabled(), border);
	draw::text(rc, string, AlignCenterCenter);
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
	colors::form = colors::black;
	colors::h1 = colors::white.mix(colors::form, 128);
	colors::h2 = colors::white.mix(colors::form, 144);
	colors::h3 = colors::white.mix(colors::form, 192);
	colors::text = colors::white;
	draw::font = metrics::font;
	draw::fore = colors::text;
	draw::fore_stroke = colors::blue;
}

bool draw::initializemap() {
	if(!game.map || !game.map[0])
		return false;
	char temp[260];
	if(!map.read(szurl(temp, "maps", game.map)))
		return false;
	return true;
}

//void draw::render(draw::controls::control& element, rect rc) {
//	while(ismodal()) {
//		render_board(current_player, current_player, 0, 0);
//		element.view(rc);
//		auto id = input();
//		if(control_board(id))
//			continue;
//	}
//}

void draw::report(const char* format) {
	while(ismodal()) {
		render_board(current_player, current_player, 0, 0);
		draw::window(gui.border * 2, gui.border * 2, gui.window_width, format);
		domodal();
		control_standart();
	}
}

static void breakparam() {
	breakmodal(hot.param);
}

action_info* draw::getaction(player_info* player, hero_info* hero) {
	while(ismodal()) {
		render_frame({0, 0, draw::getwidth(), draw::getheight()}, player, 0);
		auto x = getwidth() - gui.hero_window_width - gui.border - gui.padding;
		auto y = gui.padding + gui.border;
		y += render_hero(x, y, gui.hero_window_width, hero, false, !hero->isready(), 0) + 1;
		for(auto& e : action_data)
			y += windowb(x, y, gui.hero_window_width, e.getname(), cmd(breakparam, (int)&e), gui.border) + 1;
		y += windowb(x, y, gui.hero_window_width, msg.cancel, cmd(breakparam, 0)) + 1;
		domodal();
		control_standart();
	}
	return (action_info*)getresult();
}

province_info* draw::getprovince(player_info* player, hero_info* hero, action_info* action) {
	while(ismodal()) {
		rect rc = {0, 0, draw::getwidth(), draw::getheight()};
		render_frame(rc, player, breakparam, action->getprovince());
		auto x = getwidth() - gui.hero_window_width - gui.border - gui.padding;
		auto y = gui.padding + gui.border;
		y += render_hero(x, y, gui.hero_window_width, hero, false, !hero->isready(), 0) + 1;
		y += windowb(x, y, gui.hero_window_width, action->getname(), cmd(), gui.border) + 1;
		y += windowb(x, y, gui.hero_window_width, msg.cancel, cmd(breakparam, 0)) + 1;
		domodal();
		control_standart();
	}
	return (province_info*)getresult();
}

static void choose_action() {
	hero_info* hero = (hero_info*)hot.param;
	if(!hero)
		return;
	province_info* province = 0;
	auto action = getaction(current_player, hero);
	if(!action)
		return;
	if(action->placeable) {
		auto choose_mode = action->getprovince();
		province = getprovince(current_player, hero, action);
		if(!province)
			return;
	}
	hero->setaction(action);
	hero->setprovince(province);
}

static void end_turn() {
	breakmodal(0);
}

void player_info::makemove() {
	current_player = this;
	while(ismodal()) {
		auto x = getwidth() - gui.hero_window_width - gui.border - gui.padding;
		auto y = render_board(current_player, current_player, 0, choose_action) + gui.padding * 3;
		button(x, y, gui.hero_window_width, "��������� ���", cmd(end_turn));
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

int	draw::button(int x, int y, int width, const char* label, const runable& e, unsigned key) {
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
	case HostileProvince: return colors::red;
	case FriendlyProvince: return colors::green;
	default: return colors::gray;
	}
}