#pragma once

#include <core/manager.hpp>
#include <world/location.hpp>
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

	void loadMap(const std::string& mapFile, int x, int y)
	{
		maps.emplace_back(new Map(manager));

		auto& map = *maps.back();

		map.loadFromFile(mapFile);
		map.setLocation(x, y);

		// TODO Assert that map size is correct

		mapsByLocation[MapLocation{x, y}] = maps.size() - 1;
	}

	bool isCollidable(int x, int y)
	{
		MapLocation location{x / (int)mapSize, y / (int)mapSize};
		
		auto it = mapsByLocation.find(location);

		if(it == mapsByLocation.end())
			return true; // out of bounds
		else
		{
			auto& map = *maps[std::get<1>(*it)];

			return map.isCollidable(x % mapSize, y % mapSize);
		}
	}

private:
	Manager& manager;
	std::size_t mapSize;

	std::vector<std::unique_ptr<Map>> maps;
	std::unordered_map<MapLocation, std::size_t> mapsByLocation;
};
