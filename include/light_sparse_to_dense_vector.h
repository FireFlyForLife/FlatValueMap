#pragma once
#include <memory>
#include <unordered_map>
#include <cassert>

#include "utils/vector_utils.h"
#include "sparse_to_dense_handle.h"


namespace cof
{
	/// A sparse to dense vector. Does not include a index to vector map, so erasing elements is slower,
	/// but memory usage is less, and insertion time is slightly better.
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
