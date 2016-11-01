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

	void announceMoved(EntityIndex who, Direction direction, 
	                   bool successful, bool complete) {
		auto moved = manager.createEntity();

		manager.addTag<TEvent>(moved);
		manager.addComponent<CEventMoved>(
		    moved, CEventMoved{who, direction, successful, complete});
	}

	void handleFollowers(
	    EntityIndex beingFollowed, TileLocation prevLocation,
	    const std::vector<std::pair<EntityIndex, EntityIndex>>& followers) 
	{
		for(auto& pair: followers)
		{
			auto follower = pair.first;
			auto following = pair.second;	

			if(following != beingFollowed)
				continue;

			TileLocation followerStart;
			TileLocation followerEnd = prevLocation;

			auto& movement = manager.getComponent<CMovement>(follower);
			auto& location = manager.getComponent<CLocation>(follower);

			if(movement.moving)
				followerStart = movement.destination;
			else
				followerStart = TileLocation::fromLocation(location);

			auto xDiff = followerEnd.x - followerStart.x;
			auto yDiff = followerEnd.y - followerStart.y;

			auto& desired = manager.getComponent<CDesiredMovement>(follower);

			if(xDiff == 0)
			{
				if(yDiff > 0)
					desired.direction = DOWN;
				else if(yDiff < 0)
					desired.direction = UP;
			}
			else if(yDiff == 0)
			{
				if(xDiff > 0)
					desired.direction = RIGHT;
				else if(xDiff < 0)
					desired.direction = LEFT;
			}
		}
	}

	void handleDesired()
	{
		std::vector<std::pair<EntityIndex, EntityIndex>> followers;
		manager.forEntitiesHaving<TFollower, CFollowOrder>(
		[this, &followers](EntityIndex eI)
		{
			auto& followOrder = manager.getComponent<CFollowOrder>(eI);	

			followers.push_back(std::make_pair(eI, followOrder.entityAhead));
		});

		manager.forEntitiesHaving<CMovement, CDesiredMovement, CLocation>(
		[this, &followers](EntityIndex eI)
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
			desired.direction = NONE;

			if(!overworld.isCollidable(destination))
			{
				movement.destination = destination;
				movement.moving = true;

				handleFollowers(
				    eI, TileLocation::fromLocation(location), followers);
			}

			announceMoved(eI, movement.direction, movement.moving, false);
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
					announceMoved(eI, movement.direction, movement.moving, true);
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
