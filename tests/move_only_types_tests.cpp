#include <catch2/catch.hpp>
#include <memory>
#include <string>
#include <type_traits>

#include "flat_value_map_handle.h"
#include "flat_value_map.h"
#include "light_flat_value_map.h"


struct Entity
{
	int health = 100;
	std::string name = "";


	friend bool operator==(const Entity& lhs, const Entity& rhs)
	{
		return lhs.health == rhs.health
			&& lhs.name == rhs.name;
	}

	friend bool operator!=(const Entity& lhs, const Entity& rhs) { return !(lhs == rhs); }
};

struct OnlyMoveable
{
	std::unique_ptr<Entity> entityPtr;

	OnlyMoveable(const OnlyMoveable& other) = delete;
	OnlyMoveable(OnlyMoveable&& other) noexcept = default;
	OnlyMoveable& operator=(const OnlyMoveable& other) = delete;
	OnlyMoveable& operator=(OnlyMoveable&& other) noexcept = default;
	~OnlyMoveable() = default;
};
static_assert(!std::is_copy_constructible_v<OnlyMoveable>&& std::is_move_constructible_v<OnlyMoveable>, "The type should be move only");

using OnlyMoveableHandle = cof::FvmHandle<OnlyMoveable>;
using VecOnlyMoveHandle = cof::FvmHandle<OnlyMoveable>;


TEST_CASE("FlatValueMap with move-only type")
{
	cof::FlatValueMap<OnlyMoveableHandle, OnlyMoveable> fvm{};

	REQUIRE(fvm.empty());

	auto dogHandle = fvm.push_back(OnlyMoveable{ std::unique_ptr<Entity>{new Entity{ 50, "Yeet"} } });
	auto catHandle = fvm.push_back(OnlyMoveable{ std::unique_ptr<Entity>{new Entity{ 100, "Animal"} } });

	REQUIRE(fvm.size() == 2);

	fvm.erase(dogHandle);

	REQUIRE(fvm.size() == 1);

	CHECK((*fvm[catHandle].entityPtr == Entity{ 100, "Animal" }));

	fvm.erase(catHandle);

	CHECK(fvm.empty());
}

TEST_CASE("LightFlatValueMap with move-only type")
{
	cof::LightFlatValueMap<OnlyMoveableHandle, OnlyMoveable> fvm{};

	REQUIRE(fvm.empty());

	auto dogHandle = fvm.push_back(OnlyMoveable{ std::unique_ptr<Entity>{new Entity{ 50, "Yeet"} } });
	auto catHandle = fvm.push_back(OnlyMoveable{ std::unique_ptr<Entity>{new Entity{ 100, "Animal"} } });

	REQUIRE(fvm.size() == 2);

	fvm.erase(dogHandle);

	REQUIRE(fvm.size() == 1);

	CHECK((*fvm[catHandle].entityPtr == Entity{ 100, "Animal" }));

	fvm.erase(catHandle);

	CHECK(fvm.empty());
}


TEST_CASE("FlatValueMap with move-only vector type")
{
	cof::FlatValueMap<VecOnlyMoveHandle, std::vector<OnlyMoveable>> fvm{};

	REQUIRE(fvm.empty());

	std::vector<OnlyMoveable> listOfYeet{ };
	listOfYeet.emplace_back(std::move(OnlyMoveable{ std::unique_ptr<Entity>{new Entity{ 50, "Yeet" } } }));
	std::vector<OnlyMoveable> listOfAnimals{ };
	listOfAnimals.emplace_back(std::move(OnlyMoveable{ std::unique_ptr<Entity>{new Entity{ 100, "Animal" } } }));
	auto dogHandle = fvm.push_back(std::move(listOfYeet));
	auto catHandle = fvm.push_back(std::move(listOfAnimals));

	REQUIRE(fvm.size() == 2);
	cof::FlatValueMap<VecOnlyMoveHandle, std::vector<OnlyMoveable>> fvm2 = std::move(fvm);
	REQUIRE(fvm2.size() == 2);

	fvm2.erase(dogHandle);

	REQUIRE(fvm2.size() == 1);

	CHECK((*fvm2[catHandle].at(0).entityPtr == Entity{ 100, "Animal" }));

	fvm2.erase(catHandle);

	CHECK(fvm2.empty());
}

TEST_CASE("LightFlatValueMap with move-only vector type")
{
	cof::LightFlatValueMap<VecOnlyMoveHandle, std::vector<OnlyMoveable>> fvm{};

	REQUIRE(fvm.empty());

	std::vector<OnlyMoveable> listOfYeet{ };
	listOfYeet.emplace_back(std::move(OnlyMoveable{ std::unique_ptr<Entity>{new Entity{ 50, "Yeet" } } }));
	std::vector<OnlyMoveable> listOfAnimals{ };
	listOfAnimals.emplace_back(std::move(OnlyMoveable{ std::unique_ptr<Entity>{new Entity{ 100, "Animal" } } }));
	auto dogHandle = fvm.push_back(std::move(listOfYeet));
	auto catHandle = fvm.push_back(std::move(listOfAnimals));

	REQUIRE(fvm.size() == 2);
	cof::LightFlatValueMap<VecOnlyMoveHandle, std::vector<OnlyMoveable>> fvm2 = std::move(fvm);
	REQUIRE(fvm2.size() == 2);

	fvm2.erase(dogHandle);

	REQUIRE(fvm2.size() == 1);

	CHECK((*fvm2[catHandle].at(0).entityPtr == Entity{ 100, "Animal" }));

	fvm2.erase(catHandle);

	CHECK(fvm2.empty());
}