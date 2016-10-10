#pragma once

#include <core/ecs/signature.hpp>

namespace ecs {

template <typename TConfiguration>
struct Entity
{
	using Configuration = TConfiguration;
	using Signature = ecs::Signature<Configuration>;

	Signature signature;
	bool deleted;
};

} // namespace ecs
