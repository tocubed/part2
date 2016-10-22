#pragma once

#include <core/ecs/typelist.hpp>

#include <world/components.hpp>
#include <world/tags.hpp>

namespace World {

using ComponentList = ecs::TypeList<
	CLocation,
	CMovement,
	CDesiredMovement
>;

using TagList = ecs::TypeList<
	TPlayer,
	TMapLayer
>;

} // namespace World
