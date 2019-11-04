#include <iostream>
#include <zmq.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <windows.h>
#include <time.h>
#include "zhelpers.hpp"
#include <thread>

#include "Player.cpp"


using namespace std; 


zmq::context_t context(1);
vector <Player> players; 


void executeAction(int playerIndex, string action) {

	//std::cout << "Player: " + std::to_string(players[playerIndex].clientID) + " - action: " + action << std::endl;


	if (action == "right") {
		float oldX = players[playerIndex].getPosition().x; 
		float newX = oldX + 0.01f;

		players[playerIndex].setPosition(newX, players[playerIndex].getPosition().y);



	}
	if (action == "left") {
		float oldX = players[playerIndex].getPosition().x;
		float newX = oldX - 0.01f;

		players[playerIndex].setPosition(newX, players[playerIndex].getPosition().y);
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
		//	format: playerID action(left/right/jump) 
		//	example: 1234 left
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
			// string playerId = std::to_string(players[0].clientID);
			

			// string message1 = "t:player i:" + playerId + " x:" + std::to_string(players[0].getPosition().x) + " y:300";
			// messages.push_back(message1);

			for (int i = 0; i < players.size(); i++) {
				string playerId = std::to_string(players[i].clientID);

				string thisMessage = "t:player i:" + playerId + " x:" + std::to_string(players[i].getPosition().x) + " y:300"; 
				messages.push_back(thisMessage); 
			}


			for (int i = 0; i < messages.size(); i++) {
				//Sleep(500);
				//std::cout << "sending message: " + messages[i] << std::endl; 
				//s_send(publisher, message1);
				s_send(publisher, messages[i]);
			}

		}


		
		
		
	}

	t1.join();
	return 0; 
}

