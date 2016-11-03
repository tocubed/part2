#pragma once

#include <core/manager.hpp>
#include <core/system.hpp>

#include <render/dialoguebox.hpp>

class InputSystem : System
{
public:
	InputSystem(Manager& manager)
		: System(manager)
	{}

	void update(sf::Time delta)
	{
		bool dialogueUp = false;
		manager.forEntitiesHaving<TDialogue>(
		[this, &dialogueUp](EntityIndex eI)
		{
			dialogueUp = true;

			auto& drawable = manager.getComponent<CDrawable>(eI);
			auto dialogue = static_cast<DialogueBox*>(drawable.drawable);

			if(dialogue->allDisplayed())
				manager.deleteEntity(eI);
			else
			{
				dialogue->displayMoreCharacters();
				if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
					dialogue->displayMoreLines();
			}
		});

		if(dialogueUp)
			return;

		manager.forEntitiesHaving<TPlayer, CDesiredMovement>(
		[this](EntityIndex eI)
		{
			auto& desired = manager.getComponent<CDesiredMovement>(eI);

			desired.direction = Direction::NONE;
			
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			{
				desired.direction = Direction::UP;
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			{
				desired.direction = Direction::DOWN;
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			{
				desired.direction = Direction::LEFT;
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			{
				desired.direction = Direction::RIGHT;
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
			{
				// Do some interaction 
			}
		});
	}
};
