#include <cstdint>
#include <array>
#include <iostream>

#include "TSHash.hpp"

int main()
{
	using TSHash48 = tshash::Hash<48>;
	TSHash48::ParametersType parameters{ {1}, {{1}} };
	TSHash48 hash(parameters);
	
	std::array<uint8_t, 10> arr{ 1, 2, 3 };	
	hash.update(arr.data(), arr.size());
	
	//std::cout << hash.digest() << std::endl;
	
    return 0;
}

