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

	for (Entity& entity : entities) {
		if (entity == Entity{ "Cat", {"Animal", "Lazy"} }) {
			std::cout << "I know this one!\n";
		} else {
			std::cout << "Unknown entity detected! named: " << entity.name << '\n';
		}
	}

	Entity& dog = entities[dogHandle];
	std::cout << "I'm going to play fetch with: " << dog.name << '\n';

	entities.erase(dogHandle);
	assert(entities.size() == 1);
	entities.erase(catHandle);
	assert(entities.empty());

	return 0;
}
