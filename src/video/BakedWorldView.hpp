#pragma once

#include <glm/glm.hpp>

#include <vren/vk_helpers/image.hpp>

#include "world/Chunk.hpp"
#include "DeviceBuffer.hpp"
#include "DeviceImage3d.hpp"

namespace explo
{
	// Forward decl
	class Renderer;

	// ------------------------------------------------------------------------------------------------
	// BakedWorldViewCircularGrid
	// ------------------------------------------------------------------------------------------------

	/// A circular 3d image (just like a circular buffer, but with 3 dimensions), where every pixel is a chunk and stores the
	/// reference to its geometry (i.e. vertex, index and instance data). The advantage of using as circular structure, is for
	/// shifting whilst the player is moving.
	class BakedWorldViewCircularGrid
	{
		friend class Renderer;
		friend class BakedWorldView;
		friend class CullWorldView;

	public:
		// The size of this struct is 8 x sizeof(uint32_t) = 8 x 4 = 32 bytes
		// The required image format should be VK_FORMAT_R64G64B64A64_UINT (32 bytes), but it's not widely supported!
		// Therefore a meaningful pixel is made to be a group blocks of 2x2x2 pixels of VK_FORMAT_R8G8B8A8_UINT
		struct Pixel
		{
			uint32_t m_index_count; // If index_count == 0, then it's invalid
			uint32_t m_instance_count;
			uint32_t m_first_index;
			uint32_t m_vertex_offset;
			uint32_t m_first_instance;
			uint32_t _pad[3];

			bool is_valid() const { return m_index_count > 0; }
		};

	private:
		Renderer& m_renderer;

		int m_side;

		std::unique_ptr<DeviceImage3d> m_gpu_image;
		std::vector<Pixel> m_cpu_image;

		struct { // GPU compliant
			glm::ivec3 m_start; int m_render_distance;
		} m_image_info;

	public:
		explicit BakedWorldViewCircularGrid(Renderer& renderer, int render_distance);
		~BakedWorldViewCircularGrid();

		Pixel& read_pixel(glm::ivec3 const& position);
		void write_pixel(glm::ivec3 const& position, Pixel const& pixel);

	private:
		size_t flatten_1d_index(glm::ivec3 const& position) const;
	};

	// ------------------------------------------------------------------------------------------------
	// BakedWorldView
	// ------------------------------------------------------------------------------------------------

	class BakedWorldView
	{
		friend class Renderer;
		friend class CullWorldView;
		friend class DrawChunkList;
		friend class DebugUi;

	public:
		static constexpr size_t k_vertex_buffer_init_size = 64 * 1024 * 1024; // 64MB
		static constexpr size_t k_index_buffer_init_size = 64 * 1024 * 1024; // 64MB
		static constexpr size_t k_instance_buffer_init_size = 1024 * 1024; // 1MB

	private:
		Renderer& m_renderer;

		int m_render_distance;

		DeviceBuffer m_vertex_buffer;
		VirtualAllocator m_vertex_buffer_allocator;

		DeviceBuffer m_index_buffer;
		VirtualAllocator m_index_buffer_allocator;

		DeviceBuffer m_instance_buffer;
		VirtualAllocator m_instance_buffer_allocator;

		BakedWorldViewCircularGrid m_circular_grid;

	public:
		explicit BakedWorldView(Renderer& renderer, int render_distance);
		~BakedWorldView();

		/// Shifts the world view by a certain offset (expressed in chunk space).
		void shift(glm::ivec3 const& offset);

		/// Uploads the chunk geometry to the given world view space position.
		void upload_chunk(glm::ivec3 const& position, Chunk const& chunk);

		/// Destroys the chunk that was uploaded at the given world view space position.
		void destroy_chunk(glm::ivec3 const& position);
	};
} // namespace explo
