#pragma once

#include <cstdint>
#include <iostream>


namespace tshash {

struct VECTOR
{
	uint64_t low_part;
	uint64_t high_part;
};

VECTOR& operator >>= (VECTOR& v, uint32_t shift);
VECTOR operator >> (const VECTOR& v, uint32_t shift);
VECTOR& operator ^= (VECTOR& lhs, const VECTOR& rhs);
VECTOR operator ^ (const VECTOR& lhs, const VECTOR& rhs);
std::ostream& operator << (std::ostream& os, const VECTOR& v);

struct PARAMETERS
{
	VECTOR initial_state;
	VECTOR polynomials[2];
};

class Hash
{
public:
	constexpr explicit Hash(const PARAMETERS& parameters);
	Hash(const PARAMETERS& parameters, uint8_t* data, size_t size);
	void update(uint8_t* data, size_t size);
	VECTOR digest() { return m_state; }

private:
	void _update_bit(size_t bit);

	PARAMETERS m_parameters;
	VECTOR m_state;
};

}