#include <limits>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "TSHash.hpp"

TEST_CASE("Bit vector bit_scan_forward", "[bitvector]")
{
	using namespace tshash;

	SECTION("Single word bit vector BSF")
	{
		BIT_VECTOR<64> vec{ {0b0000'0110'0000} };
		CHECK(bit_scan_forward(vec) == 5);

		BIT_VECTOR<64> vec2{ {0b1000'0000'0000'0000} };
		CHECK(bit_scan_forward(vec2) == 15);
	}
	SECTION("Two word bit vector BSF")
	{
		BIT_VECTOR<128> vec_lower{ {0b1000'0000, std::numeric_limits<uint64_t>::max()} };
		CHECK(bit_scan_forward(vec_lower) == 7);

		BIT_VECTOR<128> vec_upper{ {0, 0b0010'0000} };
		CHECK(bit_scan_forward(vec_upper) == 64 + 5);
	}
	SECTION("Multi word bit vector BSF")
	{
		BIT_VECTOR<64 * 5> vec_middle{ {0, 0, 0b0110'0000'0000, std::numeric_limits<uint64_t>::max(), 0} };
		CHECK(bit_scan_forward(vec_middle) == 128 + 9);

		BIT_VECTOR<64 * 5> vec_top{ {0, 0, 0, 0, 0b0001'0000} };
		CHECK(bit_scan_forward(vec_top) == 4 * 64 + 4);
	}
}