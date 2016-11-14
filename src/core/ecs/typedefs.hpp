#pragma once

#include <cstdint>

using EntityIndex = std::uint32_t;

#define NULL_ENTITY EntityIndex{~0u}
