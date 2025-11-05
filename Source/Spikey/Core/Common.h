#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <span>
#include <array>
#include <functional>
#include <deque>
#include <map>
#include <unordered_map>
#include <stack>
#include <filesystem>
#include <type_traits>
#include <assert.h>

#define BIND_FUNCTION(x) std::bind(&x, this, std::placeholders::_1)
#define BIT(x) (1 << x)
#define CHECK(expr) assert(expr)

#define ENUM_FLAGS_OPERATORS(type)                                                                                                  \
inline type operator~(type a) { return (type)~(std::underlying_type_t<type>)a;}                                                     \
inline type operator|(type a, type b) { return (type)((std::underlying_type_t<type>)a | (std::underlying_type_t<type>)b); }         \
inline type operator&(type a, type b) { return (type)((std::underlying_type_t<type>)a & (std::underlying_type_t<type>)b); }         \
inline type operator^(type a, type b) { return (type)((std::underlying_type_t<type>)a ^ (std::underlying_type_t<type>)b); }         \
inline type& operator|=(type& a, type b) { return (type&)((std::underlying_type_t<type>&)a |= (std::underlying_type_t<type>&)b); }  \
inline type& operator&=(type& a, type b) { return (type&)((std::underlying_type_t<type>&)a &= (std::underlying_type_t<type>&)b); }  \
inline type& operator^=(type& a, type b) { return (type&)((std::underlying_type_t<type>&)a ^= (std::underlying_type_t<type>&)b); } 

template<typename T>
inline bool EnumHasAllFlags(T flags, T contains) {
	return ((std::underlying_type_t<T>)flags & (std::underlying_type_t<T>)contains) == ((std::underlying_type_t<T>)contains);
}

template<typename T>
inline bool EnumHasAnyFlags(T flags, T contains) {
	return ((std::underlying_type_t<T>)flags & (std::underlying_type_t<T>)contains) != 0;
}

using uint32 = unsigned int;
using uint64 = unsigned long long;
using int32 = int;
using int64 = long long;
using uint16 = unsigned short;
using int16 = short;
using uint8 = unsigned char;
using int8 = char;

using float32 = float;
using float64 = double;