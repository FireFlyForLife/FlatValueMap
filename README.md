# FlatValueMap

A sparse to dense container. 
This container has a constant lookup time (on average) thanks to the internal map which stores sparse handles to the dense indices.
It has O(1) iteration speed because all the values are stored in a vector.

The API is almost identical to std::map. With some extra additions to get more control over the internals.

## Code Example
```cpp
#include <flat_value_map_handle.h>
#include <flat_value_map.h>
#include <cassert>
#include <iostream>


using namespace cof;


class Entity
{
public:
	Entity() : name(), tags{}
	{}
	Entity(std::string name, std::vector<std::string> tags) : name(std::move(name)), tags{ std::move(tags) }
	{}
	virtual ~Entity() = default;

	std::string name;
	std::vector<std::string> tags{};


	friend bool operator==(const Entity& lhs, const Entity& rhs)
	{
		return lhs.name == rhs.name
			&& lhs.tags == rhs.tags;
	}
	friend bool operator!=(const Entity& lhs, const Entity& rhs) { return !(lhs == rhs); }
};

using EntityHandle = FvmHandle<Entity>;


int example_main(int argc, char* argv[])
{
	FlatValueMap<EntityHandle, Entity> entities{};

	auto dogHandle = entities.push_back(Entity{ "Dog", {"Animal", "Good boi"} });
	auto catHandle = entities.push_back(Entity{ "Cat", {"Animal", "Lazy"} });

	// O(1) iteration speed because the entities are stored in a vector.
	for (Entity& entity : entities) {
		if (entity == Entity{ "Cat", {"Animal", "Lazy"} }) {
			std::cout << "I know this one!\n";
		} else {
			std::cout << "Unknown entity detected! named: " << entity.name << '\n';
		}
	}

	// Lookup times are constant (on average) because the handles are stored next to the indices in an unordered_map
	Entity& dog = entities[dogHandle];
	std::cout << "I'm going to play fetch with: " << dog.name << '\n';

	entities.erase(dogHandle);
	assert(entities.size() == 1);
	entities.erase(catHandle);
	assert(entities.empty());

	return 0;
}
```

## Classes
There are two versions of the FlatValueMap, they both have an (almost) identical API but they have slightly different internals.

### cof::FlatValueMap
This implementation uses a `unordered_map<handle, index>` as sparse to dense map for quick lookup times and uses `unordered_map<index, SparseToDense::iterator>` as a DenseToSparse map for quick deletion times.
This causes it to use more memory but will perform more consistent.

### cof::LightFlatValueMap
This implementation only uses a `unordered_map<handle, index>` as sparse to dense map for quick lookup times and does not use any other map for the reverse lookup. This means that deletion complexity is linear because we loop over all elements in the sparse_to_dense map to find a matching key-value pair.
But the memory usage is smaller.
