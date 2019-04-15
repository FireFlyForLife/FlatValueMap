#pragma once
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cassert>

#include "utils/container_utils.h"
#include "sparse_to_dense_handle.h"


namespace cof
{
	// ReSharper disable CppInconsistentNaming


	/** \brief A vector like container which indexes with sparse "handles" instead of indices directly. And still has contiguous memory for it's elements. 
	 *         sparse_to_dense_vector uses more memory then light_sparse_to_dense_vector but has a lower erase() complexity on average.
	 * 
	 * \class sparse_to_dense_vector
	 * 
	 *  A sparse_to_dense_vector is a vector which uses a handle to access it's members instead of members directly. This level of indirection is useful if you need your indices to stay valid even if things get deleted etc. 
	 * The way it works is when you call operator[] with the handle, it first goes through a `unordered_map<handle_t, index_t>`(sparse to dense map) to get the index in the internal vector. This means that the elements themselves are still stored contiguously.
	 * For erase this means we can make use of the swap erase idiom to avoid moving all later elements. But to efficiently implement this, a second `unordered_map<index_t, handle_t>`(dense to sparse map) is used for getting the handle from an id.
	 * This extra "dense to sparse map" costs more memory but will increase speed. If this tradeoff is not undesired take a look at cof::light_sparse_to_dense_vector .
	*/
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

		// The sparse_to_dense map is used for finding a the raw index of the dense_vector from a sparse handle
		SparseToDenseMap sparse_to_dense{};
		// The dense_to_sparse map is used for finding a sparse handle from a raw dense_vector index.
		DenseToSparseMap dense_to_sparse{};
		// The internal dense_vector, contains all elements contiguously. 
		DenseVector dense_vector;

		SparseToDenseIterator back_element_sparse_to_dense_iterator;
		DenseToSparseIterator back_element_dense_to_sparse_iterator;
		bool back_element_cached_iterator_valid = false;

		static uint32_t internalIdCounter;

	public:
		using value_type = T;
		using allocator_type = Allocator;
		using size_type = typename DenseVector::size_type;
		using difference_type = typename DenseVector::difference_type;
		using reverse_iterator = typename DenseVector::reverse_iterator;
		using const_reverse_iterator = typename DenseVector::const_reverse_iterator;

		using iterator = typename DenseVector::iterator;
		using const_iterator = typename DenseVector::const_iterator;
		using reference = typename DenseVector::reference;
		using const_reference = typename DenseVector::const_reference;
		using pointer = typename DenseVector::pointer;
		using const_pointer = typename DenseVector::const_pointer;

		using sparse_to_dense_iterator = typename SparseToDenseMap::iterator;
		using const_sparse_to_dense_iterator = typename SparseToDenseMap::const_iterator;

	public:
		sparse_to_dense_vector() = default;

		// pushes back the moved element `t` to the internal dense_vector
		auto push_back(const T& t)->handle_t;
		// pushes back the a copy of element `t` to the internal dense_vector
		auto push_back(T&& t)->handle_t;
		// construct an element in place at the end of the internal dense_vector
		template<typename... Args>
		auto emplace_back(Args&&... args)->handle_t;

		// erase a element from the vector. This overload is the most efficient
		void erase(handle_t handleToDelete);
		// erase a element from the vector. This overload is NOT the most efficient
		// This overload does one dense_to_sparse unordered_map lookup and then calls erase() with the sparse handle
		void erase(const_iterator position);
		// erase a element from the vector. This overload is NOT the most efficient
		// This overload does a dense_to_sparse unordered_map lookup for every element in the range and then calls erase() with each sparse handle
		void erase(const_iterator first, const_iterator last);

