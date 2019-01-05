#include "draw_control.h"

void draw::controls::control::mouseinput(unsigned id, point position) {
	switch(id) {
	case MouseLeft:
		setfocus((int)this, true);
		break;
	}
}