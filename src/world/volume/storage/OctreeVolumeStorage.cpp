#include "OctreeVolumeStorage.hpp"

#include <cassert>

#include <glm/gtc/integer.hpp>

#include "util/misc.hpp"

using namespace explo;

OctreeVolumeStorage::OctreeVolumeStorage(glm::uvec3 const& size) :
	VolumeStorage(size)
{
	m_octree_size = glm::ivec3(ceil_to_power_of_2(size.x), ceil_to_power_of_2(size.y), ceil_to_power_of_2(size.z));
	m_octree_resolution = glm::log2(glm::max(glm::max(m_octree_size.x, m_octree_size.y), m_octree_size.z));

	// Initialize the root node to be a big empty node
	m_octree.m_block = 0; // Empty block
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

	OctreeNode const& node = follow_morton_code(morton_code, nullptr);
	return node.m_block;
}

void OctreeVolumeStorage::set_block_type_at(glm::ivec3 const& block_pos, uint8_t block_type)
{
	assert(
		block_pos.x >= 0 && block_pos.x < m_size.x &&
		block_pos.y >= 0 && block_pos.y < m_size.y &&
		block_pos.z >= 0 && block_pos.z < m_size.z
		);

	uint64_t morton_code = make_morton_code(block_pos);

	OctreeNode* node = &m_octree;
	for (int i = 0; i < m_octree_resolution; i++)
	{
		if (node->m_block == block_type) return; // The block is already set throughout its morton path
		else if (node->m_block != UINT8_MAX)
		{
			// A node in its path is set to another block type. Set its children to that block type, and go on with our traversal.
			// This happens when compaction is performed
			for (int j = 0; j < 8; j++)
			{
				std::shared_ptr<OctreeNode>& child = node->m_children.at(j);
				if (!child) child = std::make_shared<OctreeNode>();
				child->m_block = node->m_block;
			}
			node->m_block = UINT8_MAX; // Make the node a parent node
		}

		uint32_t child_idx = (morton_code >> (m_octree_resolution - i - 1) * 3) & 0b111;

		std::shared_ptr<OctreeNode>& child = node->m_children.at(child_idx);
		if (!child) child = std::make_shared<OctreeNode>();
		node = child.get();
	}

	node->m_block = block_type;

	// TODO Once the block is set, compact the octree: group 2x2x2 nodes set with the same block
}

void OctreeVolumeStorage::traverse_octree_recursive(OctreeNode const& node, uint32_t level, uint64_t morton_code, OctreeTraversalCallbackT const& callback) const
{
	callback(node, level, morton_code);

	if (node.m_block == UINT8_MAX)
	{
		for (int child_idx = 0; child_idx < 8; child_idx++)
		{
			OctreeNode& child = *node.m_children.at(child_idx);
			traverse_octree_recursive(child, level + 1, (morton_code << 3) | (child_idx & 0b111), callback);
		}
	}
}

void OctreeVolumeStorage::traverse_octree(OctreeTraversalCallbackT const& callback) const
{
	traverse_octree_recursive(m_octree, 0 /* level */, 0 /* morton_code */, callback);
}

OctreeVolumeStorage::OctreeNode const& OctreeVolumeStorage::follow_morton_code(uint64_t morton_code, OctreeTraversalCallbackT const& callback) const
{
	OctreeNode const* node = &m_octree;

	for (int i = 0; i < m_octree_resolution; i++)
	{
		uint32_t child_idx = (morton_code >> (m_octree_resolution - i - 1) * 3) & 0b111;

		if (callback)
		{
			callback(*node, i, morton_code);
			break; // If the callback returns false, then stops the traversal
		}

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
	int i = 0;
	while (x || y || z)
	{
		morton_code |= ((x & 1) | ((y & 1) << 1) | ((z & 1) << 2)) << (i * 3);

		x >>= 1;
		y >>= 1;
		z >>= 1;

		i++;
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
