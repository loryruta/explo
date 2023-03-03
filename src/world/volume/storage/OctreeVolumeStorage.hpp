#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <functional>

#include <glm/glm.hpp>

#include "VolumeStorage.hpp"

namespace explo
{
	class OctreeVolumeStorage : public VolumeStorage
	{
	public:
		struct OctreeNode
		{
			uint8_t m_block = UINT8_MAX; // UINT8_MAX if parent node, otherwise leaf
			std::array<std::shared_ptr<OctreeNode>, 8> m_children;

			bool is_leaf() const { return m_block != UINT8_MAX; };
		};

		using OctreeTraversalCallbackT =
			std::function<bool(OctreeNode const& node, uint32_t level, uint64_t morton_code)>;

	private:
		uint32_t m_octree_resolution;
		OctreeNode m_octree;
		size_t m_num_octree_nodes = 1;

	public:
		explicit OctreeVolumeStorage(glm::uvec3 const& size);
		~OctreeVolumeStorage();

		uint32_t get_octree_resolution() const { return m_octree_resolution; };
		OctreeNode& get_octree() { return m_octree; };

		size_t get_byte_size() const override;

		uint8_t get_block_type_at(glm::ivec3 const& block_pos) const override;
		void set_block_type_at(glm::ivec3 const& block_pos, uint8_t block_type) override;

		void traverse_octree(OctreeTraversalCallbackT const& callback) const;
		OctreeNode const& follow_morton_code(uint64_t morton_code, OctreeTraversalCallbackT const& callback) const;

		static glm::ivec3 make_position_from_morton_code(uint64_t morton_code);

	private:
		void traverse_octree_recursive(OctreeNode const& node, uint32_t level, uint64_t morton_code, OctreeTraversalCallbackT const& callback) const;

		static uint64_t make_morton_code(glm::ivec3 const& block_pos);
	};

} // namespace explo
