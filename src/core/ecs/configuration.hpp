#pragma once

#include <cstddef>

namespace ecs {

template <typename TComponentList, typename TTagList>
struct Configuration
{
	using ComponentList = TComponentList;
	using TagList = TTagList;
	using ComponentAndTagList =
	    typename ComponentList::template Concatenate<TagList>;

	template <typename T>
	static constexpr bool isComponent()
	{
		return ComponentList::template contains<T>();
	}

	template <typename T>
	static constexpr bool isTag()
	{
		return TagList::template contains<T>();
	}

	template <typename T>
	static constexpr bool isComponentOrTag()
	{
		return isComponent<T>() || isTag<T>();
	}

	static constexpr std::size_t componentCount()
	{
		return ComponentList::size();
	}

	static constexpr std::size_t tagCount()
	{
		return TagList::size();
	}

	static constexpr std::size_t componentAndTagCount()
	{
		return componentCount() + tagCount();
	}

	template <typename T>
	static constexpr std::size_t componentID()
	{
		return ComponentList::template index<T>();
	}

	template <typename T>
	static constexpr std::size_t tagID()
	{
		return TagList::template index<T>();
	}

	template <typename T>
	static constexpr std::size_t componentOrTagID()
	{
		return ComponentAndTagList::template index<T>();
	}

	template <typename T>
	static constexpr std::size_t componentBit()
	{
		return componentID<T>();
	}

	template <typename T>
	static constexpr std::size_t tagBit()
	{
		return componentCount() + tagID<T>();
	}

	template <typename T>
	static constexpr std::size_t componentOrTagBit()
	{
		return componentOrTagID<T>();
	}

};

} // namespace ecs
