#include "DeltaChunkIterator.hpp"

#include "log.hpp"
#include "util/Aabb.hpp"

using namespace explo;

DeltaChunkIterator::DeltaChunkIterator(
    glm::ivec3 const &old_center, glm::ivec3 const &new_center, glm::ivec3 const &render_distance, CallbackT const &callback
) :
    m_old_center(old_center),
    m_new_center(new_center),
    m_render_distance(render_distance),
    m_callback(callback)
{
}

DeltaChunkIterator::~DeltaChunkIterator() {}

void DeltaChunkIterator::iterate_along_axis(int axis)
{
    assert(axis >= 0 && axis < 3);
    assert(m_new_center[axis] != m_old_center[axis]);

    int ax0 = axis;  // The axis of movement
    int ax1 = (axis + 1) % 3;
    int ax2 = (axis + 2) % 3;

    int axis_delta = m_new_center[axis] - m_old_center[axis];
    int axis_from = glm::max(glm::abs(axis_delta) - m_render_distance[ax0], m_render_distance[ax0] + 1);
    int axis_to = glm::abs(axis_delta) + m_render_distance[ax0] + 1;

    for (int c = axis_from; c < axis_to; c++)
    {
        for (int a = -m_render_distance[ax1]; a <= m_render_distance[ax1]; a++)
        {
            for (int b = -m_render_distance[ax2]; b <= m_render_distance[ax2]; b++)
            {
                glm::ivec3 chunk_pos{};
                chunk_pos[ax0] = m_old_center[ax0] + c * glm::sign(axis_delta);
                chunk_pos[ax1] = m_new_center[ax1] + a;
                chunk_pos[ax2] = m_new_center[ax2] + b;

                if (!m_visited_chunks.contains(chunk_pos))
                {
                    m_callback(chunk_pos);
                    m_visited_chunks.emplace(chunk_pos);
                }
            }
        }
    }
}

void DeltaChunkIterator::iterate()
{
    for (int axis = 0; axis < 3; axis++)
    {
        if (m_new_center[axis] != m_old_center[axis]) iterate_along_axis(axis);
    }
}
