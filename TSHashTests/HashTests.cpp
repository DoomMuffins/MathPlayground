#include <numeric>
#include "Catch/catch.hpp"
#include "TSHash.hpp"

using namespace tshash;

TEST_CASE("Single word TSHash", "[tshash]")
{
	using Hash64 = Hash<64>;

	Hash64::ParametersType parameters = {
		{{0xFFFF'0000'FFFF'0000, 0}},
		{{
			Hash64::create_polynomial({65}),
			Hash64::create_polynomial({64})
		}}
	};
	
	Hash64 hash(parameters);
	
	std::array<uint8_t, 1> single_byte_buffer{ 0b1010'1010 };
	Hash64::DigestType single_byte_buffer_expected{ { 0xFA00007FFF80007D } };
	
	std::array<uint8_t, 10> multi_byte_buffer{};
	std::iota(multi_byte_buffer.begin(), multi_byte_buffer.end(), 0x10 - 5);

	SECTION("Single update call gives correct result")
	{
		hash.update_bytecount(single_byte_buffer.data(), single_byte_buffer.size());
		const auto digest = hash.digest();
		REQUIRE(digest == single_byte_buffer_expected);
	}
	SECTION("Update after reset gives same result")
	{
		hash.update_bytecount(single_byte_buffer.data(), single_byte_buffer.size());
		hash.reset();
		hash.update_bytecount(single_byte_buffer.data(), single_byte_buffer.size());
		const auto digest = hash.digest();
		REQUIRE(digest == single_byte_buffer_expected);
	}
	SECTION("Chaining update calls is the same as a single update call on the concatenated buffer")
	{
		const auto middle = multi_byte_buffer.size() / 2;
		hash.update_bytecount(multi_byte_buffer.data(), middle);
		hash.update_bytecount(multi_byte_buffer.data() + middle, multi_byte_buffer.size() - middle);
		const auto multicall_digest = hash.digest();

		hash.reset();

		hash.update_bytecount(multi_byte_buffer.data(), multi_byte_buffer.size());
		const auto singlecall_digest = hash.digest();

		REQUIRE(multicall_digest == singlecall_digest);
	}
	SECTION("Bitcount not divisible by CHAR_BIT")
	{
		std::array<uint8_t, 1> bottom_nibble_buffer{ static_cast<uint8_t>(single_byte_buffer[0] & 0x0F) };
		std::array<uint8_t, 1> top_nibble_buffer{ static_cast<uint8_t>((single_byte_buffer[0] & 0xF0) >> 4) };

		hash.update_bitcount(bottom_nibble_buffer.data(), 4);
		hash.update_bitcount(top_nibble_buffer.data(), 4);
		const auto digest = hash.digest();
		REQUIRE(digest == single_byte_buffer_expected);
	}
}
