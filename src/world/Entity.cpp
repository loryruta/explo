#include "Entity.hpp"

#include "video/RenderApi.hpp"

using namespace explo;

Entity::Entity(World& world, glm::vec3 const& init_position) :
	m_world(&world),
	m_position(init_position),
	m_yaw(0.0f),
	m_pitch(0.0f)
{
	update_orientation_matrix();
}

Entity::~Entity()
{
}

void Entity::set_world(World& world, glm::vec3 const& position)
{
	m_world = &world;
	m_position = position;

	if (m_world_view) // If the entity already had a WorldView we recreate it with the new World
	{
		int render_distance = m_world_view->get_render_distance();
		recreate_world_view(render_distance);
	}
}

void Entity::set_position(glm::vec3 const& position)
{
	if (position == m_position)
		return;

	glm::ivec3 old_chunk_position = get_chunk_position();
	m_position = position;

	if (m_world_view)
	{
		glm::ivec3 new_chunk_position = get_chunk_position();
		if (old_chunk_position != new_chunk_position)
			m_world_view->set_position(new_chunk_position);
	}
}

glm::vec3 Entity::get_chunk_relative_position() const
{
	return glm::mod(m_position, Chunk::k_world_size);
}

glm::ivec3 Entity::get_chunk_position() const
{
	return glm::floor(m_position / Chunk::k_world_size);
}

void Entity::set_rotation(float yaw, float pitch)
{
	if (yaw == m_yaw && pitch == m_pitch)
		return;

	m_yaw = yaw;
	m_pitch = pitch;

	update_orientation_matrix();
}

void Entity::update_orientation_matrix()
{
	m_orientation_matrix = build_orientation_mat(m_yaw, m_pitch);
}

glm::vec3 Entity::get_right() const
{
	return m_orientation_matrix[0];
}

glm::vec3 Entity::get_up() const
{
	return m_orientation_matrix[1];
}

glm::vec3 Entity::get_forward() const
{
	return m_orientation_matrix[2];
}

bool Entity::has_world_view() const
{
	return bool(m_world_view);
}

WorldView& Entity::recreate_world_view(int render_distance)
{
	glm::ivec3 pos = get_chunk_position();

	RenderApi::world_view_recreate(pos, render_distance);
	m_world_view = std::make_unique<WorldView>(*m_world, pos, render_distance);

	return *m_world_view;
}

WorldView& Entity::get_world_view()
{
	// TODO check m_world_view != null
	return *m_world_view;
}
