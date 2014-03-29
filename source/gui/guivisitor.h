#ifndef GUI_VISITOR_INCLUDED
#define GUI_VISITOR_INCLUDED

class GuiElement;
class Window;
class Button;
class ProgressBar;
class QuickSlotBar;
class InventoryWindow;
class WorkbenchWindow;
class FurnaceWindow;
class ChestWindow;
class Slot;
class HandSlot;
class Tooltip;

class GuiVisitor
{
public:
	virtual void visit(GuiElement&) {}
	virtual void visit(Tooltip&) {}
	virtual void visit(Button&) {}
	virtual void visit(Window&) {}
	virtual void visit(ProgressBar&) {}
	virtual void visit(QuickSlotBar&) {}
	virtual void visit(HandSlot&) {}
	virtual void visit(Slot&) {}
};

class Visited
{
public:
	virtual void invite(GuiVisitor& v) = 0;
};

#endif