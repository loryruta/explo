#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "SurfaceGenerator.hpp"
#include "world/volume/storage/OctreeVolumeStorage.hpp"

namespace explo
{
	class BlockySurfaceGenerator : public SurfaceGenerator
	{
	public:
		explicit BlockySurfaceGenerator() = default;
		~BlockySurfaceGenerator() = default;

		void generate(Chunk& chunk, SurfaceWriter& surface_writer) override;

	protected:
		void write_block_geometry(
			Chunk& chunk,
			glm::ivec3 const& block,
			uint8_t block_type,
			SurfaceWriter& surface_writer
			);

		void generate_surface_from_grid3d_volume(
			Chunk& chunk,
			VolumeStorage& volume_storage,
			SurfaceWriter& surface_writer
			);

		void generate_surface_from_octree_volume(
			Chunk& chunk,
			OctreeVolumeStorage& volume_storage,
			SurfaceWriter& surface_writer
			);
	};
} // namespace explo
