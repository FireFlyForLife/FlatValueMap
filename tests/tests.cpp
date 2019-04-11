#include <catch2/catch.hpp>

#include <string>

#include <sparse_to_dense_vector.h>
#include <light_sparse_to_dense_vector.h>


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

TEST_CASE("Basic sparse_to_dense_vector things")
{
	sparse_to_dense_vector<Entity> entity_vector{};

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

TEST_CASE("sparse_to_dense_vector erase by iterator")
{
	sparse_to_dense_vector<Entity> entity_vector{};
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

//TODO: Enable this test when erase(first,last) function is fixed
TEST_CASE("sparse_to_dense_vector erase by iterator range")	
{
	return;

	sparse_to_dense_vector<Entity> entity_vector{};
	auto dogHandle = entity_vector.push_back(Entity{ "Dog", {"Animal", "Good boi"} });
	auto catHandle = entity_vector.push_back(Entity{ "Cat", {"Animal", "Lazy"} });
	auto alienHandle = entity_vector.push_back(Entity{ "Alien", {"NonHuman"} });

	auto it = entity_vector.begin();
	it = std::next(it);

	entity_vector.erase(it, entity_vector.end());

	REQUIRE(entity_vector.size() == 1);
	CHECK(entity_vector[dogHandle] == Entity{ "Dog", {"Animal", "Good boi"} });
}

TEST_CASE("Basic light_sparse_to_dense_vector things")
{
	light_sparse_to_dense_vector<Entity> entity_vector{};

	REQUIRE(entity_vector.empty());

	auto dogHandle = entity_vector.push_back(Entity{ "Dog", {"Animal", "Good boi"} });
	auto catHandle = entity_vector.push_back(Entity{ "Cat", {"Animal", "Lazy"} });

	REQUIRE(entity_vector.size() == 2);

	entity_vector.erase(dogHandle);

	REQUIRE(entity_vector.size() == 1);

	CHECK(entity_vector[catHandle].name == "Cat");
	CHECK(entity_vector[catHandle].tags == std::vector<std::string>{"Animal", "Lazy"});
}
