#pragma once

#include <world/components.hpp>

struct CMenuClosed
{
	std::size_t choice;
};

struct CInteract
{
	TileLocation location;
};
