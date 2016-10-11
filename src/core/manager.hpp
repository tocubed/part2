#pragma once

#include <core/ecs/configuration.hpp>
#include <core/ecs/manager.hpp>

#include <core/components.hpp>
#include <core/tags.hpp>

using Configuration = ecs::Configuration<ComponentList, TagList>;
using Manager = ecs::Manager<Configuration>;
