#include <cstdint>
#include <array>
#include <random>
#include <iostream>
#include <memory>

#include "TSHash.hpp"

int main()
{
	using TSHash16 = tshash::Hash<16>;
	const TSHash16::ParametersType parameters{
		{{0xBEEF}},
		{{
			tshash::create_polynomial<16>({ 15, 12, 10 }),
			tshash::create_polynomial<16>({ 14, 13, 12, 9, 2 }),
		}}
	};
		
	std::random_device rd;
	auto gen = std::make_unique<std::mt19937>(rd());
	std::uniform_int_distribution<uint32_t> dist(std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());

	auto histogram = std::make_unique<uint32_t[]>(1 << 16);
	TSHash16 hash(parameters);

	for (size_t i = 0; i < 10'000'000; ++i)
	{
		const uint32_t random_buffer = dist(*gen);

		hash.reset();
		hash.update(reinterpret_cast<const uint8_t*>(&random_buffer), sizeof(random_buffer));
		const auto digest = hash.digest();

		if (digest.data[0] >= 1 << 16)
		{
			std::cout << "Weird digest!" << std::endl;
			char c; std::cin >> c;
		}

		histogram[digest.data[0]] += 1;
	}

	//std::cout << hash.digest() << std::endl;
	
    return 0;
}

