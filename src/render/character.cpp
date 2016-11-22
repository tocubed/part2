#include <render/character.hpp>

#include <core/manager.hpp>
#include <render/animatedsprite.hpp>

#include <array>
#include <memory>
#include <vector>

namespace Character
{
	void loadWalkingAnimations(AnimatedSprite& sprite)
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

			sprite.addAnimation("walk_" + directions[i], frames);
		}
    };

	// TODO Combine similar code
	void loadIdleAnimations(AnimatedSprite& sprite)
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

			sprite.addAnimation("idle_" + directions[i], frames);
		}
	}

	std::map<std::string, std::shared_ptr<sf::Texture>> spriteSheets;

	std::unique_ptr<AnimatedSprite> loadAnimations(std::string file)
	{
		std::shared_ptr<sf::Texture> spriteSheet;
		
		auto it = spriteSheets.find(file);
		if(it != spriteSheets.end())
			spriteSheet = it->second;
		else
		{
			spriteSheet = spriteSheets[file] = std::make_shared<sf::Texture>();

			auto loaded = spriteSheet->loadFromFile(file.c_str());
			assert(loaded); // Spritesheet loaded successfuly
		}

	    auto animatedSprite = std::make_unique<AnimatedSprite>(
	        *spriteSheet, sf::Vector2i(64, 64));
	    animatedSprite->setPosition(-16.f, -32.f);

		loadWalkingAnimations(*animatedSprite);
		loadIdleAnimations(*animatedSprite);

	    static std::vector<std::size_t> standUpAnimation{
	        20 * 13 + 5, 20 * 13 + 4, 20 * 13 + 3,
	        20 * 13 + 2, 20 * 13 + 1, 20 * 13};
		animatedSprite->addAnimation("stand_up", standUpAnimation);

	    return animatedSprite;
    }

	// TODO This should go under world/character.hpp
	EntityIndex createCharacter(Manager& manager, std::string animFile)
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
