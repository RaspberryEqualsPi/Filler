#include <map>
#include <string>
#include "ArrowPNG.h"
#include <SFML/Window.hpp>
#include "ArrowShooterPNG.h"
#include <SFML/Graphics.hpp>
int level = 1;
namespace ObjectEnums {
	namespace ArrowShooter {
		int left = 0;
		int up = 1;
		int right = 2;
		int down = 3;
	};
}
struct GhostData {
	int time;
	sf::Vector2i pPos;
};
class GhostDataArr : std::vector<GhostData> {
	int level;
};
struct Token {
	sf::CircleShape token;
	int x;
	int y;
	int timestouched = 0;
	bool debounce = true;
	sf::Color tokencolor = sf::Color::Yellow;
	void draw(sf::RenderWindow* window) {
		token.setFillColor(tokencolor);
		token.setRadius(8);
		token.setPosition(x * 16, y * 16);
		window->draw(token);
	}
};
struct Checkpoint {
	sf::CircleShape checkp;
	int x;
	int y;
	int timestouched = 0;
	sf::Color cpcolor = sf::Color(0, 255, 0);
	sf::Color activatedcolor = sf::Color(0, 128, 0);
	void draw(sf::RenderWindow* window) {
		if (timestouched > 0)
			checkp.setFillColor(activatedcolor);
		else
			checkp.setFillColor(cpcolor);
		checkp.setRadius(8);
		checkp.setPosition(x * 16, y * 16);
		window->draw(checkp);
	}
};
struct Arrow {
	sf::Texture Tarrow;
	sf::Sprite Sarrow;
	float x;
	float y;
	sf::Color tokencolor = sf::Color::Yellow;
	void draw(sf::RenderWindow* window) {
		Tarrow.loadFromMemory(ArrowPNG, ArrowPNG_length);
		Sarrow.setTexture(Tarrow);
		Sarrow.setPosition(x * 16, y * 16);
		window->draw(Sarrow);
	}
};
struct ArrowShooter {
	sf::Texture Tarrowshooter;
	sf::Sprite Sarrowshooter;
	Arrow arrow;
	int x;
	int y;
	int type = 0;
	bool stop = false;
	sf::Color tokencolor = sf::Color::Yellow;
	void draw(sf::RenderWindow* window) {
		Tarrowshooter.loadFromMemory(ArrowShooterPNG, ArrowShooterPNG_length);
		Sarrowshooter.setTexture(Tarrowshooter);
		Sarrowshooter.setOrigin(0, 0);
		if ((arrow.y <= 0 || arrow.y > 20) || (arrow.x <= 0 || arrow.x > 20)) {
			float Width1 = 17;
			float Width2 = 14;
			float X2 = Sarrowshooter.getPosition().x;
			float Height1 = 16;
			float Height2 = 16;
			float Y2 = Sarrowshooter.getPosition().y;
			arrow.x = X2 / 16;
			arrow.y = Y2 / 16;
			if(type == 1)
				arrow.y--;
			if(type == 2)
				arrow.x++;
			if(type == 3)
				arrow.y++;
			if(type == 0)
				arrow.x--;
		}
		if (type == 1) {
			Sarrowshooter.setPosition(x * 16, y * 16 + 16);
		}
		if (type == 2) {
			Sarrowshooter.setPosition(x * 16, y * 16);
		}
		if (type == 3) {
			Sarrowshooter.setPosition(x * 16 + 16, y * 16);
		}
		if (type == 0) {
			Sarrowshooter.setPosition(x * 16 + 16, y * 16 + 16);
		}
		Sarrowshooter.setOrigin(Sarrowshooter.getGlobalBounds().width, Sarrowshooter.getGlobalBounds().height);
		Sarrowshooter.setRotation(type * 90);
		arrow.Sarrow.setOrigin(0, 0);
		arrow.Sarrow.setRotation(type * 90 - 90);
		if (!stop) {
			if (type == ObjectEnums::ArrowShooter::up) {
				arrow.y -= 0.2 * (level - (level / 1.2));
			}
			if (type == ObjectEnums::ArrowShooter::down) {
				arrow.y += 0.2 * (level - (level / 1.2));
			}
			if (type == ObjectEnums::ArrowShooter::right) {
				arrow.x += 0.2 * (level - (level / 1.2));
			}
			if (type == ObjectEnums::ArrowShooter::left) {
				arrow.x -= 0.2 * (level - (level / 1.2));
			}
		}
		arrow.draw(window);
		window->draw(Sarrowshooter);
	}
};
struct Player {
	sf::CircleShape player;
	int x;
	int y;
	void draw(sf::RenderWindow* window) {
		player.setFillColor(sf::Color::Red);
		player.setRadius(8);
		player.setPosition(x * 16, y * 16);
		window->draw(player);
	}
	Player(int x1, int y1) {
		x = x1;
		y = y1;
	}
};