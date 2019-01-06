#include "main.h"

void answer_info::addv(int param, const char* format, const char* format_param) {
	stringcreator sc;
	if(elements.getcount() >= elements.getmaximum())
		return;
	if(p >= zendof(buffer))
		return;
	if(p > buffer)
		*p++ = 0;
	auto pe = elements.add();
	pe->param = param;
	pe->text = p;
	p = sc.printv(p, zendof(buffer), format, format_param);
}

void answer_info::add(int param, const char* format, ...) {
	addv(param, format, xva_start(format));
}