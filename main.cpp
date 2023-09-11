#include <iostream>
#include <SFML/Graphics.hpp>

struct Particle {
	int m_rad;
	sf::Vector2f m_pos, m_vel, m_acc;
	sf::CircleShape m_body;

	Particle(int rad, float posX, float posY, float velX, float velY, float accX, float accY)
		: m_rad(rad), m_pos(posX, posY), m_vel(velX, velY), m_acc(accX, accY)
	{
		m_body.setOrigin(m_rad, m_rad);
		m_body.setFillColor(sf::Color::Red);
		m_body.setRadius(m_rad);
		m_body.setPosition(m_pos);
	}
};

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