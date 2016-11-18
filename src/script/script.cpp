#include <script/script.hpp>

#include <script/dialogue.hpp>

#include <chaiscript/chaiscript.hpp>

ScriptSystem::ScriptSystem(Manager& manager, Overworld& overworld)
	: System(manager), overworld(overworld), chai(Script::getChaiScript())
{
	chai.add_global(chaiscript::var(this), "script");
	
	chai.add(chaiscript::fun(&ScriptSystem::openDialog), "dialog");
	chai.add(chaiscript::fun(&ScriptSystem::openMenu), "menu");
	chai.add(chaiscript::fun(&ScriptSystem::openPrompt), "prompt");

	chai.add(chaiscript::user_type<EntityIndex>(), "Entity");

	chai.add(chaiscript::user_type<TileLocation>(), "Location");
	chai.add(chaiscript::fun(&TileLocation::x), "x");
	chai.add(chaiscript::fun(&TileLocation::y), "y");

	chai.add(chaiscript::user_type<Direction>(), "Direction");
	chai.add_global_const(chaiscript::const_var(Direction::UP), "UP");
	chai.add_global_const(chaiscript::const_var(Direction::DOWN), "DOWN");
	chai.add_global_const(chaiscript::const_var(Direction::RIGHT), "RIGHT");
	chai.add_global_const(chaiscript::const_var(Direction::LEFT), "LEFT");

	chai.add(chaiscript::fun(&ScriptSystem::getTileLocation), "location");

	chai.add(chaiscript::fun(&ScriptSystem::makeEntityFace), "face");
	chai.add(chaiscript::fun(&ScriptSystem::walkToLocation), "walk_to");

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
}

void ScriptSystem::walkPath(EntityIndex character, std::vector<TileLocation> path)
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
	
	doLatent(latentWalk);
}

bool ScriptSystem::walkToLocation(EntityIndex character, TileLocation destination)
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

		walkPath(character, path);
	}

	return found;
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

