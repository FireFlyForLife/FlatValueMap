#pragma once
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cassert>

#include "utils/vector_utils.h"

// namespace cof
// {
//     //Forward declarations
//     template<typename T, typename Allocator = std::allocator<T>, typename SparseToDenseAllocator = Allocator, typename DenseToSparseAllocator = Allocator>
//     class sparse_to_dense_vector;
//
//     template<typename T, typename Allocator = std::allocator<T>, typename SparseToDenseAllocator = Allocator, typename DenseToSparseAllocator = Allocator>
//     class light_sparse_to_dense_vector;
// }

namespace cof
{
	/// Handle for a `cof::sparse_to_dense_vector<T>`
	template<typename T>
	struct std_handle {
		uint32_t id;
	};
	/// Handle for a `cof::light_sparse_to_dense_vector<T>`
	template<typename T>
	struct lstd_handle {
		uint32_t id;
	};

    /// A sparse to dense vector. has a handle to index unordered_map for lookup, and index to handle unordered_map for quick erasing using erase remove idiom.
    template<typename T, typename Allocator = std::allocator<T>, typename SparseToDenseAllocator = Allocator, typename DenseToSparseAllocator = Allocator>
    class sparse_to_dense_vector
    {
    public:
		using handle_t = std_handle<T>;

    private:
        using SparseToDenseMap = std::unordered_map<handle_t, std::size_t, std::hash<handle_t>, std::equal_to<>, SparseToDenseAllocator>;
		using SparseToDenseIterator = typename SparseToDenseMap::iterator;
        using DenseToSparseMap = std::unordered_map<std::size_t, SparseToDenseIterator, std::hash<std::size_t>, std::equal_to<>, DenseToSparseAllocator>;
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

		handle_t push_back(const T& t);
		auto push_back(T&& t) -> handle_t;

		void erase(handle_t handleToDelete);

		auto begin()->iterator;
		auto begin() const->const_iterator;
		auto end()->iterator;
		auto end() const->const_iterator;

		std::size_t size() const;
		bool empty() const;
		auto operator[](handle_t handle) -> reference;
		auto operator[](handle_t handle) const -> const_reference;
    };

	template<typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	uint32_t sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::internalIdCounter = 0;

    /// A sparse to dense vector. Does not include a index to vector map, so erasing elements is slower,
    /// but memmory usage is less, and insertion time is slightly better.
  //   template<typename T, typename Allocator = std::allocator<T>, typename SparseToDenseAllocator = Allocator, typename DenseToSparseAllocator = Allocator>
  //   class light_sparse_to_dense_vector
  //   {
  //   public:
  //       struct handle{
  //           uint32_t id;
  //       };
  //
  //   private:
  //       std::unordered_map<handle, std::size_t, std::hash<handle>, std::equal_to<handle>, Allocator> sparse_to_dense{};
  //       std::vector<T, Allocator> dense_vector;
  //
  //       static uint32_t internalIdCounter = 0;
  //
  //   public:
		// light_sparse_to_dense_vector() = default;
  //       
  //       handle push_back(const T& t) {
  //           std::size_t element_index = dense_vector.size();
  //           uint32_t element_id = ++internalIdCounter;
  //           dense_vector.push_back(t);
  //           sparse_to_dense.emplace(handle{element_id}, element_index);
  //
  //           return handle{element_id};
  //       }
  //       handle push_back(T&& t) {
  //           std::size_t element_index = dense_vector.size();
  //           uint32_t element_id = ++internalIdCounter;
  //           dense_vector.push_back(t);
  //           sparse_to_dense.emplace(handle{element_id}, element_index);
  //
  //           return handle{element_id};
  //       }
  //
  //   };

#if __cplusplus > 201703L
    namespace pmr {
        template<typename T, typename Allocator = std::pmr::polymorphic_allocator<T>>
        using sparse_to_dense_vector = cof::sparse_to_dense_vector<T, Allocator>;

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
	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::push_back(const T& t) -> handle_t
	{
		std::size_t element_index = dense_vector.size();
		uint32_t element_id = ++internalIdCounter;
		dense_vector.push_back(t);
		sparse_to_dense.emplace(handle_t{ element_id }, element_index);
		auto it = sparse_to_dense.find(handle_t{ element_id });
		dense_to_sparse.emplace(element_index, it);
		back_element_sparse_to_dense_iterator = dense_to_sparse.at(element_index);
		back_element_sparse_to_dense_iterator = it;
		back_element_cached_iterator_valid = true;

		return handle_t{ element_id };
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::push_back(T&& t) -> handle_t
	{
		std::size_t element_index = dense_vector.size();
		uint32_t element_id = ++internalIdCounter;
		dense_vector.push_back(t);
		sparse_to_dense.emplace(handle_t{element_id}, element_index);
		auto it = sparse_to_dense.find(handle_t{element_id});
		dense_to_sparse.emplace(element_index, it);
		back_element_sparse_to_dense_iterator = dense_to_sparse.at(element_index);
		back_element_sparse_to_dense_iterator = it;
		back_element_cached_iterator_valid = true;

		return handle_t{element_id};
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	void sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::erase(handle_t handleToDelete)
	{
		auto& removing_sparse_to_dense_it = sparse_to_dense.find(handleToDelete);
		assert(removing_sparse_to_dense_it != sparse_to_dense.end());
		SparseToDenseIterator std_it;
		typename DenseToSparseMap::iterator dts_it;
		if(back_element_cached_iterator_valid)
		{
			std_it = back_element_sparse_to_dense_iterator;
			dts_it = back_element_dense_to_sparse_iterator;
		}
		else
		{
			dts_it = dense_to_sparse.find(dense_vector.size() - 1);
			std_it = dts_it->second;
		}
		back_element_cached_iterator_valid = false;

		auto removed_element_index = removing_sparse_to_dense_it->second;
		assert(!dense_vector.empty() && vector_in_range(dense_vector, removed_element_index));
		auto& removed_element = dense_vector.at(removed_element_index);
		auto& last_element = dense_vector.back();
		std::swap(removed_element, last_element);

		std_it->second = removed_element_index;
		sparse_to_dense.erase(removing_sparse_to_dense_it);
		dense_to_sparse.erase(dts_it);
		dense_vector.erase(--dense_vector.end());
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
		return dense_vector.at(element_index);
	}

	template <typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
	auto sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::operator[](
		handle_t handle) const -> const_reference
	{
		assert(sparse_to_dense.find(handle) != sparse_to_dense.end());
		auto element_index = sparse_to_dense.at(handle);
		assert(vector_in_range(dense_vector, element_index));
		return dense_vector.at(element_index);
	}


	template<typename T>
	bool operator==(const std_handle<T>& lhs, const std_handle<T>& rhs)
	{
		return lhs.id == rhs.id;
	}

	template<typename T>
	bool operator==(const lstd_handle<T>& lhs, const lstd_handle<T>& rhs)
	{
		return lhs.id == rhs.id;
	}
}

namespace std
{
    template<typename T>
    struct hash<cof::std_handle<T>>
    {
	    std::size_t operator()(const cof::std_handle<T>& handle) const
	    {
			using internal_id_t = decltype(handle.id);

			return std::hash<internal_id_t>{}(handle.id);
	    }
    };

    template<typename T>
    struct hash<cof::lstd_handle<T>>
    {
        std::size_t operator()(const cof::lstd_handle<T>& handle) const
    	{
			using internal_id_t = decltype(handle.id);

			return std::hash<internal_id_t>{}(handle.id);
        }
    };
}
