#include "../include/sparse_to_dense_vector.h"
#include <string>
#include <iostream>

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

void WaitForEnterToContinue()
{
	std::cout << "To press continue, press enter...";
	std::cin.get();
}

int main() {
	sparse_to_dense_vector<Entity> entity_vector{};

	entity_vector.push_back(Entity{ "Dog", {"Animal", "Good boi"} });
	entity_vector.push_back(Entity{ "Cat", {"Animal", "Lazy"} });

	WaitForEnterToContinue();
	return 0;
}