#ifndef EVENT_MANAGER_INCLUDED
#define EVENT_MANAGER_INCLUDED

#include <string>
#include <map>
#include <list>
#include <typeinfo>
#include <queue>

class TypeInfo
{
public:
	explicit TypeInfo(const type_info& info) : _typeInfo(&info) {}

	bool operator < (const TypeInfo& rhs) const	{ return _typeInfo < rhs._typeInfo; }

private:
	const type_info* _typeInfo;
};

class Event
{
public:
	Event() {}
	virtual ~Event() {}
};


class HandlerFunction
{
public:
	virtual ~HandlerFunction() {}
	void exec(const Event& event) {call(event);}

private:
	virtual void call(const Event&) = 0;
};

template < class T, class EventT >
class MemberFunctionHandler : public HandlerFunction
{
public:
	typedef void (T::*MemberFunc)(const EventT&);
	MemberFunctionHandler(T* instance, MemberFunc memFn) : _instance(instance), _function(memFn) {}

	void call(const Event& event) { (_instance->*_function)(static_cast< const EventT& >(event)); }

private:
  T* _instance;
  MemberFunc _function;
};

template < class EventT >
class StaticFunctionHandler : public HandlerFunction
{
public:
	typedef void (*Func)(const EventT&);
	explicit StaticFunctionHandler(Func Fn) : _function(Fn) {}

	void call(const Event& event) { _function(static_cast< const EventT& >(event)); }

private:
  Func _function;
};


class EventManager
{
	typedef std::list< HandlerFunction* > HandlerList;
	typedef std::map< const TypeInfo, HandlerList > HandlerMap;

public:
	~EventManager()
	{
		for(auto& l : handlers)
		for(auto& f : l.second)
			delete f;

		while (!events.empty())
		{
			delete events.front();
			events.pop();
		}

		handlers.clear();
	}

	void queueEvent(Event* event)
	{
		events.push(event);
	}

	void dispatchEvent(const Event& event)
	{
		HandlerMap::iterator it = handlers.find(TypeInfo(typeid(event)));
		if( it != handlers.end() )
		{
			for (HandlerFunction* f : it->second)
				f->exec(event);
		}
	}

	void processEvents()
	{
		while (!events.empty())
		{
			Event* evt = events.front();
			events.pop();
			
			dispatchEvent(*evt);
			delete evt;
		}
	}

	template < class D, class B, class EventT >
	void registerHandler(D* obj, void(B::*memFn)(const EventT&))
	{
		handlers[TypeInfo(typeid(EventT))].push_back( new MemberFunctionHandler< B, EventT >(obj, memFn) );
	}

	template < class EventT >
	void registerHandler(void(*Fn)(const EventT&))
	{
		handlers[TypeInfo(typeid(EventT))].push_back( new StaticFunctionHandler< EventT >(Fn) );
	}

private:
	HandlerMap handlers;
	std::queue<Event*> events;
};

#endif
