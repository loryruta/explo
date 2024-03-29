#pragma once

#include <array>
#include <functional>
#include <glm/glm.hpp>
#include <mutex>
#include <optional>
#include <vector>
#include <vren/model/model.hpp>

#include "world/surface/SurfaceGenerator.hpp"
#include "world/volume/Octree.hpp"
#include "world/volume/VolumeGenerator.hpp"

namespace explo
{
    // Forward decl
    class World;
    class WorldView;

    class BakedWorldView;

    // ------------------------------------------------------------------------------------------------

    /* Surface */
    using chunk_surface_t = vren::model;

    /* Block */
    using block_t = uint8_t;

    class Chunk : public std::enable_shared_from_this<Chunk>
    {
        friend class World;
        friend class WorldView;
        friend class BakedWorldView;
        friend class VolumeGenerator;
        friend class SurfaceGenerator;

    public:
        static constexpr glm::ivec3 k_grid_size = glm::ivec3(16, 256, 16);  ///< How many blocks does a chunk contain
        static constexpr glm::vec3 k_world_size =
            glm::vec3(16.0f, 256.0f, 16.0f);  ///< The size of the chunk in world space (commonly equal to the grid size)

    private:
        World &m_world;
        glm::ivec3 m_position;

        mutable std::mutex m_volume_mutex;
        std::unique_ptr<Octree> m_octree;

        std::unique_ptr<Surface> m_surface;

    public:
        explicit Chunk(World &world, glm::ivec3 const &position);
        ~Chunk();

        World &get_world() const { return m_world; }
        glm::ivec3 const &get_position() const { return m_position; }

        Octree &octree() const;

        uint8_t get_block_type_at(glm::ivec3 const &block_pos) const;
        void set_block_type_at(glm::ivec3 const &block_pos, uint8_t block_type);

        bool has_surface() const { return bool(m_surface); };
        std::unique_ptr<Surface> const &get_surface() const { return m_surface; }

        glm::uvec3 const &get_grid_size() const { return k_grid_size; }

        bool test_block_position(glm::ivec3 const &block_pos);
        glm::ivec3 to_world_block_position(glm::ivec3 const &chunk_block_pos) const;
        glm::vec3 to_world_position(glm::vec3 const &chunk_pos) const;

        static glm::ivec3 to_chunk_block_position(glm::ivec3 const &block_pos);
        static glm::vec3 to_chunk_position(glm::vec3 const &world_pos);

        /// Checks whether the relative block position is inside the chunk.
        static bool test_chunk_block_position(glm::ivec3 const &chunk_block_pos);

        static glm::ivec3 get_position(glm::ivec3 const &block_pos);
    };
}  // namespace explo