		// Get a iterator to the first element in the dense_vector
		auto begin()->iterator;
		// Get a const iterator to the first element in the dense_vector
		auto begin() const->const_iterator;
		// Get a const iterator to the first element in the dense_vector
		auto cbegin() const->const_iterator;
		// Get a iterator past the last element in the dense_vector
		auto end()->iterator;
		// Get a const iterator past the last element in the dense_vector
		auto end() const->const_iterator;
		// Get a const iterator past the last element in the dense_vector
		auto cend() const->const_iterator;
		// Get a reverse iterator to the first element of the reversed container.
		auto rbegin()->iterator;
		// Get a const reverse iterator to the first element of the reversed container.
		auto rbegin() const->const_iterator;
		// Get a const reverse iterator to the first element of the reversed container.
		auto crbegin() const->const_iterator;
		// Get a reverse iterator to the last element of the reversed container.
		auto rend()->iterator;
		// Get a const reverse iterator to the last element of the reversed container.
		auto rend() const->const_iterator;
		// Get a const reverse iterator to the last element of the reversed container.
		auto crend() const->const_iterator;

		//TODO: Let the handles_begin & handles_end return a iterator which only exposes the handles, and not the indices.

		// Get a iterator to the beginning of the sparse handles map;
		auto handles_begin()->sparse_to_dense_iterator;
		// Get a const iterator to the beginning of the sparse handles map;
		auto handles_begin() const->const_sparse_to_dense_iterator;
		// Get a const iterator to the beginning of the sparse handles map;
		auto handles_cbegin() const->const_sparse_to_dense_iterator;
		// Get a iterator to the end of the sparse handles map;
		auto handles_end()->sparse_to_dense_iterator;
		// Get a const iterator to the end of the sparse handles map;
		auto handles_end() const->const_sparse_to_dense_iterator;
		// Get a const iterator to the end of the sparse handles map;
		auto handles_cend() const->const_sparse_to_dense_iterator;

		// Get a reference the first element in the vector
		auto front()->reference;
		// Get a const reference to the first element in the vector
		auto front() const->const_reference;
		// Get a reference to the last element in the vector
		auto back()->reference;
		// Get a const reference to the last element in the vector
		auto back() const->const_reference;
		// Get the data pointer to the contiguous elements
		auto data()->pointer;
		// Get the const data pointer to the contiguous elements
		auto data() const->const_pointer;

		// Check if this sparse_to_dense_vector contains a element with this handle.
		bool contains(handle_t handle) const;
		// \returns a iterator to the element in the internal dense_vector if found. Else returns end()
		auto find(handle_t handle) -> iterator;
		// \returns a const iterator to the element in the internal dense_vector if found. Else returns cend()
		auto find(handle_t handle) const->const_iterator;

		// The amount of elements in this vector
		std::size_t size() const;
		// \returns if the amount of elements in this vector equal to zero
		bool empty() const;

		// Erase all elements(and thus deconstruct all elements)
		void clear();

		// Get the element indexed by it's handle
		auto operator[](handle_t handle)->reference;
		// Get the const element indexed by it's handle
		auto operator[](handle_t handle) const->const_reference;
	};

