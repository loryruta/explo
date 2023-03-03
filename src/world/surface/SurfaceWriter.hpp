#pragma once

#include <cstdint>

#include "Surface.hpp"

namespace explo
{
	/// A helper class intended to aid the generation of a Surface.
	class SurfaceWriter
	{
	private:
		Surface& m_surface;

	public:
		explicit SurfaceWriter(Surface& surface);
		~SurfaceWriter() = default;

		Surface& surface() { return m_surface; }

		uint32_t add_vertex(SurfaceVertex const& v);
		void add_triangle(SurfaceVertex const& v0, SurfaceVertex const& v1, SurfaceVertex const& v2);

		/// Adds two triangles forming a quad; the triangles generated are (v0, v1, v2) and (v2, v1, v3).
		/// Vertex ordering (clockwise or counter-clockwise) is up to the user.
		void add_quad(SurfaceVertex const& v0, SurfaceVertex const& v1, SurfaceVertex const& v2, SurfaceVertex const& v3);

		void add_index(SurfaceIndex index);

		void add_instance(SurfaceInstance const& instance);
	};
} // namespace explo
