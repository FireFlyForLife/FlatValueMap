#pragma once
#include <memory>
#include <unordered_map>
#include <cassert>

#include "utils/container_utils.h"
#include "flat_value_map_handle.h"


namespace cof
{
	// ReSharper disable CppInconsistentNaming


	/** \brief A vector like container which indexes with sparse "handles" instead of indices directly. And still has contiguous memory for it's elements. 
	 *         light_flat_value_map is more memory efficient then flat_value_map but has a higher erase complexity on average.
	 *
	 * \class light_flat_value_map
	 *
	 * A flat_value_map is a vector which uses a handle to access it's members instead of members directly. This level of indirection is useful if you need your indices to stay valid even if things get deleted etc.
	 * The way it works is when you call operator[] with the handle, it first goes through a `unordered_map<handle_t, index_t>`(sparse to dense map) to get the index in the internal vector. This means that the elements themselves are still stored contiguously.
	 * For erase this means we can make use of the swap erase idiom to avoid moving all later elements. However, to do the index to handle lookup, we need to iterate over the unordered_map which results in lower memory usage but a higher complexity for erase().
	 * If speed is more important then the added memory cof::flat_value_map should be used.
	*/
	template<typename T, typename Allocator = std::allocator<T>, typename SparseToDenseAllocator = Allocator>
	class light_flat_value_map
	{
	public:
		using handle_t = lfvm_handle<T>;

	private:
		using SparseToDenseMap = std::unordered_map<handle_t, std::size_t, std::hash<handle_t>, std::equal_to<>, Allocator>;
		using SparseToDenseIterator = typename SparseToDenseMap::iterator;
		using DenseVector = std::vector<T, Allocator>;
		using DenseVectorIterator = typename DenseVector::iterator;

		SparseToDenseMap sparse_to_dense{};
		DenseVector dense_vector;

		static uint32_t internalIdCounter;

	public:
		using iterator = typename DenseVector::iterator;
		using const_iterator = typename DenseVector::const_iterator;
		using reference = typename DenseVector::reference;
		using const_reference = typename DenseVector::const_reference;
		using pointer = typename DenseVector::pointer;
		using const_pointer = typename DenseVector::const_pointer;

		using sparse_to_dense_iterator = typename SparseToDenseMap::iterator;
		using const_sparse_to_dense_iterator = typename SparseToDenseMap::const_iterator;

	public:
		light_flat_value_map() = default;


		/// \Category Element access

		// Get the element indexed by it's handle
		auto operator[](handle_t handle)->reference;
		// Get the const element indexed by it's handle
		auto operator[](handle_t handle) const->const_reference;

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

		// Check if this flat_value_map contains a element with this handle.
		bool contains(handle_t handle) const;
		//Temporarily disabled these functions until Issue #1 is resolved
#ifdef ENABLE_LIGHT_SPARSE_TO_DENSE_VECTOR_FIND
		auto find(handle_t handle)->iterator;
		auto find(handle_t handle) const->const_iterator;
#endif //END: ENABLE_LIGHT_SPARSE_TO_DENSE_VECTOR_FIND


		/// \Category Iterators

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

		
		/// \Category Capacity

		// The amount of elements in this vector
		size_t size() const;
		// \returns if the amount of elements in this vector equal to zero
		bool empty() const;


		/// \Category Modifiers

		// pushes back the moved element `t` to the internal dense_vector
		auto push_back(const T& t)->handle_t;
		// pushes back the a copy of element `t` to the internal dense_vector
		auto push_back(T&& t)->handle_t;
		// construct an element in place at the end of the internal dense_vector
		template<typename... Args>
		auto emplace_back(Args&&... args)->handle_t;

		// erase a element from the vector. This overload is the most efficient
		void erase(handle_t handleToRemove);

		// Erase all elements(and thus deconstruct all elements)
		void clear();
		
	private:
		// Do a dense to sparse lookup (raw index to sparse handle) by iterating over the sparse_to_dense map
		auto dense_to_sparse(std::size_t denseIndex)->SparseToDenseIterator;
	};

#if __cplusplus > 201703L
	namespace pmr {
		/**
		 * \brief A polymorphic memory allocator version of light_flat_value_map
		 *	Details
		 */
		template<typename T, typename Allocator = std::pmr::polymorphic_allocator<T>>
		using light_flat_value_map = cof::light_flat_value_map<T, Allocator>;
}
#endif
}



//=============================================================================
//                         IMPLEMENTATION:
//=============================================================================