#if __cplusplus >= 201703L
	namespace pmr {
		/**
		 * \brief A polymorphic memory allocator version of sparse_to_dense_vector
		 *	Details
		 */
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
	template<typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	uint32_t sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::internalIdCounter = 0; 

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::push_back(const T& t) -> handle_t
	{
		std::size_t element_index = dense_vector.size();
		uint32_t element_id = ++internalIdCounter;
		dense_vector.push_back(t);
		auto sparse_to_dense_it = unordered_map_emplace_and_return_iterator(sparse_to_dense, handle_t{ element_id }, element_index);
		auto dense_to_sparse_it = unordered_map_emplace_and_return_iterator(dense_to_sparse, element_index, handle_t{element_id});
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
		auto sparse_to_dense_it = unordered_map_emplace_and_return_iterator(sparse_to_dense, handle_t{element_id}, element_index);
		auto dense_to_sparse_it = unordered_map_emplace_and_return_iterator(dense_to_sparse, element_index, handle_t{ element_id });
		back_element_sparse_to_dense_iterator = sparse_to_dense_it;
		back_element_dense_to_sparse_iterator = dense_to_sparse_it;
		back_element_cached_iterator_valid = true;

		return handle_t{ element_id };
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	template <typename ... Args>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::emplace_back(
		Args&&... args) -> handle_t
	{
		std::size_t element_index = dense_vector.size();
		uint32_t element_id = ++internalIdCounter;
		dense_vector.emplace_back(std::forward<Args>(args)...);
		auto sparse_to_dense_it = unordered_map_emplace_and_return_iterator(sparse_to_dense, handle_t{element_id}, element_index);
		auto dense_to_sparse_it = unordered_map_emplace_and_return_iterator(dense_vector, element_index, handle_t{element_id});
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
	void sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::erase(
		const_iterator position)
	{
		std::size_t element_index = position - dense_vector.begin();
		//TODO: Optimize this with custom function
		assert(dense_to_sparse.find(element_index) != dense_to_sparse.end());
		handle_t handle = dense_to_sparse.at(element_index);
		
		erase(handle);
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	void sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::erase(
		const_iterator first, const_iterator last)
	{
		//TODO: Look for optimizations

		auto count = last - first;
		std::vector<handle_t> handles( count );

		auto offset = first - begin();
		for (int i = 0; i < count; ++i) {
			auto dense_to_sparse_it = dense_to_sparse.find(offset + i);
			assert(dense_to_sparse_it != dense_to_sparse.end());
			handles[i] = dense_to_sparse_it->second;
		}

		for (handle_t handle : handles) {
			erase(handle);
		}
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
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::
	cbegin() const -> const_iterator
	{
		return dense_vector.cbegin();
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
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::
	cend() const -> const_iterator
	{
		return dense_vector.cend();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::rbegin() -> iterator
	{
		return dense_vector.rbegin();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::
	rbegin() const -> const_iterator
	{
		return dense_vector.rbegin();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::
	crbegin() const -> const_iterator
	{
		return dense_vector.crbegin();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::rend() -> iterator
	{
		return dense_vector.rend();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::
	rend() const -> const_iterator
	{
		return dense_vector.rend();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::
	crend() const -> const_iterator
	{
		return dense_vector.crbegin();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::
	handles_begin() -> sparse_to_dense_iterator
	{
		return sparse_to_dense.begin();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::
	handles_begin() const -> const_sparse_to_dense_iterator
	{
		return sparse_to_dense.begin();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::
	handles_cbegin() const -> const_sparse_to_dense_iterator
	{
		return sparse_to_dense.cbegin();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::
	handles_end() -> sparse_to_dense_iterator
	{
		return sparse_to_dense.end();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::
	handles_end() const -> const_sparse_to_dense_iterator
	{
		return sparse_to_dense.end();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::
	handles_cend() const -> const_sparse_to_dense_iterator
	{
		return sparse_to_dense.cend();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::front() -> reference
	{
		return dense_vector.front();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::
	front() const -> const_reference
	{
		return dense_vector.front();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::back() -> reference
	{
		return dense_vector.back();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::
	back() const -> const_reference
	{
		return dense_vector.back();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::data() -> pointer
	{
		return dense_vector.data();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::
	data() const -> const_pointer
	{
		return dense_vector.data();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	bool sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::contains(
		handle_t handle) const
	{
		return sparse_to_dense.find(handle) != sparse_to_dense.end();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::find(
		handle_t handle) -> iterator
	{
		auto sparse_to_dense_it = sparse_to_dense.find(handle);
		if(sparse_to_dense_it != sparse_to_dense.end()) {
			std::size_t element_index = sparse_to_dense_it->second;
			assert(vector_in_range(dense_vector, element_index));

			auto dense_it = dense_vector.begin();
			std::advance(dense_it, element_index);
			return dense_it;
		}

		return dense_vector.end();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::find(
		handle_t handle) const -> const_iterator
	{
		auto sparse_to_dense_it = sparse_to_dense.find(handle);
		if (sparse_to_dense_it != sparse_to_dense.end()) {
			std::size_t element_index = sparse_to_dense_it->second;
			assert(vector_in_range(dense_vector, element_index));

			auto dense_it = dense_vector.begin();
			std::advance(dense_it, element_index);
			return dense_it;
		}

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
	void sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::clear()
	{
		dense_vector.clear();
		sparse_to_dense.clear();
		dense_to_sparse.clear();
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

	// ReSharper restore CppInconsistentNaming
}
 