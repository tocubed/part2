#pragma once

#include <SFML/Graphics.hpp>

#include <memory>
#include <iostream>

struct CDrawable
{
	std::shared_ptr<sf::Drawable> drawable;
};

class GenericDrawable : public sf::Drawable, public sf::Transformable
{
public:
	GenericDrawable(sf::VertexArray& vertices, sf::Texture& texture)
		: vertices(vertices), texture(texture)
	{}

private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		states.transform *= getTransform();
		states.texture = &texture;

		target.draw(vertices, states);
	}

private:
	sf::VertexArray& vertices;
	sf::Texture& texture;
};
