#include "bsdata.h"
#include "draw.h"

#pragma once

namespace draw {
enum column_size_s : unsigned char {
	SizeDefault,
	SizeResized, SizeFixed, SizeInner, SizeAuto,
};
enum select_mode_s : unsigned char {
	SelectCell, SelectText, SelectRow,
};
struct runable {
	virtual void			execute() const = 0;
	virtual int				getid() const = 0;
	virtual bool			isdisabled() const = 0;
};
struct cmd : runable {
	constexpr cmd() : proc(0), param(0), disabled(false) {}
	constexpr cmd(callback_proc proc, int param = 0, bool disabled = false) : proc(proc), param(param), disabled(disabled) {}
	explicit operator bool() const { return proc != 0; }
	void					execute() const override { draw::execute(proc, param); }
	int						getid() const override { return (int)proc; }
	bool					isdisabled() const { return disabled; }
private:
	callback_proc			proc;
	int						param;
	bool					disabled;
};
namespace controls {
struct control {
	bool					show_border;
	constexpr control() : show_border(true) {}
	virtual ~control() {}
	virtual const char*		getlabel(char* result, const char* result_maximum) const { return 0; }
	virtual bool			isdisabled() const { return false; }
	virtual bool			isfocusable() const { return true; }
	bool					isfocused() const;
	bool					ishilited() const;
	virtual bool			keyinput(unsigned id) { return false; }
	virtual void			mouseinput(unsigned id, point mouse); // Default behaivor set focus
	virtual void			mousewheel(unsigned id, point mouse, int value) {}
	virtual void			redraw() {}
	virtual void			view(const rect& rc);
};
// Analog of listbox element
struct list : control {
	int						origin, current, current_hilite, origin_width;
	int						lines_per_page, pixels_per_line, pixels_per_width;
	bool					show_grid_lines, show_selection, show_header;
	bool					hilite_odd_lines;
	rect					current_rect, view_rect;
	constexpr list() : origin(0), current(0), current_hilite(-1), origin_width(0),
		lines_per_page(0), pixels_per_line(0), pixels_per_width(0),
		show_grid_lines(false), show_selection(true), show_header(true), hilite_odd_lines(true),
		current_rect(), view_rect() {}
	void					correction();
	void					correction_width();
	virtual void			ensurevisible(); // Ånsure that current selected item was visible on screen if current 'count' is count of items per line
	int						find(int line, int column, const char* name, int lenght = -1) const;
	virtual int				getcolumn() const { return 0; } // Get current column
	virtual int				getident() const { return 6 * 2 + 4; } // get row ident in levels
	virtual int				getlevel(int index) const { return 0; } // get row ident in levels
	inline int				getline() const { return current; } // get current line
	int						getlinesperpage(int height) const { return height / pixels_per_line; }
	virtual const char*		getname(char* result, const char* result_max, int line, int column) const { return 0; }
	virtual int				getmaximum() const { return 0; }
	virtual int				getmaximumwidth() const { return 0; }
	static int				getrowheight(); // Get default row height for any List Control
	virtual int				getwidth(int column) const { return 0; } // Get column width
	void					hilight(const rect& rc, const rect* prc = 0) const;
	virtual bool			isgroup(int index) const { return false; }
	bool					isopen(int index) const;
	bool					keyinput(unsigned id) override;
	void					mousehiliting(const rect& rc, point mouse);
	virtual void			mouseinput(unsigned id, point position) override;
	virtual void			mouseselect(int id, bool pressed);
	virtual void			mousewheel(unsigned id, point position, int step) override;
	void					redraw() override;
	virtual void			row(const rect& rc, int index);
	virtual int				rowheader(const rect& rc) const { return 0; }
	virtual void			rowhilite(const rect& rc, int index) const;
	virtual void			select(int index, int column);
	void					treemark(rect rc, int index, int level) const;
	virtual bool			treemarking(bool run) { return true; }
	void					view(const rect& rc) override;
};
}
}