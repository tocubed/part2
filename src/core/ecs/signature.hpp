#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace ecs {

namespace detail {

template <std::size_t N>
struct bitset : std::array<std::uint64_t, 1 + N/64>
{
	bool get(std::size_t i) const
	{
		auto index = i / 64;
		auto bitmask = ((std::uint64_t)1 << (i & 63));
		
		return (static_cast<const bitset&>(*this)[index] & bitmask) != 0;
	}

	void set(std::size_t i, bool v)
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

	bool operator==(const bitset& rhs) const
	{
		for(auto i = 0u; i < this->size(); ++i)
			if(static_cast<const bitset&>(*this)[i] != rhs[i])
				return false;

		return true;
	}

	bitset operator&(const bitset& rhs) const
	{
		bitset b{};
		
		for(auto i = 0u; i < this->size(); ++i)
			const_cast<std::uint64_t&>(static_cast<const bitset&>(b)[i]) =
			    static_cast<const bitset&>(*this)[i] & rhs[i];

		return b;
	}

	bitset operator|(const bitset& rhs) const
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
	bool getComponent() const
	{
		return Base::get(Configuration::template componentBit<T>());
	}

	template <typename T>
	bool getTag() const
	{
		return Base::get(Configuration::template tagBit<T>());
	}

	template <typename T>
	bool getComponentOrTag() const
	{
		return Base::get(Configuration::template componentOrTagBit<T>());
	}

	template <typename... Ts>
	void setComponents()
	{
		int dummy[] =
			{(Base::set(Configuration::template componentBit<Ts>(), true), 0)...};
		(void)dummy;
	}

	template <typename T>
	void setComponent()
	{
		setComponents<T>();
	}

	template <typename... Ts>
	void setTags()
	{
		int dummy[] =
			{(Base::set(Configuration::template tagBit<Ts>(), true), 0)...};
		(void)dummy;
	}

	template <typename T>
	void setTag()
	{
		setTags<T>();
	}

	template <typename... Ts>
	void setComponentOrTags()
	{
		int dummy[] = {(
		    Base::set(Configuration::template componentOrTagBit<Ts>(), true), 0)...};
		(void)dummy;
	}

	template <typename T>
	void setComponentOrTag()
	{
		setComponentOrTags<T>();
	}

	template <typename... Ts>
	void clearComponents()
	{
		int dummy[] =
			{(Base::set(Configuration::template componentBit<Ts>(), false), 0)...};
		(void)dummy;
	}

	template <typename T>
	void clearComponent()
	{
		clearComponents<T>();
	}

	template <typename... Ts>
	void clearTags()
	{
		int dummy[] =
			{(Base::set(Configuration::template tagBit<Ts>(), false), 0)...};
		(void)dummy;
	}

	template <typename T>
	void clearTag()
	{
		clearTags<T>();
	}

	template <typename... Ts>
	void clearComponentOrTags()
	{
		int dummy[] = {(
		    Base::set(Configuration::template componentOrTagBit<Ts>(), false), 0)...};
		(void)dummy;
	}

	template <typename T>
	void clearComponentOrTag()
	{
		clearComponentOrTags<T>();
	}

    template <typename... Ts>
    static Signature create()
    {
        Signature signature{};

        int dummy[] = {(signature.template setComponentOrTag<Ts>(), 0)...};
        (void)dummy;

        return signature;
    }

	bool matches(const Signature& s) const
	{
		return s == (s & *this); 
	};

};

} // namespace ecs
