#include "MovingPlatform.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <chrono>
#include <iostream>

class MovingPlatform : public sf::RectangleShape {
public:
	MovingPlatform(sf::Vector2f vector) : sf::RectangleShape(vector) {

	}
};