#include "BakedWorldView.hpp"

#include "Renderer.hpp"

using namespace explo;

// --------------------------------------------------------------------------------------------------------------------------------
// BakedWorldViewCircularGrid
// --------------------------------------------------------------------------------------------------------------------------------

BakedWorldViewCircularGrid::BakedWorldViewCircularGrid(
	Renderer& renderer,
	glm::ivec3 const& render_distance
	) :
	m_renderer(renderer),

	m_render_distance(render_distance),
	m_side(m_render_distance * 2 + 1)
{
	m_start = glm::ivec3(0);

	glm::ivec3 gpu_image_size(
		m_side.x * 2, // The X axis is doubled in order to store the chunk information
		m_side.y,
		m_side.z
		);
	m_gpu_image = std::make_unique<DeviceImage3d>(m_renderer, gpu_image_size, VK_FORMAT_R32G32B32A32_UINT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	m_cpu_image.resize(m_side.x * m_side.y * m_side.z);
}

BakedWorldViewCircularGrid::~BakedWorldViewCircularGrid()
{
}

glm::ivec3 BakedWorldViewCircularGrid::to_image_index(glm::ivec3 const& pos) const
{
	return (m_start + pos) % m_side;
}

size_t BakedWorldViewCircularGrid::to_flatten_index(glm::ivec3 const& pos) const
{
	glm::ivec3 mod_pos = to_image_index(pos);
	return mod_pos.y * (m_side.x * m_side.z) + mod_pos.x * m_side.z + mod_pos.z;
}

BakedWorldViewCircularGrid::Pixel& BakedWorldViewCircularGrid::read_pixel(glm::ivec3 const& pos)
{
	assert(
		pos.x >= 0 && pos.x < m_side.x &&
		pos.y >= 0 && pos.y < m_side.y &&
		pos.z >= 0 && pos.z < m_side.z
		);

	return m_cpu_image.at(to_flatten_index(pos));
}

void BakedWorldViewCircularGrid::write_pixel(glm::ivec3 const& pos, Pixel const& pixel)
{
	assert(
		pos.x >= 0 && pos.x < m_side.x &&
		pos.y >= 0 && pos.y < m_side.y &&
		pos.z >= 0 && pos.z < m_side.z
		);


	// We split the pixel that we have to write into two pixels that lie side-by-side, along the X axis, in the final image
	glm::uvec4 p1 = {pixel.m_index_count, pixel.m_instance_count, pixel.m_first_index, pixel.m_vertex_offset};
	glm::uvec4 p2 = {pixel.m_first_instance, 1, 2, 3 /* Random numbers that can be useful for debug */};

	glm::ivec3 img_idx = to_image_index(pos);

	m_gpu_image->write_pixel(img_idx * glm::ivec3(2, 1, 1), &p1, sizeof(p1));
	m_gpu_image->write_pixel(img_idx * glm::ivec3(2, 1, 1) + glm::ivec3(1, 0, 0), &p2, sizeof(p2));

	m_cpu_image[to_flatten_index(pos)] = pixel;
}

// --------------------------------------------------------------------------------------------------------------------------------
// BakedWorldView
// --------------------------------------------------------------------------------------------------------------------------------

BakedWorldView::BakedWorldView(
	Renderer& renderer,
	glm::ivec3 const& init_position,
	glm::ivec3 const& render_distance
	) :
	m_renderer(renderer),

	m_position(init_position),
	m_render_distance(render_distance),

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

bool BakedWorldView::is_chunk_position_inside(glm::ivec3 const& chunk_pos) const
{
	glm::ivec3 side = m_render_distance * 2 + 1;
	glm::ivec3 rel_pos = to_relative_chunk_position(chunk_pos);
	return
		rel_pos.x >= 0 && rel_pos.x < side.x &&
		rel_pos.y >= 0 && rel_pos.y < side.y &&
		rel_pos.z >= 0 && rel_pos.z < side.z;
}

glm::ivec3 BakedWorldView::to_relative_chunk_position(glm::ivec3 const& chunk_pos) const
{
	return chunk_pos - m_position + m_render_distance;
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

void BakedWorldView::set_position(glm::ivec3 const& new_position)
{
	glm::ivec3 offset = new_position - m_position;

	m_circular_grid.m_start = pmod(m_circular_grid.m_start + offset, m_circular_grid.m_side);

	m_position = new_position;
}

void BakedWorldView::upload_chunk(Chunk const& chunk)
{
	// The user asked to upload a chunk that is outside the world view. This can happen frequently because the chunk construction
	// is asynchronous and if the player is travelling through the world fast, chunks could be built when they're no longer inside
	// the world view
	glm::ivec3 chunk_pos = chunk.get_position();
	if (!is_chunk_position_inside(chunk_pos)) return;

	std::unique_ptr<Surface> const& surface = chunk.m_surface;

	// The chunk doesn't have the surface! Instead of throwing, we silently ignore the uploading
	if (!surface ||
		surface->m_vertices.empty() ||
		surface->m_indices.empty() ||
		surface->m_instances.empty()
		) return;

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

	m_circular_grid.write_pixel(to_relative_chunk_position(chunk_pos), pixel);
}

void BakedWorldView::destroy_chunk(glm::ivec3 const& chunk_pos)
{
	// If the chunk asked to be destroyed isn't covered by the world view anymore, we have lost references to it (that are
	// stored in the circular image): we can't free its geometry!
	// IMPORTANT: The responsibility of destroying chunks that exit the world view is left to the user!

	if (!is_chunk_position_inside(chunk_pos)) return;

	glm::ivec3 rel_pos = to_relative_chunk_position(chunk_pos);
	BakedWorldViewCircularGrid::Pixel& pixel = m_circular_grid.read_pixel(rel_pos);

	if (pixel.m_index_count == 0) return; // The pixel was invalid

	size_t vertex_offset = pixel.m_vertex_offset * sizeof(SurfaceVertex);
	size_t index_offset = pixel.m_first_index * sizeof(SurfaceIndex);
	size_t instance_offset = pixel.m_first_instance * sizeof(SurfaceInstance);

	m_vertex_buffer_allocator.free(vertex_offset);
	m_index_buffer_allocator.free(index_offset);
	m_instance_buffer_allocator.free(instance_offset);

	pixel.m_index_count = 0; // Invalidate the pixel

	m_circular_grid.write_pixel(rel_pos, pixel);
}
