#pragma once

#include <core/manager.hpp>
#include <render/boxframe.hpp>
#include <render/dialoguebox.hpp>
#include <render/menubox.hpp>

namespace Dialogue
{
	struct Statics
	{
		sf::Texture* boxBorder;
		sf::Texture* boxBackground;
		sf::Font* font;

		bool loaded;
	};

	static Statics getStatics()
	{
	    static Statics statics;

		if(!statics.loaded)
		{
			statics.boxBorder = new sf::Texture;
			statics.boxBackground = new sf::Texture;
			statics.font = new sf::Font;

			statics.boxBorder->loadFromFile("assets/images/boxborder.png");
		    statics.boxBackground->
				loadFromFile("assets/images/boxbackground.png");

			statics.boxBorder->setRepeated(true);
			statics.boxBackground->setRepeated(true);

		    statics.font->loadFromFile("assets/fonts/test.ttf");

			statics.loaded = true;
		}

		return statics;
	}

    static DialogueBox*
    loadDialogueBox(sf::Vector2i dimensions, const std::string& text) 
	{
		auto statics = getStatics();

	    auto boxTexture = Render::createBoxFrame(
	        *statics.boxBorder, *statics.boxBackground, dimensions);

	    auto box = new DialogueBox(dimensions, *boxTexture, *statics.font);
		box->setText(text);

		return box;
	}

    static MenuBox*
    loadMenuBox(const std::vector<std::string>& options) 
	{
		auto statics = getStatics();

	    auto box = new MenuBox(
	        *statics.font, *statics.boxBorder, *statics.boxBackground);
	    box->setOptions(options);

		return box;
	}

    static EntityIndex
    createMenu(Manager& manager, const std::vector<std::string>& options)
	{
		auto menu = manager.createEntity();

		CLocation location{};
		manager.forEntitiesHaving<TPlayer>(
		[&manager, &location](EntityIndex player)
		{
			location = manager.getComponent<CLocation>(player);
		});

		location.x += 3 * 32;
		location.y -= 0 * 32;

		location.zLevel = 1000;

		manager.addComponent(menu, CLocation{location});
	    manager.addComponent(
	        menu, CDrawable{loadMenuBox(options)});
	    manager.addTag<TMenu>(menu);

		return menu;
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
