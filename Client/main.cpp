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
bool upPressed = false;
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
	json connectMessage =
	{
		{"clientID", clientID},
		{"type", "firstConnection"},
		{"timestamp", 666},
		{"message", "connecting"}
	};

	while (window.isOpen()) {

		// if this is our first time connecting, let server know to display us 
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
			//if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			if (upPressed)
			{
				json userInput =
				{
					{"clientID", clientID},
					{"type", "userInput"},
					{"timestamp", 666},
					{"message", "up"},
				};

				//cout << "message sent: " + userInput.dump() << endl;
				s_send(sender, userInput.dump());
				upPressed = false; 
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
	// create player circle 
	Player player(50.f);
	player.setPosition(100.f, 100.f);
	player.setFillColor(sf::Color::Green);
	srand(time(0));
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

		vector<json> playersList = subMessage.at(0); 
		vector<json> movingPlatformsList = subMessage.at(1); 
		vector<json> platformsList = subMessage.at(2); 
		
		// parse player information 
		for (int i = 0; i < playersList.size(); i++) {
			
			json thisPlayer = playersList.at(i); 
			int id = thisPlayer.at("id"); 
			float newX = thisPlayer.at("xPosition"); 
			float newY = thisPlayer.at("yPosition"); 
		
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
		// parse movingPlatform information 
		for (int i = 0; i < movingPlatformsList.size(); i++) {

			json thisMovingPlatform = movingPlatformsList.at(i);
			int id = thisMovingPlatform.at("id");
			float newX = thisMovingPlatform.at("xPosition");
			float newY = thisMovingPlatform.at("yPosition");

			map<int, MovingPlatform>::iterator it = movingPlatforms.find(id);

			// movingPlatform found 
			if (it != movingPlatforms.end()) {
				it->second.setPosition(newX, newY);
			}
			// movingPlatform not found, create a new movingPlatform 
			else {
				
				MovingPlatform movingPlatform(sf::Vector2f(120.f, 50.f));
				movingPlatform.setSize(sf::Vector2f(100.f, 100.f));
				movingPlatform.setPosition(newX, newY);
				movingPlatform.id = id;
				movingPlatforms.insert(pair<int, MovingPlatform>(movingPlatform.id, movingPlatform));

			}

		}
		// parse platform information 
		for (int i = 0; i < platformsList.size(); i++) {

			json thisPlatform = platformsList.at(i);
			int id = thisPlatform.at("id");
			float newX = thisPlatform.at("xPosition");
			float newY = thisPlatform.at("yPosition");
			string type = thisPlatform.at("type"); 

			map<int, Platform>::iterator it = platforms.find(id);

			// Platform found 
			if (it != platforms.end()) {
				it->second.setPosition(newX, newY);
			}
			// Platform not found, create a new Platform 
			else {

				cout << type << endl; 

				float platX; 
				float platY; 
				float platwidth; 
				float platheight; 
				sf::Color color; 
				if (type == "deathZone") {
					platX = 200.f; 
					platY = 200.f; 
					platwidth = 100.f; 
					platheight = 100.f;
					color = sf::Color::Red; 
				}
				else {
					platX = 120.f; 
					platY = 250.f; 
					platwidth = 100.f; 
					platheight = 100.f;
					color = sf::Color::Blue; 
				}
				Platform platform(sf::Vector2f(platX, platY));
				platform.setSize(sf::Vector2f(platwidth, platheight));
				platform.setPosition(newX, newY);
				platform.setFillColor(color);
				platform.setOutlineColor(sf::Color::White);
				platform.setOutlineThickness(5);
				platform.id = id;
				platforms.insert(pair<int, Platform>(platform.id, platform));

			}

		}
		

		// check for window related events
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			else if (event.type == sf::Event::LostFocus) {
				hasFocus = false; 
			}
			else if (event.type == sf::Event::GainedFocus) {
				hasFocus = true;
			}
			else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Up) {
				upPressed = true; 
			}
		}

		
		// clear the window with black color
		window.clear(sf::Color::Black);

		
		// re-draw players  
		map<int, Player>::iterator itr;
		for (itr = players.begin(); itr != players.end(); ++itr) {
			window.draw(itr->second);
		}

		// re-draw moving platforms   
		map<int, MovingPlatform>::iterator itr2;
		for (itr2 = movingPlatforms.begin(); itr2 != movingPlatforms.end(); ++itr2) {
			window.draw(itr2->second);
		}

		// re-draw platforms 
		map<int, Platform>::iterator itr3;
		for (itr3 = platforms.begin(); itr3 != platforms.end(); ++itr3) {
			window.draw(itr3->second);
		}


		// end the current frame
		window.display();

	}

	t1.join(); 

	return 0;
} 