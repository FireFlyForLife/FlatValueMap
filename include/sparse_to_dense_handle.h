#pragma once
#include <functional>

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
