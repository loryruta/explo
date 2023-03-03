#pragma once

#include <cstdint>

#include <vren/gpu_repr.hpp>

namespace explo
{
	using SurfaceVertex = vren::vertex;

	using SurfaceIndex = uint32_t;

	using SurfaceInstance = vren::mesh_instance;

	struct Surface
	{
		std::vector<SurfaceVertex> m_vertices;
		std::vector<SurfaceIndex> m_indices;
		std::vector<SurfaceInstance> m_instances;
	};
} // namespace explo
