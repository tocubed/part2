#pragma once

#include <core/ecs/typelist.hpp>
#include <world/movement.hpp>


namespace Input {

using ComponentList = ecs::TypeList<
	CDesiredMovement 
>;

using TagList = ecs::TypeList<
	TPlayer
>;

} // namespace Input
