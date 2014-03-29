#include "gui.h"
#include "window.h"
#include "main/item.h"
#include "main/tileentity.h"
#include "main/defines.h"

#include "tools/templatetools.h"

using namespace std;

extern EventManager& eventManager();

GuiElement::~GuiElement()
{
	while (!children.empty())
		delete children.front();

	if (parent)
		parent->children.remove(this);

	if (this == Gui::elementUnderMouse)	Gui::elementUnderMouse = 0;
	if (this == Gui::dragged) Gui::dragged = 0;
	if (this == Gui::focused) Gui::focused = 0;
	if (this == Gui::clickedOn[0]) Gui::clickedOn[0] = 0;
	if (this == Gui::clickedOn[1]) Gui::clickedOn[1] = 0;
	if (this == Gui::clickedOn[2]) Gui::clickedOn[2] = 0;
}

GuiElement* GuiElement::getElementAt(int px, int py)
{
	GuiElement* element = 0;

	if ( visible() )
	{
		px -= x; py -= y;
		if (px >= 0 && px < w && py >= 0 && py <= h)
		{
			for (GuiElement* child : children)
			{
				element = child->getElementAt(px, py);
				if (element)
					break;
			}
			if (!element)
				element = this;
		}
	}

	return element;
}

void GuiElement::update(float dt)
{
	list<GuiElement*>::iterator it = children.begin();
	while (it != children.end())
	{
		list<GuiElement*>::iterator tmp = it++;
		(*tmp)->update(dt);
	}

	onUpdate(dt);
}

void GuiElement::bringToFront()
{
	if (parent)
	{
		parent->children.remove(this);
		parent->children.push_front(this);
	}
}

void GuiElement::setTexture(int texture, float u0, float v0, float u1, float v1)
{
	this->texture = texture;
	this->u0 = u0;
	this->v0 = v0;
	this->u1 = u1;
	this->v1 = v1;
}


Button::Button(GuiElement& parent, int x, int y, int w, int h)
	:GuiElement(parent, x, y, w, h), highlighted(false) {}


Tooltip::Tooltip(GuiElement& parent, const string& text)
	: GuiElement(parent, 0, 0, 25, 25), text(text) {}



Container::Container(GuiElement& parent)
	: GuiElement(parent, parent.x, parent.y, parent.w, parent.h) {}



GuiElement* Container::getElementAt(int px, int py)
{
	GuiElement* element = GuiElement::getElementAt(px, py);
	return (element == this) ? 0 : element;
}


ProgressBar::ProgressBar(GuiElement& parent, int x, int y, int w, int h, bool vertical)
	:GuiElement(parent, x,y,w,h), vertical(vertical), percent(0) {}


HandSlot::HandSlot(GuiElement& parent, int x, int y, Item* item) 
	: GuiElement(parent, x,y,32,32), item(item) {}


HandSlot* Slot::handSlot = 0;

Slot::Slot(GuiElement& parent, int x, int y, Item* item, int bg) 
	: GuiElement(parent, x,y,32,32), item(item), background(bg), highlighted(false) {}


void Slot::onMouseEnter()
{ 
	highlighted = true;
	if (item->id && !Slot::handSlot->item->id)
		Gui::setTooltip( getItemName(item->id) );
}

void Slot::onMouseLeave() 
{
	highlighted = false;
	Gui::clearTooltip();
}

void Slot::onMouseDown(int btn)
{
	if (btn == 0 || btn == 1)
	{
		if ( handSlot->item->id == 0 )
		{
			const unsigned char amount = (btn == 0) ? item->count : (item->count+1)/2;
		
			*handSlot->item = *item;
			handSlot->item->count = 0;

			handSlot->item->add( item->remove(amount) );
			takeItems(amount);

			Gui::clearTooltip();
		}

		else if ( doesAccept(*handSlot->item) )
		{
			const unsigned char amount = (btn==0) ? handSlot->item->count : 1;

			if ( item->id == 0 )
			{
				*item = *handSlot->item;
				item->count = 0;
			}

			if ( handSlot->item->id == item->id )
				handSlot->item->remove( item->add(amount) );

			else
				swap(*item, *handSlot->item);

			Gui::setTooltip( getItemName(item->id) );
		}
	}
}



ArmorSlot::ArmorSlot(GuiElement& parent, int x, int y, Item* item, int bg, ItemCategory accepts)
	: Slot(parent, x, y, item, bg), acceptedCategory(accepts) {}

bool ArmorSlot::doesAccept(const Item& item)
{
	const ItemInfo& info = ItemInfo::get(item.id);
	return info.category == acceptedCategory;
}



