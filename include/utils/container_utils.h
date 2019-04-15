#pragma once
#include <vector>
#include <unordered_map>
#include "utils/defines.h"

//TODO: Move these functions to the Utilities module

//Returns true if the index won't produce a out_of_range exception
template<typename T>
bool vector_in_range(const std::vector<T>& vector, size_t index)
{
	return index >= 0 && index < vector.size();
}

template<typename T, typename Allocator, typename... Args>
NO_DISCARD auto vector_emplace_back_and_return_iterator(std::vector<T, Allocator>& vector, Args&&... args) -> typename std::vector<T, Allocator>::iterator
{
#if __cplusplus >= 201703L
	// In C++17 emplace returns the iterator directly
	return vector.emplace_back(std::forward<Args>(args)...);
#else
	// Before C++17 emplace doesn't directly return the iterator
	vector.emplace_back(std::forward<Args>(args)...);
	return --vector.end();
#endif
}

template<typename T, typename Allocator>
NO_DISCARD auto vector_push_back_and_return_iterator(std::vector<T, Allocator>& vector, const T& value) -> typename std::vector<T, Allocator>::iterator
{
#if __cplusplus >= 201703L
	// In C++17 push_back returns the iterator directly
	return vector.push_back(value);
#else
	// Before C++17 push_back doesn't directly return the iterator
	vector.push_back(value);
	return --vector.end();
#endif
}

template<typename T, typename Allocator>
NO_DISCARD auto vector_push_back_and_return_iterator(std::vector<T, Allocator>& vector, T&& value) -> typename std::vector<T, Allocator>::iterator
{
#if __cplusplus >= 201703L
	// In C++17 push_back returns the iterator directly
	return vector.push_back(value);
#else
	// Before C++17 push_back doesn't directly return the iterator
	vector.push_back(value);
	return --vector.end();
#endif
}

// Will also check if insertion actually happened (when another element already exists, a assertion is called)
template<typename T, typename E, typename Hasher, typename KeyEq, typename Allocator, typename... Args>
NO_DISCARD auto unordered_map_emplace_and_return_iterator(std::unordered_map<T, E, Hasher, KeyEq, Allocator>& map, Args&&... args) 
	-> typename std::unordered_map<T, E, Hasher, KeyEq, Allocator>::iterator
{
	auto iterator_and_success = map.emplace(std::forward<Args>(args)...);
	assert(iterator_and_success.second);
	return iterator_and_success.first;
}

// Will NOT check if insertion actually happened (when another element already exists for example)
template<typename T, typename E, typename Hasher, typename KeyEq, typename Allocator, typename... Args>
NO_DISCARD auto unordered_map_emplace_and_return_iterator_no_check(std::unordered_map<T, E, Hasher, KeyEq, Allocator>& map, Args&&... args)
	-> typename std::unordered_map<T, E, Hasher, KeyEq, Allocator>::iterator
{
	auto iterator_and_success = map.emplace(std::forward<Args>(args)...);
	return iterator_and_success.first;
}
