#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <chrono>
#include <iostream>

class Platform : public sf::RectangleShape {
public:

	int id;
	Platform(sf::Vector2f vector) : sf::RectangleShape(vector) {
		this->id = 0;
	}
};