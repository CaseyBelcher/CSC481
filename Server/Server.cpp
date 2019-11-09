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


void executeAction(int clientID, string action) {

	cout << "Player: " + to_string(players.at(clientID).clientID) + " - message: " + action << endl; 


	if (action == "left") {
		float oldX = players.at(clientID).getPosition().x; 
		float newX = oldX - 0.01f;

		players.at(clientID).setPosition(newX, players.at(clientID).getPosition().y);
	}

	



	//std::cout << "Player: " + std::to_string(players[playerIndex].clientID) + " - action: " + action << std::endl;
	

	/* 

	if (action == "right") {
		float oldX = players[playerIndex].getPosition().x; 
		float newX = oldX + 0.01f;

		players[playerIndex].setPosition(newX, players[playerIndex].getPosition().y);



	}
	else if (action == "left") {
		float oldX = players[playerIndex].getPosition().x;
		float newX = oldX - 0.01f;

		players[playerIndex].setPosition(newX, players[playerIndex].getPosition().y);
	}

	else {
		// create object from string literal
			// json j = "{ \"happy\": true, \"pi\": 3.141 }"_json;
		
		// parse explicitly
			// auto j3 = json::parse("{ \"happy\": true, \"pi\": 3.141 }");

		// json j = json::parse(action); 
		
		//// find an entry
		//json::iterator it = j.find("one"); 
		//if (it != j.end()) {

		//	//std::cout << it.value() << std::endl;
		//	//std::cout << "what" << std::endl;

		//	string whatever = it.value(); 
		//	std::cout << whatever << std::endl;

		//
		//
		//}
	
	
	
	}

	*/ 



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
		executeAction(thisClientID, message); 

		
		

		//	format: playerID action(left/right/jump) 
		//	example: 1234 left
		
		/* 
		
		stringstream myStream(contents);
		string token;
		vector <string> tokens;
		while (getline(myStream, token, ' ')) {
			if (!token.empty()) {
				tokens.push_back(token);
			}
		}

		// if no players, make a new one 
		if (players.empty()) {

			//std::cout << "No players...creating new one..." << std::endl;

			// TODO render new player 
			Player player(50.f);
			player.setPosition(100.f, 100.f);


			stringstream stringToInt(tokens[0]); 
			stringToInt >> player.clientID;
			
			players.push_back(player); 
			executeAction(players.size()-1, tokens[1]);

		}
		// we have some players, so try and find the one that sent this message 
		else {
			
			stringstream stringToInt(tokens[0]);
			int thisId = 0;
			stringToInt >> thisId;

			for (int i = 0; i < players.size(); i++) {
				
				// once we find the player of this message, execute action for that player
				if (players[i].clientID == thisId) {
					//std::cout << "Player found..." << std::endl;
					executeAction(i, tokens[1]);
					break;
				}
				// if we are on the last player, and he doesn't match, we need to add this player 
				else if (i == players.size() - 1) {
					//std::cout << "Creating new player..." << std::endl;
					// TODO render new player 
					Player player(50.f);
					player.setPosition(100.f, 100.f);

					
					stringstream stringToInt(tokens[0]); 
					stringToInt >> player.clientID;
					players.push_back(player);
					executeAction(i + 1, tokens[1]);
					break;
				}

			}
		}

		*/ 


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
	
			/*
			map<int, Player>::iterator itr;
			for (itr = players.begin(); itr != players.end(); ++itr) {
				window.draw(itr->second);
			}
			*/ 


			vector<string> messages;
			

			/* 
			for (int i = 0; i < players.size(); i++) {
				string playerId = std::to_string(players[i].clientID);

				string thisMessage = "t:player i:" + playerId + " x:" + std::to_string(players[i].getPosition().x) + " y:300"; 
				messages.push_back(thisMessage); 
			}
			*/ 

			map<int, Player>::iterator itr;
			for (itr = players.begin(); itr != players.end(); ++itr) {
				string playerId = to_string(itr->second.clientID);

				string thisMessage = "t:player i:" + playerId + " x:" + std::to_string(players[i].getPosition().x) + " y:300";
				messages.push_back(thisMessage);
			}


			for (int i = 0; i < messages.size(); i++) {
				//Sleep(500);
				std::cout << "sending message: " + messages[i] << std::endl; 
				s_send(publisher, messages[i]);
			}

		}


		
		
		
	}

	t1.join();
	return 0; 
}

