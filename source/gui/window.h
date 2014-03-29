#ifndef WINDOW_INCLUDED
#define WINDOW_INCLUDED

#include <list>
#include <functional>

#include "guivisitor.h"
#include "../tools/property.h"
#include "main/eventmanager.h"
#include "main/defines.h"

class Item;
class Furnace;

class GuiElement : public Visited
{
public:
        GuiElement() :  x(0), y(0), w(0), h(0), texture(0),
                        u0(0), v0(0), u1(1.f), v1(1.f),
                        parent(0), visible(true) {}

        GuiElement(GuiElement& parent, int x, int y, int w, int h) :
            x(x), y(y), w(w), h(h),  texture(-1),
            u0(0), v0(0), u1(1.f), v1(1.f),
            parent(&parent), visible(true)
	{
            parent.children.push_front(this);
	}

	virtual ~GuiElement();

	int x, y, w, h;
	int texture;
	float u0, v0, u1, v1;

	GuiElement* parent;
	std::list<GuiElement*> children;

	void setPosition(int px, int py) { x = px; y = py; }
	void setSize(int width, int height) { w = width; h = height; }
	void setWidth(int width) { w = width; }
	void setHeight(int height) { h = height; }

	void bringToFront();
	void update(float dt);
	void setTexture(int texture) { this->texture = texture; }
	void setTexture(int texture, float u0, float v0, float u1, float v1);
	
	virtual GuiElement* getElementAt(int px, int py);
	virtual void invite(GuiVisitor& v) { v.visit(*this); }

	Property<bool> visible;
	
	virtual bool startDrag() { return false; }
	virtual void onUpdate(float) {}
	virtual void onEndDrag() {}
	virtual void onDrag(int, int) {}
	virtual void onDrop(GuiElement&) {}
	virtual void onClick() {}
	virtual void onMouseDown(int) {}
	virtual void onMouseUp(int) {}
	virtual void onMouseEnter() {}
	virtual void onMouseLeave() {}
	virtual void onKey(int, int) {}
};



class Tooltip : public GuiElement
{
public:
	Tooltip(GuiElement& parent, const std::string& text);

	virtual void onUpdate(float) { bringToFront(); }
	virtual void invite(GuiVisitor& v) { v.visit(*this); }

	Property<std::string> text;
};


class Container : public GuiElement
{
public:
	explicit Container(GuiElement& parent);

	virtual GuiElement* getElementAt(int, int);
};


class Button : public GuiElement
{
public:
	Button(GuiElement& parent, int x, int y, int w, int h);

	std::function<void()> sigClick;
	virtual void onMouseEnter() { highlighted = true; }
	virtual void onMouseLeave() { highlighted = false; }
	virtual void onClick() { sigClick(); }
	virtual void invite(GuiVisitor& v) { v.visit(*this); }

	bool highlighted;
};


class ProgressBar : public GuiElement
{
public:
	ProgressBar(GuiElement& parent, int x, int y, int w, int h, bool vertical = false);

	bool vertical;
	int percent;

	virtual void invite(GuiVisitor& v) { v.visit(*this); }
};


class HandSlot : public GuiElement
{
public:
	HandSlot(GuiElement& parent, int x, int y, Item* item);

	Item* item;

	virtual GuiElement* getElementAt(int, int) { return 0; }
	virtual void onUpdate(float) { bringToFront(); }
	virtual void invite(GuiVisitor& v) { v.visit(*this); }
};

class Slot : public GuiElement
{
public:
	Slot(GuiElement& parent, int x, int y, Item* item, int bg = -1);

	Item* item;
	int background;
	bool highlighted;

	virtual void invite(GuiVisitor& v) { v.visit(*this); }

	virtual void onMouseEnter();
	virtual void onMouseLeave();
	virtual void onMouseDown(int btn);

	virtual bool doesAccept(const Item&) { return true; }
	virtual void takeItems(unsigned char) {}

	static HandSlot* handSlot;
};



class ArmorSlot : public Slot
{
public:
	ArmorSlot(GuiElement& parent, int x, int y, Item* item, int bg, ItemCategory accepts);

	virtual bool doesAccept(const Item& item);
	ItemCategory acceptedCategory;
};



class CraftingSlot : public Slot
{
public:
	CraftingSlot(GuiElement& parent, int x, int y, Item* item, int bg = -1);

	void addInput(Slot* slot) { input.push_back(slot); }
	virtual bool doesAccept(const Item&) { return false; }
	virtual void removeUsedItems(unsigned char amount);
	virtual void onMouseDown(int btn);

	std::vector<Slot*> input;
};



class Window : public GuiElement
{
public:
	Window(GuiElement& parent, int x, int y, int w, int h );

	virtual void invite(GuiVisitor& v) { v.visit(*this); }

	virtual bool startDrag() { return true; }
	virtual void onDrag(int dx, int dy);

	void close() { onClose(); visible(false); }

	virtual void onClose();
};


struct EventWindowClose : public Event
{
	explicit EventWindowClose(Window* w) : window(w) {}
	Window* window;
};


class InventoryWindow : public Window
{
public:
	InventoryWindow(GuiElement& parent, int x, int y, int w, int h );

	void setInventory( std::vector<Item>& inventory );	

	virtual void invite(GuiVisitor& v) { v.visit(*this); }
};



class WorkbenchWindow : public Window
{
public:
	WorkbenchWindow(GuiElement& parent, int x, int y, int w, int h);

	void setInventory( std::vector<Item>& inventory );	

	virtual void invite(GuiVisitor& v) { v.visit(*this); }
};



class FurnaceWindow : public Window
{
public:
	FurnaceWindow(GuiElement& parent, int x, int y, int w, int h, Furnace& furnace );

	virtual void invite(GuiVisitor& v) { v.visit(*this); }
	virtual void onUpdate(float dt);

	Furnace* furnace;
	ProgressBar *cookProgress, *burnProgress;
};



class ChestWindow : public Window
{
public:
	ChestWindow(GuiElement& parent, int x, int y, int w, int h );

	void setInventory( std::vector<Item>& inventory );	

	virtual void invite(GuiVisitor& v) { v.visit(*this); }
};



class QuickSlotBar : public Window
{
public:
	QuickSlotBar(GuiElement& parent, int x, int y, int w, int h );

	void setInventory( std::vector<Item>& inventory );	

	virtual void invite(GuiVisitor& v) { v.visit(*this); }
	virtual bool startDrag() { return false; }

	int selection;
};

#endif
