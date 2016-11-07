#pragma once

#include <SFML/Graphics.hpp>

namespace Render {

static sf::Texture* createBoxFrame(
    const sf::Texture& border, const sf::Texture& background,
    sf::Vector2i dimensions) 
{
	auto xOffset = border.getSize().x;
	auto yOffset = border.getSize().y;

	sf::Sprite borderSprite(border, {0, 0, dimensions.x, dimensions.y});
	sf::Sprite bgSprite(
	    background,
	    {0, 0, dimensions.x - 2 * xOffset, dimensions.y - 2 * yOffset});

	sf::RenderTexture render;
	render.create(dimensions.x, dimensions.y); 

	render.draw(borderSprite);

	auto transformed = sf::RenderStates::Default;
	transformed.transform *= sf::Transform().translate(xOffset, yOffset);

	render.draw(bgSprite, transformed);

	return new sf::Texture(render.getTexture());
}

} // namespace Render
