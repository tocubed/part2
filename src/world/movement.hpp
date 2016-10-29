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

	void announceMoved(EntityIndex who, CLocation location, Direction direction, 
	                   bool successful, bool complete) {
		auto moved = manager.createEntity();

		manager.addTag<TEvent>(moved);
		manager.addComponent<CEventMoved>(
		    moved, CEventMoved{who, location, direction, successful, complete});
	}

	void handleDesired()
	{
		manager.forEntitiesHaving<CMovement, CDesiredMovement, CLocation>(
		[this](EntityIndex eI)
		{
			auto& movement = manager.getComponent<CMovement>(eI);
			if(movement.moving)
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

			movement.direction = desired.direction;

			if(!overworld.isCollidable(destination))
			{
				movement.destination = destination;
				movement.moving = true;
			}

			announceMoved(eI, location, movement.direction, movement.moving, false);
		});

		manager.forEntitiesHaving<TFollower>(
			[this](EntityIndex eI1)
		{
			auto& movement = manager.getComponent<CMovement>(eI1);
			auto& desired = manager.getComponent<CDesiredMovement>(eI1);
			auto& location = manager.getComponent<CLocation>(eI1);
			auto destination = TileLocation::fromLocation(location);

			std::deque<CEventMoved> eventMovedList;
			manager.forEntitiesHaving<TEvent>(
				[this, &eventMovedList](EntityIndex eI2)
			{
				eventMovedList.push_back(manager.getComponent<CEventMoved>(eI2));
			});

			if (eventMovedList.size() == 0)
			{
				return;
			}

			for (std::deque<CEventMoved>::iterator it = eventMovedList.begin();
				it != eventMovedList.end(); ++it)
			{
				if (manager.getComponent<CFollowOrder>(eI1).entityAhead == (*it).who)
				{
					if (!((*it).successful) || (*it).complete) {
						break;
					}

					if (location.x < (*it).prevLocation.x
						&& location.y == (*it).prevLocation.y) {
						desired.direction = Direction::RIGHT;
						destination.x += 1;
						movement.moving = true;
						break;
					}

					if (location.x > (*it).prevLocation.x
						&& location.y == (*it).prevLocation.y) {
						desired.direction = Direction::LEFT;
						destination.x -= 1;
						movement.moving = true;
						break;
					}

					if (location.x == (*it).prevLocation.x
						&& location.y < (*it).prevLocation.y) {
						desired.direction = Direction::DOWN;
						destination.y += 1;
						movement.moving = true;
						break;
					}

					if (location.x == (*it).prevLocation.x
						&& location.y >(*it).prevLocation.y) {
						desired.direction = Direction::UP;
						destination.y -= 1;
						movement.moving = true;
						break;
					}

					else
					{
						movement.moving = false;
						break;
					}
				}
			}

			if (!movement.moving) {
				return;
			}

			movement.direction = desired.direction;
			movement.destination = destination;
			announceMoved(eI1, location, movement.direction, movement.moving, false);
		});
	}
	

	void move(sf::Time delta)
	{
		// TODO Fix this to work better with an animation system
		accumulator += delta;
		while(accumulator >= sf::milliseconds(8))
		{
			accumulator -= sf::milliseconds(8);
			manager.forEntitiesHaving<CMovement, CLocation>([this](EntityIndex eI)
			{
				auto& movement = manager.getComponent<CMovement>(eI);

				if(!movement.moving)
					return;

				auto destination = movement.destination.toLocation();
				auto& location = manager.getComponent<CLocation>(eI);

				if(destination.x == location.x && destination.y == location.y)
				{
					announceMoved(eI, location, movement.direction, movement.moving, true);
					movement.moving = false;
					return;
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
