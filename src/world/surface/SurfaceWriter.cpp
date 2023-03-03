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
	add_vertex(v0);
	add_vertex(v1);
	add_vertex(v2);
}

void SurfaceWriter::add_quad(SurfaceVertex const& v0, SurfaceVertex const& v1, SurfaceVertex const& v2, SurfaceVertex const& v3)
{
	add_triangle(v0, v1, v2);
	add_triangle(v2, v1, v3);
}

void SurfaceWriter::add_index(uint32_t index)
{
	m_surface.m_indices.push_back(index);
}

void SurfaceWriter::add_instance(SurfaceInstance const& instance)
{
	m_surface.m_instances.push_back(instance);
}
