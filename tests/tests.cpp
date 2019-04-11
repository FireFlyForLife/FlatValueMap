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
};

TEST_CASE("Basic sparse_to_dense_vector things")
{
	return;
	sparse_to_dense_vector<Entity> entity_vector{};

	REQUIRE(entity_vector.empty());

	auto dogHandle = entity_vector.push_back(Entity{ "Dog", {"Animal", "Good boi"} });
	auto catHandle = entity_vector.push_back(Entity{ "Cat", {"Animal", "Lazy"} });

	REQUIRE(entity_vector.size() == 2);

	entity_vector.erase(dogHandle);

	REQUIRE(entity_vector.size() == 1);

	CHECK(entity_vector[catHandle].name == "Cat");
	CHECK(entity_vector[catHandle].tags == std::vector<std::string>{"Animal", "Lazy"});
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
