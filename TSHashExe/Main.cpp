#include <cstdint>
#include <array>
#include <random>
#include <iostream>
#include <fstream>
#include <memory>
#include <bitset>
#include <chrono>
#include <algorithm>
#include <numeric>

#include "TSHash.hpp"
#include "Utils.hpp"


class Timer
{
public:
	using clock = typename std::chrono::high_resolution_clock;
	using time_point = typename clock::time_point;
	using duration = typename clock::duration;

	Timer() { reset(); }
	void reset() { m_start = clock::now(); }
	duration elapsed() { return m_start - clock::now(); }

private:
	time_point m_start;
};


template<class THash>
void run_benchmark(THash hash, size_t iterations)
{
	Timer timer;
	using duration = decltype(timer)::duration;

	std::vector<duration> durations;
	durations.reserve(iterations);

	for (size_t i = 0; i < iterations; ++i)
	{
		hash.reset();
		timer.reset();

		durations.push_back(timer.elapsed());
	}

	auto [minimum, maximum] = std::minmax_element(durations.cbegin(), durations.cend());
	auto average = std::accumulate(durations.cbegin(), durations.cend(), duration::zero());
}

int old_stuff()
{
	constexpr size_t Bits = 16;

	using TSHash16 = tshash::Hash<Bits>;
	const TSHash16::ParametersType parameters{
		{ { 0xBEEF } },
		{ {
			TSHash16::create_polynomial({ 16, 11, 4 }),
			TSHash16::create_polynomial({ 10, 8, 7, 4, 1 }),
		} }
	};

	uint8_t a = 0b10101010;
	auto dig = TSHash16::compute_bytecount(parameters, &a, 1);

	auto res = bitvector_to_bytearray(parameters.initial_state);

	auto cyclic_group_poly0 = get_cyclic_group_for_polynomial(parameters.polynomials[0]);
	auto cyclic_group_poly1 = get_cyclic_group_for_polynomial(parameters.polynomials[1]);

	std::random_device rd;
	auto gen = std::make_unique<std::mt19937>(rd());
	std::uniform_int_distribution<uint32_t> dist(std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());

	auto digest_to_src = std::make_unique<std::array<std::vector<uint32_t>, (1 << Bits)>>();
	TSHash16 hash(parameters);

	for (size_t i = 0; i < 10'000'000; ++i)
	{
		const uint32_t random_buffer = dist(*gen);

		hash.reset();
		hash.update_bytecount(reinterpret_cast<const uint8_t*>(&random_buffer), 4);
		const auto digest_vector = hash.digest();
		const auto digest = digest_vector.data[0];

		if (digest >= (1 << Bits))
		{
			std::cout << "Weird digest!" << std::endl;
			char c; std::cin >> c;
		}

		(*digest_to_src)[digest].push_back(random_buffer);
	}

	auto nonzero_count = std::count_if(digest_to_src->cbegin(), digest_to_src->cend(), [](auto x) {return x.size() != 0; });
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

int main()
{
	using Hash256 = tshash::Hash<256 - 2>;
	Hash256 hash{ {} };

	run_benchmark(hash, 100000);
}

