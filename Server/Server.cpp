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


// #include "Events.cpp"
#include "TimeLine.cpp"

using namespace std; 
using json = nlohmann::json;

zmq::context_t context(1);

map<int, Player> players;
map<int, MovingPlatform> movingPlatforms; 
map<int, Platform> platforms; 

map<int, Player> recordedPlayers;
map<int, MovingPlatform> recordedMovingPlatforms;
map<int, Platform> recordedPlatforms;

map<int, Player> originalPlayers;
map<int, MovingPlatform> originalMovingPlatforms;
map<int, Platform> originalPlatforms;


float spawnX = 100.f; 
float spawnY = 100.f; 
float windowWidth = 800.f; 
float windowHeight = 600.f; 

float verticalVelocity = 0.f; 
float recordedVerticalVelocity; 
float originalVerticalVelocity; 

float gravityPull = 5.f; 
bool currentlyRecording = false; 
int recordingStartTime; 
int replayStartTime; 
int timeoutSize = 10; 
 
bool currentlyPlaying = false;  
 


Gametime thisTime(1000); 




///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
////////////////////////////////////////////////////////


class MyEvent {
public:
	string type;
	int priority;
	int objectID;
	string objectType;
	string direction;
	float stepSize = 0;
	int timestamp;

	MyEvent(string type, int priority, int objectID, string objectType, string direction, float stepSize) {
		this->type = type;
		this->priority = priority;
		this->objectID = objectID;
		this->objectType = objectType;
		this->direction = direction;
		this->stepSize = stepSize;
	}

};

queue<MyEvent> recording;



void clearQueueWhat(queue<MyEvent>& q)
{
	queue<MyEvent> empty;
	swap(q, empty);
}








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

	// used during the start of a replay to eliminate straggling client events 
	void clearAllEvents() {
		clearQueueWhat(firstEvents);
		clearQueueWhat(secondEvents);
	}


	// register an event handler for a type of event  
	void registerHandler(string type, EventHandler& handler) {



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

			// worried about when the recording process clears this manager of events 
			if (!firstEvents.empty()) {
				firstEvents.pop();

			}

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

			// worried about when the recording process clears this manager of events 
			if (!secondEvents.empty()) {
				secondEvents.pop();

			}

		}


	}


};

MyEventManager eventManager;


///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
////////////////////////////////////////////////////////


void isCollidingIntoPlayer(string direction, int movingPlatformID, float stepSize) {
	
	map<int, Player>::iterator itr2;
	for (itr2 = players.begin(); itr2 != players.end(); ++itr2) {

		if (movingPlatforms.at(movingPlatformID).getGlobalBounds().intersects(itr2->second.getGlobalBounds())) {
			MyEvent newEvent("collision", 1, itr2->second.clientID, "player", direction, stepSize);
			eventManager.addEvent(newEvent);
		}

	}
}

void isColliding(string direction, int objectID, string objectType, float stepSize) {

	// colliding with left of window 
	if (players.at(objectID).getPosition().x <= 0) {
		MyEvent event("collision", 1, objectID, "player", "left", stepSize);
		eventManager.addEvent(event); 
	}
	// colliding with top of window 
	if (players.at(objectID).getPosition().y <= 0) {
		MyEvent event("collision", 1, objectID, "player", "up", stepSize);
		eventManager.addEvent(event);

	}
	// colliding with right of window 
	if (players.at(objectID).getPosition().x + players.at(objectID).getLocalBounds().width >= windowWidth) {
		MyEvent event("collision", 1, objectID, "player", "right", stepSize);
		eventManager.addEvent(event);

	}
	// colliding with bottom of window 
	if (players.at(objectID).getPosition().y + players.at(objectID).getLocalBounds().height >= windowHeight) {
		MyEvent event("collision", 1, objectID, "player", "down", stepSize);
		eventManager.addEvent(event);

	}
	



	// check if colliding with any of the moving platforms 
	map<int, MovingPlatform>::iterator itr2;
	for (itr2 = movingPlatforms.begin(); itr2 != movingPlatforms.end(); ++itr2) {

		if (players.at(objectID).getGlobalBounds().intersects(itr2->second.getGlobalBounds())) {
			MyEvent newEvent("collision", 1, objectID, objectType, direction, stepSize);
			eventManager.addEvent(newEvent);
		}

	}

	// check if colliding with any of the static platforms 
	map<int, Platform>::iterator itr3;
	for (itr3 = platforms.begin(); itr3 != platforms.end(); ++itr3) {

		if (players.at(objectID).getGlobalBounds().intersects(itr3->second.getGlobalBounds())) {
			if (itr3->second.type == "deathZone") {
				MyEvent newEvent("death", 1, objectID, objectType, "", stepSize);
				eventManager.addEvent(newEvent);
			}
			else {
				MyEvent newEvent("collision", 1, objectID, objectType, direction, stepSize);
				eventManager.addEvent(newEvent);
			}
		}

	}

	// TODO 
	// maybe check with other players if this isn't too slow or complicated 

}

