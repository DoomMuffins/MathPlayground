#include <cstdint>
#include <array>
#include <random>
#include <iostream>
#include <fstream>
#include <memory>
#include <bitset>

#include "TSHash.hpp"
#include <Utils.hpp>

int main()
{
	constexpr size_t Bits = 16;

	using TSHash16 = tshash::Hash<Bits>;
	const TSHash16::ParametersType parameters{
		{{0xBEEF}},
		{{
			TSHash16::create_polynomial({ 16, 12, 4 }),
			TSHash16::create_polynomial({ 13, 8, 6, 5, 2 }),
		}}
	};
	
	auto cyclic_group_poly0 = get_cyclic_group_for_polynomial(parameters.polynomials[0]);
	auto cyclic_group_poly1 = get_cyclic_group_for_polynomial(parameters.polynomials[1]);

	std::random_device rd;
	auto gen = std::make_unique<std::mt19937>(rd());
	std::uniform_int_distribution<uint32_t> dist(std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());
	
	auto digest_to_src = std::make_unique<std::array<std::vector<uint32_t>, 1 << (Bits - 1)>>();
	TSHash16 hash(parameters);

	for (size_t i = 0; i < 10'000'000; ++i)
	{
		const uint32_t random_buffer = dist(*gen);

		hash.reset();
		hash.update_bytecount(reinterpret_cast<const uint8_t*>(&random_buffer), 4);
		const auto digest_vector = hash.digest();
		const auto digest = digest_vector.data[0] >> 1;

		if (digest >= 1 << (Bits - 1))
		{
			std::cout << "Weird digest!" << std::endl;
			char c; std::cin >> c;
		}

		(*digest_to_src)[digest].push_back(random_buffer);
	}

	auto nonzero_count = std::count_if(digest_to_src->cbegin(), digest_to_src->cend(), [](auto x) {return x.size() != 0;});
	std::cout << nonzero_count << std::endl;
	
	std::vector<uint64_t> digest_sizes{};
	for (uint64_t digest = 0; digest < digest_to_src->size(); ++digest)
	{
		digest_sizes.push_back((*digest_to_src)[digest].size());
	}
	
	std::ofstream of("Z:\\temp\\MathPlayground.txt");
	of << "[";
	for (auto digest : digest_sizes)
	{
		of << digest << ", ";
	}
	of << "]";

    return 0;
}

