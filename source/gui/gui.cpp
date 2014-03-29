#include "gui.h"
#include "window.h"
#include <GL/glfw.h>

using namespace std;

GuiElement* Gui::elementUnderMouse = 0;
GuiElement* Gui::dragged = 0;
GuiElement* Gui::focused = 0;
GuiElement* Gui::clickedOn[3] = {0,0,0};
Tooltip* Gui::tooltip = 0;
int Gui::mouseX = -1, Gui::mouseY = -1;

GuiElement& Gui::root()
{
	static GuiElement root;
	glfwGetWindowSize(&root.w, &root.h);
	return root;
}

GuiElement& Gui::windows()
{
	static Container windows(root());
	windows.x = root().x;
	windows.y = root().y;
	windows.w = root().w;
	windows.h = root().h;
	return windows;
}

GuiElement* Gui::getElementAt(int px, int py)
{
	GuiElement* element = 0;

	for (GuiElement* child : root().children)
	{
		element = child->getElementAt(px, py);
		if (element)
			break;
	}
	return element;
}

void Gui::update(float dt)
{
	root().update(dt);
}

bool Gui::OnKey(int key, int state)
{
	if (focused)
		focused->onKey(key, state);

	return focused != 0;
}

bool Gui::OnMouseBtn(int btn, int state)
{
	if (btn == 2) return false; //Ignore the middle button

	if (state)
	{
		clickedOn[btn] = elementUnderMouse;

		GuiElement* e = elementUnderMouse;
		while (e)
		{
			e->bringToFront();
			e = e->parent;
		}

		if (elementUnderMouse)
			elementUnderMouse->onMouseDown(btn);
	}
	else
	{
		if (dragged)
		{
			if (btn == 0)
			{
				dragged->onEndDrag();

				if (elementUnderMouse)
					elementUnderMouse->onDrop(*dragged);

				dragged = 0;
			}
		}
		else if (elementUnderMouse)
		{
			elementUnderMouse->onMouseUp(btn);

			if (elementUnderMouse == clickedOn[btn])
				elementUnderMouse->onClick();
		}

		clickedOn[btn] = 0;
	}
	return elementUnderMouse != 0;
}

bool Gui::OnMouseMove(int x, int y)
{
	if (mouseX < 0) mouseX = x;
	if (mouseY < 0) mouseY = y;

	GuiElement* underMouseBefore = elementUnderMouse;
	elementUnderMouse = getElementAt(x,y);

	if (elementUnderMouse != underMouseBefore)
	{
		if (underMouseBefore)
			underMouseBefore->onMouseLeave();

		if (elementUnderMouse)
			elementUnderMouse->onMouseEnter();
	}

	if (clickedOn[0] && clickedOn[0]->startDrag())
		dragged = clickedOn[0];

	if (dragged)
		dragged->onDrag(x-mouseX, y-mouseY);
	
	mouseX = x;
	mouseY = y;

	return false;
}

bool Gui::OnMouseWheel(int)
{
	return false;
}

void Gui::setTooltip(const std::string& txt)
{
	if (!tooltip)
		tooltip = new Tooltip(root(), txt);

	tooltip->visible = true;
	tooltip->text = txt;
}

void Gui::clearTooltip()
{
	if (tooltip)
		tooltip->visible = false;
}
