#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <chrono>
#include <iostream>

using namespace std::chrono;

class Player : public sf::CircleShape {
public:

	int clientID;
	int lastPing = 0; 
	Player(float num) : CircleShape(num) {
		this->clientID = 0; 
	}

	int getClientID() {
		return this->clientID; 
	}

	void myMove(float x, float y) {
		move(x, y); 
	}
	
	void mySetPosition(float x, float y) {
		setPosition(x, y); 
	}


};