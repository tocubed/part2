#include <script/script.hpp>

#include <render/animatedsprite.hpp>
#include <script/dialogue.hpp>

#include <chaiscript/chaiscript.hpp>

namespace
{
	TileLocation constructLocation(int x, int y)
	{
		return TileLocation{x, y};
	}

}

ScriptSystem::ScriptSystem(Manager& manager, Overworld& overworld)
	: System(manager), overworld(overworld), chai(Script::getChaiScript()),
	  showHealthBars(false), healthBars()
{
	fadeRectangle =
	    std::make_shared<sf::RectangleShape>(sf::Vector2f(1000000, 1000000));
	fadeRectangle->setFillColor(sf::Color(0, 0, 0, 0));

	auto fader = manager.createEntity();

	manager.addComponent(fader, CLocation{-500000, -500000, 10000});
	manager.addComponent(fader, CDrawable{fadeRectangle}); 

	// Chaiscript Bindings
	chai.add_global(chaiscript::var(this), "script");
	
	chai.add(chaiscript::fun(&ScriptSystem::openDialog), "dialog");
	chai.add(chaiscript::fun(&ScriptSystem::openMenu), "menu");
	chai.add(chaiscript::fun(&ScriptSystem::openPrompt), "prompt");

	chai.add(chaiscript::user_type<EntityIndex>(), "Entity");
	chai.add(chaiscript::fun([](EntityIndex eI) -> EntityIndex
	{
		return EntityIndex{eI};
	}), "Entity");
	chai.add(chaiscript::bootstrap::standard_library::vector_type<
	            std::vector<EntityIndex>>("EntityVector"));

	chai.add(chaiscript::user_type<TileLocation>(), "Location");
	chai.add(chaiscript::fun(constructLocation), "Location");
	chai.add(chaiscript::fun(&TileLocation::x), "x");
	chai.add(chaiscript::fun(&TileLocation::y), "y");

	chai.add(chaiscript::bootstrap::standard_library::map_type<
	            std::map<EntityIndex, TileLocation>>("EntityLocationMap"));

	/*
	chai.add(chaiscript::user_type<Direction>(), "Direction");
	chai.add(chaiscript::fun(constructDirection), "Direction");
	chai.add_global_const(chaiscript::const_var(Direction::UP), "UP");
	chai.add_global_const(chaiscript::const_var(Direction::DOWN), "DOWN");
	chai.add_global_const(chaiscript::const_var(Direction::RIGHT), "RIGHT");
	chai.add_global_const(chaiscript::const_var(Direction::LEFT), "LEFT");
	*/
	chai.add(
	    chaiscript::fun([&manager](EntityIndex eI) -> Direction {
		    return manager.getComponent<CMovement>(eI).direction;
		}), "facing");

	chai.add(chaiscript::bootstrap::standard_library::map_type<
	            std::map<EntityIndex, Direction>>("EntityDirectionMap"));

	chai.add(chaiscript::fun(&ScriptSystem::getTileLocation), "location");

	chai.add(chaiscript::fun(&ScriptSystem::makeEntityFace), "face");
	chai.add(chaiscript::fun(&ScriptSystem::walkToLocation), "walk_to");
	chai.add(chaiscript::fun(&ScriptSystem::freezePlayer), "freeze_player");
	chai.add(chaiscript::fun(&ScriptSystem::teleport), "teleport");

	chai.add(chaiscript::fun(&ScriptSystem::playAnimation), "play");
	chai.add(chaiscript::fun(&ScriptSystem::screenFade), "fade");

	chai.add(
	    chaiscript::fun([=](bool show) { showHealthBars = show; }),
	    "show_healthbars");

	chai.add(chaiscript::fun(&ScriptSystem::getFollowers), "followers");
	chai.add(chaiscript::fun(&ScriptSystem::becomeFollower), "follow");
	chai.add(chaiscript::fun(&ScriptSystem::killCharacter), "kill_character");

	chai.add(chaiscript::fun(&ScriptSystem::loadMap), "load_map");
	chai.add(chaiscript::fun(&ScriptSystem::unloadMap), "unload_map");

	chai.add(chaiscript::fun([&manager](EntityIndex who, bool tag)
	{
		if(tag)
			manager.addTag<TFollower>(who);
		else
			manager.removeTag<TFollower>(who);
	}), "tag_follower");

	chai.add(
	    chaiscript::fun([&manager](EntityIndex who, const std::string& what) {
		    return manager.getComponent<CScripts>(who)[what];
		}),
	    "get_script");

	chai.add_global(chaiscript::var(NULL_ENTITY), "self");
	chai.add_global(chaiscript::var(NULL_ENTITY), "player");

	chai(R"(use("init.chai");)");
}

