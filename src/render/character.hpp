#pragma once

#include <core/manager.hpp>
#include <render/animatedsprite.hpp>

#include <string>

namespace Character
{
	void loadWalkingAnimations(AnimatedSprite* sprite);

	// TODO Combine similar code
	void loadIdleAnimations(AnimatedSprite* sprite);

	AnimatedSprite* loadAnimations(std::string file);

	// TODO This should go under world/character.hpp
	EntityIndex createCharacter(Manager& manager, std::string animFile);

} // namespace Character
