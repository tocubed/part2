#pragma once

#include <core/system.hpp>


class InputSystem : System
{
public:
	InputSystem(Manager& manager)
		: System(manager)
	{}

	void update(sf::Time delta)
	{
		manager.forEntitiesHaving<TPlayer> ([this](EntityIndex eI)
		{
			auto &reference = manager.getComponent<CDesiredMovement>(eI);
			
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			{
				reference.direction = reference.Direction::UP;
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			{
				reference.direction = reference.Direction::DOWN;
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			{
				reference.direction = reference.Direction::LEFT;
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			{
				reference.direction = reference.Direction::RIGHT;
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
			{
				// Do some interaction 
			}
		});
	}
};
