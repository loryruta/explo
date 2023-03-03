#include "BakedWorldView.hpp"

#include "Renderer.hpp"

using namespace explo;

// --------------------------------------------------------------------------------------------------------------------------------
// BakedWorldViewCircularGrid
// --------------------------------------------------------------------------------------------------------------------------------

BakedWorldViewCircularGrid::BakedWorldViewCircularGrid(Renderer& renderer, int render_distance) :
	m_renderer(renderer)
{
	m_side = render_distance * 2 + 1;
	glm::ivec3 gpu_image_size{
		m_side * 2, // The X axis is doubled in order to store the chunk information
		m_side,
		m_side
	};
	m_gpu_image = std::make_unique<DeviceImage3d>(m_renderer, gpu_image_size, VK_FORMAT_R32G32B32A32_UINT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	m_image_info.m_start = {0, 0, 0};
	m_image_info.m_render_distance = render_distance;
}

BakedWorldViewCircularGrid::~BakedWorldViewCircularGrid()
{
}

BakedWorldViewCircularGrid::Pixel& BakedWorldViewCircularGrid::read_pixel(glm::ivec3 const& position)
{
	return m_cpu_image.at(flatten_1d_index(position));
}

void BakedWorldViewCircularGrid::write_pixel(glm::ivec3 const& position, Pixel const& pixel)
{
	assert(
		position.x >= 0 && position.x < m_side &&
		position.y >= 0 && position.y < m_side &&
		position.z >= 0 && position.z < m_side
		);

	glm::ivec3 mod_position = (m_image_info.m_start + position) % m_side;
	m_gpu_image->write_pixel(mod_position, (void*) &pixel, sizeof(pixel));

	uint32_t i = flatten_1d_index(mod_position);
	m_cpu_image[i] = pixel;
}

size_t BakedWorldViewCircularGrid::flatten_1d_index(glm::ivec3 const& position) const
{
	return position.y * m_side * m_side + position.x * m_side + position.z;
}

// --------------------------------------------------------------------------------------------------------------------------------
// BakedWorldView
// --------------------------------------------------------------------------------------------------------------------------------

BakedWorldView::BakedWorldView(Renderer& renderer, int render_distance) :
	m_renderer(renderer),
	m_render_distance(render_distance),

	m_vertex_buffer(
		m_renderer,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		k_vertex_buffer_init_size
		),
	m_vertex_buffer_allocator(k_vertex_buffer_init_size),

	m_index_buffer(
		m_renderer,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		k_index_buffer_init_size
	),
	m_index_buffer_allocator(k_index_buffer_init_size),

	m_instance_buffer(
		m_renderer,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		k_instance_buffer_init_size
	),
	m_instance_buffer_allocator(k_instance_buffer_init_size),

	m_circular_grid(renderer, render_distance)
{
}

BakedWorldView::~BakedWorldView()
{
}

void BakedWorldView::shift(glm::ivec3 const& offset)
{
	m_circular_grid.m_image_info.m_start =
		(m_circular_grid.m_image_info.m_start + offset) % m_circular_grid.m_side;
}

void BakedWorldView::upload_chunk(glm::ivec3 const& position, Chunk const& chunk)
{
	std::unique_ptr<Surface> const& surface = chunk.m_surface;
	assert(surface);

	// Upload vertices
	size_t vertex_offset;
	size_t vertex_size = surface->m_vertices.size() * sizeof(SurfaceVertex);
	m_vertex_buffer_allocator.allocate(vertex_size, vertex_offset);
	if (vertex_offset + vertex_size > m_vertex_buffer.get_size()) // Resize the vertex buffer if necessary
		m_vertex_buffer.resize(vertex_offset + vertex_size);
	m_vertex_buffer.write(surface->m_vertices.data(), vertex_size, vertex_offset);

	// Upload indices
	size_t index_offset;
	size_t index_size = surface->m_indices.size() * sizeof(SurfaceIndex);
	m_index_buffer_allocator.allocate(index_size, index_offset);
	if (index_offset + index_size > m_index_buffer.get_size()) // Resize the index buffer if necessary
		m_index_buffer.resize(index_offset + index_size);
	m_index_buffer.write(surface->m_indices.data(), index_size, index_offset);

	// Upload instances
	size_t instance_offset;
	size_t instance_size = surface->m_instances.size() * sizeof(SurfaceInstance);
	m_instance_buffer_allocator.allocate(instance_size, instance_offset);
	if (instance_offset + instance_size > m_instance_buffer.get_size()) // Resize the instance buffer if necessary
		m_instance_buffer.resize(instance_offset + instance_size);
	m_instance_buffer.write(surface->m_instances.data(), instance_size, instance_offset);

	// Set reference within the CircularGrid
	BakedWorldViewCircularGrid::Pixel pixel{};
	pixel.m_index_count = surface->m_indices.size();
	pixel.m_instance_count = surface->m_instances.size();
	pixel.m_first_index = index_offset;
	pixel.m_vertex_offset = vertex_offset;
	pixel.m_first_instance = instance_offset;

	m_circular_grid.write_pixel(position, pixel);
}

void BakedWorldView::destroy_chunk(glm::ivec3 const& position)
{
	BakedWorldViewCircularGrid::Pixel& pixel = m_circular_grid.read_pixel(position);

	m_vertex_buffer_allocator.free(pixel.m_vertex_offset);
	m_index_buffer_allocator.free(pixel.m_first_index);
	m_instance_buffer_allocator.free(pixel.m_first_instance);

	pixel.m_index_count = 0; // Invalidate the pixel

	m_circular_grid.write_pixel(position, pixel);
}
