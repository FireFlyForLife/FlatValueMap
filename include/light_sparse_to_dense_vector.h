#pragma once
#include <memory>
#include <unordered_map>
#include <cassert>

#include "utils/container_utils.h"
#include "sparse_to_dense_handle.h"


namespace cof
{
	/** \brief A vector like container which indexes with sparse "handles" instead of indices directly. And still has contiguous memory for it's elements. 
	 *         light_sparse_to_dense_vector is more memory efficient then sparse_to_dense_vector but has a higher erase complexity on average.
	 *
	 * \class light_sparse_to_dense_vector
	 *
	 * A sparse_to_dense_vector is a vector which uses a handle to access it's members instead of members directly. This level of indirection is useful if you need your indices to stay valid even if things get deleted etc.
	 * The way it works is when you call operator[] with the handle, it first goes through a `unordered_map<handle_t, index_t>`(sparse to dense map) to get the index in the internal vector. This means that the elements themselves are still stored contiguously.
	 * For erase this means we can make use of the swap erase idiom to avoid moving all later elements. However, to do the index to handle lookup, we need to iterate over the unordered_map which results in lower memory usage but a higher complexity for erase().
	 * If speed is more important then the added memory cof::sparse_to_dense_vector should be used.
	*/

	template<typename T, typename Allocator = std::allocator<T>, typename SparseToDenseAllocator = Allocator>
	class light_sparse_to_dense_vector
	{
	public:
		using handle_t = lstd_handle<T>;

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

	public:
		light_sparse_to_dense_vector() = default;

		auto push_back(const T& t) -> handle_t;
		auto push_back(T&& t) -> handle_t;
		template<typename... Args>
		auto emplace_back(Args&&... args)->handle_t;

		void erase(handle_t handleToRemove);

		size_t size() const;
		bool empty() const;
		bool contains(handle_t handle) const;

		void clear();

		auto operator[](handle_t handle)->reference;
		auto operator[](handle_t handle) const->const_reference;

		auto front()->reference;
		auto front() const->const_reference;
		auto back()->reference;
		auto back() const->const_reference;
		auto data()->pointer;
		auto data() const->const_pointer;

		auto begin()->iterator;
		auto begin() const->const_iterator;
		auto end()->iterator;
		auto end() const->const_iterator;

		auto find(handle_t handle)->iterator;
		auto find(handle_t handle) const->const_iterator;
		
	private:
		auto dense_to_sparse(std::size_t denseIndex)->SparseToDenseIterator;
	};

	template<typename T, typename Allocator, typename SparseToDenseAllocator>
	uint32_t light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::internalIdCounter = 0;


#if __cplusplus > 201703L
	namespace pmr {
		template<typename T, typename Allocator = std::pmr::polymorphic_allocator<T>>
		using light_sparse_to_dense_vector = cof::light_sparse_to_dense_vector<T, Allocator>;
}
#endif
}



//=============================================================================
//                         IMPLEMENTATION:
//=============================================================================

namespace cof
{
	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::push_back(const T& t) -> handle_t
	{
		std::size_t element_index = dense_vector.size();
		uint32_t element_id = ++internalIdCounter;
		dense_vector.push_back(t);
		sparse_to_dense.emplace(handle_t{ element_id }, element_index);

		return handle_t{ element_id };
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::push_back(T&& t) -> handle_t
	{
		std::size_t element_index = dense_vector.size();
		uint32_t element_id = ++internalIdCounter;
		dense_vector.push_back(std::move(t));
		sparse_to_dense.emplace(handle_t{element_id}, element_index);

		return handle_t{element_id};
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	template <typename ... Args>
	auto light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::emplace_back(Args&&... args) -> handle_t
	{
		std::size_t element_index = dense_vector.size();
		uint32_t element_id = ++internalIdCounter;
		dense_vector.emplace_back(std::forward<Args>(args)...);
		sparse_to_dense.emplace(handle_t{ element_id }, element_index);

		return handle_t{ element_id };
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	void light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::erase(handle_t handleToRemove)
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
	size_t light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::size() const
	{
		return dense_vector.size();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	bool light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::empty() const
	{
		return dense_vector.empty();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	bool light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::contains(handle_t handle) const
	{
		return sparse_to_dense.find(handle) != sparse_to_dense.end();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	void light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::clear()
	{
		dense_vector.clear();
		sparse_to_dense.clear();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::operator[](handle_t handle) -> reference {
		auto std_it = sparse_to_dense.find(handle);
		assert(std_it != sparse_to_dense.end());
		std::size_t element_index = std_it->second;
		assert(vector_in_range(dense_vector, element_index));
		return dense_vector[element_index];
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::operator[](
		handle_t handle) const -> const_reference {

		auto std_it = sparse_to_dense.find(handle);
		assert(std_it != sparse_to_dense.end());
		std::size_t element_index = std_it->second;
		assert(vector_in_range(dense_vector, element_index));
		return dense_vector[element_index];
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::front() -> reference
	{
		return dense_vector.front();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::front() const -> const_reference
	{
		return dense_vector.front();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::back() -> reference
	{
		return dense_vector.back();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::back() const -> const_reference
	{
		return dense_vector.back();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::data() -> pointer
	{
		return dense_vector.data();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::data() const -> const_pointer
	{
		return dense_vector.data();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::begin() -> iterator
	{
		return dense_vector.begin();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::begin() const -> const_iterator
	{
		return dense_vector.begin();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::end() -> iterator
	{
		return dense_vector.end();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::end() const -> const_iterator
	{
		return dense_vector.end();
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::find(handle_t handle) -> iterator
	{
		auto sparse_to_dense_it = sparse_to_dense.find(handle);
		if(sparse_to_dense_it != sparse_to_dense.end()) {
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
	auto light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::find(
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

	template <typename T, typename Allocator, typename SparseToDenseAllocator>
	auto light_sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator>::dense_to_sparse(
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
}
