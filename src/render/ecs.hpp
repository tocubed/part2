#pragma once

#include <core/ecs/typelist.hpp>

#include <render/drawable.hpp>

namespace Render {

using ComponentList = ecs::TypeList<
	CDrawable
>;

using TagList = ecs::TypeList<

>;

} // namespace Render
