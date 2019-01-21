#include "main.h"

static int compare(const void* p1, const void* p2) {
	auto e1 = (choiseset::element*)p1;
	auto e2 = (choiseset::element*)p2;
	return strcmp(e1->text, e2->text);
}

int	choiseset::getweight() const {
	auto result = 0;
	for(auto& e : elements) {
		if(e.weight>0)
			result += e.weight;
	}
	return result;
}

void choiseset::sort() {
	qsort(elements.data, elements.getcount(), sizeof(elements[0]), compare);
}

const choiseset::element* choiseset::getrandom() const {
	if(!elements.getcount())
		return 0;
	auto weight = getweight();
	if(!weight)
		return elements.data + (rand() % elements.getcount());
	auto roll = rand() % weight;
	auto total = 0;
	for(auto& e : elements) {
		if(e.weight<=0)
			continue;
		total += e.weight;
		if(roll < total)
			return &e;
	}
	return 0;
}

int choiseset::getrandomparam() const {
	auto p = getrandom();
	if(!p)
		return 0;
	return p->param;
}

choiseset::element* choiseset::addv(int param, const char* format, const char* format_param) {
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

choiseset::element* choiseset::add(int param, const char* format, ...) {
	return addv(param, format, xva_start(format));
}