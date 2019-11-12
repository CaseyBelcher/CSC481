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
Gametime thisTime(500); 
bool recording = false; 
bool replaying = false; 




/* 
	Responsible for detecting user input and PUSHing to server
*/
void pushThread(int clientID) {

	// time to compare ping time to 
	int lastTime = thisTime.getTime(); 

	//  Prepare PUSH  
	zmq::socket_t sender(context, ZMQ_PUSH);
	sender.connect("tcp://localhost:5558");

	bool firstConnection = true; 
	json connectMessage =
	{
		{"clientID", clientID},
		{"type", "firstConnection"},
		{"timestamp", thisTime.getTime()},
		{"message", "connecting"}
	};

	json pingMessage1 =
	{
		{"clientID", clientID},
		{"type", "ping"},
		{"timestamp", thisTime.getTime()},
		{"message", "connecting"}
	};

	while (window.isOpen()) {

		// if this is our first time connecting, let server know to display us 
		if (firstConnection) {
			// cout << "connecting to server...." << endl; 
			s_send(sender, connectMessage.dump());
			s_send(sender, pingMessage1.dump()); 
			firstConnection = false;
		}
		
		if (thisTime.getTime() >= (lastTime + 1)) {

			json pingMessage2 =
			{
				{"clientID", clientID},
				{"type", "ping"},
				{"timestamp", thisTime.getTime()},
				{"message", "connecting"}
			};

			// cout << "ping sent" << endl; 
			s_send(sender, pingMessage2.dump());
			lastTime = thisTime.getTime();
		}

		
		if (hasFocus) {

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			{
				json userInput =
				{
					{"clientID", clientID},
					{"type", "userInput"},
					{"timestamp", thisTime.getTime()},
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
					{"timestamp", thisTime.getTime()},
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
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
			{
				json userInput =
				{
					{"clientID", clientID},
					{"type", "startRecording"},
					{"timestamp", 666},
					{"message", "blah"},
				};

				// cout << "message sent: " + userInput.dump() << endl;
				s_send(sender, userInput.dump());
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
			{
				json userInput =
				{
					{"clientID", clientID},
					{"type", "stopRecording"},
					{"timestamp", 666},
					{"message", "blah"},
				};

				// cout << "message sent: " + userInput.dump() << endl;
				s_send(sender, userInput.dump());
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
			{
				json userInput =
				{
					{"clientID", clientID},
					{"type", "replayRecording"},
					{"timestamp", 666},
					{"message", "blah"},
				};

				// cout << "message sent: " + userInput.dump() << endl;
				s_send(sender, userInput.dump());
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
			{
				json userInput =
				{
					{"clientID", clientID},
					{"type", "timeChange"},
					{"timestamp", 666},
					{"message", "normal"},
				};

				// cout << "message sent: " + userInput.dump() << endl;
				s_send(sender, userInput.dump());
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
			{
				json userInput =
				{
					{"clientID", clientID},
					{"type", "timeChange"},
					{"timestamp", 666},
					{"message", "double"},
				};

				// cout << "message sent: " + userInput.dump() << endl;
				s_send(sender, userInput.dump());
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3))
			{
				json userInput =
				{
					{"clientID", clientID},
					{"type", "timeChange"},
					{"timestamp", 666},
					{"message", "half"},
				};

				// cout << "message sent: " + userInput.dump() << endl;
				s_send(sender, userInput.dump());
			}
			
			


		}

	}
	
}

int main()
{

	Gametime testTime(1000); 

	int lastTime = 0; 
	while (1) {
		cout << testTime.getTime() << endl; 

		//int currentTime = testTime.getTime(); 
		//if (currentTime >= lastTime + 1) {
		//	cout << currentTime << endl;
		//	lastTime = currentTime; 

		//}

		bool doublecalledOnce = false; 
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2) && !doublecalledOnce) {
			//cout << "double" << endl;
			testTime.doubleTime(); 
			doublecalledOnce = true; 
		}
		bool singlecalledOnce = false;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1) && !singlecalledOnce) {
			//cout << "single" << endl;
			testTime.oneTime();
			singlecalledOnce = true;
		}
		bool halfCalledOnce = false;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3) && !halfCalledOnce) {
			//cout << "half" << endl;
			testTime.halfTime();
			halfCalledOnce = true;
		}

	}

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

		// cout << thisTime.getTime() << endl; 

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
			recording = thisPlayer.at("recording"); 
			replaying = thisPlayer.at("replaying"); 
		
			map<int, Player>::iterator it = players.find(id);

			// player found 
			if (it != players.end()) {
				it->second.setPosition(newX, newY);
				it->second.connected = true; 
			}
			// player not found, create a new player 
			else {
				Player newPlayer(50.f);
				newPlayer.setPosition(newX, newY);
				newPlayer.setFillColor(sf::Color::Green);
				newPlayer.clientID = id;
				newPlayer.connected = true; 
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

				// cout << type << endl; 

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
			else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Up && hasFocus) {
				upPressed = true; 
			}
		}

		
		// clear the window with black color
		window.clear(sf::Color::Black);

		vector<int> playersToRemove; 

		// re-draw players  
		map<int, Player>::iterator itr;
		for (itr = players.begin(); itr != players.end(); ++itr) {
			if (itr->second.connected) {
				itr->second.setFillColor(recording ? sf::Color::Blue : (replaying ? sf::Color::Red : sf::Color::Green)); 
				window.draw(itr->second);
				itr->second.connected = false;
			}
			// remove if not connected 
			else {
				playersToRemove.push_back(itr->first); 
			}

		}

		// remove all non-connected players 
		for (int i = 0; i < playersToRemove.size(); i++) {
			players.erase(playersToRemove.at(i)); 
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