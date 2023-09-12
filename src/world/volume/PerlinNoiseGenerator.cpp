#include "PerlinNoiseGenerator.hpp"


using namespace explo;

PerlinNoiseGenerator::PerlinNoiseGenerator() :
	m_perlin_noise(1693894559)
{
}

PerlinNoiseGenerator::~PerlinNoiseGenerator()
{
}

int PerlinNoiseGenerator::get_height_at(int x, int z)
{
	const double k_frequency = 0.023;
	double val = m_perlin_noise.noise2D(double(x) * k_frequency, double(z) * k_frequency);
	return ((val + 1.0) / 2.0) * k_max_world_height;
}

void PerlinNoiseGenerator::generate_volume(Chunk& chunk)
{
	for (int x = 0; x < 16; x++)
	{
		for (int z = 0; z < 16; z++)
		{
			glm::ivec3 block_pos = chunk.to_world_block_position(glm::ivec3{x, 0, z});

			int base_y = get_height_at(block_pos.x, block_pos.z);
			block_pos.y = base_y;

			if (chunk.test_block_position(block_pos))
			{
				int min_neighbor_y = block_pos.y;
				min_neighbor_y = glm::min(get_height_at(block_pos.x - 1, block_pos.z), min_neighbor_y);
				min_neighbor_y = glm::min(get_height_at(block_pos.x, block_pos.z - 1), min_neighbor_y);
				min_neighbor_y = glm::min(get_height_at(block_pos.x + 1, block_pos.z), min_neighbor_y);
				min_neighbor_y = glm::min(get_height_at(block_pos.x, block_pos.z + 1), min_neighbor_y);
				min_neighbor_y = glm::min(get_height_at(block_pos.x + 1, block_pos.z + 1), min_neighbor_y);
				min_neighbor_y = glm::min(get_height_at(block_pos.x + 1, block_pos.z - 1), min_neighbor_y);
				min_neighbor_y = glm::min(get_height_at(block_pos.x - 1, block_pos.z + 1), min_neighbor_y);
				min_neighbor_y = glm::min(get_height_at(block_pos.x - 1, block_pos.z - 1), min_neighbor_y);

				// TODO pick block types from BlockRegistry (e.g. as enums)
				chunk.set_block_type_at(chunk.to_chunk_position(block_pos), 1); // Grass
				block_pos.y--;

				int dirt_height = (int) (m_perlin_noise.noise2D(x ^ 3508739221, z ^ 2024663696) + 2.0);

				for (int i = 0; block_pos.y > min_neighbor_y && i < dirt_height; block_pos.y--, i++)
				{
					if (!chunk.test_block_position(block_pos)) break;
					chunk.set_block_type_at(chunk.to_chunk_position(block_pos), 2); // Dirt
				}

				for (; block_pos.y > min_neighbor_y; block_pos.y--)
				{
					if (!chunk.test_block_position(block_pos)) break;
					chunk.set_block_type_at(chunk.to_chunk_position(block_pos), 3); // Stone
				}
			}
		}
	}
}
