#include "OctreeVolumeStorage.hpp"

#include <cassert>

#include "util/misc.hpp"

using namespace explo;

OctreeVolumeStorage::OctreeVolumeStorage(glm::uvec3 const& size) :
	VolumeStorage(size)
{
	m_octree_resolution = glm::max(glm::max(ceil_to_power_of_2(size.x), ceil_to_power_of_2(size.y)), ceil_to_power_of_2(size.z));

	// Initialize the root node to be a big empty node
	m_octree.m_block = 0; // Empty block
	m_octree.m_children = {};
}

OctreeVolumeStorage::~OctreeVolumeStorage()
{
}

size_t OctreeVolumeStorage::get_byte_size() const
{
	return m_num_octree_nodes * sizeof(OctreeNode);
}

uint8_t OctreeVolumeStorage::get_block_type_at(glm::ivec3 const& block_pos) const
{
	assert(
		block_pos.x >= 0 && block_pos.x < m_size.x &&
		block_pos.y >= 0 && block_pos.y < m_size.y &&
		block_pos.z >= 0 && block_pos.z < m_size.z
		);

	uint64_t morton_code = make_morton_code(block_pos);
	return follow_morton_code(morton_code, nullptr).m_block;
}

void OctreeVolumeStorage::set_block_type_at(glm::ivec3 const& block_pos, uint8_t block_type)
{
	assert(
		block_pos.x >= 0 && block_pos.x < m_size.x &&
		block_pos.y >= 0 && block_pos.y < m_size.y &&
		block_pos.z >= 0 && block_pos.z < m_size.z
		);

	uint64_t morton_code = make_morton_code(block_pos);

	bool block_set = false;

	follow_morton_code(morton_code, [&](OctreeNode const& const_node, uint32_t level, uint64_t morton_code) -> bool
	{
		OctreeNode& node = const_cast<OctreeNode&>(const_node); // We don't want traverse_octree to be mutable, so we need this

		uint32_t child_idx = (morton_code >> (m_octree_resolution - level - 1) * 3) & 0b111;
		if (node.is_leaf())
		{
			if (node.m_block != block_type)
			{
				if (level + 1 == m_octree_resolution) // It's the last level, we can safely write the block
					node.m_block = block_type;
				else
				{
					// It's a higher level, we need to make it a parent and split it
					for (size_t i = 0; i < 8; i++)
					{
						OctreeNode child_node{};
						child_node.m_block = node.m_block;
						child_node.m_children = {};
						node.m_children[i] = std::make_shared<OctreeNode>(child_node);
					}
					node.m_block = UINT8_MAX; // Promote the node to parent

					// Differentiate the current child entry from the other children
					node.m_children.at(child_idx)->m_block = block_type;
				}

				block_set = false;
			}

			// We have to forcefully tell to stop the traversal since what was a leaf node before has now become a parent node
			// and thus the traversal would proceed with it
			return false;
		}

		// TODO NOT WORKING :')

		return true;
	});

	if (block_set)
	{
		// TODO if the block was set we can think of "compacting" the octree: group 2x2x2 nodes set with the same block
	}
}

void OctreeVolumeStorage::traverse_octree_recursive(
	OctreeNode const& node,
	uint32_t level,
	uint64_t morton_code,
	OctreeTraversalCallbackT const& callback
	) const
{
	bool cutoff_branch = callback(node, level, morton_code);

	if (!cutoff_branch && !node.is_leaf())
	{
		for (uint32_t child_idx = 0; child_idx < node.m_children.size(); child_idx++)
		{
			std::shared_ptr<OctreeNode> child = node.m_children.at(child_idx);
			if (!child)
				return;

			morton_code <<= 3;
			morton_code |= child_idx & 0b111;

			traverse_octree_recursive(*child, level + 1, morton_code, callback);
		}
	}
}

void OctreeVolumeStorage::traverse_octree(OctreeTraversalCallbackT const& callback) const
{
	traverse_octree_recursive(m_octree, 0, 0, callback);
}

OctreeVolumeStorage::OctreeNode const& OctreeVolumeStorage::follow_morton_code(uint64_t morton_code, OctreeTraversalCallbackT const& callback) const
{
	OctreeNode const* node = &m_octree;

	for (uint32_t level = 0; level < m_octree_resolution; level++)
	{
		uint32_t child_idx = (morton_code >> (m_octree_resolution - level - 1) * 3) & 0b111;

		if (callback && !callback(*node, level, morton_code))
			break; // If the callback returns false, then stops the traversal

		if (node->is_leaf()) break;
		else
			node = node->m_children.at(child_idx).get();
	}
	return *node;
}

uint64_t OctreeVolumeStorage::make_morton_code(glm::ivec3 const& position)
{
	uint32_t x = position.x;
	uint32_t y = position.y;
	uint32_t z = position.z;

	uint64_t morton_code = 0;
	while (x || y || z)
	{
		morton_code |= ((x & 1) << 2) | ((y & 1) << 1) | (z & 1);

		x >>= 1;
		y >>= 1;
		z >>= 1;

		morton_code <<= 3;
	}
	return morton_code;
}

glm::ivec3 OctreeVolumeStorage::make_position_from_morton_code(uint64_t morton_code)
{
	glm::ivec3 block_pos{};

	int i = 0;
	while (morton_code > 0)
	{
		block_pos.x |= (morton_code & 1) << i;
		block_pos.y |= ((morton_code & 0b10) >> 1) << i;
		block_pos.z |= ((morton_code & 0b100) >> 2) << i;

		morton_code >>= 3;
		i++;
	}
	return block_pos;
}
