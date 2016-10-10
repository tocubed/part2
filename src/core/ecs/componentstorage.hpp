#pragma once

#include <core/ecs/typedefs.hpp>

#include <tuple>
#include <unordered_map>
#include <utility>

namespace ecs {

template <typename TConfiguration>
class ComponentStorage
{
public:
	using Configuration = TConfiguration;
	using ComponentList = typename Configuration::ComponentList;

private:
	template <typename T>
	using ComponentMap = std::unordered_map<EntityIndex, T>;

	template <typename... Ts>
	using ComponentMapTuple = std::tuple<ComponentMap<Ts>...>;

	typename ComponentList::template Expand<ComponentMapTuple> mapTuple;

public:

	template <typename T>
	auto& getComponent(EntityIndex eI)
	{
		return std::get<ComponentMap<T>>(mapTuple)[eI];
	}
	
	template <typename... Ts>
	void addComponents(EntityIndex eI, Ts&&... components)
	{
		int dummy[] = 
			{(getComponent<Ts>(eI) = std::forward<Ts>(components), 0)...};
		(void)dummy;
	}

	template <typename T>
	auto& addComponent(EntityIndex eI, T&& component)
	{
		auto&& c = getComponent<T>(eI);
		c = std::forward<T>(component);

		return c;
	}

	template <typename T, typename... TArgs>
	auto& emplaceComponent(EntityIndex eI, TArgs&&... args)
	{
		auto& map = std::get<ComponentMap<T>>(mapTuple);

		auto emplace_result = map.emplace(
		    std::piecewise_construct, std::make_tuple(eI),
		    std::forward_as_tuple(std::forward<TArgs>(args)...));

		return *std::get<0>(emplace_result);
	}

	template <typename... Ts>
	void removeComponents(EntityIndex eI)
	{
		int dummy[] = {(std::get<ComponentMap<Ts>>(mapTuple).erase(eI), 0)...};
		(void)dummy;
	}

	template <typename T>
	void removeComponent(EntityIndex eI)
	{
		removeComponents<T>();
	}

};

} // namespace ecs
