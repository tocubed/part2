#pragma once

#include <core/system.hpp>
#include <render/drawable.hpp>
#include <world/location.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Transform.hpp>

#include <functional>
#include <set>

class RenderSystem : System
{
public:
	RenderSystem(Manager& manager, sf::RenderTarget& renderTarget)
		: System(manager), renderTarget(renderTarget),
		  renderList([this](EntityIndex a, EntityIndex b)
		  {
			  return this->manager.getComponent<CLocation>(a).zLevel <
			         this->manager.getComponent<CLocation>(b).zLevel;
		  })
	{}

	void update(sf::Time delta)
	{
		renderList.clear();
		manager.forEntitiesHaving<CDrawable, CLocation>([this](EntityIndex eI)
		{
			renderList.insert(eI);
		});
	}

	void render(sf::Time delta)
	{
		for(auto eI : renderList)
		{
			auto pos = manager.getComponent<CLocation>(eI);

			sf::Transform t;
			t.translate(pos.x, pos.y);

			renderTarget.draw(*manager.getComponent<CDrawable>(eI).drawable, t);
		}
	}

private:
	sf::RenderTarget& renderTarget;

	std::multiset<EntityIndex, 
		std::function<bool(EntityIndex, EntityIndex)>> renderList;
};