// for clearing the recording queue 
// duplicated from Events.cpp I think.......
//void clearQueue2(std::queue<MyEvent>& q)
//{
//	std::queue<MyEvent> empty;
//	std::swap(q, empty);
//}


class MainEventHandler : public EventHandler {
public:


	MainEventHandler() : EventHandler() {

	}

	

	//virtual void onEvent(MyEvent e) = 0;
	void onEvent(MyEvent e) {
		if (e.type == "collision") {
			// cout << "collision: " + e.direction << endl; 
			if (e.objectType == "player") {
				if (e.direction == "left") {
					players.at(e.objectID).move(e.stepSize, 0); 
				}
				if (e.direction == "right") {
					players.at(e.objectID).move(e.stepSize, 0);
				}
				if (e.direction == "up") {
					players.at(e.objectID).move(0, e.stepSize);
				}
				if (e.direction == "down") {
					players.at(e.objectID).move(0, e.stepSize);
				}
			}

			

		}
		else if (e.type == "death") {
			MyEvent event("spawn", 1, e.objectID, "player", "", 0); 
			eventManager.addEvent(event); 

		}
		else if (e.type == "spawn") {
			players.at(e.objectID).setPosition(spawnX, spawnY);

		}
		else if (e.type == "userInput") {

			// players.at(e.objectID).lastPing = thisTime.getTime(); 

			if (e.direction == "left") {

				players.at(e.objectID).move(-.1f, 0);
				isColliding("left", e.objectID, e.objectType, +.1f);

			}
			else if (e.direction == "right") {

				players.at(e.objectID).move(+.1f, 0);
				isColliding("right", e.objectID, e.objectType, -.1f);

			}
			else if (e.direction == "up") {

				// players.at(e.objectID).move(0, -.1f);
				// isColliding("up", e.objectID, e.objectType);
				verticalVelocity = -40; 


			}
			else if (e.direction == "down") {

				//players.at(e.objectID).move(0, +.1f);
				//isColliding("down", e.objectID, e.objectType, -.1f);

			}
		}
		else if (e.type == "startRecording") {

			if (!currentlyRecording) {

				// clear previous recording
				clearQueueWhat(recording);

				// store all platform, movingplatform, player positions, velocity, and startTime for recording 
				recordedPlatforms = platforms;
				recordedMovingPlatforms = movingPlatforms;
				recordedPlayers = players;
				recordedVerticalVelocity = verticalVelocity;
				recordingStartTime = thisTime.getTime();


				/*
					- set flag for executeAction() to start recording all outgoing userInput
						and ping Events
					- DO NOT record events for start/stop recording
					- Let engine re-generate collision, death, and spawn events
				*/
				currentlyRecording = true;

			} 

		}
		else if (e.type == "stopRecording") {

			// set flag for executeAction() to stop recording all outgoing Events 
			currentlyRecording = false; 
		}
		else if (e.type == "replayRecording") {

			if (!currentlyPlaying) {

				/*
					- main game loop now knows to check current time against timestamp of recorded events,
					  going through each one in the order it was raised
					- this also stops getting new events from client
				*/
				currentlyPlaying = true;
				replayStartTime = thisTime.getTime();

				// clear the event manager of any straggling client events 
				eventManager.clearAllEvents();

				// store all platform, movingplatform, and player positions for after playback 
				originalPlayers = players;
				originalPlatforms = platforms;
				originalMovingPlatforms = movingPlatforms;
				originalVerticalVelocity = verticalVelocity;

				// create everything as stored in recording 
				players = recordedPlayers;
				platforms = recordedPlatforms;
				movingPlatforms = recordedMovingPlatforms;
				verticalVelocity = recordedVerticalVelocity;

			} 

		}
		else if (e.type == "replayFinished") {
			
			cout << "replay has Finished" << endl;


			// restore the state of everything as before the recording 
			players = originalPlayers; 
			movingPlatforms = originalMovingPlatforms; 
			platforms = originalPlatforms; 
			verticalVelocity = originalVerticalVelocity; 
			thisTime.oneTime(); 

			// just to be safe, clear the event manager of any straggling recording events 
			eventManager.clearAllEvents();

			// set currentlyPlaying flag to false; start receiving client input again 
			currentlyPlaying = false; 


		} 
		else if (e.type == "ping") {
			players.at(e.objectID).lastPing = thisTime.getTime(); 

		}
		else if (e.type == "timeChange") {
			
			if (e.direction == "normal") {
				thisTime.oneTime(); 
			}
			else if (e.direction == "double") {
				thisTime.doubleTime(); 
			}
			else if (e.direction == "half") {
				thisTime.halfTime(); 
			}

		}



	}
};



