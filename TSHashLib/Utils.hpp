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
