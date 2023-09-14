#pragma once

#include <glm/glm.hpp>
#include <vren/vk_helpers/image.hpp>

#include "DeviceBuffer.hpp"
#include "DeviceImage3d.hpp"
#include "world/Chunk.hpp"

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
        friend class DebugUi;

    public:
        // The size of this struct is 8 x sizeof(uint32_t) = 8 x 4 = 32 bytes
        // The required image format should be VK_FORMAT_R64G64B64A64_UINT (32 bytes), but it's not widely supported!
        // Therefore a meaningful pixel is made to be a group blocks of 2x2x2 pixels of VK_FORMAT_R8G8B8A8_UINT
        struct Pixel
        {
            uint32_t m_index_count;  // If index_count == 0, then it's invalid
            uint32_t m_instance_count;
            uint32_t m_first_index;
            uint32_t m_vertex_offset;
            uint32_t m_first_instance;
        };

    private:
        Renderer &m_renderer;

        glm::ivec3 m_render_distance;
        glm::ivec3 m_side;

        glm::ivec3 m_start;

        std::unique_ptr<DeviceImage3d> m_gpu_image;
        std::vector<Pixel> m_cpu_image;

    public:
        explicit BakedWorldViewCircularGrid(Renderer &renderer, glm::ivec3 const &render_distance);
        ~BakedWorldViewCircularGrid();

        Pixel &read_pixel(glm::ivec3 const &pos);
        void write_pixel(glm::ivec3 const &pos, Pixel const &pixel);

    private:
        glm::ivec3 to_image_index(glm::ivec3 const &pos) const;
        size_t to_flatten_index(glm::ivec3 const &pos) const;
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
        static constexpr size_t k_vertex_buffer_init_size = 64 * 1024 * 1024;  // 64MB
        static constexpr size_t k_index_buffer_init_size = 64 * 1024 * 1024;   // 64MB
        static constexpr size_t k_instance_buffer_init_size = 1024 * 1024;     // 1MB

    private:
        Renderer &m_renderer;

        glm::ivec3 m_position;
        glm::ivec3 m_render_distance;

        DeviceBuffer m_vertex_buffer;
        VirtualAllocator m_vertex_buffer_allocator;

        DeviceBuffer m_index_buffer;
        VirtualAllocator m_index_buffer_allocator;

        DeviceBuffer m_instance_buffer;
        VirtualAllocator m_instance_buffer_allocator;

        BakedWorldViewCircularGrid m_circular_grid;

    public:
        explicit BakedWorldView(Renderer &renderer, glm::ivec3 const &init_position, glm::ivec3 const &render_distance);
        ~BakedWorldView();

        bool is_chunk_position_inside(glm::ivec3 const &chunk_pos) const;

        /// Sets the position of the world view. As internally uses a CircularGrid, chunks that would be still in WorldView
        /// after the movement will be preserved. However, the responsibility of destroying old chunks is left to the user.
        /// NOTE: Old chunks should be destroyed using `destroy_chunk()` before calling this function.
        void set_position(glm::ivec3 const &position);
        void upload_chunk(Chunk const &chunk);
        void destroy_chunk(glm::ivec3 const &chunk_pos);

    private:
        /// Places the given data in the device buffer eventually re-allocating it if doesn't fit.
        /// \return The offset, within the buffer, where the data is allocated.
        size_t place_data(DeviceBuffer &buffer, VirtualAllocator &allocator, void *data, size_t data_size);

        glm::ivec3 to_relative_chunk_position(glm::ivec3 const &chunk_pos) const;
    };
}  // namespace explo
