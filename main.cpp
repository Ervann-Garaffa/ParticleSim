#include <iostream>
#include <random>
#include <SFML/Graphics.hpp>

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 1000;

const float TAR_FPS = 144.1;
const float TAR_DT = 1000000 / TAR_FPS;
const float G = 9.81 / TAR_FPS;

const float REBOUND_EFFICIENCY = 0.9;

// Random generator + function
std::random_device rd;
std::mt19937 generator(rd());
float rand(float low, float high) {
	std::uniform_real_distribution<float> dist(low, high);
	return dist(generator);
}

// Particle definition
struct Particle {
	float m_rad;
	sf::Vector2f m_pos, m_vel, m_acc;
	sf::CircleShape m_body;

	Particle(float rad, float posX, float posY, float velX, float velY, float accX, float accY)
		: m_rad(rad), m_pos(posX, posY), m_vel(velX, velY), m_acc(accX, accY)
	{
		m_body.setOrigin(m_rad, m_rad);
		m_body.setFillColor(sf::Color::Red);
		m_body.setRadius(m_rad);
		m_body.setPosition(m_pos);
	}

	void resetForces() {
		m_acc.x = 0;
		m_acc.y = G;
	}

	void containerCollisions() {
		if (m_pos.x < 0 + m_rad) {
			m_pos.x = 0 + m_rad;
			if (m_vel.x < 0)
				m_vel.x *= -REBOUND_EFFICIENCY;
		}
		if (m_pos.x > WINDOW_WIDTH - m_rad) {
			m_pos.x = WINDOW_WIDTH - m_rad;
			if (m_vel.x > 0)
				m_vel.x *= -REBOUND_EFFICIENCY;
		}
		if (m_pos.y < 0 + m_rad) {
			m_pos.y = 0 + m_rad;
			if (m_vel.y < 0)
				m_vel.y *= -REBOUND_EFFICIENCY;
		}
		if (m_pos.y > WINDOW_HEIGHT - m_rad) {
			m_pos.y = WINDOW_HEIGHT - m_rad;
			if (m_vel.y > 0)
				m_vel.y *= -REBOUND_EFFICIENCY;
		}
	}

	void update() {
		this->resetForces();
		this->containerCollisions();
		m_vel += m_acc;
		m_pos += m_vel;
		m_body.setPosition(m_pos);
	}

	void render(sf::RenderWindow& window) {
		window.draw(m_body);
	}
};

int main() {
	sf::RenderWindow window(sf::VideoMode(1000, 1000), "Hello");

	sf::Font displayFont;
	if (!displayFont.loadFromFile("SpaceMono-Bold.ttf"))
		return -42;

	sf::Clock simClock;

	sf::Text fpsCounter;
	fpsCounter.setFillColor(sf::Color::White);
	fpsCounter.setFont(displayFont);
	fpsCounter.setCharacterSize(15);
	fpsCounter.setPosition(20, 20);

	Particle part(15, 500, 50, 0, 0, 0, 0);

	while (window.isOpen()) {
		simClock.restart();
		
		sf::Event event;
		while (window.pollEvent(event))
			if (event.type == sf::Event::Closed)
				window.close();

		window.clear();
		part.update();
		part.render(window);

		while (simClock.getElapsedTime().asMicroseconds() < TAR_DT) {}
		fpsCounter.setString(std::to_string(1000000 / simClock.restart().asMicroseconds()).append(" FPS"));
		window.draw(fpsCounter);

		window.display();
	}

	return EXIT_SUCCESS;
}