#include <world/map.hpp>

#include <core/manager.hpp>
#include <render/drawable.hpp>
#include <render/character.hpp>

#include <pugixml.hpp>

#include <cassert>
#include <fstream>
#include <memory>
#include <set>
#include <sstream>
#include <unordered_map>

Map::Map(Manager& manager) : manager(manager), mapLocation{0, 0}
{
}

void Map::setSize(std::size_t width, std::size_t height)
{
	mapWidth = width;
	mapHeight = height;
}

void Map::setLocation(TileLocation newLocation)
{
	TileLocation difference = 
		{newLocation.x - mapLocation.x, newLocation.y - mapLocation.y};

	for(auto e: childEntities)
	{
		auto& entityLocation = manager.getComponent<CLocation>(e);
		
		entityLocation.x += difference.toLocation().x;
		entityLocation.y += difference.toLocation().y;
	}

	mapLocation = newLocation;
}

void Map::createTileset(std::size_t numTiles)
{
	auto size = 1;
	while(numTiles > size * size)
		size *= 2;

	// Tileset fits within a single texture
	assert(size * 32 <= sf::Texture::getMaximumSize());

	tileset.create(size * 32, size * 32, sf::Color::Transparent);
}

void Map::addSingleFromTileset(std::size_t id, const sf::Image& source,
	                           unsigned int sourceX, unsigned int sourceY)
{
	auto x = id % (tileset.getSize().x / 32);
	auto y = id / (tileset.getSize().x / 32);

	tileset.copy(
		source, x * 32, y * 32, sf::IntRect(sourceX, sourceY, 32, 32));
}

void Map::addFromTileset(std::size_t startID, const sf::Image& source)
{
	auto width = source.getSize().x / 32;
	auto height = source.getSize().y / 32;

	for(auto y = 0u; y < height; y++)
		for(auto x = 0u; x < width; x++)
		{
			auto offset = (x + width * y);

			addSingleFromTileset(startID + offset, source, x * 32, y * 32);
		}
}

void Map::finalizeTileset()
{
	auto loaded = tilesetTexture.loadFromImage(tileset);

	// Texture resource loads on GPU
	assert(loaded); 

	// Clear image resource
	tileset.create(0, 0);
}

auto Map::createTileLayer(std::string&& name, int zLevel)
{
	tileLayers.emplace_back(mapWidth * mapHeight);

	tileLayerVertices.emplace_back(std::make_unique<sf::VertexArray>());
	tileLayerZLevels.push_back(zLevel);

	auto index = tileLayers.size() - 1;

	tileLayersByName.emplace(std::move(name), index);
	tileLayersByZLevel.emplace(zLevel, index);

	return index;
}

auto& Map::getTileLayerArray(std::size_t index)
{
	return tileLayers[index];
}

auto& Map::getTileLayerArray(const std::string& name)
{
	auto found = tileLayersByName.find(name);

	// Layer with name is found 
	assert(found != tileLayersByName.end());

	return getTileLayerArray(std::get<1>(*found));
}

bool Map::isCollidable(TileLocation location)
{
	auto it = tileLayersByName.find("Collision");

	for(auto e: childEntities)
		if(manager.hasTag<TTileCollidable>(e))
		{
			auto childLoc =
			    TileLocation::fromLocation(manager.getComponent<CLocation>(e));

			childLoc.x -= mapLocation.x;
			childLoc.y -= mapLocation.y;

			if(childLoc == location)
			   return true;	
		}

	if(it == tileLayersByName.end())
		return false; // no collision layer
	else
	{
		auto& layer = getTileLayerArray(std::get<1>(*it));

		return layer[location.x + mapWidth * location.y] != 0;
	}
}

EntityIndex Map::getScriptedEntity(TileLocation location)
{
	for(auto e: childEntities)
		if(manager.hasComponent<CScripts>(e))
		{
			auto childLoc =
			    TileLocation::fromLocation(manager.getComponent<CLocation>(e));

			childLoc.x -= mapLocation.x;
			childLoc.y -= mapLocation.y;

			if(childLoc == location)
				return e;
		}

	return NULL_ENTITY;
}

