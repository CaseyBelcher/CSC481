#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <chrono>
#include <iostream>

using namespace std;


class Platform : public sf::RectangleShape {
public:

	int id;
	string type; 
	Platform(sf::Vector2f vector, string type) : sf::RectangleShape(vector) {
		this->id = 0;
		this->type = type; 
	}
};