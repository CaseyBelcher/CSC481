#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <chrono>
#include <iostream>

class MovingPlatform : public sf::RectangleShape {
public:

	int id;
	MovingPlatform(sf::Vector2f vector) : sf::RectangleShape(vector) {
		this->id = 0;
	}

	void myMove(float x, float y) {
		move(x, y); 
	}
};