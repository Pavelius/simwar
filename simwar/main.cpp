#include "bslog.h"
#include "draw_control.h"
#include "main.h"

static bool initialize_messages(const char* locale_name) {
	static const char* prefixs[] = {"", "s", "s_of", "_negative", 0};
	static const char* skip_name[] = {"gui", "game", 0};
	auto url_errors = "errors.txt";
	io::file::remove(url_errors);
	if(true) {
		bslog errors(url_errors);
		bsdata::readl("text", locale_name, errors, prefixs, key_requisits, skip_name);
	}
	return !io::file::exist(url_errors);
}

int main(int argc, char* argv[]) {
	gui.initialize();
	if(!initialize_messages("ru"))
		return 0;
	srand(clock());
	draw::initialize();
	draw::create(-1, -1, 800, 600, WFResize|WFMinmax, 32);
	draw::setcaption(msg.title);
	menu_info::choose_block("main_menu");
	return 0;
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main(0, 0);
}