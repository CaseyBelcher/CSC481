#include "Platform.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <chrono>
#include <iostream>

class Platform : public sf::RectangleShape {
	public:
		Platform(sf::Vector2f vector) : sf::RectangleShape(vector) {

		}
};