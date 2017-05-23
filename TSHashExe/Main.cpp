#include <cstdint>
#include <array>
#include <random>
#include <iostream>

#include "TSHash.hpp"

int main()
{
	using TSHash16 = tshash::Hash<16>;
	TSHash16::ParametersType parameters;

	parameters.initial_state = { {0xBEEF} };
	parameters.polynomials[0] = { {(1 << 0) | (1 << 10) | (1 << 12) | (1 << 15)} };
	parameters.polynomials[1] = { {(1 << 0) | (1 << 2) | (1 << 9) | (1 << 12) | (1 << 13) | (1 << 14)} };

	TSHash16 hash(parameters);
	
	std::array<uint8_t, 10> arr{ 1, 2, 3 };	
	hash.update(arr.data(), arr.size());
	
	//std::cout << hash.digest() << std::endl;
	
    return 0;
}

