#include "main.h"

static int compare(const void* p1, const void* p2) {
	auto e1 = (answer_info::element*)p1;
	auto e2 = (answer_info::element*)p2;
	return strcmp(e1->text, e2->text);
}

void answer_info::sort() {
	qsort(elements.data, elements.getcount(), sizeof(elements[0]), compare);
}

void answer_info::addv(int param, const char* format, const char* format_param) {
	if(elements.getcount() >= elements.getmaximum())
		return;
	if(get() >= end())
		return;
	if(get() > begin())
		addsz();
	auto pe = elements.add();
	pe->param = param;
	pe->text = get();
	string::addv(format, format_param);
}

void answer_info::add(int param, const char* format, ...) {
	addv(param, format, xva_start(format));
}