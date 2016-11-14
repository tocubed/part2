#pragma once

#include <core/ecs/typelist.hpp>

#include <script/components.hpp>
#include <script/tags.hpp>

namespace Script {

using ComponentList = ecs::TypeList<
	CScripts
>;

using TagList = ecs::TypeList<
	TDialogue,
	TMenu,
	TPrompt
>;

} // namespace Script
