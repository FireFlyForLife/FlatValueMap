#pragma once
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cassert>

#include "utils/vector_utils.h"
#include "sparse_to_dense_handle.h"


namespace cof
{
	/// A sparse to dense vector. has a handle to index unordered_map for lookup, and index to handle unordered_map for quick erasing using erase remove idiom.
	template<typename T, typename Allocator = std::allocator<T>, typename SparseToDenseAllocator = Allocator, typename DenseToSparseAllocator = Allocator>
	class sparse_to_dense_vector
	{
	public:
		using handle_t = std_handle<T>;

	private:
		using SparseToDenseMap = std::unordered_map<handle_t, std::size_t, std::hash<handle_t>, std::equal_to<>, SparseToDenseAllocator>;
		using SparseToDenseIterator = typename SparseToDenseMap::iterator;
		using DenseToSparseMap = std::unordered_map<std::size_t, handle_t, std::hash<std::size_t>, std::equal_to<>, DenseToSparseAllocator>;
		using DenseToSparseIterator = typename DenseToSparseMap::iterator;
		using DenseVector = std::vector<T, Allocator>;

		SparseToDenseMap sparse_to_dense{};
		DenseToSparseMap dense_to_sparse{};
		DenseVector dense_vector;

		SparseToDenseIterator back_element_sparse_to_dense_iterator;
		DenseToSparseIterator back_element_dense_to_sparse_iterator;
		bool back_element_cached_iterator_valid = false;

		static uint32_t internalIdCounter;

	public:
		using iterator = typename DenseVector::iterator;
		using const_iterator = typename DenseVector::const_iterator;
		using reference = typename DenseVector::reference;
		using const_reference = typename DenseVector::const_reference;

	public:
		sparse_to_dense_vector() = default;

		auto push_back(const T& t)->handle_t;
		auto push_back(T&& t)->handle_t;

		void erase(handle_t handleToDelete);

		auto begin()->iterator;
		auto begin() const->const_iterator;
		auto end()->iterator;
		auto end() const->const_iterator;

		std::size_t size() const;
		bool empty() const;
		auto operator[](handle_t handle)->reference;
		auto operator[](handle_t handle) const->const_reference;
	};

	template<typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	uint32_t sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::internalIdCounter = 0;



#if __cplusplus >= 201703L
	namespace pmr {
		template<typename T, typename Allocator = std::pmr::polymorphic_allocator<T>>
		using sparse_to_dense_vector = cof::sparse_to_dense_vector<T, Allocator>;
	}
#endif
}



//=============================================================================
//                         IMPLEMENTATION:
//=============================================================================

namespace cof
{
	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::push_back(const T& t) -> handle_t
	{
		std::size_t element_index = dense_vector.size();
		uint32_t element_id = ++internalIdCounter;
		dense_vector.push_back(t);
		sparse_to_dense.emplace(handle_t{ element_id }, element_index);
		auto sparse_to_dense_it = sparse_to_dense.find(handle_t{ element_id });
		//TODO: Get direct returned iterator from .emplace()
		dense_to_sparse.emplace(element_index, handle_t{ element_id });
		auto dense_to_sparse_it = dense_to_sparse.find(element_index);
		back_element_sparse_to_dense_iterator = sparse_to_dense_it;
		back_element_dense_to_sparse_iterator = dense_to_sparse_it;
		back_element_cached_iterator_valid = true;

		return handle_t{ element_id };
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::push_back(T&& t) -> handle_t
	{
		std::size_t element_index = dense_vector.size();
		uint32_t element_id = ++internalIdCounter;
		dense_vector.push_back(t);
		sparse_to_dense.emplace(handle_t{ element_id }, element_index);
		auto sparse_to_dense_it = sparse_to_dense.find(handle_t{ element_id });
		dense_to_sparse.emplace(element_index, handle_t{ element_id });
		auto dense_to_sparse_it = dense_to_sparse.find(element_index);
		back_element_sparse_to_dense_iterator = sparse_to_dense_it;
		back_element_dense_to_sparse_iterator = dense_to_sparse_it;
		back_element_cached_iterator_valid = true;

		return handle_t{ element_id };
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	void sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::erase(handle_t handleToDelete)
	{
		auto removing_sparse_to_dense_it = sparse_to_dense.find(handleToDelete);
		assert(removing_sparse_to_dense_it != sparse_to_dense.end());
		std::size_t removed_element_index = removing_sparse_to_dense_it->second;

		DenseToSparseIterator back_dts_it;
		if (removed_element_index != dense_vector.size() - 1) {
			//Get the iterators for the back element where we are going to swap to 
			SparseToDenseIterator back_std_it;

			if (back_element_cached_iterator_valid) {
				back_dts_it = back_element_dense_to_sparse_iterator;
				back_std_it = back_element_sparse_to_dense_iterator;
			} else {
				back_dts_it = dense_to_sparse.find(dense_vector.size() - 1);
				back_std_it = sparse_to_dense.find(back_dts_it->second);
			}

			assert(vector_in_range(dense_vector, removed_element_index));
			auto& removed_element = dense_vector[removed_element_index];
			auto& last_element = dense_vector.back();
			std::swap(removed_element, last_element);

			//After the swap, we want to fixup the swapped elements indices and ids in the lookup maps
			back_std_it->second = removed_element_index;
			dense_to_sparse.at(removed_element_index) = back_std_it->first;
		} else {
			if (back_element_cached_iterator_valid) {
				back_dts_it = back_element_dense_to_sparse_iterator;
			} else {
				back_dts_it = dense_to_sparse.find(dense_vector.size() - 1);
			}
		}
		sparse_to_dense.erase(removing_sparse_to_dense_it);
		dense_to_sparse.erase(back_dts_it);
		dense_vector.pop_back();

		back_element_cached_iterator_valid = false;
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::begin() -> iterator
	{
		return dense_vector.begin();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::begin() const -> const_iterator
	{
		return dense_vector.begin();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::end() -> iterator
	{
		return dense_vector.end();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::end() const -> const_iterator
	{
		return dense_vector.end();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	std::size_t sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::size() const
	{
		return dense_vector.size();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	bool sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::empty() const
	{
		return dense_vector.empty();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::operator[](handle_t handle) -> reference
	{
		assert(sparse_to_dense.find(handle) != sparse_to_dense.end());
		auto element_index = sparse_to_dense.at(handle);
		assert(vector_in_range(dense_vector, element_index));
		return dense_vector[element_index];
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::operator[](
		handle_t handle) const -> const_reference
	{
		assert(sparse_to_dense.find(handle) != sparse_to_dense.end());
		auto element_index = sparse_to_dense.at(handle);
		assert(vector_in_range(dense_vector, element_index));
		return dense_vector[element_index];
	}
}
 