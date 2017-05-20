#include <cstdint>
#include <array>
#include <iostream>

#include "TSHash.hpp"

int main()
{
	tshash::PARAMETERS parameters{ {1, 1}, {{1, 1}, {1, 1}} };
	tshash::Hash hash(parameters);
	
	std::array<uint8_t, 10> arr{ 1, 2, 3 };	
	hash.update(arr.data(), arr.size());
	
	std::cout << hash.digest() << std::endl;
	
    return 0;
}

