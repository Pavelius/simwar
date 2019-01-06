#include "bslog.h"
#include "initializer_list.h"
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

static void test_report() {
	string sb;
	sb.set(Male);
	sb.addh("������������");
	sb.addn("��� %�� �����. ������ �� ���������.");
	sb.accept();
	draw::report(sb);
}

int main(int argc, char* argv[]) {
	if(!initialize_messages("ru"))
		return 0;
	srand(clock());
	draw::initialize();
	draw::create(-1, -1, 800, 600, WFResize|WFMinmax, 32);
	draw::setcaption(msg.title);
	if(!game.read("test"))
		return 0;
	//test_report();
	player_info::playgame();
	return 0;
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main(0, 0);
}