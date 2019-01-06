#include "bslog.h"
#include "initializer_list.h"
#include "main.h"

static bool initialize_messages() {
	static const char* prefixs[] = {"", "s", "s_of", "_negative", 0};
	static const char* requisits[] = {"name", "nameof", "nameact", "text", 0};
	static const char* skip_name[] = {"gui", "game", 0};
	auto result = true;
	auto url_errors = "errors.txt";
	io::file::remove(url_errors);
	if(true) {
		bslog errors(url_errors);
		bsdata::readl("text", "ru", errors, prefixs, requisits, skip_name);
	}
	return !io::file::exist(url_errors);
}

int main(int argc, char* argv[]) {
	if(!initialize_messages())
		return 0;
	srand(clock());
	draw::initialize();
	draw::create(-1, -1, 800, 600, WFResize|WFMinmax, 32);
	draw::setcaption(msg.title);
	if(!game.read("test"))
		return 0;
	player_info::playgame();
	return 0;
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main(0, 0);
}