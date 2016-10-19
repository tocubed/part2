#pragma once

#include <core/ecs/typelist.hpp>

#include <world/location.hpp>
#include <world/tags.hpp>

namespace World {

using ComponentList = ecs::TypeList<
	CLocation
>;

using TagList = ecs::TypeList<
	TMapLayer
>;

} // namespace World
