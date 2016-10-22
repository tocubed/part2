#pragma once

#include <core/ecs.hpp>
#include <input/ecs.hpp>
#include <render/ecs.hpp>
#include <world/ecs.hpp>

#include <core/ecs/configuration.hpp>
#include <core/ecs/manager.hpp>

// TODO Consider a cleaner method of typelist concatenation

using ComponentList = 
	Core::ComponentList::Concatenate<
	Render::ComponentList::Concatenate<
	World::ComponentList::Concatenate<
	Input::ComponentList>::Type
	>::Type>::Type
;

using TagList = 
	Core::TagList::Concatenate<
	Render::TagList::Concatenate<
	World::TagList::Concatenate<
	Input::TagList>::Type
	>::Type>::Type
;

using Configuration = ecs::Configuration<ComponentList, TagList>;
using Manager = ecs::Manager<Configuration>;
