#pragma once

#include <core/ecs/typelist.hpp>

#include <render/drawable.hpp>
#include <world/position.hpp>

using ComponentList = ecs::TypeList<
	CPosition,
	CDrawable
>;
