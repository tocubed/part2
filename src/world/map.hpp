#pragma once 

#include <core/manager.hpp>
#include <world/components.hpp>

#include <pugixml.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include <cstddef>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

class Map
{
public:
	Map(Manager& manager);

	void loadFromFile(const std::string& mapFile);

public:
	void setSize(std::size_t width, std::size_t height);
	void setLocation(TileLocation location);

	bool isCollidable(TileLocation location);
	EntityIndex getScriptedEntity(TileLocation location);

private:
	Manager& manager;

	std::size_t mapWidth;
	std::size_t mapHeight;

	TileLocation mapLocation;

private:
	void createTileset(std::size_t numTiles);

	void addSingleFromTileset(std::size_t id, const sf::Image& source,
	                          unsigned int sourceX, unsigned int sourceY);

	void addFromTileset(std::size_t startID, const sf::Image& source);

	void finalizeTileset();

	sf::Image tileset;
	sf::Texture tilesetTexture;

private:
	auto createTileLayer(std::string&& name, int zLevel);

	auto& getTileLayerArray(std::size_t index);

	auto& getTileLayerArray(const std::string& name);

	// TODO More control over visibility
	void finalizeTileLayer(std::size_t index);

	// TODO fix errors that could happen during reallocation
	// might use something else instead of unique_ptr
	std::vector<std::vector<std::size_t>> tileLayers;

	std::vector<std::unique_ptr<sf::VertexArray>> tileLayerVertices;
	std::vector<int> tileLayerZLevels;

	std::unordered_map<std::string, std::size_t> tileLayersByName;
	std::set<std::pair<int, std::size_t>> tileLayersByZLevel;

private:
	std::vector<EntityIndex> childEntities;

	void loadCharacter(
	    const std::string& animFile, TileLocation location, bool collidable,
	    const std::map<std::string, std::string>& scripts);
	void parseCharacter(pugi::xml_node xml);

private:
	std::string mapFileLocation;

	std::string correctFilePath(std::string file);

private:
	std::unordered_map<TileLocation, std::string> interactScripts;
};
