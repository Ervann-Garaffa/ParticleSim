#include <iostream>
#include <random>
#include <SFML/Graphics.hpp>

const int WINDOW_WIDTH = 3000;
const int WINDOW_HEIGHT = 1200;

const float TAR_FPS = 144.1;
const float TAR_DT = 1000000 / TAR_FPS;
const float G = 9.81 / TAR_FPS;
const float REBOUND_EFFICIENCY = 0.99;
const int EDGE = 5;
const int SIZE = EDGE * EDGE;
const float GRID_SUB_WIDTH = WINDOW_WIDTH / EDGE;
const float GRID_SUB_HEIGHT = WINDOW_HEIGHT / EDGE;

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
	int m_gridX, m_gridY; // Grid Subdivision memory stored

	Particle(float rad, float posX, float posY, float velX, float velY, float accX, float accY)
	: m_rad(rad), m_pos(posX, posY), m_vel(velX, velY), m_acc(accX, accY), 
	  m_gridX(posX/GRID_SUB_WIDTH), m_gridY(posY/GRID_SUB_HEIGHT)
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

	bool operator!=(const Particle &other) const {
		if (this->m_rad != other.m_rad ||
			this->m_pos != other.m_pos ||
			this->m_vel != other.m_vel)
			return true;
		return false;
	}

	bool changedGridSub() {
		int newGridX = m_pos.x / GRID_SUB_WIDTH;
		int newGridY = m_pos.y / GRID_SUB_HEIGHT;

		if (newGridX != m_gridX || newGridY != m_gridY)
			return true; // Grid subdivision has changed

		return false; // Grid subdivision has not changed
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
	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Hello");

	sf::Font displayFont;
	if (!displayFont.loadFromFile("res/SpaceMono-Bold.ttf"))
		return -42;

	sf::Clock simClock;
	
	sf::Text fpsCounter;
	fpsCounter.setFillColor(sf::Color::White);
	fpsCounter.setFont(displayFont);
	fpsCounter.setCharacterSize(15);
	fpsCounter.setPosition(20, 20);

	std::vector<Particle>** grid = new std::vector<Particle>*[SIZE];
	std::vector<Particle> temp;

	while (window.isOpen()) {
		simClock.restart();
		
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
			//if (event.type == sf::Event::MouseButtonPressed)
			//	temp.emplace_back(rand(1, 20), event.mouseButton.x, event.mouseButton.y, 0, 0, 0, 0);
		}

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
			temp.emplace_back(rand(1, 50), sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y, 0, 0, 0, 0);

		window.clear();

		for (auto &p : temp) {
			p.update();
			p.render(window);
		}

		while (simClock.getElapsedTime().asMicroseconds() < TAR_DT) {}
		fpsCounter.setString(std::to_string(1000000 / simClock.restart().asMicroseconds()).append(" FPS"));
		window.draw(fpsCounter);

		window.display();
	}

	return EXIT_SUCCESS;
}