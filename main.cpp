#include <iostream>
#include <SFML/Graphics.hpp>

int main() {
	sf::RenderWindow window(sf::VideoMode(1000, 1000), "Hello");

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear();

		window.display();
	}

	return EXIT_SUCCESS;
}