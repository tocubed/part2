#pragma once

#include <core/manager.hpp>
#include <core/system.hpp>

#include <render/dialoguebox.hpp>
#include <render/menubox.hpp>

#include <map>

class InputSystem : System
{
public:
	InputSystem(Manager& manager)
		: System(manager)
	{
	}

	std::map<sf::Keyboard::Key, bool> keyPressed;

	void clearKeyPresses()
	{
		keyPressed.clear();
	}

	void setKeyPressed(sf::Keyboard::Key key)
	{
		keyPressed[key] = true;
	}

	void update(sf::Time delta)
	{
		if(handleMenu())
			return;

		if(handleDialog())
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

	bool handleDialog()
	{
		bool dialogueUp = false;
		manager.forEntitiesHaving<TDialogue>(
		[this, &dialogueUp](EntityIndex eI)
		{
			dialogueUp = true;

			auto& drawable = manager.getComponent<CDrawable>(eI);
			auto dialogue = static_cast<DialogueBox*>(drawable.drawable);

			dialogue->displayMoreCharacters();

			if(keyPressed[sf::Keyboard::Space])
			{
				if(dialogue->allDisplayed() && !manager.hasTag<TPrompt>(eI))
				{
					announceDialogClosed();
					manager.deleteEntity(eI);
				}
				else
					dialogue->displayMoreLines();
			}
		});

		return dialogueUp;
	}

	bool handleMenu()
	{
		bool menuUp = false;
		manager.forEntitiesHaving<TMenu>(
		[this, &menuUp](EntityIndex eI)
		{
			menuUp = true;

			auto& drawable = manager.getComponent<CDrawable>(eI);
			auto menu = static_cast<MenuBox*>(drawable.drawable);

			if(keyPressed[sf::Keyboard::Up])
				menu->decreaseChoice();
			if(keyPressed[sf::Keyboard::Down])
				menu->increaseChoice();

			if (keyPressed[sf::Keyboard::Space])
			{
				announceMenuClosed(menu->getCurrentChoice());
				manager.deleteEntity(eI);
			}
		});

		return menuUp;
	}

	void announceDialogClosed()
	{
		auto event = manager.createEntity();
		
		manager.addTags<TEvent, TDialogClosed>(event);
	}
	
	void announceMenuClosed(std::size_t i)
	{
		auto event = manager.createEntity();
		
		manager.addTag<TEvent>(event);
		manager.addComponent(event, CMenuClosed{i});
	}
};
