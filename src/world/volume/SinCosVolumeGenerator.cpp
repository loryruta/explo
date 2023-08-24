#include "SinCosVolumeGenerator.hpp"

#include "storage/OctreeVolumeStorage.hpp"

using namespace explo;

void SinCosVolumeGenerator::generate_volume(Chunk& chunk)
{
	std::unique_ptr<OctreeVolumeStorage> volume = std::make_unique<OctreeVolumeStorage>(chunk.get_grid_size());
	volume->set_block_type_at(glm::ivec3(0, 0, 0), 1);
	chunk.set_volume(std::move(volume));
}
