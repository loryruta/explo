#include "Aabb.hpp"

using namespace explo;

Aabb::Aabb(glm::vec3 const &min, glm::vec3 const &max) :
    m_min(min),
    m_max(max)
{
}

Aabb::~Aabb() {}

bool Aabb::is_inside(glm::vec3 const &point) const
{
    return point.x >= m_min.x && point.y >= m_min.y && point.z >= m_min.z && point.x <= m_max.x && point.y <= m_max.y && point.z <= m_max.z;
}

bool Aabb::intersect(Aabb const &aabb) const
{
    return (m_min.x <= aabb.m_max.x && m_max.x >= aabb.m_min.x) && (m_min.y <= aabb.m_max.y && m_max.y >= aabb.m_min.y) &&
           (m_min.z <= aabb.m_max.z && m_max.z >= aabb.m_min.z);
}
