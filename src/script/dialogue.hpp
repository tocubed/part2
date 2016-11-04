#pragma once

#include <core/manager.hpp>
#include <render/dialoguebox.hpp>

namespace Dialogue
{
	struct Statics
	{
		sf::Texture* texture;
		sf::Font* font;

		bool loaded;
	};

    static DialogueBox*
    loadDialogueBox(sf::Vector2i dimensions, const std::string& text) 
	{
	    static Statics statics;

		if(!statics.loaded)
		{
			statics.texture = new sf::Texture;
			statics.font = new sf::Font;

			statics.texture->loadFromFile("assets/images/dialoguebox.png");
			statics.font->loadFromFile("assets/fonts/test.ttf");

			statics.loaded = true;
		}

		auto box = new DialogueBox(dimensions, *statics.texture, *statics.font);
		box->setText(text);

		return box;
	}

	static EntityIndex createDialogue(Manager& manager, const std::string& text)
	{
		sf::Vector2i dimensions{15 * 32, 4 * 32};

		auto dialogue = manager.createEntity();

		CLocation location{};
		manager.forEntitiesHaving<TPlayer>(
		[&manager, &location](EntityIndex player)
		{
			location = manager.getComponent<CLocation>(player);
		});

		location.x -= 7 * 32;
		location.y += 8 * 32 - dimensions.y;

		location.zLevel = 1000;

		manager.addComponent(dialogue, CLocation{location});
	    manager.addComponent(
	        dialogue, CDrawable{loadDialogueBox(dimensions, text)});
	    manager.addTag<TDialogue>(dialogue);

		return dialogue;
	}

} // namespace Dialogue
