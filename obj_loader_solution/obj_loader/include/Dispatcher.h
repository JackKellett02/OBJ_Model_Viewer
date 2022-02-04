#pragma once

#include "Event.h"
#include <map>
#include <list>
#include <functional>
#include <typeinfo>
#include <typeindex>

// Class Observer.
// An abstract base class for observers that allows concrete observers to derive from it.
class Observer
{
public:
	//Constructor.
	Observer() = default;
	//Destructor.
	virtual ~Observer() = default;
	//Copy, move and assignment operators have been disabled for this class.
	Observer(const Observer&) = delete;
	Observer(const Observer&&) = delete;
	Observer& operator=(const Observer&) = delete;

	//Function to perform a call to the abstract call function to post the param event to the observer function.
	void exec(Event* e)
	{
		call(e);
	}
	//Function to get an instance of the object/class the function pointer is bound to.
	virtual void* Instance() = 0;
private:
	//Abstract call function takes an event as a parameter to be implemented in derived classes.
	virtual void call(Event* e) = 0;
};

// Template class MemberObserver an observer that is a class with a designated observation member function.
// typename T is a pointer to the class owning the function to be called.
// typename ConcreteEvent is the type of event that is to be observed.
template<typename T, typename ConcreteEvent>
class MemberObserver : public Observer
{
public:
	//Typedefine for the pointer to the class member function.
	typedef void (T::* MemberFunction)(ConcreteEvent*);
	//Constructor.
	MemberObserver(T* a_instance, MemberFunction a_function) : m_instance(a_instance), m_memberFunction(a_function) {}
	//Destructor.
	~MemberObserver() { m_instance = nullptr; }

	//Function to return a pointer to the instance of the class the member function belongs to.
	void* Instance() { return (void*)m_instance; }
private:
	//Implementation of abstract base class function for calling observer function.
	void call(Event* e)
	{
		//cast event to correct type.
		(m_instance->*m_memberFunction)(static_cast<ConcreteEvent*>(e));
	}
private:
	//The member function in the class that we hold a pointer to.
	MemberFunction m_memberFunction;
	//The class instance to call the function on.
	T* m_instance;
};

//Template class GlobalObserver, template argument ConcreteEvent is deduced by the compiler.
template<typename ConcreteEvent>
class GlobalObserver : public Observer
{
public:
	//Typedef of function pointer to non-member function that takes a concrete event as a parameter.
	typedef void(*function)(ConcreteEvent*);
	//Constructor sets function pointer member to point to parameter function.
	GlobalObserver(function a_function) : m_function(a_function) {}
	//Destructor.
	~GlobalObserver() {}
	//Instance function implementation, global functions have no instance so returns nullptr.
	void* Instance() { return nullptr; }
private:
	//Call function will call global function member variable with Event parameter*.
	void call(Event* e)
	{
		//Cast event to correct type.
		(*m_function)(static_cast<ConcreteEvent*>(e));
	}
private:
	//Member variable pointer to global/static function.
	function m_function;
};

// Typedefine for std::list<Observer*> objects to improve code readability.
typedef std::list<Observer*> ObserverList;
// Dispatcher class, responsible for handling events and notifying any observers of a particular event.
// This could be improved to utilise a queue or ring buffer to process events at the start of the game loop.
// At present all events are handled immediately.
class Dispatcher
{
public:
	static Dispatcher* GetInstance() { return m_instance; }
	static Dispatcher* CreateInstance()
	{
		if(m_instance == nullptr)
		{
			m_instance = new Dispatcher();
		}
		return m_instance;
	}

	static void DestroyInstance()
	{
		if(m_instance)
		{
			delete m_instance;
			m_instance = nullptr;
		}
	}

	// Subscription function to subscribe observers to an event, member function pointer implementation.
	template< typename T, typename ConcreteEvent>
	void subscribe(T* a_instance, void(T::* memberFunction)(ConcreteEvent*))
	{
		// Get a list of observers from the subscribers map.
		ObserverList* observers = m_subscribers[typeid(ConcreteEvent)];
		// If observers is null there are no observers for this event yet.
		if (observers == nullptr)
		{
			// Create new list for event type and add this into the subscribers map.
			observers = new ObserverList();
			m_subscribers[typeid(ConcreteEvent)] = observers;
		}
		//Push a new member observer into the observers list from the subscribers map.
		observers->push_back(new MemberObserver<T, ConcreteEvent>(a_instance, memberFunction));
	}

	//Subscribe method for global functions to become event subscribers.
	template<typename ConcreteEvent>
	void subscribe(void(*Function)(ConcreteEvent*))
	{
		//Get a list of observers from the subscribers map.
		ObserverList* observers = m_subscribers[typeid(ConcreteEvent)];
		if (observers == nullptr)
		{
			//Create new list for event type and add this into the subscribers map.
			observers = new ObserverList();
			m_subscribers[typeid(ConcreteEvent)] = observers;
		}
		//Push a new member observer into the observers list from the subscribers map.
		observers->push_back(new GlobalObserver<ConcreteEvent>(Function));
	}

	//Function to publish an event has occured and notify all subscribers to the event.
	template<typename ConcreteEvent>
	void publish(ConcreteEvent* e, bool cleanup = false)
	{
		// Get the list of observers from the map.
		ObserverList* observers = m_subscribers[typeid(ConcreteEvent)];
		if (observers == nullptr) { return; }
		// FOr each observer notify them that the event has occured.
		for (auto& handler : *observers)
		{
			handler->exec(e);
			// If an event has been handled by a subscriber then we do not need to keep notifying other subscribers.
			if (static_cast<Event*>(e)->IsHandled())
			{
				break;
			}
		}
		// As we could pass through "new ConcreteEvent()" we should call delete if needed.
		if (cleanup) { delete e; }
	}
protected:
//Keep the constructors protected and use this dispatcher class as a singleton object.
	Dispatcher() {};
	~Dispatcher()
	{
		//Better clean up the subscriber map.
		for (auto it = m_subscribers.begin(); it != m_subscribers.end(); ++it)
		{
			ObserverList* obs = it->second;
			for(auto o = obs->begin(); o != obs->end(); ++o)
			{
				delete(*o);
				(*o) = nullptr;
			}
			delete obs;
		}
	}

private:
	// A has map of observers uses typeid of Event class as an index into the map.
	std::map<std::type_index, ObserverList*> m_subscribers;

	//Instance.
	static Dispatcher* m_instance;
};