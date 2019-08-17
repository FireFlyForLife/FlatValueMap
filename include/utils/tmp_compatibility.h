#if _MSVC_LANG < 201703L
#include <memory>
#endif


namespace cof
{
	/// TMP Compatibilty for pre C++17 std::allocator::rebind
	template<typename T, typename E>
	struct rebind
	{
#if _MSVC_LANG < 201703L
		using other = typename T::template rebind< E >::other;
#endif
	};
#if _MSVC_LANG >= 201703L
	template<template<typename> typename T, typename TO, typename E>
	struct rebind<T<TO>, E>
	{
		using other = T<E>;
	};
#endif
	
}