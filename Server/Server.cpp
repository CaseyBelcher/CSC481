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


using namespace std; 

// for convenience
using json = nlohmann::json;

zmq::context_t context(1);
//vector <Player> players; 
map<int, Player> players;


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
	// start pull thread 
	thread t1(pullThread);

	// Prepare PUB socket
	zmq::socket_t publisher(context, ZMQ_PUB);
	publisher.bind("tcp://*:5563");


	// format of Server message 
	// (type of object, id of object, xPosition, yPosition)
	// t:player i:1234 x:200 y:200-
	while (1) {
		Sleep(1); 

		if (!players.empty()) {
			vector<string> messages;

			map<int, Player>::iterator itr;
			for (itr = players.begin(); itr != players.end(); ++itr) {

				json publishMessage =
				{
					{"type", "player"},
					{"id", itr->second.clientID},
					{"xPosition", itr->second.getPosition().x},
					{"yPosition", itr->second.getPosition().y}, 
					{"players", {
						{
							{"id", 12345},
							{"xPosition", 33}, 
							{"yPosition", 44}
						},
						{
							{"id", 44444},
							{"xPosition", 11},
							{"yPosition", 22}
						}
					}}
				};
				
				messages.push_back(publishMessage.dump());

			}


			for (int i = 0; i < messages.size(); i++) {
				//std::cout << "sending message: " + messages[i] << std::endl; 
				s_send(publisher, messages[i]);
			}

		}


		
		
		
	}

	t1.join();
	return 0; 
}

