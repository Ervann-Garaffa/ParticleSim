#include <iostream>
#include <cmath>
#include <random>
#include <SFML/Graphics.hpp>

const double PI = std::atan(1.0f) * 4;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;

const float TAR_FPS = 60.1f;
const float TAR_DT = 1000000 / TAR_FPS;
const sf::Time TAR_FRAME_TIME = sf::microseconds(TAR_DT);

const double G = 0.0007;//0.00000000006674 m3 kg-1 s-2;		// Gravitationnal constant G in newtonian law of attraction
const float RHO = 1;					// Hypothetic density
const float EG = 9.81f / TAR_FPS;		// Calculated Earth equivalent G valid at target fps

const float REBOUND_EFFICIENCY = 0.8f;

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

enum Type {
	RANDOM					// DEFAULT
};

// Particle definition
struct Particle {
	float m_rad;
	float m_mass;
	sf::Vector2f m_pos, m_vel, m_acc;
	sf::CircleShape m_body;
	int m_gridX, m_gridY; // Grid Subdivision memory stored

	Particle(float rad, float posX, float posY, float velX, float velY, float accX, float accY)
	: m_rad(rad), m_mass(RHO*4*rad*rad*rad*PI/3), m_pos(posX, posY), m_vel(velX, velY), m_acc(accX, accY), 
	  m_gridX(posX/GRID_SUB_WIDTH), m_gridY(posY/GRID_SUB_HEIGHT)
	{
		m_body.setOrigin(m_rad, m_rad);
		m_body.setFillColor(sf::Color::Red);
		m_body.setRadius(m_rad);
		m_body.setPosition(m_pos);
	}

	Particle(Type particleType)
	{
		switch (particleType) {
		default:
			m_rad = rand(0.1f, 20);
			m_mass = RHO * 4 * m_rad * m_rad * m_rad * PI / 3;
			m_pos = sf::Vector2f(rand(0, WINDOW_WIDTH), rand(0, WINDOW_HEIGHT));
			m_vel = sf::Vector2f(0, 0);//sf::Vector2f(rand(-0.5f, 0.5f), rand(-0.5f, 0.5f));
			m_acc = sf::Vector2f(0, 0);
			m_gridX = m_pos.x / GRID_SUB_WIDTH;
			m_gridY = m_pos.y / GRID_SUB_HEIGHT;
			break;
		}
		
		m_body.setOrigin(m_rad, m_rad);
		m_body.setFillColor(sf::Color::Red);
		m_body.setRadius(m_rad);
		m_body.setPosition(m_pos);
	}

	void resetForces() {
		m_acc.x = 0;
		m_acc.y = 0; // to enable earth like gravity, replace by constant EG
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

		if (newGridX != m_gridX || newGridY != m_gridY) {
			m_gridX = m_pos.x / GRID_SUB_WIDTH;
			m_gridY = m_pos.y / GRID_SUB_HEIGHT;
			return true; // Grid subdivision has changed
		}

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

	void interact(Particle& otherParticle) {
		if (otherParticle != *this) {
			// Apply newtonian law of attraction : F = G.m1.m2/r² & F = m.a => a += G.m2/r²
			sf::Vector2f linkVector = sf::Vector2f(otherParticle.m_pos.x - this->m_pos.x, otherParticle.m_pos.y - this->m_pos.y);
			float distance = (float)sqrt(pow(otherParticle.m_pos.x - this->m_pos.x, 2) + pow(otherParticle.m_pos.y - this->m_pos.y, 2));
			if (distance <= 2.f) { distance = 2.f; }
			sf::Vector2f normedVector = linkVector / distance;

			this->m_acc += normedVector * (float)(G * otherParticle.m_mass / pow(distance, 2));

			// TODO inter-particle collisions
		}
	}

	void update() {
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

	// Dynamically sized array with automatic memory management
	std::vector<std::vector<Particle>> grid;
	grid.resize(SIZE);

	std::vector<Particle> temp; // used as a buffer to store Particles to be moved in new arrays

	simClock.restart();
	
	while (window.isOpen()) {
		
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
			// Create particles at mouse position at mouse button press
			//if (event.type == sf::Event::MouseButtonPressed)
			//	temp.emplace_back(rand(1, 20), event.mouseButton.x, event.mouseButton.y, 0, 0, 0, 0);
		}

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
			// Create randomly placed particles every frame in a grid slot
			Particle partTemp(RANDOM);
			grid[partTemp.m_gridY * EDGE + partTemp.m_gridX].emplace_back(partTemp);
			// Create particles at mouse position every frame
			//temp.emplace_back(rand(1, 50), sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y, 0, 0, 0, 0);
		}

		window.clear();

		// Through grid
		for (int x = 0; x < SIZE; x++) {
			// Through vectors
			for (int y = 0; y < grid[x].size(); y++) {
				Particle& p = grid[x][y];
				p.resetForces();
				// Through neighboring areas
				for (int j = p.m_gridY * EDGE + p.m_gridX - 1; j <= p.m_gridY * EDGE + p.m_gridX + 1; j++) {
					for (int k = j - EDGE; k <= j + EDGE; k += EDGE) {
						// Only if in bounds
						if (k >= 0 && k < SIZE) {
							// Through all particles in neighboring areas
							for (auto& otherP : grid[k]) {
								// Prevents self interaction
								if (otherP != p) {
									std::vector<sf::Vertex> lines{
										sf::Vertex(p.m_pos, sf::Color::White),
										sf::Vertex(otherP.m_pos, sf::Color::White)
									}; 
									window.draw(lines.data(), lines.size(), sf::Lines);

									p.interact(otherP);
								}
							}
						}
					}
				}
				// Check if particle has changed grid slot position and moves the object to its new valid vector
				if (p.changedGridSub()) {
					grid[p.m_gridY * EDGE + p.m_gridX].emplace_back(std::move(p));
					grid[x].erase(grid[x].begin() + y);
					y--;
				}
			}
		}
		
		for (int i = 0; i < SIZE; i++) {
			for (auto& p : grid[i]) {
				p.update();
				p.render(window);
			}
		}

		while (simClock.getElapsedTime().asMicroseconds() < TAR_DT) {}
		fpsCounter.setString(std::to_string(1000000 / simClock.restart().asMicroseconds()).append(" FPS"));
		window.draw(fpsCounter);

		window.display();
	}

	return EXIT_SUCCESS;
}