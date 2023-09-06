#include "Chunk.hpp"

#include "Game.hpp"
#include "World.hpp"

using namespace explo;

Chunk::Chunk(World& world, glm::ivec3 const& position) :
	m_world(world),
	m_position(position)
{
}

Chunk::~Chunk()
{
}

bool Chunk::has_volume() const
{
	std::lock_guard<std::mutex> lock(m_volume_mutex);
	return bool(m_volume);
}

VolumeStorage& Chunk::get_volume() const
{
	std::lock_guard<std::mutex> lock(m_volume_mutex);
	if (!m_volume) throw std::runtime_error("No volume");
	return *m_volume;
}

void Chunk::set_volume(std::unique_ptr<VolumeStorage>&& volume)
{
	std::lock_guard<std::mutex> lock(m_volume_mutex);
	m_volume = std::move(volume);
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

uint8_t Chunk::get_block_type_at(glm::ivec3 const& block) const
{
	if (!m_volume || !Chunk::test_chunk_block_position(block))
		return 0;

	return m_volume->get_block_type_at(block);
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
