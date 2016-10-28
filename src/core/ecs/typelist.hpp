#pragma once

#include <cstddef>
#include <type_traits>

namespace ecs {

namespace detail {

template <typename T>
constexpr std::size_t find(std::size_t i = 0) {
	return i;
}

template <typename T, typename U, typename... Ts>
constexpr std::size_t find(std::size_t i = 0) {
	return std::is_same<T, U>::value ? i : find<T, Ts...>(i + 1);
}

template <typename T>
struct TypeWrapper
{
	using Type = T;
};

} // namespace detail

template <typename... Ts>
struct TypeList
{
	template <typename T>
	static constexpr bool contains()
	{
		return detail::find<T, Ts...>() != sizeof...(Ts);
	}

	template <typename T>
	static constexpr std::size_t index()
	{
		return detail::find<T, Ts...>();
	}

	static constexpr std::size_t size() 
	{
		return sizeof...(Ts);
	}

	template <typename TFunc>
	static void forEach(TFunc&& func)
	{
		int dummy[] = {(func(detail::TypeWrapper<Ts>{}), 0)...};
		(void)dummy;
	}

	template <template <typename...> class T>
	using Expand = T<Ts...>;

	template <typename T>
	struct Concatenate;

	template <typename... Us>
	struct Concatenate<TypeList<Us...>>
	{
		using Type = TypeList<Ts..., Us...>;	
	};
};

} // namespace ecs
