#pragma once
#include <vector>

//TODO: Move these functions to the Utilities module

//Returns true if the index won't produce a out_of_range exception
template<typename T>
bool vector_in_range(const std::vector<T>& vector, size_t index)
{
	return index >= 0 && index < vector.size();
}
