#pragma once

#include "VolumeGenerator.hpp"

namespace explo
{
	class SinCosVolumeGenerator : public VolumeGenerator
	{
	public:
		explicit SinCosVolumeGenerator() = default;
		~SinCosVolumeGenerator() = default;

		void generate_volume(Chunk& chunk) override;
	};

} // namespace explo
