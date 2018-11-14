#include <vector>
#include "TSHash.hpp"

template<size_t Bits>
auto get_cyclic_group_for_polynomial(const tshash::BIT_VECTOR<Bits>& polynomial)
{
	using BitVectorType = std::remove_const_t<std::remove_reference_t<decltype(polynomial)>>;
	std::vector<BitVectorType> cyclic_group{};

	const auto init = tshash::create_polynomial<Bits>({});
	auto lfsr = init;
	BitVectorType mask{};
	do
	{
		cyclic_group.push_back(lfsr);
		const uint64_t lsb = lfsr.data[0] & 1;

		for (auto& submask : mask.data)
		{
			__pragma(warning(suppress:4146)) submask = (-lsb);
		}

		lfsr >>= 1;
		lfsr ^= mask & polynomial;
	} while (lfsr != init);

	return std::move(cyclic_group);
}

template<size_t Bits>
auto bitvector_to_bytearray(const tshash::BIT_VECTOR<Bits>& vec)
{	
	constexpr const auto total_words = (Bits + 63) / 64;
	constexpr const auto bytes_in_word = sizeof(vec.data[0]);
	constexpr const auto total_bytes_before_last_word = bytes_in_word * (total_words - 1);
	constexpr const auto bits_in_last_word = Bits % 64;
	constexpr const auto bytes_in_last_word = (bits_in_last_word + 8 - 1) / 8;

	std::array<uint8_t, total_bytes_before_last_word + bytes_in_last_word> result{};

	for (size_t i = 0; i < vec.data.size(); ++i)
	{
		auto current_word = vec.data[i];
		const size_t bytes_in_current_word = (i < vec.data.size() - 1) ? bytes_in_word : bytes_in_last_word;

		for (size_t j = 0; j < bytes_in_current_word; ++j)
		{
			result[bytes_in_word * i + j] = static_cast<uint8_t>(current_word & 0xFF);
			current_word >>= 8;
		}
	}

	return result;
}

template<size_t Bits>
struct COLLISION
{
	tshash::BIT_VECTOR<Bits> first;
	tshash::BIT_VECTOR<Bits> second;
};

template<size_t StartBits, size_t HashBits, class = std::enable_if_t<(StartBits > HashBits)>>
COLLISION<HashBits> find_collision_using_brent_cycle_detection(
	const tshash::BIT_VECTOR<StartBits>& start_vector, 
	const typename tshash::Hash<HashBits>::ParametersType parameters
)
{
	using HashType = tshash::Hash<HashBits>;

	size_t power = 1;
	size_t lambda = 1;


	auto tortoise = HashType::compute_bitcount(bitvector_to_bytearray(start_vector));
	auto hare = HashType::compute_bitcount(bitvector_to_bytearray(tortoise));

	while (tortoise != hare)
	{
		if (power == lambda)
		{
			tortoise = hare;
			power *= 2;
			lambda = 0;
		}

		hare = HashType::compute_bitcount(bitvector_to_bytearray(hare));
		lambda += 1;
	}

	size_t mu = 0;
	tortoise = 

	return{};
}
