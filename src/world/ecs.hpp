#pragma once

#include <core/ecs/typelist.hpp>

#include <world/components.hpp>
#include <world/tags.hpp>

namespace World {

using ComponentList = ecs::TypeList<
	CLocation,
	CMovement,
	CDesiredMovement,
	CEventMoved,
	CFollowOrder,
	CPlayer
>;

using TagList = ecs::TypeList<
	TPlayer,
	TFollower,
	TMapLayer,
	TTileCollidable
>;

} // namespace World
