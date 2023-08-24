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

glm::vec3 Chunk::to_world_position(glm::vec3 const& chunk_relative_position) const
{
	return glm::vec3(m_position) * Chunk::k_world_size + chunk_relative_position;
}

uint8_t Chunk::get_block_type_at(glm::ivec3 const& block) const
{
	if (!m_volume || !Chunk::is_block_inside(block))
		return 0;

	return m_volume->get_block_type_at(block);
}

bool Chunk::is_block_inside(glm::ivec3 const& block)
{
	return
		block.x >= 0 && block.x < k_grid_size.x &&
		block.y >= 0 && block.y < k_grid_size.y &&
		block.z >= 0 && block.z < k_grid_size.z;
}
