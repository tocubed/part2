#pragma once

#include <core/ecs/typelist.hpp>
#include <input/components.hpp>
#include <input/tags.hpp>

namespace Input {

using ComponentList = ecs::TypeList<
	CMenuClosed,
	CInteract
>;

using TagList = ecs::TypeList<
	TDialogClosed
>;

} // namespace Input
