#pragma once

// TODO Differentiate between tile location (32x32) and pixel location (1x1)

struct CLocation
{
    int x;
    int y;

	int zLevel; 
};

struct MapLocation
{
	int x;
	int y;

	bool operator==(const MapLocation& rhs) const
	{
		return (x == rhs.x) && (y == rhs.y);
	}
};

namespace std {
	template<>
	struct hash<MapLocation>
	{
		std::size_t operator()(const MapLocation& k) const
		{
			auto hashX = hash<int>{}(k.x);	

			return hash<int>{}(k.y) ^ ((hashX << 16) | (hashX >> 16));
		}
	};	
} // namespace std
