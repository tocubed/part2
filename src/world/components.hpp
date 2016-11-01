#pragma once

#include <core/ecs/typedefs.hpp>

#include <unordered_map>

struct CLocation
{
    int x;
    int y;

	// TODO Figure out a better place for this zLevel
	int zLevel; 
};

struct TileLocation
{
	int x;
	int y;

	static auto fromLocation(const CLocation& l)
	{
		return TileLocation{l.x / 32, l.y / 32};
	}

	auto toLocation() const
	{
		return CLocation{x * 32, y * 32, 0};
	}

	bool operator==(const TileLocation& rhs) const
	{
		return (x == rhs.x) && (y == rhs.y);
	}
};

namespace std {
	template<>
	struct hash<TileLocation>
	{
		std::size_t operator()(const TileLocation& k) const
		{
			auto hashX = hash<int>{}(k.x);	

			return hash<int>{}(k.y) ^ ((hashX << 16) | (hashX >> 16));
		}
	};	
} // namespace std

enum Direction
{
	UP, DOWN, LEFT, RIGHT, NONE
};

struct CMovement
{
	Direction direction;
	TileLocation destination;

	bool moving;
};

struct CDesiredMovement
{
	Direction direction;
};

struct CEventMoved
{
	EntityIndex who;
	Direction direction;
	bool successful;

	bool complete;
};

struct CFollowOrder
{
	EntityIndex entityAhead;
};
