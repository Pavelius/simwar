#include "crt.h"
#include "draw_control.h"

using namespace draw;
using namespace draw::controls;

void control::mouseinput(unsigned id, point position) {
	switch(id) {
	case MouseLeft:
		setfocus((int)this, true);
		break;
	}
}

const visual* visual::find(const char* id) const {
	if(!this)
		return 0;
	for(auto p = this; *p; p++) {
		if(p->id[0] == '*') {
			auto pf = p->child->find(id);
			if(pf)
				return pf;
		} else if(strcmp(p->id, id) == 0)
			return p;
	}
	return 0;
}

bool control::keyinput(unsigned key) {
	return false;
}