void ScriptSystem::update(sf::Time delta)
{
	if(chai.eval<EntityIndex>("player") == NULL_ENTITY)
	{
		auto player = NULL_ENTITY;

		manager.forEntitiesHaving<TPlayer>(
		    [&player](EntityIndex eI) { player = eI; });

		chai.set_global(chaiscript::var(player), "player");
	}

	// Interaction
	manager.forEntitiesHaving<TEvent, CInteract>([this](EntityIndex eI)
	{
		auto& interact = manager.getComponent<CInteract>(eI);

		auto e = overworld.getScriptedEntity(interact.location);
		if(e == NULL_ENTITY)
			return;

		auto& scripts = manager.getComponent<CScripts>(e);

		chai.set_global(chaiscript::var(e), "self");

		runScript(scripts["OnInteract"]);
	});

	for(auto it = latentFunctions.begin(); it != latentFunctions.end(); )
	{
		if((*it)())
			it = latentFunctions.erase(it);
		else
			++it;
	}

	displayHealthBars();
}

void ScriptSystem::walkPath(
    EntityIndex character, std::vector<TileLocation> path,
    std::function<void()> callback)
{
	unsigned int i = 0;

	auto latentWalk = [=]() mutable
	{
		auto location = 
	    	TileLocation::fromLocation(manager.getComponent<CLocation>(character));
		auto moving =
			manager.getComponent<CMovement>(character).moving;

		if(path[i] == location && !moving)
		{
			i++;

			if(i == path.size())
				return true;

			// Move to new destination
			auto direction = TileLocation::directionDiff(location, path[i]);

			auto& desired = manager.getComponent<CDesiredMovement>(character);
			desired.direction = direction;
		}

		return false;
	};

	auto callCallback = [=]()
	{
		callback();
		return true;
	};
	
	doLatentInOrder({latentWalk, callCallback});
}

bool ScriptSystem::walkToLocation(
    EntityIndex character, TileLocation destination,
    std::function<void()> callback) 
{
	auto start =
	    TileLocation::fromLocation(manager.getComponent<CLocation>(character));

	std::set<TileLocation> closed;
	std::set<TileLocation> open({start});

	std::map<TileLocation, TileLocation> cameFrom;

	struct DefaultInfinity
	{
		unsigned int value = ~0u;
	};
	std::map<TileLocation, DefaultInfinity> gScore;
	std::map<TileLocation, DefaultInfinity> fScore;

	auto costEstimate = [=](TileLocation a, TileLocation b)
	{
		return std::abs(b.x - a.x) + std::abs(b.y - a.y);
	};

	gScore[start].value = 0;
	fScore[start].value = costEstimate(start, destination);

	bool found = false;
	while(!open.empty())
	{
		unsigned int bestScore = ~0u;
		TileLocation current; 

		for(auto node: open)
		{
			if(fScore[node].value <= bestScore)
			{
				current = node;
				bestScore = fScore[current].value;
			}
		}
		if(current == destination)
		{
			found = true;
			break;
		}

		open.erase(current);
		closed.insert(current);
		for(int d = Direction::UP; d != Direction::NONE; ++d)
		{
			auto dir = static_cast<Direction>(d);
			TileLocation neighbor = TileLocation::moveDirection(current, dir);
			if(closed.count(neighbor) > 0)
				continue;

			if(overworld.isCollidable(neighbor))
				continue;

			auto tentative = gScore[current].value + 1;
			if(open.count(neighbor) == 0)
				open.insert(neighbor);
			else if(tentative >= gScore[neighbor].value)
				continue;

			cameFrom[neighbor] = current;
			gScore[neighbor].value = tentative;
			fScore[neighbor].value =
			    gScore[neighbor].value + costEstimate(neighbor, destination);
		}
	}

	if(found)
	{
		auto current = destination;

		std::vector<TileLocation> path;
		path.push_back(current);

		while(cameFrom.count(current) != 0)
		{
			current = cameFrom[current];
			path.push_back(current);
		}

		std::reverse(path.begin(), path.end());

		walkPath(character, path, callback);
	}

	return found;
}

void ScriptSystem::teleport(EntityIndex character, TileLocation destination)
{
	auto location = destination.toLocation();

	auto& charLocation = manager.getComponent<CLocation>(character);
	charLocation.x = location.x;
	charLocation.y = location.y;
}

void ScriptSystem::playAnimation(
    EntityIndex character, std::string animationName, int playSpeedMS,
    std::function<void()> callback)
{
	auto& drawable = manager.getComponent<CDrawable>(character);
	auto& animation = static_cast<AnimatedSprite&>(*(drawable.drawable));

	animation.playAnimation(animationName, false);
	animation.setPlaySpeed(sf::milliseconds(playSpeedMS));

	auto waitForAnimation = [&animation]()
	{
		return !animation.isPlaying();
	};

	auto callCallback = [=]() 
	{ 
		callback(); 
		return true; 
	};

	doLatentInOrder({waitForAnimation, callCallback});
}

