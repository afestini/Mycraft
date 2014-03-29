#ifndef GUI_INCLUDED
#define GUI_INCLUDED

#include <string>
#include "window.h"

class Tooltip;

class Gui
{
public:
	static GuiElement& root();
	static GuiElement& windows();
	static GuiElement* getElementAt(int px, int py);
	static GuiElement* elementUnderMouse;
	static GuiElement* dragged;
	static GuiElement* focused;
	static GuiElement* clickedOn[3];
	static Tooltip* tooltip;

	static void update(float dt);
	static void setTooltip(const std::string& txt);
	static void clearTooltip();

	static bool OnKey(int key, int state);
	static bool OnMouseBtn(int btn, int state);
	static bool OnMouseMove(int x, int y);
	static bool OnMouseWheel(int pos);

	static int mouseX, mouseY;
};

#endif