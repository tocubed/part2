#pragma once

#include <core/ecs/typelist.hpp>

// TODO Move this
class TEvent;

namespace Core {

using ComponentList = ecs::TypeList<

>;

using TagList = ecs::TypeList<
	TEvent
>;

} // namespace Core
