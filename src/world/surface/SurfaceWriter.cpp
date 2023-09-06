#include "SurfaceWriter.hpp"

using namespace explo;

SurfaceWriter::SurfaceWriter(Surface& surface) :
	m_surface(surface)
{
}

uint32_t SurfaceWriter::add_vertex(SurfaceVertex const& v)
{
	uint32_t i = m_surface.m_vertices.size();
	m_surface.m_vertices.push_back(v);
	return i;
}

void SurfaceWriter::add_triangle(SurfaceVertex const& v0, SurfaceVertex const& v1, SurfaceVertex const& v2)
{
	uint32_t i0 = add_vertex(v0);
	uint32_t i1 = add_vertex(v1);
	uint32_t i2 = add_vertex(v2);

	add_index(i0); add_index(i1); add_index(i2);
}

void SurfaceWriter::add_quad(SurfaceVertex const& v0, SurfaceVertex const& v1, SurfaceVertex const& v2, SurfaceVertex const& v3)
{
	uint32_t i0 = add_vertex(v0);
	uint32_t i1 = add_vertex(v1);
	uint32_t i2 = add_vertex(v2);
	uint32_t i3 = add_vertex(v3);

	add_index(i0); add_index(i1); add_index(i2);
	add_index(i0); add_index(i2); add_index(i3);
}

void SurfaceWriter::add_index(uint32_t index)
{
	m_surface.m_indices.push_back(index);
}

void SurfaceWriter::add_instance(SurfaceInstance const& instance)
{
	m_surface.m_instances.push_back(instance);
}
