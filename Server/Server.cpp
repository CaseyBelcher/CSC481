#include <iostream>
#include <zmq.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <windows.h>
#include <time.h>
#include "zhelpers.hpp"
#include <thread>
#include <nlohmann/json.hpp>
#include <queue> 


#include "Player.cpp"
#include "MovingPlatform.cpp"
#include "Platform.cpp"

using namespace std; 
using json = nlohmann::json;

zmq::context_t context(1);
map<int, Player> players;
map<int, MovingPlatform> movingPlatforms; 
map<int, Platform> platforms; 

class MyEvent {
	public: 
		string type; 
		int priority; 
		int objectID; 
		string objectType;
		string direction; 

		MyEvent(string type, int priority, int objectID, string objectType, string direction) {
			this->type = type; 
			this->priority = priority; 
			this->objectID = objectID; 
			this->objectType = objectType; 
			this->direction = direction; 
		}

};

class EventHandler {
public:

	EventHandler() {
	}

	virtual void onEvent(MyEvent e) = 0;

};

class MainEventHandler : EventHandler {
	public:

		void onEvent(MyEvent e) {
			if (e.type == "collision") {

			}
			else if (e.type == "death") {

			}
			else if (e.type == "spawn") {

			}
			else if (e.type == "userInput") {

				if (e.direction == "left") {
					float oldX = players.at(e.objectID).getPosition().x;
					float newX = oldX - 0.1f;

					players.at(e.objectID).setPosition(newX, players.at(e.objectID).getPosition().y);
				}
				else if (e.direction == "right") {
					float oldX = players.at(e.objectID).getPosition().x;
					float newX = oldX + 0.1f;

					players.at(e.objectID).setPosition(newX, players.at(e.objectID).getPosition().y);
				}
				else if (e.direction == "up") {
					float oldY = players.at(e.objectID).getPosition().y;
					float newY = oldY - 0.1f;

					players.at(e.objectID).setPosition(players.at(e.objectID).getPosition().x, newY);
				}
				else if (e.direction == "down") {
					float oldY = players.at(e.objectID).getPosition().y;
					float newY = oldY + 0.1f;

					players.at(e.objectID).setPosition(players.at(e.objectID).getPosition().x, newY);
				}
			}
			else if (e.type == "startRecording") {

			}
			else if (e.type == "stopRecording") {

			}
		}
};





class MyEventManager {
	public:
		// <event type, list of handlers interested in that type> 
		map<string, vector<MainEventHandler>> handlerMap; 
		queue<MyEvent> firstEvents;
		queue<MyEvent> secondEvents;


		MyEventManager() {
	
		}
	
		// register an event handler for a type of event  
		void registerHandler(string type, MainEventHandler handler) {
			
			map<string, vector<MainEventHandler>>::iterator it = handlerMap.find(type); 
			
			// if this event type not in map yet, add it 
			if (it == handlerMap.end()) {
				vector<MainEventHandler> handlers; 
				handlers.push_back(handler); 
				handlerMap.insert(pair<string, vector<MainEventHandler>>(type, handlers)); 
			}
			else {
				handlerMap.at(type).push_back(handler);
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
				vector<MainEventHandler> handlerList = handlerMap.at(thisEvent.type); 
				
				// notify each registered handler about this event  
				for (int i = 0; i < handlerList.size(); i++) {
					handlerList.at(i).onEvent(thisEvent); 
				}

				firstEvents.pop(); 

			}

			// go through events of secondary priority 
			while (!secondEvents.empty()) {

				MyEvent thisEvent = secondEvents.front();
				if (thisEvent.type != "") {

					vector<MainEventHandler> handlerList = handlerMap.at(thisEvent.type);

					// notify each registered handler about this event  
					for (int i = 0; i < handlerList.size(); i++) {
						handlerList.at(i).onEvent(thisEvent);
					}

				}

				secondEvents.pop();

			}
		}


};


MyEventManager eventManager;





void executeAction(int clientID, string message) {

	//cout << "Player: " + to_string(players.at(clientID).clientID) + " - message: " + message << endl; 
	//MyEvent(string type, int priority, int objectID, string objectType, string direction) 

	MyEvent event("userInput", 2, clientID, "player", message);
	eventManager.addEvent(event); 

}


