#include "bslog.h"
#include "initializer_list.h"
#include "main.h"

static bool initialize_messages(std::initializer_list<const char*> files) {
	auto result = true;
	auto url_errors = "errors.txt";
	if(true) {
		bslog log(url_errors);
		for(auto e : files) {
			char temp[260]; zprint(temp, "script/%1_%2.txt", e, "ru");
			msg_type->read(temp, &msg);
			result = result && log.check_required(temp, {&msg, msg_type});
		}
	}
	if(result)
		io::file::remove(url_errors);
	return result;
}

static bool initialize_map() {
	auto result = true;
	auto url_errors = "errors.txt";
	if(true) {
		bslog log(url_errors);
		bsdata::read("script/test.txt", &log);
		result = result && !log.iserrors();
	}
	if(result) {
		if(!draw::initializemap())
			return false;
	}
	if(result)
		io::file::remove(url_errors);
	return result;
}

int main(int argc, char* argv[]) {
	if(!initialize_messages({"msg"}))
		return 0;
	srand(clock());
	draw::initialize();
	draw::create(-1, -1, 800, 600, WFResize|WFMinmax, 32);
	draw::setcaption(msg.title);
	if(!initialize_map())
		return 0;
	player_info::maketurn();
	return 0;
}

int __stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main(0, 0);
}