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
	Player thisPlayer = players[playerIndex];

	std::cout << "I am called with: " + action << std::endl;


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

		//std::cout << "Waiting for action.... " << std::endl;
		//std::cout << "Total Players: " + std::to_string(players.size()) << std::endl;

		//// get PULL message from PUSHING clients 
		std::string contents = s_recv(receiver);

		//std::cout << "Action received" << std::endl;
		


		//// Parse the client's message 
		////	format: playerID action(left/right/jump) 
		////	example: 1234 left
		stringstream myStream(contents);
		string token;
		vector <string> tokens;
		bool tokensFound = false; 
		while (getline(myStream, token, ' ')) {
			if (!token.empty()) {
				tokens.push_back(token);
			}
			std::cout << "token found: " + token << std::endl; 
		}
		tokensFound = !(tokens[0].empty() || tokens[1].empty()); 

		//// find player 

		if (players.empty()) {
			//std::cout << "Creating new player - second case..." << std::endl;

			//// TODO render new player 
			//Player player(50.f);
			//player.clientID = stoi(tokens[1]); 
			//players.push_back(player); 
			//executeAction(players.size()-1, tokens[1]);
		}
		else {
			for (int i = 0; i < players.size() && tokensFound; i++) { 
			//for (int i = 0; i < 10; i++) {
				//std::cout << "what" << std::endl; 

	//			if (players[i].clientID == stoi(tokens[0])) {
					// once we find the player of this message, execute action for that player 
					//executeAction(i, tokens[1]); 
		//			break; 
				//}
			//	// if we are on the last player, and he doesn't match, we need to add this player 
				//else if( i == players.size()-1 ){
			//		std::cout << "Creating new player..." << std::endl;
			//		std::cout << "Last player id: " + std::to_string(players[i].clientID) << std::endl; 
			//		std::cout << "message id: " + stoi(tokens[0]) << std::endl; 
			//		// TODO render new player 
					//Player player(50.f);
					//player.clientID = stoi(tokens[1]);
					//players.push_back(player);
					//executeAction(i + 1, tokens[1]); 
					//break; 
				//}

			}
		}


	}
	

}





int main()
{
	float movingPlatformX = 300.f; 

	// start pull thread 
	thread t1(pullThread);

	// Prepare PUB socket
	zmq::socket_t publisher(context, ZMQ_PUB);
	publisher.bind("tcp://*:5563");

	while (1) {
		// Send PUB message to clients 
		s_send(publisher, std::to_string(movingPlatformX));
	}

	t1.join();
	return 0; 
}

