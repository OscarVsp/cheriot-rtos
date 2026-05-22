
#include <cstdint>
#include <type_traits>
#include "const.h"

template<typename T>
constexpr auto ms_from_cycles(T cycles)
  -> std::enable_if_t<std::is_integral_v<T>, T>
{
	if constexpr (std::is_unsigned_v<T>)
	{
		return static_cast<T>((static_cast<uint64_t>(cycles) * 1000ULL) /
		                      CPU_TIMER_HZ);
	}
	else
	{
		return static_cast<T>((static_cast<int64_t>(cycles) * 1000LL) /
		                      static_cast<int64_t>(CPU_TIMER_HZ));
	}
}

template<typename T>
constexpr auto cycles_from_ms(T ms)
  -> std::enable_if_t<std::is_integral_v<T>, T>
{
	if constexpr (std::is_unsigned_v<T>)
	{
		return static_cast<T>((static_cast<uint64_t>(ms) * CPU_TIMER_HZ) /
		                      1000ULL);
	}
	else
	{
		return static_cast<T>(
		  (static_cast<int64_t>(ms) * static_cast<int64_t>(CPU_TIMER_HZ)) /
		  1000LL);
	}
}

template<typename T>
constexpr auto us_from_cycles(T cycles)
  -> std::enable_if_t<std::is_integral_v<T>, T>
{
	if constexpr (std::is_unsigned_v<T>)
	{
		return static_cast<T>((static_cast<uint64_t>(cycles) * 1000000ULL) /
		                      CPU_TIMER_HZ);
	}
	else
	{
		return static_cast<T>((static_cast<int64_t>(cycles) * 1000000LL) /
		                      static_cast<int64_t>(CPU_TIMER_HZ));
	}
}

template<typename T>
constexpr auto cycles_from_us(T us)
  -> std::enable_if_t<std::is_integral_v<T>, T>
{
	if constexpr (std::is_unsigned_v<T>)
	{
		return static_cast<T>((static_cast<uint64_t>(us) * CPU_TIMER_HZ) /
		                      1000000ULL);
	}
	else
	{
		return static_cast<T>(
		  (static_cast<int64_t>(us) * static_cast<int64_t>(CPU_TIMER_HZ)) /
		  1000000LL);
	}
}