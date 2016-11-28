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

	void freezePlayer(bool freeze);

	void walkPath(
	    EntityIndex character, std::vector<TileLocation> path,
	    std::function<void()> callback);
	bool walkToLocation(
	    EntityIndex character, TileLocation destination,
	    std::function<void()> callback);
	void teleport(EntityIndex character, TileLocation destination);

	void playAnimation(
	    EntityIndex character, std::string animationName, int playSpeedMS, 
	    std::function<void()> callback);

	void openDialog(std::string text, std::function<void()> callback);
	void openMenu(
	    const std::vector<std::string>& options,
	    const std::vector<std::function<void()>>& callbacks);
	void openPrompt(
		const std::string& text,
	    const std::vector<std::string>& options,
	    const std::vector<std::function<void()>>& callbacks);

	std::vector<EntityIndex> getFollowers(EntityIndex character) const;
	void becomeFollower(EntityIndex character, EntityIndex following);

	TileLocation getTileLocation(EntityIndex entity) const;
	bool makeEntityFace(EntityIndex character, Direction dir);

	void screenFade(bool out, std::function<void()> callback);

	void loadMap(const std::string& mapFile, TileLocation location);
	void unloadMap(TileLocation location);

	void runScript(std::string script);

	void doLatent(std::function<bool()> func);
	void doLatentInOrder(std::vector<std::function<bool()>> funcs);

private:
	Overworld& overworld;
	chaiscript::ChaiScript_Basic& chai;	

	std::list<std::function<bool()>> latentFunctions;
};