void executeAction(int clientID, string message, string type) {
	if (!currentlyPlaying) {

		MyEvent event(type, 2, clientID, "player", message, 0);
		event.timestamp = thisTime.getTime();
		eventManager.addEvent(event);

		if (currentlyRecording &&
		   (event.type == "userInput" || event.type == "ping")) {

			recording.push(event);
		
		}
	}

	


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
			executeAction(thisClientID, message, type);

		}

	}
	
}



int main()
{
	MainEventHandler handler = MainEventHandler(); 

	eventManager.registerHandler("userInput", handler); 
	eventManager.registerHandler("death", handler);
	eventManager.registerHandler("spawn", handler);
	eventManager.registerHandler("collision", handler);
	eventManager.registerHandler("startRecording", handler);
	eventManager.registerHandler("stopRecording", handler);
	eventManager.registerHandler("ping", handler);
	eventManager.registerHandler("replayRecording", handler);
	eventManager.registerHandler("replayFinished", handler);
	eventManager.registerHandler("timeChange", handler); 

	// create default moving platform 
	MovingPlatform movingPlatform(sf::Vector2f(120.f, 50.f));
	movingPlatform.setSize(sf::Vector2f(100.f, 100.f));
	movingPlatform.setPosition(300.f, 300.f);
	movingPlatform.id = 1;
	movingPlatforms.insert(pair<int, MovingPlatform>(movingPlatform.id, movingPlatform));

	// create default platform 
	Platform platform(sf::Vector2f(120.f, 250.f), "platform");
	platform.setSize(sf::Vector2f(100.f, 100.f));
	platform.setPosition(20.f, 300.f);
	platform.setFillColor(sf::Color::Blue);
	platform.setOutlineColor(sf::Color::White);
	platform.setOutlineThickness(5);
	platform.id = 1;
	platforms.insert(pair<int, Platform>(platform.id, platform));


	// create death zone?
	Platform platform2(sf::Vector2f(200.f, 200.f), "deathZone");
	platform2.setSize(sf::Vector2f(100.f, 100.f));
	platform2.setPosition(700.f, 500.f);
	platform2.setFillColor(sf::Color::Red);
	platform2.setOutlineColor(sf::Color::White);
	platform2.setOutlineThickness(5);
	platform2.id = 2;
	platforms.insert(pair<int, Platform>(platform2.id, platform2));



	// start pull thread 
	thread t1(pullThread);

	// Prepare PUB socket
	zmq::socket_t publisher(context, ZMQ_PUB);
	publisher.bind("tcp://*:5563");

	bool movingLeft = false;
	int stepsTaken = 0; 
	int theLastTime = 0; 
	while (1) {

		int theCurrentTime = thisTime.getTime(); 
		if (theCurrentTime >= theLastTime + 1) {
			cout << thisTime.getTime() << endl;
			theLastTime = theCurrentTime; 
		}

		Sleep(1); 

		// - play recording by raising events 
		// - if you reach the end of recording queue, playback is done
		// - check current time against timestamp of recorded events, going through each one in the order 
		//   it was raised
		// - stop going through the map for this loop iteration when finding a later timestamp 
		if (currentlyPlaying) {

			int currentTime = thisTime.getTime(); 
			while (!recording.empty()) {
				MyEvent recEvent = recording.front(); 
				
				if ((currentTime - replayStartTime) >= (recording.front().timestamp - recordingStartTime)) {
					
					recEvent.timestamp = currentTime; 
					eventManager.addEvent(recEvent); 
					recording.pop();
				}
				else {
					break; 
				}
			}

			if (recording.empty()) {
				cout << "recording empty..." << endl; 

				//MyEvent thisEv("replayFinished", 1, 0, 0, "", 0); 
				MyEvent whatever("replayFinished", 1, 10, "blah", "blah", 2); 
				
				cout << "here 1..." << endl;

				eventManager.addEvent(whatever); 

				cout << "here 2..." << endl;

			}



		}


		// handle user-related events  
		eventManager.handleAllEvents();


		// as the server, dictate the moving platforms 
		if (movingLeft) {
			for (int i = 0; i < 10; i++) {
				movingPlatforms.at(1).move(-.05f, 0);
				isCollidingIntoPlayer("right", 1, +.05f);
				stepsTaken++;
			}
			if (stepsTaken >= 300) {
				stepsTaken = 0;
				movingLeft = false;
			}
		}
		else {
			for (int i = 0; i < 10; i++) {
				movingPlatforms.at(1).move(+.05f, 0);
				isCollidingIntoPlayer("left", 1, -.05f);
				stepsTaken++;
			}
			if (stepsTaken >= 300) {
				stepsTaken = 0;
				movingLeft = true;
			}
		}

		vector<int> playersToErase; 

		// apply verticalVelocity to all players 
		map<int, Player>::iterator itr;
		for (itr = players.begin(); itr != players.end(); ++itr ) {
			
			//bool realTimeComparison = (thisTime.getTime() - timeoutSize) > (itr->second.lastPing); 
			
			//bool replayTimeComparison = 


			// if this player hasn't pinged in the last 3 seconds, remove them  
			if ((thisTime.getTime() - timeoutSize) > (itr->second.lastPing)) {
				//players.erase(itr++);
				// cout << "ERASING PLAYER: " + to_string(itr->second.clientID) << endl; 
				playersToErase.push_back(itr->first); 
			}
			else {
				itr->second.move(0, verticalVelocity);
				isColliding("down", itr->second.clientID, "player", -verticalVelocity);
				verticalVelocity += gravityPull;
				if (verticalVelocity > gravityPull) {
					verticalVelocity = gravityPull;
				}
			}
		}

		for (int i = 0; i < playersToErase.size(); i++) {
			players.erase(playersToErase.at(i)); 
		}


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
					{"yPosition", itr->second.getPosition().y}, 
					{"recording", currentlyRecording}, 
					{"replaying", currentlyPlaying}
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
					{"yPosition", itr3->second.getPosition().y}, 
					{"type", itr3->second.type}
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

