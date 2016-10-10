#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace ecs {

namespace detail {

template <std::size_t N>
struct bitset : std::array<std::uint64_t, 1 + N/64>
{
	constexpr bool get(std::size_t i)
	{
		auto index = i / 64;
		auto bitmask = ((std::uint64_t)1 << (i & 63));
		
		return (static_cast<const bitset&>(*this)[index] & bitmask) != 0;
	}

	constexpr void set(std::size_t i, bool v)
	{
		auto index = i / 64;
		auto bitmask = ((std::uint64_t)1 << (i & 63));

		if(v == true)
			const_cast<std::uint64_t&>(
			    static_cast<const bitset&>(*this)[index]) |= bitmask;
		else
			const_cast<std::uint64_t&>(
			    static_cast<const bitset&>(*this)[index]) &= ~bitmask;
	}

	constexpr bool operator==(const bitset& rhs) const
	{
		for(auto i = 0u; i < this->size(); ++i)
			if(static_cast<const bitset&>(*this)[i] != rhs[i])
				return false;

		return true;
	}

	constexpr bitset operator&(const bitset& rhs) const
	{
		bitset b{};
		
		for(auto i = 0u; i < this->size(); ++i)
			const_cast<std::uint64_t&>(static_cast<const bitset&>(b)[i]) =
			    static_cast<const bitset&>(*this)[i] & rhs[i];

		return b;
	}

	constexpr bitset operator|(const bitset& rhs) const
	{
		bitset b{};
		
		for(auto i = 0u; i < this->size(); ++i)
			const_cast<std::uint64_t&>(static_cast<const bitset&>(b)[i]) =
			    static_cast<const bitset&>(*this)[i] | rhs[i];

		return b;
	}
};

} // namespace detail

template <typename TConfiguration>
struct Signature : detail::bitset<TConfiguration::componentAndTagCount()>
{
	using Configuration = TConfiguration;
	using Base = detail::bitset<TConfiguration::componentAndTagCount()>;

	template <typename T>
	constexpr bool getComponent() const
	{
		return get(Configuration::template componentBit<T>());
	}

	template <typename T>
	constexpr bool getTag() const
	{
		return get(Configuration::template tagBit<T>());
	}

	template <typename T>
	constexpr bool getComponentOrTag() const
	{
		return get(Configuration::template componentOrTagBit<T>());
	}

	template <typename... Ts>
	constexpr void setComponents()
	{
		int dummy[] =
			{(Base::set(Configuration::template componentBit<Ts>(), true), 0)...};
		(void)dummy;
	}

	template <typename T>
	constexpr void setComponent()
	{
		setComponents<T>();
	}

	template <typename... Ts>
	constexpr void setTags()
	{
		int dummy[] =
			{(Base::set(Configuration::template tagBit<Ts>(), true), 0)...};
		(void)dummy;
	}

	template <typename T>
	constexpr void setTag()
	{
		setTags<T>();
	}

	template <typename... Ts>
	constexpr void setComponentOrTags()
	{
		int dummy[] = {(
		    Base::set(Configuration::template componentOrTagBit<Ts>(), true), 0)...};
		(void)dummy;
	}

	template <typename T>
	constexpr void setComponentOrTag()
	{
		setComponentOrTags<T>();
	}

	template <typename... Ts>
	constexpr void clearComponents()
	{
		int dummy[] =
			{(Base::set(Configuration::template componentBit<Ts>(), false), 0)...};
		(void)dummy;
	}

	template <typename T>
	constexpr void clearComponent()
	{
		clearComponents<T>();
	}

	template <typename... Ts>
	constexpr void clearTags()
	{
		int dummy[] =
			{(Base::set(Configuration::template tagBit<Ts>(), false), 0)...};
		(void)dummy;
	}

	template <typename T>
	constexpr void clearTag()
	{
		clearTags<T>();
	}

	template <typename... Ts>
	constexpr void clearComponentOrTags()
	{
		int dummy[] = {(
		    Base::set(Configuration::template componentOrTagBit<Ts>(), false), 0)...};
		(void)dummy;
	}

	template <typename T>
	constexpr void clearComponentOrTag()
	{
		clearComponentOrTags<T>();
	}

    template <typename... Ts>
    static constexpr Signature create()
    {
        Signature signature{};

        int dummy[] = {(signature.template setComponentOrTag<Ts>(), 0)...};
        (void)dummy;

        return signature;
    }

	constexpr bool matches(const Signature& s) const
	{
		return s == (s & *this); 
	};

};

} // namespace ecs
