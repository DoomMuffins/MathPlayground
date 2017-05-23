#include "catch.hpp"
#include "TSHash.hpp"

using namespace tshash;

TEST_CASE("Single word TSHash", "[tshash]")
{
	using Hash64 = Hash<64>;

	Hash64::ParametersType parameters = {
		{{0xFFFF'0000'FFFF'0000}},
		{{
			create_polynomial<64>({63}),
			create_polynomial<64>({62})
		}}
	};
	
	Hash64 hash(parameters);
	std::array<uint8_t, 1> buffer{ 0b1010'1010 };
	hash.update(buffer.data(), buffer.size());

	auto digest = hash.digest();
	decltype(digest) expected{ { 0xFE80007FFF80007D } };
	REQUIRE(digest == expected);
}