std::string Map::getMusic()
{
	return musicFile;
}

// TODO More control over visibility
void Map::finalizeTileLayer(std::size_t index)
{
	// Build the vertices
	auto& tiles = tileLayers[index];
	auto& vertices = *tileLayerVertices[index];

	vertices.setPrimitiveType(sf::Quads);
	vertices.resize(mapWidth * mapHeight * 4);

	for(auto y = 0u; y < mapHeight; y++)
		for(auto x = 0u; x < mapWidth; x++)
		{
			auto i = x + mapWidth * y;

			auto tile = tiles[i];
			auto tx = tile % (tilesetTexture.getSize().x / 32);
			auto ty = tile / (tilesetTexture.getSize().x / 32);

			sf::Vertex* quad = &vertices[i * 4];
			
			quad[0].position = sf::Vector2f(x * 32, y * 32);
			quad[1].position = sf::Vector2f((x + 1) * 32, y * 32);
			quad[2].position = sf::Vector2f((x + 1) * 32, (y + 1) * 32);
			quad[3].position = sf::Vector2f(x * 32, (y + 1) * 32);

			quad[0].texCoords = sf::Vector2f(tx * 32, ty * 32);
			quad[1].texCoords = sf::Vector2f((tx + 1) * 32, ty * 32);
			quad[2].texCoords = sf::Vector2f((tx + 1) * 32, (ty + 1) * 32);
			quad[3].texCoords = sf::Vector2f(tx * 32, (ty + 1) * 32);
		}

	auto bounds = vertices.getBounds();

	// Create the entity
	auto drawable = std::make_shared<GenericDrawable>(vertices, tilesetTexture);
	auto zLevel = tileLayerZLevels[index];

	auto layerEntity = manager.createEntity();

	manager.addComponent<CDrawable>(layerEntity, CDrawable{drawable});
	manager.addComponent<CLocation>(
	    layerEntity, CLocation{mapLocation.toLocation().x,
	                           mapLocation.toLocation().y, zLevel});
	manager.addTag<TMapLayer>(layerEntity);

	childEntities.push_back(layerEntity);
}

std::string Map::correctFilePath(std::string file)
{
	std::string path; 
	if(!std::ifstream(file))
	{
		// May be a relative path
		path = mapFileLocation;
		path = path.substr(0, path.find_last_of('/') + 1);
		path += file;
	}
	else
		path = file;

	assert(std::ifstream(path)); // Corrected file path is valid

	return path;
}

EntityIndex Map::loadCharacter(
    const std::string& animFile, TileLocation location, bool collidable,
    const std::map<std::string, std::string>& scripts) 
{
	auto character = Character::createCharacter(manager, animFile);

	manager.getComponent<CLocation>(character) = 
		{32 * location.x, 32 * location.y, 99};

	manager.addComponent(character, CScripts{scripts});

	if(collidable)
		manager.addTag<TTileCollidable>(character);

	childEntities.push_back(character);

	return character;
}

void Map::parseCharacter(pugi::xml_node xml)
{
	std::string spriteSheet;
	Direction facing = DOWN;
	bool collidable = true;

	std::map<std::string, std::string> scripts;

	for(auto property: xml.child("properties").children("property"))
	{
		auto name = property.attribute("name").value();

		if(name == std::string("Spritesheet"))
		{
			spriteSheet = property.attribute("value").value();
			spriteSheet = correctFilePath(spriteSheet);
		}
		else if(name == std::string("Collidable"))
		{
			collidable = property.attribute("value").as_bool();
		}
		else if(name == std::string("Orientation"))
		{
			facing = directionFromString(property.attribute("value").value());
		}
		else // Script
		{
			auto scriptName = property.attribute("name").value();
			
			std::string script;
			if(property.attribute("type").value() == std::string("file"))
			{
				std::string scriptFile = property.attribute("value").value();
				scriptFile = correctFilePath(scriptFile);

				std::ifstream ifstream(scriptFile);
				std::stringstream stream;

				stream << ifstream.rdbuf();
				script = stream.str();
			}
			else if(property.attribute("value").empty())
				script = property.child_value();
			else
				script = property.attribute("value").value();

			scripts[scriptName] = script;
		}
	}

	auto x = xml.attribute("x").as_int() / 32;
	auto y = xml.attribute("y").as_int() / 32;

	auto character =
	    loadCharacter(spriteSheet, TileLocation{x, y}, collidable, scripts);

	manager.getComponent<CMovement>(character).direction = facing;
}

