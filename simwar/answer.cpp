#include "main.h"

static int compare(const void* p1, const void* p2) {
	auto e1 = (answer_info::element*)p1;
	auto e2 = (answer_info::element*)p2;
	return strcmp(e1->text, e2->text);
}

int	answer_info::getweight() const {
	auto result = 0;
	for(auto& e : elements)
		result += e.weight;
	return result;
}

void answer_info::sort() {
	qsort(elements.data, elements.getcount(), sizeof(elements[0]), compare);
}

const answer_info::element* answer_info::getrandom() const {
	if(!elements.getcount())
		return 0;
	auto weight = getweight();
	if(!weight)
		return elements.data + (rand() % elements.getcount());
	auto roll = rand() % weight;
	auto total = 0;
	for(auto& e : elements) {
		if(!e.weight)
			continue;
		total += e.weight;
		if(roll < total)
			return &e;
	}
	return 0;
}

int answer_info::getrandomparam() const {
	auto p = getrandom();
	if(!p)
		return 0;
	return p->param;
}

answer_info::element* answer_info::addv(int param, const char* format, const char* format_param) {
	if(elements.getcount() >= elements.getmaximum())
		return 0;
	if(get() >= end())
		return 0;
	if(get() > begin())
		addsz();
	auto pe = elements.add();
	pe->param = param;
	pe->text = get();
	pe->weight = 0;
	string::addv(format, format_param);
	return pe;
}

answer_info::element* answer_info::add(int param, const char* format, ...) {
	return addv(param, format, xva_start(format));
}