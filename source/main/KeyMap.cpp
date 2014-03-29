#include "JSFramework.h"

unsigned JSApp::InputToInt(std::string key) {
	if (key=="KEY_A") return 'A';
	if (key=="KEY_B") return 'B';
	if (key=="KEY_C") return 'C';
	if (key=="KEY_D") return 'D';
	if (key=="KEY_E") return 'E';
	if (key=="KEY_F") return 'F';
	if (key=="KEY_G") return 'G';
	if (key=="KEY_H") return 'H';
	if (key=="KEY_I") return 'I';
	if (key=="KEY_J") return 'J';
	if (key=="KEY_K") return 'K';
	if (key=="KEY_L") return 'L';
	if (key=="KEY_M") return 'M';
	if (key=="KEY_N") return 'N';
	if (key=="NUM_O") return 'O';
	if (key=="KEY_P") return 'P';
	if (key=="KEY_Q") return 'Q';
	if (key=="KEY_R") return 'R';
	if (key=="KEY_S") return 'S';
	if (key=="KEY_T") return 'T';
	if (key=="KEY_U") return 'U';
	if (key=="KEY_V") return 'V';
	if (key=="NUM_W") return 'W';
	if (key=="KEY_X") return 'X';
	if (key=="KEY_Y") return 'Y';
	if (key=="KEY_Z") return 'Z';

	if (key=="KEY_^") return '^';
	if (key=="KEY_~") return '~';
	if (key=="KEY_0") return '0';
	if (key=="KEY_1") return '1';
	if (key=="KEY_2") return '2';
	if (key=="KEY_3") return '3';
	if (key=="KEY_4") return '4';
	if (key=="KEY_5") return '5';
	if (key=="KEY_6") return '6';
	if (key=="KEY_7") return '7';
	if (key=="KEY_8") return '8';
	if (key=="KEY_9") return '9';

	if (key=="KEY_.") return '.';
	if (key=="KEY_,") return ',';
	if (key=="KEY_-") return '-';

	if (key=="NUM_0") return GLFW_KEY_KP_0;
	if (key=="NUM_1") return GLFW_KEY_KP_1;
	if (key=="NUM_2") return GLFW_KEY_KP_2;
	if (key=="NUM_3") return GLFW_KEY_KP_3;
	if (key=="NUM_4") return GLFW_KEY_KP_4;
	if (key=="NUM_5") return GLFW_KEY_KP_5;
	if (key=="NUM_6") return GLFW_KEY_KP_6;
	if (key=="NUM_7") return GLFW_KEY_KP_7;
	if (key=="NUM_8") return GLFW_KEY_KP_8;
	if (key=="NUM_9") return GLFW_KEY_KP_9;

	if (key=="NUM_/") return GLFW_KEY_KP_DIVIDE;
	if (key=="NUM_*") return GLFW_KEY_KP_MULTIPLY;
	if (key=="NUM_-") return GLFW_KEY_KP_SUBTRACT;
	if (key=="NUM_+") return GLFW_KEY_KP_ADD;
	if (key=="NUM_.") return GLFW_KEY_KP_DECIMAL;
	if (key=="NUM_=") return GLFW_KEY_KP_EQUAL;
	if (key=="NUM_ENTER") return GLFW_KEY_KP_ENTER;

	if (key=="LEFT_SHIFT") return GLFW_KEY_LSHIFT;
	if (key=="RIGHT_SHIFT") return GLFW_KEY_RSHIFT;
	if (key=="LEFT_CTRL") return GLFW_KEY_LCTRL;
	if (key=="RIGHT_CTRL") return GLFW_KEY_RCTRL;
	if (key=="LEFT_ALT") return GLFW_KEY_LALT;
	if (key=="RIGHT_ALT") return GLFW_KEY_RALT;
	if (key=="TAB") return GLFW_KEY_TAB;
	if (key=="ENTER") return GLFW_KEY_ENTER;

	if (key=="CURSOR_UP") return GLFW_KEY_UP;
	if (key=="CURSOR_DOWN") return GLFW_KEY_DOWN;
	if (key=="CURSOR_LEFT") return GLFW_KEY_LEFT;
	if (key=="CURSOR_RIGHT") return GLFW_KEY_RIGHT;

	if (key=="BACKSPACE") return GLFW_KEY_BACKSPACE;
	if (key=="DELETE") return GLFW_KEY_DEL;
	if (key=="INSERT") return GLFW_KEY_INSERT;
	if (key=="HOME") return GLFW_KEY_HOME;
	if (key=="END") return GLFW_KEY_END;
	if (key=="PAGEUP") return GLFW_KEY_PAGEUP;
	if (key=="PAGEDOWN") return GLFW_KEY_PAGEDOWN;
	if (key=="ESCAPE") return GLFW_KEY_ESC;
	if (key=="SPACE") return GLFW_KEY_SPACE;

	if (key=="MOUSE_LEFT") return 350;
	if (key=="MOUSE_RIGHT") return 351;
	if (key=="MOUSE_MIDDLE") return 352;

	if (key=="MOUSE_X") return 0;
	if (key=="MOUSE_Y") return 1;
	if (key=="MOUSE_Z") return 2;
	return 0;
}