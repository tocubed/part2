#pragma once 

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <render/boxframe.hpp>

#include <string>
#include <vector>

class MenuBox : public sf::Drawable, public sf::Transformable
{
public:
	MenuBox(sf::Font& font, 
		const sf::Texture& boxBorder, const sf::Texture& boxBackground)
		: font(font), boxBorder(boxBorder), boxBackground(boxBackground) 
	{
	}

	void setOptions(const std::vector<std::string>& options)
	{
		auto maxWidth = 0u;
		for(auto option: options)
		{
			lines.emplace_back();

			auto& text = lines.back();

			text.setFont(font);
			text.setCharacterSize(24);
			text.setString(option);
			text.move(24, 0);

			text.setStyle(sf::Text::Style::Bold);
			auto width = text.getLocalBounds().width;
			text.setStyle(sf::Text::Style::Regular);

			if(width >= maxWidth)
				maxWidth = width;
		}

		dimensions.x = 16 * (maxWidth / 16 + 1) + 48;
		dimensions.y = 32 * (options.size() + 1);

		box.setTexture(
		    *Render::createBoxFrame(boxBorder, boxBackground, dimensions));

		setCurrentChoice(0);
	}

	void setCurrentChoice(std::size_t i)
	{
		auto prev = currentChoice;
		currentChoice = i;

		lines[prev].setStyle(sf::Text::Style::Regular);
		lines[currentChoice].setStyle(sf::Text::Style::Bold);
	}

	std::size_t getCurrentChoice() const
	{
		return currentChoice;
	}

	void increaseChoice()
	{
		setCurrentChoice((currentChoice + 1u) % lines.size());
	}
	
	void decreaseChoice()
	{
		setCurrentChoice((currentChoice + lines.size() - 1u) % lines.size());
	}

private:
	sf::Sprite box;
	const sf::Texture& boxBorder;
	const sf::Texture& boxBackground;

	sf::Font& font;

	sf::Vector2i dimensions;

private:
	mutable std::vector<sf::Text> lines;

	std::size_t currentChoice;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		states.transform *= getTransform();

		target.draw(box, states);

		states.transform *= sf::Transform().translate(0, 16);

		for(auto& text: lines)
		{
			target.draw(text, states); 
			states.transform *= sf::Transform().translate(0, 32);
		}
	}
};
