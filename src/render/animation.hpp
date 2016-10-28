#pragma once

#include <core/manager.hpp>
#include <core/system.hpp>
#include <render/animatedsprite.hpp>

class AnimationSystem : System
{
public:
	AnimationSystem(Manager& manager)
		: System(manager)
	{
	}

	void handleCharacterIdle()
	{
		manager.forEntitiesHaving<TAnimated, CMovement>([this](EntityIndex eI)
		{
			auto& drawable = manager.getComponent<CDrawable>(eI);
			auto& animatedSprite = 
					static_cast<AnimatedSprite&>(*(drawable.drawable));

			auto& movement = manager.getComponent<CMovement>(eI);

			if(movement.moving || animatedSprite.isPlaying())
				return;

			std::string animationName = "idle";
			switch(movement.direction)
			{
			case UP:
				animationName += "_up"; break;
			case DOWN:
				animationName += "_down"; break;
			case RIGHT:
				animationName += "_right"; break;
			case LEFT:
				animationName += "_left"; break;
			// TODO Discourage or eliminate NONE usage
			case NONE:
				movement.direction = DOWN;
				animationName += "_down"; break;
			}

			animatedSprite.playAnimation(animationName, true);
			animatedSprite.setPlaySpeed(sf::milliseconds(1000));
		});
	}

	void handleCharacterMovement()
	{
		manager.forEntitiesHaving<TEvent, CEventMoved>([this](EntityIndex eI)
		{
			auto& moved = manager.getComponent<CEventMoved>(eI);

			if(manager.hasTag<TAnimated>(moved.who) &&
			   manager.hasComponent<CMovement>(moved.who))
			{
				auto& drawable = manager.getComponent<CDrawable>(moved.who);
				auto& animatedSprite = 
					static_cast<AnimatedSprite&>(*(drawable.drawable));

				if(moved.complete)
				{
					animatedSprite.stopAnimation();
					return;
				}

				std::string animationName = "walk_";

				// TODO Centralize this conversion somewhere
				switch(moved.direction)
				{
				case UP:
					animationName += "up"; break;
				case DOWN:
					animationName += "down"; break;
				case RIGHT:
					animationName += "right"; break;
				case LEFT:
					animationName += "left"; break;
				}

				animatedSprite.playAnimation(animationName, false);
				animatedSprite.setPlaySpeed(sf::milliseconds(25));
			}
		});
	}

	void animate(sf::Time delta)
	{
		manager.forEntitiesHaving<TAnimated, CDrawable>(
		[this, delta](EntityIndex eI)
		{
			auto& drawable = manager.getComponent<CDrawable>(eI);

			auto& animatedSprite = 
				static_cast<AnimatedSprite&>(*(drawable.drawable));

			animatedSprite.update(delta);
		});
	}

	void update(sf::Time delta)
	{
		handleCharacterMovement();
		handleCharacterIdle();
		animate(delta);
	}
};
