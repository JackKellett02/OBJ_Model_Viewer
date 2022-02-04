#pragma once

//! Event Class.
/*!
 An abstract base class for concrete event classes to inherit from.
 */

class Event
{
public:
	//Default constructor.
	//Constructs a base event class object. Sets handled to false.
	Event() : m_bHandled(false) {}
	//Destructor.
	virtual ~Event() {};
	//Using the 'using' command to create an alias for const char*.
	using DescriptorType = const char*;
	//Returns the descriptor type of the event.
	//Abstract function to be implement in derived classes.
	//Return DescriptorType returns the type of Event as a const char*.
	virtual DescriptorType type() const = 0;
	//Function used to set if an event has been handled. Events that are handled do not report
	//To any subsequent observers.
	void Handled() { m_bHandled = true; }
	//Function to set that an event has been handled.
	bool IsHandled() { return m_bHandled; }
private:
	//Variable indicates if event has been processed or not.
	bool m_bHandled;
};