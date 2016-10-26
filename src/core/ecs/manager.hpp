#pragma once

#include <core/ecs/componentstorage.hpp>
#include <core/ecs/entity.hpp>
#include <core/ecs/signature.hpp>

#include <deque>
#include <vector>

namespace ecs {

template <typename TConfiguration>
class Manager
{
public:
	using Configuration = TConfiguration;
	using Entity = ecs::Entity<Configuration>;
	using ComponentStorage = ecs::ComponentStorage<Configuration>;
	using Signature = ecs::Signature<Configuration>;

	using ComponentList = typename Configuration::ComponentList;

private:
	std::vector<Entity> entities;
	std::deque<EntityIndex> freeIndices; 

	auto createEntityIndex()
	{
		EntityIndex eI; 

		if(!freeIndices.empty())
		{
			eI = freeIndices.back();
			freeIndices.pop_back();
		}
		else
		{
			eI = entities.size();
			entities.reserve(eI + 1);
		}

		return eI;
	}

	auto& getEntity(EntityIndex eI)
	{
		return entities[eI];
	}

public:
	auto createEntity()
	{
		auto eI = createEntityIndex();
		entities.emplace(entities.begin() + eI);

		return eI;
	}

	bool isDeleted(EntityIndex eI) const
	{
		return entities[eI].deleted;
	}

	template <typename TFunc>
	void forEntities(TFunc&& func)
	{
		for(EntityIndex eI = 0; eI < entities.size(); ++eI)
			if(!isDeleted(eI))
				func(eI);
	}

	template <typename TFunc>
	void forEntitiesMatching(Signature s, TFunc&& func)
	{
		for(EntityIndex eI = 0; eI < entities.size(); ++eI)
			if(getEntity(eI).signature.matches(s) && !isDeleted(eI))
				func(eI);
	}

	template <typename... Ts, typename TFunc>
	void forEntitiesHaving(TFunc&& func)
	{
		for(EntityIndex eI = 0; eI < entities.size(); ++eI)
			if (getEntity(eI).signature.matches(
			        Signature::template create<Ts...>()) && !isDeleted(eI))
				func(eI);
	}


private:
	ComponentStorage components;

public:
	template <typename T>
	auto& getComponent(EntityIndex eI)
	{
		return components.template getComponent<T>(eI);
	}
	
	template <typename... Ts>
	void addComponents(EntityIndex eI, Ts&&... componentz)
	{
		auto& e = getEntity(eI);
		e.signature.template setComponents<Ts...>();

		components.addComponents(eI, std::forward<Ts>(componentz)...);
	}

	template <typename T>
	auto& addComponent(EntityIndex eI, T&& component)
	{
		auto& e = getEntity(eI);
		e.signature.template setComponent<T>();

		return components.addComponent(eI, std::forward<T>(component));
	}

	template <typename T, typename... TArgs>
	auto& emplaceComponent(EntityIndex eI, TArgs&&... args)
	{
		auto& e = getEntity(eI);
		e.signature.template setComponent<T>();

		return components.template emplaceComponent<T>(
		    eI, std::forward<TArgs>(args)...);
	}

	template <typename... Ts>
	void removeComponents(EntityIndex eI)
	{
		auto& e = getEntity(eI);
		e.signature.template clearComponents<Ts...>();
		
		components.template removeComponents<Ts...>();
	}

	template <typename T>
	void removeComponent(EntityIndex eI)
	{
		removeComponents<T>(eI);
	}

	template <typename... Ts>
	bool hasComponents(EntityIndex eI)
	{
		auto& e = getEntity(eI);
		return e.signature.matches(Signature::template create<Ts...>());
	}

	template <typename T>
	bool hasComponent(EntityIndex eI)
	{
		return hasComponents<T>(eI);
	}

public:
	template <typename... Ts>
	void addTags(EntityIndex eI)
	{
		auto& e = getEntity(eI);
		e.signature.template setTags<Ts...>();
	}

	template <typename T>
	void addTag(EntityIndex eI)
	{
		addTags<T>(eI);
	}

	template <typename... Ts>
	void removeTags(EntityIndex eI)
	{
		auto& e = getEntity(eI);
		e.signature.template clearTags<Ts...>();
	}

	template <typename T>
	void removeTag(EntityIndex eI)
	{
		removeTags<T>(eI);
	}

	template <typename... Ts>
	bool hasTags(EntityIndex eI)
	{
		auto& e = getEntity(eI);
		return e.signature.matches(Signature::template create<Ts...>());
	}

	template <typename T>
	bool hasTag(EntityIndex eI)
	{
		return hasTags<T>(eI);
	}

public:
	void deleteEntity(EntityIndex eI)
	{
		auto& e = getEntity();
		e.deleted = true;

		ComponentList::forEach([&](auto t) {
			using Component = decltype(t);

			if(e.signature.template getComponent<Component>())
				removeComponent<Component>(eI);
		});

		freeIndices.push_back(eI);
	}
};

} // namespace ecs
