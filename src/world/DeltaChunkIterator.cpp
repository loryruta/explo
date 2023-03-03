#include "DeltaChunkIterator.hpp"

#include "WorldView.hpp"
#include "util/Aabb.hpp"

using namespace explo;

DeltaChunkIterator::DeltaChunkIterator(
	glm::ivec3 const& old_center,
	glm::ivec3 const& new_center,
	int render_distance,
	CallbackT const& callback
) :
	m_old_center(old_center),
	m_new_center(new_center),
	m_render_distance(render_distance),
	m_callback(callback)
{
	m_covered.resize(WorldView::calc_size(m_render_distance));
}

void DeltaChunkIterator::iterate_recursive(glm::ivec3 const& current_chunk)
{
	size_t world_view_size = WorldView::calc_size(m_render_distance);

	// Make the left-bottom-back chunk in the world view be the (0, 0, 0) and right-top-front the (world_view_side, world_view_side, world_view_side)
	glm::ivec3 rel_chunk_pos = current_chunk - m_new_center + m_render_distance;

	uint32_t chunk_idx = WorldView::relative_position_to_index(m_render_distance, rel_chunk_pos);
	assert(chunk_idx < world_view_size);

	if (m_covered[chunk_idx]) return;

	m_covered[chunk_idx] = true;
	m_callback(current_chunk);

	glm::ivec3 offsets[] = {
		glm::ivec3(0, 1, 0),
		glm::ivec3(0, -1, 0),
		glm::ivec3(1, 0, 0),
		glm::ivec3(-1, 0, 0),
		glm::ivec3(0, 0, 1),
		glm::ivec3(0, 0, -1),
	};

	Aabb old_view_box(m_old_center - m_render_distance, m_old_center + m_render_distance);
	Aabb new_view_box(m_new_center - m_render_distance, m_new_center + m_render_distance);

	for (glm::ivec3 const& offset : offsets)
	{
		glm::ivec3 neighbor_chunk = current_chunk + offset;

		if (old_view_box.is_inside(neighbor_chunk))
			continue; // The neighbor is still part of the old world view, we don't visit it

		if (!new_view_box.is_inside(neighbor_chunk))
			continue; // The neighbor isn't part of the new world view, we don't visit it

		iterate_recursive(neighbor_chunk);
	}
}

void DeltaChunkIterator::iterate()
{
	if (m_old_center == m_new_center)
		return;

	// Finds an initial chunk that is part of the new world view to start the recursion from
	glm::ivec3 initial_chunk = m_new_center + glm::sign(m_new_center - m_old_center) * m_render_distance;
	iterate_recursive(initial_chunk);
}
