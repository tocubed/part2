#pragma once

namespace Character
{
	static void loadAnimations(
    	Manager& manager, EntityIndex character, std::string file)
	{
		// TODO Remove allocating on the heap and leaking
		auto spriteSheet = new sf::Texture;

		auto loaded = spriteSheet->loadFromFile(file.c_str());
		assert(loaded); // Spritesheet loaded successfuly

	    auto animatedSprite =
	        new AnimatedSprite(*spriteSheet, sf::Vector2i(64, 64));
		animatedSprite->setPosition(-16.f, -32.f);

		manager.addTag<TAnimated>(character);
		manager.addComponent(character, CDrawable{animatedSprite});

		std::array<std::size_t, 9> walk{0, 1, 2, 3, 4, 5, 6, 7, 8};
		std::array<std::size_t, 4> walkRows{8 * 13, 9 * 13, 10 * 13, 11 * 13};
		std::array<std::string, 4> walkNames{
			std::string("up"), std::string("left"),
			std::string("down"), std::string("right")
		};

		for(auto i = 0u; i < 4; i++)
		{
			std::vector<std::size_t> frames;
			for(auto j = 0u; j < 9; j++)
				frames.push_back(walkRows[i] + walk[j]);

			animatedSprite->addAnimation("walk_" + walkNames[i], frames);
			
			// TODO Really directions names, separate animation
			animatedSprite->addAnimation("idle_" + walkNames[i],
					std::vector<std::size_t>{walkRows[i]}); 
		}
    }

	// TODO Seperate code into this
	static void loadWalkingAnimations() {};

} // namespace Character
