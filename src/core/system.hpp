#pragma once

#include <core/manager.hpp>

#include <SFML/System/Time.hpp>

struct System
{
	System(Manager& manager) : manager(manager) {}

	void update(sf::Time);
	void render(sf::Time);

	Manager& manager;
};
