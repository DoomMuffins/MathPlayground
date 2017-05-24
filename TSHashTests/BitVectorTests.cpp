#include <limits>
#include "Catch/catch.hpp"
#include "TSHash.hpp"

using namespace tshash;

TEST_CASE("Bit vector bit_scan_forward", "[bitvector]")
{
	SECTION("Single word bit vector BSF")
	{
		const BIT_VECTOR<64> vec{ { 0b0000'0110'0000 } };
		CHECK(bit_scan_forward(vec) == 5);

		const BIT_VECTOR<64> vec2{ { 0b1000'0000'0000'0000 } };
		CHECK(bit_scan_forward(vec2) == 15);
	}
	SECTION("Two word bit vector BSF")
	{
		const BIT_VECTOR<128> vec_lower{ { 0b1000'0000, std::numeric_limits<uint64_t>::max() } };
		CHECK(bit_scan_forward(vec_lower) == 7);

		const BIT_VECTOR<128> vec_upper{ { 0, 0b0010'0000 } };
		CHECK(bit_scan_forward(vec_upper) == 64 + 5);
	}
	SECTION("Multi word bit vector BSF")
	{
		const BIT_VECTOR<64 * 5> vec_middle{ { 0, 0, 0b0110'0000'0000, std::numeric_limits<uint64_t>::max(), 0 } };
		CHECK(bit_scan_forward(vec_middle) == 128 + 9);

		const BIT_VECTOR<64 * 5> vec_top{ { 0, 0, 0, 0, 0b0001'0000 } };
		CHECK(bit_scan_forward(vec_top) == 4 * 64 + 4);
	}
}

TEST_CASE("Bit vector equality operators", "[bitvector]")
{
	SECTION("Single word bit vector equality operators")
	{
		const BIT_VECTOR<64> lhs{ { 0xBAADF00DD00FDAAB } };
		BIT_VECTOR<64> rhs{ { 0xBAADF00DD00FDAAB } };

		CHECK(lhs == rhs);
		rhs.data[0] ^= 1;
		CHECK(lhs != rhs);
	}
	SECTION("Multi word bit vector equality operators")
	{
		const BIT_VECTOR<128> lhs{ { 0xBAADF00DD00FDAAB, 123456 } };
		BIT_VECTOR<128> rhs{ { 0xBAADF00DD00FDAAB, 123456 } };

		CHECK(lhs == rhs);

		rhs.data[0] ^= 1;
		CHECK(lhs != rhs);

		rhs.data[0] ^= 1;
		rhs.data[1] ^= 1;
		CHECK(lhs != rhs);
	}
}

TEST_CASE("Bit vector right shift operator", "[bitvector]")
{
	SECTION("Single word bit vector right shift operator")
	{
		BIT_VECTOR<64> vec1{ { 0xBAADF00DD00FDAAB } };
		vec1 >>= 3;
		const BIT_VECTOR<64> vec2{ { 0xBAADF00DD00FDAAB >> 3 } };

		CHECK(vec1 == vec2);
	}
	SECTION("Multi word bit vector right shift operator")
	{
		BIT_VECTOR<128> vec1{ { 0x0000'FFFF'0000'FFFF, 0x0000'FFFF'0000'FFFF } };
		vec1 >>= 16;
		const BIT_VECTOR<128> vec2{ { 0xFFFF'0000'FFFF'0000, 0x0000'0000'FFFF'0000 } };

		CHECK(vec1 == vec2);
	}
	SECTION("Multi word bit vector right shift operator, shift bigger than word size")
	{
		BIT_VECTOR<64 * 3> vec1{ { 0x0000'FFFF'0000'FFFF, 0x0000'FFFF'0000'FFFF, 0x0000'FFFF'0000'FFFF } };
		vec1 >>= 64 + 16;
		const BIT_VECTOR<64 * 3> vec2{ { 0xFFFF'0000'FFFF'0000, 0x0000'0000'FFFF'0000, 0 } };

		CHECK(vec1 == vec2);
	}
}

TEST_CASE("Bit vector xor operator", "[bitvector]")
{
	SECTION("Single word bit vector xor operator")
	{
		BIT_VECTOR<64> vec1{ { 0x0000'FFFF'0000'FFFF } };
		const BIT_VECTOR<64> vec2{ { 0x1234'4321'1234'4321 } };
		const BIT_VECTOR<64> expected{ { 0x0000'FFFF'0000'FFFF ^ 0x1234'4321'1234'4321 } };

		vec1 ^= vec2;
		CHECK(vec1 == expected);
	}
	SECTION("Multi word bit vector xor operator")
	{
		BIT_VECTOR<128> vec1{ { 0x0000'FFFF'0000'FFFF, 0x0000'FFFF'0000'FFFF } };
		const BIT_VECTOR<128> vec2{ { 0x1234'4321'1234'4321, 0x1234'4321'1234'4321 } };
		const BIT_VECTOR<128> expected{ { 0x0000'FFFF'0000'FFFF ^ 0x1234'4321'1234'4321, 0x0000'FFFF'0000'FFFF ^ 0x1234'4321'1234'4321 } };

		vec1 ^= vec2;
		CHECK(vec1 == expected);
	}
}

TEST_CASE("Bit vector polynomial creation", "[bitvector]")
{
	SECTION("Single word polynomial")
	{
		const auto polynomial = create_polynomial<64>({ 1, 2, 3 });
		decltype(polynomial) expected{ {0xF000'0000'0000'0000} };
		CHECK(polynomial == expected);
	}
	SECTION("Single word polynomial, highest term degree")
	{
		const auto polynomial = create_polynomial<64>({ 63 });
		decltype(polynomial) expected{ { 0x8000'0000'0000'0001 } };
		CHECK(polynomial == expected);
	}
	SECTION("Multi word polynomial")
	{
		const auto polynomial = create_polynomial<128>({ 1, 2, 3 });
		decltype(polynomial) expected{ { 0, 0xF000'0000'0000'0000 } };
		CHECK(polynomial == expected);
	}
	SECTION("Multi word polynomial, highest term degree")
	{
		const auto polynomial = create_polynomial<128>({ 127 });
		decltype(polynomial) expected{ { 1, 0x8000'0000'0000'0000 } };
		CHECK(polynomial == expected);
	}
}