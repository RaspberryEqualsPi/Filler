#pragma once
#include <map>
#include <string>
#include "ArcadeTTF.h"
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <windows.h>
#include "RoundedRectangle.h"
bool appInFocus(sf::RenderWindow* app) // credits to someone on the sfml forums (forgot their username)
{
	if (app == NULL)
		return false;
	HWND handle = app->getSystemHandle();
	bool one = handle == GetFocus();
	bool two = handle == GetForegroundWindow();
	if (one != two)
	{
		SetFocus(handle);
		SetForegroundWindow(handle);
	}
	return one && two;
}

namespace FillerUI {
	bool clicking = false;
	bool rclicking = false;
	void SetClicking(bool state) {
		clicking = state;
	}
	void SetRightClicking(bool state) {
		rclicking = state;
	}
	struct TextButton {
		int cornerPoints = 10;
		sf::Color textColor = sf::Color::Black;
		int textSize = 8;
		std::string text;
		float sizeX = 90;
		float sizeY = 32;
		float radius = sizeX / 10;
		std::string font = "data/fonts/Arcade.ttf";
		sf::Text sfText;
		sf::Font sfFont;
		sf::RoundedRectangleShape rect = sf::RoundedRectangleShape(sf::Vector2f(sizeX, sizeY), radius, cornerPoints);
		void draw(sf::RenderWindow* window) {
			rect.setSize(sf::Vector2f(sizeX, sizeY));
			rect.setCornerPointCount(cornerPoints);
			rect.setCornersRadius(radius);
			sfFont.loadFromMemory(ArcadeTTF, ArcadeTTF_length);
			sfText.setFont(sfFont);
			sfText.setString(text);
			sfText.setCharacterSize(textSize);
			sfText.setFillColor(textColor);
			float textWidth = sfText.getGlobalBounds().width;
			float textHeight = sfText.getGlobalBounds().height;
			float buttonWidth = rect.getGlobalBounds().width;
			float buttonHeight = rect.getGlobalBounds().height;
			float buttonX = rect.getPosition().x;
			float buttonY = rect.getPosition().y;
			sfText.setPosition(((buttonWidth - textWidth) / 2) + buttonX, ((buttonHeight - textHeight) / 2) + buttonY);
			window->draw(rect);
			window->draw(sfText);
		}
		bool clicked(sf::RenderWindow* window) {
			auto mouse_pos = sf::Mouse::getPosition(*window);
			auto translated_pos = window->mapPixelToCoords(mouse_pos);
			return rect.getGlobalBounds().contains(translated_pos) && clicking && appInFocus(window);
		}
		bool hovering(sf::RenderWindow* window) {
			auto mouse_pos = sf::Mouse::getPosition(*window);
			auto translated_pos = window->mapPixelToCoords(mouse_pos);
			return rect.getGlobalBounds().contains(translated_pos) && appInFocus(window);
		}
	};
	struct HoverSensitiveTextButton {
		int cornerPoints = 10;
		sf::Color textColor = sf::Color::Black;
		int textSize = 8;
		std::string text;
		float sizeX = 90;
		float sizeY = 32;
		float radius = sizeX / 10;
		std::string font = "data/fonts/Arcade.ttf";
		sf::Text sfText;
		sf::Font sfFont;
		sf::RoundedRectangleShape rect = sf::RoundedRectangleShape(sf::Vector2f(sizeX, sizeY), radius, cornerPoints);
		bool hovering(sf::RenderWindow* window) {
			auto mouse_pos = sf::Mouse::getPosition(*window);
			auto translated_pos = window->mapPixelToCoords(mouse_pos);
			return rect.getGlobalBounds().contains(translated_pos) && appInFocus(window);
		}
		void draw(sf::RenderWindow* window) {
			rect.setSize(sf::Vector2f(sizeX, sizeY));
			rect.setCornerPointCount(cornerPoints);
			rect.setCornersRadius(radius);
			sfFont.loadFromMemory(ArcadeTTF, ArcadeTTF_length);
			sfText.setFont(sfFont);
			sfText.setString(text);
			sfText.setCharacterSize(textSize);
			sfText.setFillColor(textColor);
			float textWidth = sfText.getGlobalBounds().width;
			float textHeight = sfText.getGlobalBounds().height;
			float buttonWidth = rect.getGlobalBounds().width;
			float buttonHeight = rect.getGlobalBounds().height;
			float buttonX = rect.getPosition().x;
			float buttonY = rect.getPosition().y;
			sfText.setPosition(((buttonWidth - textWidth) / 2) + buttonX, ((buttonHeight - textHeight) / 2) + buttonY);
			if (hovering(window))
				rect.setFillColor(sf::Color(211, 211, 211));
			else
				rect.setFillColor(sf::Color::White);
			window->draw(rect);
			window->draw(sfText);
		}
		bool clicked(sf::RenderWindow* window) {
			auto mouse_pos = sf::Mouse::getPosition(*window);
			auto translated_pos = window->mapPixelToCoords(mouse_pos);
			return rect.getGlobalBounds().contains(translated_pos) && clicking && appInFocus(window);
		}
	};
}