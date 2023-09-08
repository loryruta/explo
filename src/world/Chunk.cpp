#include "Chunk.hpp"

#include <glm/gtc/integer.hpp>

#include "Game.hpp"
#include "World.hpp"

using namespace explo;

Chunk::Chunk(World& world, glm::ivec3 const& position) :
	m_world(world),
	m_position(position)
{
	uint32_t max_side = std::max(std::max(Chunk::k_grid_size.x, Chunk::k_grid_size.y), Chunk::k_grid_size.z);
	uint32_t octree_depth = glm::log2(ceil_to_power_of_2(max_side));
	m_octree = std::make_unique<Octree>(octree_depth);
}

Chunk::~Chunk()
{
}

Octree& Chunk::octree() const
{
	return *m_octree;
}

glm::ivec3 Chunk::to_world_block_position(glm::ivec3 const& chunk_block_pos) const
{
	return m_position * Chunk::k_grid_size + chunk_block_pos;
}

bool Chunk::test_block_position(glm::ivec3 const& block_pos)
{
	glm::ivec3 chunk_pos = Chunk::get_position(block_pos);
	return chunk_pos == m_position;
}

glm::vec3 Chunk::to_world_position(glm::vec3 const& chunk_pos) const
{
	return glm::vec3(m_position) * Chunk::k_world_size + chunk_pos;
}

glm::ivec3 Chunk::to_chunk_block_position(glm::ivec3 const& block_pos)
{
	return glm::floor(glm::vec3(block_pos) / glm::vec3(Chunk::k_grid_size));
}

glm::vec3 Chunk::to_chunk_position(glm::vec3 const& world_pos)
{
	return glm::mod(world_pos, Chunk::k_world_size);
}

uint8_t Chunk::get_block_type_at(glm::ivec3 const& block_pos) const
{
	if (Chunk::test_chunk_block_position(block_pos))
		return 0; // TODO if the block coord is not inside the chunk, throw an error instead!
	return m_octree->get_voxel_at(Octree::to_morton_code(block_pos));
}

void Chunk::set_block_type_at(glm::ivec3 const& block_pos, uint8_t block_type)
{
	assert(Chunk::test_chunk_block_position(block_pos));
	return m_octree->set_voxel_at(Octree::to_morton_code(block_pos), block_type);
}

bool Chunk::test_chunk_block_position(glm::ivec3 const& chunk_block_pos)
{
	return
		chunk_block_pos.x >= 0 && chunk_block_pos.x < k_grid_size.x &&
		chunk_block_pos.y >= 0 && chunk_block_pos.y < k_grid_size.y &&
		chunk_block_pos.z >= 0 && chunk_block_pos.z < k_grid_size.z;
}

glm::ivec3 Chunk::get_position(glm::ivec3 const& block_pos)
{
	return glm::floor(glm::vec3(block_pos) / glm::vec3(Chunk::k_grid_size));
}
