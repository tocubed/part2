#pragma once

#include <core/manager.hpp>
#include <core/system.hpp>
#include <script/chaiscript.hpp>
#include <world/overworld.hpp>

#include <functional>
#include <list>

class ScriptSystem : System
{
public:
	ScriptSystem(Manager& manager, Overworld& world);

	void update(sf::Time delta);

	void openDialog(std::string text, std::function<void()> callback);
	void openMenu(
	    const std::vector<std::string>& options,
	    const std::vector<std::function<void()>>& callbacks);
	void openPrompt(
		const std::string& text,
	    const std::vector<std::string>& options,
	    const std::vector<std::function<void()>>& callbacks);

	TileLocation getTileLocation(EntityIndex entity) const;
	bool makeEntityFace(EntityIndex character, Direction dir);

	void runScript(std::string script);

	void doLatent(std::function<bool()> func);
	void doLatentInOrder(std::vector<std::function<bool()>> funcs);

private:
	Overworld& overworld;
	chaiscript::ChaiScript_Basic& chai;	

	std::list<std::function<bool()>> latentFunctions;
};
