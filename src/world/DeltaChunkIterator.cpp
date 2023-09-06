#include "DeltaChunkIterator.hpp"

#include "log.hpp"
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
}

void DeltaChunkIterator::iterate_along_axis(int axis)
{
	assert(axis >= 0 && axis < 3);
	assert(m_new_center[axis] != m_old_center[axis]);

	int axis_delta = m_new_center[axis] - m_old_center[axis];
	int axis_from = glm::max(glm::abs(axis_delta) - m_render_distance, m_render_distance + 1);
	int axis_to = glm::abs(axis_delta) + m_render_distance + 1;

	int it = 0;

	for (int c = axis_from; c < axis_to; c++)
	{
		for (int a = -m_render_distance; a <= m_render_distance; a++)
		{
			for (int b = -m_render_distance; b <= m_render_distance; b++)
			{
				glm::ivec3 chunk_pos{};
				chunk_pos[axis] = m_old_center[axis] + c * glm::sign(axis_delta);
				chunk_pos[(axis + 1) % 3] = m_new_center[(axis + 1) % 3] + a;
				chunk_pos[(axis + 2) % 3] = m_new_center[(axis + 2) % 3] + b;

				m_callback(chunk_pos);

				it++;
			}
		}
	}
}

void DeltaChunkIterator::iterate()
{
	for (int axis = 0; axis < 3; axis++)
	{
		if (m_new_center[axis] != m_old_center[axis])
			iterate_along_axis(axis);
	}
}
