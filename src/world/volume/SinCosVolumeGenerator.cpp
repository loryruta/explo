#include "SinCosVolumeGenerator.hpp"

#include "util/misc.hpp"

using namespace explo;

void SinCosVolumeGenerator::generate_volume(Chunk &chunk)
{
    if (chunk.get_position().y != 0 && chunk.get_position().y != 1) return;

    for (int x = 0; x < Chunk::k_grid_size.x; x++)
    {
        for (int z = 0; z < Chunk::k_grid_size.z; z++)
        {
            glm::ivec3 block_pos = chunk.to_world_block_position(glm::ivec3{x, 0, z});
            block_pos.y = glm::floor(((glm::sin(block_pos.x * 0.09f) + 1.0f) / 2.0f) * ((glm::cos(block_pos.z * 0.09f) + 1.0f) / 2.0f) * 32.0f);

            if (chunk.test_block_position(block_pos)) chunk.set_block_type_at(chunk.to_chunk_position(block_pos), 1);
        }
    }
}
