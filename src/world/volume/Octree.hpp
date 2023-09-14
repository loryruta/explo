#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <vector>

namespace explo
{
    class Octree
    {
        static constexpr size_t k_grow_size = 1024;

        using TraversalCallbackT = std::function<void(uint32_t value, uint32_t level, uint32_t morton_code)>;

    private:
        std::vector<uint32_t> m_data;
        uint32_t m_depth;
        uint32_t m_next_alloc_index = 8;

    public:
        explicit Octree(uint32_t depth);
        ~Octree();

        void const *data() const { return m_data.data(); }
        size_t size() const { return m_data.size(); }

        uint32_t get_voxel_at(uint32_t morton_code) const;
        void set_voxel_at(uint32_t morton_code, uint32_t value);

        // TODO Add a function to compact the octree: group 2x2x2 nodes with identical value into one

        void traverse(TraversalCallbackT const &callback) const;

        static uint32_t to_morton_code(glm::ivec3 const &voxel_pos);
        static glm::ivec3 to_voxel_position(uint32_t morton_code);

    private:
        void traverse_r(uint32_t node_idx, uint32_t depth, uint32_t morton_code, TraversalCallbackT const &callback) const;
    };
}  // namespace explo
