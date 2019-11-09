#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <chrono>
#include <iostream>
#include "Player.cpp"
#include "Platform.cpp"
#include "MovingPlatform.cpp"
#include "TimeLine.cpp"
#include <zmq.hpp>
#include "zhelpers.hpp"
#include <thread>
#include <time.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <iomanip> 
#include <nlohmann/json.hpp>

using namespace std::chrono; 
using namespace std; 
// for convenience
using json = nlohmann::json;





//  Prepare our context
zmq::context_t context(1);

// create game window 
sf::RenderWindow window(sf::VideoMode(800, 600), "My window", sf::Style::Resize | sf::Style::Close);

bool hasFocus = true; 
map<int, MovingPlatform> movingPlatforms;
map<int, Platform> platforms; 
map<int, Player> players; 





void detectCollision(Player player, sf::FloatRect playerBound, sf::FloatRect platformBound, sf::FloatRect movingPlatformBound) {
	if (playerBound.intersects(platformBound) || playerBound.intersects(movingPlatformBound)) {
		player.setOutlineColor(sf::Color::Red);
		player.setOutlineThickness(10);
	}
	else {
		player.setOutlineColor(sf::Color::White);
		player.setOutlineThickness(1);
	}
}


/* 
	Responsible for detecting user input and PUSHing to server
*/
void pushThread(int clientID) {

	//  Prepare PUSH  
	zmq::socket_t sender(context, ZMQ_PUSH);
	sender.connect("tcp://localhost:5558");

	bool firstConnection = true; 
	while (window.isOpen()) {

		// if this is our first time connecting, let server know to display us 
		json connectMessage =
		{
			{"clientID", clientID},
			{"type", "firstConnection"}, 
			{"timestamp", 666}, 
			{"message", "connecting"}
		};
		if (firstConnection) {
			// cout << "connecting to server...." << endl; 
			s_send(sender, connectMessage.dump());
			firstConnection = false;
		}
		
		
		if (hasFocus) {

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			{
				json userInput =
				{
					{"clientID", clientID},
					{"type", "userInput"},
					{"timestamp", 666},
					{"message", "left"},  
				};

				// cout << "message sent: " + userInput.dump() << endl;
				s_send(sender, userInput.dump());
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			{
				json userInput =
				{
					{"clientID", clientID},
					{"type", "userInput"},
					{"timestamp", 666},
					{"message", "right"},
				};

				// cout << "message sent: " + userInput.dump() << endl;
				s_send(sender, userInput.dump());
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			{
				json userInput =
				{
					{"clientID", clientID},
					{"type", "userInput"},
					{"timestamp", 666},
					{"message", "up"},
				};

				// cout << "message sent: " + userInput.dump() << endl;
				s_send(sender, userInput.dump());
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			{
				json userInput =
				{
					{"clientID", clientID},
					{"type", "userInput"},
					{"timestamp", 666},
					{"message", "down"},
				};

				// cout << "message sent: " + userInput.dump() << endl;
				s_send(sender, userInput.dump());
			}


		}

	}
	
}

int main()
{



	




	// create moving platform rectangle 
	MovingPlatform movingPlatform(sf::Vector2f(120.f, 50.f));
	movingPlatform.setSize(sf::Vector2f(100.f, 100.f));
	movingPlatform.setPosition(300.f, 300.f);
	movingPlatform.id = 1; 
	pair<int, MovingPlatform> thisPair(); 
	

	movingPlatforms.insert(pair<int, MovingPlatform>(movingPlatform.id, movingPlatform)); 

	// create static platform rectangle and set its texture 
	Platform platform(sf::Vector2f(120.f, 250.f));
	platform.setSize(sf::Vector2f(100.f, 100.f));
	platform.setPosition(20.f, 300.f);
	platform.setFillColor(sf::Color::Blue);
	platform.setOutlineColor(sf::Color::White);
	platform.setOutlineThickness(5);
	platform.id = 1; 
	platforms.insert(pair<int, Platform>(platform.id, platform));


	// create player circle 
	Player player(50.f);
	player.setPosition(100.f, 100.f);
	player.setFillColor(sf::Color::Green);
	// Use current time as seed for random generator 
	srand(time(0));
	// client identifier 
	player.clientID = rand(); 
	players.insert(pair<int, Player>(player.clientID, player));


	// start push thread 
	thread t1(pushThread, player.clientID);
	// Prepare SUB socket 
	zmq::socket_t subscriber(context, ZMQ_SUB);
	subscriber.connect("tcp://localhost:5563");
	subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);


	while (window.isOpen()) {

		//  get SUB message from Server  
		std::string contents = s_recv(subscriber);
		//std::cout << "contents: " + contents << std::endl;

		// Parse message 
		json subMessage = json::parse(contents); 
		string type = subMessage.at("type"); 
		int id = subMessage.at("id"); 
		float newX = subMessage.at("xPosition"); 
		float newY = subMessage.at("yPosition"); 
		
		vector<json> playersList = subMessage.at("players"); 
		json firstPlayer = playersList.at(0);
		cout << firstPlayer.dump() << endl; 

		
		
		if (type == "player") {

			map<int, Player>::iterator it = players.find(id);

			// player found 
			if (it != players.end()) {
				it->second.setPosition(newX, newY);
			}
			// player not found, create a new player 
			else {
				Player newPlayer(50.f);
				newPlayer.setPosition(newX, newY);
				newPlayer.setFillColor(sf::Color::Green);
				newPlayer.clientID = id;
				players.insert(pair<int, Player>(newPlayer.clientID, newPlayer));
			}

		}
		else if (type == "movingPlatform") {

		}
		else if (type == "platform") {

		}
		


		// check all the window's events that were triggered since the last iteration of the loop
		sf::Event event;
		while (window.pollEvent(event))
		{
			// "close requested" event: we close the window
			if (event.type == sf::Event::Closed) {
				window.close();
			}


			else if (event.type == sf::Event::LostFocus) {
				//std::cout << "LostFocus case 2...." << std::endl;
				hasFocus = false; 
			}

			else if (event.type == sf::Event::GainedFocus) {
				//std::cout << "GainedFocus case 2...." << std::endl;
				hasFocus = true;
			}
		}

		
		// clear the window with black color
		window.clear(sf::Color::Black);

		
		// re-draw everything 
		window.draw(movingPlatform);
		map<int, Player>::iterator itr;
		for (itr = players.begin(); itr != players.end(); ++itr) {
			window.draw(itr->second); 
		}
		window.draw(platform);


		// end the current frame
		window.display();

	}

	t1.join(); 

	return 0;
} 