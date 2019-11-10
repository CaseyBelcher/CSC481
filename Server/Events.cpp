#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <windows.h>
#include <queue> 


#include "Player.cpp"
#include "MovingPlatform.cpp"
#include "Platform.cpp"

using namespace std;










class MyEvent {
public:
	string type;
	int priority;
	int objectID;
	string objectType;
	string direction;
	float stepSize = 0; 

	MyEvent(string type, int priority, int objectID, string objectType, string direction, float stepSize) {
		this->type = type;
		this->priority = priority;
		this->objectID = objectID;
		this->objectType = objectType;
		this->direction = direction;
		this->stepSize = stepSize; 
	}

};



class EventHandler {
public:

	EventHandler() {
	}

	virtual void onEvent(MyEvent e) = 0;


};



class MyEventManager {
public:
	// <event type, list of handlers interested in that type> 
	map<string, vector<EventHandler*>> handlerMap;
	queue<MyEvent> firstEvents;
	queue<MyEvent> secondEvents;


	MyEventManager() {

	}

	// register an event handler for a type of event  
	void registerHandler(string type, EventHandler &handler) {

		

		map<string, vector<EventHandler*>>::iterator it = handlerMap.find(type);

		// if this event type not in map yet, add it 
		if (it == handlerMap.end()) {
			vector<EventHandler*> handlers;
			handlers.push_back(&handler);
			handlerMap.insert(pair<string, vector<EventHandler*>>(type, handlers));
		}
		else {
			handlerMap.at(type).push_back(&handler);
		}
	}


	void addEvent(MyEvent event) {
		if (event.type != "") {



			if (event.priority == 1) {
				firstEvents.push(event);
			}
			else {
				secondEvents.push(event);
			}

		}
	}

	void handleAllEvents() {
		 
		// go through first priority events first 
		while (!firstEvents.empty()) {

			MyEvent thisEvent = firstEvents.front();
			vector<EventHandler*> handlerList = handlerMap.at(thisEvent.type);

			// notify each registered handler about this event  
			for (int i = 0; i < handlerList.size(); i++) {
				//handlerList.at(i)->onEvent
				handlerList.at(i)->onEvent(thisEvent);
			}

			firstEvents.pop();

		}

		// go through events of secondary priority 
		while (!secondEvents.empty()) {

			MyEvent thisEvent = secondEvents.front();
			if (thisEvent.type != "") {

				vector<EventHandler*> handlerList = handlerMap.at(thisEvent.type);

				// notify each registered handler about this event  
				for (int i = 0; i < handlerList.size(); i++) {
					handlerList.at(i)->onEvent(thisEvent);
				}

			}

			secondEvents.pop();

		}

		 
	}


};

