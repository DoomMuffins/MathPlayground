#include <intrin.h>
#include <cstdint>
#include <array>

namespace tshash {

struct VECTOR
{
	uint64_t low_part;
	uint64_t high_part;
};

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

template<class Parameters>
class TSHash
{
public:
	constexpr explicit TSHash();
	TSHash(uint8_t* data, size_t size);
	void update(uint8_t* data, size_t size);
	VECTOR digest() { return m_state; }

private:
	void _update_bit(size_t bit);

	VECTOR m_state;
};

template <class Parameters>
constexpr TSHash<Parameters>::TSHash(): 
	m_state(Parameters::initial_state())
{
}

template<class Parameters>
TSHash<Parameters>::TSHash(uint8_t* data, size_t size) : 
	TSHash()
{
	update(data, size);
}

template<class Parameters>
void TSHash<Parameters>::update(uint8_t* data, size_t size)
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

template<class Parameters>
void TSHash<Parameters>::_update_bit(size_t bit)
{
	constexpr const auto polynomials = Parameters::polynomials();
	
	const uint32_t shift_amount = bit_scan_forward(m_state) + 1;

	m_state >>= shift_amount;
	m_state ^= polynomials[bit];
}

}

struct MyParameters
{
	static constexpr const tshash::VECTOR initial_state() { return{ 0, 0b0'1100'0000 }; }
	static constexpr const std::array<tshash::VECTOR, 2> polynomials() {
		return{ { { 1, 1 }, {1, 1} } };
	}
};

int main()
{
	std::array<uint8_t, 10> arr{ 1, 2, 3 };
	tshash::TSHash<MyParameters> hash;
	hash.update(arr.data(), arr.size());
    return 0;
}

