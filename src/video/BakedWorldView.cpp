#include "BakedWorldView.hpp"

#include "Renderer.hpp"

using namespace explo;

// --------------------------------------------------------------------------------------------------------------------------------
// BakedWorldViewCircularGrid
// --------------------------------------------------------------------------------------------------------------------------------

BakedWorldViewCircularGrid::BakedWorldViewCircularGrid(Renderer& renderer, int render_distance) :
	m_renderer(renderer),

	m_start{},
	m_render_distance(render_distance)
{
	m_side = m_render_distance * 2 + 1;

	glm::ivec3 gpu_image_size{
		m_side * 2, // The X axis is doubled in order to store the chunk information
		m_side,
		m_side
	};
	m_gpu_image = std::make_unique<DeviceImage3d>(m_renderer, gpu_image_size, VK_FORMAT_R32G32B32A32_UINT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	m_cpu_image.resize(m_side * m_side * m_side);
}

BakedWorldViewCircularGrid::~BakedWorldViewCircularGrid()
{
}

BakedWorldViewCircularGrid::Pixel& BakedWorldViewCircularGrid::read_pixel(glm::ivec3 const& position)
{
	assert(
		position.x >= 0 && position.x < m_side &&
		position.y >= 0 && position.y < m_side &&
		position.z >= 0 && position.z < m_side
		);

	size_t i = flatten_1d_index(position);
	return m_cpu_image.at(i);
}

void BakedWorldViewCircularGrid::write_pixel(glm::ivec3 const& position, Pixel const& pixel)
{
	assert(
		position.x >= 0 && position.x < m_side &&
		position.y >= 0 && position.y < m_side &&
		position.z >= 0 && position.z < m_side
		);

	glm::ivec3 mod_pos = (m_start + position) % m_side;

	// We split the pixel that we have to write into two pixels that lie side-by-side, along the X axis, in the final image
	glm::uvec4 p1 = {pixel.m_index_count, pixel.m_instance_count, pixel.m_first_index, pixel.m_vertex_offset};
	glm::uvec4 p2 = {pixel.m_first_instance, 1, 2, 3 /* Random numbers that can be useful for debug */};

	m_gpu_image->write_pixel(mod_pos * glm::ivec3(2, 1, 1), &p1, sizeof(p1));
	m_gpu_image->write_pixel(mod_pos * glm::ivec3(2, 1, 1) + glm::ivec3(1, 0, 0), &p2, sizeof(p2));

	uint32_t i = flatten_1d_index(mod_pos);
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

	m_vertex_buffer(
		m_renderer,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		k_vertex_buffer_init_size
		),
	m_vertex_buffer_allocator(k_vertex_buffer_init_size, sizeof(SurfaceVertex), 128 * 1024 /* 128KB */),

	m_index_buffer(
		m_renderer,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		k_index_buffer_init_size
	),
	m_index_buffer_allocator(k_index_buffer_init_size, sizeof(SurfaceIndex), 128 * 1024 /* 128KB */),

	m_instance_buffer(
		m_renderer,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		k_instance_buffer_init_size
	),
	m_instance_buffer_allocator(k_instance_buffer_init_size, sizeof(SurfaceInstance), 1024 /* 1KB */),

	m_circular_grid(renderer, render_distance)
{
}

BakedWorldView::~BakedWorldView()
{
}

void BakedWorldView::shift(glm::ivec3 const& offset)
{
	int side = m_circular_grid.m_side;
	m_circular_grid.m_start = pmod(m_circular_grid.m_start + offset, glm::ivec3(side));
}

size_t BakedWorldView::place_data(DeviceBuffer& buffer, VirtualAllocator& allocator, void* data, size_t data_size)
{
	size_t alloc_offset;
	while (true)
	{
		if (allocator.allocate(data_size, alloc_offset)) break;

		// If it can't allocate, try to slightly increase the buffer size
		size_t buffer_size = glm::ceil(double(allocator.get_size()) * 1.7);
		allocator.resize(buffer_size);
	}

	// If a resize was done, make sure the buffer size matches the virtual allocator size
	if (buffer.get_size() != allocator.get_size())
		buffer.resize(allocator.get_size());

	buffer.write(data, data_size, alloc_offset);
	return alloc_offset;
}

void BakedWorldView::upload_chunk(glm::ivec3 const& position, Chunk const& chunk)
{
	std::unique_ptr<Surface> const& surface = chunk.m_surface;
	assert(surface);

	if (surface->m_vertices.empty() || surface->m_indices.empty() || surface->m_instances.empty()) return;

	size_t vertex_offset = place_data(
		m_vertex_buffer,
		m_vertex_buffer_allocator,
		surface->m_vertices.data(),
		surface->m_vertices.size() * sizeof(SurfaceVertex)
		);

	size_t index_offset = place_data(
		m_index_buffer,
		m_index_buffer_allocator,
		surface->m_indices.data(),
		surface->m_indices.size() * sizeof(SurfaceIndex)
		);

	size_t instance_offset = place_data(
		m_instance_buffer,
		m_instance_buffer_allocator,
		surface->m_instances.data(),
		surface->m_instances.size() * sizeof(SurfaceInstance)
		);

	// Set the chunk's draw call within the circular grid
	assert(vertex_offset % sizeof(SurfaceVertex) == 0);
	assert(index_offset % sizeof(SurfaceIndex) == 0);
	assert(instance_offset % sizeof(SurfaceInstance) == 0);

	BakedWorldViewCircularGrid::Pixel pixel{};
	pixel.m_index_count = surface->m_indices.size();
	pixel.m_instance_count = surface->m_instances.size();
	pixel.m_first_index = index_offset / sizeof(SurfaceIndex);
	pixel.m_vertex_offset = vertex_offset / sizeof(SurfaceVertex);
	pixel.m_first_instance = instance_offset / sizeof(SurfaceInstance);

	m_circular_grid.write_pixel(position, pixel);
}

void BakedWorldView::destroy_chunk(glm::ivec3 const& position)
{
	BakedWorldViewCircularGrid::Pixel& pixel = m_circular_grid.read_pixel(position);

	if (pixel.m_index_count == 0) return; // The pixel was invalid

	size_t vertex_offset = pixel.m_vertex_offset * sizeof(SurfaceVertex);
	size_t index_offset = pixel.m_first_index * sizeof(SurfaceIndex);
	size_t instance_offset = pixel.m_first_instance * sizeof(SurfaceInstance);

	m_vertex_buffer_allocator.free(vertex_offset);
	m_index_buffer_allocator.free(index_offset);
	m_instance_buffer_allocator.free(instance_offset);

	pixel.m_index_count = 0; // Invalidate the pixel

	m_circular_grid.write_pixel(position, pixel);
}
