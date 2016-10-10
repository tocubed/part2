#pragma once

#include <cstddef>
#include <type_traits>

namespace ecs {

namespace detail {

template <typename T, typename... Ts>
constexpr std::size_t find()
{
	const auto N{sizeof...(Ts)};
	std::array<bool, N> matching{std::is_same<T, Ts>::value...};

	for(auto i = 0u; i < N; ++i)
		if(matching[i])
			return i;

	return N;
}

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
		int dummy[] = {(func(Ts{}), 0)...};
		(void)dummy;
	}

	template <template <typename...> class T>
	using Expand = T<Ts...>;

	template <typename T>
	struct Concatenate;

	template <typename... Us>
	struct Concatenate<TypeList<Us...>> : TypeList<Ts..., Us...> {};

};

} // namespace ecs
