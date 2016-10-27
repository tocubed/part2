#include <world/map.hpp>

#include <core/manager.hpp>
#include <render/drawable.hpp>

#include <pugixml.hpp>

#include <cassert>
#include <fstream>
#include <memory>
#include <set>
#include <sstream>
#include <unordered_map>

Map::Map(Manager& manager) : manager(manager)
{
}

void Map::setSize(std::size_t width, std::size_t height)
{
	mapWidth = width;
	mapHeight = height;
}

void Map::setLocation(TileLocation location)
{
	this->location = location;

	for(auto e: tileLayerEntities)
	{
		auto& entityLocation = manager.getComponent<CLocation>(e);
		
		entityLocation.x = location.toLocation().x;
		entityLocation.y = location.toLocation().y;
	}
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
}

auto Map::createTileLayer(std::string&& name, int zLevel)
{
	tileLayers.emplace_back(mapWidth * mapHeight);

	tileLayerEntities.push_back(manager.createEntity());
	tileLayerVertices.emplace_back(new sf::VertexArray);
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

	if(it == tileLayersByName.end())
		return false; // no collision layer
	else
	{
		auto& layer = getTileLayerArray(std::get<1>(*it));

		return layer[location.x + mapWidth * location.y] != 0;
	}
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
			auto tx = tile % (tileset.getSize().x / 32);
			auto ty = tile / (tileset.getSize().x / 32);

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
	auto drawable = new GenericDrawable(vertices, tilesetTexture);
	auto zLevel = tileLayerZLevels[index];

	auto layerEntity = tileLayerEntities[index];
	manager.addComponent<CDrawable>(layerEntity, CDrawable{drawable});
	manager.addComponent<CLocation>(
	    layerEntity,
	    CLocation{location.toLocation().x, location.toLocation().y, zLevel});
	manager.addTag<TMapLayer>(layerEntity);
}

void Map::loadFromFile(const std::string& mapFile)
{
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

		if(!std::ifstream(source))
		{
			// May be a relative path
			std::string path = mapFile;
			path = path.substr(0, path.find_last_of('/') + 1);
			path += source;

			auto result = tilesetImage.loadFromFile(path.c_str());

			// Image loaded from file
			assert(result);
		}
		else
		{
			auto result = tilesetImage.loadFromFile(source);

			// Image loaded from file
			assert(result);
		}

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

		auto zLevel = i * 1000;
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

	// TODO Object and image layers
}