void ScriptSystem::freezePlayer(bool freeze)
{
	auto& player = manager.getComponent<CPlayer>(chai.eval<EntityIndex>("player"));

	player.freeze = freeze;
}

void ScriptSystem::openDialog(std::string text, std::function<void()> callback)
{
	auto dialog = Dialogue::createDialogue(manager, text);

	auto waitForDialogClose = [=]()
	{
		return manager.isDeleted(dialog);
	};

	auto callCallback = [=]() 
	{ 
		callback(); 
		return true; 
	};

	doLatentInOrder({waitForDialogClose, callCallback});
}

void ScriptSystem::openMenu(
    const std::vector<std::string>& options,
    const std::vector<std::function<void()>>& callbacks)
{
	auto menu = Dialogue::createMenu(manager, options);

	auto waitForMenuClose = [=]()
	{
		return manager.isDeleted(menu);
	};

	auto callChosenCallback = [=]()
	{
		bool called = false;

		manager.forEntitiesHaving<TEvent, CMenuClosed>(
		[=, &called](EntityIndex eI)
		{
			// TODO Check that the correct menu was closed
			auto choice = manager.getComponent<CMenuClosed>(eI).choice;

			callbacks[choice]();
			called = true;
		});

		return called;
	};

	doLatentInOrder({waitForMenuClose, callChosenCallback});
}

void ScriptSystem::openPrompt(
	const std::string& text,
    const std::vector<std::string>& options,
    const std::vector<std::function<void()>>& callbacks)
{
	auto dialog = Dialogue::createDialogue(manager, text);
	manager.addTag<TPrompt>(dialog);

	auto waitForDialog = [=]()
	{
		auto& drawable = manager.getComponent<CDrawable>(dialog);	
		auto& dialogBox = static_cast<DialogueBox&>(*drawable.drawable);

		return dialogBox.allDisplayed();
	};

	auto displayMenu = [=]()
	{
		auto menu = Dialogue::createMenu(manager, options);

		auto waitForMenuClosed = [=]()
		{
			return manager.isDeleted(menu);
		};

		auto callChosenCallback = [=]()
		{
			bool called = false;

			manager.forEntitiesHaving<TEvent, CMenuClosed>(
			[=, &called](EntityIndex eI)
			{
				// TODO Check that the correct menu was closed
				auto choice = manager.getComponent<CMenuClosed>(eI).choice;

				callbacks[choice]();
				called = true;
			});

			return called;
		};

		auto closeDialog = [=]()
		{
			manager.deleteEntity(dialog);
			return true;
		};

		doLatentInOrder({waitForMenuClosed, callChosenCallback, closeDialog});

		return true;
	};

	doLatentInOrder({waitForDialog, displayMenu});
}

std::vector<EntityIndex> ScriptSystem::getFollowers(EntityIndex character) const
{
	std::map<EntityIndex, EntityIndex> followerMap;

	manager.forEntitiesHaving<CFollowOrder>(
	[=, &followerMap](EntityIndex follower)
	{
		auto following = manager.getComponent<CFollowOrder>(follower).entityAhead;

		followerMap[following] = follower;
	});

	std::vector<EntityIndex> followers;

	auto current = character;
	while(followerMap.count(current) != 0)
	{
		current = followerMap[current];
		followers.push_back(current);
	}

	return followers;
}

void ScriptSystem::becomeFollower(EntityIndex character, EntityIndex following)
{
	auto followers = getFollowers(following);

	auto toFollow = followers.size() == 0 ? following : followers.back();

	freezePlayer(true);

	manager.addTag<TFollower>(character);
	manager.addComponent(character, CFollowOrder{toFollow});
	
	if(manager.hasTag<TTileCollidable>(character))
		manager.removeTag<TTileCollidable>(character);

	auto location =
	    TileLocation::fromLocation(manager.getComponent<CLocation>(toFollow));

	auto direction = manager.getComponent<CMovement>(toFollow).direction;

	for(auto dir: std::vector<Direction>{direction, UP, DOWN, LEFT, RIGHT, NONE})
	{
		auto finishWalk = [=]()
		{
			makeEntityFace(character, dir);
			freezePlayer(false);
		};

		// Opposite direction
		Direction moveDir;

		if(dir <= 1) moveDir = (Direction)(1 - dir);
		else if(dir <= 3) moveDir = (Direction)(5 - dir);

		auto behind = TileLocation::moveDirection(location, moveDir);
		if(walkToLocation(character, behind, finishWalk))
			break;
	}
}