/** 
	PULL thread 
*/ 
void pullThread()
{
	// Prepare PULL socket 
	zmq::socket_t receiver(context, ZMQ_PULL);
	receiver.bind("tcp://*:5558");
	
	while (1) {

		// get PULL message from PUSHING clients 
		std::string contents = s_recv(receiver);
		 
		// Parse the client's message  
		json result = json::parse(contents);
		int thisClientID = result.at("clientID"); 
		string type = result.at("type"); 
		string message = result.at("message");

		// try to find player 
		map<int, Player>::iterator it = players.find(thisClientID);
		
		// player not found, create a new player 
		if (it == players.end()) {
			Player newPlayer(50.f);
			newPlayer.setPosition(100.f, 100.f);
			newPlayer.setFillColor(sf::Color::Green);
			newPlayer.clientID = thisClientID;
			players.insert(pair<int, Player>(newPlayer.clientID, newPlayer));
		}

		// execute action for that player 
		if (type != "firstConnection") {
			executeAction(thisClientID, message);
		}

	}
	
}



int main()
{
	MainEventHandler handler; 
	
	eventManager.registerHandler("userInput", handler); 
	eventManager.registerHandler("death", handler);
	eventManager.registerHandler("spawn", handler);
	eventManager.registerHandler("collision", handler);
	eventManager.registerHandler("startRecording", handler);
	eventManager.registerHandler("stopRecording", handler);

	// create default moving platform 
	MovingPlatform movingPlatform(sf::Vector2f(120.f, 50.f));
	movingPlatform.setSize(sf::Vector2f(100.f, 100.f));
	movingPlatform.setPosition(300.f, 300.f);
	movingPlatform.id = 1;
	movingPlatforms.insert(pair<int, MovingPlatform>(movingPlatform.id, movingPlatform));

	// create default platform 
	Platform platform(sf::Vector2f(120.f, 250.f));
	platform.setSize(sf::Vector2f(100.f, 100.f));
	platform.setPosition(20.f, 300.f);
	platform.setFillColor(sf::Color::Blue);
	platform.setOutlineColor(sf::Color::White);
	platform.setOutlineThickness(5);
	platform.id = 1;
	platforms.insert(pair<int, Platform>(platform.id, platform));

	// start pull thread 
	thread t1(pullThread);

	// Prepare PUB socket
	zmq::socket_t publisher(context, ZMQ_PUB);
	publisher.bind("tcp://*:5563");

	while (1) {
		Sleep(1); 

		// handle user-related events  
		eventManager.handleAllEvents();

		// as the server, dictate the moving platforms 



		if (!players.empty()) {

			vector<string> messages;
			json rootMessage; 
			json playersMessages; 
			json movingPlatformsMessages; 
			json platformsMessages; 

			// add all players to list  
			map<int, Player>::iterator itr;
			for (itr = players.begin(); itr != players.end(); ++itr) {

				json playersMessage =
				{
					{"id", itr->second.clientID},
					{"xPosition", itr->second.getPosition().x},
					{"yPosition", itr->second.getPosition().y}
				};
				
				playersMessages.push_back(playersMessage); 

			}

			// add all movingPlatforms to list
			map<int, MovingPlatform>::iterator itr2;
			for (itr2 = movingPlatforms.begin(); itr2 != movingPlatforms.end(); ++itr2) {

				json movingPlatformMessage =
				{
					{"id", itr2->second.id},
					{"xPosition", itr2->second.getPosition().x},
					{"yPosition", itr2->second.getPosition().y}
				};

				movingPlatformsMessages.push_back(movingPlatformMessage);

			}

			// add all platforms to list 
			map<int, Platform>::iterator itr3;
			for (itr3 = platforms.begin(); itr3 != platforms.end(); ++itr3) {

				json platformMessage =
				{
					{"id", itr3->second.id},
					{"xPosition", itr3->second.getPosition().x},
					{"yPosition", itr3->second.getPosition().y}
				};

				platformsMessages.push_back(platformMessage);

			}

			rootMessage.push_back(playersMessages); 
			rootMessage.push_back(movingPlatformsMessages); 
			rootMessage.push_back(platformsMessages); 
			messages.push_back(rootMessage.dump()); 


			for (int i = 0; i < messages.size(); i++) {
				//std::cout << "sending message: " + messages[i] << std::endl; 
				s_send(publisher, messages[i]);
			}

		}

	}

	t1.join();


	return 0; 

}

