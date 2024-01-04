#include <iostream>
#include <cmath>
#include <random>
#include <SFML/Graphics.hpp>

const double PI = std::atan(1.0f) * 4;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;

const double TAR_FPS = 60.1f;
const double TAR_DT = 1000000 / TAR_FPS;
const sf::Time TAR_FRAME_TIME = sf::microseconds(TAR_DT);

const double G = 0.0007;//0.00000000006674 m3 kg-1 s-2;		// Gravitationnal constant G in newtonian law of attraction
const double RHO = 1;					// Hypothetic density
const double EG = 9.81f / TAR_FPS;		// Calculated Earth equivalent G valid at target fps

const double REBOUND_EFFICIENCY = 0.3f;

const int EDGE = 4;
const int SIZE = EDGE * EDGE;
const double GRID_SUB_WIDTH = WINDOW_WIDTH / EDGE;
const double GRID_SUB_HEIGHT = WINDOW_HEIGHT / EDGE;

// Random generator + function
std::random_device rd;
std::mt19937 generator(rd());
double rand(double low, double high) {
	std::uniform_real_distribution<double> dist(low, high);
	return dist(generator);
}

enum Type {
	RANDOM					// DEFAULT
};

// Particle definition
struct Particle {
	double m_rad;
	double m_mass;
	sf::Vector2f m_pos, m_vel, m_acc;
	sf::CircleShape m_body;
	int m_gridX, m_gridY; // Grid Subdivision memory stored

	Particle(double rad, double posX, double posY, double velX, double velY, double accX, double accY)
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

	int interact(Particle& otherParticle) {
		if (otherParticle != *this) {
			// Apply newtonian law of attraction : F = G.m1.m2/r² & F = m.a => a += G.m2/r²
			sf::Vector2f linkVector = sf::Vector2f(otherParticle.m_pos.x - this->m_pos.x, otherParticle.m_pos.y - this->m_pos.y);
			double distance = sqrt(pow(otherParticle.m_pos.x - this->m_pos.x, 2) + pow(otherParticle.m_pos.y - this->m_pos.y, 2));
			if (distance <= 2.f) { distance = 2.f; }
			sf::Vector2f normedVector = linkVector / (float)distance;

			this->m_acc += normedVector * (float)(G * otherParticle.m_mass / pow(distance, 2));

			double interactionStrength = 255 * (WINDOW_WIDTH / EDGE - distance) / (WINDOW_WIDTH / EDGE);
			if (interactionStrength < 0) { interactionStrength = 0; }
			else if (interactionStrength > 255) { interactionStrength = 255; }
			return (int)interactionStrength;

			// TODO inter-particle collisions
		}

		return 0;
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

	// Initialize a position sum and a particle sum that will add up at each interaction
	// so we can substract the average position relative to the center so all particles stay centered
	double relPositionSum = 0.f;
	double numberOfParticles = 0.f;

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
			// Through vectors (iterate in reverse to handle erasing)
			for (int y = static_cast<int>(grid[x].size()) - 1; y >= 0; --y) {
				// Additional check to ensure the index is within bounds
				if (y >= 0 && y < grid[x].size()) {
					Particle& p = grid[x][y];
					p.resetForces();
					// Through neighboring areas
					for (int j = p.m_gridY * EDGE + p.m_gridX - 1; j <= p.m_gridY * EDGE + p.m_gridX + 1; j++) {
						for (int k = j - EDGE; k <= j + EDGE; k += EDGE) {
							// Only if in bounds
							if (k >= 0 && k < SIZE) {
								// Through all particles in neighboring areas
								for (auto& otherP : grid[k]) {
									if (otherP != p) {
										// Interact and draw each interaction line
										uint8_t interactionLineOpacity = p.interact(otherP);
										std::vector<sf::Vertex> interactionLines{
												sf::Vertex(p.m_pos, sf::Color::Color(255, 255, 255, interactionLineOpacity)),
												sf::Vertex(otherP.m_pos, sf::Color::Color(255, 255, 255, interactionLineOpacity))
										};
										window.draw(interactionLines.data(), interactionLines.size(), sf::Lines);
																		
									}
								}
							}
						}
					}
					// draw velocity vector
					std::vector<sf::Vertex> velLines{
							sf::Vertex(p.m_pos, sf::Color::Color(0, 0, 255, 255)),
							sf::Vertex(p.m_pos + 100.f * p.m_vel, sf::Color::Color(0, 0, 255, 255))
					};
					window.draw(velLines.data(), velLines.size(), sf::Lines);

					// draw acceleration vector
					std::vector<sf::Vertex> accelLines{
							sf::Vertex(p.m_pos, sf::Color::Color(255, 0, 0, 255)),
							sf::Vertex(p.m_pos + 10000.f * p.m_acc, sf::Color::Color(255, 0, 0, 255))
					};
					window.draw(accelLines.data(), accelLines.size(), sf::Lines);

					// Check if particle has changed grid slot position and moves the object to its new valid vector
					if (p.changedGridSub()) {
						// The particle has changed grid slot, move it to the new vector
						int newGridIndex = p.m_gridY * EDGE + p.m_gridX;

						// Ensure the newGridIndex is within bounds
						if (newGridIndex >= 0 && newGridIndex < SIZE) {
							grid[newGridIndex].emplace_back(std::move(p));
							grid[x].erase(grid[x].begin() + y);
							y--;
						}
						else {
							// Print a message or handle the out-of-bounds case
							std::cout << "Warning: Index out of bounds for grid[" << newGridIndex << "]   P coordinates : [" << p.m_pos.x << "][" << p.m_pos.y << "]" << std::endl;
						}
					}
				} else {
					// Print a message or handle the out-of-bounds case
					std::cout << "Warning: Index out of bounds for grid[" << x << "][" << y << "]" << std::endl;
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