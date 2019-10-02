#pragma once
#include <functional>

namespace cof
{
	/// Handle for a `cof::FlatValueMap<T>`
	/// A utility class for creating a typesafe handle.
	template<typename T>
	struct FvmHandle {
		uint32_t id;

		friend bool operator==(const FvmHandle& lhs, const FvmHandle& rhs) { return lhs.id == rhs.id; }
		friend bool operator!=(const FvmHandle& lhs, const FvmHandle& rhs) { return lhs.id != rhs.id; }
		friend bool operator<(const FvmHandle& lhs, const FvmHandle& rhs) { return lhs.id < rhs.id; }
		friend bool operator<=(const FvmHandle& lhs, const FvmHandle& rhs) { return lhs.id <= rhs.id; }
		friend bool operator>(const FvmHandle& lhs, const FvmHandle& rhs) { return lhs.id > rhs.id; }
		friend bool operator>=(const FvmHandle& lhs, const FvmHandle& rhs) { return lhs.id >= rhs.id; }
	};

	/// Handle for a `cof::LightFlatValueMap<T>`
	/// A utility class for creating a typesafe handle.
	template<typename T>
	struct LfvmHandle {
		uint32_t id;

		friend bool operator==(const LfvmHandle& lhs, const LfvmHandle& rhs) { return lhs.id == rhs.id; }
		friend bool operator!=(const LfvmHandle& lhs, const LfvmHandle& rhs) { return lhs.id != rhs.id; }
		friend bool operator<(const LfvmHandle& lhs, const LfvmHandle& rhs) { return lhs.id < rhs.id; }
		friend bool operator<=(const LfvmHandle& lhs, const LfvmHandle& rhs) { return lhs.id <= rhs.id; }
		friend bool operator>(const LfvmHandle& lhs, const LfvmHandle& rhs) { return lhs.id > rhs.id; }
		friend bool operator>=(const LfvmHandle& lhs, const LfvmHandle& rhs) { return lhs.id >= rhs.id; }
	};
}

namespace std
{
	template<typename T>
	struct hash<cof::FvmHandle<T>>
	{
		std::size_t operator()(const cof::FvmHandle<T>& handle) const
		{
			using internal_id_t = decltype(handle.id);

			return std::hash<internal_id_t>{}(handle.id);
		}
	};

	template<typename T>
	struct hash<cof::LfvmHandle<T>>
	{
		std::size_t operator()(const cof::LfvmHandle<T>& handle) const
		{
			using internal_id_t = decltype(handle.id);

			return std::hash<internal_id_t>{}(handle.id);
		}
	};
}
