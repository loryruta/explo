#include "SinCosVolumeGenerator.hpp"

#include "storage/OctreeVolumeStorage.hpp"

using namespace explo;

void SinCosVolumeGenerator::generate_volume(Chunk& chunk)
{
	if (chunk.get_position().y == 0)
	{
		std::unique_ptr<OctreeVolumeStorage> volume = std::make_unique<OctreeVolumeStorage>(chunk.get_grid_size());
		for (int x = 0; x < Chunk::k_grid_size.x; x++)
		{
			for (int z = 0; z < Chunk::k_grid_size.z; z++)
			{
				volume->set_block_type_at(glm::ivec3(x, 0, z), 1);
			}
		}
		chunk.set_volume(std::move(volume));
	}
}
