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

	void npcWalkAnimations()
	{
		manager.forEntitiesHaving<TEvent, CEventMoved>([this](EntityIndex eI)
		{
			auto& moved = manager.getComponent<CEventMoved>(eI);

			if(manager.hasTag<TPlayer>(moved.who) && // TODO Should work for NPcs
			   manager.hasComponent<CDrawable>(moved.who))
			{
				auto& drawable = manager.getComponent<CDrawable>(moved.who);
				auto& animatedSprite = 
					static_cast<AnimatedSprite&>(*(drawable.drawable));

				std::string animationName = "walk";
				switch(moved.direction)
				{
				case UP:
					animationName += "_up"; break;
				case DOWN:
					animationName += "_down"; break;
				case RIGHT:
					animationName += "_right"; break;
				case LEFT:
					animationName += "_left"; break;
				}

				if(moved.successful)
					; // TODO Add fail move animations

				animatedSprite.playAnimation(animationName);
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
		npcWalkAnimations();
		animate(delta);
	}
};