void Map::loadFromFile(const std::string& mapFile)
{
	mapFileLocation = mapFile;

	pugi::xml_document mapXML;
	auto result = mapXML.load_file(mapFile.c_str());

	assert(result); // XML loaded with no parse errors

	auto map = mapXML.child("map");

	// Map tilesize is correct
	assert(map.attribute("tilewidth").as_int() == 32);
	assert(map.attribute("tileheight").as_int() == 32);

	auto width = map.attribute("width").as_int();
	auto height = map.attribute("height").as_int();

	setSize(width, height);

	auto musicProperty =
	    map.select_nodes("properties/property[@name=\"Music\"]");
	if(!musicProperty.empty())
		musicFile = correctFilePath(
		    musicProperty[0].node().attribute("value").value());

	auto numTiles = 0u;
	for(auto tileset: map.children("tileset"))
	{
		// Tileset tilesize is correct
		assert(tileset.attribute("tilewidth").as_int(32) == 32);
		assert(tileset.attribute("tileheight").as_int(32) == 32);

		numTiles += tileset.attribute("tilecount").as_int();	
	}

	createTileset(numTiles);
	for(auto tileset: map.children("tileset"))
	{
		// TODO Tilesets made of collection of images
		auto source = tileset.child("image").attribute("source").value();
		auto startID = tileset.attribute("firstgid").as_int();

		sf::Image tilesetImage;

		auto path = correctFilePath(source);
		auto result = tilesetImage.loadFromFile(path.c_str());
		assert(result); // Image loaded from file

		addFromTileset(startID, tilesetImage);

		// TODO Tile properties
	}

	finalizeTileset();

	auto i = 0u;
	for(auto layer: map.children("layer"))
	{
		// Layer size is correct
		assert(layer.attribute("width").as_int(width) == width);
		assert(layer.attribute("height").as_int(height) == height);

		// TODO Check name is valid and not taken
		auto name = layer.attribute("name").value();
		auto data = layer.child("data");
		
		// Layer data encoding is CSV	
		assert(data.attribute("encoding").value() == std::string("csv"));

		auto zLevel = i * 10;

		auto fgProperty =
		    layer.select_nodes("properties/property[@name=\"Foreground\"]");
		if(!fgProperty.empty())
			if(fgProperty[0].node().attribute("value").as_bool())
				zLevel += 100;

		auto layerIndex = createTileLayer(name, zLevel);

		auto& tiles = getTileLayerArray(layerIndex);
		auto tilesIndex = 0u;

		auto str = data.child_value();
		std::istringstream ss(str);

		std::string line;
		while(std::getline(ss, line))
		{
			std::istringstream lineSS(line);
			std::string token;

			while(std::getline(lineSS, token, ','))
			{
				auto tile = std::stoul(token);

				tiles[tilesIndex] = tile;
				tilesIndex++;
			}
		}

		// TODO More control over visibility
		auto visible = layer.attribute("visible").as_bool(true);
		if(visible)
			finalizeTileLayer(layerIndex);

		i++;
	}

	for(auto objects: map.children("objectgroup"))
		for(auto object: objects.children("object"))
		{
			auto type = object.attribute("type").value();

			if(type == std::string("Character"))
				parseCharacter(object);
		}

	// TODO Image layers
}

void Map::unload()
{
	for(auto e: childEntities)
		manager.deleteEntity(e);
}