void ScriptSystem::killCharacter(EntityIndex character)
{
	auto followers = getFollowers(character);

	if(followers.size() > 0 && manager.hasComponent<CFollowOrder>(character))
	{
		manager.getComponent<CFollowOrder>(followers[0]).entityAhead =
			manager.getComponent<CFollowOrder>(character).entityAhead;
	}

	manager.deleteEntity(character);
}

TileLocation ScriptSystem::getTileLocation(EntityIndex entity) const
{
	return TileLocation::fromLocation(manager.getComponent<CLocation>(entity));
}

bool ScriptSystem::makeEntityFace(EntityIndex character, Direction dir)
{
	auto& movement = manager.getComponent<CMovement>(character);
	if(movement.moving)
		return false;

	movement.direction = dir;

	return true;
}

void ScriptSystem::screenFade(bool out, std::function<void()> callback)
{
	int opacity = out ? 0 : 255;
	fadeRectangle->setFillColor(sf::Color(0, 0, 0, opacity));

	auto doFadeRectangle = [=]() mutable
	{
		opacity += (out ? 10 : -10);
		opacity = std::min(std::max(0, opacity), 255);

		fadeRectangle->setFillColor(sf::Color(0, 0, 0, opacity));

		return out ? opacity == 255 : opacity == 0;
	};

	auto callCallback = [=]()
	{
		callback();
		return true;
	};

	doLatentInOrder({doFadeRectangle, callCallback});
}

void ScriptSystem::makeHealthBarFor(EntityIndex character)
{
	auto healthbar = manager.createEntity();

	manager.addComponent(healthbar, CLocation{});
	manager.addComponent(
	    healthbar, CDrawable{std::make_shared<sf::RectangleShape>()});

	healthBars[character] = healthbar;
}

void ScriptSystem::setHealthBarFor(EntityIndex character, int health)
{
	if(health <= 0)
		return;

	auto it = healthBars.find(character);
	auto healthbar = it->second;

	auto& drawable = manager.getComponent<CDrawable>(healthbar);
	auto& bar = static_cast<sf::RectangleShape&>(*(drawable.drawable));

	if(health <= 25)
		bar.setFillColor(sf::Color(255, 0, 0, 200));
	else if(health <= 50)
		bar.setFillColor(sf::Color(255, 255, 0, 200));
	else
		bar.setFillColor(sf::Color(0, 255, 0, 200));

	bar.setOutlineColor(sf::Color(0, 0, 0, 255));
	bar.setOutlineThickness(1);

	bar.setSize(sf::Vector2f((32 * health) / 100, 4));
}

void ScriptSystem::displayHealthBars()
{
	for(auto it: healthBars)
	{
		auto character = it.first;
		auto healthbar = it.second;

		auto& drawable = manager.getComponent<CDrawable>(healthbar);
		auto& bar = static_cast<sf::RectangleShape&>(*(drawable.drawable));

		// Hide all bars
		bar.setFillColor(sf::Color(0, 0, 0, 0));
		bar.setOutlineColor(sf::Color(0, 0, 0, 0));
		bar.setOutlineThickness(0);

		auto& char_location = manager.getComponent<CLocation>(character);
		auto& bar_location = manager.getComponent<CLocation>(healthbar);
		
		// Follow character
		bar_location.x = char_location.x;
		bar_location.y = char_location.y - 24;
		bar_location.zLevel = char_location.zLevel + 1;
	}

	if(!showHealthBars)
		return;

	auto enemy = chai.eval<EntityIndex>("combat_enemy");
	if(healthBars.count(enemy) == 0)
		makeHealthBarFor(enemy);

	auto enemy_health = chai.eval<int>("enemy_health");
	setHealthBarFor(enemy, enemy_health);

	for(auto i = 0; i < chai.eval<int>("int(index_to_follower.size())"); i++)
	{
		auto follower = chai.eval<EntityIndex>(
		    "index_to_follower[" + std::to_string(i) + "]");
		if(healthBars.count(follower) == 0)
			makeHealthBarFor(follower);

		auto follower_health = chai.eval<int>(
		    "follower_healths[" + std::to_string(i) + "]");
		setHealthBarFor(follower, follower_health);
	}
}

void ScriptSystem::loadMap(const std::string& mapFile, TileLocation location)
{
	overworld.loadMap(mapFile, location);
}

void ScriptSystem::unloadMap(TileLocation location)
{
	overworld.unloadMap(location);
}

void ScriptSystem::runScript(std::string script)
{
	chai(script);
}

void ScriptSystem::doLatent(std::function<bool()> func)
{
	latentFunctions.push_back(func);
}

void ScriptSystem::doLatentInOrder(std::vector<std::function<bool()>> funcs)
{
	auto index = 0;

	auto combined = [index, funcs]() mutable
	{
		while(index < funcs.size())
			if(funcs[index]())
				index++;
			else
				break;

		return index >= funcs.size();
	};

	doLatent(combined);
}

