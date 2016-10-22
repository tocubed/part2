#pragma once

#include <core/manager.hpp>
#include <core/system.hpp>
#include <world/overworld.hpp>

#include <iostream>

class MovementSystem : System
{
public:
	MovementSystem(Manager& manager, Overworld& overworld)
		: System(manager), overworld(overworld)
	{
	}

	void handleDesired()
	{
		manager.forEntitiesHaving<CMovement, CDesiredMovement, CLocation>(
		[this](EntityIndex eI)
		{
			auto& movement = manager.getComponent<CMovement>(eI);
			if(movement.direction != Direction::NONE)
				return;

			auto& desired = manager.getComponent<CDesiredMovement>(eI);
			auto& location = manager.getComponent<CLocation>(eI);

			auto destination = TileLocation::fromLocation(location);

			switch(desired.direction)
			{
			case UP:
				destination.y -= 1; break;
			case DOWN:
				destination.y += 1; break;
			case RIGHT:
				destination.x += 1; break;
			case LEFT:
				destination.x -= 1; break;
			default:
				return;
			}

			if(overworld.isCollidable(destination))
				return; // TODO Add a bump animation when movement not allowed

			movement.direction = desired.direction;
			movement.destination = destination;
		});
	}

	void move(sf::Time delta)
	{
		// TODO Fix this to work better with an animation system
		accumulator += delta;
		while(accumulator >= sf::milliseconds(15))
		{
			accumulator -= sf::milliseconds(15);
			manager.forEntitiesHaving<CMovement, CLocation>([this](EntityIndex eI)
			{
				auto& movement = manager.getComponent<CMovement>(eI);
				auto& location = manager.getComponent<CLocation>(eI);

				auto destination = movement.destination.toLocation();
				if(destination.x == location.x && destination.y == location.y)
				{
					movement.direction = Direction::NONE;
					movement.destination = {};
				}

				switch(movement.direction)
				{
				case UP:
					location.y -= 1; break;
				case DOWN:
					location.y += 1; break;
				case RIGHT:
					location.x += 1; break;
				case LEFT:
					location.x -= 1; break;
				default:
					return;
				}
			});
		}
	}

	void update(sf::Time delta)
	{
		handleDesired();
		move(delta);
	}

private:
	Overworld& overworld;

	sf::Time accumulator;
};
