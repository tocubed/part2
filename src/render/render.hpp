#pragma once

#include <core/manager.hpp>
#include <core/system.hpp>
#include <render/drawable.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Transform.hpp>

#include <functional>
#include <set>

class RenderSystem : System
{
public:
	RenderSystem(Manager& manager, sf::RenderTarget& renderTarget)
		: System(manager), renderTarget(renderTarget), 
		  view(renderTarget.getView()),
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

		manager.forEntitiesHaving<TPlayer, CLocation>([this](EntityIndex eI)
		{
			auto& location = manager.getComponent<CLocation>(eI);

			view.setCenter(location.x, location.y);
		});
	}

	void render(sf::Time delta)
	{
		renderTarget.setView(view);

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
	sf::View view;

	std::multiset<EntityIndex, 
		std::function<bool(EntityIndex, EntityIndex)>> renderList;
};
