#pragma once

#include <core/ecs/typelist.hpp>

#include <render/drawable.hpp>

// TODO Move this elsewhere
class TAnimated;

namespace Render {

using ComponentList = ecs::TypeList<
	CDrawable
>;

using TagList = ecs::TypeList<
	TAnimated
>;

} // namespace Render
