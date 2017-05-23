#pragma once

#include <array>
#include <cstdint>
#include <iostream>
#include <iomanip>


namespace tshash {

template<size_t Bits>
struct BIT_VECTOR
{
	std::array<uint64_t, (Bits + 63) / 64> data;
};

template<size_t Bits>
uint32_t bit_scan_forward(const BIT_VECTOR<Bits>& v)
{
	for (size_t i = 0; i < v.data.size(); ++i)
	{
		unsigned long set_bit_index;
		if (_BitScanForward64(&set_bit_index, v.data[i]))
		{
			return static_cast<uint32_t>(set_bit_index + 64 * i);
		}
	}
	return static_cast<uint32_t>(-1);
}

template<size_t Bits>
bool operator == (const BIT_VECTOR<Bits>& lhs, const BIT_VECTOR<Bits>& rhs)
{
	return lhs.data == rhs.data;
}

template<size_t Bits>
bool operator != (const BIT_VECTOR<Bits>& lhs, const BIT_VECTOR<Bits>& rhs)
{
	return !(lhs == rhs);
}

template<size_t Bits>
BIT_VECTOR<Bits>& operator >>= (BIT_VECTOR<Bits>& v, uint32_t shift)
{
	for (; shift >= 64; shift -= 64)
	{
		for (size_t i = 0; i < v.data.size() - 1; ++i)
		{
			v.data[i] = v.data[i + 1];
		}
		v.data.back() = 0;
	}

	for (size_t i = 0; i < v.data.size() - 1; ++i)
	{
		auto& current_word = v.data[i];
		current_word >>= shift;
		current_word |= (v.data[i + 1] << (64 - shift));
	}
	v.data.back() >>= shift;

	return v;
}

template<size_t Bits>
BIT_VECTOR<Bits> operator >> (const BIT_VECTOR<Bits>& v, uint32_t shift)
{
	auto result = v;
	result >>= shift;
	return result;
}

template<size_t Bits>
BIT_VECTOR<Bits>& operator ^= (BIT_VECTOR<Bits>& lhs, const BIT_VECTOR<Bits>& rhs)
{
	for (size_t i = 0; i < lhs.data.size(); ++i)
	{
		lhs.data[i] ^= rhs.data[i];
	}
	return lhs;
}

template<size_t Bits>
BIT_VECTOR<Bits> operator ^ (const BIT_VECTOR<Bits>& lhs, const BIT_VECTOR<Bits>& rhs)
{
	auto result = lhs;
	result ^= rhs;
	return result;
}

template<size_t Bits>
std::ostream& operator << (std::ostream& os, const BIT_VECTOR<Bits>& v)
{
	for (size_t i = v.data.size() - 1; i != 0; --i)
	{
		os << std::hex << std::setfill('0') << std::setw(2 * sizeof(v.data[i])) << v.data[i];
	}
	os << std::hex << std::setfill('0') << std::setw(2 * (Bits % 64) / 8) << v.data.front();
	return os;
}

template<size_t Bits>
struct PARAMETERS
{
	BIT_VECTOR<Bits> initial_state;
	std::array<BIT_VECTOR<Bits>, 2> polynomials;
};

template<size_t Bits>
class Hash
{
public:
	using BitVectorType = BIT_VECTOR<Bits>;
	using ParametersType = PARAMETERS<Bits>;	

	constexpr explicit Hash(const ParametersType& parameters) :
		m_parameters(parameters),
		m_state(parameters.initial_state)
	{}

	Hash(const ParametersType& parameters, uint8_t* data, size_t size) :
		Hash(parameters)
	{
		update(data, size);
	}

	void update(uint8_t* data, size_t size)
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

	BitVectorType digest() { return m_state; }
	void reset() { m_state = m_parameters.initial_state; }

private:
	void _update_bit(size_t bit)
	{
		const uint32_t shift_amount = bit_scan_forward(m_state) + 1;

		m_state >>= shift_amount;
		m_state ^= m_parameters.polynomials[bit];
	}

	const ParametersType m_parameters;
	BitVectorType m_state;
};

}