CraftingSlot::CraftingSlot(GuiElement& parent, int x, int y, Item* item, int bg) 
	: Slot(parent, x, y, item, bg) {}

void CraftingSlot::removeUsedItems(unsigned char amount)
{
	for (Slot* s : input)
		s->item->remove(amount);
}

void CraftingSlot::onMouseDown(int btn)
{
	if (btn == 0)
	{
		if ( handSlot->item->id == 0 || handSlot->item->id == item->id )
		{
			if ( handSlot->item->count + item->count <= ItemInfo(item->id).maxStackSize )
			{
				if ( handSlot->item->id == 0 )
					swap(*handSlot->item, *item);
				else
					item->remove( handSlot->item->add(item->count) );

				removeUsedItems(1);
				Gui::clearTooltip();
			}
		}
	}
}


Window::Window(GuiElement& parent, int x, int y, int w, int h ) 
	: GuiElement(parent, x,y,w,h) 
{
	Button* closeBtn = new Button(*this, w - 45, 4, 29, 18);
	closeBtn->sigClick = bind(&Window::close, this);
}

void Window::onClose()
{
	eventManager().queueEvent( new EventWindowClose(this) );
}


void Window::onDrag(int dx, int dy)
{
	x += dx;
	y += dy;
	x = max(x, 25 - w);
	y = max(y, 25 - h);
	x = min(x, parent->w - 25);
	y = min(y, parent->h - 25);
}



InventoryWindow::InventoryWindow(GuiElement& parent, int x, int y, int w, int h ) 
	: Window(parent, x,y,w,h) {}


void InventoryWindow::setInventory( vector<Item>& inventory )
{
	for (int i=0; i<4; ++i)
		new ArmorSlot(*this, 16, 16 + i*36, &inventory[103-i], 15 + i*16, ItemCategory(1+i));

	for (int i=0; i<27; ++i)
		new Slot(*this, 16 + (i%9)*36, 168 + (i/9)*36, &inventory[9+i]);

	CraftingSlot* craftingSlot = new CraftingSlot(*this, 288, 72, &inventory[99]);

	for (int i=0; i<4; ++i)
	{
		Slot* slot = new Slot(*this, 176 + (i%2)*36, 52 + (i/2)*36, &inventory[80+i]);
		craftingSlot->addInput(slot);
	}
}



WorkbenchWindow::WorkbenchWindow(GuiElement& parent, int x, int y, int w, int h ) 
	: Window(parent, x,y,w,h) {}


void WorkbenchWindow::setInventory( vector<Item>& inventory )
{
	CraftingSlot* craftingSlot = new CraftingSlot(*this, 250, 72, &inventory[9]);

	for (int i=0; i<9; ++i)
	{
		Slot* slot = new Slot(*this, 59 + (i%3)*36, 33 + (i/3)*36, &inventory[i]);
		craftingSlot->addInput(slot);
	}
}



FurnaceWindow::FurnaceWindow(GuiElement& parent, int x, int y, int w, int h, Furnace& f ) 
	: Window(parent, x,y,w,h), furnace(&f), cookProgress(0), burnProgress(0) 
{
	new Slot(*this, 112, 34, &f.items[0]);
	new Slot(*this, 112, 106, &f.items[1]);
	new CraftingSlot(*this, 232, 71, &f.items[2]);

	burnProgress = new ProgressBar(*this, 114, 74, 28, 28, true);
	cookProgress = new ProgressBar(*this, 160, 70, 43, 30, false);
}

void FurnaceWindow::onUpdate(float)
{
	burnProgress->setTexture(texture, 352/512.f, 0, 380/512.f, 28/512.f);
	cookProgress->setTexture(texture, 354/512.f, 28/512.f, 397/512.f, 58/512.f);

	cookProgress->percent = clamp( 100 - (100 * furnace->cookTime) / furnace->fullCookTime, 0, 100);
	burnProgress->percent = clamp( (100 * furnace->burnTime) / furnace->fullBurnTime, 0, 100);
}


ChestWindow::ChestWindow(GuiElement& parent, int x, int y, int w, int h ) 
	: Window(parent, x,y,w,h) {}


void ChestWindow::setInventory( vector<Item>& inventory )
{
	for (int i=0; i<27; ++i)
		new Slot(*this, 16 + (i%9)*36, 36 + (i/9)*36, &inventory[i]);
}


QuickSlotBar::QuickSlotBar(GuiElement& parent, int x, int y, int w, int h ) 
	: Window(parent, x,y,w,h), selection(0) {}


void QuickSlotBar::setInventory( vector<Item>& inventory )
{
	for (int i=0; i<9; ++i)
		new Slot(*this, 6 + i*40, 6, &inventory[i]);
}
