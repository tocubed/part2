#pragma once

#include <core/manager.hpp>
#include <world/map.hpp>

#include <cassert>
#include <memory>
#include <unordered_map>
#include <vector>

class Overworld
{
public:

	Overworld(Manager& manager, std::size_t mapSize)
		: manager(manager), mapSize(mapSize)
	{
	}

	void loadMap(const std::string& mapFile, TileLocation location)
	{
		maps.emplace_back(std::make_unique<Map>(manager));

		auto& map = *maps.back();

		map.loadFromFile(mapFile);
		map.setLocation(location);

		// TODO Assert that map size is correct

		mapsByLocation[location] = maps.size() - 1;
	}

	void unloadMap(TileLocation location)
	{
		auto it = mapsByLocation.find(location);

		if(it != mapsByLocation.end())
		{
			auto i = it->second;

			maps[i]->unload();

			// Erase the map
			// TODO Reclaim the unused space
			maps[i] = std::make_unique<Map>(manager);
		}
	}

	TileLocation getMapLocation(TileLocation location)
	{
		const auto floor_div = [](int x, int y)
		{
			auto q = x / y;
			auto r = x % y;

			auto d = (r != 0) && ((r < 0) != (y < 0));
			return q - d;
		};

		TileLocation mapLocation;
		mapLocation.x = floor_div(location.x, (int)mapSize) * (int)mapSize;
		mapLocation.y = floor_div(location.y, (int)mapSize) * (int)mapSize;

		return mapLocation;
	}

	bool isCollidable(TileLocation location)
	{
		auto mapLocation = getMapLocation(location);

		auto it = mapsByLocation.find(mapLocation);

		if(it == mapsByLocation.end())
			return true; // out of bounds
		else
		{
			auto& map = *maps[std::get<1>(*it)];

			return map.isCollidable(
			    TileLocation{
			        location.x - mapLocation.x, location.y - mapLocation.y});
		}
	}
	
	EntityIndex getScriptedEntity(TileLocation location)
	{
		auto mapLocation = getMapLocation(location);

		auto it = mapsByLocation.find(mapLocation);

		if(it == mapsByLocation.end())
			return -1; // out of bounds
		else
		{
			auto& map = *maps[std::get<1>(*it)];

			return map.getScriptedEntity(TileLocation{
			    location.x - mapLocation.x, location.y - mapLocation.y});
		}
	}

	std::string getMusic(TileLocation location)
	{
		auto mapLocation = getMapLocation(location);

		auto it = mapsByLocation.find(mapLocation);

		if(it == mapsByLocation.end())
			return ""; // out of bounds
		else
		{
			auto& map = *maps[std::get<1>(*it)];

			return map.getMusic();
		}
	}

private:
	Manager& manager;
	std::size_t mapSize;

	std::vector<std::unique_ptr<Map>> maps;
	std::unordered_map<TileLocation, std::size_t> mapsByLocation;
};
