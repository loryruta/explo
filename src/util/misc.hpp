#pragma once

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace explo
{
	struct vec_hash
	{
		template<glm::length_t _length, typename _t>
		size_t operator()(glm::vec<_length, _t> const& vec) const
		{
			size_t result = 0;
			for (glm::length_t i = 0; i < _length; i++)
				result ^= vec[i];
			return result;
		}
	};

	inline float normalize_angle(float a) // angle in radians
	{
		constexpr float pi = glm::pi<float>();

		a = glm::mod(a + pi, pi * 2);
		if (a < 0)
			a += pi * 2;
		return a - pi;
	}

	/// Converts the given byte size to a human readable string (e.g. 567296 -> 554Kb)
	std::string stringify_byte_size(size_t byte_size);

	uint64_t get_nanos_since_epoch();
	
	template<typename _IntT>
	_IntT ceil_to_power_of_2(_IntT n)
	{
		n--;
		n |= n >> 1;
		n |= n >> 2;
		n |= n >> 4;
		n |= n >> 8;
		n |= n >> 16;
		n++;
		return n;
	}

} // namespace explo