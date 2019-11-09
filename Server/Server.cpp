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


#include "Player.cpp"
#include "MovingPlatform.cpp"
#include "Platform.cpp"

using namespace std; 

// for convenience
using json = nlohmann::json;

zmq::context_t context(1);
map<int, Player> players;
map<int, MovingPlatform> movingPlatforms; 
map<int, Platform> platforms; 


void executeAction(int clientID, string message) {

	//cout << "Player: " + to_string(players.at(clientID).clientID) + " - message: " + message << endl; 


	if (message == "left") {
		float oldX = players.at(clientID).getPosition().x; 
		float newX = oldX - 0.1f;

		players.at(clientID).setPosition(newX, players.at(clientID).getPosition().y);
	}
	else if (message == "right") {
		float oldX = players.at(clientID).getPosition().x;
		float newX = oldX + 0.1f;

		players.at(clientID).setPosition(newX, players.at(clientID).getPosition().y);
	}
	else if (message == "up") {
		float oldY = players.at(clientID).getPosition().y; 
		float newY = oldY - 0.1f; 

		players.at(clientID).setPosition(players.at(clientID).getPosition().x, newY);
	}
	else if (message == "down") {
		float oldY = players.at(clientID).getPosition().y;
		float newY = oldY + 0.1f;

		players.at(clientID).setPosition(players.at(clientID).getPosition().x, newY);
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
			executeAction(thisClientID, message);
		}

	}
	
}



int main()
{
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

