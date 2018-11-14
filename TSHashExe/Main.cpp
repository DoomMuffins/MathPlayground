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
#include <random>

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
	duration elapsed() { return clock::now() - m_start; }

private:
	time_point m_start;
};

std::vector<uint8_t> generate_random_buffer(size_t size)
{
	std::random_device device;
	std::mt19937 generator(device());
	std::uniform_int_distribution<uint16_t> byte_dist(std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max());
	const auto get_byte = [&]() { return static_cast<uint8_t>(byte_dist(generator)); };

	std::vector<uint8_t> buffer(size);
	std::generate(buffer.begin(), buffer.end(), get_byte);
	return buffer;
}

template<class THash>
void run_benchmark(THash hash, size_t input_size_bits, size_t iterations)
{
	Timer timer;
	using duration = decltype(timer)::duration;

	std::vector<duration> durations;
	durations.reserve(iterations);

	const auto random_buffer = generate_random_buffer(input_size_bits / 8);

	for (size_t i = 0; i < iterations; ++i)
	{
		hash.reset();
		timer.reset();
		hash.update_bytecount(random_buffer.data(), random_buffer.size());
		durations.push_back(timer.elapsed());
	}

	const auto count_micros = [](duration d) { return std::chrono::duration_cast<std::chrono::microseconds>(d).count(); };

	auto [minimum, maximum] = std::minmax_element(durations.cbegin(), durations.cend());
	auto average = std::accumulate(durations.cbegin(), durations.cend(), duration::zero()) / durations.size();

	std::cout << "Benchmarked " << typeid(THash).name() << ":\n";
	std::cout << "\tIterations = " << iterations << "\n";
	std::cout << "\tInput size in bits = " << input_size_bits << "\n";
	std::cout << "\tResults in microseconds (min, max, avg) = (" << 
		count_micros(*minimum) << ", " << count_micros(*maximum) << ", " << count_micros(average) << ")\n";
	std::cout << std::endl;
}

template<class THash>
void run_benchmarks(THash hash)
{
	const auto iterations = 10'000u;
	const auto lengths = { 1u << 10, 1u << 15, 1u << 20 };

	for (const auto length : lengths)
	{
		run_benchmark(hash, length, iterations);
	}
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
	tshash::Hash<64 - 2> hash64(
		{ 
			{{1ULL << 63}},
			{{
				{{0xEEB971953B36F7DFULL}},
				{{0xC1F42000C9DCCC21ULL}},
			}},
		});
	tshash::Hash<128 - 2> hash128(
		{
			{{1ULL << 63, 0ULL}},
			{{
				{{0xE316D2B7A1D68538ULL, 0x91CF82D7B80CDE58ULL}},
				{{0xD262CE47A21F52EFULL, 0xB96D860AB623015CULL}},
			}},
		});
	tshash::Hash<256 - 2> hash256(
		{
			{{1ULL << 63, 0ULL, 0ULL, 0ULL}},
			{{
				{{0xCB0AA2844801B2F0ULL, 0x0E146435DD975282ULL, 0x932FF05A9609D68FULL, 0x87B1819987613907ULL}},
				{{0xA1D0FFE0CDD65BE4ULL, 0x6016745BE32ED6EDULL, 0xB569A4709E15E2C7ULL, 0xA00001191C46B14BULL}},
			}},
		});
	tshash::Hash<512 - 2> hash512(
		{
			{{ 1ULL << 63, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL }},
			{{
				{{0xD1408326329D071BULL, 0xE91EA3B7F759E195ULL, 0x3AA1E8A23EF14E24ULL, 0x7FC99FD45931E716ULL, 
				  0xCE73BC0F535C3F66ULL, 0xA1FACDC2A5CB094AULL, 0x9B87B326968100C6ULL, 0xF43DD64DCAC6FD17ULL}},
				{{0xFE06ADC46ADAD722ULL, 0x7A6A23BAEC3D6C41ULL, 0x4FF3607D57BCD5D6ULL, 0x056DECDF1FCD508CULL,
				  0x85B52D7E6D28509AULL, 0x3B9CB4DC6A974C78ULL, 0xDD5D0FEA7ECB471AULL, 0x6EC47C35B1D93F4AULL}},
			}},
		});
	
	run_benchmarks(hash64);
	run_benchmarks(hash128);
	run_benchmarks(hash256);
	run_benchmarks(hash512);

	return 0;
}

