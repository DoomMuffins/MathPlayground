#pragma once

#include <intrin.h>
#include <iomanip>
#include "TSHash.hpp"

namespace tshash {

uint32_t bit_scan_forward(const VECTOR& v)
{
	unsigned long shift_amount;
	if (!_BitScanForward64(&shift_amount, v.low_part))
	{
		_BitScanForward64(&shift_amount, v.high_part);
		shift_amount += 64;
	}
	return shift_amount;
}

VECTOR& operator >>= (VECTOR& v, uint32_t shift)
{
	for (; shift >= 64; shift -= 64)
	{
		v.low_part = v.high_part;
		v.high_part = 0;
	}

	v.low_part >>= shift;
	v.low_part |= v.high_part << (64 - shift);
	v.high_part >>= shift;
	return v;
}

VECTOR operator >> (const VECTOR& v, uint32_t shift)
{
	VECTOR result = v;
	result >>= shift;
	return result;
}

VECTOR& operator ^= (VECTOR& lhs, const VECTOR& rhs)
{
	lhs.low_part ^= rhs.low_part;
	lhs.high_part ^= rhs.high_part;
	return lhs;
}

VECTOR operator ^ (const VECTOR& lhs, const VECTOR& rhs)
{
	VECTOR result = lhs;
	result ^= rhs;
	return result;
}

std::ostream& operator<<(std::ostream& os, const VECTOR& v)
{
	os << std::hex << std::setfill('0') << std::setw(2 * sizeof(v.high_part)) << v.high_part << v.low_part;
	return os;
}

constexpr Hash::Hash(const PARAMETERS& parameters) :
	m_parameters(parameters),
	m_state(parameters.initial_state)
{
}

Hash::Hash(const PARAMETERS& parameters, uint8_t* data, size_t size) :
	Hash(parameters)
{
	update(data, size);
}

void Hash::update(uint8_t* data, size_t size)
{
	for (size_t i = 0; i < size; ++i)
	{
		uint8_t current_byte = data[i];
		for (size_t j = 0; j < 8; ++j, current_byte >>= 1)
		{
			// TODO: Consider loading m_state to a local
			_update_bit(current_byte & 1);
		}
	}	
}

void Hash::_update_bit(size_t bit)
{
	const uint32_t shift_amount = bit_scan_forward(m_state) + 1;

	m_state >>= shift_amount;
	m_state ^= m_parameters.polynomials[bit];
}

}