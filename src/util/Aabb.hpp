#pragma once

#include <glm/glm.hpp>

namespace explo
{
	struct Aabb
	{
		glm::vec3 m_min;
		glm::vec3 m_max;

		explicit Aabb(glm::vec3 const& min, glm::vec3 const& max);
		~Aabb();

		bool is_inside(glm::vec3 const& point) const;
		bool intersect(Aabb const& aabb) const;
	};

} // namespace explo
