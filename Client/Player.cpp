#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <chrono>
#include <iostream>

using namespace std::chrono;

class Player : public sf::CircleShape {
public:

	int clientID; 
	Player(float num) : CircleShape(num) {
		this->clientID = 0; 
	}


};