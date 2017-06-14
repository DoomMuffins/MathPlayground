#pragma once

#include <array>
#include <cstdint>
#include <iostream>
#include <iomanip>


namespace tshash {

// TODO: protect against usages that set more bits than the vector declares to handle?

template<size_t Bits>
struct BIT_VECTOR
{
	std::array<uint64_t, (Bits + 63) / 64> data;

	template<size_t DstBits>
	explicit operator BIT_VECTOR<DstBits>() const
	{
		BIT_VECTOR<DstBits> vec{};
		
		constexpr auto total_bits_to_copy = std::min(Bits, DstBits);
		constexpr auto words_to_copy = total_bits_to_copy / 64;
		constexpr auto last_bits_to_copy = total_bits_to_copy % 64;
		
		std::copy_n(data.begin(), words_to_copy, vec.data.begin());
		// avoiding "conditional expression is constant" without ugly pragmas
		if ((void)0, last_bits_to_copy > 0)
		{
			vec.data[words_to_copy] = data[words_to_copy] & ((1ULL << last_bits_to_copy % 64) - 1);
		}		
		
		return vec;
	}
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
BIT_VECTOR<Bits>& operator &= (BIT_VECTOR<Bits>& lhs, const BIT_VECTOR<Bits>& rhs)
{
	for (size_t i = 0; i < lhs.data.size(); ++i)
	{
		lhs.data[i] &= rhs.data[i];
	}
	return lhs;
}

template<size_t Bits>
BIT_VECTOR<Bits> operator & (const BIT_VECTOR<Bits>& lhs, const BIT_VECTOR<Bits>& rhs)
{
	auto result = lhs;
	result &= rhs;
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
void set_polynomial_term(BIT_VECTOR<Bits>& vec, size_t term_degree)
{
	const size_t tap_bit_index = Bits - term_degree - 1;
	auto& word = vec.data[tap_bit_index / 64];
	word |= 1ULL << (tap_bit_index % 64);
}

template<size_t Bits>
BIT_VECTOR<Bits> create_polynomial(const size_t* term_degrees, size_t term_degrees_count)
{
	BIT_VECTOR<Bits> polynomial{};
	for (size_t i = 0; i < term_degrees_count; ++i)
	{
		set_polynomial_term(polynomial, term_degrees[i]);
	}

	// The constant term must always be set to avoid degenerate case
	set_polynomial_term(polynomial, 0);
	
	return polynomial;
}

template<size_t Bits>
BIT_VECTOR<Bits> create_polynomial(std::initializer_list<size_t> term_degrees_list)
{
	return create_polynomial<Bits>(term_degrees_list.begin(), term_degrees_list.size());
}

template<size_t Bits>
class Hash
{
public:
	using DigestType = BIT_VECTOR<Bits>;
	using BitVectorType = BIT_VECTOR<Bits + 2>;
	using ParametersType = PARAMETERS<Bits + 2>;	

	constexpr explicit Hash(const ParametersType& parameters) :
		m_parameters(parameters),
		m_state(parameters.initial_state)
	{}

	Hash(const ParametersType& parameters, const uint8_t* data, size_t size) :
		Hash(parameters)
	{
		update_bytecount(data, size);
	}

	void update_bytecount(const uint8_t* data, size_t bytecount)
	{
		update_bitcount(data, bytecount * CHAR_BIT);
	}

	void update_bitcount(const uint8_t* data, size_t bitcount)
	{
		const size_t bytecount = (bitcount + CHAR_BIT - 1) / CHAR_BIT;

		for (size_t i = 0; i < bytecount; ++i)
		{
			uint8_t current_byte = data[i];
			const size_t bits = (i < bytecount - 1) ? CHAR_BIT : (1 + (bitcount - 1) % CHAR_BIT);
			
			for (size_t j = 0; j < bits; ++j, current_byte >>= 1)
			{
				// TODO: Consider loading m_state to a local
				_update_bit(current_byte & 1);
			}
		}
	}

	DigestType digest() const { return static_cast<DigestType>(m_state); }
	void reset() { m_state = m_parameters.initial_state; }

	static BitVectorType create_polynomial(std::initializer_list<size_t> term_degrees_list) { return tshash::create_polynomial<Bits + 2>(term_degrees_list); }

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