namespace cof
{
	template<typename T, typename Allocator, typename SparseToDenseAllocator>
	uint32_t light_flat_value_map<T, Allocator, SparseToDenseAllocator>::internalIdCounter = 0;


	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::operator[](handle_t handle) -> reference {
		auto std_it = sparse_to_dense.find(handle);
		assert(std_it != sparse_to_dense.end());
		std::size_t element_index = std_it->second;
		assert(vector_in_range(dense_vector, element_index));
		return dense_vector[element_index];
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::operator[](
		handle_t handle) const -> const_reference {

		auto std_it = sparse_to_dense.find(handle);
		assert(std_it != sparse_to_dense.end());
		std::size_t element_index = std_it->second;
		assert(vector_in_range(dense_vector, element_index));
		return dense_vector[element_index];
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::front() -> reference
	{
		return dense_vector.front();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::front() const -> const_reference
	{
		return dense_vector.front();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::back() -> reference
	{
		return dense_vector.back();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::back() const -> const_reference
	{
		return dense_vector.back();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::data() -> pointer
	{
		return dense_vector.data();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::data() const -> const_pointer
	{
		return dense_vector.data();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	bool light_flat_value_map<T, Allocator, SparseToDenseAllocator>::contains(handle_t handle) const
	{
		return sparse_to_dense.find(handle) != sparse_to_dense.end();
	}

#ifdef ENABLE_LIGHT_SPARSE_TO_DENSE_VECTOR_FIND
	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::find(handle_t handle) -> iterator
	{
		auto sparse_to_dense_it = sparse_to_dense.find(handle);
		if (sparse_to_dense_it != sparse_to_dense.end()) {
			std::size_t element_index = sparse_to_dense_it->second;
			assert(vector_in_range(dense_vector, element_index));

			auto dense_it = dense_vector.begin();
			std::advance(dense_it, element_index);
			return dense_it;
		} else {
			return dense_vector.end();
		}
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::find(
		handle_t handle) const -> const_iterator
	{
		auto sparse_to_dense_it = sparse_to_dense.find(handle);
		if (sparse_to_dense_it != sparse_to_dense.end()) {
			std::size_t element_index = sparse_to_dense_it->second;
			assert(vector_in_range(dense_vector, element_index));

			auto dense_it = dense_vector.begin();
			std::advance(dense_it, element_index);
			return dense_it;
		} else {
			return dense_vector.end();
		}
	}
#endif //END: ifdef ENABLE_LIGHT_SPARSE_TO_DENSE_VECTOR_FIND


	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::begin() -> iterator
	{
		return dense_vector.begin();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::begin() const -> const_iterator
	{
		return dense_vector.begin();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::cbegin() const -> const_iterator
	{
		return dense_vector.cbegin();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::end() -> iterator
	{
		return dense_vector.end();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::end() const -> const_iterator
	{
		return dense_vector.end();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::cend() const -> const_iterator
	{
		return dense_vector.cend();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::rbegin() -> iterator
	{
		return dense_vector.rbegin();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::rbegin() const -> const_iterator
	{
		return dense_vector.rbegin();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::crbegin() const -> const_iterator
	{
		return dense_vector.crbegin();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::rend() -> iterator
	{
		return dense_vector.rend();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::rend() const -> const_iterator
	{
		return dense_vector.rend();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::crend() const -> const_iterator
	{
		return dense_vector.crend();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::
		handles_begin() -> sparse_to_dense_iterator
	{
		return sparse_to_dense.begin();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::
		handles_begin() const -> const_sparse_to_dense_iterator
	{
		return sparse_to_dense.begin();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::
		handles_cbegin() const -> const_sparse_to_dense_iterator
	{
		return sparse_to_dense.cbegin();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::
		handles_end() -> sparse_to_dense_iterator
	{
		return sparse_to_dense.end();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::
		handles_end() const -> const_sparse_to_dense_iterator
	{
		return sparse_to_dense.end();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::
		handles_cend() const -> const_sparse_to_dense_iterator
	{
		return sparse_to_dense.cend();
	}


	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	size_t light_flat_value_map<T, Allocator, SparseToDenseAllocator>::size() const
	{
		return dense_vector.size();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	bool light_flat_value_map<T, Allocator, SparseToDenseAllocator>::empty() const
	{
		return dense_vector.empty();
	}


	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::push_back(const T& t) -> handle_t
	{
		std::size_t element_index = dense_vector.size();
		uint32_t element_id = ++internalIdCounter;
		dense_vector.push_back(t);
		sparse_to_dense.emplace(handle_t{ element_id }, element_index);

		return handle_t{ element_id };
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::push_back(T&& t) -> handle_t
	{
		std::size_t element_index = dense_vector.size();
		uint32_t element_id = ++internalIdCounter;
		dense_vector.push_back(std::move(t));
		sparse_to_dense.emplace(handle_t{element_id}, element_index);

		return handle_t{element_id};
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	template <typename ... Args>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::emplace_back(Args&&... args) -> handle_t
	{
		std::size_t element_index = dense_vector.size();
		uint32_t element_id = ++internalIdCounter;
		dense_vector.emplace_back(std::forward<Args>(args)...);
		sparse_to_dense.emplace(handle_t{ element_id }, element_index);

		return handle_t{ element_id };
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	void light_flat_value_map<T, Allocator, SparseToDenseAllocator>::erase(handle_t handleToRemove)
	{
		auto sparse_to_dense_it = sparse_to_dense.find(handleToRemove);
		assert(sparse_to_dense_it != sparse_to_dense.end());
		if (sparse_to_dense_it != sparse_to_dense.end()) {
			std::size_t element_index = sparse_to_dense_it->second;

			if (element_index != dense_vector.size() - 1) {
				// If not the last element, swap to last place and do fix up.
				std::size_t last_element = dense_vector.size() - 1;
				std::swap(dense_vector[element_index], dense_vector[last_element]);
				auto std_last_element_it = dense_to_sparse(last_element);
				std_last_element_it->second = element_index;
			}
			dense_vector.pop_back();
			sparse_to_dense.erase(sparse_to_dense_it);
		}
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	void light_flat_value_map<T, Allocator, SparseToDenseAllocator>::clear()
	{
		dense_vector.clear();
		sparse_to_dense.clear();
	}


	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_flat_value_map<T, Allocator, SparseToDenseAllocator>::dense_to_sparse(
		std::size_t denseIndex) -> SparseToDenseIterator
	{
		for (auto it = sparse_to_dense.begin(); it != sparse_to_dense.end(); ++it) {
			if(denseIndex == it->second) {
				return it;
			}
		}
		assert(false);
		return sparse_to_dense.end();
	}


	// ReSharper restore CppInconsistentNaming
}
