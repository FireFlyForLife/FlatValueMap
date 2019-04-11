#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include <iostream>


void WaitForEnterToContinue()
{
	std::cout << "To press continue, press enter...";
	std::cin.get();
}

int main(int argc, char* argv[]) {
	// global setup...

	int result = Catch::Session().run(argc, argv);

	// global clean-up...

	return result;
}