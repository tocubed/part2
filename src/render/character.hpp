#pragma once

#include <core/manager.hpp>
#include <render/animatedsprite.hpp>

#include <array>
#include <vector>

namespace Character
{
	static void loadWalkingAnimations(AnimatedSprite* sprite)
	{
	    static std::array<std::string, 4> directions = 
			{"up", "left", "down", "right"};

	    static std::array<std::size_t, 4> rows = 
			{8 * 13, 9 * 13, 10 * 13, 11 * 13};

	    static std::vector<std::size_t> offsets{0, 1, 2, 3, 4, 5, 6, 7, 8};

		for(auto i = 0u; i < 4; i++)
		{
			auto frames = offsets;
			for(auto j = 0u; j < frames.size(); j++)
				frames[j] += rows[i];

			sprite->addAnimation("walk_" + directions[i], frames);
		}
    };

	// TODO Combine similar code
	static void loadIdleAnimations(AnimatedSprite* sprite)
	{
	    static std::array<std::string, 4> directions = 
			{"up", "left", "down", "right"};

	    static std::array<std::size_t, 4> rows = 
			{8 * 13, 9 * 13, 10 * 13, 11 * 13};

	    static std::vector<std::size_t> offsets{0};

		for(auto i = 0u; i < 4; i++)
		{
			auto frames = offsets;
			for(auto j = 0u; j < frames.size(); j++)
				frames[j] += rows[i];

			sprite->addAnimation("idle_" + directions[i], frames);
		}
	}

	static AnimatedSprite* loadAnimations(std::string file)
	{
		// TODO Remove allocating on the heap and leaking
		auto spriteSheet = new sf::Texture;

		auto loaded = spriteSheet->loadFromFile(file.c_str());
		assert(loaded); // Spritesheet loaded successfuly

	    auto animatedSprite =
	        new AnimatedSprite(*spriteSheet, sf::Vector2i(64, 64));
		animatedSprite->setPosition(-16.f, -32.f);

		loadWalkingAnimations(animatedSprite);
		loadIdleAnimations(animatedSprite);

		return animatedSprite;
    }

	// TODO This should go under world/character.hpp
	static EntityIndex createCharacter(Manager& manager, std::string animFile)
	{
		auto character = manager.createEntity();

		manager.addComponent(character, CLocation{});
		manager.addComponent(character, CDesiredMovement{Direction::NONE});
	    manager.addComponent(character, CMovement{Direction::DOWN, TileLocation{}});

	    manager.addComponent(character, CDrawable{loadAnimations(animFile)});
		manager.addTag<TAnimated>(character);

		return character;
	}

} // namespace Character
