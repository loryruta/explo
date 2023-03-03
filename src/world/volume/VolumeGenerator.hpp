#pragma once

#include "world/Chunk.hpp"

namespace explo
{
	class VolumeGenerator
	{
	public:
		explicit VolumeGenerator() = default;
		~VolumeGenerator() = default;

		/// Generates and sets the volume storage for the given chunk.
		virtual void generate_volume(Chunk& chunk) = 0;
	};
} // namespace explo
