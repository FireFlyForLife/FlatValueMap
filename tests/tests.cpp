#include <catch2/catch.hpp>

#include <string>

#include <flat_value_map_handle.h>
#include <flat_value_map.h>
#include <light_flat_value_map.h>


using namespace cof;


class Entity
{
public:
	Entity() : name(), tags{}
	{}
	Entity(std::string name, std::vector<std::string> tags) : name(std::move(name)), tags{std::move(tags)}
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


TEST_CASE("Basic FlatValueMap things")
{
	FlatValueMap<EntityHandle, Entity> entity_vector{};

	REQUIRE(entity_vector.empty());

	auto dogHandle = entity_vector.push_back(Entity{ "Dog", {"Animal", "Good boi"} });
	auto catHandle = entity_vector.push_back(Entity{ "Cat", {"Animal", "Lazy"} });

	REQUIRE(entity_vector.size() == 2);

	entity_vector.erase(dogHandle);

	REQUIRE(entity_vector.size() == 1);

	CHECK(entity_vector[catHandle] == Entity{ "Cat", {"Animal", "Lazy"} });

	entity_vector.erase(catHandle);

	CHECK(entity_vector.empty());
}

TEST_CASE("FlatValueMap erase by iterator")
{
	FlatValueMap<EntityHandle, Entity> entity_vector{};
	auto dogHandle = entity_vector.push_back(Entity{ "Dog", {"Animal", "Good boi"} });
	auto catHandle = entity_vector.push_back(Entity{ "Cat", {"Animal", "Lazy"} });
	auto maikoHandle = entity_vector.push_back(Entity{ "Maiko", {"Human", "Programmer"} });
	auto alienHandle = entity_vector.push_back(Entity{ "Alien", {"NonHuman"} });

	REQUIRE(entity_vector.size() == 4);

	auto it = entity_vector.begin();
	it = std::next(it, 2);

	REQUIRE(*it == Entity{ "Maiko", {"Human", "Programmer"} });

	entity_vector.erase(it);

	CHECK(entity_vector.size() == 3);

	CHECK(entity_vector[dogHandle] == Entity{ "Dog", {"Animal", "Good boi"} });
	CHECK(entity_vector[catHandle] == Entity{ "Cat", {"Animal", "Lazy"} });
	CHECK(entity_vector[alienHandle] == Entity{ "Alien", {"NonHuman"} });
}

TEST_CASE("FlatValueMap erase by iterator range")	
{
	{
		FlatValueMap<EntityHandle, Entity> entity_vector{};
		auto dogHandle = entity_vector.push_back(Entity{ "Dog", {"Animal", "Good boi"} });
		auto catHandle = entity_vector.push_back(Entity{ "Cat", {"Animal", "Lazy"} });
		auto alienHandle = entity_vector.push_back(Entity{ "Alien", {"NonHuman"} });

		auto it = entity_vector.begin();
		it = std::next(it);

		entity_vector.erase(it, entity_vector.end());

		REQUIRE(entity_vector.size() == 1);
		CHECK(entity_vector[dogHandle] == Entity{ "Dog", {"Animal", "Good boi"} });
	}
	{
		FlatValueMap<EntityHandle, Entity> entity_vector{};
		auto dogHandle = entity_vector.push_back(Entity{ "Dog", {"Animal", "Good boi"} });
		auto catHandle = entity_vector.push_back(Entity{ "Cat", {"Animal", "Lazy"} });
		auto maikoHandle = entity_vector.push_back(Entity{ "Maiko", {"Human", "Programmer"} });
		auto alienHandle = entity_vector.push_back(Entity{ "Alien", {"NonHuman"} });

		entity_vector.erase(entity_vector.begin(), entity_vector.end());

		CHECK(entity_vector.empty());
	}

	{
		FlatValueMap<EntityHandle, Entity> entity_vector{};
		auto dogHandle = entity_vector.push_back(Entity{ "Dog", {"Animal", "Good boi"} });
		auto catHandle = entity_vector.push_back(Entity{ "Cat", {"Animal", "Lazy"} });
		auto maikoHandle = entity_vector.push_back(Entity{ "Maiko", {"Human", "Programmer"} });
		auto alienHandle = entity_vector.push_back(Entity{ "Alien", {"NonHuman"} });

		auto it = entity_vector.begin();
		it = std::next(it, 1);
		entity_vector.erase(it, entity_vector.end());

		CHECK(entity_vector.size() == 1);
		CHECK(entity_vector[dogHandle] == Entity{ "Dog", {"Animal", "Good boi"} });

	}
}

TEST_CASE("FlatValueMap things::find")
{
	FlatValueMap<EntityHandle, Entity> entity_vector;

	auto dogHandle = entity_vector.push_back(Entity{ "Dog", {"Animal", "Good boi"} });
	auto catHandle = entity_vector.push_back(Entity{ "Cat", {"Animal", "Lazy"} });
	auto maikoHandle = entity_vector.push_back(Entity{ "Maiko", {"Human", "Programmer"} });
	auto alienHandle = entity_vector.push_back(Entity{ "Alien", {"NonHuman"} });

	REQUIRE(entity_vector.size() == 4);

	{
		auto catIt = entity_vector.find(catHandle);
		auto maikoit = entity_vector.find(maikoHandle);
		auto alienIt = entity_vector.find(alienHandle);
		auto dogIt = entity_vector.find(dogHandle);

		CHECK(*catIt == Entity{ "Cat", {"Animal", "Lazy"} });
		CHECK(*maikoit == Entity{ "Maiko", {"Human", "Programmer"} });
		CHECK(*alienIt == Entity{ "Alien", {"NonHuman"} });
		CHECK(*dogIt == Entity{ "Dog", {"Animal", "Good boi"} });
	}

	entity_vector.erase(catHandle);
	entity_vector.erase(alienHandle);

	{
		auto catIt = entity_vector.find(catHandle);
		auto maikoit = entity_vector.find(maikoHandle);
		auto alienIt = entity_vector.find(alienHandle);
		auto dogIt = entity_vector.find(dogHandle);

		CHECK(catIt == entity_vector.end());
		CHECK(*maikoit == Entity{ "Maiko", {"Human", "Programmer"} });
		CHECK(alienIt == entity_vector.end());
		CHECK(*dogIt == Entity{ "Dog", {"Animal", "Good boi"} });
	}
}

TEST_CASE("Basic LightFlatValueMap things")
{
	LightFlatValueMap<EntityHandle, Entity> entity_vector{};

	REQUIRE(entity_vector.empty());

	auto dogHandle = entity_vector.push_back(Entity{ "Dog", {"Animal", "Good boi"} });
	auto catHandle = entity_vector.push_back(Entity{ "Cat", {"Animal", "Lazy"} });

	REQUIRE(entity_vector.size() == 2);

	entity_vector.erase(dogHandle);

	REQUIRE(entity_vector.size() == 1);

	CHECK(entity_vector[catHandle].name == "Cat");
	CHECK(entity_vector[catHandle].tags == std::vector<std::string>{"Animal", "Lazy"});
}
