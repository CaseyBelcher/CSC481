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

using namespace std::chrono; 
using namespace std; 

//  Prepare our context
zmq::context_t context(1);

// create game window 
sf::RenderWindow window(sf::VideoMode(800, 600), "My window", sf::Style::Resize | sf::Style::Close);

 

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
	Responsible for detecting user input and sending to server
*/
void pushThread(int clientID) {
	
	//  Prepare PUSH  
	zmq::socket_t sender(context, ZMQ_PUSH);
	sender.connect("tcp://localhost:5558");

	int onlyOnce = 0;
	while (window.isOpen()) {

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && onlyOnce == 0 )
		{
			// PUSH message to Server
			string message = std::to_string(clientID) + " left"; 
			std::cout << "sending: " + message << std::endl; 
			s_send(sender, message);
			std::cout << "Command Sent" << std::endl;
			onlyOnce = 1; 
		}
	
	}
	
}

int main()
{

	

	// create moving platform rectangle 
	MovingPlatform movingPlatform(sf::Vector2f(120.f, 50.f));
	movingPlatform.setSize(sf::Vector2f(100.f, 100.f));
	movingPlatform.setPosition(300.f, 300.f);

	// create static platform rectangle and set its texture 
	Platform platform(sf::Vector2f(120.f, 250.f));
	platform.setSize(sf::Vector2f(100.f, 100.f));
	platform.setPosition(20.f, 300.f);
	platform.setFillColor(sf::Color::Blue);
	sf::Texture texture;
	if (!texture.create(10, 10)) {
		return -1;
	}
	sf::Uint8* pixels = new sf::Uint8[10 * 10 * 4];
	int colorChange = 500;
	for (int i = 0; i < 400; i++) {
		pixels[i] = colorChange;
		colorChange += 30;
	}
	texture.update(pixels);
	texture.setRepeated(true);
	platform.setTexture(&texture);
	platform.setOutlineColor(sf::Color::White);
	platform.setOutlineThickness(5);

	// create player circle 
	Player player(50.f);
	player.setPosition(100.f, 100.f);
	player.setFillColor(sf::Color::Green);
	// Use current time as seed for random generator 
	srand(time(0));
	// client identifier 
	player.clientID = rand(); 


	// start push thread 
	thread t1(pushThread, player.clientID);
	// Prepare SUB socket 
	zmq::socket_t subscriber(context, ZMQ_SUB);
	subscriber.connect("tcp://localhost:5563");
	subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);


	// number of moves the movingplatform has made 
	int platformMoves = 0;
	// direction of the movingplatform 
	int platformDirection = 1;
	Gametime myTime(2);
	int lastTime = myTime.getTime();
	while (window.isOpen()) {

		//  get SUB message from Server  
		std::string contents = s_recv(subscriber);
		//std::cout << contents << std::endl;

		float movingPlatformX = std::stof(contents); 
		movingPlatform.setPosition(movingPlatformX, 300.f); 

		



		// if at least 10 milliseconds have elapsed 
		if (myTime.getTime() >= lastTime + 10) {


			lastTime = myTime.getTime();



			// check all the window's events that were triggered since the last iteration of the loop
			sf::Event event;
			while (window.pollEvent(event))
			{
				// "close requested" event: we close the window
				if (event.type == sf::Event::Closed)
					window.close();
			}

			// clear the window with black color
			window.clear(sf::Color::Black);

			// draw everything here...

			
			if (platformDirection == 1) {
				movingPlatform.move(0, -4.f);

				if (platformMoves > 25) {
					platformMoves = 0;
					platformDirection = 0;
				}


			}
			else {
				movingPlatform.move(0, 4.f);

				if (platformMoves > 25) {
					platformMoves = 0;
					platformDirection = 1;
				}


			}
			platformMoves++;

			sf::FloatRect playerBound = player.getGlobalBounds();
			sf::FloatRect platformBound = platform.getGlobalBounds();
			sf::FloatRect movingPlatformBound = movingPlatform.getGlobalBounds();

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			{
				player.move(-1.f, 0.f);

				if (playerBound.intersects(platformBound) || playerBound.intersects(movingPlatformBound)) {
					player.setOutlineColor(sf::Color::Red);
					player.setOutlineThickness(10);
					player.move(2.f, 0.f);
				}
				else {
					player.setOutlineColor(sf::Color::White);
					player.setOutlineThickness(1);
				}
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			{
				player.move(1.f, 0.f);
				if (playerBound.intersects(platformBound) || playerBound.intersects(movingPlatformBound)) {
					player.setOutlineColor(sf::Color::Red);
					player.setOutlineThickness(10);
					player.move(-2.f, 0.f);
				}
				else {
					player.setOutlineColor(sf::Color::White);
					player.setOutlineThickness(1);
				}
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			{
				player.move(0.f, -1.f);
				if (playerBound.intersects(platformBound) || playerBound.intersects(movingPlatformBound)) {
					player.setOutlineColor(sf::Color::Red);
					player.setOutlineThickness(10);
					player.move(0.f, 2.f);
				}
				else {
					player.setOutlineColor(sf::Color::White);
					player.setOutlineThickness(1);
				}
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			{
				player.move(0.f, 1.f);
				if (playerBound.intersects(platformBound) || playerBound.intersects(movingPlatformBound)) {
					player.setOutlineColor(sf::Color::Red);
					player.setOutlineThickness(10);
					player.move(0.f, -2.f);
				}
				else {
					player.setOutlineColor(sf::Color::White);
					player.setOutlineThickness(1);
				}
			}


			//detectCollision(player, playerBound, platformBound, movingPlatformBound); 
			if (playerBound.intersects(platformBound) || playerBound.intersects(movingPlatformBound)) {
				player.setOutlineColor(sf::Color::Red);
				player.setOutlineThickness(10);
			}
			else {
				player.setOutlineColor(sf::Color::White);
				player.setOutlineThickness(1);
			}

			/*sf::Text text;
			sf::Font font;
			font.loadFromFile("arial.ttf");
			text.setFont(font);
			text.setCharacterSize(20);
			text.setString("HEYYYYY");
			text.setFillColor(sf::Color::Red);
			text.setStyle(sf::Text::Bold); */

			window.draw(movingPlatform);
			window.draw(player);
			window.draw(platform);
			//window.draw(text); 

			// end the current frame
			window.display();



		}


		

	}

	t1.join(); 

	return 0;
} 