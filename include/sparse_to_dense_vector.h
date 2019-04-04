#include <vector>
#include <unordered_map>
#include <cstdint>

namespace cof
{
    //Forward declarations
    template<typename T, typename Allocator = std::allocator<T>, typename SparseToDenseAllocator = Allocator, typename DenseToSparseAllocator = Allocator>
    class sparse_to_dense_vector;

    template<typename T, typename Allocator = std::allocator<T>, typename SparseToDenseAllocator = Allocator, typename DenseToSparseAllocator = Allocator>
    class light_sparse_to_dense_vector;
}

namespace std
{
    //std specialisations forward declarations
    template<typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
    struct hash<cof::sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::handle>;

    template<typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
    struct hash<cof::sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::handle>;
}

namespace cof
{
    /// A sparse to dense vector. has a handle to index unordered_map for lookup, and index to handle unordered_map for quick erasing using erase remove idiom.
    template<typename T, typename Allocator = std::allocator<T>, typename SparseToDenseAllocator = Allocator, typename DenseToSparseAllocator = Allocator>
    class sparse_to_dense_vector
    {
    public:
        struct handle{
            uint32_t id;
        };

    private:
        using SparseToDenseMap = std::unordered_map<handle, std::size_t, std::hash<handle>, std::equal_to<handle>, SparseToDenseAllocator>;
        using DenseToSparseMap = std::unordered_map<std::size_t, SparseToDenseMap::iterator, std::hash<std::size_t>, std::equal_to<std::size_t>, DenseToSparseAllocator>;

        SparseToDenseMap sparse_to_dense{};
        DenseToSparseMap dense_to_sparse{};
        std::vector<T, Allocator> dense_vector;

        static uint32_t internalIdCounter = 0;

    public:
        sparse_to_dense_vector() = default;
        
        handle push_back(const T& t) {
            std::size_t element_index = dense_vector.size();
            uint32_t element_id = ++internalIdCounter;
            dense_vector.push_back(t);
            sparse_to_dense.emplace(handle{element_id}, element_index);
            auto it = sparse_to_dense.find(handle{element_id});
            dense_to_sparse.emplace(element_index, it);

            return handle{element_id};
        }
        handle push_back(T&& t) {
            std::size_t element_index = dense_vector.size();
            uint32_t element_id = ++internalIdCounter;
            dense_vector.push_back(t);
            sparse_to_dense.emplace(element_id, element_index);
            auto it = sparse_to_dense.find(handle{element_id});
            dense_to_sparse.emplace(element_index, it);

            return handle{element_id};
        }
        void erase(handle handleToDelete) {
            // ...
        }
    };

    /// A sparse to dense vector. Does not include a index to vector map, so erasing elements is slower,
    /// but memmory usage is less, and insertion time is slightly better.
    template<typename T, typename Allocator = std::allocator<T>, typename SparseToDenseAllocator = Allocator, typename DenseToSparseAllocator = Allocator>
    class light_sparse_to_dense_vector
    {
    public:
        struct handle{
            uint32_t id;
        };

    private:
        std::unordered_map<handle, std::size_t, std::hash<handle>, std::equal_to<handle>, Allocator> sparse_to_dense{};
        std::vector<T, Allocator> dense_vector;

        static uint32_t internalIdCounter = 0;

    public:
        sparse_to_dense_vector() = default;
        
        handle push_back(const T& t) {
            std::size_t element_index = dense_vector.size();
            uint32_t element_id = ++internalIdCounter;
            dense_vector.push_back(t);
            sparse_to_dense.emplace(handle{element_id}, element_index);

            return handle{element_id};
        }
        handle push_back(T&& t) {
            std::size_t element_index = dense_vector.size();
            uint32_t element_id = ++internalIdCounter;
            dense_vector.push_back(t);
            sparse_to_dense.emplace(handle{element_id}, element_index);

            return handle{element_id};
        }

    };

#if __cplusplus > 201703L
    namespace pmr {
        template<typename T, typename Allocator = std::pmr::polymorphic_allocator<T>>
        using sparse_to_dense_vector = mylib::sparse_to_dense_vector<T, Allocator>;

        template<typename T, typename Allocator = std::pmr::polymorphic_allocator<T>>
        using light_sparse_to_dense_vector = mylib::light_sparse_to_dense_vector<T, Allocator>;
    }
#endif
}

namespace std 
{
    template<typename T, typename Allocator, typename SparseToDenseAllocator, typename DenseToSparseAllocator>
    struct hash<cof::sparse_to_dense_vector<T, Allocator, SparseToDenseAllocator, DenseToSparseAllocator>::handle>
    {
        std::size_t operator()(const cof::sparse_to_dense_vector::handle& handle) {
            return handle.id;
        }
    };
}